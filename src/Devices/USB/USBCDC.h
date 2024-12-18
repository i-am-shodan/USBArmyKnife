#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <functional>

#include "../../Devices/USB/USBCore.h"
#include "../../USBArmyKnifeCapability.h"

enum HostCommand : uint8_t
{
    MIN = 0,
    Execute = 1,
    DebugMsg = 2,
    WSCONNECT = 3,
    WSDATA = 4,
    WSDISCONNECT = 5,
    WSDATARECV = 6,
    RequestAgentStatus = 7,
    AgentStatus = 8,
    ExecuteResult = 9,
    MicPcmData = 10,
    MAX = 11
};

class USBCDCWrapper : USBArmyKnifeCapability {
public:
  USBCDCWrapper();

  void begin(const unsigned long& baud);
  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
  virtual void end();
  void setCallback(const HostCommand&, std::function<void(uint8_t*, const size_t)>);
  bool isConnectionEstablished();
  void writeBinary(const HostCommand&, const uint8_t *, const size_t&);
  void writeDebugString(const std::string &msg);
};

namespace Devices::USB
{
    extern USBCDCWrapper CDC;
}