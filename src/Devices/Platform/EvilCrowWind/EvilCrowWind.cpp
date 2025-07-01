#ifdef EVIL_CROW_CABLE_WIND

#include <uptime.h>
#include "driver/temperature_sensor.h"

#include "../BoardSupport.h"
#include "../../../Debug/Logging.h"
#include "../../../Attacks/Marauder/Marauder.h"
#include "../../../Devices/WiFi/HardwareWiFi.h"
#define TAG "Board"

static temperature_sensor_handle_t temp_sensor = nullptr;
static unsigned long lastMinuteWeChecked = 0;
static bool overheatingProtectionEnabled = true;

BoardSupport::BoardSupport()
{
}

void BoardSupport::begin(Preferences &prefs)
{
    // https://github.com/i-am-shodan/USBArmyKnife/issues/168
    // the operating temperature of the ESP32-S3-PICO-1-N8R2 which most cables are build around is –40 ~ 85 C
    // we do want to limit internal temperature sensor to a more reasonable range as some were built using ESP32-S3-PICO-1-N8R8
    // which can only handle 65 C

    // The ESP32 can measure -10 ~ 80 with good accuracy
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 80);
    if (temperature_sensor_install(&temp_sensor_config, &temp_sensor) != ESP_OK)
    {
        Debug::Log.error(TAG, "Failed to install temperature sensor");
        return;
    }

    if (temperature_sensor_enable(temp_sensor) != ESP_OK)
    {
        Debug::Log.error(TAG, "Failed to enable temperature sensor");
        return;
    }
}

void BoardSupport::loop(Preferences &prefs)
{
    if (lastMinuteWeChecked == uptime::getMinutes())
    {
        return; // we already checked this minute
    }
    
    float tsens_value;
    if (temperature_sensor_get_celsius(temp_sensor, &tsens_value) == ESP_OK)
    {
        Debug::Log.info(TAG, "Temperature sensor value: " + std::to_string(tsens_value) + " C");
        if (tsens_value > 60.0f && !overheatingProtectionEnabled)
        {
            // Turn off Marauder and wifi to try and reduce temperature
            Attacks::Marauder.run("stopscan");
            Devices::WiFi.setWiFi(false);
            overheatingProtectionEnabled = true;
        }
        else if (tsens_value <= 40.0f && overheatingProtectionEnabled)
        {
            // Turn on wifi again
            Devices::WiFi.setWiFi(true);
            overheatingProtectionEnabled = false;
        }
    }
    else
    {
        Debug::Log.error(TAG, "Failed to read temperature sensor");
    }
    
    lastMinuteWeChecked = uptime::getMinutes();
}

#endif