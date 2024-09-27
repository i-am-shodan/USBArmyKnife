#pragma once

#include "string"

#include "../../USBArmyKnifeCapability.h"
#include "../Storage/HardwareStorage.h"

class HardwareTFT : USBArmyKnifeCapability {
public:
  HardwareTFT();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
  void displayPng(HardwareStorage& storage, const std::string& filename);
  void display(const int &x, const int &y, const std::string &str);
  void clearScreen();
};

namespace Devices
{
    extern HardwareTFT TFT;
};