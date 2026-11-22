// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, AJR, Couriersud, Ernesto Corvi
//
// pp.c -- the Namco Pole Position board as MAME 0.288 emulates it, in C for
// the cart:
//
//   polepos.cpp (Nicola Salmoria and others): the memory maps, the inputs,
//     the interrupts, the steering and the customs' wiring
//   namco06.cpp, namco51.cpp, namco52.cpp, namco53.cpp, namco54.cpp: the
//     06xx bus controller and the four MCU customs' glue
//   74259.cpp (AJR), watchdog.cpp (Aaron Giles), adc0804.cpp (AJR): the
//     latch, the watchdog and the pedals' A/D converter
//   schedule.cpp, diexec.cpp, screen.cpp, timer.cpp (Aaron Giles and
//     others): MAME's timeslices for its seven CPUs, its timers, the
//     input line event queues, VBLANK and the scanline timer
//
// The CPUs (z80.c, z8k.c, mb88.c) run in MAME's order and timeslices: up to
// the next timer, each from where it stopped, a write that adds a timer at
// the head of the list ending the timeslice as it does in MAME. MAME's
// license (BSD-3-Clause) is in README.md.
//
// Built with PP_EXACT, all seven CPUs are scheduled as MAME schedules them,
// and every frame's RAM and registers are MAME's (the tests). Without it,
// the cart's way: only the Z80 and the two Z8002s are scheduled. The 51xx
// (coins, credits, buttons) and the 53xx (the wheel) answer the 06xx as
// their programs do, from the inputs, without running (h51, h53 below: the
// bytes are the ones the real programs give, test/iocmp.sh). The 52xx and
// the 54xx, which only make sound, run their programs out of the
// timeslices: what the Z80 sends them (their IRQs, the bytes, the resets)
// is queued with its time, and they catch up, together, at each 50 Hz
// sound update and at the end of each run, so the sound has them in time.
// And the scanline timer stops only at the lines that do something (64,
// 192, 240), not at every one, so the timeslices are fewer and longer: the
// CPUs meet less often, the game's RAM drifts from MAME's by a frame here
// and there, and the picture is MAME's in about four frames in five (the
// rest a few pixels off: test/ppv_host.c -off -1 over ppscript.py drive).
#include "pp.h"

#include <string.h>

#include "mb88.h"
#include "ppsnd.h"
#include "z80.h"
#include "z8k.h"

#if defined(PICO_ON_DEVICE) && PICO_ON_DEVICE
#define PP_HOT __attribute__((section(".time_critical.pp")))
#else
#define PP_HOT
#endif

// ---------------------------------------------------------------------------
// attotime (src/emu/attotime.h), held as attoseconds from an epoch that
// pp_frame moves on a second at a time (ep_s), so that a time is one 64-bit
// number. MAME's sums and comparisons of attotimes are exact, and so are
// these; a time can be a little before the epoch (the last VBLANK's start).

typedef int64_t at_t;                   // a time (from the epoch) or a duration
#define AS_PER_SEC 1000000000000000000LL
#define AT_NEVER INT64_MAX
static int32_t ep_s;                    // the epoch: attotime(ep_s, 0)

PP_HOT static inline at_t at_add(at_t t, at_t d) { return (t == AT_NEVER || d == AT_NEVER) ? AT_NEVER : t + d; }
// floor(a * f / 1e18) for 0 <= a < 1e18, without 128-bit sums
PP_HOT static inline uint64_t mul_frac(uint64_t a, uint32_t f) {
	uint64_t ah = a / 1000000000u, al = a % 1000000000u;
	uint64_t x = ah * f, y = al * f;
	uint64_t q1 = x / 1000000000u, r1 = x % 1000000000u;
	return q1 + (r1 * 1000000000u + y) / (uint64_t)AS_PER_SEC;
}
PP_HOT static inline pp_at_t at_abs(at_t t) {  // as MAME's (seconds, attoseconds)
	pp_at_t r = { ep_s, t };
	while (r.a >= AS_PER_SEC) { r.a -= AS_PER_SEC; r.s++; }
	while (r.a < 0) { r.a += AS_PER_SEC; r.s--; }
	return r;
}
PP_HOT static inline uint64_t at_ticks(at_t t, uint32_t f) {                  // as_ticks
	pp_at_t a = at_abs(t);
	return (uint64_t)a.s * f + mul_frac((uint64_t)a.a, f);
}
PP_HOT static inline at_t at_from_ticks(uint64_t ticks, uint32_t f) {         // from_ticks
	int64_t apt = AS_PER_SEC / f;
	return ((int64_t)(ticks / f) - ep_s) * AS_PER_SEC + (int64_t)(ticks % f) * apt;
}

// ---------------------------------------------------------------------------
// the machine's constants

#define HZ_AS(f) (AS_PER_SEC / (f))     // HZ_TO_ATTOSECONDS
#define PIX_AS HZ_AS(6144000)           // MASTER_CLOCK / 4
#define HTOTAL 384
#define VTOTAL 264
#define VBSTART 240                     // the visible area's bottom + 1
#define SCAN_AS (PIX_AS * HTOTAL)
#define FRAME_AS (SCAN_AS * VTOTAL)
#define VBLANK_AS (SCAN_AS * (VTOTAL - 224))
#define QUANTUM_AS HZ_AS(60)            // no machine sets a smaller one
#define N06_CLOCK 48000u                // MASTER_CLOCK / 8 / 64
#define ADC_CLOCK 384000u               // MASTER_CLOCK / 8 / 8
#define MCU_CLOCK 1536000u              // MASTER_CLOCK / 8 / 2, divided by 6 inside

enum { D_Z80, D_SUB1, D_SUB2, D_51, D_52, D_53, D_54, NDEV };
#ifdef PP_EXACT
#define NSCHED NDEV                     // the devices in the timeslices
#else
#define NSCHED 3
#endif

static const pp_roms_t *roms;
pp_inputs_t pp_in = { PP_IN0, PP_DSWA, PP_DSWB, 0, 0, 0 };
uint16_t pp_sprite[0x800], pp_road[0x400], pp_alpha[0x400], pp_view[0x800];
uint16_t pp_hscroll, pp_vscroll;
uint8_t pp_nvram[0x800], pp_z80ram[0x400];
uint8_t pp_latch, pp_chacl;
uint32_t pp_frames, pp_resets;
void (*pp_on_vblank)(void);
pp_stats_t pp_stats;

// the CPUs' programs, in SRAM (the ROMs' store may be in the PSRAM), and
// where the Z8002s' writes to ROM go; before pp_init, room the plugin
// borrows (pp_spare)
static struct {
	uint16_t sub[2][0x2000];            // the Z8002s' 16K of ROM as words
	uint8_t main[0x3000];               // the Z80's
	uint8_t mcu[4][0x400];              // the customs'
	uint16_t sink_page[0x400];
} sram __attribute__((aligned(8)));
static const uint16_t zero_page[0x400]; // 0x4000-0x7fff of their ROM regions (ROMREGION_ERASE00)

void *pp_spare(uint32_t *n) { *n = (uint32_t)sizeof sram; return &sram; }
static uint8_t sink[0x100];
static z8k_t sub[2];
static mb88_t mcu[4];

// ---------------------------------------------------------------------------
// the scheduler (schedule.cpp, diexec.cpp)

#define SUSPEND_RESET 0x0004            // SUSPEND_REASON_RESET

typedef struct {
	at_t local;                         // m_localtime
	at_t ready;                         // local + aspc: a timeslice to this or later runs it
	int64_t aspc;                       // m_attoseconds_per_cycle
	uint32_t cps;                       // m_cycles_per_second
	int divshift;
	uint32_t divisor;
	float inv;                          // 1 / divisor (div_cycles)
	uint32_t total;                     // m_totalcycles (to 23 minutes; the tests' traces)
	bool started;                       // total isn't 0
	int64_t aspclk;                     // the device's m_attoseconds_per_clock
	uint32_t clock;
	int cpc;                            // clocks a cycle (cycles_to_clocks)
	uint32_t suspend, nextsuspend;
	bool eat, nexteat;
	bool go;                            // suspend == 0 || eat
} exec_t;

static exec_t ex[NDEV];
static int exec_list[NDEV];
static bool all_run;                    // none suspended: the list is 0-6 (timeslice's quick way)
static int cur = -1;                    // the device executing
static bool suspend_pending;

