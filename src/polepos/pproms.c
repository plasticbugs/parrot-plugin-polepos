// pproms.c -- see pproms.h. The sizes and CRC-32s are MAME 0.288's
// (polepos.cpp's polepos, namco51.cpp to namco54.cpp).
#include "pproms.h"

#include <stddef.h>
#include <string.h>

#ifdef PPROMS_PSRAM
pp_rom_store_t *pp_rom_store;           // the plugin's: in the PSRAM
#else
static pp_rom_store_t store;
pp_rom_store_t *pp_rom_store = &store;
#endif

#define S(f) offsetof(pp_rom_store_t, f)
const pp_rom_file_t pp_rom_files[PP_ROM_FILES] = {
    {"pp3_9.6h", 0x2000, 0xc0511173u, S(maincpu)},
    {"pp1_10b.5h", 0x1000, 0x7174bcb7u, S(maincpu) + 0x2000},
    {"pp3_2.8l", 0x2000, 0xfafb9049u, S(sub1)},          // the even bytes (the words' high halves)
    {"pp3_1.8m", 0x2000, 0x65c1c2c2u, S(sub1) + 0x2000}, // the odd bytes
    {"pp3_6.4l", 0x2000, 0xacc1ebc3u, S(sub2)},
    {"pp3_5.4m", 0x2000, 0x46e5c99au, S(sub2) + 0x2000},
    {"pp3_28.1f", 0x1000, 0x2e77187eu, S(chars)},
    {"pp1_29.1e", 0x1000, 0x706e888au, S(tiles)},
    {"pp3_25.1n", 0x2000, 0xb52c086bu, S(sprites)},
    {"pp3_26.1m", 0x2000, 0xd24a5707u, S(sprites) + 0x2000},
    {"pp1_17.5n", 0x2000, 0x2e134b46u, S(bigsprites)},
    {"pp1_19.4n", 0x2000, 0x43ff83e1u, S(bigsprites) + 0x2000},
    {"pp1_21.3n", 0x2000, 0x5f958eb4u, S(bigsprites) + 0x4000},
    {"pp1_18.5m", 0x2000, 0x6f9997d2u, S(bigsprites) + 0x8000},
    {"pp1_20.4m", 0x2000, 0xec18075bu, S(bigsprites) + 0xa000},
    {"pp1_22.3m", 0x2000, 0x1d2f30b1u, S(bigsprites) + 0xc000},
    {"pp1_30.3a", 0x2000, 0xee6b3315u, S(road)},
    {"pp1_31.2a", 0x2000, 0x6d1e7042u, S(road) + 0x2000},
    {"pp1_32.1a", 0x1000, 0x4e97f101u, S(road) + 0x4000},
    {"pp1_27.1l", 0x1000, 0xa61bff15u, S(scalelut)},
    {"pp1-7.8l", 0x0100, 0xf07ff2adu, S(proms)},
    {"pp1-8.9l", 0x0100, 0xadbde7d7u, S(proms) + 0x100},
    {"pp1-9.10l", 0x0100, 0xddac786au, S(proms) + 0x200},
    {"pp2-10.2h", 0x0100, 0x1e8d0491u, S(proms) + 0x300},
    {"pp1-11.4d", 0x0100, 0x0e4fe8a0u, S(proms) + 0x400},
    {"pp1-15.9a", 0x0100, 0x2d502464u, S(proms) + 0x500},
    {"pp1-16.10a", 0x0100, 0x027aa62cu, S(proms) + 0x600},
    {"pp1-17.11a", 0x0100, 0x1f8d0df3u, S(proms) + 0x700},
    {"pp1-12.3c", 0x0400, 0x7afc7cfcu, S(proms) + 0x800},
    {"pp3-6.6m", 0x0400, 0x63fb6057u, S(proms) + 0xc00},
    {"pp1-5.3b", 0x0100, 0x8568deccu, S(wave)},
    {"pp1_15.6a", 0x2000, 0xb5ad4d5fu, S(engine)},
    {"pp1_16.5a", 0x2000, 0x8fdd2f6fu, S(engine) + 0x2000},
    {"pp2_11.2e", 0x2000, 0x5b4cf05eu, S(speech)},
    {"pp2_12.2f", 0x2000, 0x32b694c2u, S(speech) + 0x2000},
    {"pp2_13.1e", 0x2000, 0x8842138au, S(speech) + 0x4000},
    {"51xx.bin", 0x0400, 0xc2f57ef8u, S(mcu[0])},
    {"52xx.bin", 0x0400, 0x3257d11eu, S(mcu[1])},
    {"53xx.bin", 0x0400, 0xb326fecbu, S(mcu[2])},
    {"54xx.bin", 0x0400, 0xee7357e0u, S(mcu[3])},
};
#undef S

uint32_t pp_crc32(uint32_t crc, const uint8_t *p, uint32_t n) {
    static const uint32_t t[16] = {0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4,
                                   0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
                                   0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c};
    crc = ~crc;
    while (n--) {
        crc ^= *p++;
        crc = (crc >> 4) ^ t[crc & 15];
        crc = (crc >> 4) ^ t[crc & 15];
    }
    return ~crc;
}

int pproms_wanted(uint32_t size, uint32_t crc, uint64_t found) {
    for (int i = 0; i < PP_ROM_FILES; i++)
        if (pp_rom_files[i].size == size && pp_rom_files[i].crc == crc && !(found & 1ull << i)) return i;
    return -1;
}

bool pproms_size_ok(uint32_t size) {
    for (int i = 0; i < PP_ROM_FILES; i++)
        if (pp_rom_files[i].size == size) return true;
    return false;
}

void pproms_put(int i, const uint8_t *data) { memcpy((uint8_t *)pp_rom_store + pp_rom_files[i].at, data, pp_rom_files[i].size); }

void pproms_roms(pp_roms_t *r) {
    pp_rom_store_t *s = pp_rom_store;
    r->maincpu = s->maincpu;
    r->sub1_hi = s->sub1; r->sub1_lo = s->sub1 + 0x2000;
    r->sub2_hi = s->sub2; r->sub2_lo = s->sub2 + 0x2000;
    for (int m = 0; m < 4; m++) r->mcu[m] = s->mcu[m];
    r->chars = s->chars; r->tiles = s->tiles; r->sprites = s->sprites; r->bigsprites = s->bigsprites;
    r->road = s->road; r->scalelut = s->scalelut; r->proms = s->proms; r->wave = s->wave;
    r->engine = s->engine; r->speech = s->speech;
}
