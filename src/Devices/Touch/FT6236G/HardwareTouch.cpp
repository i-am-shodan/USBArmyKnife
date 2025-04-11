#ifndef NO_TOUCH
#include "../HardwareTouch.h"
#include "../../../Debug/Logging.h"

#include <Wire.h>
#include <Adafruit_FT6206.h>

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();

#define TAG "Touch"

static bool started = false;

namespace Devices
{
  HardwareTouch Touch;
}

HardwareTouch::HardwareTouch()
{
}

void HardwareTouch::begin(Preferences &prefs)
{
  do
  {
    if (!Wire1.begin(BOARD_TOUCH_SDA, BOARD_TOUCH_SCL))
    {
      Debug::Log.error(TAG, "Error setting up i2c");
      break;
    }

    // pass in 'sensitivity' coefficient and I2C bus
    if (!ctp.begin(40, &Wire1)) {  
      Debug::Log.error(TAG, "Couldn't start FT6206 touchscreen controller");
      break;
    }

    started = true;
  } while (false);
}

void HardwareTouch::loop(Preferences& prefs)
{
  if (!started)
  {
    return;
  }

  if (!ctp.touched()) {
    return;
  }

  // Retrieve a point  
  TS_Point p = ctp.getPoint();
  xPos = p.x;
  yPos = p.y;
  wasTouched = true;
}

#endif