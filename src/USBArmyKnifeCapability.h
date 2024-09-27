#pragma once

#include <Preferences.h>

#define CATEGORY_USB 1
#define CATEGORY_WIFI 2

class USBArmyKnifeCapability
{

public:
  USBArmyKnifeCapability(void) {}

  virtual void begin(Preferences &prefs) = 0;
  virtual void loop(Preferences &prefs) = 0;
  virtual void end()
  {
  }

  enum class SettingType : uint8_t
  {
    String = 0,
    Bool = 1,
    Int16 = 2,
    UInt16 = 3
  };

protected:
  void registerUserConfigurableSetting(const uint8_t group, const char *name, const USBArmyKnifeCapability::SettingType type, const char* defaultValue);
  void registerUserConfigurableSetting(const uint8_t group, const char *name, const USBArmyKnifeCapability::SettingType type, const int16_t defaultValue);
  void registerUserConfigurableSetting(const uint8_t group, const char *name, const USBArmyKnifeCapability::SettingType type, const uint16_t defaultValue);
  void registerUserConfigurableSetting(const uint8_t group, const char *name, const USBArmyKnifeCapability::SettingType type, const bool defaultValue);
};