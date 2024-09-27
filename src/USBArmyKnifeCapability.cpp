#include "USBArmyKnifeCapability.h"
#include "Utilities/Settings.h"

#include <string>

void USBArmyKnifeCapability::registerUserConfigurableSetting(const uint8_t group, const char *name, const USBArmyKnifeCapability::SettingType type, const char *defaultValue)
{
    registerSettingName(group, name, type, defaultValue);
}

void USBArmyKnifeCapability::registerUserConfigurableSetting(const uint8_t group, const char *name, const USBArmyKnifeCapability::SettingType type, const int16_t defaultValue)
{
    registerSettingName(group, name, type, std::to_string(defaultValue));
}

void USBArmyKnifeCapability::registerUserConfigurableSetting(const uint8_t group, const char *name, const USBArmyKnifeCapability::SettingType type, const uint16_t defaultValue)
{
    registerSettingName(group, name, type, std::to_string(defaultValue));
}

void USBArmyKnifeCapability::registerUserConfigurableSetting(const uint8_t group, const char *name, const USBArmyKnifeCapability::SettingType type, const bool defaultValue)
{
    registerSettingName(group, name, type, std::to_string(defaultValue));
}