#include "USBHID.h"

#include <Adafruit_TinyUSB.h>
#include <queue>

#include "../../Debug/Logging.h"
#define TAG_USB "USB"
#define HID_WAIT_TIME 10

// Report ID
enum
{
    RID_KEYBOARD = 1,
    RID_MOUSE,
    RID_CONSUMER_CONTROL, // Media, volume etc ..
};

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] =
    {
        TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(RID_KEYBOARD)),
        TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(RID_MOUSE)),
        TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(RID_CONSUMER_CONTROL))};

struct KEY_EVENT
{
    uint8_t Keycode[6] = {0};
    uint8_t Modifiers = 0;
    uint8_t ID = 0;
};

struct MOUSE_EVENT
{
    int8_t xDelta = 0;
    int8_t yDelta = 0;
};

static std::queue<KEY_EVENT> eventsToProcess;
static std::queue<std::pair<uint8_t, uint16_t>> cdcEventsToProcess;
static std::queue<MOUSE_EVENT> mouseEventsToProcess;
Adafruit_USBD_HID *usb_hid = nullptr;

USBHID::USBHID()
{
}

void USBHID::begin(Preferences &prefs)
{
    if (Devices::USB::Core.currentDeviceType() != USBDeviceType::None && Devices::USB::Core.currentClassType() == USBClassType::HID && usb_hid == nullptr)
    {
        // need to start USB HID
        usb_hid = new Adafruit_USBD_HID(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false);
        // Set up HID
        usb_hid->setPollInterval(2);
        usb_hid->setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
        usb_hid->setStringDescriptor("HID Composite");
        usb_hid->begin();

        if (TinyUSBDevice.mounted())
        {
            TinyUSBDevice.detach();
            delay(10);
            TinyUSBDevice.attach();
        }
    }
}

bool USBHID::IsQueueEmpty()
{
    return eventsToProcess.empty();
}

void USBHID::keyboard_press(const uint8_t modifiers, const uint8_t key1, const uint8_t key2, const uint8_t key3, const uint8_t key4, const uint8_t key5, const uint8_t key6)
{
    if (usb_hid == nullptr)
    {
        return;
    }

    KEY_EVENT event;
    event.ID = RID_KEYBOARD;
    event.Keycode[0] = key1;
    event.Keycode[1] = key2;
    event.Keycode[2] = key3;
    event.Keycode[3] = key4;
    event.Keycode[4] = key5;
    event.Keycode[5] = key6;
    event.Modifiers = modifiers;

    eventsToProcess.push(event);
}

void USBHID::keyboard_release()
{
    if (usb_hid == nullptr)
    {
        return;
    }

    KEY_EVENT event;
    event.ID = RID_KEYBOARD;
    event.Keycode[0] = 0;
    event.Modifiers = 0;

    eventsToProcess.push(event);
}

void USBHID::consumer_device_keypress(const uint16_t keyCode)
{
    if (usb_hid == nullptr)
    {
        return;
    }

    cdcEventsToProcess.push(std::pair<uint8_t, uint16_t>(RID_CONSUMER_CONTROL, keyCode));
    cdcEventsToProcess.push(std::pair<uint8_t, uint16_t>(RID_CONSUMER_CONTROL, 0));
}

void USBHID::mouseMove(int8_t xDelta, int8_t yDelta)
{
    if (usb_hid == nullptr)
    {
        return;
    }

    MOUSE_EVENT event;
    event.xDelta = xDelta;
    event.yDelta = yDelta;

    mouseEventsToProcess.push(event);
}

void USBHID::loop(Preferences &prefs)
{
    if (usb_hid == nullptr)
    {
        return;
    }

    // not enumerated()/mounted() yet: nothing to do
    if (!TinyUSBDevice.mounted())
    {
        return;
    }

    if (!eventsToProcess.empty() || !cdcEventsToProcess.empty() || !mouseEventsToProcess.empty())
    {
        // Remote wakeup
        if (TinyUSBDevice.suspended())
        {
            // Wake up host if we are in suspend mode
            // and REMOTE_WAKEUP feature is enabled by host
            TinyUSBDevice.remoteWakeup();
        }

        if (usb_hid->ready())
        {
            if (!cdcEventsToProcess.empty())
            {
                Debug::Log.info(TAG_USB, "CDC report");
                if (usb_hid->sendReport16(cdcEventsToProcess.front().first, cdcEventsToProcess.front().second))
                {
                    cdcEventsToProcess.pop();
                    delay(HID_WAIT_TIME);
                }
            }
            else if (!mouseEventsToProcess.empty())
            {
                auto report = mouseEventsToProcess.front();
                bool sent = usb_hid->mouseMove(RID_MOUSE, report.xDelta, report.yDelta);

                if (sent)
                {
                    mouseEventsToProcess.pop();
                    delay(HID_WAIT_TIME);
                }
                else
                {
                    Debug::Log.error(TAG_USB, "Send data mouse failed");
                }
            }
            else
            {
                bool sent = false;
                auto report = eventsToProcess.front();
                if (report.Modifiers == 0 && report.Keycode[0] == 0)
                {
                    sent = usb_hid->keyboardRelease(report.ID);
                }
                else
                {
                    sent = usb_hid->keyboardReport(report.ID, report.Modifiers, report.Keycode);
                }
                
                if (sent)
                {
                    eventsToProcess.pop();
                    delay(HID_WAIT_TIME);
                }
                else
                {
                    Debug::Log.error(TAG_USB, "Send data keyboard failed");
                }
            }
        }
    }
}