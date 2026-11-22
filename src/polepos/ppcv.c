// ppcv.c -- see ppcv.h. The lists and the sound are pmcv.c's (../pacman),
// the picture and its layout are new.
#include "ppcv.h"

#include <math.h>
#include <string.h>

#include "ppdac3.h"

#include "../sdk/cvscale.h"
#include "ppsnd.h"

// the hot parts; kept apart with PPCV_PROFILE (test/vidcount.sh's
// --profile counts them by function then)
#ifdef PPCV_PROFILE
#define PART __attribute__((noinline)) PPCV_HOT
#else
#define PART PPCV_HOT
#endif

// ---- lists (pmcv.c's, from gbkernel.asm's costs) ---------------------------------
#define FRAME_T    (262 * 228)
#define VBLANK_T   (70 * 228)
#define C_START    480
#define C_RUN_VB   106
#define C_BYTE_VB  23
#define C_RUN_ACT  106
#define C_BYTE_ACT 31
#define C_PSG      75
#define C_PSG_BYTE 52
#define C_REG      114
#define C_END_VB   54
#define C_END      52
#define VB_LIMIT   (VBLANK_T - 600)
#define LIST_LIMIT (FRAME_T - 1200)

// The console goes from one list straight into the next, so a list's
// length is the sample clock (pmcv.c's)
#define C_FRAME    357
#define PCM_L      PPCV_LIST_T
#define PCM_FIRST  (C_FRAME + C_END_VB)
#define C_SAMPLE   (C_PSG + 3 * C_PSG_BYTE)
#define PCM_STEP   (((PCM_L - PCM_FIRST - C_SAMPLE - C_END) << 8) / (PPCV_PCM_N - 1))
// speech: a sample is three bytes; none can start after SP_LAST
#define C_SPEECH   (C_PSG + 3 * C_PSG_BYTE)
#define SP_LAST    (PCM_L - C_END - C_SPEECH)
#define SP_END     (PCM_L - C_END)

#define PATTERNS 0x0000
#define COLORS   0x2000
#define NAMES    0x1800
#define BLACK    1

typedef struct {
    uint8_t *b;
    size_t n;
    uint32_t t;
    bool vblank;
} list_t;

PPCV_HOT static void l_run_head(list_t *l, uint16_t addr, int n) {
    l->t += l->vblank ? C_RUN_VB + n * C_BYTE_VB : C_RUN_ACT + n * C_BYTE_ACT;
    l->b[l->n++] = 1;
    l->b[l->n++] = (uint8_t)addr;
    l->b[l->n++] = (uint8_t)((addr >> 8) & 0x3F) | 0x40;
    l->b[l->n++] = (uint8_t)n;
}

PPCV_HOT static void l_psg(list_t *l, const uint8_t *d, int n) {
    l->t += C_PSG + C_PSG_BYTE * n;
    l->b[l->n++] = 2;
    l->b[l->n++] = (uint8_t)n;
    memcpy(l->b + l->n, d, n);
    l->n += n;
}

PPCV_HOT static void l_end_vblank(list_t *l) {
    if (!l->vblank) return;
    l->vblank = false;
    l->t += C_END_VB;
    l->b[l->n++] = 0;
}

PPCV_HOT static int room(const list_t *l) {
    int t = l->vblank ? ((int)VB_LIMIT - (int)l->t - C_END_VB - C_RUN_VB) / C_BYTE_VB
                      : ((int)LIST_LIMIT - (int)l->t - C_END - C_RUN_ACT) / C_BYTE_ACT;
    return t > 255 ? 255 : t;
}

// ---- colours -----------------------------------------------------------------------
const uint32_t ppcv_tms_rgb[16] = {0x000000, 0x000000, 0x21C842, 0x5EDC78, 0x5455ED, 0x7D76FC, 0xD4524D, 0x42EBF5,
                                   0xFC5554, 0xFF7978, 0xD4C154, 0xE6CE80, 0x21B03B, 0xC95BBA, 0xCCCCCC, 0xFFFFFF};

// The PROMs' colours as TMS9918 colours: the nearest in OKLab, but for
// these (by the PROMs' colour; the play and attract runs' census):
//   the road 515151 (nearest dark red)                    14 grey
//   its lines E0E0E0 (nearest grey, the road's now)       15 white
//   the sky 6270FF to 1F70FF (dark and light blue)        5 light blue, all of it
//   the far hills 2D5162 (dark blue)                      4 dark blue, against the sky
//   the kerbs' red E02D0E (dark red, the mountains')      8 medium red
//   the title's sand E0BC70 (dark yellow)                 11 light yellow
//   the cars' purple 700E9D (dark blue)                   13 magenta
//   the title's orange FF7000 (medium red)                9 light red
uint8_t ppcv_color[128];
static const uint8_t color_fix[][2] = {
    {64, 14}, {72, 15}, {11, 5}, {12, 5}, {13, 5}, {14, 5}, {15, 5}, {3, 4},
    {71, 8}, {44, 11}, {108, 11}, {22, 13}, {86, 13}, {42, 9},
};

static float tms_lab[16][3];
static uint8_t nearer[16][16][16];      // [c][a][b]: 1 if c is nearer a than b
static bool tms_ready, roms_in;

static void oklab(uint32_t rgb, float *out) {
    float c[3];
    for (int i = 0; i < 3; i++) {
        float v = (float)((rgb >> (16 - 8 * i)) & 255) / 255.0f;
        c[i] = v <= 0.04045f ? v / 12.92f : powf((v + 0.055f) / 1.055f, 2.4f);
    }
    float l = cbrtf(0.4122214708f * c[0] + 0.5363325363f * c[1] + 0.0514459929f * c[2]);
    float m = cbrtf(0.2119034982f * c[0] + 0.6806995451f * c[1] + 0.1073969566f * c[2]);
    float s = cbrtf(0.0883024619f * c[0] + 0.2817188376f * c[1] + 0.6299787005f * c[2]);
    out[0] = 0.2104542553f * l + 0.7936177850f * m - 0.0040720468f * s;
    out[1] = 1.9779984951f * l - 2.4285922050f * m + 0.4505937099f * s;
    out[2] = 0.0259040371f * l + 0.7827717662f * m - 0.8086757660f * s;
}

