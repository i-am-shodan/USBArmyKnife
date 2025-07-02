#include "Logging.h"
#include "../Devices/USB/USBCDC.h"
#include "../Attacks/Agent/Agent.h"

static std::vector<std::string> logMsg;

#define LOG_DELAY_UNTIL_DEBUGGER                    "loggerdelay"
#define LOG_DELAY_UNTIL_DEBUGGER_DEFAULT            true

static bool loggerDelayUntilDebuggerConnected = LOG_DELAY_UNTIL_DEBUGGER_DEFAULT;
static bool writtenPendingEntries = false;

namespace Debug
{
    Logging Log;
}

Logging::Logging()
{
}

std::vector<std::string>& Logging::getLogs()
{
    return logMsg;
}

static void writeDebugOut(const std::string& component, const std::string& msg)
{
    logMsg.emplace_back(msg);
    if (loggerDelayUntilDebuggerConnected == false || (loggerDelayUntilDebuggerConnected && Attacks::Agent.isAgentConnected()))
    {
        Devices::USB::CDC.writeDebugString(std::string(component) + "|" + msg);
    } 
}

void Logging::info(const char *component, const std::string &msg)
{
    // Ensure we can't overflow the debug log with large amounts of data
    if (msg.length() > MAX_LOG_ENTRY_IN_CHARS)
    {
        const std::string err = "ERROR LOG MESSAGE TOO LARGE!";
        writeDebugOut(component, err);
    }
    else
    {
        writeDebugOut(component, msg);
    }
}

void Logging::warning(const char *component, const std::string &msg)
{
    info(component, msg);
}

void Logging::error(const char *component, const std::string &msg)
{
    info(component, msg);
}

extern "C" void log_compat(const char* msg)
{
    std::string logMsg(msg);
    Debug::Log.info("C", logMsg);
}

void Logging::loop(Preferences &prefs)
{
    if (logMsg.size() >= 30)
    {
        // Remove the first 5 elements, and shift everything else down by 5 indices
        logMsg.erase(logMsg.begin(), logMsg.begin() + 5);
    }

    if (loggerDelayUntilDebuggerConnected && !writtenPendingEntries && Attacks::Agent.isAgentConnected())
    {
        for (const auto& msg : logMsg)
        {
            Devices::USB::CDC.writeDebugString("OLD|" + msg);
        }
        writtenPendingEntries = true;
    }
}

void Logging::begin(Preferences &prefs)
{
    registerUserConfigurableSetting(CATEGORY_USB, LOG_DELAY_UNTIL_DEBUGGER, USBArmyKnifeCapability::SettingType::Bool, LOG_DELAY_UNTIL_DEBUGGER_DEFAULT);
    loggerDelayUntilDebuggerConnected = prefs.getBool(LOG_DELAY_UNTIL_DEBUGGER, LOG_DELAY_UNTIL_DEBUGGER_DEFAULT);
}