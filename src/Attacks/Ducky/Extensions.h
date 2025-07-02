#include "Adafruit_TinyUSB.h"
#include "esp_ota_ops.h"

#include "../../Debug/Logging.h"

#include "../../Devices/Button/HardwareButton.h"
#include "../../Devices/Storage/HardwareStorage.h"
#include "../../Devices/USB/USBCore.h"
#include "../../Devices/USB/USBHID.h"
#include "../../Devices/USB/USBMSC.h"
#include "../../Devices/USB/USBNCM.h"
#include "../../Devices/TFT/HardwareTFT.h"
#include "../../Devices/LED/HardwareLED.h"
#include "../../Devices/WiFi/HardwareWiFi.h"
#include "../../Devices/Touch/HardwareTouch.h"
#include "../../Comms/Web/WebServer.h"

#include "../../Attacks/Marauder/Marauder.h"
#include "../../Attacks/Agent/Agent.h"

#include "../../Utilities/Settings.h"

#include "DuckyPayload.h"

// For VID and PID str handling
static std::string vidStr;
static const std::string Constant_VID = "#_VID_";
static std::string pidStr;
static const std::string Constant_PID = "#_PID_";
static const std::string Constant_FileIndexFileName = "#_FILE_INDEX_FILE_NAME_";

// For FILE_INDEX_VALID() and LOAD_FILES_FROM_SD
static std::vector<std::string> curListOfFiles;
static int curFileIndexVariable = 0;

bool startsWith(const std::string& str, const std::string& prefix) {
    return str.rfind(prefix, 0) == 0;
}

static int handleCalc(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Devices::USB::HID.consumer_device_keypress(HID_USAGE_CONSUMER_AL_CALCULATOR);
    return true;
}

static int handleDisplayClear(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Devices::TFT.clearScreen();
    return true;
}

static int handleTFT_OFF(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Devices::TFT.powerOff();
    return true;
}

static int handleTFT_ON(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Devices::TFT.powerOn();
    return true;
}

static int handleDisplayPNG(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    std::string arg = str.substr(str.find(' ') + 1);
    Devices::TFT.displayPng(Devices::Storage, arg);
    return true;
}

static int asciiOrVariableToInt(const std::string &input, const std::unordered_map<std::string, int> &variables)
{
    if (variables.find(input) == variables.cend())
    {
        return atoi(input.c_str());
    }
    else
    {
        return variables.at(input);
    }
}

static int handleDisplayText(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    std::string remainingArgs = str.substr(str.find(' ') + 1);

    auto xPosStr = remainingArgs.substr(0, remainingArgs.find(' '));
    int xPos = asciiOrVariableToInt(xPosStr, variables);

    auto yStartPos = xPosStr.length() + 1;
    auto yCount = remainingArgs.find(' ', xPosStr.length() + 1) - yStartPos;

    auto yPosStr = remainingArgs.substr(yStartPos, yCount);
    int yPos = asciiOrVariableToInt(yPosStr, variables);

    auto text = remainingArgs.substr(xPosStr.length() + yPosStr.length() + 2);

    for (const auto& pair : variables)
    {
        text = Ducky::replaceAllOccurrences(text, pair.first, std::to_string(pair.second));
    }

    for (const auto& pair : constants)
    {
        text = Ducky::replaceAllOccurrences(text, pair.first, pair.second);
    }

    Devices::TFT.display(xPos, yPos, text);
    return true;
}

static int handleUSBMode(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    bool mountAsCdrom = startsWith(str, "USB_MOUNT_CDROM_READ_ONLY");

    std::string arg = str.substr(str.find(' ') + 1);
    const auto entries = Ducky::SplitString(arg);

    if (entries.size() == 1)
    {
        Debug::Log.info(LOG_DUCKY, "Mounting disk " + entries[0]);
        return Devices::USB::MSC.mountDiskImage(entries[0], mountAsCdrom);
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "Bad argument count: " + std::to_string(entries.size()));
    }
    return false;
}

static int handleLED(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    std::string arg = str.substr(str.find(' ') + 1);

    const auto entries = Ducky::SplitString(arg);
    if (entries.size() == 4)
    {
        auto hue = asciiOrVariableToInt(entries[0], variables);
        auto sat = asciiOrVariableToInt(entries[1], variables);
        auto lum = asciiOrVariableToInt(entries[2], variables);
        auto brightness = asciiOrVariableToInt(entries[3], variables);

        Devices::LED.changeLEDState(true, hue, sat, lum, brightness);
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "LED command has invalid arguments");
    }

    return true;
}

