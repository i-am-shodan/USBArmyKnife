#pragma once

#include <string>

#include "../../USBArmyKnifeCapability.h"

class SerialAgent : USBArmyKnifeCapability {
public:
  SerialAgent();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);

  void run(const std::string& cmd);
  bool isAgentConnected() { return m_agentIsConnected; }
  std::string machineName() { return m_machineName; }

private:
  bool m_agentIsConnected = false;
  std::string m_machineName;
};

namespace Attacks
{
    extern SerialAgent Agent;
}