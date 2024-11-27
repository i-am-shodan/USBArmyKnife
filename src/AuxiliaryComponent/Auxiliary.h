#pragma once

#include "../USBArmyKnifeCapability.h"

class Auxiliary : USBArmyKnifeCapability {
public:
  Auxiliary();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
};