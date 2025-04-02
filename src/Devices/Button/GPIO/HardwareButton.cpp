#ifndef NO_BUTTON
#ifndef LILYGO_T_WATCH_S3
#include <OneButton.h>

#include "../HardwareButton.h"
#include "../../../Debug/Logging.h"

static OneButton button(BTN_PIN, true);

namespace Devices
{
    HardwareButton Button;
}

HardwareButton::HardwareButton()
{
}

void HardwareButton::loop(Preferences& prefs)
{
    button.tick();
}

void HardwareButton::begin(Preferences &prefs)
{
  button.attachClick([] {
    Devices::Button.setButtonPressState(true, button.isLongPressed());
  });

  button.attachLongPressStop([] {
    Devices::Button.setButtonPressState(true, true);
  });
}
#endif
#endif