#pragma once
#include "BoardSupportImpl.h"

class ESP32BoardSupport : public BoardSupport {
public:
  ESP32BoardSupport();

  virtual void loop(Preferences& prefs) override;
  virtual void begin(Preferences& prefs) override;
  bool hasCrashed();
};