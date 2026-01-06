#include "BoardSupportImpl.h"

#ifndef ARDUINO_ARCH_ESP32
#ifndef EVIL_CROW_CABLE_WIND
#ifndef LILYGO_T_WATCH_S3
namespace Devices
{
    BoardSupport Board;
}
#endif
#endif
#endif

BoardSupport::BoardSupport()
{
}

void BoardSupport::begin(Preferences &prefs)
{
}

void BoardSupport::loop(Preferences &prefs)
{
}