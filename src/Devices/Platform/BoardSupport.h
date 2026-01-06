#pragma once

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
#elif defined(EVIL_CROW_CABLE_WIND)
  #define DEVICE_MAKE_MODEL "EVIL_CROW_CABLE_WIND"
#elif defined(POCKET_DONGLE_S3)
  #define DEVICE_MAKE_MODEL "POCKET_DONGLE_S3"
#elif defined(ARDUINO_ARCH_ESP32S3)
  #define DEVICE_MAKE_MODEL "ARDUINO_ARCH_ESP32S3"
#elif defined(ARDUINO_ARCH_ESP32S2)
  #define DEVICE_MAKE_MODEL "ARDUINO_ARCH_ESP32S2"
#elif defined(ARDUINO_ARCH_RP2040)
  #define DEVICE_MAKE_MODEL "ARDUINO_ARCH_RP2040"
#else
  #define DEVICE_MAKE_MODEL "UNKNOWN"
#endif

#ifdef EVIL_CROW_CABLE_WIND
  #include "EvilCrowWind/EvilCrowWind.h"
#elif defined(LILYGO_T_WATCH_S3)
  #include "TWatchS3/TWatchS3.h"
#elif defined(ARDUINO_ARCH_ESP32)
  #include "ESP32BoardSupport.h"
#else
  #include "BoardSupportImpl.h"
#endif

namespace Devices
{
#ifdef EVIL_CROW_CABLE_WIND
  extern EvilCrowWind Board;
#elif defined(LILYGO_T_WATCH_S3)
  extern TWatchS3 Board;
#elif defined(ARDUINO_ARCH_ESP32)
  extern ESP32BoardSupport Board;
#else
  extern BoardSupport Board;
#endif
}