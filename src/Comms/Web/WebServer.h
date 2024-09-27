#pragma once

#include "../../USBArmyKnifeCapability.h"

class WebSite : USBArmyKnifeCapability {
public:
  WebSite();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
};

namespace Comms
{
    extern WebSite Web;
}