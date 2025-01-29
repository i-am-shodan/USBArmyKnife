#include "Marauder.h"

#define LOG_MARAUDER "ESP32M"
#include <string>
#include "../../Debug/Logging.h"
#include "../../Devices/Storage/HardwareStorage.h"

namespace Attacks
{
  ESP32Marauder Marauder;
}

ESP32Marauder::ESP32Marauder()
{
}

#ifndef NO_ESP_MARAUDER
#include <esp32_marauder/EvilPortal.h>
#include <esp32_marauder/CommandLine.h>
#include <esp32_marauder/SDInterface.h>
#include <esp32_marauder/WiFiScan.h>
#include <esp32_marauder/settings.h>
#include <esp32_marauder/Buffer.h>

size_t esp32m_println(const char* msg)
{
    const auto s = std::string(msg);
    Debug::Log.info("ESP32M", s);
    return s.length();
}

size_t esp32m_print(const String &s)
{
    return esp32m_println(s.c_str());
}

size_t esp32m_print(const char* msg)
{
    return esp32m_println(msg);
}

size_t esp32m_print(const char m)
{
    const auto strVal = std::to_string(m);
    return esp32m_println(strVal.c_str());
}

size_t esp32m_println(const String &s)
{
    return esp32m_println(s.c_str());
}

size_t esp32m_println(const size_t val)
{
    const auto strVal = std::to_string(val);
    return esp32m_println(strVal.c_str());
}

size_t esp32m_println()
{
  return 0;
}

size_t esp32m_println(esp_err_t&, int)
{
  return 0;
}

#ifndef NO_SD
  #include "../../Devices/Storage/ESP32/SDMMCFS2.h"
  using namespace fs;
  #define FILE_INTERFACE SD_MMC_2
#else
  #include <sd_defines.h>
#endif

WiFiScan wifi_scan_obj;
EvilPortal evil_portal_obj;
Buffer buffer_obj;
Settings settings_obj;
CommandLine cli_obj;
SDInterface sd_obj;

const String PROGMEM version_number = MARAUDER_VERSION;
static uint32_t currentTime = 0;
static std::string nextMarauderCommandToRun;
static bool marauderActivated = false;

void ESP32Marauder::begin(Preferences &prefs)
{
  buffer_obj = Buffer();
  (void)sd_obj.initSD();
  sd_obj.supported = true;
#ifndef NO_SD // todo this should prob be an API call
  sd_obj.cardType = SD_MMC_2.cardType();
  sd_obj.cardSizeMB = SD_MMC_2.cardSize() / 1024 / 1024;
#else
  sd_obj.cardType = CARD_SD;
  sd_obj.cardSizeMB = SPIFFS.totalBytes() / 1024 / 1024;
#endif
  sd_obj.card_sz = std::to_string(sd_obj.cardSizeMB).c_str();

  sd_obj.sd_files = new LinkedList<String>();
  sd_obj.sd_files->add("Back");

  running = 
#ifdef NO_SD
  settings_obj.begin();
#else
  settings_obj.begin(FILE_INTERFACE, "/esp32m_settings.json");
#endif

  if (!running)
  {
    Debug::Log.info(LOG_MARAUDER, "Error getting settings");
    return;
  }

  wifi_scan_obj.RunSetup();
  evil_portal_obj.setup();
  cli_obj.RunSetup();
}

void ESP32Marauder::loop(Preferences &prefs)
{
  // whilst we init the esp32 marauder code to save ram/cycles we don't
  // start to do work until we receive our first command
  if (marauderActivated)
  {
    currentTime = millis();
    wifi_scan_obj.main(currentTime);

    // Save buffer to SD and/or serial
    buffer_obj.save();
    settings_obj.main(currentTime);

    if (!nextMarauderCommandToRun.empty())
    {
      Debug::Log.info(LOG_MARAUDER, std::string("Running cmd ") + nextMarauderCommandToRun);
      cli_obj.runCommand(nextMarauderCommandToRun.c_str());
      nextMarauderCommandToRun.clear();
      Debug::Log.info(LOG_MARAUDER, std::string("Finished cmd ") + nextMarauderCommandToRun);
    }
  }
}

void ESP32Marauder::run(const std::string &cmd)
{
  marauderActivated = true;
  nextMarauderCommandToRun = cmd;
}

uint16_t ESP32Marauder::getPacketCount()
{
  return buffer_obj.getPacketsSinceLastCheck();
}

#else
void ESP32Marauder::begin(Preferences &prefs)
{

}

void ESP32Marauder::loop(Preferences &prefs)
{

}

void ESP32Marauder::run(const std::string &cmd)
{
  Debug::Log.info(LOG_MARAUDER, "ESP32Marauder is not supported on this platform");
}

uint16_t ESP32Marauder::getPacketCount()
{
  return 0;
}

#endif