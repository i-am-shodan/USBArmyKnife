#include "HardwareStorage.h"
#include "../TFT/HardwareTFT.h"
#include "../../Debug/Logging.h"
#include "../../pin_config.h"

#include "SDMMCFS2.h"
using namespace fs;

#define LOG_MMC "MMC"

static uint8_t cachedCapacity = 0;
static std::vector<std::string> filesCache;

namespace Devices
{
    HardwareStorage Storage;
}

std::string HardwareStorage::readFile(fs::FS &fs, const char *path)
{
    File file = fs.open(path);
    if (!file)
    {
        return "";
    }
    std::string line;
    while (file.available())
    {
        line = line + (char)file.read();
    }
    return line;
}

void HardwareStorage::writeFileData(const std::string& filename, const uint8_t *buffer, const size_t size)
{
    File file = SD_MMC_2.open(filename.c_str(), FILE_WRITE);
    if (!file)
    {
        Debug::Log.info(LOG_MMC, "Could not open file");
        return;
    }
    
    if (file.write(buffer, size) != size)
    {
        Debug::Log.info(LOG_MMC, "write err");
    }

    file.close();
}

std::size_t HardwareStorage::getFileSize(const std::string& filename)
{
    File file = SD_MMC_2.open(filename.c_str());
    if (!file)
    {
        Debug::Log.info(LOG_MMC, "Could not open file: " + filename);
        return 0;
    }

    // Get the file size
    int ret = file.size();
    file.close();
    return ret;
}

bool HardwareStorage::doesFileExist(const std::string& filename)
{
    return SD_MMC_2.exists(filename.c_str());
}

uint8_t* HardwareStorage::readFileAsBinary(const std::string& filename)
{
    File file = SD_MMC_2.open(filename.c_str());
    if (!file)
    {
        Debug::Log.info(LOG_MMC, "Could not open file: " + filename);
        return 0;
    }

    // Get the file size
    size_t fileSize = file.size();

    // Allocate memory for the file content
    uint8_t *fileContent = (uint8_t *)malloc(fileSize + 1); // +1 for null terminator
    if (!fileContent)
    {
        Debug::Log.info(LOG_MMC, "Could not alloc");
        file.close();
        return 0;
    }

    // Read the entire file into the buffer
    if (file.read((uint8_t *)fileContent, fileSize) != fileSize)
    {
        Debug::Log.info(LOG_MMC, "Could not read");
        free(fileContent);
        file.close();
        return 0;
    }

    // Null-terminate the buffer
    fileContent[fileSize] = '\0';

    return fileContent;
}

std::string HardwareStorage::readLineFromFile(const std::string &filename, const int lineNumber)
{
    File file = SD_MMC_2.open(filename.c_str());
    if (!file)
    {
        Debug::Log.info(LOG_MMC, "Could not open file: " + filename);
        return "";
    }

    int currentLine = 0;
    String line;
    while (file.available())
    {
        line = file.readStringUntil('\n'); // todo read up to EOF
        if (currentLine == lineNumber)
        {
            break;
        }
        line = "";
        currentLine++;
    }

    std::string ret = std::string(line.c_str());

    if (ret.length() == 1 && ret[0] == '\n')
    {
        return "";
    }
    if (ret.length() == 2 && ret[0] == '\r' && ret[1] == '\n')
    {
        return "";
    }

    return ret;
}

static void listDir(std::vector<std::string> &files, fs::FS &fs, const char *dirname)
{
    File root = fs.open(dirname);
    if (!root)
    {
        return;
    }
    if (!root.isDirectory())
    {
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            listDir(files, fs, file.path());
        }
        else
        {
            std::string filename = std::string(file.name());
            files.emplace_back("/" + filename);
        }
        file = root.openNextFile();
    }
}

std::vector<std::string> HardwareStorage::listFiles()
{
    if (filesCache.size() != 0)
    {
        return filesCache;
    }

    if (SD_MMC_2.cardType() != CARD_NONE)
    {
        listDir(filesCache, SD_MMC_2, "/");
    }

    return filesCache;
}

uint8_t HardwareStorage::usedPercentage()
{
    if (cachedCapacity != 0)
    {
        return cachedCapacity;
    }
    cachedCapacity = (SD_MMC_2.usedBytes() / SD_MMC_2.totalBytes()) * 100;
    return cachedCapacity;
}

bool HardwareStorage::createEmptyFile(const std::string &filename)
{
    File file = SD_MMC_2.open(filename.c_str(), FILE_WRITE);
    if (!file)
    {
        return false;
    }
    file.close();
    filesCache.clear();
    return true;
}

bool HardwareStorage::deleteFile(const std::string& filename)
{
    filesCache.clear();
    return SD_MMC_2.remove(filename.c_str());
}

HardwareStorage::HardwareStorage()
{
    SD_MMC = SD_MMC_2;
}

void HardwareStorage::loop(Preferences &prefs)
{
}

void HardwareStorage::begin(Preferences &prefs)
{
    SD_MMC_2.setPins(SD_MMC_CLK_PIN, SD_MMC_CMD_PIN, SD_MMC_D0_PIN, SD_MMC_D1_PIN, SD_MMC_D2_PIN, SD_MMC_D3_PIN);
    if (!SD_MMC_2.begin("/sdcard", false, false, SDMMC_FREQ_52M))
    {
        Devices::TFT.clearScreen();
        Devices::TFT.display(0, 0, "Could not start SD driver");
        Debug::Log.info(LOG_MMC, "SD_MMC_2 could not be started");
    }
    else
    {
        uint8_t cardType = SD_MMC_2.cardType();
        if (cardType == CARD_NONE)
        {
            Devices::TFT.clearScreen();
            Devices::TFT.display(0, 0, "Could not find SD card");
            Debug::Log.info(LOG_MMC, "Could not find SD Card");
        }
        else
        {
            running = true;
        }
    }
}