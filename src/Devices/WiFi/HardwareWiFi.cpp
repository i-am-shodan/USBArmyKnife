#include "HardwareWiFi.h"
#include <WiFi.h>
#include <uptime.h>

#include "../../Debug/Logging.h"
#include "../../Attacks/Agent/Agent.h"
#include "../../Comms/Web/WebServer.h"

#ifndef WIFI_POWER
  #define WIFI_POWER WIFI_POWER_21dBm // default power level is MAX
#endif

#ifndef WIFI_SLEEP
  #define WIFI_SLEEP false // default power level is MAX
#endif

#define LOG_WIFI "WiFi"

#define WIFI_AP_MODE                    "wifi-ap-mode"
#define WIFI_AP_MODE_DEFAULT            true

#define WIFI_AP_NAME                    "wifi-ap"
#define WIFI_AP_NAME_DEFAULT            "iPhone14"

#define WIFI_AP_PWD                     "wifi-pwd"
#define WIFI_AP_PWD_DEFAULT             "password"

#define WIFI_DESIRED_BOOT_STATE         "wifi-bootstate"
#define WIFI_DESIRED_BOOT_STATE_DEFAULT true

static bool webServerRunning = false;
static bool desiredWiFiState = true; // we want WiFi up
static String wiFiApName = WIFI_AP_NAME_DEFAULT;
static String wiFiApPwd = WIFI_AP_PWD_DEFAULT;
static bool wifiAPMode = WIFI_AP_MODE_DEFAULT;

static unsigned long lastMinValueChecked = 0;

namespace Devices
{
    HardwareWiFi WiFi;
}

#ifndef NO_WIFI
HardwareWiFi::HardwareWiFi()
{
}

void HardwareWiFi::setWiFi(const bool& state)
{
  desiredWiFiState = state;
}

bool HardwareWiFi::getState()
{
  return (wifiAPMode == true && WiFi.AP.started()) || (wifiAPMode == false && WiFi.status() == WL_CONNECTED);
}

std::string HardwareWiFi::currentIPAddress()
{
  return std::string(WiFi.localIP().toString().c_str());
}

void HardwareWiFi::begin(Preferences& prefs)
{
  registerUserConfigurableSetting(CATEGORY_WIFI, WIFI_AP_MODE, USBArmyKnifeCapability::SettingType::Bool, WIFI_AP_MODE_DEFAULT);
  registerUserConfigurableSetting(CATEGORY_WIFI, WIFI_AP_NAME, USBArmyKnifeCapability::SettingType::String, WIFI_AP_NAME_DEFAULT);
  registerUserConfigurableSetting(CATEGORY_WIFI, WIFI_AP_PWD, USBArmyKnifeCapability::SettingType::String, WIFI_AP_PWD_DEFAULT);
  registerUserConfigurableSetting(CATEGORY_WIFI, WIFI_DESIRED_BOOT_STATE, USBArmyKnifeCapability::SettingType::Bool, WIFI_DESIRED_BOOT_STATE_DEFAULT);

  wifiAPMode = prefs.getBool(WIFI_AP_MODE, WIFI_AP_MODE_DEFAULT);
  wiFiApName = prefs.getString(WIFI_AP_NAME, WIFI_AP_NAME_DEFAULT);
  wiFiApPwd = prefs.getString(WIFI_AP_PWD, WIFI_AP_PWD_DEFAULT);
  desiredWiFiState = prefs.getBool(WIFI_DESIRED_BOOT_STATE, WIFI_DESIRED_BOOT_STATE_DEFAULT);

  Debug::Log.info(LOG_WIFI, (wifiAPMode ? std::string("Mode : AP, ") : std::string("Mode: Station, ")) + std::string("SSID = '")+wiFiApName.c_str()+ std::string("', WifiPwd = '")+wiFiApPwd.c_str() + "'");

  if (desiredWiFiState == true)
  {
    if (wifiAPMode)
    {
      WiFi.softAPConfig(IPAddress(4, 3, 2, 1), INADDR_NONE, IPAddress(255, 255, 255, 0));
      WiFi.softAP(wiFiApName, wiFiApPwd);
    }
    else
    {
      WiFi.begin(wiFiApName,wiFiApPwd);
    }
    WiFi.setTxPower(WIFI_POWER);
    WiFi.setSleep(WIFI_SLEEP);
  } 
}

void HardwareWiFi::loop(Preferences& prefs)
{
  if ((!webServerRunning && desiredWiFiState && wifiAPMode && WiFi.getMode() == WIFI_MODE_AP) || // set wiFiUp to true if running in AP mode OR
     (!webServerRunning && desiredWiFiState && !wifiAPMode && WiFi.status() == WL_CONNECTED)) // set wifi up if we've connected to the station
  {
    Debug::Log.info(LOG_WIFI, "Starting webserver");
    Comms::Web.begin(prefs);
    webServerRunning = true;
  }
  else if (WiFi.getMode() != WIFI_OFF && desiredWiFiState == false)
  {
    Debug::Log.info(LOG_WIFI, "Disabling WiFi");
    WiFi.mode(WIFI_OFF);
    Debug::Log.info(LOG_WIFI, "WiFi disabled");
    Comms::Web.end();
    Debug::Log.info(LOG_WIFI, "Webserver stopped");
    webServerRunning = false;
  }
  else if (WiFi.getMode() == WIFI_OFF && desiredWiFiState == true)
  {
    Debug::Log.info(LOG_WIFI, "Enabling WiFi");
    begin(prefs);
  }
  else if (WiFi.getMode() != WIFI_OFF)
  {
    Comms::Web.loop(prefs);
  }

  // If we are connected to something that can show these debug logs then emit them
  // Otherwise we don't want to spam the logs
  if (Attacks::Agent.isAgentConnected() && !wifiAPMode && WiFi.status() == WL_CONNECTED && uptime::getMinutes() != lastMinValueChecked) {
    Debug::Log.info(LOG_WIFI, std::string("Got external IP: ")+WiFi.localIP().toString().c_str());
    lastMinValueChecked = uptime::getMinutes();
  }
}

void HardwareWiFi::end()
{
  WiFi.mode(WIFI_OFF);
  Comms::Web.end();
  webServerRunning = false;
}
#else
HardwareWiFi::HardwareWiFi()
{
}

void HardwareWiFi::setWiFi(const bool& state)
{
}

void HardwareWiFi::begin(Preferences& prefs)
{
}

void HardwareWiFi::loop(Preferences& prefs)
{
}

void HardwareWiFi::end()
{
}

bool HardwareWiFi::getState()
{
  return false;
}

std::string HardwareWiFi::currentIPAddress()
{
  return std::string();
}

#endif