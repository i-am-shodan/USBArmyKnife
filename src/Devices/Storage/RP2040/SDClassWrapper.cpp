#ifdef ARDUINO_ARCH_RP2040

#include "SDClassWrapper.h"

using namespace fs;

static bool sdInitialized = false;

SDClassWrapper::SDClassWrapper(FSImplPtr impl) : FS(impl)
{
}

bool SDClassWrapper::begin(const bool &format)
{
    SPI.setRX(SD_MISO);
    SPI.setTX(SD_MOSI);
    SPI.setSCK(SD_SCLK);
    SPI.setMOSI(SD_MOSI);
    SPI.setMISO(SD_MISO);

    sdInitialized = SD.begin(SD_CS, 5000000, SPI);

    return sdInitialized;
}

File SDClassWrapper::open(const char *path, const char *mode)
{
    return SD.open(path, mode);
}

bool SDClassWrapper::exists(const char *path)
{
    return SD.exists(path);
}

bool SDClassWrapper::remove(const char *path)
{
    return SD.remove(path);
}

int SDClassWrapper::usedBytes()
{
    return 0;
}

int SDClassWrapper::totalBytes()
{
    return SD.size();
}

bool VFSImpl::setConfig(const FSConfig &cfg)
{
    return false;
}

bool VFSImpl::begin()
{
    return false;
}

void VFSImpl::end()
{
}

bool VFSImpl::format()
{
    return false;
}

bool VFSImpl::info(FSInfo &info)
{
    return false;
}

FileImplPtr VFSImpl::open(const char *path, OpenMode openMode, AccessMode accessMode)
{
    return FileImplPtr();
}

bool VFSImpl::exists(const char *path)
{
    return false;
}

DirImplPtr VFSImpl::openDir(const char *path)
{
    return DirImplPtr();
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

bool VFSImpl::stat(const char *path, FSStat *st)
{
    return false;
}

SDClassWrapper fs::SDCard = SDClassWrapper(FSImplPtr(new VFSImpl()));

#endif