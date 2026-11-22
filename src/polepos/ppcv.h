// ppcv: the cart's side of Pole Position on the ColecoVision. Portable C
// (no pico-sdk), used by the plugin and by the Mac tests. (Pac-Man's pmcv.)
//
// The console runs gbkernel (coleco/gbcv/kernel/gbkernel.asm): each TV
// frame it takes a command list from the cart and runs it. This makes those
// lists, from the board's RAM (ppvid.c draws MAME's picture) and sound:
//
//   once:                     ppcv_init(e, roms)
//                             (NULL before the ROMs are in: messages only;
//                             call again with them to start the game)
//   at each VBLANK:           ppcv_vblank(e)           (from pp_on_vblank)
//   each list:                ppcv_sound(e, samples, PPCV_PCM_N)
//                             (or ppcv_speech(e, samples, pos, n))
//                             len = ppcv_tv_frame(e, list)
//   a message instead:        ppcv_canvas(e, px), ppcv_sound(e, NULL, 0)
//
// The picture. The game's whole picture changes most frames (the road's
// colour bands are how it moves), more bytes than a list can carry, so a
// new picture is made only once the last one has nearly all gone: at each
// VBLANK ppcv_vblank copies the RAM the picture needs (unless a picture is
// being made from the last copy), and ppcv_tv_frame makes the picture from
// the newest copy, PPCV_LINES lines a list, and sends what differs from
// what the console has, as many bytes as the list has time for. Only the
// lines made from MAME lines that changed since the last picture are made
// again (a background line's tiles and scroll, a road line's colours and
// scroll, sprites now or then, its text row); the rest are left alone.
// ppcv_t's every (PPCV_EVERY, 1) is how many lists at least from one
// picture to the next: 2 halves the picture's cost to core 0.
//
// The layout (PPCV_SCALE). 2, half size, the build's: MAME's 256 x 224 at
// 128 x 112 in the middle of the TMS9918's 256 x 192 (x 64, y 40). Its text
// rows at the top and bottom (TOP, TIME, LAP / SCORE, SPEED; the credits)
// are drawn at full size in the borders, the top four rows of 8 lines at
// y 0-31 and the bottom four at 160-191. In the small picture those rows'
// letters are left out: a text cell there shows the colour around its
// letter, where that is a colour (the title's sand), or what's under it.
// 3, three quarters (for comparison): MAME's lines 32-223 at 192 x 144 (x
// 32, y 40), the top text rows at full size at y 0-31. A pair of pixels
// (half) keeps the one that differs from both its neighbours where the
// other doesn't, so lines one pixel wide (the road's edges, its stripes,
// the start line) survive; 3/4 is sdk/cvscale.h's.
//
// Colours: each of the PROMs' 128 as the nearest TMS9918 colour in OKLab,
// but for those the picture needs apart (ppcv_color). Graphics II gives
// two colours to each 8 x 1; the two most used are kept and the others go
// to the nearer of them.
//
// The sound. The sound agent's samples (MAME's +-1.0 as +-32767),
// PPCV_PCM_N a list, go out through the SN76489's three tones parked at a
// period of 1, their attenuators summed as a DAC (ppdac3.h, Donkey Kong's
// and the movies' way), three PSG bytes a sample. Every list also writes
// the rest of the chip: the three tones' periods, the noise's control and
// attenuator.
//
// While the 52xx talks the lists carry ppsnd's 8 kHz speech samples
// instead (ppcv_speech), each at its place in the list, through the same
// DAC, high-pass, scale and soft knee. The picture gets about 40% of its
// usual bytes until the speech ends.
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "pp.h"
#include "ppvid.h"

#ifndef PPCV_SCALE
#define PPCV_SCALE 2                    // 2: half size; 3: three quarters
#endif

#define PPCV_W      256
#define PPCV_H      192
#define PPCV_BYTES  6144                // pattern bytes, and as many colour bytes
#define PPCV_LIST_MAX 3000
#define PPCV_PCM_N  100                 // samples a list
#define PPCV_SPEECH_MAX 140             // speech samples a list at most (ppsnd's, 134 or 135)
#define PPCV_LIST_T 60036               // a list's length in the console's T (3,579,545 a second)
// the picture's lines made in a list: half of them, so a picture takes
// two lists (30 a second) and core 0 never has a whole one to make at
// once. All (112 at half size, 144 at 3/4) makes 60 a second for twice
// the cycles.
#ifndef PPCV_LINES
#define PPCV_LINES  (PPCV_SCALE == 2 ? 56 : 72)
#endif
// a new picture is begun once fewer bytes than this are left to send
#ifndef PPCV_START
#define PPCV_START  600
#endif
// and no sooner than this many lists after the last was begun (ppcv_t's
// every, which the plugin may change as it goes)
#ifndef PPCV_EVERY
#define PPCV_EVERY  1
#endif
// the board's cycles (3.072 MHz) in a list, << 16
#define PPCV_LIST_CYCLES_Q16 ((uint64_t)PPCV_LIST_T * PP_CLOCK * 65536u / 3579545u)
// the samples' rate, << 8: PPCV_PCM_N a list
#define PPCV_RATE_Q8 ((uint32_t)((uint64_t)PPCV_PCM_N * 3579545u * 256u / PPCV_LIST_T))

#if defined(PICO_ON_DEVICE) && PICO_ON_DEVICE
#define PPCV_HOT __attribute__((section(".time_critical.ppcv")))
#define PPCV_DATA __attribute__((section(".time_critical.ppcv_data")))    // a hot table, in RAM
#else
#define PPCV_HOT
#define PPCV_DATA
#endif