static void exec_init(exec_t *e, uint32_t clock, int cpc) {
	e->local = 0;
	e->started = false;
	e->clock = clock;
	e->cpc = cpc;
	e->aspclk = HZ_AS(clock);
	e->cps = (clock + (uint32_t)cpc - 1) / (uint32_t)cpc;
	e->aspc = HZ_AS(e->cps);
	int64_t attos = e->aspc;
	e->divshift = 0;
	while (attos >= (1LL << 31)) { e->divshift++; attos >>= 1; }
	e->divisor = (uint32_t)attos;
	e->inv = 1.0f / (float)e->divisor;
	e->total = 0;
	e->suspend = e->nextsuspend = 0;
	e->eat = e->nexteat = false;
	e->go = true;
	e->ready = e->local + e->aspc;
}

// divu_64x32(x, divisor), x < 2^47: a float guess, made exact (the M33 has
// no 64-bit divide; the library's takes a hundred cycles)
PP_HOT static inline uint32_t div_cycles(uint64_t x, const exec_t *e) {
#if defined(__arm__) || defined(PP_FLOAT_DIV)
	float xf = (float)(uint32_t)(x >> 24) * 16777216.0f + (float)(uint32_t)(x & 0xffffff);
	uint32_t q = (uint32_t)(xf * e->inv);
	int64_t r = (int64_t)x - (int64_t)((uint64_t)q * e->divisor);
	while (r < 0) { q--; r += e->divisor; }
	while (r >= (int64_t)e->divisor) { q++; r -= e->divisor; }
	return q;
#else
	return (uint32_t)(x / e->divisor);
#endif
}

PP_HOT static inline at_t cycles_at(const exec_t *e, uint64_t c) {            // the scheduler's deltatime
	if (c < e->cps) return (int64_t)c * e->aspc;
	at_t r = (int64_t)(c / e->cps) * AS_PER_SEC + (int64_t)(c % e->cps) * e->aspc;
	return r;
}

PP_HOT static inline at_t clocks_at(const exec_t *e, uint64_t n) {            // device_t::clocks_to_attotime
	if (n < e->clock) return (int64_t)n * e->aspclk;
	at_t r = (int64_t)(n / e->clock) * AS_PER_SEC + (int64_t)(n % e->clock) * e->aspclk;
	return r;
}

PP_HOT static inline int dev_run(int d, int cycles) {
	switch (d) {
	case D_Z80: return z80_run(cycles);
	case D_SUB1: case D_SUB2: return z8k_run(&sub[d - D_SUB1], cycles);
	default: return mb88_run(&mcu[d - D_51], cycles);
	}
}

PP_HOT static inline int dev_done(int d) {
	switch (d) {
	case D_Z80: return z80_done();
	case D_SUB1: case D_SUB2: return z8k_done();
	default: return mb88_done();
	}
}

PP_HOT static void abort_timeslice(void) {
	switch (cur) {
	case -1: break;
	case D_Z80: z80_abort(); break;
	case D_SUB1: case D_SUB2: z8k_abort(); break;
	default: mb88_abort(); break;
	}
}

PP_HOT static void rebuild_execute_list(void) {
	int n = 0;
	for (int d = 0; d < NSCHED; d++) if (ex[d].suspend == 0) exec_list[n++] = d;
	all_run = n == NSCHED;
	for (int d = 0; d < NSCHED; d++) if (ex[d].suspend != 0) exec_list[n++] = d;
}

PP_HOT static void suspend_dev(int d, uint32_t reason, bool eat) {
	ex[d].nextsuspend |= reason;
	ex[d].nexteat = eat;
	suspend_pending = true;
	if (cur == d) abort_timeslice();
}

PP_HOT static void resume_dev(int d, uint32_t reason) {
	ex[d].nextsuspend &= ~reason;
	suspend_pending = true;
	if (cur == d) abort_timeslice();
}

PP_HOT static void apply_suspend_changes(void) {
	uint32_t changed = 0;
	for (int k = 0; k < NSCHED; k++) {
		exec_t *e = &ex[exec_list[k]];
		changed |= e->suspend ^ e->nextsuspend;
		e->suspend = e->nextsuspend;
		e->eat = e->nexteat;
		e->go = e->suspend == 0 || e->eat;
	}
	if (changed) rebuild_execute_list();
	else suspend_pending = false;
}

// ---------------------------------------------------------------------------
// timers: MAME's list, in expiry order, a new one after those that expire with it

enum {
	T_SND, T_VBSTART, T_VBEND, T_SCAN0, T_SCANLINE, T_06XX, T_ADC, T_SER0, T_SER3 = T_SER0 + 3,
	T_SYNC,                             // temporary timers (synchronize()) from here
	T_COUNT = T_SYNC + 48
};
enum {                                  // what a sync timer does
	SY_LINE,                            // an input line's event queue (param: the line)
	SY_06_DATA, SY_06_CTRL,             // namco_06xx write_sync, ctrl_w_sync
	SY_51_RW, SY_51_WRITE, SY_51_O,     // namco_51xx rw_sync, write_sync, O_w_sync
	SY_52_WRITE, SY_54_WRITE,
};

typedef struct {
	at_t expire, period;
	int32_t param;
	bool enabled, listed;
	uint8_t type;
	int8_t prev, next;
} tmr_t;

static tmr_t tm[T_COUNT];
static int tl_head = -1;
static int sync_free;                   // the free sync timers, linked by next
static at_t basetime;
static int cb_timer = -1;               // the timer whose callback runs
static at_t cb_expire;
static bool cb_modified;

// machine().time()
PP_HOT static at_t now(void) {
	if (cb_timer >= 0) return cb_expire;
	if (cur >= 0) return ex[cur].local + clocks_at(&ex[cur], (uint64_t)dev_done(cur) * (uint64_t)ex[cur].cpc);
	return basetime;
}

PP_HOT pp_at_t pp_now(void) { return at_abs(now()); }

PP_HOT static void tl_remove(int i) {
	tmr_t *t = &tm[i];
	if (!t->listed) return;
	if (t->prev >= 0) tm[t->prev].next = t->next;
	else tl_head = t->next;
	if (t->next >= 0) tm[t->next].prev = t->prev;
	t->listed = false;
}

PP_HOT static void tl_insert(int i) {
	tmr_t *t = &tm[i];
	if (t->expire == AT_NEVER || !t->enabled) return;       // MAME's inactive list
	int prev = -1, c = tl_head;
	while (c >= 0 && tm[c].expire <= t->expire) { prev = c; c = tm[c].next; }
	t->prev = (int8_t)prev;
	t->next = (int8_t)c;
	if (prev >= 0) tm[prev].next = (int8_t)i;
	else tl_head = i;
	if (c >= 0) tm[c].prev = (int8_t)i;
	t->listed = true;
}

PP_HOT static inline at_t head_expire(void) { return tl_head < 0 ? AT_NEVER : tm[tl_head].expire; }

PP_HOT static void tm_adjust(int i, at_t delay, int32_t param, at_t period) {  // emu_timer::adjust
	tmr_t *t = &tm[i];
	if (cb_timer == i) cb_modified = true;
	t->param = param;
	t->enabled = true;
	if (delay < 0) delay = 0;
	t->expire = at_add(now(), delay);
	t->period = period;
	tl_remove(i);
	tl_insert(i);
	if (tl_head == i) abort_timeslice();
}

PP_HOT static void synchronize(int type, int32_t param) {
	int i = sync_free;
	if (i < 0) {                        // (more than MAME would have at once: can't happen)
		for (;;) {}
	}
	sync_free = tm[i].next;
	tmr_t *t = &tm[i];
	t->type = (uint8_t)type;
	t->param = param;
	t->period = AT_NEVER;
	t->expire = now();
	t->enabled = true;
	t->listed = false;
	tl_insert(i);
	pp_stats.syncs++;
	if (tl_head == i) abort_timeslice();
}

// ---------------------------------------------------------------------------
// input lines (device_execute_interface::device_input): each line's events
// wait in its queue for a timer at now, which applies them all

enum {
	L_Z80_IRQ, L_Z80_NMI,
	L_SUB1_NVI, L_SUB1_RESET, L_SUB2_NVI, L_SUB2_RESET,
	L_51_IRQ, L_51_TC, L_51_RESET, L_52_IRQ, L_52_RESET, L_53_IRQ, L_53_RESET, L_54_IRQ, L_54_RESET,
	NLINES
};
static uint8_t line_dev[NLINES] = {
	D_Z80, D_Z80, D_SUB1, D_SUB1, D_SUB2, D_SUB2, D_51, D_51, D_51, D_52, D_52, D_53, D_53, D_54, D_54
};
static struct { uint8_t q[32]; uint8_t n; } lq[NLINES];

static void line_apply(int l, int state);
#ifndef PP_EXACT
static void mq_push(int k, uint8_t kind, uint8_t v);
static uint8_t mq_line[2] = {0xff, 0xff};               // the 52xx's and 54xx's IRQ as last queued
static void h51_vblank(void);
#endif