PPCV_HOT static float d2(const float *a, const float *b) {
    float x = a[0] - b[0], y = a[1] - b[1], z = a[2] - b[2];
    return x * x + y * y + z * z;
}

// the TMS9918's colours (all a message needs)
PPCV_HOT static void tms_init(void) {
    for (int i = 1; i < 16; i++) oklab(ppcv_tms_rgb[i], tms_lab[i]);
    for (int c = 1; c < 16; c++)
        for (int a = 1; a < 16; a++)
            for (int b = 1; b < 16; b++) nearer[c][a][b] = d2(tms_lab[c], tms_lab[a]) <= d2(tms_lab[c], tms_lab[b]);
    tms_ready = true;
}

// the PROMs' colours as TMS9918 ones (after ppvid_init)
PPCV_HOT static void prom_colors(void) {
    for (int i = 0; i < 128; i++) {
        float lab[3], best = 1e9f;
        oklab(ppvid_rgb[i], lab);
        for (int c = 1; c < 16; c++)
            if (d2(lab, tms_lab[c]) < best) { best = d2(lab, tms_lab[c]); ppcv_color[i] = (uint8_t)c; }
    }
    for (size_t k = 0; k < sizeof color_fix / sizeof color_fix[0]; k++) ppcv_color[color_fix[k][0]] = color_fix[k][1];
}

// ---- the layout --------------------------------------------------------------------
#if PPCV_SCALE == 2
#define PLAY_SRC   0                    // MAME's first line in the small picture
#define PLAY_LINES 112                  // its lines on the TMS9918
#define PLAY_X     64
#define PLAY_W     128
#define TEXT_BANDS 2
#elif PPCV_SCALE == 3
#define PLAY_SRC   32
#define PLAY_LINES 144
#define PLAY_X     32
#define PLAY_W     192
#define TEXT_BANDS 1
#else
#error "PPCV_SCALE is 2 or 3"
#endif
// the first list's lines: the second has the sprites' and the text's work too
// (at half size 68 and 44 make the two cost about the same)
#define FIRST_LINES (PPCV_SCALE == 2 && PPCV_LINES == 56 ? 68 : PPCV_LINES)
#define PLAY_Y     40
// the text rows drawn at full size: MAME's lines from, 32 of them, at the TMS9918's line to
static struct { uint8_t from, to; } text_band[2] = {{0, 0}, {192, 160}};

// MAME's text on line k is drawn at full size, not in the small picture
PPCV_HOT static inline bool text_moved(int k) { return k < 32 || (TEXT_BANDS > 1 && k >= 192); }

// MAME's line k, in TMS9918 colours (ppvid's map), for the small picture
PPCV_HOT PART static void src_line(int k, uint8_t *buf) {
    ppvid_back(k, buf);
    ppvid_sprites(k, buf);
    if (text_moved(k)) ppvid_text_under(k, buf);
    else ppvid_text(k, buf);
}

// ---- what changed since the last picture -------------------------------------------
// MAME's line k (0-223) is as it was when what it's made from is: a
// background line's scroll and its tile row, a road line's colours and
// scroll (ppvid_line_key), no sprite on it then or now, and its text row
// (or its fills, ppvid_text_under). The small picture's lines made
// only from lines as they were are left as they are.
static struct {
    bool valid;
    uint32_t key[PPVID_H];
    uint16_t view[0x400];               // the background's tiles
    uint16_t alpha[28][32];             // the text's rows 2-29
    uint32_t sprites[7];                // the lines with sprites, a bit each
    uint16_t hscroll;
    uint8_t chacl;
} was;
static uint32_t changed[7];             // the lines not as they were, a bit each

PPCV_HOT static inline bool line_changed(int k) { return changed[k >> 5] >> (k & 31) & 1; }

PPCV_HOT PART static void find_changes(const ppvid_ram_t *r) {
    bool all = !was.valid;
    bool scroll = (r->hscroll & 511) != (was.hscroll & 511), text_all = r->chacl != was.chacl;
    uint16_t rows = 0;                  // background tile rows 2-15 changed, a bit each
    uint32_t arows = 0;                 // text rows 2-29
    if (all || memcmp(r->view, was.view, sizeof was.view)) {
        for (int ty = 2; ty < 16; ty++)
            for (int c = 0; c < 64; c++)
                if (r->view[c * 16 + ty] != was.view[c * 16 + ty]) { rows |= (uint16_t)(1u << ty); break; }
        memcpy(was.view, r->view, sizeof was.view);
    }
    for (int ty = 2; ty < 30; ty++)
        if (all || text_all || memcmp(&r->alpha[ty * 32], was.alpha[ty - 2], 64)) {
            arows |= 1u << ty;
            memcpy(was.alpha[ty - 2], &r->alpha[ty * 32], 64);
        }
    memset(changed, 0, sizeof changed);
    for (int k = 0; k < PPVID_H; k++) {
        int y = k + PPVID_TOP;
        bool c = all;
        uint32_t key = ppvid_line_key(k), bit = 1u << (k & 31);
        bool spr = ppvid_line_sprites(k) != 0;
        if (y < 128) c |= scroll || (rows >> (y >> 3) & 1);
        else c |= key != was.key[k];
        c |= spr || (was.sprites[k >> 5] & bit);
        c |= arows >> (y >> 3) & 1;
        if (c) changed[k >> 5] |= bit;
        was.key[k] = key;
        if (spr) was.sprites[k >> 5] |= bit;
        else was.sprites[k >> 5] &= ~bit;
    }
    was.hscroll = r->hscroll;
    was.chacl = r->chacl;
    was.valid = true;
}

// the small picture's lines
typedef struct {
#if PPCV_SCALE == 2
    uint8_t hr[4][PLAY_W];              // MAME's lines halved across, by line & 3
    int16_t tag[4];                     // which line each is (-1: none)
#else
    uint8_t strip[4][256];              // a group of 4 of MAME's lines
    uint8_t out[3][PLAY_W];             // and the 3 they make
    int16_t group;                      // which group they are (-1: none)
#endif
} scaler_t;

PPCV_HOT static void scaler_reset(scaler_t *sc) {
#if PPCV_SCALE == 2
    for (int i = 0; i < 4; i++) sc->tag[i] = -1;
#else
    sc->group = -1;
#endif
}

