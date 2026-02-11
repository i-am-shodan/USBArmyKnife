#pragma once
#include "../ESP32BoardSupport.h"

class EvilCrowWind : public ESP32BoardSupport {
public:
  EvilCrowWind();

  void loop(Preferences& prefs) override;
  void begin(Preferences& prefs) override;
  bool hasCrashed() { return ESP32BoardSupport::hasCrashed(); }
};

namespace Devices
{
  extern EvilCrowWind Board;
}