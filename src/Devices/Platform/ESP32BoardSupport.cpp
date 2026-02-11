#ifdef ARDUINO_ARCH_ESP32
#include "ESP32BoardSupport.h"

#include <backtrace_saver.hpp>

#include "../../Debug/Logging.h"
#include "../../Devices/USB/USBCDC.h"
#include "../../Devices/TFT/HardwareTFT.h"
#define TAG "BOARD"

static unsigned long previousMillis = 0;
static bool hasHadCrash = false;
static bool emittedCrashDump = false;

namespace Devices
{
    ESP32BoardSupport Board;
}

ESP32BoardSupport::ESP32BoardSupport()
{
}

void ESP32BoardSupport::begin(Preferences &prefs)
{
    BoardSupport::begin(prefs);

    backtrace_saver::init();
    const auto &resetReason = esp_reset_reason();
    hasHadCrash = resetReason != ESP_RST_UNKNOWN && resetReason != ESP_RST_POWERON && resetReason != ESP_RST_SW;
}

bool ESP32BoardSupport::hasCrashed()
{
    return hasHadCrash;
}

void ESP32BoardSupport::loop(Preferences &prefs)
{
    BoardSupport::loop(prefs);

    if (hasHadCrash && !emittedCrashDump)
    {
        const auto RED = Devices::TFT.convertStringToColor("RED");
        const auto BLACK = Devices::TFT.convertStringToColor("BLACK");
        const auto WHITE = Devices::TFT.convertStringToColor("WHITE");
        const auto LIGHTGREY = Devices::TFT.convertStringToColor("LIGHTGREY");

        Devices::TFT.clearScreen();
        Devices::TFT.setBackgroundColor(RED);
        Devices::TFT.setForegroundColor(BLACK);
        Devices::TFT.display(0, 0, "Crash dump, raise a bug!");
        Devices::TFT.setBackgroundColor(BLACK);
        Devices::TFT.setForegroundColor(WHITE);

        const backtrace_saver::re_restart_debug_t current_debug_info = backtrace_saver::debugGet();
        char pcBuffer[16] = {0};

        std::string stackTrace;
        stackTrace.reserve(200);
        uint8_t lineNumber = 2;

        for (int i = 0; i < CONFIG_RESTART_DEBUG_STACK_DEPTH && current_debug_info.backtracePC[i] != 0; i++)
        {
            sprintf(pcBuffer, "%x", current_debug_info.backtracePC[i]);
            const auto& pcBufStr = std::string(pcBuffer);
            stackTrace += pcBufStr + " ";

#ifdef DISPLAY_HEIGHT
            if (lineNumber * 10 < DISPLAY_HEIGHT)
            {
                Devices::TFT.setForegroundColor(i % 2 == 0 ? WHITE : LIGHTGREY);

                Devices::TFT.display(
                    i % 2 == 0 ? 0 : 60,
                    lineNumber * 10, pcBufStr);
            }

            lineNumber = i % 2 != 0 ? lineNumber + 1 : lineNumber;
#endif
        }

        // addr2line -pfiaC -e .pio/build/esp32/firmware.elf 400d1ba7 400d575f 40082e1d 400846c8 400829c5 400891ad
        Debug::Log.info(TAG, stackTrace);

        emittedCrashDump = true;
    }
}
#endif