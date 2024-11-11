#pragma once

#ifdef ARDUINO_ARCH_RP2040
#include <SD.h>

#include "FS.h"
#include "FSImpl.h"

namespace fs
{

    class SDClassWrapper : public fs::FS
    {
    public:
        SDClassWrapper(FSImplPtr impl);
        bool begin(const bool &);
        File open(const char *path, const char *mode);
        bool exists(const char *path);
        bool remove(const char *path);

        int usedBytes();
        int totalBytes();
    };

    class VFSImpl : public FSImpl
    {
    public:
        virtual bool setConfig(const FSConfig &cfg);
        virtual bool begin();
        virtual void end();
        virtual bool format();
        virtual bool info(FSInfo &info);
        virtual FileImplPtr open(const char *path, OpenMode openMode, AccessMode accessMode);
        virtual bool exists(const char *path);
        virtual DirImplPtr openDir(const char *path);
        virtual bool rename(const char *pathFrom, const char *pathTo);
        virtual bool remove(const char *path);
        virtual bool mkdir(const char *path);
        virtual bool rmdir(const char *path);
        virtual bool stat(const char *path, FSStat *st);
    };

    extern fs::SDClassWrapper SDCard;

}
#endif