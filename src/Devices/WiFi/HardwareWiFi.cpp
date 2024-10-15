#include "HardwareWiFi.h"
#include <WiFi.h>

#include "../../Debug/Logging.h"
#define LOG_WIFI "WiFi"

#define WIFI_AP_NAME                    "wifi-ap"
#define WIFI_AP_NAME_DEFAULT            "iPhone14"

#define WIFI_AP_PWD                     "wifi-pwd"
#define WIFI_AP_PWD_DEFAULT             "password"

#define WIFI_DESIRED_BOOT_STATE         "wifi-bootstate"
#define WIFI_DESIRED_BOOT_STATE_DEFAULT true

static bool wiFiUp = false;
static bool desiredWiFiState = true; // we want WiFi up
static String wiFiApName;
static String wiFiApPwd;

namespace Devices
{
    HardwareWiFi WiFi;
}

HardwareWiFi::HardwareWiFi()
{
  registerUserConfigurableSetting(CATEGORY_WIFI, WIFI_AP_NAME, USBArmyKnifeCapability::SettingType::String, WIFI_AP_NAME_DEFAULT);
  registerUserConfigurableSetting(CATEGORY_WIFI, WIFI_AP_PWD, USBArmyKnifeCapability::SettingType::String, WIFI_AP_PWD_DEFAULT);
  registerUserConfigurableSetting(CATEGORY_WIFI, WIFI_DESIRED_BOOT_STATE, USBArmyKnifeCapability::SettingType::Bool, WIFI_DESIRED_BOOT_STATE_DEFAULT);
}

void HardwareWiFi::setWiFi(const bool& state)
{
  desiredWiFiState = state;
}

void HardwareWiFi::begin(Preferences& prefs)
{
  wiFiApName = prefs.getString(WIFI_AP_NAME, WIFI_AP_NAME_DEFAULT);
  wiFiApPwd = prefs.getString(WIFI_AP_PWD, WIFI_AP_PWD_DEFAULT);
  desiredWiFiState = prefs.getBool(WIFI_DESIRED_BOOT_STATE, WIFI_DESIRED_BOOT_STATE_DEFAULT);

  Debug::Log.info(LOG_WIFI, std::string("WifiPwd = '")+wiFiApPwd.c_str() + "'");

  if (wiFiUp == false && desiredWiFiState == true)
  {
    WiFi.softAPConfig(IPAddress(4, 3, 2, 1), INADDR_NONE, IPAddress(255, 255, 255, 0));
    WiFi.softAP(wiFiApName, wiFiApPwd);
    #if defined(LOLIN_WIFI_FIX) && (defined(ARDUINO_ARCH_ESP32C3) || defined(ARDUINO_ARCH_ESP32S2) || defined(ARDUINO_ARCH_ESP32S3))
      WiFi.setTxPower(WIFI_POWER_8_5dBm);
    #endif
    WiFi.setSleep(false);
    wiFiUp = true;
  } 
}

void HardwareWiFi::loop(Preferences& prefs)
{
  if (wiFiUp && desiredWiFiState == false)
  {
    WiFi.mode(WIFI_OFF);
  }
  else if (wiFiUp == false && desiredWiFiState == true)
  {
    begin(prefs);
  }
}

void HardwareWiFi::end()
{
  WiFi.mode(WIFI_OFF);
}