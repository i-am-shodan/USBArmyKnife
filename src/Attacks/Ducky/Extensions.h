#include "Adafruit_TinyUSB.h"

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

#include "../../Attacks/Marauder/Marauder.h"
#include "../../Attacks/Agent/Agent.h"

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

static int handleCalc(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    Devices::USB::HID.consumer_device_keypress(HID_USAGE_CONSUMER_AL_CALCULATOR);
    return true;
}

static int handleDisplayClear(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    Devices::TFT.clearScreen();
    return true;
}

static int handleTFT_OFF(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    Devices::TFT.powerOff();
    return true;
}

static int handleDisplayPNG(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    std::string arg = str.substr(str.find(' ') + 1);
    Devices::TFT.displayPng(Devices::Storage, arg);
    return true;
}

static int handleDisplayText(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    std::string remainingArgs = str.substr(str.find(' ') + 1);

    auto xPosStr = remainingArgs.substr(0, remainingArgs.find(' '));
    int xPos = atoi(xPosStr.c_str());

    auto yStartPos = xPosStr.length() + 1;
    auto yCount = remainingArgs.find(' ', xPosStr.length() + 1) - yStartPos;

    auto yPosStr = remainingArgs.substr(yStartPos, yCount);
    int yPos = atoi(yPosStr.c_str());

    auto text = remainingArgs.substr(xPosStr.length() + yPosStr.length() + 2);

    Devices::TFT.display(xPos, yPos, text);
    return true;
}

static int handleUSBMode(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    std::string arg = str.substr(str.find(' ') + 1);
    const auto entries = Ducky::SplitString(arg);

    if (entries.size() == 1)
    {
        Debug::Log.info(LOG_DUCKY, "Mounting disk " + entries[0]);
        return Devices::USB::MSC.mountDiskImage(entries[0]);
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "Bad argument count: " + std::to_string(entries.size()));
    }
    return false;
}

static int handleLED(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    std::string arg = str.substr(str.find(' ') + 1);

    const auto entries = Ducky::SplitString(arg);
    if (entries.size() == 4)
    {
        auto hue = atoi(entries[0].c_str());
        auto sat = atoi(entries[1].c_str());
        auto lum = atoi(entries[2].c_str());
        auto brightness = atoi(entries[3].c_str());

        Devices::LED.changeLEDState(true, hue, sat, lum, brightness);
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "LED command has invalid arguments");
    }

    return true;
}

static int handleFileExists(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    // str is the current line, we need to peak in the constant #FILE
    if (constants.find("#FILE") != constants.cend())
    {
        auto ret = Devices::Storage.doesFileExist(constants["#FILE"]);

        Debug::Log.info(LOG_DUCKY, "FILE_EXISTS() returned " + std::to_string(ret));
        return ret;
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "Error handling FILE_EXISTS() no param");
        return false;
    }
}

static int handleCreateFile(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    // str is the current line, we need to peak in the constant #FILE
    if (constants.find("#FILE") != constants.cend())
    {
        return Devices::Storage.createEmptyFile(constants["#FILE"]);
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "Error handling CREATE_FILE() no param");
        return false;
    }
}

static int handleESP32Marauder(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    std::string arg = str.substr(str.find(' ') + 1);
    Attacks::Marauder.run(arg);
    return true;
}

static int handleWiFiOff(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    Devices::WiFi.setWiFi(false);
    return true;
}

static int handleWiFiOn(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    Devices::WiFi.setWiFi(true);
    return true;
}

static int handleSerial(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    std::string arg = str.substr(str.find(' ') + 1);
    auto speed = atol(arg.c_str());
    Serial.begin(speed);

    return true;
}

static int handleUsbNcmPcapOn(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    Devices::USB::NCM.startPacketCollection();
    return true;
}

static int handleUsbNcmPcapOff(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    Devices::USB::NCM.stopPacketCollection();
    return true;
}

static int handleAgentRun(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    std::string arg = str.substr(str.find(' ') + 1);
    Attacks::Agent.run(arg);
    return true;
}

static int handleDeleteFile(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    // str is the current line, we need to peak in the constant #FILE
    if (constants.find("#FILE") != constants.cend())
    {
        auto ret = Devices::Storage.deleteFile(constants["#FILE"]);

        Debug::Log.info(LOG_DUCKY, "DELETE_FILE() returned " + std::to_string(ret));
        return ret;
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "Error handling DELETE_FILE() no param");
        return false;
    }
}

static int handleAgentConnected(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    return Attacks::Agent.isAgentConnected();
}

// handles both LOAD_FILES_FROM_SD() and IS_FILE_INDEX_VALID()
static int handleFileIndex(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
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
            curFileIndexVariable = variables[varName];
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

static int handleButtonPress(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    Debug::Log.info(LOG_DUCKY, "handleButtonPress '" + str + "'");
    if (str == "BUTTON_LONG_PRESS()")
    {
        return wasLastPressLong;
    }
    else
    {
        return wasLastPressLong == false;
    }
}

static int handleRunPayload(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    std::string arg = str.substr(str.find(' ') + 1);
    if (arg.ends_with(".ds"))
    {
        Attacks::Ducky.setPayload(arg);
        restartPayload = true;
    }
    else if (arg.ends_with(".png"))
    {
        currentlyExecutingFile.clear();
        nextLineNumberToExecute = 0;
        Attacks::Ducky.setPayloadCmdLine("DISPLAY_PNG " + arg);
    }

    return true;
}

void MSCActivityWaitTask(void *arg)
{
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(150));
        if (Devices::USB::MSC.hasActivity())
        {
            Devices::USB::MSC.resetActivityState();
            break;
        }
    }

    timeToWait = 0;
    vTaskDelete(NULL);
}

