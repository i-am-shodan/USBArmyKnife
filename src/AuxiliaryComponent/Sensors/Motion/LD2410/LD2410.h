#pragma once

#include "../../../../USBArmyKnifeCapability.h"

class LD2410MotionSensorExtension : USBArmyKnifeCapability {
public:
  LD2410MotionSensorExtension();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
};