static int handleLEDBlue(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Devices::LED.changeLEDState(true, 240, 100, 100, 255);
    return true;
}

static int handleFileExists(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    // str is the current line, we need to peak in the constant #FILE
    if (constants.find("#FILE") != constants.cend())
    {
        auto ret = Devices::Storage.doesFileExist(constants.at("#FILE"));

        Debug::Log.info(LOG_DUCKY, "FILE_EXISTS() returned " + std::to_string(ret));
        return ret;
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "Error handling FILE_EXISTS() no param");
        return false;
    }
}

static int handleCreateFile(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    // str is the current line, we need to peak in the constant #FILE
    if (constants.find("#FILE") != constants.cend())
    {
        return Devices::Storage.createEmptyFile(constants.at("#FILE"));
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "Error handling CREATE_FILE() no param");
        return false;
    }
}

static int handleESP32Marauder(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    std::string arg = str.substr(str.find(' ') + 1);
    Attacks::Marauder.run(arg);
    return true;
}

static int handleWebOff(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Comms::Web.end();
    return true;
}

static int handleWiFiOff(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Devices::WiFi.setWiFi(false);
    return true;
}

static int handleWiFiOn(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Devices::WiFi.setWiFi(true);
    return true;
}

static int handleSerial(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    std::string arg = str.substr(str.find(' ') + 1);
    auto speed = asciiOrVariableToInt(arg.c_str(), variables);
    Serial.begin(speed);

    return true;
}

static int handleSetSetting(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    bool ret = false;

    do
    {
        const auto split = Ducky::SplitString(str);
        if (split.size() != 3)
        {
            Debug::Log.error(LOG_DUCKY, "SET_SETTING needs two arguments has: "+std::to_string(split.size() - 1));
            Debug::Log.error(LOG_DUCKY, str);
            totalErrors++;
            break;
        }

        const auto settingName = split[1];
        auto settingValue = split[2];

        if (startsWith(str, "SET_SETTING_UINT16") && settingValue.rfind("0x", 0) != 0)
        {
            // need to add 0x to front
            settingValue = "0x" + settingValue;
        }

        if (startsWith(str, "SET_SETTING_BOOL"))
        {
            if (settingValue == "0" || settingValue == "1" || settingValue == "TRUE" || settingValue == "FALSE")
            {
                // convert to 0 or 1
                if (settingValue == "TRUE" || settingValue == "1")
                {
                    settingValue = "1";
                }
                else
                {
                    settingValue = "0";
                }
            }
            else
            {
                Debug::Log.error(LOG_DUCKY, "Invalid bool type, should be 0,1,TRUE or FALSE, got " + settingValue);
                totalErrors++;
                break;
            }
        }

        Debug::Log.info(LOG_DUCKY, "Trying to set "+settingName+" to " +settingValue);
        ret = setSettingValue(*preferences, settingName, settingValue);

    } while (false);

    return ret;
}

static int handleUsbNcmPcapOn(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Devices::USB::NCM.startPacketCollection();
    return true;
}

static int handleUsbNcmPcapOff(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Devices::USB::NCM.stopPacketCollection();
    return true;
}

static int handleAgentRun(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    std::string arg = str.substr(str.find(' ') + 1);
    Attacks::Agent.run(arg);
    return true;
}

void doAgentRunResultWait(const std::function<void(const int &)> &delay)
{
    while (true)
    {
        delay(150);
        if (Attacks::Agent.hasAgentCmdResult())
        {
            Attacks::Agent.resetAgentCmdResultState();
            break;
        }
    }

    timeToWait = 0;
}

#ifdef ARDUINO_ARCH_ESP32
void AgentRunResultWaitTask(void *arg)
{
    doAgentRunResultWait(esp32_task_delay);
    vTaskDelete(NULL);
}
#endif

