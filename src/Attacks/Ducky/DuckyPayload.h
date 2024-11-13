#pragma once

#include <string>
#include <functional>

#include "../../USBArmyKnifeCapability.h"

#define AUTORUN_FILENAME "/autorun.ds"
#define LOG_DUCKY "DUCKY"

class DuckyPayload : USBArmyKnifeCapability {
public:
  DuckyPayload();

  virtual void begin(Preferences& prefs);
  virtual void loop(Preferences& prefs);

  void setPayloadCmdLine(const std::string& cmdLine);
  void setPayload(const std::string& path);
  std::string getPayloadRunningStatus();
  uint8_t getTotalErrors();
  void registerExtension(const std::string& command, std::function<int(const std::string&, const std::unordered_map<std::string, std::string>&, const std::unordered_map<std::string, int>&)> callback);
};

namespace Attacks
{
    extern DuckyPayload Ducky;
}