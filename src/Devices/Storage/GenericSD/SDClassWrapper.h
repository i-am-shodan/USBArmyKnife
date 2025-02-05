#pragma once

#ifdef USE_SD_INTERFACE
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
        virtual FileImplPtr open(const char *path, const char *mode, const bool create);
        virtual bool exists(const char *path);
        virtual bool rename(const char *pathFrom, const char *pathTo);
        virtual bool remove(const char *path);
        virtual bool mkdir(const char *path);
        virtual bool rmdir(const char *path);
        void mountpoint(const char *);
        const char *mountpoint();
    };

    extern fs::SDClassWrapper SDCard;

}
#endif