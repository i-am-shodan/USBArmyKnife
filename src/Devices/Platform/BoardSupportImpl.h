#pragma once

#include "../../USBArmyKnifeCapability.h"

class BoardSupport : public USBArmyKnifeCapability {
public:
  BoardSupport();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
  bool hasCrashed() { return false; }
};