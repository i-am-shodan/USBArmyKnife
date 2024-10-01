#include "USBMSC.h"

#include <Adafruit_TinyUSB.h>

#include "../../Devices/Storage/SDMMCFS2.h"
using namespace fs;

#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

#include "../../pin_config.h"

#include "../../Debug/Logging.h"
#define TAG_USB "USB"

#define LOGICAL_BLOCK_SIZE 512
static Adafruit_USBD_MSC usb_msc;
static sdmmc_card_t *card = NULL; // raw card access

static File mscFile;

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
static int32_t msc_read_cb(uint32_t lba, void *buffer, uint32_t bufsize)
{
    Devices::USB::MSC.setActivityStateState(true);

    if (card == NULL)
    {
        mscFile.seek(lba * LOGICAL_BLOCK_SIZE);
        if (mscFile.readBytes((char *)buffer, bufsize) == bufsize)
        {
            // bufSize should always be a multiple of LOGICAL_BLOCK_SIZE so on read success return that
            return bufsize;
        }
    }
    else
    {
        uint32_t sectors = (bufsize / card->csd.sector_size);
        if (sdmmc_read_sectors(card, buffer, lba, sectors) == ESP_OK)
        {
            // bufSize should always be a multiple of LOGICAL_BLOCK_SIZE so on read success return that
            return bufsize;
        }
    }

    return -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
static int32_t msc_write_cb(uint32_t lba, uint8_t *buffer, uint32_t bufsize)
{
    Devices::USB::MSC.setActivityStateState(true);

    if (card == NULL)
    {
        // writing to file images is not currently supported, we lie and say the
        // write occured as that will make it look like everything is ok to the OS
        // fs cache manager
        return bufsize;
    }
    else
    {
        uint32_t count = (bufsize / card->csd.sector_size);
        return sdmmc_write_sectors(card, buffer, lba, count) == ESP_OK ? bufsize : -1;
    }
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
static void msc_flush_cb()
{
    if (card == NULL)
    {
        mscFile.flush();
    }
}

static size_t open_msc(const char *path)
{
    mscFile = SD_MMC_2.open(path);
    if (!mscFile)
    {
        return 0;
    }

    return mscFile.size();
}

static size_t getinternalMMCSectorSize()
{
    if (card == NULL)
    {
        return 0;
    }
    return card->csd.sector_size;
}

bool USBMSC::mountSD()
{
    if (card == NULL)
    {
        card = SD_MMC_2.getCard();

        // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
        usb_msc.setID("Adafruit", "Mass Storage", "1.0");

        auto sectorSize = getinternalMMCSectorSize();

        usb_msc.setCapacity(card->csd.capacity, sectorSize);

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
    else
    {
        Debug::Log.info(TAG_USB, "Device already mounted");
        return false;
    }
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

bool USBMSC::mountDiskImage(const std::string &imageLocation)
{
    std::size_t size = open_msc(imageLocation.c_str());

    if (size != 0)
    {
        // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
        usb_msc.setID("Adafruit", "Mass Storage", "1.0");

        usb_msc.setCapacity(size / LOGICAL_BLOCK_SIZE, LOGICAL_BLOCK_SIZE);

        // Set callback
        usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

        // Set Lun ready (disk is always ready)
        usb_msc.setUnitReady(true);

        Devices::USB::Core.reset();

        if (!usb_msc.begin())
        {
            Debug::Log.info(TAG_USB, "Failed to start USB MSC");
            return false;
        }

        Debug::Log.info(TAG_USB, "Disk image mounted");
        return true;
    }
    else
    {
        Debug::Log.info(TAG_USB, "Could not load " + imageLocation);
        return false;
    }
}