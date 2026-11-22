// polepos -- Namco's Pole Position (1982) on the ColecoVision: the cart runs
// the arcade board (../pp.c, MAME 0.288's emulation: a Z80, two Z8002s, the
// four Namco MCUs and the sound) from the user's ROMs on the card and sends
// the picture and the sound to the console as the Game Boy kernel's lists
// (../ppcv.c). RESET goes back to the menu.
//
//   core 1  serves the kernel and a list a TV frame (cvframes.c)
//   core 0  this loop: a list at a time (../pprun.c), the board run for the
//           list's time, the picture's next lines and the list's samples,
//           handed to core 1 as soon as it will take it
//
// A list that isn't ready when the console wants it makes the console run
// the last one again (its sound once more) and puts the game a list
// behind. The note counts those.
//
// The ROMs (MAME's "polepos" set, and namco51.zip to namco54.zip or their
// files) are read from the card's /POLEPOS folder, loose or zipped, or from
// /POLEPOS.ZIP, each known by its size and CRC-32.
//
// Controls (../pppad.h): the joystick or a USB pad's d-pad or left stick
// turn the wheel; right fire or a pad's A or ZR is the accelerator, left
// fire or B or ZL the brake; joystick up or R picks HI, down or L LO;
// keypad * or # or - is a coin.
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/watchdog.h"

#include "cvdiag.h"
#include "cvframes.h"
#include "cvpad.h"
#include "cvpsram.h"
#include "cvsd.h"
#include "cvxip.h"
#include "vdp.h"
#include "gbkernel.h"
#include "handoff.h"
#include "ppcard.h"
#include "ppcv.h"
#include "pppad.h"
#include "pprun.h"

#define OPEN_US     20000           // serve the console this long before the card's (blocking) start
#define WATCHDOG_MS 4000            // a hang reboots into the menu
#define NOTE_US     1000000         // the menu's note is brought up to date this often
#define GAME        "Pole Position"       // in the menu's notes
#define TAG         "PP"

static ppcv_t enc;
static pprun_t run;
static pppad_t pad;
static pp_roms_t roms;
static uint8_t road[0x5000];                // the road's ROMs in SRAM: the picture reads them all, more than the XIP cache holds
static uint8_t list[PPCV_LIST_MAX];
static size_t list_len;
static bool list_pending;                   // built, not published yet
static uint32_t seen_frames;
// The board's SRAM that pp_init fills (pp_spare) holds the card's state
// and its reads (a zip's directory, a packed ROM and the ROM) until then.
static fs_t *fs;
// a message's canvas: in the PSRAM, with the ROMs' store after it (or, if
// the PSRAM doesn't answer, in the board's SRAM that pp_init fills later)
static canvas_t *canvas;
#define CANVAS      (*canvas)
#define STORE_AT    ((sizeof(canvas_t) + 0xfffu) & ~0xfffu)

// ---- the console ---------------------------------------------------------------------
static void tasks(void) {
    cvpad_task();
    cvf_task(time_us_32());
    watchdog_update();
}

static void publish(void) {
    if (list_pending && cvf_publish(list, list_len)) list_pending = false;
}

// TV frames the console has started since the last call (0: none yet, or
// the last list isn't out): for the messages' lists, a TV frame each
static uint32_t new_frames(void) {
    tasks();
    publish();
    if (list_pending) return 0;
    uint32_t f = cvf_frames(), n = f - seen_frames;
    seen_frames = f;
    return n;
}

static void send(void) {
    list_len = ppcv_tv_frame(&enc, list);
    list_pending = true;
    publish();
}

// ---- messages ------------------------------------------------------------------------
static void center(int y, const char *s, uint8_t color, int scale) {
    int w = (int)strlen(s) * 8 * scale;
    canvas_text(&CANVAS, w < VDP_W ? (VDP_W - w) / 2 : 0, y, s, color, scale);
}

static void message(const char *head, const char *l1, const char *l2, const char *l3) {
    canvas_init(&CANVAS);
    center(32, "POLE POSITION", C_LRED, 2);
    if (head) center(80, head, C_WHITE, 1);
    if (l1) center(100, l1, C_GRAY, 1);
    if (l2) center(112, l2, C_GRAY, 1);
    if (l3) center(124, l3, C_GRAY, 1);
    center(176, "RESET: BACK TO THE MENU", C_LBLUE, 1);
    ppcv_canvas(&enc, &CANVAS.px[0][0]);
}

// serve the console, a list a frame, for us microseconds (forever: 0)
static void serve_for(uint32_t us) {
    uint32_t t0 = time_us_32();
    while (!us || time_us_32() - t0 < us) {
        if (!new_frames()) continue;
        ppcv_sound(&enc, NULL, 0);
        send();
    }
}

static void __attribute__((noreturn)) fail(const char *why, const char *l2, const char *l3, const char *menu_note) {
    message("IT CAN'T START", why, l2, l3);
    cvf_note(menu_note);
    serve_for(0);
    for (;;) tight_loop_contents();
}

// between a file's sectors: the console keeps its picture
static void card_tick(void) {
    if (!new_frames()) return;
    ppcv_sound(&enc, NULL, 0);
    send();
}

// ---- the game ------------------------------------------------------------------------
static uint32_t published, frames_at_start, us_max, pics_at_note;

