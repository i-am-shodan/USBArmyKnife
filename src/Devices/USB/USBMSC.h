#pragma once

#include <string.h>

#include "../../USBArmyKnifeCapability.h"
#include "USBCore.h"

class USBMSC : USBArmyKnifeCapability {
public:
  USBMSC();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
  virtual void end();

  bool mountDiskImage(const std::string&);
  bool mountSD();
  bool hasActivity() { return mscActivity; }
  void resetActivityState() { mscActivity = false; }
  void setActivityStateState(bool state) { mscActivity = state; }
private:
  bool mscActivity = false;
};

namespace Devices::USB
{
    extern USBMSC MSC;
}