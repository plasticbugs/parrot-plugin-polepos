// ppvid.c -- see ppvid.h. polepos_v.cpp's palette, tilemaps, road and
// zoom_sprite, a line at a time; each step is MAME's, in MAME's order.
#include "ppvid.h"

#include <string.h>

#define CLEAR 0xFF                      // a see-through pen

static const pp_roms_t *R;
static const ppvid_ram_t *S;            // the picture's RAM
uint32_t ppvid_rgb[128];
static uint8_t alpha_idx[128][4], spr_idx[128][16], bg_idx[64][4], road_idx[16][64];    // PROM colours (CLEAR: see-through)
static uint8_t alpha_lut[128][4], spr_lut[128][16], bg_lut[64][4], road_lut[16][64];    // the same through the map
static uint16_t vpm[256];               // the vertical position modifier PROMs, 12 bits
static uint32_t pens4[256];             // a 2-plane ROM byte's 4 pixels, a byte each: bit 7-x << 1 | bit 3-x
static uint8_t char_pens[256];          // the pens (a bit each) a character uses
static uint8_t alpha_clear[128];        // the pens (a bit each) a text colour doesn't show
static uint16_t spr_pens[2][128];       // the pens a sprite uses (small, big)
static uint16_t spr_clear[128];         // the pens a sprite colour doesn't show
static uint32_t road_steps[256][2];     // a road ROM byte's set bits among 7 to 9 - j, a byte each, for pixels j 0-7

void ppvid_init(const pp_roms_t *r) {
	R = r;
	const uint8_t *p = r->proms;
	for (int i = 0; i < 128; i++) {
		uint32_t c = 0;
		for (int g = 0; g < 3; g++) {
			int v = p[g * 0x100 + i];
			int k = 0x0e * (v & 1) + 0x1f * (v >> 1 & 1) + 0x43 * (v >> 2 & 1) + 0x8f * (v >> 3 & 1);
			c = c << 8 | (uint32_t)k;
		}
		ppvid_rgb[i] = c;
	}
	// alpha: 0x20 + v, the 128V half 0x60 + v; 15 is 0x2f, the tilemap's see-through
	for (int c = 0; c < 128; c++) {
		alpha_clear[c] = 0;
		for (int k = 0; k < 4; k++) {
			int v = p[0x300 + (c & 63) * 4 + k];
			alpha_idx[c][k] = v == 15 ? CLEAR : (uint8_t)((c & 64 ? 0x60 : 0x20) + v);
			if (v == 15) alpha_clear[c] |= (uint8_t)(1u << k);
		}
	}
	for (int c = 0; c < 64; c++)
		for (int k = 0; k < 4; k++) bg_idx[c][k] = p[0x400 + c * 4 + k];
	// sprites: 0x10 + v, the 128V half 0x50 + v; 15 is 0x1f, zoom_sprite's see-through
	for (int c = 0; c < 128; c++) {
		spr_clear[c] = 0;
		for (int k = 0; k < 16; k++) {
			int v = p[0xc00 + (c & 63) * 16 + k];
			spr_idx[c][k] = v == 15 ? CLEAR : (uint8_t)((c & 64 ? 0x50 : 0x10) + v);
			if (v == 15) spr_clear[c] |= (uint16_t)(1u << k);
		}
	}
	for (int c = 0; c < 16; c++)
		for (int k = 0; k < 64; k++) road_idx[c][k] = (uint8_t)(0x40 + p[0x800 + c * 64 + k]);
	for (int i = 0; i < 256; i++) vpm[i] = (uint16_t)(p[0x500 + i] + (p[0x600 + i] << 4) + (p[0x700 + i] << 8));
	for (int b = 0; b < 256; b++) {
		uint32_t w = 0;
		for (int x = 0; x < 4; x++) w |= (uint32_t)((b >> (7 - x) & 1) << 1 | (b >> (3 - x) & 1)) << (8 * x);
		pens4[b] = w;
	}
	for (int c = 0; c < 256; c++) {
		uint8_t m = 0;
		for (int k = 0; k < 16; k++) {
			uint32_t w = pens4[r->chars[c * 16 + k]];
			for (int x = 0; x < 4; x++) m |= (uint8_t)(1u << (w >> (8 * x) & 3));
		}
		char_pens[c] = m;
	}
	for (int big = 0; big < 2; big++)
		for (int c = 0; c < 128; c++) {
			uint16_t m = 0;
			int n = big ? 256 : 64;
			const uint8_t *lo = big ? r->bigsprites + c * 256 : r->sprites + c * 64, *hi = lo + (big ? 0x8000 : 0x2000);
			for (int k = 0; k < n; k++) {
				uint32_t w = pens4[lo[k]] << 2 | pens4[hi[k]];
				for (int x = 0; x < 4; x++) m |= (uint16_t)(1u << (w >> (8 * x) & 15));
			}
			spr_pens[big][c] = m;
		}
	for (int b = 0; b < 256; b++) {
		uint8_t d[8] = {0, 0};
		for (int j = 2; j < 8; j++) d[j] = (uint8_t)(d[j - 1] + (b >> (9 - j) & 1));
		memcpy(road_steps[b], d, 8);
	}
	ppvid_map(NULL);
}