static int handleWaitForUSBStorageActivity(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    // We can do other things while we are waiting so we kick off a task to wait
    // return, which causes us to loop again
    // in our loop we check if the task is finished and only continue processing if it has
    Debug::Log.info(LOG_DUCKY, "Waiting for MSC activity");

    timeToWait = -1;

    xTaskCreate(
        MSCActivityWaitTask, // Function that should be called
        "MSCWait",           // Name of the task (for debugging)
        1000,                // Stack size (bytes)
        NULL,                // Parameter to pass
        1,                   // Task priority
        NULL                 // Task handle
    );

    return true;
}

void MSCActivityWaitToStopTask(void *arg)
{
    uint8_t numberOfPeriodsOfZeroActivity = 0;

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
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
    vTaskDelete(NULL);
}

static int handleWaitForUSBStorageActivityToStop(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
{
    // We can do other things while we are waiting so we kick off a task to wait
    // return, which causes us to loop again
    // in our loop we check if the task is finished and only continue processing if it has
    Debug::Log.info(LOG_DUCKY, "Waiting for MSC activity to cease for 1000ms");

    timeToWait = -1;

    xTaskCreate(
        MSCActivityWaitToStopTask, // Function that should be called
        "MSCWait",           // Name of the task (for debugging)
        1000,                // Stack size (bytes)
        NULL,                // Parameter to pass
        1,                   // Task priority
        NULL                 // Task handle
    );

    return true;
}

static int handleRawHid(const std::string &str, std::unordered_map<std::string, std::string> constants, std::unordered_map<std::string, int> variables)
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

            Debug::Log.info(LOG_DUCKY, "Pressing key "+std::to_string(uint8Value));

            uint8_t modKey = modStr == "SHIFT" ? (uint8_t) USBKeyDefinition::UsbHidModifiers::LeftShift : 0;
            
            Devices::USB::HID.keyboard_press(modKey, uint8Value, 0, 0, 0, 0, 0);
            Devices::USB::HID.keyboard_release();

            ret = true;
        }
        else
        {
            Debug::Log.info(LOG_DUCKY, "Invalid value");
        }
    }
    else
    {
        Debug::Log.info(LOG_DUCKY, "Invalid value, must start with 0x");
    }

    return ret;
}

void addDuckyScriptExtensions(
    std::unordered_map<std::string, std::function<int(std::string, std::unordered_map<std::string, std::string>, std::unordered_map<std::string, int>)>> &extCommands,
    std::vector<std::function<std::pair<std::string, std::string>()>> &consts)
{
    // Ok so whats the difference here between a command and a function?
    // A command is an action, it performs an action on something
    // A function returns a value, it can optionally perform an action as well

    // Display/UI related
    extCommands["TFT_OFF"] = handleTFT_OFF;
    extCommands["DISPLAY_PNG"] = handleDisplayPNG;
    extCommands["DISPLAY_TEXT"] = handleDisplayText;
    extCommands["DISPLAY_CLEAR"] = handleDisplayClear;
    extCommands["LED"] = handleLED;

    // Other attacks
    extCommands["CALC"] = handleCalc;
    extCommands["ESP32M"] = handleESP32Marauder;
    extCommands["RUN_PAYLOAD"] = handleRunPayload;
    extCommands["RAW_HID"] = handleRawHid;

    // Device related
    extCommands["WEB_OFF"] = handleWiFiOff;
    extCommands["WEB_ON"] = handleWiFiOn;
    extCommands["WIFI_OFF"] = handleWiFiOff;
    extCommands["WIFI_ON"] = handleWiFiOn;
    extCommands["SERIAL"] = handleSerial;

    // USB
    extCommands["USB_MOUNT_DISK_READ_ONLY"] = handleUSBMode;
    extCommands["USB_NCM_PCAP_ON"] = handleUsbNcmPcapOn;
    extCommands["USB_NCM_PCAP_OFF"] = handleUsbNcmPcapOff;
    extCommands["WAIT_FOR_USB_STORAGE_ACTIVITY"] = handleWaitForUSBStorageActivity;
    extCommands["WAIT_FOR_USB_STORAGE_ACTIVITY_TO_STOP"] = handleWaitForUSBStorageActivityToStop;

    // Agent
    extCommands["AGENT_RUN"] = handleAgentRun;

    // Functions
    extCommands["FILE_EXISTS()"] = handleFileExists;
    extCommands["CREATE_FILE()"] = handleCreateFile;
    extCommands["DELETE_FILE()"] = handleDeleteFile;
    extCommands["AGENT_CONNECTED()"] = handleAgentConnected;
    extCommands["FILE_INDEX_VALID()"] = handleFileIndex;
    extCommands["LOAD_FILES_FROM_SD()"] = handleFileIndex;
    extCommands["BUTTON_LONG_PRESS()"] = handleButtonPress;
    extCommands["BUTTON_SHORT_PRESS()"] = handleButtonPress;

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