#ifndef NO_LED
#ifndef LED_CI_PIN
#include "../HardwareLED.h"
#include <FastLED.h>

namespace Devices
{
    HardwareLED LED;
}

static CRGB leds[NUM_LEDS];

void HardwareLED::changeLEDState(bool on, uint8_t hue, uint8_t saturation, uint8_t lum, uint8_t brightness)
{
  if (on == true)
  {
    // For some reason I need to do colour correction here. I wonder if the CRGB is the wrong byte order?
    if (hue == 0 && saturation == 100 && lum == 100) // red
    {
      hue = 100;
    }
    else if (hue == 100 && saturation == 100 && lum == 100) // green
    {
      hue = 0;
    }
    else if (hue == 240 && saturation == 100 && lum == 100) // blue
    {
      hue = 160;
    }

    saturation = map(saturation, 0, 100, 30, 255);
    lum = map(lum, 0, 100, 100, 255);
        
    leds[0] = CHSV(hue, saturation, lum);
    
    FastLED.setBrightness(brightness);
  }
  else
  {
	  leds[0] = CRGB::Black;
    FastLED.setBrightness(0);
  }
  FastLED.show();
}

HardwareLED::HardwareLED()
{
}

void HardwareLED::loop(Preferences& prefs)
{
}

void HardwareLED::begin(Preferences &prefs)
{
  FastLED.addLeds<WS2812, LED_DI_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(0);
  FastLED.show();
}
#endif
#endif