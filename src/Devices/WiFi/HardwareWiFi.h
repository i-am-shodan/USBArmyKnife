#pragma once

#include "../../USBArmyKnifeCapability.h"

class HardwareWiFi : USBArmyKnifeCapability {
public:
  HardwareWiFi();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
  virtual void end();
  void setWiFi(const bool& state);
  bool getState();
};

namespace Devices
{
    extern HardwareWiFi WiFi;
}