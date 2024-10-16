#include <esp32_marauder/EvilPortal.h>
#include <esp32_marauder/CommandLine.h>
#include <esp32_marauder/SDInterface.h>
#include <esp32_marauder/WiFiScan.h>
#include <esp32_marauder/settings.h>
#include <esp32_marauder/Buffer.h>

#ifndef NO_SD
  #include "../../Devices/Storage/SDMMCFS2.h"
  using namespace fs;
#else
  #include <sd_defines.h>
#endif

#include "Marauder.h"

#define LOG_MARAUDER "ESP32M"

#include "../../Debug/Logging.h"

WiFiScan wifi_scan_obj;
EvilPortal evil_portal_obj;
Buffer buffer_obj;
Settings settings_obj;
CommandLine cli_obj;
SDInterface sd_obj;
LedInterface led_obj;

const String PROGMEM version_number = MARAUDER_VERSION;
static uint32_t currentTime = 0;

static std::string nextMarauderCommandToRun;
static bool marauderActivated = false;

namespace Attacks
{
  ESP32Marauder Marauder;
}

ESP32Marauder::ESP32Marauder()
{
}

void ESP32Marauder::begin(Preferences &prefs)
{
  led_obj.RunSetup();
  buffer_obj = Buffer();
  sd_obj.initSD();
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

  settings_obj.begin();
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