#include "USBCore.h"

#include "USBMSC.h"

#include <Adafruit_TinyUSB.h>

#include "USBNCM.h"
#include "USBCDC.h"
#include "USBMSC.h"
#include "USBHID.h"

#include "../../Debug/Logging.h"
#define LOG_USB "USB"

#define USB_DeviceType "usbDeviceType"
#define USB_DeviceType_Default (uint16_t) USBDeviceType::Serial

#define USB_ClassType "usbClassType"
#define USB_ClassType_Default (uint16_t) USBClassType::HID

#define USB_DeviceVID "usbDeviceVID"
#define USB_DeviceVID_Default (uint16_t)0xcafe

#define USB_DevicePID "usbDevicePID"
#define USB_DevicePID_Default (uint16_t)0x403f

#define USB_Version "usbVersion"
#define USB_Version_Default (uint16_t)0x0101

#define USB_DeviceVersion "usbDeviceVersion"
#define USB_DeviceVersion_Default (uint16_t)0x0101

#define USB_DeviceManufacturer "usbDeviceManufacturer"
#define USB_DeviceManufacturer_Default "Espressif Systems"

#define USB_DeviceProductDescriptor "usbDeviceProductDescriptor"
#define USB_DeviceProductDescriptor_Default "TinyUSB Device"

std::string curUSBMode = "NONE";

extern Preferences prefs;

namespace Devices::USB
{
  USBCore Core;
  // devices
  USBCDCWrapper CDC;
  USBNCM NCM;

  // classes
  USBHID HID;
  USBMSC MSC;
}

USBCore::USBCore()
    : curDeviceType(USBDeviceType::None),
      curClassType(USBClassType::None)
{
  registerUserConfigurableSetting(CATEGORY_USB, USB_DeviceType, USBArmyKnifeCapability::SettingType::UInt16, USB_DeviceType_Default);
  registerUserConfigurableSetting(CATEGORY_USB, USB_ClassType, USBArmyKnifeCapability::SettingType::UInt16, USB_ClassType_Default);
  registerUserConfigurableSetting(CATEGORY_USB, USB_DeviceVID, USBArmyKnifeCapability::SettingType::UInt16, USB_DeviceVID_Default);
  registerUserConfigurableSetting(CATEGORY_USB, USB_DevicePID, USBArmyKnifeCapability::SettingType::UInt16, USB_DevicePID_Default);
  registerUserConfigurableSetting(CATEGORY_USB, USB_Version, USBArmyKnifeCapability::SettingType::UInt16, USB_Version_Default);
  registerUserConfigurableSetting(CATEGORY_USB, USB_DeviceVersion, USBArmyKnifeCapability::SettingType::UInt16, USB_DeviceVersion_Default);
  registerUserConfigurableSetting(CATEGORY_USB, USB_DeviceManufacturer, USBArmyKnifeCapability::SettingType::String, USB_DeviceManufacturer_Default);
  registerUserConfigurableSetting(CATEGORY_USB, USB_DeviceProductDescriptor, USBArmyKnifeCapability::SettingType::String, USB_DeviceProductDescriptor_Default);
}

