#ifndef UF2FORMAT_H
#define UF2FORMAT_H 1

#include <libopencm3/stm32/flash.h>

#include "uf2cfg.h"
#include "target.h"
#include "config.h"

#include <stdint.h>
#include <stdbool.h>

// All entries are little endian.

#define UF2_MAGIC_START0 0x0A324655UL // "UF2\n"
#define UF2_MAGIC_START1 0x9E5D5157UL // Randomly selected
#define UF2_MAGIC_END 0x0AB16F30UL    // Ditto

// If set, the block is "comment" and should not be flashed to the device
#define UF2_FLAG_NOFLASH 0x00000001
#define UF2_FLAG_FAMILYID_PRESENT 0x00002000

#define UF2_IS_MY_FAMILY(bl) \
    (((bl)->flags & UF2_FLAG_FAMILYID_PRESENT) && (bl)->familyID == UF2_FAMILY)

#define MAX_BLOCKS (FLASH_SIZE_OVERRIDE / 256 + 100)
typedef struct {
    uint32_t numBlocks;
    uint32_t numWritten;
    uint8_t writtenMask[MAX_BLOCKS / 8 + 1];
} WriteState;

typedef struct {
    // 32 byte header
    uint32_t magicStart0;
    uint32_t magicStart1;
    uint32_t flags;
    uint32_t targetAddr;
    uint32_t payloadSize;
    uint32_t blockNo;
    uint32_t numBlocks;
    uint32_t familyID;

    // raw data;
    uint8_t data[476];

    // store magic also at the end to limit damage from partial block reads
    uint32_t magicEnd;
} UF2_Block;

int write_block(uint32_t lba, const uint8_t *copy_from);
int read_block(uint32_t block_no, uint8_t *data);
void ghostfat_1ms(void);

static inline bool is_uf2_block(const void *data) {
    const UF2_Block *bl = (const UF2_Block *)data;
    return bl->magicStart0 == UF2_MAGIC_START0 && bl->magicStart1 == UF2_MAGIC_START1 &&
           bl->magicEnd == UF2_MAGIC_END;
}

#endif