static uint8_t mapped(const uint8_t *map, uint8_t v) { return v == CLEAR || !map ? v : map[v]; }

void ppvid_map(const uint8_t *map) {
	for (int c = 0; c < 128; c++)
		for (int k = 0; k < 4; k++) alpha_lut[c][k] = mapped(map, alpha_idx[c][k]);
	for (int c = 0; c < 128; c++)
		for (int k = 0; k < 16; k++) spr_lut[c][k] = mapped(map, spr_idx[c][k]);
	for (int c = 0; c < 64; c++)
		for (int k = 0; k < 4; k++) bg_lut[c][k] = mapped(map, bg_idx[c][k]);
	for (int c = 0; c < 16; c++)
		for (int k = 0; k < 64; k++) road_lut[c][k] = mapped(map, road_idx[c][k]);
}

PPVID_HOT void ppvid_snap(ppvid_ram_t *s) {
	memcpy(s->pos, &pp_sprite[0x380], sizeof s->pos);
	memcpy(s->size, &pp_sprite[0x780], sizeof s->size);
	memcpy(s->road, pp_road, sizeof s->road);
	memcpy(s->alpha, pp_alpha, sizeof s->alpha);
	memcpy(s->view, pp_view, sizeof s->view);
	s->hscroll = pp_hscroll;
	s->vscroll = pp_vscroll;
	s->chacl = pp_chacl;
}

// ---- the sprites' places ------------------------------------------------------------
typedef struct {
	int16_t sx, sy;                     // MAME's sx, sy
	uint8_t sizex, sizey, code, color, big, fx;
} spr_t;
static spr_t spr[64];
static uint64_t on_line[256];           // bitmap line 0-255: the sprites on it, a bit each
static uint32_t text_cells[32];         // tile row: the cells that draw anything

PPVID_HOT void ppvid_begin(const ppvid_ram_t *s) {
	S = s;
	memset(on_line, 0, sizeof on_line);
	const uint16_t *pos = s->pos, *siz = s->size;
	for (int i = 0; i < 64; i++, pos += 2, siz += 2) {
		spr_t *o = &spr[i];
		o->sx = (int16_t)((pos[1] & 0x3ff) - 0x40 + 4);
		o->sy = (int16_t)(512 - (pos[0] & 0x1ff) + 1);     // buffered, a line late
		o->sizex = (uint8_t)((siz[1] & 0x3f00) >> 8);
		o->sizey = (uint8_t)((siz[0] & 0x3f00) >> 8);
		o->code = (uint8_t)(siz[0] & 0x7f);
		o->fx = (uint8_t)(siz[0] >> 7 & 1);
		o->color = (uint8_t)((siz[1] & 0x3f) | (o->sy >= 128 ? 0x40 : 0));    // 128V
		o->big = (uint8_t)(siz[0] >> 15);
		// none of it on the screen (its columns all past 255), or all of it see-through: left out
		int n = o->big ? 0x40 : 0x20, x0 = o->sx & 0x3ff, x1 = x0 + ((n - 1) * (1 + o->sizex) >> 6);
		if ((x0 > 0xff && x1 < 0x400) || !(spr_pens[o->big][o->code] & ~spr_clear[o->color])) continue;
		for (int y = 0; y <= o->sizey; y++) {
			int yy = (o->sy + y) & 0x1ff;
			if (yy >= 0x10 && yy < 0xf0) on_line[yy] |= 1ull << i;
		}
	}
	// CHACL off: the low bytes only, colour 0 (then 0x40 for the 128V half)
	for (int r = 2; r < 30; r++) {
		uint32_t m = 0;
		for (int c = 0; c < 32; c++) {
			uint16_t w = s->alpha[r * 32 + c];
			int color = (s->chacl ? (w & 0x3f00) >> 8 : 0) | (r >= 16 ? 0x40 : 0);
			if (char_pens[w & 0xff] & ~alpha_clear[color]) m |= 1u << c;
		}
		text_cells[r] = m;
	}
}