static int handleWaitForhandleAgentRunResult(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Debug::Log.info(LOG_DUCKY, "Waiting for agent run result");

    timeToWait = -1;

#ifdef ARDUINO_ARCH_ESP32
    xTaskCreate(
        AgentRunResultWaitTask, // Function that should be called
        "AgentRunWait",         // Name of the task (for debugging)
        1000,                   // Stack size (bytes)
        NULL,                   // Parameter to pass
        1,                      // Task priority
        NULL                    // Task handle
    );
#else
    doAgentRunResultWait([](const uint32_t &time) { loop(); });
#endif

    return true;
}

static int handleDeleteFile(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    // str is the current line, we need to peak in the constant #FILE
    if (constants.find("#FILE") != constants.cend())
    {
        auto ret = Devices::Storage.deleteFile(constants.at("#FILE"));

        Debug::Log.info(LOG_DUCKY, "DELETE_FILE() returned " + std::to_string(ret));
        return ret;
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "Error handling DELETE_FILE() no param");
        return false;
    }
}

static int handleAgentConnected(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    return Attacks::Agent.isAgentConnected();
}

// handles both LOAD_FILES_FROM_SD() and IS_FILE_INDEX_VALID()
static int handleFileIndex(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    bool ret = false;

    if (str == "LOAD_FILES_FROM_SD()")
    {
        curListOfFiles = Devices::Storage.listFiles();
        ret = true;
    }
    else
    {
        // FILE_INDEX_VALID()
        const std::string varName = "$FILE_INDEX";

        if (variables.find(varName) == variables.cend())
        {
            Debug::Log.info(LOG_DUCKY, "could not find var!");
            ret = false;
        }
        else
        {
            curFileIndexVariable = variables.at(varName);
            if (curFileIndexVariable >= curListOfFiles.size())
            {
                ret = false;
            }
            else
            {
                ret = true;
            }
        }
    }

    return ret;
}

static int handleButtonPress(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Debug::Log.info(LOG_DUCKY, "handleButtonPress '" + str + "'");
    if (str == "BUTTON_LONG_PRESS()")
    {
        return lastButtonPressState == 1;
    }
    else
    {
        return lastButtonPressState == 0;
    }
}

static int handleGetSettingValue(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    // str is the current line, we need to peak in the constant #FILE
    if (constants.find("#SETTING_NAME") != constants.cend())
    {
        bool error = false;
        uint16_t value = getIntegerSettingValue(*preferences, constants.at("#SETTING_NAME"), error);
        if (error)
        {
            // todo, this isn't a great pattern. We should be able to set an error here to stop further script
            // execution. The best we can do is return 0 but what if a setting was really 0.
            // DuckyScript interpeter either needs:
            // * a method we can call to stop execution
            // * a better function declation that we can return an error through
            Debug::Log.error(LOG_DUCKY, "GET_SETTING_VALUE() error, setting is unknown or string type");
            totalErrors++;
            return 0;
        }
        Debug::Log.info(LOG_DUCKY, "GET_SETTING_VALUE() for "+constants.at("#SETTING_NAME")+" returned " + std::to_string(value));
        return value;
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "Error handling GET_SETTING_VALUE() no param");
        return false;
    } 
}

static int handleGetRecvPackets(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    return Attacks::Marauder.getPacketCount();
}

static int handleRunPayload(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    const std::string arg = str.substr(str.find(' ') + 1);
    if (arg.ends_with(".ds"))
    {
        Attacks::Ducky.setPayload(arg);
        duckyFileParser.Restart();
    }
    else if (arg.ends_with(".png"))
    {
        currentlyExecutingFile.clear();
        Attacks::Ducky.setPayloadCmdLine("DISPLAY_PNG " + arg);
    }

    return true;
}

void doMSCActivityWait(const std::function<void(const int &)> &delay)
{
    while (true)
    {
        delay(150);
        if (Devices::USB::MSC.hasActivity())
        {
            Devices::USB::MSC.resetActivityState();
            break;
        }
    }

    timeToWait = 0;
}

#ifdef ARDUINO_ARCH_ESP32
void MSCActivityWaitTask(void *arg)
{
    doMSCActivityWait(esp32_task_delay);
    vTaskDelete(NULL);
}
#endif

