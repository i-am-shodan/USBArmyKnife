#pragma once

#define BTN_PIN     0

#define NUM_LEDS        1
#define LED_DI_PIN     40
#define LED_CI_PIN     39

#define SD_MMC_D0_PIN  14
#define SD_MMC_D1_PIN  17
#define SD_MMC_D2_PIN  21
#define SD_MMC_D3_PIN  18
#define SD_MMC_CLK_PIN 12
#define SD_MMC_CMD_PIN 16

// Display (ST7735s) hardware configuration:
#define DISPLAY_RST      1
#define DISPLAY_DC       2
#define DISPLAY_MOSI     3
#define DISPLAY_CS       4
#define DISPLAY_SCLK     5
#define DISPLAY_LEDA    38
#define DISPLAY_MISO    -1
#define DISPLAY_BUSY    -1
#define DISPLAY_WIDTH  160
#define DISPLAY_HEIGHT  80
#define TFT_WIDTH       80
#define TFT_HEIGHT      160