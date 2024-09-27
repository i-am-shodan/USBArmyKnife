#include "Logging.h"
#include "../Devices/USB/USBCDC.h"

static std::vector<std::string> logMsg;

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

void Logging::info(const char *component, const std::string &msg)
{
    // Ensure we can't overflow the debug log with large amounts of data
    if (msg.length() > MAX_LOG_ENTRY_IN_CHARS)
    {
        const std::string err = "ERROR LOG MESSAGE TOO LARGE!";
        logMsg.emplace_back(std::string(component) + " - " + err);
        Devices::USB::CDC.writeDebugString(std::string(component)+"|"+err);
    }
    else
    {
        logMsg.emplace_back(msg);
        Devices::USB::CDC.writeDebugString(std::string(component)+"|"+msg);
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
}

void Logging::begin(Preferences &prefs)
{
}