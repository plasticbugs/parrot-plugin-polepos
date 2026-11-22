// pprun: the cart's loop, a list at a time, shared by the plugin and the
// Mac tests so both run the board the same way. Portable. (Pac-Man's
// pmrun.c.)
//
// Each list runs the board for the list's time (PPCV_LIST_T of the
// console's T: 16.77 ms, a little more than the board's 16.50 ms frame), to
// a cycle count kept from the start so nothing drifts, plus PPRUN_LEAD so a
// list always finds its samples made; PPCV_PCM_N samples of the sound go
// with it, or while the 52xx talks its 8 kHz samples, each at its place
// (ppsnd_take_list, ppcv_speech). The picture is ppcv's: it copies the RAM it needs at each VBLANK
// (pp_on_vblank) and works on the newest copy a list at a time.
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "pp.h"
#include "ppcv.h"

#define PPRUN_LEAD 2600                 // Z80 cycles: 0.85 ms, about 5 samples

typedef struct {
    ppcv_t *cv;
    uint32_t lists;                     // since the start
    uint64_t target;                    // the Z80 cycle this list runs to
    uint32_t underruns;                 // samples a list had to make up
    int16_t pcm[PPCV_SPEECH_MAX];       // the list's samples (PPCV_PCM_N; speech's: up to PPCV_SPEECH_MAX)
    uint16_t pos[PPCV_SPEECH_MAX];      // speech's: each one's place in the list
} pprun_t;

// the board started (pp_init done) and the sound's rate set with it
void pprun_start(pprun_t *r, ppcv_t *cv);
// one list: the board for its time, then the list into out; returns its length
size_t pprun_list(pprun_t *r, uint8_t *out);
