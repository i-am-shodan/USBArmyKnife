#pragma once

#include "../../USBArmyKnifeCapability.h"
#include "IStorage.h"

class HardwareStorage : IStorage, USBArmyKnifeCapability {
public:
  HardwareStorage();

  virtual void loop(Preferences& prefs);
  virtual void begin(Preferences& prefs);

  bool isRunning() { return running; }
  virtual uint8_t usedPercentage();
  virtual std::vector<std::string> listFiles();
  virtual size_t getFileSize(const std::string& filename);
  virtual bool doesFileExist(const std::string& filename);
  virtual bool createEmptyFile(const std::string &filename);
  virtual uint8_t* readFileAsBinary(const std::string& filename);
  virtual std::string readLineFromFile(const std::string &filename, const int lineNumber);
  virtual void writeFileData(const std::string& filename, const uint8_t *buffer, const size_t size);
  virtual std::string readFile(fs::FS &fs, const char *path);
  virtual bool deleteFile(const std::string& filename);
private:
  bool running = false;
};

namespace Devices
{
    extern HardwareStorage Storage;
}