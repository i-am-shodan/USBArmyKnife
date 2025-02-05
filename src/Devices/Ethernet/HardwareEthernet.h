#pragma once

#include "../../USBArmyKnifeCapability.h"
#include <functional>

class HardwareEthernet : USBArmyKnifeCapability {
public:
  HardwareEthernet();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
  virtual void end();

  bool writeData(uint8_t*, const size_t);
  void setCallback(std::function<void(uint8_t*, const size_t)>);
};

namespace Devices
{
    extern HardwareEthernet Ethernet;
}