PP_HOT static void set_line(int l, int state) {                // set_state_synced
#ifndef PP_EXACT
	switch (l) {                        // the customs: queued (52xx, 54xx) or no program (51xx, 53xx)
	case L_52_IRQ: case L_54_IRQ: {
		int k = l == L_54_IRQ;
		if (mq_line[k] != state) { mq_line[k] = (uint8_t)state; mq_push(k, 0, (uint8_t)state); }
		return;
	}
	case L_52_RESET: case L_54_RESET: mq_push(l == L_54_RESET, 2, (uint8_t)state); return;
	case L_51_IRQ: case L_51_TC: case L_51_RESET: case L_53_IRQ: case L_53_RESET: return;
	default: break;
	}
#endif
	if (lq[l].n >= sizeof lq[l].q) {                    // full: applied now (MAME logs it)
		for (int i = 0; i < lq[l].n; i++) line_apply(l, lq[l].q[i]);
		lq[l].n = 0;
	}
	lq[l].q[lq[l].n++] = (uint8_t)state;
	if (lq[l].n == 1) synchronize(SY_LINE, l);
}

PP_HOT static void line_events(int l) {                        // empty_event_queue
	for (int i = 0; i < lq[l].n; i++) line_apply(l, lq[l].q[i]);
	lq[l].n = 0;
}

static void dev_reset(int d);

PP_HOT static void line_apply(int l, int state) {
	int d = line_dev[l];
	switch (l) {
	case L_SUB1_RESET: case L_SUB2_RESET: case L_51_RESET: case L_52_RESET: case L_53_RESET: case L_54_RESET:
		if (state) suspend_dev(d, SUSPEND_RESET, true);
		else if (ex[d].nextsuspend & SUSPEND_RESET) {
			dev_reset(d);
			resume_dev(d, SUSPEND_RESET);
		}
		return;
	case L_Z80_IRQ: z80_set_irq(state); return;
	case L_Z80_NMI: z80_set_nmi(state, ex[D_Z80].started || cur == D_Z80); return;
	case L_SUB1_NVI: z8k_set_input(&sub[0], Z8K_NVI_LINE, state); return;
	case L_SUB2_NVI: z8k_set_input(&sub[1], Z8K_NVI_LINE, state); return;
	case L_51_IRQ: mb88_set_input(&mcu[0], MB88_IRQ_LINE, state); return;
	case L_51_TC: mb88_set_input(&mcu[0], MB88_TC_LINE, state); return;
	case L_52_IRQ: mb88_set_input(&mcu[1], MB88_IRQ_LINE, state); return;
	case L_53_IRQ: mb88_set_input(&mcu[2], MB88_IRQ_LINE, state); return;
	case L_54_IRQ: mb88_set_input(&mcu[3], MB88_IRQ_LINE, state); return;
	}
}

// device_t::reset() of a CPU: interface_pre_reset (the cycle count, resume), device_reset
PP_HOT static void dev_reset(int d) {
	ex[d].total = 0;
	ex[d].started = false;
	resume_dev(d, ~0u);
	switch (d) {
	case D_Z80: z80_reset(); break;
	case D_SUB1: case D_SUB2: z8k_reset(&sub[d - D_SUB1]); break;
	default: mb88_reset(&mcu[d - D_51]); break;
	}
}

// ---------------------------------------------------------------------------
// the screen (screen.cpp) and the scanline timer (timer.cpp)

static at_t vblank_start_time, vblank_end_time;
static bool scan_first;
static bool frame_done, watchdog_fired;
static int watchdog_counter;
static uint8_t sub_irq_mask;

PP_HOT static at_t time_until_pos(int v) {
	v += VTOTAL - VBSTART;
	v %= VTOTAL;
	int64_t target = (int64_t)v * SCAN_AS;
	int64_t curd = now() - vblank_start_time;
	if (target <= curd + PIX_AS / 2) target += FRAME_AS;
	while (target <= curd) target += FRAME_AS;
	return target - curd;
}

PP_HOT static int vpos(void) {
	int64_t delta = now() - vblank_start_time;
	delta += PIX_AS / 2;
	int v = (int)(delta / SCAN_AS);
	return (VBSTART + v) % VTOTAL;
}

PP_HOT static void cb_vbstart(void) {          // vblank_begin
	vblank_start_time = now();
	vblank_end_time = vblank_start_time + VBLANK_AS;
	pp_frames++;                        // machine().video().frame_update()
	frame_done = true;
#ifndef PP_EXACT
	h51_vblank();                       // (its timer counts VBLANKs; it takes the inputs it read before this one's)
#endif
	if (pp_on_vblank) pp_on_vblank();
	if (--watchdog_counter == 0) watchdog_fired = true;  // the watchdog's VBLANK callback
#ifdef PP_EXACT
	set_line(L_51_TC, 0);               // screen_vblank(1): namco_51xx vblank -> TC clear
#endif
	tm_adjust(T_VBSTART, time_until_pos(VBSTART), 0, AT_NEVER);
	tm_adjust(T_VBEND, vblank_end_time - now(), 0, AT_NEVER);
}

PP_HOT static void cb_vbend(void) {
	set_line(L_51_TC, 1);               // screen_vblank(0): TC asserted
}

PP_HOT static void cb_scanline(void) {         // timer_device::scanline_tick, polepos_state::scanline
	int next = 0;
	if (!scan_first) {
		int v = vpos();
		if ((v == 64 || v == 192) && (pp_latch & 1)) set_line(L_Z80_IRQ, 1);
		if (v == 240 && sub_irq_mask) {
			set_line(L_SUB1_NVI, 1);
			set_line(L_SUB2_NVI, 1);
		}
#ifdef PP_EXACT
		if (v + 1 < VTOTAL) next = v + 1;
#else
#ifdef PP_SCANSTEP
		next = v + PP_SCANSTEP;
		if (v < 64 && next > 64) next = 64;
		else if (v < 192 && next > 192) next = 192;
		else if (v < 240 && next > 240) next = 240;
		if (next >= VTOTAL) next = 0;
#else
		next = v < 64 ? 64 : v < 192 ? 192 : v < 240 ? 240 : 64;   // (only these lines do anything)
#endif
#endif
	}
	scan_first = false;
	tm_adjust(T_SCANLINE, time_until_pos(next), 0, AT_NEVER);
}

#ifdef PP_DEBUG
#include <stdio.h>
FILE *pp_dbg;
#define DBG(...) do { if (pp_dbg) fprintf(pp_dbg, __VA_ARGS__); } while (0)
#else
#define DBG(...) ((void)0)
#endif

// ---------------------------------------------------------------------------
// the 06xx (namco06.cpp) and the customs' glue (namco5x.cpp, polepos.cpp)

static uint8_t n06_control;
static bool n06_timer_state, n06_read_stretch;
static uint8_t n51_portO, n51_rw, n52_cmd, n53_portO, n54_cmd;
#ifdef PP_EXACT
#define N54_OUT(c, d) ppsnd_54xx(c, d)
#else
static void q54_out(int c, uint8_t d);
#define N54_OUT(c, d) q54_out(c, d)
#endif
static uint16_t n52_address;
static uint8_t steer_last, steer_delta;
static int16_t steer_accum;
static uint8_t adc_input, auto_start_mask;
static bool adc_intr;
static uint8_t adc_result;

PP_HOT static void n06_set_nmi(int state) { set_line(L_Z80_NMI, state); }    // the Z80 is never suspended
#ifdef PP_IOLOG
#include <stdio.h>
FILE *pp_iolog;
#define IOLOG(...) do { if (pp_iolog) fprintf(pp_iolog, __VA_ARGS__); } while (0)
#else
#define IOLOG(...) do { } while (0)
#endif

static uint8_t in0_port(void);

#ifndef PP_EXACT
// The 51xx without its program: the bytes it gives the 06xx, as its
// program gives them (worked out from the program's answers in the exact
// build, test/iocmp.sh; the commands as namco51.cpp lists them). Its
// inputs are sampled at each VBLANK, as its timer counts them: a coin or a
// start that came on since the last is taken then, and a button's edge is
// shown until the next. MAME's default DIP switches only (the bytes that
// carry DSWB were seen with those).
static struct {
	uint8_t mode;                       // 0 switch (command 5), 1 credits (2)
	bool game, game_next;               // a game started (until command 2): the buttons' bytes live; from the next VBLANK
	uint8_t b[3];                       // the buttons' bytes as last made (kept between games)
	uint8_t args;                       // command 1's arguments still to come
	uint8_t coinage[4];                 // coins a credit, credits a coin; for each slot
	uint8_t coins[2], credits;
	uint8_t was;                        // what was pressed at the last VBLANK (IN0 inverted)
	uint8_t edge;                       // came on at the last VBLANK
	uint8_t idx;                        // the next byte of a read
	uint8_t next;                       // the byte it has made ready
} h51;

