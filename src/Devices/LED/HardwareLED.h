#pragma once

#include "../../USBArmyKnifeCapability.h"

class HardwareLED : USBArmyKnifeCapability {
public:
  HardwareLED();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
  void changeLEDState(bool on, uint8_t hue, uint8_t saturation, uint8_t lum, uint8_t brightness);
};

namespace Devices
{
    extern HardwareLED LED;
}