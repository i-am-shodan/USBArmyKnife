#include "Settings.h"
#include "../USBArmyKnifeCapability.h"
#include "../Debug/Logging.h"

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <ArduinoJson.h>

#define LOG_SETTINGS "Settings"

static std::unordered_map<uint8_t, std::vector<std::tuple<std::string, USBArmyKnifeCapability::SettingType, std::string>>> settingLookup;

void registerSettingName(const uint8_t group, const std::string &name, const USBArmyKnifeCapability::SettingType &type, const std::string &defaultValue)
{
    if (settingLookup.find(group) == settingLookup.cend())
    {
        settingLookup[group] = std::vector<std::tuple<std::string, USBArmyKnifeCapability::SettingType, std::string>>();
    }

    // ensure we can't register the same name in the same group twice
    const bool notFound = std::none_of(settingLookup[group].cbegin(), settingLookup[group].cend(),
                                       [&name](const std::tuple<std::string, USBArmyKnifeCapability::SettingType, std::string> &item)
                                       {
                                           return std::get<0>(item) == name;
                                       });

    if (notFound)
    {
        settingLookup[group].emplace_back(name, type, defaultValue);
    }
}

bool setSettingValue(Preferences &prefs, const std::string &name, const std::string &value)
{
    bool ret = false;

    int tempInt = 0;

    for (const auto &settingCategory : settingLookup)
    {
        for (const auto &setting : settingCategory.second)
        {
            if (name == get<0>(setting))
            {
                const USBArmyKnifeCapability::SettingType &settingType = get<1>(setting);

                switch (settingType)
                {
                case USBArmyKnifeCapability::SettingType::String:
                    prefs.putString(name.c_str(), value.c_str());
                    ret = true;
                    break;
                case USBArmyKnifeCapability::SettingType::Bool:
                    if (value.length() == 1)
                    {
                        prefs.putBool(name.c_str(), value == "1");
                        ret = true;
                    }
                    else
                    {
                        Debug::Log.info(LOG_SETTINGS, "Invalid setting value, bool is too large");
                    }
                    break;
                case USBArmyKnifeCapability::SettingType::Int16:
                    tempInt = std::stoi(value);
                    if (tempInt <= static_cast<int>(INT16_MAX) && tempInt >= static_cast<int>(INT16_MIN))
                    {
                        int16_t int16Value = 0;
                        int16Value = static_cast<int16_t>(tempInt);
                        prefs.putShort(name.c_str(), int16Value);
                        ret = true;
                    }
                    else
                    {
                        Debug::Log.info(LOG_SETTINGS, "Invalid setting value");
                    }
                    break;
                case USBArmyKnifeCapability::SettingType::UInt16:
                    if (value.rfind("0x", 0) == 0)
                    {
                        tempInt = std::stoi(value, nullptr, 16);
                        if (tempInt <= static_cast<int>(INT16_MAX) && tempInt >= 0)
                        {
                            uint16_t uint16Value = 0;
                            uint16Value = static_cast<uint16_t>(tempInt);
                            prefs.putUShort(name.c_str(), uint16Value);
                            ret = true;
                        }
                        else
                        {
                            Debug::Log.info(LOG_SETTINGS, "Invalid setting value, invalid value");
                        }
                    }
                    else
                    {
                        Debug::Log.info(LOG_SETTINGS, "Invalid setting value, must start with 0x");
                    }
                    break;
                default:
                    Debug::Log.info(LOG_SETTINGS, "Unknown setting type");
                    break;
                };
            }
        }
    }

    return ret;
}

uint16_t getIntegerSettingValue(Preferences &prefs, const std::string &name, bool &error)
{
    error = false;

    for (const auto &settingCategory : settingLookup)
    {
        const std::vector<std::tuple<std::string, USBArmyKnifeCapability::SettingType, std::string>> &settingInCategory = settingCategory.second;
        for (const auto &setting : settingInCategory)
        {
            const std::string &settingName = get<0>(setting);
            const USBArmyKnifeCapability::SettingType &settingType = get<1>(setting);

            if (settingName != name)
            {
                continue;
            }
            else if (settingType == USBArmyKnifeCapability::SettingType::String)
            {
                goto error;
            }
            else if (!prefs.isKey(settingName.c_str()))
            {
                const std::string &defaultValue = get<2>(setting);
                return (uint16_t)atoi(defaultValue.c_str());
            }
            else
            {
                switch (settingType)
                {
                case USBArmyKnifeCapability::SettingType::Bool:
                    return prefs.getBool(settingName.c_str());
                    break;
                case USBArmyKnifeCapability::SettingType::Int16:
                    return prefs.getShort(settingName.c_str());
                    break;
                case USBArmyKnifeCapability::SettingType::UInt16:
                    return prefs.getUShort(settingName.c_str());
                default: // string and other types
                    break;
                };
            }
        }
    }

error:
    error = true;
    return 0;
}

void enumerateSettingsAsJson(Preferences &prefs, JsonArray array)
{
    for (const auto &settingCategory : settingLookup)
    {
        uint8_t category = settingCategory.first;
        const std::vector<std::tuple<std::string, USBArmyKnifeCapability::SettingType, std::string>> &settingInCategory = settingCategory.second;

        const JsonObject categoryObj = array.add<JsonObject>();

        switch (category)
        {
        case CATEGORY_USB:
            categoryObj["name"] = "USB";
            break;
        case CATEGORY_WIFI:
            categoryObj["name"] = "WIFI";
            break;
        default:
            categoryObj["name"] = "UNKNOWN";
            break;
        }

        const auto &settingsArray = categoryObj["settings"].to<JsonArray>();

        for (const auto &setting : settingInCategory)
        {
            const std::string &settingName = get<0>(setting);
            const USBArmyKnifeCapability::SettingType &settingType = get<1>(setting);
            const std::string &defaultValue = get<2>(setting);

            const auto &settingsJObject = settingsArray.add<JsonObject>();

            settingsJObject["name"] = settingName;
            settingsJObject["type"] = (uint8_t)settingType;
            settingsJObject["default"] = defaultValue;

            if (!prefs.isKey(settingName.c_str()))
            {
                settingsJObject["state"] = "unset";
                settingsJObject["value"] = defaultValue;
            }
            else
            {
                settingsJObject["state"] = "set";

                switch (settingType)
                {
                case USBArmyKnifeCapability::SettingType::String:
                    settingsJObject["value"] = prefs.getString(settingName.c_str());
                    break;
                case USBArmyKnifeCapability::SettingType::Bool:
                    settingsJObject["value"] = prefs.getBool(settingName.c_str());
                    break;
                case USBArmyKnifeCapability::SettingType::Int16:
                    settingsJObject["value"] = prefs.getShort(settingName.c_str());
                    break;
                case USBArmyKnifeCapability::SettingType::UInt16:
                    settingsJObject["value"] = prefs.getUShort(settingName.c_str());
                default:
                    break;
                };
            }
        }
    }
}