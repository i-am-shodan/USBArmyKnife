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

#include "Utilities/Format.h"

#include "MyLD2410.h"
// #define sensorSerial Serial1
HardwareSerial sensorSerial(2);
MyLD2410 sensor(sensorSerial);
static bool working = false;

static Preferences prefs;

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

  Devices::LED.begin(prefs);
  Devices::TFT.begin(prefs);
  Devices::Button.begin(prefs);

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
    Devices::TFT.display(0, 8 + 8, "USB CLASS: HID");
  }
  else if (Devices::USB::Core.currentClassType() == USBClassType::Storage)
  {
    Devices::TFT.display(0, 8 + 8, "USB CLASS: Storage");
  }
  else
  {
    Devices::TFT.display(0, 8 + 8, "USB CLASS: None");
  }

/*
  sensorSerial.begin(256000, SERIAL_8N1, 44, 43);
  if (sensor.begin())
  {
    sensor.setBaud(7);
    sensorSerial.end();
    sensorSerial.begin(256000, SERIAL_8N1, 44, 43);
    if (sensor.begin())
    {
      Devices::TFT.display(0, 8+8+8, "Working");
      working = true;
    }
  }*/

  
    uint32_t BAUDS[9]{ 0, 9600, 19200, 38400, 57600, 115200, 230400, 256000, 460800 };
    for (int x = 1; x < sizeof(BAUDS); x++)
    {
      sensorSerial.begin(BAUDS[x], SERIAL_8N1, 44, 43);
      delay(2000);

      if (!sensor.begin()) {
        Devices::TFT.clearScreen();
        Debug::Log.info("Main", "Failed to communicate with sensor");
        Devices::TFT.display(0, 0, "Sensor error: "+std::to_string(BAUDS[x]));
      }
      else
      {
        Devices::TFT.clearScreen();
        Devices::TFT.display(0, 0, "Working with "+std::to_string(BAUDS[x]));

        if (!sensor.setBaud(x))
        {
          Devices::TFT.display(0, 8, "Set rate error");
          break;
        }

        sensorSerial.end();
        sensorSerial.begin(BAUDS[x], SERIAL_8N1, 44, 43);
        if (sensor.begin())
        {
          working = true;
        }
        break;
      }
    }
    Devices::TFT.display(0, 8+8, "Finished: "+std::to_string(working));
}

static unsigned long previousMillis = 0;

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

  if (working && sensor.check() == MyLD2410::Response::DATA && millis() - previousMillis >= 1000)
  {
    unsigned long distance = sensor.detectedDistance();
    Devices::TFT.clearScreen();
    Devices::TFT.display(0, 0, sensor.statusString());
    Devices::TFT.display(0, 8, "Distance: " + std::to_string(distance));
    Devices::TFT.display(0, 8 + 8, "Light level: " + std::to_string(sensor.getLightLevel()));

    previousMillis = millis();
  }
}

#endif /* ARDUINO_USB_MODE */