#ifdef EXT_SENSOR_MOTION_LD2410
#include "LD2410.h"

#include <string>
#include <unordered_map>
#include <functional>
#include "MyLD2410.h"

#include "../../../../Attacks/Ducky/DuckyPayload.h"
#include "../../../../Debug/Logging.h"

#define LOG_LD2410 "LD2410"
#define DEFAULT_BAUD_RATE 256000
#define DEFAULT_BAUD_RATE_INDEX 7

HardwareSerial sensorSerial(2);
MyLD2410 sensor(sensorSerial);

static bool deviceDiscovered = false;

LD2410MotionSensorExtension::LD2410MotionSensorExtension()
{
}

void LD2410MotionSensorExtension::begin(Preferences &prefs)
{
  sensorSerial.begin(DEFAULT_BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
  delay(2000);
  if (sensor.begin())
  {
    if (sensor.setBaud(DEFAULT_BAUD_RATE_INDEX) && sensor.begin())
    {
      sensor.enhancedMode(true);
      Debug::Log.error(LOG_LD2410, "Started communication with LD2410");
      deviceDiscovered = true;
    }
    else
    {
      Debug::Log.error(LOG_LD2410, "Could not communicate with LD2410");
    }
  }

  Attacks::Ducky.registerExtension("LD2410_CONNECTED()", [this](const std::string& str, const std::unordered_map<std::string, std::string>& constants, const std::unordered_map<std::string, int>& variables)
  {
    return deviceDiscovered && millis() - sensor.getSensorData().timestamp <= 5000;
  });

  Attacks::Ducky.registerExtension("LD2410_GET_DISTANCE()", [this](const std::string& str, const std::unordered_map<std::string, std::string>& constants, const std::unordered_map<std::string, int>& variables)
  {
    const unsigned long distance = sensor.detectedDistance();
    if (distance >= INT_MAX)
    {
      return INT_MAX;
    }
    return (int)distance;
  });

  Attacks::Ducky.registerExtension("LD2410_GET_LIGHT_LEVEL()", [this](const std::string& str, const std::unordered_map<std::string, std::string>& constants, const std::unordered_map<std::string, int>& variables)
  {
    // DISPLAY_TEXT 0 35 Run #_FILE_INDEX_FILE_NAME_
    return sensor.getLightLevel();
  });

  Attacks::Ducky.registerExtension("LD2410_DETECTED_MOTION()", [this](const std::string& str, const std::unordered_map<std::string, std::string>& constants, const std::unordered_map<std::string, int>& variables)
  {
    return sensor.movingTargetDetected();
  });

  Attacks::Ducky.registerDynamicVariable([this]()
  {
    return std::pair("#_LD2410_LIGHT_LEVEL_", std::to_string(sensor.getLightLevel()));
  });
}

void LD2410MotionSensorExtension::loop(Preferences& prefs)
{
  if (deviceDiscovered)
  {
    sensor.check();
  }
}
#endif