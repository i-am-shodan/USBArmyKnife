#pragma once

#include <string>
#include <cstdint>

#ifdef NO_SD
  #ifdef ARDUINO_ARCH_RP2040
    #include <LittleFS.h>
  #else
    #include <SPIFFS.h>
  #endif
#elif ARDUINO_ARCH_RP2040
  #include <SD.h>
#else
  #include <SD_MMC.h>
#endif

class IStorage {
public:
  virtual ~IStorage() {} // Virtual destructor (important for polymorphism)

  virtual uint8_t usedPercentage() = 0;
  virtual std::vector<std::string> listFiles() = 0;
  virtual size_t getFileSize(const std::string& filename) = 0;
  virtual bool doesFileExist(const std::string& filename) = 0;
  virtual bool createEmptyFile(const std::string &filename) = 0;
  virtual uint8_t* readFileAsBinary(const std::string& filename) = 0;
  virtual std::string readLineFromFile(const std::string &filename, const int lineNumber) = 0;
  virtual void writeFileData(const std::string& filename, const uint8_t *buffer, const size_t size) = 0;
  virtual std::string readFile(fs::FS &fs, const char *path) = 0;
};