#include <stdio.h>
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "dhcpserver/dhcpserver_options.h"
#include "lwip/esp_netif_net_stack.h"
#include "esp_mac.h"
#include "usb_ncm.h"

IsPacketReady hasNewPacket = NULL;
GetPacket getNextPacket = NULL;
FinishedProcessing finishedProcessingPacket = NULL;
TransmitPacket transmitPacket = NULL;
OnPacket onNewPacket = NULL;

static const char *TAG = "tusb_ncm";
static esp_netif_t *s_netif = NULL;

void set_packet_handler(OnPacket handler)
{
    onNewPacket = handler;
}

esp_err_t poll_for_packets()
{
    uint8_t *buf = NULL;
    uint32_t len = 0;
    esp_err_t ret = ESP_FAIL;

    if (hasNewPacket == NULL || finishedProcessingPacket == NULL || s_netif == NULL)
    {
        return ret;
    }

    do
    {
        if (hasNewPacket() == false)
        {
            ret = ESP_OK;
            break;
        }

        buf = getNextPacket(&len);

        if (buf == NULL)
        {
            finishedProcessingPacket();
            ret = ESP_OK;
            break;
        }

        if (len == 0)
        {
            finishedProcessingPacket();
            ret = ESP_OK;
            break;
        }

        void *buf_copy = malloc(len);
        if (!buf_copy)
        {
            ret = ESP_ERR_NO_MEM;
            finishedProcessingPacket();
            break;
        }
        memcpy(buf_copy, buf, len);

        if (onNewPacket != NULL)
        {
            onNewPacket(buf_copy, len, true);
        }

        ret = esp_netif_receive(s_netif, buf_copy, len, NULL);
        finishedProcessingPacket();

        break;

    } while (false);

    return ESP_OK;
}

static void l2_free(void *h, void *buffer)
{
    free(buffer);
}

static esp_err_t netif_transmit(void *h, void *buffer, size_t len)
{
    if (transmitPacket == NULL || buffer == NULL || len == 0)
    {
        return 0;
    }

    if (onNewPacket != NULL)
    {
        onNewPacket(buffer, len, false);
    }

    (void)transmitPacket(buffer, len);

    return ESP_OK;
}

esp_err_t usb_ncm_init(IsPacketReady pktReady, GetPacket getPacket, FinishedProcessing finished, TransmitPacket transmit)
{
    ip4_addr_t ip;
    ip4_addr_t gw;
    ip4_addr_t netmask;
    ip4addr_aton(STATIC_IP_ADDR, &ip);
    ip4addr_aton((const char *)DEFAULT_GATEWAY, &gw);
    ip4addr_aton((const char *)DEFAULT_NETMASK, &netmask);

    hasNewPacket = pktReady;
    getNextPacket = getPacket;
    finishedProcessingPacket = finished;
    transmitPacket = transmit;

    esp_netif_init();

    // Convert to esp_netif_ip_info_t
    esp_netif_ip_info_t ip_info;
    memset(&ip_info, 0, sizeof(esp_netif_ip_info_t));
    ip_info.ip.addr = ip.addr;
    ip_info.gw.addr = gw.addr;
    ip_info.netmask.addr = netmask.addr;

    // with OUI range MAC to create a virtual netif running http server
    // this needs to be different to usb_interface_mac (==client)
    uint8_t lwip_addr[6] = {0x02, 0x02, 0x11, 0x22, 0x33, 0x02};

    // Definition of
    // 1) Derive the base config (very similar to IDF's default WiFi AP with DHCP server)
    esp_netif_inherent_config_t base_cfg = {
        .flags = (esp_netif_flags_t)(ESP_NETIF_DHCP_SERVER | ESP_NETIF_FLAG_EVENT_IP_MODIFIED | ESP_NETIF_FLAG_AUTOUP),
        .ip_info = &ip_info,
        .get_ip_event = IP_EVENT_ETH_GOT_IP,
        .lost_ip_event = IP_EVENT_ETH_LOST_IP,
        .if_key = "usb_eth",
        .if_desc = "usb ncm config device",
        .route_prio = 10};

    // 2) Use static config for driver's config pointing only to static transmit and free functions
    esp_netif_driver_ifconfig_t driver_cfg = {
        .handle = (void *)1,             // not using an instance, USB-NCM is a static singleton (must be != NULL)
        .transmit = netif_transmit,      // point to static Tx function
        .driver_free_rx_buffer = l2_free // point to Free Rx buffer function
    };

    // Config the esp-netif with:
    //   1) inherent config (behavioural settings of an interface)
    //   2) driver's config (connection to IO functions -- usb)
    //   3) stack config (using lwip IO functions -- derive from eth)
    esp_netif_config_t cfg = {
        .base = &base_cfg,
        .driver = &driver_cfg,
        .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH, // USB-NCM is an Ethernet netif from lwip perspective, we already have IO definitions for that:
    };

    s_netif = esp_netif_new(&cfg);
    if (s_netif == NULL)
    {
        return ESP_FAIL;
    }
    if (esp_netif_set_mac(s_netif, lwip_addr) != ESP_OK)
    {
        return ESP_FAIL;
    }

    // start the interface manually (as the driver has been started already)
    esp_netif_action_start(s_netif, 0, 0, 0);

    if (!esp_netif_is_netif_up(s_netif))
    {
        return ESP_FAIL;
    }

    return ESP_OK;
}
