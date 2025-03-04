#ifdef WAVESHARE_RP2350_ETH
#include "HardwareEthernet.h"
#include "CH9120/CH9120.h"

static std::vector<std::function<void(uint8_t *, size_t)>> callbacks;

HardwareEthernet::HardwareEthernet()
{

}

void HardwareEthernet::begin(Preferences& prefs)
{
    CH9120_init();
}

void HardwareEthernet::loop(Preferences& prefs)
{
    RX_TX();
}

bool HardwareEthernet::writeData(uint8_t* buffer, const size_t size)
{

}

void HardwareEthernet::setCallback(std::function<void(uint8_t *, const size_t)> callback)
{
    callbacks.emplace_back(callback);
}

void HardwareEthernet::end()
{

}
#endif