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

#include "Comms/Web/WebServer.h"

#include "Attacks/Marauder/Marauder.h"
#include "Attacks/Ducky/DuckyPayload.h"
#include "Attacks/Agent/Agent.h"

#include "Debug/Logging.h"

static Preferences prefs;

void setup()
{
  prefs.begin("usbarmyknife");

  // First set up our core components / hw
  Debug::Log.begin(prefs);

  Devices::Storage.begin(prefs);
  if (!Devices::Storage.isRunning())
  {
    delay(60 * 1000);
    ESP.restart();
  }

  // ESP32 Marauder uses a BT library that gets stuck in an infinite loop if it
  // fails to init. We init Marauder early as this means we should have as few tasks
  // as possible up in the air
  // If you plug in and don't see any LEDs, try commenting this line out
  Attacks::Marauder.begin(prefs);

  Devices::LED.begin(prefs);
  Devices::TFT.begin(prefs);
  Devices::Button.begin(prefs);
  
  Devices::USB::Core.begin(prefs);
  Devices::WiFi.begin(prefs);

  Comms::Web.begin(prefs);

  Attacks::Ducky.begin(prefs);
  Attacks::Agent.begin(prefs);

  Devices::TFT.display(0, 0, "Device now running");
  Debug::Log.info("Main", "Running!");

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

  Comms::Web.loop(prefs);

  Attacks::Ducky.loop(prefs);
  Attacks::Marauder.loop(prefs);
  Attacks::Agent.loop(prefs);
}

#endif /* ARDUINO_USB_MODE */