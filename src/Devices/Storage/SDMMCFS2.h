#ifndef NO_SD
#pragma once

#include <SD_MMC.h>

namespace fs {

class SDMMCFS2 : public SDMMCFS {
public:
  sdmmc_card_t* getCard() { return _card; }
};

extern fs::SDMMCFS2 SD_MMC_2;

}
#endif