void USBCore::changeUSBMode(DuckyInterpreter::USB_MODE &mode, const uint16_t &vidValue, const uint16_t &pidValue, const std::string &man, const std::string &prod, const std::string &serial)
{
  // There are two methods to set a USB device type
  // * Set boot configuration which set the USB device options as soon as we get power
  // * No not appear as a USB device when we are plugged in, let the script set the parameters

  if (curDeviceType != USBDeviceType::None)
  {
    Debug::Log.warning(LOG_USB, "Device started with USB Device type NOT set to None, this is recommended when using ATTACKMODE command");
  }

  auto startingMode = curDeviceType;

  if (mode == DuckyInterpreter::USB_MODE::OFF)
  {
    Debug::Log.info(LOG_USB, "Changing USB mode to OFF");
    end();
    tud_disconnect();
    curDeviceType = USBDeviceType::None;
    curClassType = USBClassType::None;
    return;
  }
  else if (mode & DuckyInterpreter::USB_MODE::HID)
  {
    Debug::Log.info(LOG_USB, "Changing USB mode to HID");
    curDeviceType = USBDeviceType::USBSerial;
    curClassType = USBClassType::HID;
  }
  else if (mode & DuckyInterpreter::USB_MODE::STORAGE)
  {
    Debug::Log.info(LOG_USB, "Changing USB mode to STORAGE");
    curDeviceType = USBDeviceType::USBSerial;
    curClassType = USBClassType::Storage;
  }

  Preferences prefs;
  prefs.begin("usbarmyknife");

  // these functions are happy with being called twice
  Devices::USB::CDC.begin(115200);
  Devices::USB::NCM.begin(prefs);
  Devices::USB::HID.begin(prefs);
  Devices::USB::MSC.begin(prefs);

  if (startingMode == USBDeviceType::None && mode != DuckyInterpreter::USB_MODE::OFF)
  {
    vid = vidValue == 0 ? prefs.getUShort(USB_DeviceVID, USB_DeviceVID_Default) : vidValue;
    pid = pidValue == 0 ? prefs.getUShort(USB_DevicePID, USB_DevicePID_Default) : pidValue;

    // we are moving from USB off to USB on, ensure USB strings are set correctly
    TinyUSBDevice.setID(vid, pid);
    TinyUSBDevice.setManufacturerDescriptor(man.c_str());
    TinyUSBDevice.setProductDescriptor(prod.c_str());
    TinyUSBDevice.setSerialDescriptor(serial.c_str());
  }

  if (mode & DuckyInterpreter::USB_MODE::STORAGE)
  {
    Devices::USB::MSC.mountSD();
  }
}

void USBCore::begin(Preferences &prefs)
{
  curDeviceType = (USBDeviceType)prefs.getUShort(USB_DeviceType, USB_DeviceType_Default);
  curClassType = (USBClassType)prefs.getUShort(USB_ClassType, USB_ClassType_Default);

  // The device class coming from an NCM device isn't currently compatible when running in NCM mode
  // As such we disable HID etc when running in NCM
  if (curDeviceType == USBDeviceType::NCM)
  {
    curClassType = USBClassType::None;
  }

  vid = prefs.getUShort(USB_DeviceVID, USB_DeviceVID_Default);
  pid = prefs.getUShort(USB_DevicePID, USB_DevicePID_Default);

  Devices::USB::CDC.begin(115200);
  Devices::USB::NCM.begin(prefs);
  Devices::USB::HID.begin(prefs);
  Devices::USB::MSC.begin(prefs);

  // You can only set these values after the USB stack has been official started, otherwise
  // the values get overwriten by defaults
  if (curDeviceType != USBDeviceType::None)
  {
    TinyUSBDevice.setID(vid, pid); // USB_PID

    const uint16_t version = prefs.getUShort(USB_Version, USB_Version_Default);
    TinyUSBDevice.setVersion(version);

    const uint16_t deviceVersion = prefs.getUShort(USB_DeviceVersion, USB_DeviceVersion_Default);
    TinyUSBDevice.setVersion(deviceVersion);

    const String manufacturerArdString = prefs.getString(USB_DeviceManufacturer, USB_DeviceManufacturer_Default);
    manufacturer = std::string(manufacturerArdString.c_str());
    TinyUSBDevice.setManufacturerDescriptor(manufacturer.c_str());

    const String productArdString = prefs.getString(USB_DeviceProductDescriptor, USB_DeviceProductDescriptor_Default);
    product = std::string(productArdString.c_str());
    TinyUSBDevice.setProductDescriptor(product.c_str());
  }
}

void USBCore::reset()
{
  if (TinyUSBDevice.ready() && TinyUSBDevice.mounted())
  {
    Debug::Log.info(LOG_USB, "USB reset");
    if (!TinyUSBDevice.detach())
    {
      Debug::Log.info(LOG_USB, "Detach failed");
    }

    tud_disconnect();
    delay(100);
    tud_connect();

    TinyUSB_Device_Init(0, curDeviceType == USBDeviceType::NCM);

    TinyUSBDevice.attach();
  }
}

void USBCore::loop(Preferences &prefs)
{
#ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
#endif

  Devices::USB::NCM.loop(prefs);
  Devices::USB::HID.loop(prefs);
  Devices::USB::MSC.loop(prefs);
  Devices::USB::CDC.loop(prefs);
}

void USBCore::end()
{
  Devices::USB::CDC.end();
  Devices::USB::NCM.end();
  // Devices::USB::HID.end();
  Devices::USB::MSC.end();
}