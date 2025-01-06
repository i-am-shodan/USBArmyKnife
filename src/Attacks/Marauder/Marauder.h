#pragma once

#include "../../USBArmyKnifeCapability.h"
#include <string>

class ESP32Marauder : USBArmyKnifeCapability {
public:
  ESP32Marauder();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);

  void run(const std::string& cmd);
  bool isRunning() { return running; }
private:
  bool running = false;
};

namespace Attacks
{
    extern ESP32Marauder Marauder;
}