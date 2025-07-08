#pragma once

#include "../../USBArmyKnifeCapability.h"

class HardwareTouch : USBArmyKnifeCapability {
public:
  HardwareTouch();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
  bool hasBeenTouched() { return wasTouched; }
  void resetTouchState() { wasTouched = false; }
  int16_t getXPos() { return xPos; }
  int16_t getYPos() { return yPos; }
private:
  bool wasTouched = false;
  int16_t xPos = 0;
  int16_t yPos = 0;
};

namespace Devices
{
    extern HardwareTouch Touch;
}