PPVID_HOT uint32_t ppvid_text_cells(int r) { return r >= 2 && r < 30 ? text_cells[r] : 0; }

PPVID_HOT uint64_t ppvid_line_sprites(int line) { return on_line[line + PPVID_TOP]; }

PPVID_HOT uint32_t ppvid_line_key(int line) {
	int y = line + PPVID_TOP;
	if (y < 128) return 0;
	int yoffs = ((vpm[y] + S->vscroll) >> 3) & 0x1ff;
	return (uint32_t)(S->road[yoffs] & 15) | (uint32_t)(S->road[0x380 + (y & 0x7f)] & 0x3ff) << 4;
}

// a sprite's line yy into out, as zoom_sprite draws it. MAME steps
// through its 0x40 (big) or 0x20 halves of pixels, each on the column it's
// at then, a column on each time the sizes add up past 0x40: half o is at
// x0 + (o * (1 + sizex) >> 6). Here a pixel at a time, its two halves in
// MAME's order, so the same pixels are written in the same order.
PPVID_HOT static void sprite_line(const spr_t *s, int yy, uint8_t *out) {
	int y = (yy - s->sy) & 0x1ff;
	int dy = R->scalelut[(y << 6) + s->sizey] & 0x1f;
	uint8_t row[32];
	int n;
	if (s->big) {
		const uint8_t *lo = R->bigsprites + s->code * 256 + dy * 8, *hi = lo + 0x8000;
		for (int g = 0; g < 8; g++) {
			uint32_t w = pens4[lo[g]] << 2 | pens4[hi[g]];
			memcpy(row + 4 * g, &w, 4);
		}
		n = 32;
	} else {
		dy >>= 1;
		const uint8_t *lo = R->sprites + s->code * 64 + dy * 4, *hi = lo + 0x2000;
		for (int g = 0; g < 4; g++) {
			uint32_t w = pens4[lo[g]] << 2 | pens4[hi[g]];
			memcpy(row + 4 * g, &w, 4);
		}
		n = 16;
	}
	int fx = s->fx ? n - 1 : 0;
	const uint8_t *lut = spr_lut[s->color];
	int x0 = s->sx & 0x3ff, step = 1 + s->sizex, acc = 0;
	if (x0 + (((2 * n - 1) * step) >> 6) < 0x100) {     // all on the screen
		uint8_t *o = out + x0;
		for (int p = 0; p < n; p++, acc += 2 * step) {
			uint8_t v = lut[row[p ^ fx]];
			if (v == CLEAR) continue;
			o[acc >> 6] = v;
			o[(acc + step) >> 6] = v;
		}
		return;
	}
	for (int p = 0; p < n; p++, acc += 2 * step) {
		uint8_t v = lut[row[p ^ fx]];
		if (v == CLEAR) continue;
		int a = (x0 + (acc >> 6)) & 0x3ff, b = (x0 + ((acc + step) >> 6)) & 0x3ff;
		if (a < 0x100) out[a] = v;
		if (b < 0x100) out[b] = v;
	}
}

