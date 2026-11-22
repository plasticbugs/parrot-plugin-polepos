// pproms.h -- where the ROMs go: MAME's "polepos" set and the four Namco
// MCUs (51xx.bin to 54xx.bin, which MAME keeps in namco51.zip to
// namco54.zip), each file known by its size and CRC-32, never by its name.
// Portable: the plugin and the Mac tests share it. (Pac-Man's pmroms.h.)
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "pp.h"

typedef struct {
    const char *name;                   // MAME's name, for messages
    uint32_t size, crc;
    uint32_t at;                        // where its bytes go in the store
} pp_rom_file_t;

#define PP_ROM_FILES 40                 // (not pp1-13, pp1-14 or pp1-4, which MAME doesn't use)
#define PP_ROMS_GAME ((1ull << 36) - 1) // the set's files
#define PP_ROMS_ALL  ((1ull << PP_ROM_FILES) - 1)
extern const pp_rom_file_t pp_rom_files[PP_ROM_FILES];

typedef struct {
    uint8_t maincpu[0x3000], sub1[0x4000], sub2[0x4000], mcu[4][0x400];
    uint8_t chars[0x1000], tiles[0x1000], sprites[0x4000], bigsprites[0x10000];
    uint8_t road[0x5000], scalelut[0x1000], proms[0x1040], wave[0x100], engine[0x4000], speech[0x8000];
} pp_rom_store_t;
// the store: a static one unless the plugin points it at the PSRAM (its
// 212K are more than SRAM can spare; pp_init copies the CPUs' programs out)
extern pp_rom_store_t *pp_rom_store;

uint32_t pp_crc32(uint32_t crc, const uint8_t *p, uint32_t n);
// which of the files still to find (not in found) has this size and CRC-32 (-1: none)
int pproms_wanted(uint32_t size, uint32_t crc, uint64_t found);
bool pproms_size_ok(uint32_t size);     // one of the files' sizes
void pproms_put(int i, const uint8_t *data);
// the board's ROMs from the store
void pproms_roms(pp_roms_t *r);
