#pragma once

#ifndef SDInterface_h
#define SDInterface_h

#include "configs.h"

#include "settings.h"
#ifdef USE_SD_MMC_INTERFACE
  #include "../../../src/Devices/Storage/ESP32/SDMMCFS2.h"
  #define SD  fs::SD_MMC_2
#elif USE_SD_INTERFACE
    // generic esp32 SD interface
    #include "SD.h"
    using namespace fs;
    #define FILE_INTERFACE SD
#elif USE_SPIFFS_INTERFACE
  #include <SPIFFS.h>
  #define SD SPIFFS
#else
  #include "SD.h"
#endif

#include "Buffer.h"
#ifdef HAS_SCREEN
  #include "Display.h"
#endif
#include <Update.h>

extern Buffer buffer_obj;
extern Settings settings_obj;
#ifdef HAS_SCREEN
  extern Display display_obj;
#endif

#ifdef KIT
  #define SD_DET 4
#endif

class SDInterface {

  private:
#if defined(MARAUDER_M5STICKC)
  SPIClass *spiExt;
#endif
    bool checkDetectPin();

  public:
    uint8_t cardType;
    //uint64_t cardSizeBT;
    //uint64_t cardSizeKB;
    uint64_t cardSizeMB;
    //uint64_t cardSizeGB;
    bool supported = false;

    String card_sz;
  
    bool initSD();

    LinkedList<String>* sd_files;

    void listDir(String str_dir);
    void listDirToLinkedList(LinkedList<String>* file_names, String str_dir = "/", String ext = "");
    File getFile(String path);
    void runUpdate();
    void performUpdate(Stream &updateSource, size_t updateSize);
    void main();
    bool removeFile(String file_path);
};

#endif