static int handleWaitForUSBStorageActivity(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    // We can do other things while we are waiting so we kick off a task to wait
    // return, which causes us to loop again
    // in our loop we check if the task is finished and only continue processing if it has
    Debug::Log.info(LOG_DUCKY, "Waiting for MSC activity");

    timeToWait = -1;

#ifdef ARDUINO_ARCH_ESP32
    xTaskCreate(
        MSCActivityWaitTask, // Function that should be called
        "MSCWait",           // Name of the task (for debugging)
        1000,                // Stack size (bytes)
        NULL,                // Parameter to pass
        1,                   // Task priority
        NULL                 // Task handle
    );
#else
    doMSCActivityWait([](const uint32_t &time) { loop(); });
#endif

    return true;
}

static void doMSCActivityWaitToStop(const std::function<void(const int &)> &delay)
{
    uint8_t numberOfPeriodsOfZeroActivity = 0;

    while (true)
    {
        delay(100);
        if (Devices::USB::MSC.hasActivity())
        {
            Devices::USB::MSC.resetActivityState();
            numberOfPeriodsOfZeroActivity = 0;
        }
        else
        {
            numberOfPeriodsOfZeroActivity++;
        }

        if (numberOfPeriodsOfZeroActivity >= 10)
        {
            break;
        }
    }

    timeToWait = 0;
}

#ifdef ARDUINO_ARCH_ESP32
void MSCActivityWaitToStopTask(void *arg)
{
    doMSCActivityWaitToStop(esp32_task_delay);
    vTaskDelete(NULL);
}
#endif

static int handleWaitForUSBStorageActivityToStop(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    // We can do other things while we are waiting so we kick off a task to wait
    // return, which causes us to loop again
    // in our loop we check if the task is finished and only continue processing if it has
    Debug::Log.info(LOG_DUCKY, "Waiting for MSC activity to cease for 1000ms");

    timeToWait = -1;

#ifdef ARDUINO_ARCH_ESP32
    xTaskCreate(
        MSCActivityWaitToStopTask, // Function that should be called
        "MSCWait",                 // Name of the task (for debugging)
        1000,                      // Stack size (bytes)
        NULL,                      // Parameter to pass
        1,                         // Task priority
        NULL                       // Task handle
    );
#else
    doMSCActivityWaitToStop([](const uint32_t &time) { loop(); });
#endif

    return true;
}

static int handleWaitForButtonOrTimeout(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    const std::string arg = str.substr(str.find(' ') + 1);
    auto timeout = asciiOrVariableToInt(arg, variables);

    waitForButtonWithTimeout(timeout);
    return true;
}

static int handleRawHid(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    bool ret = false;

    const std::string arg = str.substr(str.find(' ') + 1);
    const auto words = Ducky::SplitString(arg);

    if (words.size() == 0 || words.size() > 2)
    {
        Debug::Log.info(LOG_DUCKY, "Invalid syntax");
        return false;
    }

    const auto hidStr = words.size() == 1 ? words[0] : words[1];
    const auto modStr = words.size() == 1 ? std::string() : words[0];

    if (hidStr.rfind("0x", 0) == 0)
    {
        auto tempInt = std::stoi(hidStr, nullptr, 16);
        if (tempInt <= static_cast<int>(UINT8_MAX) && tempInt >= 0)
        {
            uint8_t uint8Value = 0;
            uint8Value = static_cast<uint8_t>(tempInt);

            Debug::Log.info(LOG_DUCKY, "Pressing key " + std::to_string(uint8Value));

            uint8_t modKey = modStr == "SHIFT" ? (uint8_t)USBKeyDefinition::UsbHidModifiers::LeftShift : 0;

            Devices::USB::HID.keyboard_press(modKey, uint8Value, 0, 0, 0, 0, 0);
            Devices::USB::HID.keyboard_release();

            ret = true;
        }
        else
        {
            Debug::Log.error(LOG_DUCKY, "Invalid value");
        }
    }
    else
    {
        Debug::Log.error(LOG_DUCKY, "Invalid value, must start with 0x");
    }

    return ret;
}

static int handleLog(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    std::string text = str.substr(str.find(' ') + 1);

    for (const auto& pair : variables)
    {
        text = Ducky::replaceAllOccurrences(text, pair.first, std::to_string(pair.second));
    }

    for (const auto& pair : constants)
    {
        text = Ducky::replaceAllOccurrences(text, pair.first, pair.second);
    }

    Debug::Log.info(LOG_DUCKY, text);
    return true;
}

