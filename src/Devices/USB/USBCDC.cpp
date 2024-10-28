#include <HardwareSerial.h>
#include <unordered_map>

#include "USBCDC.h"
#include "../../Debug/Logging.h"

#include <Adafruit_TinyUSB.h>

#define MAX_SERIAL_BUFFER_SIZE 2048
#define MAX_SERIAL_READ_ERRORS 5
#define SIZEOF_TAG 1
#define SIZEOF_LENGTH 4
#define LOG_CDC "CDC"

static uint8_t *serialPortRecvBuffer = nullptr;
static std::unordered_map<HostCommand, std::function<void(uint8_t *, size_t)>> callbacks;

#define USB_SERIALRAW "usbSerialRaw"
#define USB_SERIALRAW_DEFAULT false
static bool usbRawMode = USB_SERIALRAW_DEFAULT;
static bool usbInitCalled = false;
static uint8_t errorCount = 0;

USBCDCWrapper::USBCDCWrapper()
{
    registerUserConfigurableSetting(CATEGORY_USB, USB_SERIALRAW, USBArmyKnifeCapability::SettingType::Bool, USB_SERIALRAW_DEFAULT);
}

void USBCDCWrapper::setCallback(const HostCommand &tag, std::function<void(uint8_t *, const size_t)> callback)
{
    callbacks[tag] = callback;
}

void USBCDCWrapper::begin(const unsigned long &baud)
{
    // ensure this can be called multiple times without error
    if (Devices::USB::Core.currentDeviceType() != USBDeviceType::Serial)
    {
        return;
    }

    if (usbInitCalled == false)
    {
        TinyUSB_Device_Init(0, false);
        usbInitCalled = true;
    }

    if (serialPortRecvBuffer == nullptr)
    {
        serialPortRecvBuffer = (uint8_t *)malloc(MAX_SERIAL_BUFFER_SIZE);
    }
    Serial.begin(baud);
}

void USBCDCWrapper::begin(Preferences &prefs)
{
    usbRawMode = prefs.getBool(USB_SERIALRAW, USB_SERIALRAW_DEFAULT);

    begin(115200);
}

void USBCDCWrapper::loop(Preferences &prefs)
{
    if (Devices::USB::Core.currentDeviceType() == USBDeviceType::Serial && errorCount <= MAX_SERIAL_READ_ERRORS && Serial.available())
    {
        bool success = false;

        do
        {
            if (Serial.readBytes(serialPortRecvBuffer, SIZEOF_TAG + SIZEOF_LENGTH) != SIZEOF_TAG + SIZEOF_LENGTH) // tag + length
            {
                // read error
                Debug::Log.error(LOG_CDC, "Read error");
                break;
            }

            if (serialPortRecvBuffer[0] == HostCommand::MIN || serialPortRecvBuffer[0] >= HostCommand::MAX)
            {
                // invalid data
                Debug::Log.error(LOG_CDC, "Invalid data tag error");
                break;
            }

            const HostCommand tag = (HostCommand)serialPortRecvBuffer[0];

            const auto it = callbacks.find(tag);
            if (it == callbacks.cend())
            {
                Debug::Log.error(LOG_CDC, "Invalid command " + std::to_string(tag));
                break;
            }

            const uint32_t len = *((uint32_t *)(serialPortRecvBuffer + 1));

            if (len > MAX_SERIAL_BUFFER_SIZE)
            {
                // invalid buffer size
                Debug::Log.info(LOG_CDC, "Buffer size error" + std::to_string(len));
                break;
            }

            int totalRead = 0;
            while (totalRead != len)
            {
                int read = Serial.read(serialPortRecvBuffer + totalRead, len - totalRead);
                totalRead += read;
            }

            callbacks[tag](serialPortRecvBuffer, totalRead);
            success = true;

        } while (false);

        if (!success)
        {
            errorCount++;
            if (errorCount >= MAX_SERIAL_READ_ERRORS)
            {
                Debug::Log.error(LOG_CDC, "Too many serial read error occured, monitoring has stopped");
            }
        }
    }
}

void USBCDCWrapper::writeBinary(const HostCommand &tag, const uint8_t *buffer, const size_t &size)
{
    if (usbRawMode)
    {
        Debug::Log.info(LOG_CDC, "Cannot write binary, incorrect mode");
        return;
    }

    if (serialPortRecvBuffer == nullptr || errorCount >= MAX_SERIAL_READ_ERRORS)
    {
        Debug::Log.info(LOG_CDC, "Cannot write binary, not started");
        return;
    }

    Serial.write((uint8_t)tag);        // tag
    Serial.write((uint8_t *)&size, 4); // length
    if (size != 0)
    {
        Serial.write(buffer, size); // value
    }
}

void USBCDCWrapper::writeDebugString(const std::string &msg)
{
    // be careful about calling debug log here as you could get into a loop

    if (usbRawMode)
    {
        Serial.println(msg.c_str());
    }
    else if (serialPortRecvBuffer == nullptr)
    {
        return;
    }
    else
    {
        writeBinary(HostCommand::DebugMsg, (uint8_t *)msg.c_str(), msg.length());
    }
}

void USBCDCWrapper::end()
{
    if (serialPortRecvBuffer != nullptr)
    {
        Serial.end();
    }
}