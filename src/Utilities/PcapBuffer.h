#pragma once

#ifdef NO_SD
  #include <SPIFFS.h>
#elif ARDUINO_ARCH_RP2040
  #include <SD.h>
#else
  #include <SD_MMC.h>
#endif

#include <string>
#include <queue>

struct PcapBufferBlock {
  unsigned long time;
  uint8_t* buffer;
  uint32_t length;
}; 

class PcapBuffer {
  public:
    PcapBuffer(std::string file_name, fs::FS* fs, uint32_t dataLinkType = 1);
    ~PcapBuffer();
    bool append(uint8_t *packet, uint32_t len);
    void flushToDisk();
  private:
    fs::File m_file;
    std::queue<PcapBufferBlock> m_blocks;
};