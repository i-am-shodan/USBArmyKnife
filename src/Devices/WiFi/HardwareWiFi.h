#pragma once

#include "../../USBArmyKnifeCapability.h"

class HardwareWiFi : USBArmyKnifeCapability {
public:
  HardwareWiFi();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
  void setWiFi(const bool& state);
};

namespace Devices
{
    extern HardwareWiFi WiFi;
}