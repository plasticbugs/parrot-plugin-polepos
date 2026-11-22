// ppvid: the board's picture, as MAME 0.288 draws it (polepos_v.cpp's
// screen_update), from pp.h's RAM. Portable C, used by the plugin and the
// Mac tests.
//
// The picture is MAME's visible area, 256 x 224 (the bitmap's lines 16 to
// 239), a colour a pixel: the index of one of the three colour PROMs' 128
// colours (ppvid_rgb), or that index through a map (ppvid_map, ppcv's
// TMS9918 colours). It is made a line at a time from a copy of the RAM the
// picture uses (6.6K), so a caller can make it after the VBLANK, a few
// lines at a time, and never hold the whole picture:
//
//   once:              ppvid_init(roms)
//   at a VBLANK:       ppvid_snap(&ram)        (the RAM as it is)
//   then:              ppvid_begin(&ram)       (the sprites' places, the text's cells)
//                      ppvid_line(y, out)      (any lines, y 0-223, 256 pixels)
//
// The layers, back to front, as MAME has them: the background tiles (the
// top half, scrolled by pp_hscroll), the road (the bottom half, from the
// road RAM and pp_vscroll), the 64 sprites zoomed by the scale PROM (0
// first, so 63 in front), then the alpha text.
#pragma once
#include <stdint.h>

#include "pp.h"

#define PPVID_W 256
#define PPVID_H 224
#define PPVID_TOP 16                    // MAME's bitmap line of the picture's line 0

#if defined(PICO_ON_DEVICE) && PICO_ON_DEVICE
#define PPVID_HOT __attribute__((section(".time_critical.ppvid")))
#else
#define PPVID_HOT
#endif

// what a picture is made from: the sprites' places and sizes (pp_sprite
// 0x380-0x3ff and 0x780-0x7ff), the road, alpha and background RAM, the
// scrolls and CHACL
typedef struct {
	uint16_t pos[128], size[128];
	uint16_t road[0x400], alpha[0x400], view[0x400];
	uint16_t hscroll, vscroll;
	uint8_t chacl;
} ppvid_ram_t;

void ppvid_init(const pp_roms_t *r);
// the colours the lines are made in: map[i] for PROM colour i (NULL: i itself)
void ppvid_map(const uint8_t *map);
void ppvid_snap(ppvid_ram_t *s);
void ppvid_begin(const ppvid_ram_t *s);
void ppvid_line(int y, uint8_t *out);
// the whole picture (tests): PPVID_W x PPVID_H
void ppvid_render(uint8_t *px);

// parts of a line, for callers that make the layers themselves (ppcv):
// the background or the road (every pixel), the sprites and the text
// (only what isn't see-through is written)
void ppvid_back(int y, uint8_t *out);
void ppvid_sprites(int y, uint8_t *out);
void ppvid_text(int y, uint8_t *out);
// the text without its letters (ppcv: the rows it draws larger elsewhere):
// each cell in its pen 0's colour, the colour around the letters, where
// that isn't see-through (the title's sand), else nothing (the scores over the sky)
void ppvid_text_under(int y, uint8_t *out);
// the text's 8 x 8 cells in tile row r (0-31) that draw anything, a bit a column
uint32_t ppvid_text_cells(int r);
// the sprites that draw on line y, a bit each (those that can't show left out)
uint64_t ppvid_line_sprites(int y);
// what road line y is made from (its colours and scroll; 0 for the background's lines)
uint32_t ppvid_line_key(int y);

// the 128 colours as 0xRRGGBB
extern uint32_t ppvid_rgb[128];
