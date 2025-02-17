#if ARDUINO_USB_MODE
#warning This sketch should be used when USB is in OTG mode
void setup() {}
void loop() {}
#else

#include <uptime.h>
#include <Adafruit_TinyUSB.h>

#include "Devices/Button/HardwareButton.h"
#include "Devices/LED/HardwareLED.h"
#include "Devices/TFT/HardwareTFT.h"
#include "Devices/Storage/HardwareStorage.h"
#include "Devices/USB/USBCore.h"
#include "Devices/WiFi/HardwareWiFi.h"
#include "Devices/Microphone/HardwareMicrophone.h"

#include "Comms/Web/WebServer.h"

#include "Attacks/Marauder/Marauder.h"
#include "Attacks/Ducky/DuckyPayload.h"
#include "Attacks/Agent/Agent.h"

#include "Debug/Logging.h"
#include "AuxiliaryComponent/Auxiliary.h"

#include "Utilities/Format.h"
#include "version.h"

#define TAG "main"

static Preferences prefs;
static Auxiliary aux;

void setup()
{
  prefs.begin("usbarmyknife");

  // First set up our core components / hw
  Debug::Log.begin(prefs);

  Devices::Storage.begin(prefs);
  // ESP32 Marauder uses a BT library that gets stuck in an infinite loop if it
  // fails to init. We init Marauder early as this means we should have as few tasks
  // as possible up in the air
  // If you plug in and don't see any LEDs, try commenting this line out
  Attacks::Marauder.begin(prefs);

  Devices::TFT.begin(prefs);
  Devices::LED.begin(prefs);
  Devices::Button.begin(prefs);
  Devices::Mic.begin(prefs);
  
  Devices::USB::Core.begin(prefs);
  Devices::WiFi.begin(prefs);

  Comms::Web.begin(prefs);

  Attacks::Ducky.begin(prefs);
  Attacks::Agent.begin(prefs);

  if (!Devices::Storage.isRunning())
  {
#ifndef NO_SD
    AskFormatSD(prefs);
#endif
  }
  else if (!Attacks::Marauder.isRunning())
  {
    // Most users won't see this error as devices without an SD won't have a screen either
    Devices::TFT.display(0, 0, "Error FlashFS invalid");
    for (int x = 0; x < 5; x++)
    {
      // They might see/report some debug output
      Devices::LED.changeLEDState(true, 0, 100, 100, x % 2 == 0 ? 255 : 0); // flash RED led
      Debug::Log.error(TAG, "Flash filesystem is invalid, upload new FS image");
      delay(1000);
    }
#ifdef ARDUINO_ARCH_ESP32
    // Other platforms don't implement ESP32 Marauder so we don't have to worry about a pi
    ESP.restart();
#endif
  }

  Devices::TFT.display(0, 0, "Device now running");
  Debug::Log.info(TAG, "Running!");

  if (Devices::USB::Core.currentDeviceType() == USBDeviceType::Serial)
  {
    Devices::TFT.display(0, 8, "USB MODE: Serial");
  }
  else if (Devices::USB::Core.currentDeviceType() == USBDeviceType::NCM)
  {
    Devices::TFT.display(0, 8, "USB MODE: NCM");
  }
  else
  {
    Devices::TFT.display(0, 8, "USB MODE: Disabled");
  }

  if (Devices::USB::Core.currentClassType() == USBClassType::HID)
  {
    Devices::TFT.display(0, 8+8, "USB CLASS: HID");
  }
  else if (Devices::USB::Core.currentClassType() == USBClassType::Storage)
  {
    Devices::TFT.display(0, 8+8, "USB CLASS: Storage");
  }
  else
  {
    Devices::TFT.display(0, 8+8, "USB CLASS: None");
  }

  auto versionStr = std::string("Version: ")+GIT_COMMIT_HASH;
  Devices::TFT.display(0, 8+8+8, versionStr);
  Debug::Log.info(TAG, versionStr);

  aux.begin(prefs);
}

void loop()
{
  uptime::calculateUptime();

  Debug::Log.loop(prefs);

  Devices::USB::Core.loop(prefs);
  Devices::WiFi.loop(prefs);
  Devices::Storage.loop(prefs);
  Devices::Button.loop(prefs);
  Devices::LED.loop(prefs);
  Devices::TFT.loop(prefs);
  Devices::Mic.loop(prefs);

  Comms::Web.loop(prefs);

  Attacks::Ducky.loop(prefs);
  Attacks::Marauder.loop(prefs);
  Attacks::Agent.loop(prefs);

  aux.loop(prefs);
}

#endif /* ARDUINO_USB_MODE */