static void h51_reset(void) {
	memset(&h51, 0, sizeof h51);
	h51.coinage[0] = h51.coinage[1] = h51.coinage[2] = h51.coinage[3] = 1;
	h51.b[1] = pp_in.dswb;              // (what its program gives before the first game)
	h51.b[2] = 0xff;
}

PP_HOT static void h51_write(uint8_t d) {
	if (h51.args) { h51.coinage[4 - h51.args--] = d; return; }
	switch (d & 7) {
	case 1: h51.args = 4; break;
	case 2: h51.mode = 1; h51.game = h51.game_next = false; break;
	case 5: h51.mode = 0; break;
	default: break;                     // 0, 6, 7 nothing; 3, 4 the joystick's remapping (polepos has none)
	}
}

PP_HOT static void h51_vblank(void) {
	uint8_t in = (uint8_t)~in0_port(), press = in & (uint8_t)~h51.was;
	h51.was = in;
	h51.edge = press & 3;
	if (h51.mode == 0) return;
	if (h51.game_next) { h51.game = true; h51.game_next = false; }
	for (int k = 0; k < 2; k++)
		if ((press & (0x10 << k)) && h51.credits < 99) {
			uint8_t need = h51.coinage[2 * k] ? h51.coinage[2 * k] : 1;
			if (++h51.coins[k] >= need) { h51.coins[k] = 0; h51.credits += h51.coinage[2 * k + 1]; }
		}
	if ((press & 0x40) && h51.credits < 99) h51.credits++;              // service: a credit
	if (h51.credits > 99) h51.credits = 99;
	if ((press & 0x04) && h51.credits >= 1) {                           // the program's start
		h51.credits--;
		h51.game_next = true;
	}
}

PP_HOT static uint8_t h51_byte(int k) {
	uint8_t in0 = in0_port(), dsw = pp_in.dswb;
	if (h51.mode == 0) return k == 0 ? in0 : k == 1 ? dsw : 0xff;
	if (k == 0) return (uint8_t)((h51.credits / 10) << 4 | h51.credits % 10);
	if (h51.game) {                                                     // the buttons: IN0 bits 0, 1
		int b = k - 1;
		h51.b[k] = (uint8_t)((k == 1 ? dsw & 0x0f : dsw >> 4) | (!(h51.edge >> b & 1)) << 4 | (in0 >> b & 1) << 5);
	}
	return h51.b[k];
}

// The 53xx without its program, as polepos wires it (mode 0): the wheel's
// count, which follows the dial as the program follows its pulses (as fast
// as it can: about 30.5 a frame), the three steering inputs that aren't
// there, and the DIP switch bytes as the program gives them for MAME's
// defaults.
#define H53_RATE 40347u                 // counts a Z80 cycle, << 26: 30.5 a frame of 50688
static struct { uint8_t count, last, idx, next; int16_t owed; uint32_t room; at_t at; } h53;

static void h53_reset(void) {
	h53.count = h53.idx = h53.next = 0;
	h53.owed = 0;
	h53.room = 0;
	h53.last = pp_in.steer;
	h53.at = now();
}

PP_HOT static uint8_t h53_byte(int k) {
	static uint8_t dsw[4] = {0xff, 0xff, 0xf0, 0xff};
	if (k == 0) {
		at_t t = now();
		uint64_t cyc = (uint64_t)(t - h53.at) / (uint64_t)HZ_AS(PP_CLOCK);
		h53.at = t;
		uint64_t room = (h53.owed ? h53.room : 0) + cyc * H53_RATE;      // (it can't save up while the wheel is still)
		if (room > (64ull << 26)) room = 64ull << 26;
		h53.owed += (int8_t)(pp_in.steer - h53.last);
		h53.last = pp_in.steer;
		int can = (int)(room >> 26), step = h53.owed > can ? can : h53.owed < -can ? -can : h53.owed;
		h53.count = (uint8_t)(h53.count + step);
		h53.owed = (int16_t)(h53.owed - step);
		h53.room = (uint32_t)(room - ((uint64_t)(step < 0 ? -step : step) << 26));
		return h53.count;
	}
	return k < 4 ? 0 : dsw[k - 4];
}

// The 52xx and the 54xx out of the timeslices: what reaches them, queued
// with its time; at a catch-up both run to it, a little at a time each, the
// one behind first, so their writes to the sound come in time order.
enum { EV_IRQ, EV_CMD, EV_RESET };
typedef struct { at_t t; uint8_t kind, v; } mev_t;
#define MQ_N 64
static struct { mev_t q[MQ_N]; uint16_t head, n; bool held; } mq[2];      // [0] the 52xx, [1] the 54xx
#define MCU_STEP 64                     // cycles (250 us) a turn
int pp_mq_full;                     // (tests: catch-ups made early because a queue filled)
static void mcu_catch_up(at_t until);

PP_HOT static void mq_push(int k, uint8_t kind, uint8_t v) {
	if (mq[k].n == MQ_N) { pp_mq_full++; mcu_catch_up(now()); }
	if (mq[k].n == MQ_N) return;        // (can't be: the catch-up takes all before now)
	mev_t *e = &mq[k].q[(mq[k].head + mq[k].n++) % MQ_N];
	e->t = now();
	e->kind = kind;
	e->v = v;
}

// The 54xx's program runs on whether it sounds or not, making noise at no
// volume. Silent (its outputs all 0, the same for Q54_QUIET cycles), it
// stands still instead: each command's interrupt is taken and its handler
// run as it comes, and one that changes its RAM (a sound, its settings)
// sets it going again. What differs: the noise's phase when it next sounds.
#define Q54_QUIET 16384
static struct { bool still; uint8_t out[3]; uint32_t outs, seen, since; } q54;   // outs: the changes to the sound
static uint8_t q54_ram[64];             // its RAM as it took a command's interrupt (mb88's irq_snap)

PP_HOT static void q54_out(int c, uint8_t d) {
	if (q54.out[c] != d) { q54.out[c] = d; q54.outs++; }
	ppsnd_54xx(c, d);
}

PP_HOT static void q54_reset(void) {
	q54.still = false;
	q54.seen = q54.outs;
	q54.since = 0;
	mcu[3].stop_rti = 0;
	mcu[3].irq_snap = q54_ram;
}

PP_HOT static void q54_ran(exec_t *e, int ran) {
	e->total += (uint32_t)ran;
	e->started |= ran != 0;
	e->local += (int64_t)(uint32_t)ran * e->aspc;
}

// true if it took the turn (standing still, or a command's handler)
PP_HOT static bool q54_turn(exec_t *e, at_t until) {
	mb88_t *m = &mcu[3];
	if (m->stop_rti) {                  // a command's handler, run as the program runs it (it may wait for
		if (m->stop_rti != 3) return false;     // more bytes), to its RTI: the same RAM (but the registers
		m->stop_rti = 0;                // it keeps at 2 to 4) and outputs, still again
		bool same = q54.outs == q54.seen;
		for (int i = 0; i < 64 && same; i++)
			if ((i < 2 || i > 4) && m->ram[i] != q54_ram[i]) same = false;
		if (!same) {
			q54.seen = q54.outs;
			q54.since = e->total;
			return false;
		}
		q54.still = true;
	}
	if (!q54.still) {
		if (q54.outs != q54.seen) { q54.seen = q54.outs; q54.since = e->total; return false; }
		if (e->total - q54.since < Q54_QUIET || m->in_irq || q54.out[0] | q54.out[1] | q54.out[2]) return false;
		q54.still = true;
	}
	if ((m->pending_irq & m->pio) && !m->in_irq) {     // a command
		q54.still = false;
		m->stop_rti = 1;
		return false;
	}
	at_t to = mq[1].n && mq[1].q[mq[1].head].t < until ? mq[1].q[mq[1].head].t : until;
	int n = (int)div_cycles((uint64_t)(to - e->local) >> e->divshift, e) + 1;
	mb88_skipped[3] += (unsigned)n;
	q54_ran(e, n);
	return true;
}

