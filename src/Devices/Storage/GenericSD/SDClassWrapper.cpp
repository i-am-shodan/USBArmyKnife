#ifdef USE_SD_INTERFACE

#include "SDClassWrapper.h"

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

    return sdInitialized;
}

File SDClassWrapper::open(const char *path, const char *mode)
{
    if (!sdInitialized)
    {
        return File();
    }

    return SD.open(path, mode);
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