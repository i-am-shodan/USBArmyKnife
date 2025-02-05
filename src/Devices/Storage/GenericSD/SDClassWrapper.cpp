#ifdef USE_SD_INTERFACE

#include "SDClassWrapper.h"
#include "../../../Debug/Logging.h"
#include <Arduino.h>

using namespace fs;

static bool sdInitialized = false;

SDClassWrapper::SDClassWrapper(FSImplPtr impl) : FS(impl)
{
}

bool SDClassWrapper::begin(const bool &format)
{
    pinMode(SD_MISO_PIN, INPUT_PULLUP);
    SPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);

    sdInitialized = SD.begin(SD_CS_PIN);

    Debug::Log.info(" ", "SD init "+std::to_string(sdInitialized));

    uint32_t cardSize = SD.cardSize() / (1024 * 1024);
    Debug::Log.info(" ", "SDCard Size: " + std::to_string(cardSize));

    return sdInitialized;
}

File SDClassWrapper::open(const char *path, const char *mode)
{
    Debug::Log.error(" ", "Got here 2");

    if (!sdInitialized)
    {
        Debug::Log.error(" ", "SD no init ");
        return File();
    }

    auto file = SD.open(path, mode);

    if (!file) {
        Debug::Log.error(" ", std::string("File could not be opened ")+path);
    }
    else
    {
        Debug::Log.error(" ", std::string("File was opened! ")+path);
    }

    return file;
}

bool SDClassWrapper::exists(const char *path)
{
    if (!sdInitialized)
    {
        return false;
    }
    return SD.exists(path);
}

bool SDClassWrapper::remove(const char *path)
{
    if (!sdInitialized)
    {
        return false;
    }

    return SD.remove(path);
}

int SDClassWrapper::usedBytes()
{
    return 0;
}

int SDClassWrapper::totalBytes()
{
    if (!sdInitialized)
    {
        return 1;
    }

    return SD.cardSize();
}

FileImplPtr VFSImpl::open(const char *path, const char *mode, const bool create)
{
    SD.op

    Debug::Log.error(" ", "Got here 3");
    return FileImplPtr();
    //return SD.open(path, mode, create);
}

bool VFSImpl::exists(const char *path)
{
    return false;
}

bool VFSImpl::rename(const char *pathFrom, const char *pathTo)
{
    return false;
}

bool VFSImpl::remove(const char *path)
{
    return false;
}

bool VFSImpl::mkdir(const char *path)
{
    return false;
}

bool VFSImpl::rmdir(const char *path)
{
    return false;
}

void VFSImpl::mountpoint(const char *)
{
}

const char *mountpoint()
{
    return "/";
}

SDClassWrapper fs::SDCard = SDClassWrapper(FSImplPtr(new VFSImpl()));

#endif