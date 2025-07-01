#ifndef NO_IR
#include "HardwareIR.h"
#include "../../Debug/Logging.h"
#include "../../Attacks/Ducky/DuckyPayload.h"
#include "../../Devices/Storage/HardwareStorage.h"

#include <string>
#include <unordered_map>
#include <functional>

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <DuckyParse.h>

#define TAG "IR"

static IRsend irsend(IR_PIN);
static decode_type_t typeOfSignal = decode_type_t::UNKNOWN;
static uint32_t value = 0;
static uint16_t repeats = 0;

static const uint16_t* raw = nullptr;
static uint16_t sizeOfRaw = 0;
static uint16_t freq = 0;

static bool calibrated = false;

namespace Devices
{
    HardwareIR IR;
}

// Requires a 29630Hz carrier
// https://github.com/CR-DMcDonald/IRExitSensors/blob/main/ds-k7p03.md
static const uint16_t DSK7P03[] = {
    // on for 337500uS
    60000, 0,
    60000, 0,
    60000, 0,
    60000, 0,
    60000, 0,
    37500, 0,
    // off for 366900uS
    0, 60000,
    0, 60000,
    0, 60000,
    0, 60000,
    0, 60000,
    0, 60000,
    0, 6900
};

HardwareIR::HardwareIR()
{
}

void HardwareIR::begin(Preferences &prefs)
{
    irsend.begin();

    // NEC1: {D=4,F=8} = IR_NEC 4 8 10
    Attacks::Ducky.registerExtension("IR_NEC", [this](const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
    {
        const std::string arg = str.substr(str.find(' ') + 1);

        const auto entries = Ducky::SplitString(arg);

        if (entries.size() != 3)
        {
            Debug::Log.info(TAG, "Not enough params to IR_NEC");
        }
        else
        {
            const auto address = atoi(entries[0].c_str());
            const auto command = atoi(entries[1].c_str());
            const auto repeat = atoi(entries[2].c_str());
    
            value = irsend.encodeNEC(address, command);

            repeats = repeat;
            typeOfSignal = decode_type_t::NEC;

            Debug::Log.info(TAG, "Set IR_NEC cmd");
        }
        return true;
    });

    Attacks::Ducky.registerExtension("IR_DOOR_DSK7P03", [this](const std::string &str, const std::unordered_map<std::string, std::string> &constants, const std::unordered_map<std::string, int> &variables)
    {
        raw = &(DSK7P03[0]);
        sizeOfRaw = sizeof(DSK7P03)/sizeof(uint16_t);
        freq = 29630;
        repeats = 10;
        typeOfSignal = decode_type_t::RAW;
        return true;
    });

    Debug::Log.info(TAG, "IR set up");
}

void HardwareIR::loop(Preferences &prefs)
{
    if (typeOfSignal == decode_type_t::NEC)
    {
        irsend.sendNEC(value, 32, repeats);
        typeOfSignal = decode_type_t::UNKNOWN;
        Debug::Log.info(TAG, "Sent code");
    }
    else if (typeOfSignal == decode_type_t::RAW && raw != nullptr)
    {
        for (int x = 0; x < repeats; x++)
        {
            irsend.sendRaw(raw, sizeOfRaw, freq);
        }
        typeOfSignal = decode_type_t::UNKNOWN;
        Debug::Log.info(TAG, "Sent raw");
    }
}
#endif