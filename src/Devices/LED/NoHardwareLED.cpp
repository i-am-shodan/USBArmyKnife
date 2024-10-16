#ifdef NO_LED
#include "HardwareLED.h"

namespace Devices
{
    HardwareLED LED;
}

void HardwareLED::changeLEDState(bool on, uint8_t hue, uint8_t saturation, uint8_t lum, uint8_t brightness)
{
}

HardwareLED::HardwareLED()
{
}

void HardwareLED::loop(Preferences& prefs)
{
}

void HardwareLED::begin(Preferences &prefs)
{
}
#endif