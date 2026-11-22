// ppcard.h -- the ROMs on the card, through fsro: MAME's polepos.zip and
// namco51.zip to namco54.zip as they are, or the loose files, in /POLEPOS,
// or /POLEPOS.ZIP. Each file is known by its size and CRC-32 (pproms.c),
// never its name. Portable: the plugin and a Mac test share it. (Pac-Man's
// pmcard.c.)
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "../sdplayer/fsro.h"
#include "pproms.h"

#define PPCARD_HOME "/POLEPOS"
#define PPCARD_ZIP  "/POLEPOS.ZIP"

// Every file in PPCARD_HOME (zips or loose), then PPCARD_ZIP, until all
// are found: each ROM to its place in pp_rom_store. A zip is never read
// whole: its directory, then each wanted entry's packed bytes into buf and
// unpacked after them. buf: max bytes (the directory, and a packed ROM with
// the ROM after it, must fit); tick: called between sectors (the plugin
// serves the console), or NULL. Returns pproms' found bits.
uint64_t ppcard_load_roms(fs_t *fs, uint8_t *buf, uint32_t max, void (*tick)(void));
