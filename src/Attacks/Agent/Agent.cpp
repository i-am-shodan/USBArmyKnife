#include "Agent.h"

#include "../../Debug/Logging.h"
#include "../../Devices/USB/USBCDC.h"

namespace Attacks
{
    SerialAgent Agent;
}

#define LOG_AGENT "agent"

#define USB_AGENT_POLLING_ENABLED "agentPollingEnabled"
#define USB_AGENT_POLLING_ENABLED_DEFAULT true

#define USB_AGENT_POLLING_TIME_IN_SEC "agentPollingTime"
#define USB_AGENT_POLLING_TIME_IN_SEC_DEFAULT 15

static bool agentPollingEnabled = USB_AGENT_POLLING_ENABLED_DEFAULT;
static bool agentPollingIntervalInSec = USB_AGENT_POLLING_TIME_IN_SEC_DEFAULT;
static unsigned long previousMillis = 0; 

SerialAgent::SerialAgent()
{
}

void SerialAgent::begin(Preferences &prefs)
{
    registerUserConfigurableSetting(CATEGORY_USB, USB_AGENT_POLLING_ENABLED, USBArmyKnifeCapability::SettingType::Bool, USB_AGENT_POLLING_ENABLED_DEFAULT);
    registerUserConfigurableSetting(CATEGORY_USB, USB_AGENT_POLLING_TIME_IN_SEC, USBArmyKnifeCapability::SettingType::Int16, (int16_t)USB_AGENT_POLLING_TIME_IN_SEC_DEFAULT);

    agentPollingEnabled = prefs.getBool(USB_AGENT_POLLING_ENABLED, USB_AGENT_POLLING_ENABLED_DEFAULT);
    agentPollingIntervalInSec = prefs.getInt(USB_AGENT_POLLING_TIME_IN_SEC, USB_AGENT_POLLING_TIME_IN_SEC_DEFAULT);

    Devices::USB::CDC.setCallback(HostCommand::AgentStatus, [this](uint8_t *buffer, const size_t size) -> void
    {
        if (!m_agentIsConnected)
        {
            Debug::Log.info(LOG_AGENT, "Agent has responded with status");
            m_agentIsConnected = true;
            m_machineName = std::string((char*)buffer, size);
        }
    });
    Devices::USB::CDC.setCallback(HostCommand::ExecuteResult, [this](uint8_t *buffer, const size_t size) -> void
    {
        // todo rather that just putting it in the log we should find a nice home for it
        // for now use the log but limit the size
        const std::string msg = "Agent execution result\n";
        Debug::Log.info(LOG_AGENT, msg + std::string((char*)buffer, size + msg.length() > MAX_LOG_ENTRY_IN_CHARS ? MAX_LOG_ENTRY_IN_CHARS - msg.length() : size));
    });
}

void SerialAgent::loop(Preferences &prefs)
{
    if (agentPollingEnabled && m_agentIsConnected == false)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= agentPollingIntervalInSec * 1000) {
            Devices::USB::CDC.writeBinary(HostCommand::RequestAgentStatus, nullptr, 0);
            previousMillis = currentMillis;
        }
    }
}

void SerialAgent::run(const std::string& cmd)
{
    Devices::USB::CDC.writeBinary(HostCommand::Execute, (uint8_t*)cmd.c_str(), cmd.length());
}