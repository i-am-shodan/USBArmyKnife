#pragma once

#include "../../USBArmyKnifeCapability.h"

class ESP32Marauder : USBArmyKnifeCapability {
public:
  ESP32Marauder();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);

  void run(const std::string& cmd);
};

namespace Attacks
{
    extern ESP32Marauder Marauder;
}