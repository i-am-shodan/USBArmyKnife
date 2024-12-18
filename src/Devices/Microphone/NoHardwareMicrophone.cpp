#ifdef NO_MIC
#include "HardwareMicrophone.h"

namespace Devices
{
    HardwareMicrophone Mic;
}

HardwareMicrophone::HardwareMicrophone()
{
}

void HardwareMicrophone::begin(Preferences &prefs)
{
}

void HardwareMicrophone::loop(Preferences &prefs)
{
}

void HardwareMicrophone::setCallback(const std::function<bool(uint8_t *, const size_t)> &callback)
{
}
#endif