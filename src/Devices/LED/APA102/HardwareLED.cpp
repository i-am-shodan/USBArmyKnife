#ifndef NO_LED
#ifdef LED_CI_PIN
#include "../HardwareLED.h"
#include <APA102.h>

namespace Devices
{
    HardwareLED LED;
}

static APA102<LED_DI_PIN, LED_CI_PIN> ledStrip;

// Set the number of LEDs to control.
const uint16_t ledCount = 1;

// Create a buffer for holding the colors (3 bytes per color).
static rgb_color colors[ledCount];

const uint8_t brightness = 10;

static rgb_color hsvToRgb(uint16_t h, uint8_t s, uint8_t v)
{
    uint8_t f = (h % 60) * 255 / 60;
    uint8_t p = (255 - s) * (uint16_t)v / 255;
    uint8_t q = (255 - f * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t t = (255 - (255 - f) * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t r = 0, g = 0, b = 0;
    switch((h / 60) % 6){
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    return rgb_color(r, g, b);
}

void HardwareLED::changeLEDState(bool on, uint8_t hue, uint8_t saturation, uint8_t lum, uint8_t brightness)
{
  if (!on)
  {
    ledStrip.write(colors, 1, 0);
  }
  else
  {
    // our brightness is between 0 - 255, need to rescale between 0-31
    brightness = (uint8_t) ((float) brightness * (31.f / 255.f));
    colors[0] = hsvToRgb(hue, saturation, lum);
    ledStrip.write(colors, 1, brightness);
  }
}

HardwareLED::HardwareLED()
{
}

void HardwareLED::loop(Preferences& prefs)
{
}

void HardwareLED::begin(Preferences &prefs)
{
  changeLEDState(true, 100, 100, 100, 200);
}
#endif
#endif