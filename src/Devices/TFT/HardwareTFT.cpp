#ifndef NO_TFT
#include "HardwareTFT.h"

#include "../../Debug/Logging.h"

#include <LovyanGFX.hpp>
#include <PNGdec.h>

#define LOG_TFT "TFT"

// In the future if you are trying to add another board configuration here it would make sense
// To have multiple lgfx::LGFX_Device objects wrapped in ifdefs rather that they single instance we have now
// Right now we are using DISPLAY_TYPE_ST7735S to refer to a device instead of a panel. As such if a new
// board with a ST7735S comes on board we might not have the right x,y offsets
class LGFX_Panel : public lgfx::LGFX_Device
{
#ifdef DISPLAY_TYPE_ST7735S
    lgfx::Panel_ST7735S _panel_instance;
#elif DISPLAY_TYPE_ST7789
    lgfx::Panel_ST7789 _panel_instance;
#else
    #error Invalid display type // Full list here: https://github.com/lovyan03/LovyanGFX/tree/master/src/lgfx/v1/panel
#endif
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;

public:
    LGFX_Panel(void)
    {
        {
            auto cfg = _bus_instance.config();

            cfg.spi_host = SPI3_HOST;          // SPI2_HOST is in use by the RGB led
            cfg.spi_mode = 0;                  // Set SPI communication mode (0 ~ 3)
            cfg.freq_write = 27000000;         // SPI clock when sending (max 80MHz, rounded to 80MHz divided by an integer)
            cfg.freq_read = 16000000;          // SPI clock when receiving
            cfg.spi_3wire = true;              // Set true when receiving on the MOSI pin
            cfg.use_lock = false;              // Set true when using transaction lock
            cfg.dma_channel = SPI_DMA_CH_AUTO; // Set the DMA channel to use (0=not use DMA / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=auto setting)

            cfg.pin_sclk = DISPLAY_SCLK; // set SPI SCLK pin number
            cfg.pin_mosi = DISPLAY_MOSI; // Set MOSI pin number for SPI
            cfg.pin_miso = DISPLAY_MISO; // Set MISO pin for SPI (-1 = disable)
            cfg.pin_dc = DISPLAY_DC;     // Set SPI D/C pin number (-1 = disable)

            _bus_instance.config(cfg);              // Apply the setting value to the bus.
            _panel_instance.setBus(&_bus_instance); // Sets the bus to the panel.
        }

        {
            auto cfg = _panel_instance.config(); // Obtain the structure for display panel settings.

            cfg.pin_cs = DISPLAY_CS;     // Pin number to which CS is connected (-1 = disable)
            cfg.pin_rst = DISPLAY_RST;   // pin number where RST is connected (-1 = disable)
            cfg.pin_busy = DISPLAY_BUSY; // pin number to which BUSY is connected (-1 = disable)

            cfg.panel_width = DISPLAY_HEIGHT; // actual displayable width. Note: width/height swapped due to the rotation
            cfg.panel_height = DISPLAY_WIDTH; // Actual displayable height Note: width/height swapped due to the rotation
#ifdef DISPLAY_TYPE_ST7735S
            cfg.offset_x = 26;                // Panel offset in X direction
            cfg.offset_y = 1;                 // Y direction offset amount of the panel
            cfg.dummy_read_pixel = 8;         // Number of bits for dummy read before pixel read
            cfg.dummy_read_bits = 1;          // Number of dummy read bits before non-pixel data read
#elif  DISPLAY_TYPE_ST7789
            cfg.offset_x = 34;                // Panel offset in X direction
            cfg.offset_y = -0;                 // Y direction offset amount of the panel
            cfg.dummy_read_pixel = 8;         // Number of bits for dummy read before pixel read
            cfg.dummy_read_bits = 1;          // Number of dummy read bits before non-pixel data read          
#endif
            cfg.offset_rotation = 1;          // Rotation direction value offset 0~7 (4~7 are upside down)
            cfg.readable = true;              // set to true if data can be read
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false; // Set to true for panels that transmit data length in 16-bit units with 16-bit parallel or SPI
            cfg.bus_shared = true;  // If the bus is shared with the SD card, set to true (bus control with drawJpgFile etc.)

            // Please set the following only when the display is shifted with a driver with a variable number of pixels such as ST7735 or ILI9163.
#ifdef DISPLAY_TYPE_ST7735S
            cfg.memory_width = 132;  // Maximum width supported by driver IC
            cfg.memory_height = 160; // Maximum height supported by driver IC
#endif
            _panel_instance.config(cfg);
        }

        {
            auto cfg = _light_instance.config();

            cfg.pin_bl = DISPLAY_LEDA; // pin number to which the backlight is connected
            cfg.invert = true;         // true to invert backlight brightness
            cfg.freq = 12000;          // Backlight PWM frequency
            cfg.pwm_channel = 7;       // PWM channel number to use

            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        setPanel(&_panel_instance);
    }
};

static LGFX_Panel lcd;
static PNG png; // PNG decoder instance
static int16_t xpos = 0;
static int16_t ypos = 0;

namespace Devices
{
    HardwareTFT TFT;
}

static void pngDraw(PNGDRAW *pDraw)
{
    uint16_t lineBuffer[TFT_HEIGHT];
    png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
    lcd.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}

void HardwareTFT::display(const int &x, const int &y, const std::string &str)
{
    lcd.setCursor(x, y);
    lcd.setTextColor(TFT_WHITE);
    lcd.println(str.c_str());
    lcd.display();
}

void HardwareTFT::clearScreen()
{
    lcd.clear(TFT_BLACK);
    lcd.display();
}

void HardwareTFT::powerOff()
{
    lcd.clearDisplay();
    lcd.setBrightness(0);
    lcd.sleep();
}

void HardwareTFT::displayPng(HardwareStorage &storage, const std::string &filename)
{
    xpos = 0;
    ypos = 0;
    size_t size = storage.getFileSize(filename);

    if (size == 0)
    {
        Debug::Log.info(LOG_TFT, "invalid file size");
    }

    uint8_t *data = storage.readFileAsBinary(filename);

    if (data == NULL)
    {
        Debug::Log.info(LOG_TFT, "invalid data");
        return;
    }

    int16_t rc = png.openFLASH((uint8_t *)data, size, pngDraw);
    if (rc == PNG_SUCCESS)
    {
        rc = png.decode(NULL, 0);
        lcd.display();
    }
    else
    {
        Debug::Log.info(LOG_TFT, "Could not write PNG");
    }

    free(data);
}

HardwareTFT::HardwareTFT()
{
}

void HardwareTFT::loop(Preferences &prefs)
{
}

void HardwareTFT::begin(Preferences &prefs)
{
    lcd.init();
    lcd.setBrightness(128);
    lcd.clear(TFT_BLACK);
    lcd.display();
}
#endif