PP_HOT static void mq_apply(int k, const mev_t *e) {
	int m = k ? 3 : 1;
	switch (e->kind) {
	case EV_IRQ: mb88_set_input(&mcu[m], MB88_IRQ_LINE, e->v); break;
	case EV_CMD: mb88_wake(&mcu[m]); if (k) n54_cmd = e->v; else n52_cmd = e->v; break;
	default:                            // the reset line: held, then let go (device_reset)
		IOLOG("%u X %d %d held %d\n", pp_frames, k, e->v, mq[k].held);
		if (e->v) mq[k].held = true;
		else if (mq[k].held) {
			mq[k].held = false;
			ex[D_51 + m].total = 0;
			ex[D_51 + m].started = false;
			mb88_reset(&mcu[m]);
			if (k) q54_reset();
		}
		break;
	}
}

PP_HOT static void mcu_catch_up(at_t until) {
	int saved = cb_timer, saved_cur = cur;
	cb_timer = -1;                      // (now() is the MCU's own time while it runs)
	for (;;) {
		int k = ex[D_52].local <= ex[D_54].local ? 0 : 1, d = k ? D_54 : D_52;
		exec_t *e = &ex[d];
		if (e->local >= until) break;   // (the one behind is there: both are)
		while (mq[k].n && mq[k].q[mq[k].head].t <= e->local) {
			mq_apply(k, &mq[k].q[mq[k].head]);
			mq[k].head = (uint16_t)((mq[k].head + 1) % MQ_N);
			mq[k].n--;
		}
		if (k && !mq[1].held && q54_turn(e, until)) continue;
		// a step at a time, but not when the other is waiting periodically or standing
		// still: it writes nothing until its next event (and one waiting so, nothing)
		at_t stop = e->local + MCU_STEP * e->aspc;
		if (mcu[d - D_51].pw) stop = until;
		else if (k ? mcu[1].pw : mcu[3].pw || q54.still) {
			stop = mq[!k].n ? mq[!k].q[mq[!k].head].t : until;
			if (stop < e->local + MCU_STEP * e->aspc) stop = e->local + MCU_STEP * e->aspc;
		}
		if (stop > until) stop = until;
		if (mq[k].n && mq[k].q[mq[k].head].t < stop) stop = mq[k].q[mq[k].head].t;
		int cycles = (int)div_cycles((uint64_t)(stop - e->local) >> e->divshift, e) + 1;
		int ran = cycles;
		if (!mq[k].held) {
			cur = d;
			ran = mb88_run(&mcu[d - D_51], cycles);
			cur = -1;
#ifndef PP_NO_TRACE
			pp_stats.runs[d]++;
			pp_stats.cycles[d] += (uint64_t)ran;
#endif
		}
		e->total += (uint32_t)ran;
		e->started |= ran != 0;
		e->local += (int64_t)(uint32_t)ran * e->aspc;
	}
	cur = saved_cur;
	cb_timer = saved;
}
#endif

PP_HOT static void cb_06xx(void) {             // nmi_generate
	n06_timer_state = !n06_timer_state;
	DBG("%lld 06xx %d 54total %llu\n", (long long)now(), n06_timer_state, (unsigned long long)ex[D_54].total);
#ifdef PP_EXACT
	if (n06_timer_state) synchronize(SY_51_RW, (n06_control >> 4) & 1);   // rw[0]: 51xx rw
#else
	// the 51xx's and 53xx's IRQ: the byte they made ready at the last is on
	// their port, and they make the next (the Z80 reads at this NMI the one
	// from the last: the first rising edge of a read has no NMI)
	if (n06_timer_state && (n06_control & 0x10)) {
		if (n06_control & 1) { n51_portO = h51.next; h51.next = h51.idx < 3 ? h51_byte(h51.idx++) : 0xff; }
		if (n06_control & 2) { n53_portO = h53.next; h53.next = h53.idx < 8 ? h53_byte(h53.idx++) : 0xff; }
	}
#endif
	n06_set_nmi(n06_timer_state && !n06_read_stretch);
	n06_read_stretch = false;
#ifdef PP_EXACT
	set_line(L_51_IRQ, (n06_control & 1) && n06_timer_state);
	set_line(L_53_IRQ, ((n06_control >> 1) & 1) && n06_timer_state);
#endif
	set_line(L_52_IRQ, ((n06_control >> 2) & 1) && n06_timer_state);
	set_line(L_54_IRQ, ((n06_control >> 3) & 1) && n06_timer_state);
}

PP_HOT static uint8_t n06_data_r(void) {
	if (!(n06_control & 0x10)) return 0;                // a read in write mode
	uint8_t r = 0xff;
	if (n06_control & 1) r &= n51_portO;
	if (n06_control & 2) r &= n53_portO;
	IOLOG("%u R %02x %02x in0 %02x as %u st %02x\n", pp_frames, n06_control, r, pp_in.in0, auto_start_mask, pp_in.steer);
	return r;                           // (the 52xx and 54xx have no read: 0xff)
}

PP_HOT static void n06_write_sync(uint8_t d) {
	if (n06_control & 0x10) return;     // a write in read mode
	IOLOG("%u W %02x %02x\n", pp_frames, n06_control, d);
#ifdef PP_EXACT
	if (n06_control & 1) synchronize(SY_51_WRITE, d);
	if (n06_control & 4) synchronize(SY_52_WRITE, d);
	if (n06_control & 8) synchronize(SY_54_WRITE, d);
#else
	if (n06_control & 1) h51_write(d);
	if (n06_control & 4) mq_push(0, EV_CMD, d);
	if (n06_control & 8) mq_push(1, EV_CMD, d);
#endif
}

PP_HOT static void n06_ctrl_sync(uint8_t d) {
	DBG("%lld ctrl %02x\n", (long long)now(), d);
	IOLOG("%u C %02x\n", pp_frames, d);
	n06_control = d;
#ifndef PP_EXACT
	if (d & 1) h51.idx = 0;
	if (d & 2) h53.idx = 0;
#endif
	if ((d & 0xe0) == 0) {
		tm_adjust(T_06XX, AT_NEVER, 0, AT_NEVER);
		n06_timer_state = false;
		n06_set_nmi(0);
		set_line(L_51_IRQ, 0);
		set_line(L_53_IRQ, 0);
		set_line(L_52_IRQ, 0);
		set_line(L_54_IRQ, 0);
	} else {
		if (d & 0x10) {
			n06_set_nmi(0);
			n06_read_stretch = true;
		} else n06_read_stretch = false;
		uint32_t divisor = 1u << ((d & 0xe0) >> 5);
		int64_t hz = HZ_AS(N06_CLOCK / divisor);           // attotime / 2 rounds (attotime.cpp)
		at_t period = hz / 2 + (hz % 2 >= 1);
		at_t t = now();
		at_t delay = at_from_ticks(at_ticks(t, N06_CLOCK) + 1, N06_CLOCK) - t;
		tm_adjust(T_06XX, delay, 0, period);
	}
}

PP_HOT static uint8_t steering_changed(void) {
	uint8_t steer_new = pp_in.steer;
	steer_accum += (int8_t)(steer_new - steer_last) * 2;
	steer_last = steer_new;
	if (steer_accum < 0) {
		steer_delta = 0;
		steer_accum++;
	} else if (steer_accum > 0) {
		steer_delta = 1;
		steer_accum--;
	}
	return steer_accum & 1;
}

PP_HOT static uint8_t in0_port(void) {         // IN0 with bit 2 the program's auto start (active low)
	return (uint8_t)((pp_in.in0 & ~0x04) | ((auto_start_mask ^ 1) << 2));
}

PP_HOT uint8_t mb88_read_k(int id) {
	switch (id) {
	case 0: return (uint8_t)((n51_rw << 3) | (n51_portO & 7));
	case 1: return n52_cmd & 0x0f;
	case 2: return 0;
	default: return n54_cmd >> 4;
	}
}

PP_HOT uint8_t mb88_read_r(int id, int n) {
	switch (id) {
	case 0:
		switch (n) {
		case 0: return pp_in.dswb & 0x0f;
		case 1: return pp_in.dswb >> 4;
		case 2: return in0_port() & 0x0f;
		default: return in0_port() >> 4;
		}
	case 1:
		if (n >= 2) return 0;
		{
			uint8_t b = n52_address < 0x8000 ? roms->speech[n52_address] : 0xff;
			return n == 0 ? (b & 0x0f) : (b >> 4);
		}
	case 2:
		switch (n) {
		case 0: return steering_changed();
		case 1: return steer_delta;
		case 2: return pp_in.dswa & 0x0f;
		default: return pp_in.dswa >> 4;
		}
	default:
		return n == 0 ? (n54_cmd & 0x0f) : 0;
	}
}