// one list: the controls, the board for the list's time, the list made,
// then handed over as soon as core 1 takes it
static void game_list(void) {
    uint32_t t0 = time_us_32();
    uint8_t joy, fire, key;
    cvf_input(&joy, &fire, &key);
    gamepad_t g;
    bool have = cvpad_get(0, &g);
    pppad_frame(&pad, joy, fire, key, have ? &g : NULL, &pp_in);
    list_len = pprun_list(&run, list);
    uint32_t us = time_us_32() - t0;
    if (us > us_max) us_max = us;
    while (!cvf_publish(list, list_len)) tasks();
    published++;
    tasks();
}

// the menu's bottom line, should RESET come: pictures a second, lists the
// console ran again for want of a new one (since the start), the slowest
// list this second
static void update_note(void) {
    static uint32_t t_last;
    uint32_t t = time_us_32();
    if (t - t_last < NOTE_US) return;
    int32_t again = (int32_t)(cvf_frames() - frames_at_start) - (int32_t)(published - 1);
    if (again < 0) again = 0;
    unsigned per_s = MIN(enc.stats.pictures - pics_at_note, 99u), ms = MIN(us_max / 1000, 99u), tenths = us_max / 100 % 10;
    uint32_t unsel, early, unhdr;
    cvf_bus_counts(&unsel, &early, &unhdr);
    char n[48];
    snprintf(n, sizeof n, TAG " %u/s %ldL %u.%ums r%u s%u h%u", per_s, (long)MIN(again, 9999), ms, tenths,
             (unsigned)MIN(pp_resets, 9u), (unsigned)MIN(unsel, 999u), (unsigned)MIN(unhdr, 999u));
    cvf_note(n);
    t_last = t;
    pics_at_note = enc.stats.pictures;
    us_max = 0;
}

int main(void) {
    static const cvf_config_t cfg = {gbkernel_bin, GBKERNEL_BIN_LEN, handoff_bin, HANDOFF_BIN_LEN};
    bool quad = cvxip_fast();                       // quad flash reads: before core 1 and the PSRAM (cvxip.h)
    cvf_start(&cfg);
    cvd_start();
    cvd_stage(quad ? 101 : 100);
    bool psram = cvpsram_init();
    uint32_t spare_n;
    uint8_t *spare = pp_spare(&spare_n);
    canvas = psram ? (canvas_t *)CVPSRAM_CACHED : (canvas_t *)spare;
    if (spare_n < sizeof(canvas_t)) for (;;) {}     // (51200 bytes: it holds a canvas)
    uint32_t room_n = (spare_n - (uint32_t)sizeof(fs_t)) & ~7u;
    fs = (fs_t *)(spare + room_n);
    cvpad_init();
    ppcv_init(&enc, NULL);                          // the messages' (no ROMs yet)
    message("LOOKING FOR THE ROMS", "IN " PPCARD_HOME " ON THE CARD", NULL, NULL);
    serve_for(OPEN_US);
    if (!psram) fail("THE PSRAM DIDN'T ANSWER", NULL, NULL, GAME ": the PSRAM didn't answer");
    pp_rom_store = (pp_rom_store_t *)(CVPSRAM_CACHED + STORE_AT);
    memset(pp_rom_store, 0, sizeof *pp_rom_store);  // (the speech ROM's last 8K stay 0)

    if (!cvsd_card_in() || !cvsd_init()) fail(cvsd_card_in() ? "THE CARD DIDN'T ANSWER" : "NO CARD IN THE SLOT", NULL, NULL, GAME ": no card");
    if (fs_mount(fs, cvsd_read) != FS_OK) fail("NO FAT OR EXFAT ON THE CARD", NULL, NULL, GAME ": the card didn't mount");
    cvd_stage(2);
    watchdog_enable(WATCHDOG_MS, true);
    uint64_t found = ppcard_load_roms(fs, spare, room_n, card_tick);
    cvd_stage(3);
    if (found != PP_ROMS_ALL) {
        int missing = 0;
        const char *first = "";
        for (int i = 0; i < PP_ROM_FILES; i++)
            if (!(found & 1ull << i) && !missing++) first = pp_rom_files[i].name;
        char l2[40], note[48];
        if (!found) snprintf(l2, sizeof l2, "NONE OF ITS %d FILES IS THERE", PP_ROM_FILES);
        else snprintf(l2, sizeof l2, "%d OF %d MISSING: %.14s", missing, PP_ROM_FILES, first);
        for (char *c = l2; *c; c++)
            if (*c >= 'a' && *c <= 'z') *c = (char)(*c - 'a' + 'A');
        snprintf(note, sizeof note, GAME ": %d of %d ROMs missing", missing, PP_ROM_FILES);
        bool mcus = (found & PP_ROMS_GAME) == PP_ROMS_GAME;
        fail(mcus ? "PUT NAMCO51.ZIP TO NAMCO54.ZIP," : "PUT MAME'S POLEPOS.ZIP AND",
             mcus ? "OR 51XX-54XX.BIN, IN " PPCARD_HOME : "NAMCO51-54.ZIP IN " PPCARD_HOME, l2, note);
    }
    pproms_roms(&roms);
    memcpy(road, roms.road, sizeof road);
    roms.road = road;
    pp_init(&roms);
    ppcv_init(&enc, &roms);
    pprun_start(&run, &enc);
    cvd_stage(4);
    cvf_note(GAME);

    while (list_pending) tasks(), publish();          // the last message's list out first
    frames_at_start = cvf_frames();
    for (;;) {
        game_list();
        update_note();
    }
}
