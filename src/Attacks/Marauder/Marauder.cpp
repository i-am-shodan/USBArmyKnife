#include <esp32_marauder/EvilPortal.h>
#include <esp32_marauder/CommandLine.h>
#include <esp32_marauder/SDInterface.h>
#include <esp32_marauder/WiFiScan.h>
#include <esp32_marauder/settings.h>
#include <esp32_marauder/Buffer.h>

#include "../../Devices/Storage/SDMMCFS2.h"
using namespace fs;

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
static uint32_t currentTime  = 0;

static std::string nextMarauderCommandToRun;

namespace Attacks
{
    ESP32Marauder Marauder;
}

ESP32Marauder::ESP32Marauder()
{

}

void ESP32Marauder::begin(Preferences& prefs)
{
  led_obj.RunSetup();
  buffer_obj = Buffer();
  sd_obj.initSD();
  settings_obj.begin();
  wifi_scan_obj.RunSetup();
  buffer_obj = Buffer();
  evil_portal_obj.setup();
  cli_obj.RunSetup();

  sd_obj.supported = true;
  sd_obj.cardType = SD_MMC_2.cardType();
  sd_obj.cardSizeMB = SD_MMC_2.cardSize();
  sd_obj.card_sz = std::to_string(SD_MMC_2.cardSize() / 1024 / 1024).c_str();
  sd_obj.sd_files = new LinkedList<String>();
  sd_obj.sd_files->add("Back");
}

void ESP32Marauder::loop(Preferences& prefs)
{
    currentTime = millis();
    wifi_scan_obj.main(currentTime);

    // Save buffer to SD and/or serial
    buffer_obj.save();
    settings_obj.main(currentTime);

    if (!nextMarauderCommandToRun.empty())
    {
      Debug::Log.info(LOG_MARAUDER, std::string("Running cmd ")+nextMarauderCommandToRun);
      cli_obj.runCommand(nextMarauderCommandToRun.c_str());
      nextMarauderCommandToRun.clear();
      Debug::Log.info(LOG_MARAUDER, std::string("Finished cmd ")+nextMarauderCommandToRun);
    }
}

void ESP32Marauder::run(const std::string& cmd)
{
  nextMarauderCommandToRun = cmd;
}