static int handleKeyboardLayout(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    const std::string arg = str.substr(str.find(' ') + 1);

    const auto entries = Ducky::SplitString(arg);
    if (entries.size() == 1)
    {
        const auto &ret = duckyFileParser.SetKeyboardLayout(entries[0]);

        if (!ret)
        {
            Debug::Log.error(LOG_DUCKY, "Invalid keyboard layout, did you forget to compile it in?");
        }
        else
        {
            Debug::Log.info(LOG_DUCKY, "Keyboard layout set to " + entries[0]);
            return true;
        }
    }
    else
    {
        Debug::Log.error(LOG_DUCKY, "Invalid layout, too many tokens");
    }

    return false;
}

static int partitionSwap(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Debug::Log.info(LOG_DUCKY, "Starting partition swap, this will reboot the device");

#ifdef LILYGO_T_WATCH_S3
    const esp_partition_t *partition = esp_ota_get_next_update_partition(NULL);

    if (partition == nullptr)
    {
        Debug::Log.error(LOG_DUCKY, "Could not find partition");
        return true;
    }

    if (esp_ota_set_boot_partition(partition) == ESP_OK) {
        Debug::Log.info(LOG_DUCKY, "Swapped partitions, will reboot");
        ESP.restart();
    }
    else
    {
        Debug::Log.error(LOG_DUCKY, "Could not swap partitions");
    }
#else
    Debug::Log.error(LOG_DUCKY, "Partition swap not supported on this device");
#endif
    return true;
}

static void doTouchActivityWait(const std::function<void(const int &)> &delay)
{
    Devices::Touch.resetTouchState();

    while (true)
    {
        delay(150);
        if (Devices::Touch.hasBeenTouched())
        {
            break;
        }
    }

    timeToWait = 0;
}

#ifdef ARDUINO_ARCH_ESP32
void doTouchActivityWaitTask(void *arg)
{
    doTouchActivityWait(esp32_task_delay);
    vTaskDelete(NULL);
}
#endif

static int waitForTouch(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    // We can do other things while we are waiting so we kick off a task to wait
    // return, which causes us to loop again
    // in our loop we check if the task is finished and only continue processing if it has
    Debug::Log.info(LOG_DUCKY, "Waiting for touch activity");

    timeToWait = -1;

#ifdef ARDUINO_ARCH_ESP32
    xTaskCreate(
        doTouchActivityWaitTask, // Function that should be called
        "TouchWait",                 // Name of the task (for debugging)
        1000,                      // Stack size (bytes)
        NULL,                      // Parameter to pass
        1,                         // Task priority
        NULL                       // Task handle
    );
#else
    doTouchActivityWait([](const uint32_t &time) { loop(); });
#endif

    return true;
}

static int getTouchPosition(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    const auto pos = startsWith(str, "GET_X_POS") ? Devices::Touch.getXPos() : Devices::Touch.getYPos();
    Debug::Log.info(LOG_DUCKY, str + " " + std::to_string(pos) );
    return pos;
}

static int handleMouseJiggle(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    Devices::USB::HID.mouseMove(1, 1);
    Devices::USB::HID.mouseMove(-1, -1);
    return true;
}

static int handleMouseMove(const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
{
    const std::string arg = str.substr(str.find(' ') + 1);

    const auto entries = Ducky::SplitString(arg);
    if (entries.size() == 2)
    {
        int xDelta = asciiOrVariableToInt(entries[0], variables);
        int yDelta = asciiOrVariableToInt(entries[1], variables);
        Devices::USB::HID.mouseMove(xDelta, yDelta);
        return true;
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "Bad argument count: " + std::to_string(entries.size()));
        return false;
    }
}

