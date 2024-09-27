#include "USBMSC.h"

#include <Adafruit_TinyUSB.h>

#include "SD_MMC.h"

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

    mscFile.seek(lba * LOGICAL_BLOCK_SIZE);
    return mscFile.readBytes((char *)buffer, bufsize) >= 0 ? LOGICAL_BLOCK_SIZE : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
static int32_t msc_write_cb(uint32_t lba, uint8_t *buffer, uint32_t bufsize)
{
    Devices::USB::MSC.setActivityStateState(true);

    mscFile.seek(lba * LOGICAL_BLOCK_SIZE);
    return mscFile.write(buffer, bufsize) >= 0 ? LOGICAL_BLOCK_SIZE : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
static void msc_flush_cb()
{
    // nothing to do
    mscFile.flush();
}

static size_t open_msc(const char *path)
{
    mscFile = SD_MMC.open(path);
    if (!mscFile)
    {
        return 0;
    }

    return mscFile.size();
}

static size_t open_internal_msc()
{
    SD_MMC.end();

    sdmmc_host_t host = {
        .flags = SDMMC_HOST_FLAG_4BIT | SDMMC_HOST_FLAG_DDR,
        .slot = SDMMC_HOST_SLOT_1,
        .max_freq_khz = SDMMC_FREQ_DEFAULT,
        .io_voltage = 3.3f,
        .init = &sdmmc_host_init,
        .set_bus_width = &sdmmc_host_set_bus_width,
        .get_bus_width = &sdmmc_host_get_slot_width,
        .set_bus_ddr_mode = &sdmmc_host_set_bus_ddr_mode,
        .set_card_clk = &sdmmc_host_set_card_clk,
        .do_transaction = &sdmmc_host_do_transaction,
        .deinit = &sdmmc_host_deinit,
        .io_int_enable = sdmmc_host_io_int_enable,
        .io_int_wait = sdmmc_host_io_int_wait,
        .command_timeout_ms = 0,
    };
    sdmmc_slot_config_t slot_config = {
        .clk = (gpio_num_t)SD_MMC_CLK_PIN,
        .cmd = (gpio_num_t)SD_MMC_CMD_PIN,
        .d0 = (gpio_num_t)SD_MMC_D0_PIN,
        .d1 = (gpio_num_t)SD_MMC_D1_PIN,
        .d2 = (gpio_num_t)SD_MMC_D2_PIN,
        .d3 = (gpio_num_t)SD_MMC_D3_PIN,
        .cd = SDMMC_SLOT_NO_CD,
        .wp = SDMMC_SLOT_NO_WP,
        .width = 4, // SDMMC_SLOT_WIDTH_DEFAULT,
        .flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP,
    };

    gpio_set_pull_mode((gpio_num_t)SD_MMC_CMD_PIN, GPIO_PULLUP_ONLY); // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode((gpio_num_t)SD_MMC_D0_PIN, GPIO_PULLUP_ONLY);  // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode((gpio_num_t)SD_MMC_D1_PIN, GPIO_PULLUP_ONLY);  // D1, needed in 4-line mode only
    gpio_set_pull_mode((gpio_num_t)SD_MMC_D2_PIN, GPIO_PULLUP_ONLY);  // D2, needed in 4-line mode only
    gpio_set_pull_mode((gpio_num_t)SD_MMC_D3_PIN, GPIO_PULLUP_ONLY);  // D3, needed in 4- and 1-line modes

    const char mount_point[] = "/sdcard";
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {.format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024};

    auto mountret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (mountret != ESP_OK || card == NULL)
    {
        return 0;
    }

    return card->csd.capacity;
}

static size_t getinternalMMCSectorSize()
{
    if (card == NULL)
    {
        return 0;
    }
    return card->csd.sector_size;
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

        // Set Lun ready (RAM disk is always ready)
        usb_msc.setUnitReady(true);

        usb_msc.begin();

        if (TinyUSBDevice.mounted())
        {
            TinyUSBDevice.detach();
            delay(10);
            TinyUSBDevice.attach();
        }

        Debug::Log.info(TAG_USB, "Disk image mounted");
        return true;
    }
    else
    {
        Debug::Log.info(TAG_USB, "Could not load "+imageLocation);
        return false;
    }
}