PP_HOT void mb88_write_r(int id, int n, uint8_t d) {
	if (id == 1) {
		if (n == 2) n52_address = (uint16_t)((n52_address & 0xfff0) | (d & 0x0f));
		else if (n == 3) n52_address = (uint16_t)((n52_address & 0xff0f) | ((d & 0x0f) << 4));
	} else if (id == 3 && n == 1) N54_OUT(2, d & 0x0f);
}

PP_HOT void mb88_write_p(int id, uint8_t d) {
	if (id == 1) ppsnd_52xx(d & 0x0f);  // the 52xx's DAC
	// (the 51xx's P is the coin counters; the 53xx's goes nowhere)
}

PP_HOT void mb88_write_o(int id, uint8_t d, uint8_t mask) {
	switch (id) {
	case 0: synchronize(SY_51_O, d); break;
	case 1: n52_address = (uint16_t)((n52_address & 0x00ff) | (d << 8)); break;
	case 2: n53_portO = d; break;
	default:
		if (mask == 0x0f) N54_OUT(0, d & 0x0f);
		else N54_OUT(1, d >> 4);
		break;
	}
}

PP_HOT int mb88_read_si(int id) { return id == 1 ? 1 : 0; }

uint32_t pp_serial_on;                  // (tests: how often the customs turn it on)
PP_HOT void mb88_serial_adjust(int id, bool on) {
	at_t p = HZ_AS(MCU_CLOCK / 6);
#ifndef PP_EXACT
	if (on) pp_serial_on++;             // (the 52xx at its start: EN #$20, then reads SB, all 1s)
	mcu[id].ser_on = on;                // no timer: mb88_run ticks it
	return;
#endif
	if (on) tm_adjust(T_SER0 + id, p, 0, p);
	else tm_adjust(T_SER0 + id, AT_NEVER, 0, AT_NEVER);
}

// ---------------------------------------------------------------------------
// the latch, the ADC, memory

PP_HOT static void latch_bit(int bit, int state) {             // ls259 write_bit / clear_outputs
	if (((pp_latch >> bit) & 1) == state) return;
	pp_latch ^= (uint8_t)(1u << bit);
	IOLOG("%u L %d %d\n", pp_frames, bit, state);
	switch (bit) {
	case 0:                                             // IRQON: the IRQ cleared when it goes low
		if (!state) set_line(L_Z80_IRQ, 0);
		break;
	case 1:                                             // IOSEL: the customs' reset
#ifndef PP_EXACT
		if (!state) { h51_reset(); h53_reset(); }
#endif
		set_line(L_51_RESET, !state);
		set_line(L_52_RESET, !state);
		set_line(L_53_RESET, !state);
		set_line(L_54_RESET, !state);
		break;
	case 2:                                             // CLSON: sound enable, engine
		ppsnd_enable(state);
		break;
	case 3: adc_input = (uint8_t)state; break;          // GASEL
	case 4: set_line(L_SUB1_RESET, !state); break;      // RESB
	case 5: set_line(L_SUB2_RESET, !state); break;      // RESA
	case 6: auto_start_mask = (uint8_t)!state; break;   // SB0
	case 7: pp_chacl = (uint8_t)state; break;           // CHACL
	}
}

PP_HOT static void cb_adc(void) {              // conversion_done
	adc_result = adc_input & 1 ? pp_in.accel : pp_in.brake;
	adc_intr = true;
}

PP_HOT uint8_t z80_board_rd(uint16_t a) {
	switch (a >> 12) {
	case 0x4:
		if (a < 0x4800) return (uint8_t)pp_sprite[a & 0x7ff];
		if (a < 0x4c00) return (uint8_t)pp_road[a & 0x3ff];
		return (uint8_t)pp_alpha[a & 0x3ff];
	case 0x5:
		if (a < 0x5800) return (uint8_t)pp_view[a & 0x7ff];
		return 0;
	case 0x9:
		if (a & 0x100) return n06_control;
		return n06_data_r();
	case 0xa:
		if (a & 0x300) return 0;
		{
			uint8_t r = 0xff;
			if (vpos() >= 128) r ^= 0x02;
			if (adc_intr) r ^= 0x08;
			return r;
		}
	default:
		return 0;
	}
}

PP_HOT void z80_board_wr(uint16_t a, uint8_t d) {
	switch (a >> 12) {
	case 0x4:
		if (a < 0x4800) pp_sprite[a & 0x7ff] = (uint16_t)((pp_sprite[a & 0x7ff] & 0xff00) | d);
		else if (a < 0x4c00) pp_road[a & 0x3ff] = (uint16_t)((pp_road[a & 0x3ff] & 0xff00) | d);
		else pp_alpha[a & 0x3ff] = (uint16_t)((pp_alpha[a & 0x3ff] & 0xff00) | d);
		return;
	case 0x5:
		if (a < 0x5800) pp_view[a & 0x7ff] = (uint16_t)((pp_view[a & 0x7ff] & 0xff00) | d);
		return;
	case 0x8:                                           // 0x83xx and mirrors
		if ((a & 0xff) < 0xc0) { pp_z80ram[0x300 + (a & 0xff)] = d; return; }
		pp_z80ram[0x3c0 + (a & 0x3f)] = d;              // polepos_sound_r reads the registers back
		ppsnd_wsg(a & 0x3f, d);
		return;
	case 0x9:
		synchronize((a & 0x100) ? SY_06_CTRL : SY_06_DATA, d);
		return;
	case 0xa:
		switch ((a >> 8) & 3) {
		case 0: latch_bit(a & 7, d & 1); break;
		case 1: watchdog_counter = 16; break;
		case 2: ppsnd_engine(0, d); break;
		default: ppsnd_engine(1, d); break;
		}
		return;
	default:
		return;
	}
}

PP_HOT uint8_t z80_board_in(uint16_t port) {
	if ((port & 0xff) == 0) {           // the ADC0804, read strobed
		adc_intr = false;
		return adc_result;
	}
	return 0;
}

PP_HOT void z80_board_out(uint16_t port, uint8_t d) {
	(void)d;
	if ((port & 0xff) == 0) {           // the ADC0804: a new conversion
		adc_intr = false;
		if (!tm[T_ADC].listed) tm_adjust(T_ADC, 74 * HZ_AS(ADC_CLOCK), 0, AT_NEVER);
	}
}

PP_HOT uint8_t z80_board_irq_ack(void) { return 0xff; }       // (polepos's Z80 runs in IM 1)

PP_HOT uint16_t z8k_board_rd(int id, uint16_t a) { (void)id; (void)a; return 0; }     // unmapped

PP_HOT void z8k_board_wr(int id, uint16_t a, uint16_t d, uint16_t mask) {
	if (a < 0x8000) {
		if (a >= 0x6000) {              // z8002_nvi_enable_w: the CPU's own NVI
			uint16_t data = (uint16_t)(d & 1);         // (a byte's data is in both halves)
			(void)mask;
			sub_irq_mask = (uint8_t)data;
			if (!data) set_line(id == 0 ? L_SUB1_NVI : L_SUB2_NVI, 0);
		}
		return;
	}
	if (a >= 0xc000) {
		if (a & 0x0600) return;
		if (a & 0x100) pp_vscroll = (uint16_t)((pp_vscroll & ~mask) | (d & mask));
		else pp_hscroll = (uint16_t)((pp_hscroll & ~mask) | (d & mask));
	}
}

static void map_pages(void) {
	for (int p = 0; p < 256; p++) {
		z80_rd_page[p] = 0;
		z80_wr_page[p] = 0;
		if (p < 0x30) { z80_rd_page[p] = sram.main + (p << 8); z80_wr_page[p] = sink; }
		else if (p < 0x40) z80_rd_page[p] = z80_wr_page[p] = pp_nvram + ((p & 7) << 8);
		else if (p >= 0x80 && p < 0x90) {
			z80_rd_page[p] = pp_z80ram + ((p & 3) << 8);  // 0x8000-0x83bf, the WSG's registers after
			if ((p & 3) != 3) z80_wr_page[p] = pp_z80ram + ((p & 3) << 8);
		}
	}
	for (int c = 0; c < 2; c++) {
		z8k_t *s = &sub[c];
		for (int p = 0; p < 32; p++) {
			s->rd[p] = 0;
			s->wr[p] = 0;
		}
		for (int p = 0; p < 8; p++) { s->rd[p] = sram.sub[c] + p * 0x400; s->wr[p] = sram.sink_page; }
		for (int p = 8; p < 12; p++) s->rd[p] = zero_page;                  // 0x4000-0x5fff: no writes
		for (int p = 12; p < 16; p++) s->rd[p] = zero_page;                 // 0x6000-0x7fff: NVI enable
		s->rd[16] = s->wr[16] = pp_sprite;  s->rd[17] = s->wr[17] = pp_sprite + 0x400;
		s->rd[18] = s->wr[18] = pp_road;
		s->rd[19] = s->wr[19] = pp_alpha;
		s->rd[20] = s->wr[20] = pp_view;    s->rd[21] = s->wr[21] = pp_view + 0x400;
		// 0xb000-0xbfff unmapped, 0xc000-0xffff the scroll registers (writes only)
	}
	for (int p = 8; p < 12; p++) { sub[0].wr[p] = sub[1].wr[p] = sram.sink_page; }
}

