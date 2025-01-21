#pragma once

#include <cstdint>
#include <string>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "../USBArmyKnifeCapability.h"

void registerSettingName(const uint8_t group, const std::string &name, const USBArmyKnifeCapability::SettingType &type, const std::string& defaultValue);
void enumerateSettingsAsJson(Preferences &prefs, JsonArray array);
bool setSettingValue(Preferences &prefs, const std::string &name, const std::string &value);
uint16_t getIntegerSettingValue(Preferences &prefs, const std::string &name, bool &error);