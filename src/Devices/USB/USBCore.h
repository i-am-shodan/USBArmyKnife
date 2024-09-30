#pragma once

#include <DuckyParse.h>
#include <cstdint>

#include "../../USBArmyKnifeCapability.h"

enum class USBDeviceType : uint16_t
{
  None = 0,
  Serial = 1,
  NCM = 2
};

enum class USBClassType : uint16_t
{
  None = 0,
  HID = 1,
  Storage = 2
};

class USBCore : USBArmyKnifeCapability
{
public:
  USBCore();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences &prefs);
  virtual void end();

  USBDeviceType currentDeviceType() { return curDeviceType; } // what device we are currently configured as
  USBClassType currentClassType() { return curClassType; };   // what device class is currently enabled
  uint16_t getVID() { return vid; }
  uint16_t getPID() { return pid; }
  const std::string getDeviceManufacturer() { return manufacturer; }
  const std::string getProduct() { return product; }

  void changeUSBMode(DuckyInterpreter::USB_MODE &mode, const uint16_t &vid, const uint16_t &pid, const std::string &man, const std::string &prod, const std::string &serial);
  void reset();
  
  std::string getCurrentUSBMode()
  {
    if (curDeviceType == USBDeviceType::None)
    {
      return "None";
    }
    else
    {
      std::string ret = curDeviceType == USBDeviceType::Serial ? "Serial" : "NCM";

      if (curClassType != USBClassType::None)
      {
        ret += curClassType == USBClassType::HID ? " + HID" : " + Storage";
      }
      return ret;
    }
  }
private:
  USBDeviceType curDeviceType;
  USBClassType curClassType;
  uint16_t vid;
  uint16_t pid;
  std::string manufacturer;
  std::string product;
};

namespace Devices::USB
{
  extern USBCore Core;
}