// ---- the layers ---------------------------------------------------------------------
PPVID_HOT void ppvid_back(int line, uint8_t *out) {
	int y = line + PPVID_TOP;
	uint8_t scan[256 + 8];
	if (y < 128) {                      // the background tilemap, 64 x 16 tiles (columns first)
		int sx = S->hscroll & 511, ty = y >> 3, ry = y & 7;
		int tx = sx >> 3;
		uint8_t *d = scan;
		for (int k = 0; k < 33; k++, tx = (tx + 1) & 63, d += 8) {
			uint16_t w = S->view[tx * 16 + ty];
			// the code's 0x100 ((w & 0x4000) >> 6) goes past the 256 tiles
			const uint8_t *lut = bg_lut[(w & 0x3f00) >> 8];
			const uint8_t *t = R->tiles + (w & 0xff) * 16 + ry;
			uint32_t a = pens4[t[0]], b = pens4[t[8]];
			d[0] = lut[a & 3]; d[1] = lut[a >> 8 & 3]; d[2] = lut[a >> 16 & 3]; d[3] = lut[a >> 24];
			d[4] = lut[b & 3]; d[5] = lut[b >> 8 & 3]; d[6] = lut[b >> 16 & 3]; d[7] = lut[b >> 24];
		}
		memcpy(out, scan + (sx & 7), 256);
		return;
	}
	// the road
	int yoffs = ((vpm[y] + S->vscroll) >> 3) & 0x1ff;
	const uint8_t *lut = road_lut[S->road[yoffs] & 15];
	int xoffs = S->road[0x380 + (y & 0x7f)] & 0x3ff;
	int xscroll = xoffs & 7;
	xoffs &= ~7;
	uint8_t *d = scan;
	const uint8_t *rom = R->road + ((y & 0x7f) << 6);
	int row = (y & 0x7f) << 6;          // (k below, 0-63, never carries out of the row's 64)
	const uint8_t *rom2 = R->road + 0x4000 + ((row & 0xfff) | ((row & 0x1000) >> 1));
	for (int x = 0; x < 256 / 8 + 1; x++, xoffs += 8, d += 8) {
		if (xoffs & 0x200) {            // the road ROMs off: 0
			memset(d, lut[0], 8);
			continue;
		}
		int k = (xoffs & 0x1f8) >> 3;
		int control = rom[k];
		int bits1 = rom[0x2000 + k];
		int bits2 = rom2[k];
		// MAME steps by bits 8 to 1 after each pixel (bit 8 of a byte is 0,
		// bit 1's step comes after the last), so pixel j is the road value
		// plus bits 7 to 9 - j: bits1's once, bits2's twice, and once more
		// each of either where carry in is 0 (road_steps: all 8 at once)
		const uint32_t *t1 = road_steps[bits1], *t2 = road_steps[bits2];
		uint32_t base = (uint32_t)(control & 0x3f) * 0x01010101u;
		uint32_t lo = base + t1[0] + 2 * t2[0], hi = base + t1[1] + 2 * t2[1];
		if (!(control >> 7)) {
			const uint32_t *t3 = road_steps[bits1 | bits2];
			lo += t3[0];
			hi += t3[1];
		}
		lo &= 0x3f3f3f3fu;
		hi &= 0x3f3f3f3fu;
		d[0] = lut[lo & 0xff]; d[1] = lut[lo >> 8 & 0xff]; d[2] = lut[lo >> 16 & 0xff]; d[3] = lut[lo >> 24];
		d[4] = lut[hi & 0xff]; d[5] = lut[hi >> 8 & 0xff]; d[6] = lut[hi >> 16 & 0xff]; d[7] = lut[hi >> 24];
	}
	memcpy(out, scan + xscroll, 256);
}

PPVID_HOT void ppvid_sprites(int line, uint8_t *out) {
	uint64_t m = on_line[line + PPVID_TOP];
	while (m) {
		int i = __builtin_ctzll(m);
		m &= m - 1;
		sprite_line(&spr[i], line + PPVID_TOP, out);
	}
}

PPVID_HOT void ppvid_text(int line, uint8_t *out) {
	int y = line + PPVID_TOP, ty = y >> 3, ry = y & 7;
	uint32_t m = text_cells[ty];
	const uint16_t *row = &S->alpha[ty * 32];
	while (m) {
		int c = __builtin_ctz(m);
		m &= m - 1;
		uint16_t w = row[c];
		int color = (S->chacl ? (w & 0x3f00) >> 8 : 0) | (ty >= 16 ? 0x40 : 0);
		const uint8_t *lut = alpha_lut[color];
		const uint8_t *t = R->chars + (w & 0xff) * 16 + ry;
		uint32_t a = pens4[t[0]], b = pens4[t[8]];
		uint8_t *o = out + c * 8;
		for (int k = 0; k < 4; k++, a >>= 8, b >>= 8) {
			uint8_t u = lut[a & 3], v = lut[b & 3];
			if (u != CLEAR) o[k] = u;
			if (v != CLEAR) o[k + 4] = v;
		}
	}
}

PPVID_HOT void ppvid_text_under(int line, uint8_t *out) {
	int y = line + PPVID_TOP, ty = y >> 3;
	uint32_t m = text_cells[ty];
	const uint16_t *row = &S->alpha[ty * 32];
	while (m) {
		int c = __builtin_ctz(m);
		m &= m - 1;
		uint16_t w = row[c];
		int color = (S->chacl ? (w & 0x3f00) >> 8 : 0) | (ty >= 16 ? 0x40 : 0);
		uint8_t v = alpha_lut[color][0];
		if (v != CLEAR) memset(out + c * 8, v, 8);
	}
}

PPVID_HOT void ppvid_line(int line, uint8_t *out) {
	ppvid_back(line, out);
	ppvid_sprites(line, out);
	ppvid_text(line, out);
}

PPVID_HOT void ppvid_render(uint8_t *px) {
	for (int y = 0; y < PPVID_H; y++) ppvid_line(y, px + y * PPVID_W);
}
