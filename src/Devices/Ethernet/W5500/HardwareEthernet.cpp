#ifdef WAVESHARE_ESP32_S3_ETH
#include "../HardwareEthernet.h"
#include "../../../Debug/Logging.h"

static std::vector<std::function<void(uint8_t *, size_t)>> callbacks;

#include <SPI.h>
#include <Ethernet.h>

#define LOG_ETH "Eth"

EthernetClient client;
extern uint8_t tud_network_mac_address[6];

HardwareEthernet::HardwareEthernet()
{
}

void HardwareEthernet::begin(Preferences &prefs)
{
    Ethernet.init(ETH_INT);

    do
    {
        if (Ethernet.begin(tud_network_mac_address) == 0)
        {
            Debug::Log.error(LOG_ETH, "Ethernet.begin error");
            break;
        }

        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {
            Debug::Log.error(LOG_ETH, "Ethernet EthernetNoHardware");
            break;
        }

        if (Ethernet.linkStatus() == LinkOFF)
        {
            Debug::Log.error(LOG_ETH, "Ethernet LinkOFF");
            break;
        }

        Debug::Log.info(LOG_ETH, Ethernet.localIP().toString().c_str());

    } while (false);
}

void HardwareEthernet::loop(Preferences &prefs)
{
    switch (Ethernet.maintain())
    {
    case 1:
        // renewed fail
        Debug::Log.info(LOG_ETH, "Error: renewed fail");
        break;

    case 2:
        // renewed success
        Debug::Log.info(LOG_ETH, "Renewed success");
        // print your local IP address:
        Debug::Log.info(LOG_ETH, std::string("My IP address: ")+Ethernet.localIP().toString().c_str());
        break;

    case 3:
        // rebind fail
        Debug::Log.info(LOG_ETH, "Error: rebind fail");
        break;

    case 4:
        // rebind success
        Debug::Log.info(LOG_ETH, "Rebind success");
        Debug::Log.info(LOG_ETH, std::string("My IP address: ")+Ethernet.localIP().toString().c_str());
        break;

    default:
        // nothing happened
        break;
    }
}

bool HardwareEthernet::writeData(uint8_t *buffer, const size_t size)
{
    return true;
}

void HardwareEthernet::setCallback(std::function<void(uint8_t *, const size_t)> callback)
{
    callbacks.emplace_back(callback);
}

void HardwareEthernet::end()
{
}
#endif