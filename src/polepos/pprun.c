// pprun.c -- see pprun.h.
#include <string.h>

#include "pprun.h"
#include "ppsnd.h"

static pprun_t *running;

PPCV_HOT static void vblank(void) {
    if (running) ppcv_vblank(running->cv);
}

void pprun_start(pprun_t *r, ppcv_t *cv) {
    memset(r, 0, sizeof *r);
    r->cv = cv;
    running = r;
    pp_on_vblank = vblank;
    ppsnd_rate_q8(PPCV_RATE_Q8);
    r->target = pp_time();
}

PPCV_HOT size_t pprun_list(pprun_t *r, uint8_t *out) {
    r->lists++;
    r->target = ((uint64_t)r->lists * PPCV_LIST_CYCLES_Q16 >> 16) + PPRUN_LEAD;
    pp_run_to(r->target);
    bool speech;
    int n = ppsnd_take_list(r->pcm, r->pos, PPCV_PCM_N, &speech);
    if (speech) ppcv_speech(r->cv, r->pcm, r->pos, n);
    else {
        if (n < PPCV_PCM_N) {           // short: the last sample held
            r->underruns += (uint32_t)(PPCV_PCM_N - n);
            int16_t v = n ? r->pcm[n - 1] : 0;
            while (n < PPCV_PCM_N) r->pcm[n++] = v;
        }
        ppcv_sound(r->cv, r->pcm, n);
    }
    return ppcv_tv_frame(r->cv, out);
}
