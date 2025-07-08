#pragma once

#include <string>
#include <vector>
#include <functional>

#include "../../USBArmyKnifeCapability.h"

class HardwareIR : USBArmyKnifeCapability {
public:
HardwareIR();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
};

namespace Devices
{
    extern HardwareIR IR;
};