#pragma once

#include "../../USBArmyKnifeCapability.h"

class BoardSupport : USBArmyKnifeCapability {
public:
  BoardSupport();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);
};

#ifdef LILYGO_T_DONGLE_S3
  #define DEVICE_MAKE_MODEL "LILYGO_T_DONGLE_S3"
#elif defined(WAVESHARE_ESP32_S3_LCD_147)
  #define DEVICE_MAKE_MODEL "WAVESHARE_ESP32_S3_LCD_147"
#elif defined(M5_ATOM_S3U)
  #define DEVICE_MAKE_MODEL "M5_ATOM_S3U"
#elif defined(WAVESHARE_RP2040_GEEK)
  #define DEVICE_MAKE_MODEL "WAVESHARE_RP2040_GEEK"
#elif defined(WAVESHARE_ESP32_S3_ETH)
  #define DEVICE_MAKE_MODEL "WAVESHARE_ESP32_S3_ETH"
#elif defined(WAVESHARE_ESP32_S3_GEEK)
  #define DEVICE_MAKE_MODEL "WAVESHARE_ESP32_S3_GEEK"
#elif defined(LILYGO_T_WATCH_S3)
  #define DEVICE_MAKE_MODEL "LILYGO_T_WATCH_S3"
#elif defined(ARDUINO_ARCH_ESP32S3)
  #define DEVICE_MAKE_MODEL "ARDUINO_ARCH_ESP32S3"
#elif defined(ARDUINO_ARCH_ESP32S2)
  #define DEVICE_MAKE_MODEL "ARDUINO_ARCH_ESP32S2"
#elif defined(ARDUINO_ARCH_RP2040)
  #define DEVICE_MAKE_MODEL "ARDUINO_ARCH_RP2040"
#else
  #define DEVICE_MAKE_MODEL "UNKNOWN"
#endif