// ---------------------------------------------------------------------------
// the timers' callbacks

PP_HOT static void sync_cb(tmr_t *t) {
	switch (t->type) {
	case SY_LINE: line_events(t->param); break;
	case SY_06_DATA: n06_write_sync((uint8_t)t->param); break;
	case SY_06_CTRL: n06_ctrl_sync((uint8_t)t->param); break;
	case SY_51_RW: n51_rw = (uint8_t)t->param; break;
	case SY_51_WRITE: case SY_51_O: n51_portO = (uint8_t)t->param; break;
	case SY_52_WRITE: mb88_wake(&mcu[1]); n52_cmd = (uint8_t)t->param; break;
	case SY_54_WRITE: n54_cmd = (uint8_t)t->param; break;
	}
}

PP_HOT static void execute_timers(void) {
	while (tl_head >= 0 && tm[tl_head].expire <= basetime) {
		int h = tl_head;
		tmr_t *t = &tm[h];
		bool was_enabled = t->enabled;
		if (t->period == AT_NEVER || t->period == 0) t->enabled = false;
		cb_timer = h;
		cb_expire = t->expire;
		cb_modified = false;
		pp_stats.timers++;
		DBG(" timer %d type %d at %lld\n", h, h >= T_SYNC ? t->type : -1, (long long)t->expire);
		if (was_enabled) {
			switch (h) {
#ifdef PP_EXACT
			case T_SND: ppsnd_update(); break;
#else
			case T_SND: mcu_catch_up(cb_expire); ppsnd_update(); break;
#endif
			case T_VBSTART: cb_vbstart(); break;
			case T_VBEND: cb_vbend(); break;
			case T_SCAN0: tm_adjust(T_SCAN0, time_until_pos(0), 0, AT_NEVER); break;
			case T_SCANLINE: cb_scanline(); break;
			case T_06XX: cb_06xx(); break;
			case T_ADC: cb_adc(); break;
			case T_SER0: case T_SER0 + 1: case T_SER0 + 2: case T_SER3: mb88_serial_timer(&mcu[h - T_SER0]); break;
			default: sync_cb(t); break;
			}
		}
		if (!cb_modified) {
			if (h < T_SYNC) {           // schedule_next_period
				t->expire = at_add(t->expire, t->period);
				tl_remove(h);
				tl_insert(h);
			} else {
				tl_remove(h);
				t->next = (int8_t)sync_free;
				sync_free = h;
			}
		}
		cb_timer = -1;
	}
}

PP_HOT static void rebase(void) {              // the epoch a second on (the listed timers' times; the others get new ones)
	basetime -= AS_PER_SEC;
	for (int d = 0; d < NDEV; d++) { ex[d].local -= AS_PER_SEC; ex[d].ready -= AS_PER_SEC; }
	for (int i = 0; i < T_COUNT; i++) if (tm[i].listed) tm[i].expire -= AS_PER_SEC;
#ifndef PP_EXACT
	for (int k = 0; k < 2; k++)
		for (int i = 0; i < mq[k].n; i++) mq[k].q[(mq[k].head + i) % MQ_N].t -= AS_PER_SEC;
#endif
	vblank_start_time -= AS_PER_SEC;
	vblank_end_time -= AS_PER_SEC;
	ep_s++;
}

// The quick way through the list, when none is suspended: each device's
// constants known (CPU_* the Z80's and the Z8002s', MCU_* the customs'),
// so the division is a multiply and the times stay in registers. The same
// steps as the loop in timeslice().
#define CPU_ASPC HZ_AS(PP_CLOCK)
#define CPU_SHIFT 8
#define MCU_ASPC HZ_AS(MCU_CLOCK / 6)
#define MCU_SHIFT 11
static bool quick_ok;                   // the constants are exec_init's (pp_init)

PP_HOT static inline __attribute__((always_inline)) uint32_t qdiv(uint64_t x, uint32_t divisor, float inv) {
#if defined(__arm__) || defined(PP_FLOAT_DIV)
	float xf = (float)(uint32_t)(x >> 24) * 16777216.0f + (float)(uint32_t)(x & 0xffffff);
	uint32_t q = (uint32_t)(xf * inv);
	int64_t r = (int64_t)x - (int64_t)((uint64_t)q * divisor);
	while (r < 0) { q--; r += divisor; }
	while (r >= (int64_t)divisor) { q++; r -= divisor; }
	return q;
#else
	(void)inv;
	return (uint32_t)(x / divisor);
#endif
}

#ifndef PP_NO_TRACE
#define QSTAT(d, ran) (pp_stats.runs[d]++, pp_stats.cycles[d] += (uint64_t)(ran))
#else
#define QSTAT(d, ran) ((void)0)
#endif
#define QSTEP(D, ASPC, SHIFT, RUN) do { \
	exec_t *e_ = &ex[D]; \
	if (target >= e_->ready) { \
		int cycles = (int)qdiv((uint64_t)(target - e_->local) >> SHIFT, (uint32_t)((ASPC) >> SHIFT), \
		                       1.0f / (float)(uint32_t)((ASPC) >> SHIFT)); \
		cur = D; \
		int ran = RUN; \
		cur = -1; \
		QSTAT(D, ran); \
		e_->total += (uint32_t)ran; \
		e_->started |= ran != 0; \
		at_t l_ = e_->local + (int64_t)(uint32_t)ran * (ASPC);      /* (ran < cps: a run is a quantum at most) */ \
		e_->local = l_; \
		e_->ready = l_ + (ASPC); \
		if (l_ < target) target = l_ > base ? l_ : base; \
	} \
} while (0)

static PP_HOT at_t quick_slice(at_t target, at_t base) {
	QSTEP(D_Z80, CPU_ASPC, CPU_SHIFT, z80_run(cycles));
	QSTEP(D_SUB1, CPU_ASPC, CPU_SHIFT, z8k_run(&sub[0], cycles));
	QSTEP(D_SUB2, CPU_ASPC, CPU_SHIFT, z8k_run(&sub[1], cycles));
#ifdef PP_EXACT
	QSTEP(D_51, MCU_ASPC, MCU_SHIFT, mb88_run(&mcu[0], cycles));
	QSTEP(D_52, MCU_ASPC, MCU_SHIFT, mb88_run(&mcu[1], cycles));
	QSTEP(D_53, MCU_ASPC, MCU_SHIFT, mb88_run(&mcu[2], cycles));
	QSTEP(D_54, MCU_ASPC, MCU_SHIFT, mb88_run(&mcu[3], cycles));
#endif
	return target;
}

static PP_HOT void timeslice(void) {
	pp_stats.slices++;
	if (basetime >= AS_PER_SEC) rebase();
	while (basetime < head_expire()) {
		at_t target = basetime + QUANTUM_AS;
		at_t he = head_expire();
		if (he < target) target = he;
		if (suspend_pending) apply_suspend_changes();
		DBG("slice base %lld target %lld head %d\n", (long long)basetime, (long long)target, tl_head);
#ifndef PP_DEBUG
		if (all_run && quick_ok) {
			basetime = quick_slice(target, basetime);
			continue;
		}
#endif
		for (int k = 0; k < NSCHED; k++) {
			int d = exec_list[k];
			exec_t *e = &ex[d];
			if (!e->go || target < e->ready) continue;        // (delta < aspc)
			int cycles = (int)div_cycles((uint64_t)(target - e->local) >> e->divshift, e);
			int ran = cycles;
			if (e->suspend == 0) {
				cur = d;
				ran = dev_run(d, cycles);
				cur = -1;
#ifndef PP_NO_TRACE
				pp_stats.runs[d]++;
				pp_stats.cycles[d] += (uint64_t)ran;
#endif
			}
			e->total += (uint32_t)ran;
			e->started |= ran != 0;
			DBG("  dev %d local %lld want %d ran %d\n", d, (long long)e->local, cycles, ran);
			e->local += (uint32_t)ran < e->cps ? (int64_t)(uint32_t)ran * e->aspc : cycles_at(e, (uint64_t)ran);
			e->ready = e->local + e->aspc;
			if (e->local < target)
				target = e->local > basetime ? e->local : basetime;
		}
		basetime = target;
	}
	execute_timers();
}

