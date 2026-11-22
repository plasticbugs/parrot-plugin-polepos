// ppsnd.h -- Pole Position's sound, as MAME 0.288 makes it: the 8-voice
// WSG (namco.cpp's polepos_wsg_device), the engine (polepos_a.cpp) and the
// discrete circuits after the 52xx's DAC and the 54xx's outputs. The board
// (pp.c) calls these at the time of each write (pp_now()). See ppsnd.c.
#ifndef PPSND_H
#define PPSND_H

#include <stdbool.h>
#include <stdint.h>

#include "pp.h"

void ppsnd_init(const pp_roms_t *r);
void ppsnd_reset(void);
void ppsnd_update(void);                // the sound manager's 50 Hz update
void ppsnd_enable(int state);           // the latch's CLSON: WSG sound enable, engine clear
void ppsnd_wsg(int offset, uint8_t d);  // 0x83c0-0x83ff
void ppsnd_engine(int msb, uint8_t d);  // 0xa200 (lsb) and 0xa300 (msb)
void ppsnd_52xx(uint8_t d);             // the 52xx's P: its DAC
void ppsnd_54xx(int n, uint8_t d);      // the 54xx's three outputs

// The output: mono samples, MAME's -1..1 as +-32767, each the mean over
// its time (a box). Set the rate once (from the next sample on; 800 Hz to
// 48 kHz, << 8); then after each list's pp_run_to, take what is done: about
// rate / list rate a list (100 at PPCV_RATE_Q8, now and then 99 or 101),
// fewer than max left for the next take. The board's calls above come from
// pp.c by themselves. ppsnd_update keeps the streams under 21 ms apart.
#define PPSND_LIST_RATE_Q8 ((uint32_t)(100ull * 3579545u * 256u / 60036u))   // 100 a list
void ppsnd_rate_q8(uint32_t hz_q8);
int ppsnd_take(int16_t *out, int max);

// The same for the cart's run loop (pprun.c), with the speech: call it
// instead of ppsnd_take, once a list, after the list's pp_run_to (from the
// first call the 52xx is kept apart). Usually it is ppsnd_take: at most
// max samples (100 a list: hold the last when short) and *speech false;
// pos isn't set. While the 52xx talks, and 0.3 s after, *speech is true:
// the samples are 8 kHz (134 or 135 a list, twice the 52xx's 4 kHz), the
// 52xx's over the rest at half its usual level, and pos[i] is where in
// the list sample i is for, in the console's T from its start (0 to
// 60035, rising, the list's time as it is). out and pos must have room
// for PPSND_SPEECH_MAX. The player plays speech samples
// PPSND_SPEECH_GAIN_Q8 / 256 louder than the others (ppcv_speech).
#define PPSND_SPEECH_MAX 140
#define PPSND_SPEECH_GAIN_Q8 410        // 1.6
int ppsnd_take_list(int16_t *out, uint16_t *pos, int max, bool *speech);

extern unsigned ppsnd_parts;            // for the tests: the parts heard (1 WSG, 2 engine, 4 54xx, 8 52xx)
// for the tests: each of the calls above as it comes, before it is handled
enum { PPSND_EV_RESET, PPSND_EV_UPDATE, PPSND_EV_ENABLE, PPSND_EV_WSG, PPSND_EV_ENGINE, PPSND_EV_52XX, PPSND_EV_54XX };
extern void (*ppsnd_tap)(int what, int a, int d);

#endif
