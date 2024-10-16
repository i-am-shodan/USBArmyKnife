#include "USBNCM.h"
#include "../../Debug/Logging.h"

#include <Adafruit_TinyUSB.h>
#include <usb_ncm.h>
#include <tcp_server.h>

#ifdef NO_SD
    #include <SPIFFS.h>
    #define FILE_INTERFACE SPIFFS
#else
    #include "SDMMCFS2.h"
    using namespace fs;
    #define FILE_INTERFACE SD_MMC_2
#endif

#include "../../Utilities/PcapBuffer.h"

static Adafruit_USBD_NET usbNetworking;
static bool readyForPackets = false;
static PcapBuffer* buf = nullptr;

#define USBNCM_PCAPONSTART  "pcapNcmOnStart"
#define USBNCM_PCAPONSTART_DEFAULT  false

USBNCM::USBNCM()
{
    registerUserConfigurableSetting(CATEGORY_USB, USBNCM_PCAPONSTART, USBArmyKnifeCapability::SettingType::Bool, USBNCM_PCAPONSTART_DEFAULT);
}

extern "C" void createPcap(uint8_t *buffer, uint32_t len, bool received)
{
    if (buf != nullptr)
    {
        buf->append(buffer, len);
    }
}

void USBNCM::startPacketCollection()
{
    if (Devices::USB::Core.currentDeviceType() == USBDeviceType::NCM && buf == nullptr)
    {
        buf = new PcapBuffer(std::string("usbncm"), &FILE_INTERFACE);
        set_packet_handler(&createPcap);
        Debug::Log.info("NCM", "Started PCAP");
    }
    else
    {
        Debug::Log.error("NCM", "Error starting PCAP, incorrect mode or missing SD");
    }
}

void USBNCM::stopPacketCollection()
{
    Debug::Log.info("NCM", "Stopping PCAP");
    set_packet_handler(NULL);
    if (buf != nullptr)
    {
        delete buf;
        buf = nullptr;
    }
}

void USBNCM::begin(Preferences &prefs)
{
    if (Devices::USB::Core.currentDeviceType() == USBDeviceType::NCM)
    {
        if (readyForPackets == false && usb_ncm_init(&usbnet_hasNewPacket, &usbnet_getPacket, &usbnet_releasePacket, &usbnet_transmitPacket) == ESP_OK)
        {
            bool pcapOnStart = prefs.getBool(USBNCM_PCAPONSTART, USBNCM_PCAPONSTART_DEFAULT);
            if (pcapOnStart)
            {
                startPacketCollection();
            }

            //Starting this task seems to take resources away from NimBLE meaning its tasks can't start
            //xTaskCreate(tcp_server_task, "tcp_server", 4096, (void *)1234, 4, NULL);
            Debug::Log.info("NCM", "NCM Init OK");
            readyForPackets = true;
            TinyUSB_Device_Init(0, true);
        }
        else
        {
            Debug::Log.error("NCM", "NCM Init error");
        }
    }
}

void USBNCM::loop(Preferences &prefs)
{
    if (Devices::USB::Core.currentDeviceType() == USBDeviceType::NCM && readyForPackets)
    {
        poll_for_packets();
    }

    if (buf != nullptr)
    {
        // access to files 
        // Most applications need to use the protocol layer only in one task.
        // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/sdmmc.html
        buf->flushToDisk();
    }
}

void USBNCM::end()
{
    if (buf != nullptr)
    {
        stopPacketCollection();
    }
}