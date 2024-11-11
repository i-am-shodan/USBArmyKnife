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
  bool hasAgentCmdResult() { return agentCmdResult; }
  void resetAgentCmdResultState() { agentCmdResult = false; }

private:
  bool m_agentIsConnected = false;
  bool agentCmdResult = false;
  std::string m_machineName;
};

namespace Attacks
{
    extern SerialAgent Agent;
}