// ---------------------------------------------------------------------------

PP_HOT uint64_t pp_time(void) { return at_ticks(basetime, PP_CLOCK); }

void (*pp_trace)(int cpu, uint32_t pc, uint64_t total);
uint32_t pp_trace_mask;

static void tr_z80(void) {
	if (!(pp_trace_mask & 1)) return;
	z80_regs_t r;
	z80_regs(&r);
	pp_trace(0, r.pc, (uint64_t)ex[0].total + (uint64_t)z80_done());
}
static void tr_z8k(const z8k_t *c) {
	int d = 1 + c->id;
	if (pp_trace_mask & (1u << d)) pp_trace(d, c->pc, (uint64_t)ex[d].total + (uint64_t)z8k_done());
}
static void tr_mb88(const mb88_t *m) {
	int d = 3 + m->id;
	if (pp_trace_mask & (1u << d)) pp_trace(d, (uint32_t)(m->PA << 6 | m->PC), (uint64_t)ex[d].total + (uint64_t)mb88_done());
}

void pp_trace_on(void) {
	z80_trace = pp_trace ? tr_z80 : 0;
	z8k_trace = pp_trace ? tr_z8k : 0;
	mb88_trace = pp_trace ? tr_mb88 : 0;
}

void pp_cpu_regs(int cpu, uint16_t *r, int *n) {
	if (cpu == 0) {
		z80_regs_t z;
		z80_regs(&z);
		uint16_t w[19] = { z.pc, z.sp, z.af, z.bc, z.de, z.hl, z.ix, z.iy, z.af2, z.bc2, z.de2, z.hl2, z.wz,
		                   z.r, z.i, z.im, z.iff1, z.iff2, z.halt };
		memcpy(r, w, sizeof w);
		*n = 19;
	} else if (cpu <= 2) {
		const z8k_t *c = &sub[cpu - 1];
		for (int i = 0; i < 16; i++) r[i] = z8k_reg(c, i);
		r[16] = (uint16_t)c->pc;
		r[17] = c->fcw;
		*n = 18;
	} else {
		mb88_t *m = &mcu[cpu - 3];
		mb88_wake(m);
		r[0] = m->PA; r[1] = m->PC;
		r[2] = m->A; r[3] = m->X; r[4] = m->Y;
		r[5] = m->pio; r[6] = (uint16_t)(m->TH << 4 | m->TL); r[7] = m->SI;
		r[8] = (uint16_t)(m->st | m->zf << 1 | m->cf << 2 | m->vf << 3 | m->sf << 4 | m->iff << 5);
		*n = 9;
	}
}

uint8_t pp_mcu_ram(int m, int a) { mb88_wake(&mcu[m]); return mcu[m].ram[a & 0x3f]; }

static void board_reset(void) {         // every device's device_reset, in MAME's order
#ifdef PP_EXACT
	for (int d = 0; d < NDEV; d++) dev_reset(d);
#else
	mcu_catch_up(basetime);
	for (int d = 0; d < NSCHED; d++) dev_reset(d);
	for (int k = 0; k < 2; k++) {
		mq[k].head = mq[k].n = 0;
		mq[k].held = false;
		mq_line[k] = 0xff;
		ex[k ? D_54 : D_52].total = 0;
		mb88_reset(&mcu[k ? 3 : 1]);
	}
	q54_reset();
	h51_reset();
	h53_reset();
#endif
	// 06xx
	n06_control = 0;
	// watchdog
	watchdog_counter = 16;
	// the scanline timer: fire now, then at line 0
	scan_first = true;
	tm_adjust(T_SCANLINE, 0, 0, AT_NEVER);
	// the latch: clear_outputs(0)
	for (int b = 0; b < 8; b++) latch_bit(b, 0);
	ppsnd_reset();
}

void pp_init(const pp_roms_t *r) {
	roms = r;
	memcpy(sram.main, r->maincpu, sizeof sram.main);
	for (int m = 0; m < 4; m++) memcpy(sram.mcu[m], r->mcu[m], sizeof sram.mcu[m]);
	memset(sram.sink_page, 0, sizeof sram.sink_page);
	for (int i = 0; i < 0x2000; i++) {
		sram.sub[0][i] = (uint16_t)(r->sub1_hi[i] << 8 | r->sub1_lo[i]);
		sram.sub[1][i] = (uint16_t)(r->sub2_hi[i] << 8 | r->sub2_lo[i]);
	}
	memset(tm, 0, sizeof tm);
	tl_head = -1;
	sync_free = -1;
	for (int i = T_COUNT - 1; i >= T_SYNC; i--) { tm[i].next = (int8_t)sync_free; sync_free = i; }
	basetime = 0;
	ep_s = 0;
	cur = -1;
	cb_timer = -1;
	memset(lq, 0, sizeof lq);
	memset(&pp_stats, 0, sizeof pp_stats);
	memset(pp_sprite, 0, sizeof pp_sprite);
	memset(pp_road, 0, sizeof pp_road);
	memset(pp_alpha, 0, sizeof pp_alpha);
	memset(pp_view, 0, sizeof pp_view);
	memset(pp_z80ram, 0, sizeof pp_z80ram);
	memset(pp_nvram, 0xff, sizeof pp_nvram);            // nvram_device::DEFAULT_ALL_1
	pp_hscroll = pp_vscroll = 0;
	pp_chacl = 0;
	pp_frames = pp_resets = 0;
	sub_irq_mask = 0;
	n06_control = 0; n06_timer_state = n06_read_stretch = false;
	n51_portO = n51_rw = n52_cmd = n53_portO = n54_cmd = 0;
	n52_address = 0;
	steer_last = steer_delta = 0; steer_accum = 0;
	adc_input = 0; auto_start_mask = 0; adc_intr = false; adc_result = 0;
	watchdog_fired = false;
	vblank_start_time = 0;
	vblank_end_time = VBLANK_AS;
	map_pages();

	// device_start, in MAME's order
	exec_init(&ex[D_Z80], PP_CLOCK, 1);
	exec_init(&ex[D_SUB1], PP_CLOCK, 1);
	exec_init(&ex[D_SUB2], PP_CLOCK, 1);
	for (int m = 0; m < 4; m++) exec_init(&ex[D_51 + m], MCU_CLOCK, 6);
	quick_ok = ex[D_Z80].aspc == CPU_ASPC && ex[D_Z80].divshift == CPU_SHIFT && ex[D_51].aspc == MCU_ASPC &&
	           ex[D_51].divshift == MCU_SHIFT && ex[D_Z80].cps > 51200 + 100 && ex[D_51].cps > 4267 + 100;
	rebuild_execute_list();
	suspend_pending = false;
	z80_start();
	z8k_init();
	z8k_start(&sub[0], 0);
	z8k_start(&sub[1], 1);
	for (int m = 0; m < 4; m++) {
		memset(&mcu[m], 0, sizeof mcu[m]);
		mcu[m].id = m;
		mcu[m].rom = sram.mcu[m];
		mb88_find_idle(&mcu[m]);
	}
	static uint8_t pz52[MB88_PZ_LEN];   // the 52xx's inputs: the IRQ, its command (mq_apply, SY_52_WRITE), the
	mcu[1].pz_snap = pz52;              // speech ROM at the address it writes (mb88.c: the periodic wait)
	ppsnd_init(r);
	// the sound manager's 50 Hz update, then the screen's timers
	tm_adjust(T_SND, HZ_AS(50), 0, HZ_AS(50));
	tm_adjust(T_VBSTART, time_until_pos(VBSTART), 0, AT_NEVER);
	tm_adjust(T_SCAN0, time_until_pos(0), 0, AT_NEVER);
	pp_latch = 0xff;                    // the latch's m_q until its reset

	board_reset();
}

PP_HOT uint32_t pp_run_to(uint64_t until) {
	uint32_t f0 = pp_frames;
	at_t to = at_from_ticks(until, PP_CLOCK);           // (once: the 64-bit divisions are slow on the M33)
	while (basetime < to) {
		int32_t e = ep_s;
		timeslice();
		if (ep_s != e) to -= AS_PER_SEC;                // rebased
	}
#ifndef PP_EXACT
	mcu_catch_up(basetime);
#endif
	return pp_frames - f0;
}

PP_HOT bool pp_frame(void) {
	frame_done = false;
	while (!frame_done) timeslice();
#ifndef PP_EXACT
	mcu_catch_up(basetime);
#endif
	if (watchdog_fired) {               // schedule_soft_reset: every device's reset
		watchdog_fired = false;
		pp_resets++;
		board_reset();
		return false;
	}
	return true;
}
