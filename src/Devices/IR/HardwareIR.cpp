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
static bool calibrated = false;

namespace Devices
{
    HardwareIR IR;
}

HardwareIR::HardwareIR()
{
}

void HardwareIR::begin(Preferences &prefs)
{
    irsend.begin();

    // IR_NEC 4 8 10
    Attacks::Ducky.registerExtension("IR_NEC", [this](const std::string& str, const std::unordered_map<std::string, std::string>& constants, const std::unordered_map<std::string, int>& variables)
    {
        const std::string arg = str.substr(str.find(' ') + 1);

        const auto entries = Ducky::SplitString(arg);

        if (entries.size() != 3)
        {
            Debug::Log.info(TAG, "Not enough params to IR_NEC");
        }
        else
        {
            auto address = atoi(entries[0].c_str());
            auto command = atoi(entries[1].c_str());
            auto repeat = atoi(entries[2].c_str());

            Debug::Log.info(TAG, "Parsed as "+std::to_string(address) + " " + std::to_string(command) + " " + std::to_string(repeat));
    
            value = irsend.encodeNEC(address, command);

            typeOfSignal = decode_type_t::NEC;
            repeats = atoi(entries[3].c_str());
        }
        return true;
    });

    Attacks::Ducky.registerExtension("IR_PRONTO", [this](const std::string& str, const std::unordered_map<std::string, std::string>& constants, const std::unordered_map<std::string, int>& variables)
    {
        // is it a bad idea to send here?
        const std::string filename = str.substr(str.find(' ') + 1);

        do
        {
            size_t len = Devices::Storage.getFileSize(filename);

            if (len == 0)
            {
                Debug::Log.info(TAG, "The filename was invalid: "+filename);
                break;
            }

            uint8_t* data = Devices::Storage.readFileAsBinary(filename);
            if (data == nullptr)
            {
                break;
            }

            uint16_t* pronto = (uint16_t*) data;
            irsend.sendPronto(pronto, len / 2, 10);

            Debug::Log.info(TAG, "Sent queued");

        } while (false);
        
        return true;
    });

    Debug::Log.info(TAG, "IR set up");
}

void HardwareIR::loop(Preferences &prefs)
{
    if (!calibrated)
    {
        noInterrupts();
        irsend.calibrate();
        interrupts();
        calibrated = true;
        Debug::Log.info(TAG, "IR calibrated");
    }

    if (typeOfSignal == decode_type_t::NEC)
    {
        noInterrupts();
        irsend.sendNEC(value, 32, repeats);
        interrupts();
        typeOfSignal = decode_type_t::UNKNOWN;
        Debug::Log.info(TAG, "Sent code");
    }
}
#endif