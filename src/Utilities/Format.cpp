#ifndef NO_SD
#include "../Devices/Button/HardwareButton.h"
#include "../Devices/LED/HardwareLED.h"
#include "../Devices/TFT/HardwareTFT.h"
#include "../Devices/Storage/HardwareStorage.h"
#include "../Devices/WiFi/HardwareWiFi.h"

static bool running = false;

#ifdef ARDUINO_ARCH_ESP32
void FormatStatusUpdateTask(void *arg)
{
    const std::string line1 = "Formatting";
    const std::string line2 = "This will take some time.";

    int count = 0;
    Devices::TFT.clearScreen();
    Devices::TFT.display(0, 0, line1);
    Devices::TFT.display(0, 10, line2);

    while (running)
    {
        vTaskDelay(pdMS_TO_TICKS(1500));
        Devices::LED.changeLEDState(count % 2 == 0, 24, 100, 100, 255);
        Devices::TFT.display(count, 20, ".");
        count++;

        if (count >= DISPLAY_WIDTH -1)
        {
            Devices::TFT.clearScreen();
            Devices::TFT.display(0, 0, line1);
            Devices::TFT.display(0, 10, line2);
            count = 0;
        }
    }
    vTaskDelete(NULL);
}
#endif

void AskFormatSD(Preferences &prefs)
{
    // Display a message when the SD card cannot be found. Some devices are very picky about the
    // make and model of SD card they support, if you are debugging and are reading this try another card
    // and format with 1 FAT32 partition <32gb
    Devices::TFT.display(0, 0, "Error: Valid SD not found");

    Devices::TFT.display(0, 20, "Hold button to attempt");
    Devices::TFT.display(0, 30, "to format the SD card.");

    Devices::TFT.display(0, 50, "Note: the drive will not");
    Devices::TFT.display(0, 60, "be readable on Windows");

    // we don't want people using the web interface if the SD card is bad
    Devices::WiFi.end();

    for (int x = 0; x < 60 * 1000; x++)
    {
        delay(1);
        Devices::Button.loop(prefs);
        if (Devices::Button.hasButtonBeenPressed() && Devices::Button.wasLastPressLong())
        {
            running = true;

#ifdef ARDUINO_ARCH_ESP32
            xTaskCreate(
                FormatStatusUpdateTask, // Function that should be called
                "FormatStatus",         // Name of the task (for debugging)
                4000,                   // Stack size (bytes)
                NULL,                   // Parameter to pass
                1,                      // Task priority
                NULL            // Task handle
            );
#else
            Devices::TFT.clearScreen();
            Devices::TFT.display(0, 0, "Formatting...");
#endif
            Devices::Storage.begin(prefs, true);
            running = false;
            delay(1500); // rubbish way to wait for the task

            Devices::TFT.clearScreen();
            Devices::TFT.display(0, 0, "Formatting...complete");

            delay(5000);
            break;
        }
    }

#ifdef ARDUINO_ARCH_ESP32
    ESP.restart();
#elif defined(ARDUINO_ARCH_RP2040)
    watchdog_reboot(0, SRAM_END, 0);
#endif
}
#endif