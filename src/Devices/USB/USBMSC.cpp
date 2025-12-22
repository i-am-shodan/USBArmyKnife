#include "USBMSC.h"

#include <Adafruit_TinyUSB.h>

#include "../../Devices/Storage/HardwareStorage.h"
#include "../../Debug/Logging.h"
#define TAG_USB "USB"
#define LOGICAL_BLOCK_SIZE 512

static Adafruit_USBD_MSC usb_msc;
static File mscFile;

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
static int32_t msc_read_cb(uint32_t lba, void *buffer, uint32_t bufsize)
{
    Devices::USB::MSC.setActivityStateState(true);

    if (mscFile && strlen(mscFile.name()) != 0)
    {
        if (!mscFile.seek(lba * LOGICAL_BLOCK_SIZE))
        {
            Debug::Log.error(TAG_USB, "Disk seek error");
            return -1;
        }
        
        if (mscFile.readBytes((char *)buffer, bufsize) == bufsize)
        {
            // bufSize should always be a multiple of LOGICAL_BLOCK_SIZE so on read success return that
            return bufsize;
        }
        else
        {
            Debug::Log.error(TAG_USB, "Could not read data from file");
        }
    }
    else if (Devices::Storage.isRawAccessSupported())
    {
        uint32_t sectors = (bufsize / Devices::Storage.sectorSize());
        return Devices::Storage.readRawSectors((uint8_t*)buffer, lba, sectors);
    }
    else
    {
        Debug::Log.error(TAG_USB, "No disk data to read");
    }
    return -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
static int32_t msc_write_cb(uint32_t lba, uint8_t *buffer, uint32_t bufsize)
{
    Devices::USB::MSC.setActivityStateState(true);

    if (mscFile && strlen(mscFile.name()) != 0)
    {
        // writing to file images is not currently supported, we lie and say the
        // write occured as that will make it look like everything is ok to the OS
        // fs cache manager
        return bufsize;
    }
    else if (Devices::Storage.isRawAccessSupported())
    {
        uint32_t sectors = (bufsize / Devices::Storage.sectorSize());
        return Devices::Storage.writeRawSectors((uint8_t*)buffer, lba, sectors);
    }
    else
    {
        Debug::Log.error(TAG_USB, "Could not write");
    }
    return -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
static void msc_flush_cb()
{
    if (strlen(mscFile.name()) != 0)
    {
        mscFile.flush();
    }
    else
    {
        Devices::Storage.flush();
    }
}

static size_t open_msc(const char *path)
{
    mscFile = Devices::Storage.openFile(path, "r");
    if (strlen(mscFile.name()) == 0)
    {
        return 0;
    }

    return mscFile.size();
}

bool USBMSC::mountSD()
{
    if (!Devices::Storage.isRawAccessSupported())
    {
        Debug::Log.error(TAG_USB, "Raw SD card access is not supported on this device");
        return false;
    }

    if (mscFile.available())
    {
        Debug::Log.info(TAG_USB, "Device already mounted");
        return false;
    }

    // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
    usb_msc.setID("Adafruit", "Mass Storage", "1.0");

    auto capacity = Devices::Storage.deviceCapacity();
    auto sectorSize = Devices::Storage.sectorSize();

    usb_msc.setCapacity(capacity, sectorSize);

    // Set callback
    usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

    // Set Lun ready (disk is always ready)
    usb_msc.setUnitReady(true);

    // FIXME: This shouldn't cause an issue but the device doesn't appear
    Devices::USB::Core.reset();

    if (!usb_msc.begin())
    {
        Debug::Log.info(TAG_USB, "Failed to start USB MSC");
        return false;
    }

    Debug::Log.info(TAG_USB, "SD mounted");
    return true;
}

USBMSC::USBMSC()
{
}

void USBMSC::begin(Preferences &prefs)
{
}

void USBMSC::loop(Preferences &prefs)
{
}

void USBMSC::end()
{
    if (!mscFile)
    {
        usb_msc.setUnitReady(false);
        mscFile.flush();
    }
}

bool USBMSC::mountDiskImage(const std::string &imageLocation, bool mountAsCD)
{
#ifdef ARDUINO_ARCH_RP2040
    if (mountAsCD)
    {
        Debug::Log.error(TAG_USB, "CDROM support is not supported on this device");
        return false;
    }
#endif

    std::size_t size = open_msc(imageLocation.c_str());

    if (size != 0)
    {
#ifndef ARDUINO_ARCH_RP2040
        usb_msc.setCDROM(mountAsCD);
#endif

        // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
        if (mountAsCD)
        {
            usb_msc.setID("Adafruit", "USB CDROM", "1.0");
        }
        else
        {
            usb_msc.setID("Adafruit", "Mass Storage", "1.0");
        }

        usb_msc.setCapacity(size / LOGICAL_BLOCK_SIZE, LOGICAL_BLOCK_SIZE);

        // Set callback
        usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

        // Set Lun ready (disk is always ready)
        usb_msc.setUnitReady(true);

        if (!usb_msc.begin())
        {
            Debug::Log.info(TAG_USB, "Failed to start USB MSC");
            return false;
        }

        Devices::USB::Core.reset();

        Debug::Log.info(TAG_USB, mountAsCD ? "CDROM image mounted" : "Disk image mounted");
        return true;
    }
    else
    {
        Debug::Log.error(TAG_USB, "Could not load " + imageLocation);
        return false;
    }
}