#if PPCV_SCALE == 2
// 256 pixels to 128: of each pair, the one unlike both its neighbours
// where the other is like one of them (a line 1 pixel wide stays), else
// the first (cvscale_pick). Two pairs at a time, alike ones quickly.
PPCV_HOT PART static void halve_across(const uint8_t *s, uint8_t *o) {
    for (int x = 0; x < PLAY_W; x += 2) {
        uint32_t w;
        memcpy(&w, s + 2 * x, 4);       // a0 b0 a1 b1
        uint8_t a0 = (uint8_t)w, b0 = (uint8_t)(w >> 8), a1 = (uint8_t)(w >> 16), b1 = (uint8_t)(w >> 24);
        if (!((w ^ (w >> 8)) & 0x00FF00FFu)) {
            o[x] = a0;
            o[x + 1] = a1;
            continue;
        }
        uint8_t before = x ? s[2 * x - 1] : a0, after = x + 2 < PLAY_W ? s[2 * x + 4] : b1;
        o[x] = a0 == b0 ? a0 : cvscale_pick(a0, b0, before, a1);
        o[x + 1] = a1 == b1 ? a1 : cvscale_pick(a1, b1, b0, after);
    }
}

// MAME's line k, halved across (made once a picture)
PPCV_HOT PART static const uint8_t *halved(scaler_t *sc, int k) {
    static uint8_t buf[256];
    int i = k & 3;
    if (sc->tag[i] != k) {
        src_line(k, buf);
        halve_across(buf, sc->hr[i]);
        sc->tag[i] = (int16_t)k;
    }
    return sc->hr[i];
}
#endif

// the small picture's line y needs making again (its lines aren't as they were)
PPCV_HOT static bool play_changed(int y) {
#if PPCV_SCALE == 2
    int k0 = 2 * y - 1, k1 = 2 * y + 2;
    if (k0 < 0) k0 = 0;
    if (k1 > PPVID_H - 1) k1 = PPVID_H - 1;
#else
    int k0 = PLAY_SRC + 4 * (y / 3), k1 = k0 + 3;
#endif
    for (int k = k0; k <= k1; k++)
        if (line_changed(k)) return true;
    return false;
}

// line y (0 to PLAY_LINES - 1) of the small picture
PPCV_HOT PART static const uint8_t *play_line(scaler_t *sc, int y) {
#if PPCV_SCALE == 2
    static uint8_t out[PLAY_W];
    const uint8_t *b = y ? halved(sc, 2 * y - 1) : NULL;
    const uint8_t *u = halved(sc, 2 * y), *v = halved(sc, 2 * y + 1);
    const uint8_t *a = 2 * y + 2 < PPVID_H ? halved(sc, 2 * y + 2) : v;
    if (!b) b = u;
    for (int x = 0; x < PLAY_W; x += 4) {
        uint32_t uw, vw;
        memcpy(&uw, u + x, 4);
        memcpy(&vw, v + x, 4);
        if (uw == vw) {
            memcpy(out + x, &uw, 4);
            continue;
        }
        for (int j = x; j < x + 4; j++) out[j] = u[j] == v[j] ? u[j] : cvscale_pick(u[j], v[j], b[j], a[j]);
    }
    return out;
#else
    int g = y / 3;
    if (sc->group != g) {
        for (int k = 0; k < 4; k++) src_line(PLAY_SRC + 4 * g + k, sc->strip[k]);
        cvscale34(&sc->strip[0][0], 256, 256, 4, 0, 256, &sc->out[0][0], PLAY_W);
        sc->group = (int16_t)g;
    }
    return sc->out[y % 3];
#endif
}

static scaler_t scaler;
#ifdef PPCV_VERIFY
static uint8_t verify_pic[PLAY_LINES][PLAY_W];      // the small picture as made (tests: the lines left alone checked)
uint32_t ppcv_verify_bad;
#endif