typedef struct {
    uint32_t tv_frames, pictures, pat_bytes, col_bytes, runs, refreshed, pcm_frames;
    uint32_t settled;                   // lists that ended with nothing left to send
    uint32_t t_max;                     // the longest list, in T
    uint32_t snaps;                     // VBLANKs whose RAM was copied
    uint32_t lines;                     // the small picture's lines made
    uint32_t lines_same;                // and those found as they were (left alone)
    uint32_t clipped;                   // samples the soft knee took in (ppcv.c's LOUD_KNEE)
    uint32_t speech_lists;              // lists with speech samples
    uint32_t speech_merged;             // speech samples at the list's end, where the PSG can't be written, put in the one before
} ppcv_stats_t;

typedef struct {
    // the console's VRAM as the lists so far leave it, and the newest picture
    uint8_t vram_pat[PPCV_BYTES], vram_col[PPCV_BYTES];
    uint8_t want_pat[PPCV_BYTES], want_col[PPCV_BYTES];
    uint32_t pending;                   // bytes where want and vram differ
    bool names_done;
    uint16_t names_at;
    uint16_t cursor;                    // where the next list's scan starts
    uint16_t refresh_at;
    bool dirty;                         // want differs from vram somewhere (or may)
    uint32_t bands;                     // the same, for each band of 8 lines (256 bytes)

    // the picture: the RAM it's made from, and how far it has got
    ppvid_ram_t ram;
    bool have_ram;                      // a VBLANK's RAM is in ram
    bool making;                        // a picture is being made from it (ram stays)
    uint16_t line_at;                   // the next of its lines to make
    uint8_t every, since;               // a picture begun no sooner than every lists after the last (since)
    bool pic_valid;                     // the borders are black, the text rows drawn
    uint16_t text_was[8][32];           // the text rows drawn at full size, as they were
    uint8_t chacl_was;

    // the sound
    bool pcm;                           // the lists carry samples (the game is on)
    uint8_t pcm_gain;                   // 0-32: the DAC comes up over the first lists
    int32_t hp_x, hp_y;                 // the high-pass's last input and output (<< 8)
    uint8_t pcm_bytes[PPCV_PCM_N][3];  // each sample's three attenuators, in the order written
    uint8_t tone_bytes[12];
    uint8_t tone_n;
    uint8_t resync;
    ppcv_stats_t stats;

    // the speech (after the rest: the hot code's offsets as they were)
    bool speech;                        // the next list's samples are speech (ppcv_speech)
    uint8_t sp_n;
    uint8_t sp_att[3];                  // the three tones' attenuations as the last sample left them (both kinds of list)
    uint8_t sp_bytes[PPCV_SPEECH_MAX][3];
    uint16_t sp_pos[PPCV_SPEECH_MAX];   // where each goes in the list (T)
} ppcv_t;

void ppcv_init(ppcv_t *e, const pp_roms_t *roms);
// at each VBLANK (pp_on_vblank): the RAM the picture needs copied, unless
// a picture is being made from the last copy
void ppcv_vblank(ppcv_t *e);
// a picture: 256 x 192 pixels, a TMS9918 colour each (0 is black); for messages
void ppcv_canvas(ppcv_t *e, const uint8_t *px);

// The next list's sound: n samples at PPCV_RATE_Q8, MAME's +-1.0 as
// +-32767; n = 0: none (a message: the console waits for the vertical
// blank again, and the DAC is let go)
void ppcv_sound(ppcv_t *e, const int16_t *s, int n);
// The next list's sound while the 52xx talks: n speech samples (ppsnd_take_list's,
// MAME's +-1.0 as +-32767, to be made PPSND_SPEECH_GAIN_Q8 / 256 louder),
// pos[i] sample i's place in the list in T from its start (rising, under
// PPCV_LIST_T). From the next ppcv_sound on, the usual lists again.
void ppcv_speech(ppcv_t *e, const int16_t *s, const uint16_t *pos, int n);

// the next list into out (PPCV_LIST_MAX bytes); returns its length
size_t ppcv_tv_frame(ppcv_t *e, uint8_t *out);

// the picture the console shows once the lists so far have run, a TMS9918
// colour a pixel (tests, previews)
void ppcv_screen(const ppcv_t *e, uint8_t *px);
// the picture wanted, the same way
void ppcv_wanted(const ppcv_t *e, uint8_t *px);
// the picture before Graphics II's two colours in 8: scaled and laid out,
// every pixel its own colour, from the RAM copy (tests, previews)
void ppcv_ideal(const ppvid_ram_t *ram, uint8_t *px);
// bytes the console still needs (tests; a whole scan)
uint32_t ppcv_backlog(const ppcv_t *e);
// the DAC's level for an attenuator pair (tests: the lists' sound as a WAV)
float ppcv_dac_level(int att1, int att2);
// and for all three tones' attenuators (tests: the speech lists)
float ppcv_dac_level3(int att0, int att1, int att2);
// tests built with PPCV_VERIFY: the small picture's lines left alone
// (found as they were) that, made again, weren't
extern uint32_t ppcv_verify_bad;
// the TMS9918's colours as 0xRRGGBB (previews)
extern const uint32_t ppcv_tms_rgb[16];
// the PROMs' 128 colours as TMS9918 colours
extern uint8_t ppcv_color[128];