void addDuckyScriptExtensions(
    ExtensionCommands &extCommands,
    UserDefinedConstants &consts)
{
    // Ok so whats the difference here between a command and a function?
    // A command is an action, it performs an action on something
    // A function returns a value, it can optionally perform an action as well

    // Display/UI related
    extCommands["TFT_OFF"] = handleTFT_OFF;
    extCommands["TFT_ON"] = handleTFT_ON;
    extCommands["DISPLAY_PNG"] = handleDisplayPNG;
    extCommands["DISPLAY_TEXT"] = handleDisplayText;
    extCommands["DISPLAY_CLEAR"] = handleDisplayClear;
    extCommands["LED"] = handleLED;
    extCommands["LED_B"] = handleLEDBlue;

    // touch
    extCommands["WAIT_FOR_TOUCH"] = waitForTouch;
    extCommands["GET_X_POS()"] = getTouchPosition;
    extCommands["GET_Y_POS()"] = getTouchPosition;

    // Other attacks
    extCommands["CALC"] = handleCalc;
    extCommands["ESP32M"] = handleESP32Marauder;
    extCommands["RUN_PAYLOAD"] = handleRunPayload;
    extCommands["RAW_HID"] = handleRawHid;

    // Utilities
    extCommands["LOG"] = handleLog;
    extCommands["PARTITION_SWAP"] = partitionSwap;

    // Device related
    extCommands["WEB_OFF"] = handleWebOff;
    extCommands["WIFI_OFF"] = handleWiFiOff;
    extCommands["WIFI_ON"] = handleWiFiOn;
    extCommands["SERIAL"] = handleSerial;
    extCommands["SET_SETTING_BOOL"] = handleSetSetting;
    extCommands["SET_SETTING_INT16"] = handleSetSetting;
    extCommands["SET_SETTING_UINT16"] = handleSetSetting;
    extCommands["SET_SETTING_STRING"] = handleSetSetting;

    // USB
    extCommands["USB_MOUNT_DISK_READ_ONLY"] = handleUSBMode;
    extCommands["USB_MOUNT_CDROM_READ_ONLY"] = handleUSBMode;
    extCommands["USB_NCM_PCAP_ON"] = handleUsbNcmPcapOn;
    extCommands["USB_NCM_PCAP_OFF"] = handleUsbNcmPcapOff;
    extCommands["WAIT_FOR_USB_STORAGE_ACTIVITY"] = handleWaitForUSBStorageActivity;
    extCommands["WAIT_FOR_USB_STORAGE_ACTIVITY_TO_STOP"] = handleWaitForUSBStorageActivityToStop;
    extCommands["WAIT_FOR_BUTTON_PRESS_OR_TIMEOUT"] = handleWaitForButtonOrTimeout;
    extCommands["MOUSE_JIGGLE"] = handleMouseJiggle;
    extCommands["MOUSE_MOVE"] = handleMouseMove;

    extCommands["KEYBOARD_LAYOUT"] = handleKeyboardLayout;

    // Agent
    extCommands["AGENT_RUN"] = handleAgentRun;
    extCommands["WAIT_FOR_AGENT_RUN_RESULT"] = handleWaitForhandleAgentRunResult;

    // Functions
    extCommands["FILE_EXISTS()"] = handleFileExists;
    extCommands["CREATE_FILE()"] = handleCreateFile;
    extCommands["DELETE_FILE()"] = handleDeleteFile;
    extCommands["AGENT_CONNECTED()"] = handleAgentConnected;
    extCommands["FILE_INDEX_VALID()"] = handleFileIndex;
    extCommands["LOAD_FILES_FROM_SD()"] = handleFileIndex;
    extCommands["BUTTON_LONG_PRESS()"] = handleButtonPress;
    extCommands["BUTTON_SHORT_PRESS()"] = handleButtonPress;
    extCommands["GET_SETTING_VALUE()"] = handleGetSettingValue;
    extCommands["ESP32M_GET_RECV_PACKETS()"] = handleGetRecvPackets;

    consts.emplace_back([]
                        {
        if (vidStr.empty())
        {
            char hexString[5] = {0};
            sprintf(hexString,"%x",Devices::USB::Core.getVID()); // converts to hexadecimal base.
            vidStr = std::string(hexString);
        }
        return std::pair(Constant_VID, vidStr); });
    consts.emplace_back([]
                        {
        if (pidStr.empty())
        {
            char hexString[5] = {0};
            sprintf(hexString,"%x",Devices::USB::Core.getPID()); // converts to hexadecimal base.
            pidStr = std::string(hexString);
        }
        return std::pair(Constant_PID, pidStr); });
    consts.emplace_back([]
                        {
        if (curListOfFiles.size() != 0)
        {
            return std::pair(Constant_FileIndexFileName, curListOfFiles[curFileIndexVariable]);
        }
        else
        {
            return std::pair(Constant_FileIndexFileName, std::string());
        } });
}