// ---- the encoder -------------------------------------------------------------------
// 8 pixels (TMS9918 colours, 1-15) as a pattern byte and a colour byte,
// the colour byte kept as the console has it where it will do (pmcv.c's
// span, without the sprites' holes); the bytes still to send counted
PPCV_HOT PART static inline void span(ppcv_t *e, unsigned a, const uint8_t *c) {
    uint8_t vc = e->vram_col[a], vp = e->vram_pat[a];
    int before = (e->want_pat[a] != vp) + (e->want_col[a] != vc);
    uint8_t c0 = c[0], wp, wc;
    uint32_t w0, w1, k0 = c0 * 0x01010101u;
    memcpy(&w0, c, 4);
    memcpy(&w1, c + 4, 4);
    if (w0 == k0 && w1 == k0) {
        // one colour: kept as the console has it if either of its colours is this one
        uint8_t vf = vc >> 4, vb = vc & 15;
        if (vf == c0 || vb == c0) {
            wc = vc;
            wp = vf == vb ? vp : vf == c0 ? 0xFF : 0x00;
        } else {
            wc = (uint8_t)(c0 << 4 | c0);
            wp = vp;
        }
    } else {
        uint8_t d = 0;
        unsigned m0 = 0;
        int i = 0;
        for (; i < 8; i++) {
            uint8_t x = c[i];
            if (x == c0) m0 |= 0x80u >> i;
            else if (d && x != d) break;
            else d = x;
        }
        uint8_t fg, bg;
        if (i == 8) {                   // two colours
            static uint8_t pop4[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
            int n0 = pop4[m0 & 15] + pop4[m0 >> 4];     // (the M33 has no popcount instruction)
            uint8_t a1 = n0 > 4 ? c0 : n0 < 4 ? d : (c0 < d ? c0 : d), a2 = a1 == c0 ? d : c0;
            uint8_t vf = vc >> 4, vb = vc & 15;
            if (vf == a1 && vb == a2) { fg = a1; bg = a2; }
            else { fg = a2; bg = a1; }
            wp = (uint8_t)(fg == c0 ? m0 : ~m0 & 0xFF);
        } else {                        // more: the two most used (ties: the lower colour), the rest to the nearer
            uint8_t cnt[16] = {0};
            for (int k = 0; k < 8; k++) cnt[c[k]]++;
            uint8_t a1 = 0, a2 = 0;
            for (int k = 1; k < 16; k++)
                if (cnt[k] > cnt[a1] || (!a1 && cnt[k])) a1 = (uint8_t)k;
            for (int k = 1; k < 16; k++)
                if (k != a1 && cnt[k] && (!a2 || cnt[k] > cnt[a2])) a2 = (uint8_t)k;
            uint8_t vf = vc >> 4, vb = vc & 15;
            if (vf == a1 && vb == a2) { fg = a1; bg = a2; }
            else { fg = a2; bg = a1; }
            uint8_t pat = 0;
            for (int k = 0; k < 8; k++) {
                uint8_t x = c[k];
                pat = (uint8_t)(pat << 1 | (x == fg || (x != bg && nearer[x][fg][bg])));
            }
            wp = pat;
        }
        wc = (uint8_t)(fg << 4 | bg);
    }
    e->want_pat[a] = wp;
    e->want_col[a] = wc;
    e->pending += (uint32_t)((wp != vp) + (wc != vc) - before);
}

// a line of the TMS9918's picture: n pixels from x0 (multiples of 8)
PPCV_HOT PART static void encode_line(ppcv_t *e, int ty, int x0, const uint8_t *px, int n) {
    unsigned a = (unsigned)((ty >> 3) * 256 + x0 + (ty & 7));
    for (int k = 0; k < n; k += 8, a += 8) span(e, a, px + k);
    e->bands |= 1u << (ty >> 3);
}

PPCV_HOT static void black_line(ppcv_t *e, int ty, int x0, int n) {
    static uint8_t black[8] = {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK};
    unsigned a = (unsigned)((ty >> 3) * 256 + x0 + (ty & 7));
    for (int k = 0; k < n; k += 8, a += 8) span(e, a, black);
    e->bands |= 1u << (ty >> 3);
}

// the text rows at full size, where they have changed (all after a message)
PPCV_HOT static void make_text(ppcv_t *e) {
    uint8_t buf[256];
    for (int band = 0; band < TEXT_BANDS; band++)
        for (int r = 0; r < 4; r++) {
            int src = text_band[band].from + 8 * r, row = (src + PPVID_TOP) >> 3;
            uint16_t *was = e->text_was[band * 4 + r];
            if (e->pic_valid && e->chacl_was == e->ram.chacl && !memcmp(was, &e->ram.alpha[row * 32], 64)) continue;
            memcpy(was, &e->ram.alpha[row * 32], 64);
            for (int k = 0; k < 8; k++) {
                memset(buf, BLACK, sizeof buf);
                ppvid_text(src + k, buf);
                encode_line(e, text_band[band].to + 8 * r + k, 0, buf, 256);
            }
        }
    e->chacl_was = e->ram.chacl;
}

// the black around the picture (after a message)
PPCV_HOT static void make_borders(ppcv_t *e) {
    for (int ty = 0; ty < PPCV_H; ty++) {
        bool text = ty < 32 || (TEXT_BANDS > 1 && ty >= 160);
        if (text) continue;
        if (ty >= PLAY_Y && ty < PLAY_Y + PLAY_LINES) {
            black_line(e, ty, 0, PLAY_X);
            black_line(e, ty, PLAY_X + PLAY_W, PPCV_W - PLAY_X - PLAY_W);
        } else
            black_line(e, ty, 0, PPCV_W);
    }
}

PPCV_HOT void ppcv_vblank(ppcv_t *e) {
    if (e->making || !roms_in) return;
    ppvid_snap(&e->ram);
    e->have_ram = true;
    e->stats.snaps++;
}

// a new picture begun, if the last has nearly all gone and PPCV_EVERY
// lists have passed; then its next lines
PPCV_HOT PART static void make_picture(ppcv_t *e) {
    if (e->since < 255) e->since++;
    if (!e->making) {
        if (!e->have_ram || e->pending > PPCV_START || e->since < e->every) return;
        ppvid_begin(&e->ram);
        e->making = true;
        e->since = 0;
        e->line_at = 0;
        if (!e->pic_valid) {
            make_borders(e);
            was.valid = false;
        }
        make_text(e);
        find_changes(&e->ram);
        scaler_reset(&scaler);
        e->pic_valid = true;
    }
    int end = e->line_at ? e->line_at + PPCV_LINES : FIRST_LINES;
    if (end > PLAY_LINES) end = PLAY_LINES;
    for (int y = e->line_at; y < end; y++) {
        bool c = play_changed(y);
#ifdef PPCV_VERIFY
        const uint8_t *px = play_line(&scaler, y);
        if (c) memcpy(verify_pic[y], px, PLAY_W);
        else if (memcmp(verify_pic[y], px, PLAY_W)) ppcv_verify_bad++;
#endif
        if (!c) {
            e->stats.lines_same++;
            continue;
        }
        encode_line(e, PLAY_Y + y, PLAY_X, play_line(&scaler, y), PLAY_W);
        e->stats.lines++;
    }
    e->line_at = (uint16_t)end;
    e->dirty = true;
    if (end == PLAY_LINES) {
        e->making = false;
        e->have_ram = false;
        e->stats.pictures++;
    }
}

void ppcv_canvas(ppcv_t *e, const uint8_t *px) {
    uint8_t c[8];
    for (int y = 0; y < PPCV_H; y++)
        for (int cx = 0; cx < 32; cx++) {
            for (int i = 0; i < 8; i++) {
                uint8_t v = px[y * PPCV_W + cx * 8 + i];
                c[i] = v ? v : BLACK;
            }
            span(e, (unsigned)((y >> 3) * 256 + cx * 8 + (y & 7)), c);
        }
    e->bands = ~0u;
    e->pic_valid = false;
    e->making = false;
    e->have_ram = false;
    e->dirty = true;
}

PPCV_HOT static void screen_of(const uint8_t *pat, const uint8_t *colt, uint8_t *px) {
    for (int y = 0; y < PPCV_H; y++)
        for (int x = 0; x < PPCV_W; x++) {
            unsigned a = (unsigned)(((y >> 3) * 32 + (x >> 3)) * 8 + (y & 7));
            uint8_t c = colt[a];
            c = (pat[a] & (0x80 >> (x & 7))) ? c >> 4 : c & 15;
            px[y * PPCV_W + x] = c ? c : BLACK;
        }
}

PPCV_HOT void ppcv_screen(const ppcv_t *e, uint8_t *px) { screen_of(e->vram_pat, e->vram_col, px); }
PPCV_HOT void ppcv_wanted(const ppcv_t *e, uint8_t *px) { screen_of(e->want_pat, e->want_col, px); }

void ppcv_ideal(const ppvid_ram_t *ram, uint8_t *px) {
    static scaler_t sc;
    scaler_reset(&sc);
    memset(px, BLACK, PPCV_W * PPCV_H);
    ppvid_begin(ram);
    for (int band = 0; band < TEXT_BANDS; band++)
        for (int k = 0; k < 32; k++) ppvid_text(text_band[band].from + k, px + (text_band[band].to + k) * PPCV_W);
    for (int y = 0; y < PLAY_LINES; y++) memcpy(px + (PLAY_Y + y) * PPCV_W + PLAY_X, play_line(&sc, y), PLAY_W);
}

// ---- what the console needs ----------------------------------------------------------
PPCV_HOT static inline bool need_pat(const ppcv_t *e, unsigned j) { return e->want_pat[j] != e->vram_pat[j]; }
PPCV_HOT static inline bool need_col(const ppcv_t *e, unsigned j) { return e->want_col[j] != e->vram_col[j]; }

// The next bytes to send, from the cursor on (pmcv.c's next_run)
PPCV_HOT PART static bool next_run(ppcv_t *e, bool *color, unsigned *s, int *m, unsigned most) {
    if (!e->dirty) return false;
    unsigned i = e->cursor;
    int band = -1;
    for (unsigned k = 0; k < PPCV_BYTES;) {
        if (!(i & 255)) {
            if (band >= 0) e->bands &= ~(1u << band);
            band = -1;
            if (k + 256 <= PPCV_BYTES && !(e->bands >> (i >> 8) & 1)) {
                k += 256;
                i = i + 256 == PPCV_BYTES ? 0 : i + 256;
                continue;
            }
            band = (int)(i >> 8);
        }
        if (!(i & 3) && k + 4 <= PPCV_BYTES) {
            uint32_t w, v, wc, vc;
            memcpy(&w, e->want_pat + i, 4);
            memcpy(&v, e->vram_pat + i, 4);
            memcpy(&wc, e->want_col + i, 4);
            memcpy(&vc, e->vram_col + i, 4);
            if (w == v && wc == vc) {
                k += 4;
                i = i + 4 == PPCV_BYTES ? 0 : i + 4;
                continue;
            }
        }
        bool nc = need_col(e, i);
        if (!nc && !need_pat(e, i)) {
            k++;
            i = i + 1 == PPCV_BYTES ? 0 : i + 1;
            continue;
        }
        e->cursor = (uint16_t)i;
        *color = nc;
        *s = i;
        unsigned end = i + 1, gap = 0;
        for (unsigned j = i + 1; j < PPCV_BYTES && j - i < most; j++) {
            if (nc ? need_col(e, j) : need_pat(e, j)) { end = j + 1; gap = 0; }
            else if (++gap > 3) break;
        }
        *m = (int)(end - i);
        return true;
    }
    e->dirty = false;
    return false;
}

PPCV_HOT static void put_run(ppcv_t *e, list_t *l, bool color, unsigned s, int n) {
    l_run_head(l, (uint16_t)((color ? COLORS : PATTERNS) + s), n);
    uint8_t *d = l->b + l->n;
    const uint8_t *w = (color ? e->want_col : e->want_pat) + s;
    uint8_t *v = (color ? e->vram_col : e->vram_pat) + s;
    for (int k = 0; k < n; k++) e->pending -= w[k] != v[k];
    memcpy(d, w, n);
    memcpy(v, w, n);
    if (color) e->stats.col_bytes += (unsigned)n;
    else e->stats.pat_bytes += (unsigned)n;
    l->n += n;
    e->stats.runs++;
}

// what's on screen written again as it is (pmcv.c's)
PPCV_HOT static void refresh_run(ppcv_t *e, list_t *l, int n) {
    unsigned at = e->refresh_at;
    bool color = at >= PPCV_BYTES;
    unsigned s = color ? at - PPCV_BYTES : at;
    if (s + n > PPCV_BYTES) n = (int)(PPCV_BYTES - s);
    l_run_head(l, (uint16_t)((color ? COLORS : PATTERNS) + s), n);
    memcpy(l->b + l->n, (color ? e->vram_col : e->vram_pat) + s, n);
    l->n += n;
    e->refresh_at = (uint16_t)((at + n) % (2 * PPCV_BYTES));
    e->stats.refreshed += (unsigned)n;
}

PPCV_HOT uint32_t ppcv_backlog(const ppcv_t *e) {
    uint32_t n = 0;
    for (unsigned j = 0; j < PPCV_BYTES; j++) n += need_pat(e, j) + need_col(e, j);
    return n;
}

// the name table: name i at place i in each third (it changes never)
PPCV_HOT static void names_run(ppcv_t *e, list_t *l, int n) {
    int left = 768 - e->names_at;
    if (n > left) n = left;
    l_run_head(l, (uint16_t)(NAMES + e->names_at), n);
    for (int k = 0; k < n; k++) l->b[l->n++] = (uint8_t)(e->names_at + k);
    e->names_at = (uint16_t)(e->names_at + n);
    if (e->names_at == 768) e->names_done = true;
}

// ---- a list, the console waiting for the vertical blank (messages) -------------------
PPCV_HOT static void plain_list(ppcv_t *e, list_t *l) {
    if (e->resync) {
        l_end_vblank(l);
        if (e->tone_n) l_psg(l, e->tone_bytes, e->tone_n);
        if (e->resync-- == 2) return;
    } else if (e->tone_n)
        l_psg(l, e->tone_bytes, e->tone_n);
    while (!e->names_done) {
        int n = room(l);
        if (n < 16 && l->vblank) { l_end_vblank(l); continue; }
        if (n < 1) break;
        names_run(e, l, n);
    }
    bool color;
    unsigned s;
    int m;
    while (e->names_done && next_run(e, &color, &s, &m, 255)) {
        int n = room(l);
        if (n < m && l->vblank) {
            if (n >= 8) put_run(e, l, color, s, n);
            l_end_vblank(l);
            continue;
        }
        if (n < 1) break;
        put_run(e, l, color, s, n < m ? n : m);
    }
    for (int k = 0; k < 2 && e->names_done; k++) {
        int n = room(l);
        if (n < 32) break;
        refresh_run(e, l, n);
    }
    l_end_vblank(l);
}

// ---- a list with samples in it (pmcv.c's pcm_list, without the sprites) -----------------
PPCV_HOT PART static void fill(ppcv_t *e, list_t *l, uint32_t until) {
    int avail = (int)until - (int)l->t;
    if (avail < C_RUN_ACT + C_BYTE_ACT) return;
    int n = (avail - C_RUN_ACT) / C_BYTE_ACT;
    if (n > 255) n = 255;
    if (!e->names_done) { names_run(e, l, n); return; }
    bool color;
    unsigned s;
    int m;
    if (!next_run(e, &color, &s, &m, 1)) {
        if (e->refresh_at % PPCV_BYTES + (unsigned)n > PPCV_BYTES) e->refresh_at = (uint16_t)(e->refresh_at / PPCV_BYTES * PPCV_BYTES);
        refresh_run(e, l, n);
        return;
    }
    if (s + (unsigned)n > PPCV_BYTES) s = PPCV_BYTES - (unsigned)n;
    put_run(e, l, color, s, n);
}

PPCV_HOT PART static void pcm_list(ppcv_t *e, list_t *l) {
    l->t = C_FRAME;
    l_end_vblank(l);
    unsigned tone_at = 0;
    for (int k = 0; k < PPCV_PCM_N; k++) {
        uint8_t b[10];
        int n = 0;
        b[n++] = e->pcm_bytes[k][0];
        b[n++] = e->pcm_bytes[k][1];
        b[n++] = e->pcm_bytes[k][2];
        while (tone_at < e->tone_n && n < 5) {
            unsigned g = 1;
            while (tone_at + g < e->tone_n && !(e->tone_bytes[tone_at + g] & 0x80)) g++;
            if (n + (int)g > 6) break;
            memcpy(b + n, e->tone_bytes + tone_at, g);
            n += (int)g;
            tone_at += g;
        }
        if (k + 1 == PPCV_PCM_N) {
            l_psg(l, b, n);
            break;
        }
        uint32_t until = PCM_FIRST + (((uint32_t)(k + 1) * PCM_STEP) >> 8);
        int avail = (int)until - (int)l->t - (C_PSG + C_PSG_BYTE * n);
        // a short run of the picture's as one of two (pmcv.c's)
        bool color;
        unsigned s;
        int m = 0;
        bool two = e->names_done && next_run(e, &color, &s, &m, 7) && m <= 2 &&
                   avail >= 2 * C_RUN_ACT + C_BYTE_ACT * (m + 2);
        int need = two ? m + 1 : 1;
        int base = avail - (two ? 2 : 1) * C_RUN_ACT, best = 0, best_left = 99;
        for (int r = 0; r < 3 && base - C_PSG_BYTE * r >= C_BYTE_ACT * need; r++) {
            int left = (base - C_PSG_BYTE * r) % C_BYTE_ACT;
            if (left < best_left) { best_left = left; best = r; }
        }
        while (best--) b[n++] = e->pcm_bytes[k][2];     // (the last attenuator again: no change)
        l_psg(l, b, n);
        if (two) put_run(e, l, color, s, m);
        fill(e, l, until);
    }
    e->stats.pcm_frames++;
    e->resync = 2;
}

// ---- a list with speech samples: each at its place, the picture's runs between ----------
// (flatten: its own copies of fill and the rest, so the usual list's code is as it was)
__attribute__((flatten)) PPCV_HOT static void speech_list(ppcv_t *e, list_t *l) {
    l->t = C_FRAME;
    l_end_vblank(l);
    unsigned tone_at = 0;
    int n = e->sp_n;
    uint32_t until = e->sp_pos[0];
    for (int k = 0;; k++) {
        fill(e, l, until);              // the picture's runs to the next sample
        if (k == n) break;
        uint8_t b[12];
        int nb = 3, most = k == n - 1 ? 12 : 6;     // (the last takes what's left of the parking)
        memcpy(b, e->sp_bytes[k], 3);
        until = k + 1 < n ? e->sp_pos[k + 1] : SP_END;
        while (tone_at < e->tone_n && (nb < 5 || k == n - 1)) {
            unsigned g = 1;
            while (tone_at + g < e->tone_n && !(e->tone_bytes[tone_at + g] & 0x80)) g++;
            if (nb + (int)g > most || (int)l->t + C_PSG + C_PSG_BYTE * (nb + (int)g) > (int)until) break;
            memcpy(b + nb, e->tone_bytes + tone_at, g);
            nb += (int)g;
            tone_at += g;
        }
        // the last attenuator again (no change) to leave a run of the
        // picture the least time over, or in a short gap no run and only
        // those, whichever leaves less (the next sample is that much early)
        int avail = (int)until - (int)l->t - (C_PSG + C_PSG_BYTE * nb), pad = 0, best_left = 99;
        if (avail > 0 && avail < 5 * C_PSG_BYTE) {
            pad = avail / C_PSG_BYTE;
            best_left = avail - C_PSG_BYTE * pad;
        }
        for (int r = 0; r < 4 && avail - C_RUN_ACT - C_PSG_BYTE * r >= C_BYTE_ACT; r++) {
            int left = (avail - C_RUN_ACT - C_PSG_BYTE * r) % C_BYTE_ACT;
            if (left <= best_left) { best_left = left; pad = r; }
        }
        while (pad-- > 0 && nb < (int)sizeof b) b[nb++] = b[2];
        l_psg(l, b, nb);
    }
    e->stats.pcm_frames++;
    e->stats.speech_lists++;
    e->resync = 2;
}

static inline size_t list_end(ppcv_t *e, list_t *l) {
    l->t += C_END;
    l->b[l->n++] = 0;
    e->tone_n = 0;
    e->stats.tv_frames++;
    if (l->t > e->stats.t_max) e->stats.t_max = l->t;
    if (!e->dirty) e->stats.settled++;
    return l->n;
}

// (apart, so ppcv_tv_frame's own list stays in registers as it was)
__attribute__((noinline, cold)) PPCV_HOT static size_t speech_frame(ppcv_t *e, uint8_t *out) {
    list_t l = {out, 0, C_START, true};
    speech_list(e, &l);
    return list_end(e, &l);
}

PPCV_HOT size_t ppcv_tv_frame(ppcv_t *e, uint8_t *out) {
    make_picture(e);
    if (__builtin_expect(e->speech, 0)) return speech_frame(e, out);
    list_t l = {out, 0, C_START, true};
    if (e->pcm) pcm_list(e, &l);
    else plain_list(e, &l);
    return list_end(e, &l);
}

// ---- the sound ---------------------------------------------------------------------------
// The DAC: the three tones at a period of 1, their attenuators as measured
// on the user's console (coleco/ladder.json), summed (Donkey Kong 0.1.2's,
// ../dkong/dkcv.c). ppdac3.h's 512 levels go from 0 to 2.40, each a triple
// of attenuators chosen as Movies' table is (tools/mkdac3.py: within 1.5
// levels of its level, neighbouring levels' triples alike, so a small
// change makes small partial sums between a sample's three writes). The
// usual lists and the speech's use the same table and rest at the same
// middle (DAC_MID, 1.20), so a list of one kind follows one of the other
// at the same level.
//
// Up to 0.1.1 the usual lists' DAC was two tones (128 levels, 0 to 1.40,
// resting at 0.71) and the speech's three tones resting at 0.71 too, the
// high-pass held so the 52xx's sound, all above its rest, had the 1.69 to
// the top. Now the high-pass runs on through the speech (the TV's own
// coupling takes the DC off anyway), so the speech swings either side of
// the rest like the rest of the sound, and both have 1.20 either side.
//
// PPCV_SCALE_LEVELS levels for SCALE_FULL of the samples, for the usual
// lists and the speech alike, so their balance is as before; past LOUD_KNEE
// a soft knee rounds the loudest off toward the DAC's end, never clipping.
// 0.1.2: 240 levels, 2.9 times 0.1.1's 35 two-tone levels (+9.3 dB):
// 0.1.1 played some 13 dB under an ordinary ColecoVision cartridge (a
// game's square tones swing twice as far as a parked tone's level can).
#define DAC_LEVELS PPDAC3_LEVELS
#define DAC_MID    255
#define DAC_HALF   255.0f               // the levels either side of DAC_MID
#ifndef PPCV_SCALE_LEVELS
#define PPCV_SCALE_LEVELS 240
#endif
#define SCALE_FULL 8192
#define LOUD_KNEE  191.0f               // the soft knee: 3/4 of the 255 levels either side
#define HP_SHIFT   6                    // a pole at 15 Hz
#ifndef SP_HP_SHIFT
#define SP_HP_SHIFT 6                   // the speech's, at its 8 kHz: 20 Hz
#endif
static const float ladder[3][16] = {
    {1.0f, 0.7914f, 0.6320f, 0.5084f, 0.4019f, 0.3161f, 0.2459f, 0.1912f, 0.1396f, 0.1080f, 0.0844f, 0.0672f, 0.0524f, 0.0421f, 0.0310f, 0.0f},
    {1.0f, 0.7816f, 0.6125f, 0.4850f, 0.3925f, 0.3109f, 0.2416f, 0.1872f, 0.1373f, 0.1058f, 0.0828f, 0.0653f, 0.0533f, 0.0412f, 0.0306f, 0.0f},
    {1.0f, 0.7979f, 0.6229f, 0.4921f, 0.3942f, 0.3004f, 0.2348f, 0.1874f, 0.1419f, 0.1106f, 0.0846f, 0.0677f, 0.0534f, 0.0420f, 0.0328f, 0.0f}};
static uint8_t limit[512];                  // |level| (0-511) to 0-255: straight to LOUD_KNEE, then soft
static uint16_t dac3[PPDAC3_LEVELS];        // pp_dac3 copied to SRAM (the plugin runs from flash)
static int16_t lq[3][16];                   // ladder in 1/16384ths, for put_sample's integer sums
static bool dac_ready;

PPCV_HOT float ppcv_dac_level(int a1, int a2) { return ladder[1][a1 & 15] + ladder[2][a2 & 15]; }
float ppcv_dac_level3(int a0, int a1, int a2) { return ladder[0][a0 & 15] + ladder[1][a1 & 15] + ladder[2][a2 & 15]; }

static void dac_init(void) {
    for (int i = 0; i < 512; i++) {
        float v = (float)i;
        if (v > LOUD_KNEE) v = LOUD_KNEE + (DAC_HALF - LOUD_KNEE) * (1.0f - expf(-(v - LOUD_KNEE) / (DAC_HALF - LOUD_KNEE)));
        limit[i] = (uint8_t)(v + 0.5f);
    }
    memcpy(dac3, pp_dac3, sizeof dac3);
    for (int c = 0; c < 3; c++)
        for (int a = 0; a < 16; a++) lq[c][a] = (int16_t)lrintf(ladder[c][a] * 16384.0f);
    dac_ready = true;
}

// a sample in: the high-pass, the scale, the knee and the DAC's ramp up;
// out its level (0 to DAC_LEVELS - 1)
PPCV_HOT static int dac_sample(ppcv_t *e, int32_t x, int k, int shift) {
    e->hp_y += ((x - e->hp_x) << 8) - (e->hp_y >> shift);
    e->hp_x = x;
    int32_t v = (e->hp_y >> 8) * PPCV_SCALE_LEVELS / SCALE_FULL;
    int32_t a = v < 0 ? -v : v;
    if (a > (int32_t)LOUD_KNEE) e->stats.clipped++;
    a = limit[a > 511 ? 511 : a];
    int32_t lvl = DAC_MID + (v < 0 ? -a : a);
    // (the DAC's rest is 1.7 times the two tones': up to it over 43 ms, as steep as before)
    if (e->pcm_gain < 32 && !(k & 7)) e->pcm_gain++;
    lvl = lvl * e->pcm_gain / 32;
    return lvl < 0 ? 0 : lvl > DAC_LEVELS - 1 ? DAC_LEVELS - 1 : lvl;
}

// a sample's three writes, in the order whose two partial sums (the chip
// between the writes, 52 T each) are nearest the new level. Written a,
// then b, then c: off the new level by |d_b + d_c|, then by |d_c|. In
// integers, the orders tried in turn and the first of equals kept (Cloak's
// clcv.c; about half the cycles of floats).
static PPCV_DATA const uint8_t order[6][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};
static inline int32_t iabs(int32_t x) { return x < 0 ? -x : x; }
PART static void put_sample(ppcv_t *e, uint8_t *out, uint16_t t) {
    const int w0 = t >> 8, w1 = t >> 4 & 15, w2 = t & 15;
    const int32_t d0 = lq[0][w0] - lq[0][e->sp_att[0]], d1 = lq[1][w1] - lq[1][e->sp_att[1]],
                  d2 = lq[2][w2] - lq[2][e->sp_att[2]];
    const int32_t sum = d0 + d1 + d2;
    const int32_t f0 = iabs(sum - d0), f1 = iabs(sum - d1), f2 = iabs(sum - d2);
    const int32_t l0 = iabs(d0), l1 = iabs(d1), l2 = iabs(d2);
    int32_t best = f0 + l2, v;
    int pick = 0;
    if ((v = f0 + l1) < best) { best = v; pick = 1; }
    if ((v = f1 + l2) < best) { best = v; pick = 2; }
    if ((v = f1 + l0) < best) { best = v; pick = 3; }
    if ((v = f2 + l1) < best) { best = v; pick = 4; }
    if ((v = f2 + l0) < best) pick = 5;
    const uint32_t b = (uint32_t)(0x90 | w0) | (uint32_t)(0xB0 | w1) << 8 | (uint32_t)(0xD0 | w2) << 16;
    out[0] = (uint8_t)(b >> 8 * order[pick][0]);
    out[1] = (uint8_t)(b >> 8 * order[pick][1]);
    out[2] = (uint8_t)(b >> 8 * order[pick][2]);
    e->sp_att[0] = (uint8_t)w0;
    e->sp_att[1] = (uint8_t)w1;
    e->sp_att[2] = (uint8_t)w2;
}

PPCV_HOT void ppcv_sound(ppcv_t *e, const int16_t *s, int n) {
    if (!dac_ready) dac_init();
    e->tone_n = 0;
    e->speech = false;
    if (n <= 0) {
        if (e->pcm) {
            static uint8_t off[] = {0x9F, 0xBF, 0xDF, 0xFF};
            memcpy(e->tone_bytes, off, sizeof off);
            e->tone_n = sizeof off;
            e->pcm = false;
        }
        return;
    }
    if (!e->pcm) {
        e->pcm = true;
        e->pcm_gain = 0;
        e->hp_x = s[0];
        e->hp_y = 0;
        e->sp_att[0] = e->sp_att[1] = e->sp_att[2] = 15;    // (as the lists before left them)
    }
    // Every list, the whole chip but the tones' attenuators (the samples'):
    // the three tones at a period of 1, the noise's control, the noise
    // silent. A byte the console misreads (a bus hiccup) then lasts a list
    // at most (pmcv.c's park, and the rest).
    static const uint8_t park[] = {0x81, 0x00, 0xA1, 0x00, 0xC1, 0x00, 0xE4, 0xFF};
    memcpy(e->tone_bytes, park, sizeof park);
    e->tone_n = sizeof park;
    for (int k = 0; k < PPCV_PCM_N; k++)
        put_sample(e, e->pcm_bytes[k], dac3[dac_sample(e, s[k < n ? k : n - 1], k, HP_SHIFT)]);
}

// Speech: the same high-pass, scale, knee and DAC as the usual lists, at
// the speech's 8 kHz, each sample at its place in the list.
PPCV_HOT void ppcv_speech(ppcv_t *e, const int16_t *s, const uint16_t *pos, int n) {
    if (!dac_ready) dac_init();
    e->tone_n = 0;
    if (n > PPCV_SPEECH_MAX) n = PPCV_SPEECH_MAX;
    static const int16_t none = 0;
    static const uint16_t at0 = 0;
    if (n <= 0) { s = e->pcm ? NULL : &none; pos = &at0; }     // (none: the last level held)
    if (!e->pcm) {
        e->pcm = true;
        e->pcm_gain = 0;
        e->hp_x = 0;
        e->hp_y = 0;
        e->sp_att[0] = e->sp_att[1] = e->sp_att[2] = 15;
    }
    e->speech = true;
    static const uint8_t park[] = {0x81, 0x00, 0xA1, 0x00, 0xC1, 0x00, 0xE4, 0xFF};
    memcpy(e->tone_bytes, park, sizeof park);
    e->tone_n = sizeof park;
    int m = n > 0 ? n : 1;
    while (m > 1 && PCM_FIRST + pos[m - 1] > SP_LAST) m--;
    int32_t y = e->hp_x;
    for (int k = 0; k < m; k++) {
        if (s) {
            int32_t x = s[k];
            if (k == m - 1 && m < n) {      // and those after it: their mean
                for (int j = m; j < n; j++) x += s[j];
                x /= n - m + 1;
                e->stats.speech_merged += (uint32_t)(n - m);
            }
            y = x * PPSND_SPEECH_GAIN_Q8 / 256;
        }
        put_sample(e, e->sp_bytes[k], dac3[dac_sample(e, y, k, SP_HP_SHIFT)]);
        e->sp_pos[k] = (uint16_t)(PCM_FIRST + pos[k]);
    }
    e->sp_n = (uint8_t)m;
}

void ppcv_init(ppcv_t *e, const pp_roms_t *roms) {
    memset(e, 0, sizeof *e);
    e->dirty = true;
    e->bands = (1u << (PPCV_BYTES / 256)) - 1;
    e->every = PPCV_EVERY;
    was.valid = false;
    if (!tms_ready) tms_init();
    if (!dac_ready) dac_init();
    // no ROMs yet: messages only (ppcv_canvas), no game picture
    roms_in = roms != NULL;
    if (!roms_in) return;
    ppvid_init(roms);
    prom_colors();
    ppvid_map(ppcv_color);
}
