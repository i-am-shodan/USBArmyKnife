#ifdef LILYGO_T_WATCH_S3
#include "../HardwareButton.h"
#include "../../../Debug/Logging.h"

#define TAG "Button"

// The button is actually managed by the board support class
// Due to it being related to the board power circuit

namespace Devices
{
  HardwareButton Button;
}

HardwareButton::HardwareButton()
{
}

void HardwareButton::begin(Preferences &prefs)
{
}

void HardwareButton::loop(Preferences &prefs)
{
}

#endif