// #include "Common.h"

// #include <vector>





// int32_t msc_raw_read(uint32_t lba, void *buffer, uint32_t bufsize)
// {
//     if (card == NULL)
//     {
//         return 0;
//     }

//     uint32_t count = (bufsize / card->csd.sector_size);
//     sdmmc_read_sectors(card, buffer, lba, count);
//     return bufsize;
// }

// int32_t msc_raw_write(uint32_t lba, uint8_t *buffer, uint32_t bufsize)
// {
//     if (card == NULL)
//     {
//         return 0;
//     }

//     uint32_t count = (bufsize / card->csd.sector_size);
//     sdmmc_write_sectors(card, buffer, lba, count);
//     return bufsize;
// }

// void msc_raw_flush()
// {
//     // nothing to do
// }

