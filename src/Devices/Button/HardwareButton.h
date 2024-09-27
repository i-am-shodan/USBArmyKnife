#pragma once

#include "../../USBArmyKnifeCapability.h"

class HardwareButton : USBArmyKnifeCapability {
public:
  HardwareButton();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
  bool hasButtonBeenPressed() { return buttonPressedState; }
  void resetButtonPressedState() { buttonPressedState = false; }
  void setButtonPressState(bool state, bool isLongPress) { buttonPressedState = state; buttonPressedStateWasLong = isLongPress; }
  bool wasLastPressLong() { return buttonPressedStateWasLong; }
private:
  bool buttonPressedState = false;
  bool buttonPressedStateWasLong = false;
};

namespace Devices
{
    extern HardwareButton Button;
}