// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Derrick Renaud
//
// ppsnd.c -- Pole Position's sound for the cart, as MAME 0.288 makes it:
//
//   namco.cpp (Nicola Salmoria, Aaron Giles): the WSG, polepos_wsg_device
//   polepos_a.cpp (Derrick Renaud): the engine and the discrete circuits
//     after the 52xx and the 54xx (MAME's discrete nodes, Derrick Renaud)
//   sound.cpp, speaker.cpp: the routes and the gains to the speakers
//
// One mono stream, the four speaker channels' mean, in bins at the list's
// rate (a box filter). Time is the WSG's stream's ticks, 192 kHz. Each
// stream is made when MAME makes it (the write that changes it, the 50 Hz
// update), up to the tick of now, now's included:
//   the WSG: each bin's sum over its ticks of each voice's 192 kHz steps,
//     from the waveform's integral (no per-tick loop);
//   the engine: MAME's 24 kHz samples and its three filters (which MAME
//     sets up for 48 kHz), then as MAME's resampler takes them to the
//     speakers' 48 kHz: each sample, then the four around halfway;
//   the discrete circuits: MAME's 48 kHz steps; a channel whose input and
//     filters are at rest isn't run.
// The WSG comes out 14.5 ticks late at MAME's speakers (its resampler puts
// the middle of its 48 kHz sample D, tick 4D + 2, on the WSG's 4D - 13 and
// a half) and the engine 24, and so they do here. The mix is in floats, not MAME's
// doubles; the WSG's 192 kHz steps are an integral (see above); nothing
// else is left out.
#include "ppsnd.h"

#include <math.h>
#include <string.h>

#if defined(PICO_ON_DEVICE) && PICO_ON_DEVICE
#define SND_HOT __attribute__((section(".time_critical.ppsnd")))
#else
#define SND_HOT
#endif

#define TICK_HZ 192000u
#define WSG_LAG 14                      // and a half, in the integral
#define ENG_LAG 24
#ifndef PPSND_NBIN
#define PPSND_NBIN 256                  // bins the streams can be ahead of ppsnd_take (a power of 2)
#endif
#define NB PPSND_NBIN
#define NBM (NB - 1)
#define M48 1024                        // the 48 kHz ring's samples (a power of 2)
#define M48M (M48 - 1)
#define NBUF 64                         // a stream's 48 kHz samples made at a time
#define CLIP(x, lo, hi) fminf(fmaxf(x, lo), hi)

void (*ppsnd_tap)(int what, int a, int d);
unsigned ppsnd_parts = 15;
#define TAP(w, a, d) do { if (ppsnd_tap) ppsnd_tap(w, a, d); } while (0)

// ---------------------------------------------------------------------------
// time: pp_now() as ticks (floor(t * 192000), exactly; it wraps in 6.2 hours)

static bool at_zero;                    // MAME's streams don't update at time 0

static uint32_t tick_now(void) {
	pp_at_t t = pp_now();
	at_zero = t.s == 0 && t.a == 0;
	uint64_t a = (uint64_t)t.a, a3 = a * 3;     // ticks: floor(3a / 15625e9)
	uint32_t q = (uint32_t)(((uint64_t)(uint32_t)(a >> 28) * 221361u) >> 32);   // within one
	if ((uint64_t)(q + 1) * 15625000000000ull <= a3) q++;
	else if ((uint64_t)q * 15625000000000ull > a3) q--;
	return (uint32_t)t.s * TICK_HZ + q;
}

// ---------------------------------------------------------------------------
// the bins: each the sum over its ticks of the mono sound (MAME's -1..1)

static float bin_acc[NB];
static uint8_t bin_len[NB];             // its ticks
static uint32_t bin_read, bin_alloc, bin_frac;
static uint32_t bin_t0;                 // bin_read's first tick, where bins_t0() has put it
static uint64_t bin_step = ((uint64_t)TICK_HZ << 40) / PPSND_LIST_RATE_Q8;   // ticks a bin, 32.32

typedef struct { uint32_t k, end; } cur_t;      // a stream's bin, and its end in the stream's ticks

static void bin_new(void) {
	if (bin_alloc - bin_read >= NB) { bin_acc[bin_read & NBM] = 0; bin_read++; }   // not taken: lost
	uint64_t e = (uint64_t)bin_frac + bin_step;
	bin_len[bin_alloc & NBM] = (uint8_t)(e >> 32);
	bin_frac = (uint32_t)e;
	bin_alloc++;
}

static inline void cur_next(cur_t *c) {
	c->k++;
	if (c->k == bin_alloc) bin_new();
	c->end += bin_len[c->k & NBM];
}

static void cur_start(cur_t *c, uint32_t lag) {         // at the stream's tick 0, `lag` ticks late
	c->k = bin_read;
	c->end = bin_len[c->k & NBM] - lag;
	while ((int32_t)c->end <= 0) cur_next(c);
}

// the 48 kHz streams (the engine, the discrete circuits) add into one ring
// of MAME's 48 kHz samples, its tick / 4; at an update and a take it goes
// into the bins. In it the discrete circuits' units: the engine is scaled
// to them.

static float m48[M48];
static uint32_t m48_t;                  // the next sample's tick to go to the bins
static cur_t m48_cur;
static float scratch[NBUF];             // for samples that are thrown away
#define DGAIN (0.90f * 16383.0f / 32768.0f)     // DISCRETE_OUTPUT 32767/2, route 0.9

static SND_HOT void m48_box(uint32_t to) {      // the samples before tick `to` into the bins
	uint32_t t = m48_t;
	while ((int32_t)(to - t) > 0) {
		uint32_t end = m48_cur.end, stop = (int32_t)(end - to) < 0 ? end : to;
		uint32_t i = t >> 2, m = (stop - t) >> 2;       // the bin's whole samples
		float s0 = 0, s1 = 0;
		for (; m >= 2; m -= 2, i += 2) {
			float *y0 = &m48[i & M48M], *y1 = &m48[(i + 1) & M48M];
			s0 += *y0; s1 += *y1;
			*y0 = *y1 = 0;
		}
		if (m) { s0 += m48[i & M48M]; m48[i & M48M] = 0; i++; }
		t = i << 2;
		float *acc = &bin_acc[m48_cur.k & NBM];
		*acc += (s0 + s1) * (4 * DGAIN);
		if (t == end) cur_next(&m48_cur);
		else if (stop == end) {         // a sample across the bin's end
			float *y = &m48[i & M48M], v = *y * DGAIN;
			*y = 0;
			*acc += v * (float)(end - t);
			cur_next(&m48_cur);
			bin_acc[m48_cur.k & NBM] += v * (float)(4 - (end - t));
			t += 4;
		}
	}
	m48_t = t;
}

// where a stream's samples from tick t go: *n cut (to whole `unit`s) so
// they don't pass the ring's end. If the ring would fill, its oldest go to
// the bins first; a late stream's samples that went already are thrown
// away. (Neither happens while ppsnd_update comes every 20 ms.)
static SND_HOT float *m48_at(uint32_t t, int *n, int unit) {
	int32_t back = (int32_t)(m48_t - t);
	if (back > 0) {
		int k = ((back + 4 * unit - 1) / (4 * unit)) * unit;
		if (*n > k) *n = k;
		memset(scratch, 0, sizeof scratch);
		return scratch;
	}
	uint32_t i = (t >> 2) & M48M;
	if (*n > (int)(M48 - i)) *n = (int)(M48 - i);
	int32_t over = (int32_t)(t + 4u * (uint32_t)*n - m48_t) - 4 * M48;
	if (over > 0) m48_box(m48_t + (uint32_t)over);
	return &m48[i];
}

// ---------------------------------------------------------------------------
// the WSG: 8 voices, a 16-bit frequency added each tick to a counter whose
// bits 17-21 pick the waveform's step (MAME's clock is 48 kHz, so 17
// fraction bits); four volumes (the speakers'), a voice with all four 0
// doesn't move. Each bin: per voice, the integral of the waveform over the
// counter's path, over the frequency (the sum of the ticks: the integral
// from half a tick before; and half a tick more of the lag).

typedef struct { int32_t p, w; } wstep_t;      // the integral to the step's start (x 2^17), the step's value
static wstep_t wave_i[8][32];
static int32_t wave_cycle[8];                   // a whole waveform's integral
static struct { uint32_t freq, counter; float gf; uint8_t sel; } voice[8];
static uint8_t wregs[0x40];
static bool wsg_enable;
static uint8_t wsg_on;                  // the voices that move (a bit each)
static uint32_t wsg_pos;                // the next tick to make
static cur_t wsg_cur;

#define WMASK 0x3fffffu                 // 32 steps of 2^17

static inline int32_t wint(const wstep_t *w, uint32_t x) {
	const wstep_t *s = &w[x >> 17];
	return s->p + s->w * (int32_t)(x & 0x1ffff);
}

static SND_HOT void wsg_make(uint32_t to) {     // to the tick before `to`
	while ((int32_t)(to - wsg_pos) > 0) {
		uint32_t e = wsg_cur.end;
		if ((int32_t)(e - to) > 0) e = to;
		uint32_t n = e - wsg_pos;
		if (wsg_enable && wsg_on) {
			float s = 0;
			for (unsigned m = wsg_on; m; m &= m - 1) {
				int v = __builtin_ctz(m);
				uint32_t f = voice[v].freq, c = voice[v].counter, d = n * f;
				const wstep_t *w = wave_i[voice[v].sel];
				if (f) {
					uint32_t a = (c - f) & WMASK, b = a + d;
					int32_t D = wint(w, b & WMASK) - wint(w, a) + (int32_t)(b >> 22) * wave_cycle[voice[v].sel];
					s += (float)D * voice[v].gf;
				} else
					s += (float)((int32_t)n * w[(c >> 17) & 31].w) * voice[v].gf;
				voice[v].counter = c + d;
			}
			if (ppsnd_parts & 1) bin_acc[wsg_cur.k & NBM] += s;
		}
		wsg_pos = e;
		if (e == wsg_cur.end) cur_next(&wsg_cur);
	}
}

static void wsg_voice(int ch) {         // polepos_sound_w's sums, from the registers
	const uint8_t *r = &wregs[ch * 4];
	int vol = (r[3] >> 4) + (r[3] & 15) + (r[0x23] >> 4) + (r[2] >> 4);
	if (r[0x23] & 8) vol = 0;           // the 54xx or the 52xx selected
	voice[ch].freq = (uint32_t)(r[0] | r[1] << 8);
	voice[ch].sel = r[0x23] & 7;
	float g = 0.80f / 4096 * (float)vol;        // route 0.8, MIX_RES 1024, the four channels' mean
	voice[ch].gf = voice[ch].freq ? g / (float)voice[ch].freq : g;
	if (vol) wsg_on |= (uint8_t)(1u << ch);
	else wsg_on &= (uint8_t)~(1u << ch);
}

// ---------------------------------------------------------------------------
// the engine: the ROM's samples at a rate from the 12 bits written, 24 kHz,
// through two band-passes and a high-pass (each clipped), mixed. MAME's
// lofi resampler makes the speakers' 48 kHz from them: sample k three
// samples late, then -1/16, 9/16, 9/16, -1/16 of k-1 to k+2 (its cubic at
// halfway). Here each new sample k gives sample k-2's two 48 kHz samples.

static const uint8_t *eng_rom;
static float eng_k1[8], eng_k0[8];      // a slot's sample: rom * k1 + k0
static float eb[3][3];                  // b0 a1 a2: band-passes (b1 0, b2 -b0), the high-pass (b1 -2 b0, b2 b0)
static float ew[3];                     // their weights in the mix, with the route's gain (/ DGAIN)
static float ex1, ex2, ey[3][2];
static float eh[3];                     // the last three samples made, oldest first
static uint32_t eng_pos, eng_step, eng_tick;    // eng_tick: the next sample's, 8 a sample
static uint8_t eng_msb, eng_lsb;
static bool eng_enable;
#define ENG_M48 (ENG_LAG - 16)          // its 48 kHz samples' ticks in the ring: two samples after it, and the lag

static SND_HOT void eng_block(float *out, int n) {      // n samples, 2n added to out
	float h0 = eh[0], h1 = eh[1], h2 = eh[2];
	if (!eng_enable) {                  // MAME's stream is 0 and nothing moves
		for (int i = 0; i < n; i++) {
			out[2 * i] += h1;
			out[2 * i + 1] += 0.5625f * (h1 + h2) - 0.0625f * h0;
			h0 = h1; h1 = h2; h2 = 0;
		}
		eh[0] = h0; eh[1] = h1; eh[2] = h2;
		return;
	}
	unsigned slot = (eng_msb >> 3) & 7;
	const uint8_t *base = eng_rom + slot * 0x800;
	float k1 = eng_k1[slot], k0 = eng_k0[slot];
	float b0 = eb[0][0], a10 = eb[0][1], a20 = eb[0][2];
	float b1 = eb[1][0], a11 = eb[1][1], a21 = eb[1][2];
	float b2 = eb[2][0], a12 = eb[2][1], a22 = eb[2][2];
	float w0 = ew[0], w1 = ew[1], w2 = ew[2];
	float x1 = ex1, x2 = ex2;
	float p1 = ey[0][0], p2 = ey[0][1], q1 = ey[1][0], q2 = ey[1][1], r1 = ey[2][0], r2 = ey[2][1];
	uint32_t pos = eng_pos, step = eng_step;
	// a sample: x into x2 (which goes 1 then), the filters' new outputs into their y2s
#define ENG_STEP(x1, x2, p1, p2, q1, q2, r1, r2, o) do { \
		float x = (float)base[(pos >> 12) & 0x7ff] * k1 + k0; \
		pos += step; \
		float d = x - x2; \
		p2 = b0 * d - a10 * p1 - a20 * p2; \
		q2 = b1 * d - a11 * q1 - a21 * q2; \
		r2 = b2 * (d - 2 * x1 + 2 * x2) - a12 * r1 - a22 * r2; \
		x2 = x; \
		float e = CLIP(p2, -2.0f, 1.5f) * w0 + CLIP(q2, -2.0f, 1.5f) * w1 + CLIP(r2, -2.0f, 1.5f) * w2; \
		(o)[0] += h1; \
		(o)[1] += 0.5625f * (h1 + h2) - 0.0625f * (h0 + e); \
		h0 = h1; h1 = h2; h2 = e; \
	} while (0)
	int i = 0;
	for (; i + 2 <= n; i += 2) {        // two at a time, so the states don't move
		ENG_STEP(x1, x2, p1, p2, q1, q2, r1, r2, out + 2 * i);
		ENG_STEP(x2, x1, p2, p1, q2, q1, r2, r1, out + 2 * i + 2);
	}
	if (i < n) {
		ENG_STEP(x1, x2, p1, p2, q1, q2, r1, r2, out + 2 * i);
		float t = x1; x1 = x2; x2 = t;
		t = p1; p1 = p2; p2 = t; t = q1; q1 = q2; q2 = t; t = r1; r1 = r2; r2 = t;
	}
#undef ENG_STEP
	ex1 = x1; ex2 = x2;
	ey[0][0] = p1; ey[0][1] = p2; ey[1][0] = q1; ey[1][1] = q2; ey[2][0] = r1; ey[2][1] = r2;
	eh[0] = h0; eh[1] = h1; eh[2] = h2;
	eng_pos = pos;
}

static SND_HOT void eng_make(uint32_t tick) {
	uint32_t end = (tick & ~7u) + 8;    // its samples to now's, now's included
	while ((int32_t)(end - eng_tick) > 0) {
		int n = (int)((end - eng_tick) >> 2);
		if (n > NBUF) n = NBUF;
		if (!eng_enable && eh[0] == 0 && eh[1] == 0 && eh[2] == 0) { eng_tick += 4u * (uint32_t)n; continue; }
		float *out = m48_at(eng_tick + ENG_M48, &n, 2);
		eng_block(ppsnd_parts & 2 ? out : scratch, n / 2);
		eng_tick += 4u * (uint32_t)n;
	}
}

static void eng_rate(void) {
	uint32_t clock = (3072000u / 16) * ((eng_msb + 1u) * 64 + eng_lsb + 1u) / (64 * 64);
	eng_step = (clock << 12) / 24000;
}

// ---------------------------------------------------------------------------
// the discrete circuits, by input: 0-2 the 54xx's outputs (CHANL3, CHANL2,
// CHANL1: a DAC, an op-amp band-pass, clipped at its rails), 3 the 52xx's
// DAC (CHANL4: a high-pass, a low-pass, halved, clamped), at 48 kHz

#define VREF 2.0f                       // 5 V * 1K / (1.5K + 1K)
#define DQUIET 3e-5f                    // at rest: within this of 0 (half a 16-bit step)
static float dv[4][16];                 // an input's value: 0-2 the op-amp's input, (dac - vRef) rTotal / r1; 3 dac - vRef
static float dc[4][3];                  // b0 a1 a2: 0-2 their band-passes (with the op-amp's gain), 3 the high-pass
static float dl[3];                     // 3's low-pass
static float dx[4][2], dy[4][2], dly[2];        // x1 x2 and y1 y2 (0-2: the output less vRef), 3's low-pass y1 y2
static float din_v[4];                  // each input's value now
static uint8_t din[4];
static uint8_t dquiet;                  // the inputs whose circuits are at rest (a bit each)
static uint32_t disc_tick;              // the next sample's, 4 a sample

// (each runs two samples at a time, the new values into the older ones, so
// the states don't move)
static SND_HOT void opamp_run(int c, float *out, int n) {      // n samples added to out
	float b0 = dc[c][0], a1 = dc[c][1], a2 = dc[c][2];
	float x1 = dx[c][0], x2 = dx[c][1], y1 = dy[c][0], y2 = dy[c][1], v = din_v[c];
#define OP_STEP(x1, x2, y1, y2, o) do { \
		y2 = CLIP(b0 * (v - x2) - a1 * y1 - a2 * y2, -2.0f, 1.5f);      /* its output 0-3.5 V */ \
		x2 = v; \
		(o) += y2; \
	} while (0)
	int i = 0;
	for (; i + 2 <= n; i += 2) {
		OP_STEP(x1, x2, y1, y2, out[i]);
		OP_STEP(x2, x1, y2, y1, out[i + 1]);
	}
	if (i < n) {
		OP_STEP(x1, x2, y1, y2, out[i]);
		float t = x1; x1 = x2; x2 = t;
		t = y1; y1 = y2; y2 = t;
	}
#undef OP_STEP
	dx[c][0] = x1; dx[c][1] = x2; dy[c][0] = y1; dy[c][1] = y2;
	if (x1 == v && x2 == v && fabsf(y1) < DQUIET && fabsf(y2) < DQUIET) dquiet |= (uint8_t)(1u << c);
}

static SND_HOT void chanl4_run(float *out, int n) {
	float h0 = dc[3][0], ha1 = dc[3][1], ha2 = dc[3][2], l0 = dl[0], la1 = dl[1], la2 = dl[2];
	float x1 = dx[3][0], x2 = dx[3][1], h1 = dy[3][0], h2 = dy[3][1], l1 = dly[0], l2 = dly[1], v = din_v[3];
#define C4_STEP(x1, x2, h1, h2, l1, l2, o) do { \
		float h = h0 * (v - 2 * x1 + x2) - ha1 * h1 - ha2 * h2; \
		l2 = l0 * (h + 2 * h1 + h2) - la1 * l1 - la2 * l2; \
		h2 = h; \
		x2 = v; \
		(o) += CLIP(l2 * 0.5f, 0.0f, 1.5f);     /* 0 to 5 V less the rail's 1.5, less vRef */ \
	} while (0)
	int i = 0;
	for (; i + 2 <= n; i += 2) {
		C4_STEP(x1, x2, h1, h2, l1, l2, out[i]);
		C4_STEP(x2, x1, h2, h1, l2, l1, out[i + 1]);
	}
	if (i < n) {
		C4_STEP(x1, x2, h1, h2, l1, l2, out[i]);
		float t = x1; x1 = x2; x2 = t;
		t = h1; h1 = h2; h2 = t;
		t = l1; l1 = l2; l2 = t;
	}
#undef C4_STEP
	dx[3][0] = x1; dx[3][1] = x2; dy[3][0] = h1; dy[3][1] = h2; dly[0] = l1; dly[1] = l2;
	if (x1 == v && x2 == v && fabsf(h1) < DQUIET && fabsf(h2) < DQUIET && fabsf(l1) < DQUIET && fabsf(l2) < DQUIET)
		dquiet |= 8;
}

static SND_HOT void disc_make(uint32_t tick) {      // the 54xx's three
	uint32_t end = (tick & ~3u) + 4;    // its samples to now's, now's included
	uint8_t off = ppsnd_parts & 4 ? 0 : 7;
	while ((int32_t)(end - disc_tick) > 0) {
		int n = (int)((end - disc_tick) >> 2);
		if (n > NBUF) n = NBUF;
		uint8_t run = (uint8_t)~(dquiet | off) & 7;
		if (run) {
			float *out = m48_at(disc_tick, &n, 1);
			for (int c = 0; c < 3; c++)
				if (run & (1u << c)) opamp_run(c, out, n);
		}
		disc_tick += 4u * (uint32_t)n;
	}
}

// The 52xx's circuit (CHANL4) keeps its own time, so the 54xx's writes
// (which the board makes out of time order with the 52xx's, by up to a
// slice of its MCUs' run) don't move its steps. For ppsnd_take its
// samples go to the 48 kHz ring with the others; for ppsnd_take_list into
// 8 kHz slots, six samples each (see the output).
#define S8 256                          // the slots' ring (a power of 2): 32 ms
#define S8M (S8 - 1)
static float s8[S8];
static uint32_t s8_k, s8_t;             // the oldest slot kept, and its first tick
static bool s8_used;                    // something in the slots
static bool slots;                      // CHANL4 to the slots
static uint32_t c4_tick;                // the next sample's, 4 a sample
static uint32_t c4_rest_at;             // the tick it last came to rest

static uint32_t bins_t0(void);
static bool talking;

static SND_HOT void c4_slots(int n) {   // n samples from c4_tick into the slots
	static float buf[NBUF];             // (kept at 0)
	if (!s8_used && !talking) {         // the slots from the first bin not taken (all 0)
		bin_t0 = bins_t0();
		int32_t k = (int32_t)(bin_t0 - s8_t) / 24;
		if (k > 0) { s8_k += (uint32_t)k; s8_t += 24u * (uint32_t)k; }
	}
	s8_used = true;
	chanl4_run(buf, n);
	int32_t d = (int32_t)(c4_tick - s8_t);
	if (d >= 0 && d + 4 * n <= 24 * S8) {
		uint32_t k = s8_k + (uint32_t)d / 24;
		int i = 0, m = 6 - ((d % 24) >> 2);     // the first slot's samples left
		while (i < n) {
			float *slot = &s8[k++ & S8M], v = *slot;
			if (m > n - i) m = n - i;
			for (; m > 0; m--, i++) { v += buf[i]; buf[i] = 0; }
			*slot = v;
			m = 6;
		}
	} else
		for (int i = 0; i < n; i++, d += 4) {
			if (d >= 0 && d < 24 * S8) s8[(s8_k + (uint32_t)d / 24) & S8M] += buf[i];
			buf[i] = 0;
		}
}

static SND_HOT void c4_make(uint32_t tick) {
	uint32_t end = (tick & ~3u) + 4;
	if ((dquiet & 8) || !(ppsnd_parts & 8)) {       // at rest: nothing to add
		if ((int32_t)(end - c4_tick) > 0) c4_tick = end;
		return;
	}
	while ((int32_t)(end - c4_tick) > 0) {
		int n = (int)((end - c4_tick) >> 2);
		if (n > NBUF) n = NBUF;
		if (dquiet & 8) {
			c4_tick = end;
			break;
		}
		if (!slots) chanl4_run(m48_at(c4_tick, &n, 1), n);
		else c4_slots(n);
		if (dquiet & 8) c4_rest_at = c4_tick + 4u * (uint32_t)n;
		c4_tick += 4u * (uint32_t)n;
	}
}

static void disc_in(int c, uint8_t d) {
	if (din[c] == d) return;            // (dss_input_data: only a change updates the stream)
	uint32_t t = tick_now();
	if (!at_zero) {
		if (c == 3) c4_make(t);
		else disc_make(t);
	}
	din[c] = d;
	din_v[c] = dv[c][d & 15];
	dquiet &= (uint8_t)~(1u << c);
}

// ---------------------------------------------------------------------------
// the filters, from MAME's sums

static void filt2(float *k, int type, double fc, double d, double gain, double sr) {     // calculate_filter2_coefficients
	double tt = 2 * sr, tts = tt * tt, w = sr * 2.0 * tan(M_PI * fc / sr), ws = w * w;
	double den = tts + d * w * tt + ws;
	double b0 = type == 0 ? ws / den : type == 2 ? d * w * tt / den : tts / den;
	k[0] = (float)(b0 * gain);
	k[1] = (float)(2.0 * (-tts + ws) / den);
	k[2] = (float)((tts - d * w * tt + ws) / den);
}

static void dac(double *v, const double *r) {  // DISCRETE_DAC_R1, 4 V on
	double rt = 1.0 / (1.0 / r[0] + 1.0 / r[1] + 1.0 / r[2] + 1.0 / r[3]);
	for (int i = 0; i < 16; i++) {
		double it = 0;
		for (int b = 0; b < 4; b++) if ((i >> b) & 1) it += 4.0 / r[b];
		v[i] = it * rt;
	}
}

static void coefficients(void) {
	// the engine's (polepos_sound_device::device_start), for the machine's 48 kHz
	static const double r1[2] = { 220e3, 150e3 }, r2[2] = { 33e3, 22e3 }, r3[2] = { 390e3, 330e3 }, c[2] = { 0.01e-6, 0.0047e-6 };
	for (int k = 0; k < 2; k++) {       // opamp_m_bandpass_setup
		double r_in = 1.0 / (1.0 / r1[k] + 1.0 / r2[k]), gain = r2[k] / (r1[k] + r2[k]);
		double fc = 1.0 / (2 * M_PI * sqrt(r_in * r3[k] * c[k] * c[k]));
		double d = 2 * c[k] / sqrt(r3[k] / r_in * c[k] * c[k]);
		filt2(eb[k], 2, fc, d, gain * -r3[k] / r_in * 0.5, 48000);
	}
	filt2(eb[2], 1, 950, 1.0 / .707, 1, 48000);
	static const double rout[3] = { 4700, 7500, 10000 };
	double rtot = 1.0 / (1.0 / rout[0] + 1.0 / rout[1] + 1.0 / rout[2]);
	for (int k = 0; k < 3; k++) ew[k] = (float)(rtot / 2 / rout[k] * (double)(float)(0.90 * 0.77) / (double)DGAIN);
	// volume_table (its R167 and R168 shunts are R166's, as MAME has them)
	double sh = 1.0 / (1.0 / 1000.0 + 1.0 / 250);
	for (int s = 0; s < 8; s++) {
		double vol = ((s & 4 ? 4700 : sh) + (s & 2 ? 2200 : sh) + (s & 1 ? 1000 : sh) + 2200) / 10000;
		eng_k1[s] = (float)(3.4 / 255 * vol);
		eng_k0[s] = (float)(-2 * vol);
	}
	// the discrete circuits
	static const double r54[4] = { 47e3, 22e3, 10e3, 4.7e3 }, r52[4] = { 100e3, 47e3, 22e3, 10e3 };
	double d54[16], d52[16];
	dac(d54, r54);
	dac(d52, r52);
	double r54t = 1.0 / (1.0 / 47e3 + 1.0 / 22e3 + 1.0 / 10e3 + 1.0 / 4.7e3);
	static const double rin[3] = { 22e3, 15e3, 22e3 }, rg[3] = { 22e3, 15e3, 12e3 }, rf[3] = { 180e3, 120e3, 120e3 },
	                    cc[3] = { 0.047e-6, 0.022e-6, 0.0022e-6 };         // CHANL3, 2, 1
	for (int k = 0; k < 3; k++) {       // DISC_OP_AMP_FILTER_IS_BAND_PASS_1M
		double r = r54t + rin[k], rt = 1.0 / (1.0 / r + 1.0 / rg[k]);
		double fc = 1.0 / (2 * M_PI * sqrt(rt * rf[k] * cc[k] * cc[k]));
		double d = 2 * cc[k] / sqrt(rf[k] / rt * cc[k] * cc[k]);
		filt2(dc[k], 2, fc, d, -rf[k] / rt * 0.5, 48000);
		for (int i = 0; i < 16; i++) dv[k][i] = (float)((d54[i] - (double)VREF) / r * rt);
	}
	filt2(dc[3], 1, 100, 1.0 / 0.3, 1, 48000);
	filt2(dl, 0, 1200, 1.0 / 0.8, 1, 48000);
	for (int i = 0; i < 16; i++) dv[3][i] = (float)(d52[i] - (double)VREF);
}

// ---------------------------------------------------------------------------
// the board's calls

static bool ending, after;              // (the output's: see there)

void ppsnd_init(const pp_roms_t *r) {
	eng_rom = r->engine;
	for (int w = 0; w < 8; w++) {
		int32_t p = 0;
		for (int s = 0; s < 32; s++) {
			int32_t v = (r->wave[w * 32 + s] & 0x0f) - 8;
			wave_i[w][s].p = p << 17;
			wave_i[w][s].w = v;
			p += v;
		}
		wave_cycle[w] = p << 17;
	}
	coefficients();
	// device_start: the WSG enabled, everything else 0
	memset(voice, 0, sizeof voice);
	memset(wregs, 0, sizeof wregs);
	wsg_enable = true;
	wsg_on = 0;
	eng_pos = 0; eng_msb = eng_lsb = 0; eng_enable = false;
	eng_rate();
	ex1 = ex2 = 0; memset(ey, 0, sizeof ey); memset(eh, 0, sizeof eh);
	memset(din, 0, sizeof din);
	for (int c = 0; c < 4; c++) din_v[c] = dv[c][0];
	memset(dx, 0, sizeof dx); memset(dy, 0, sizeof dy); memset(dly, 0, sizeof dly);
	dquiet = 0;
	// the bins from time 0
	memset(bin_acc, 0, sizeof bin_acc);
	bin_read = bin_alloc = bin_frac = 0;
	bin_new();
	memset(m48, 0, sizeof m48);
	wsg_pos = eng_tick = disc_tick = c4_tick = m48_t = 0;
	memset(s8, 0, sizeof s8);
	s8_k = s8_t = c4_rest_at = bin_t0 = 0;
	s8_used = slots = talking = ending = after = false;
	cur_start(&wsg_cur, WSG_LAG);
	cur_start(&m48_cur, 0);
}

void ppsnd_reset(void) {                // device_reset: the engine's filters; the discrete's inputs (not its filters)
	TAP(PPSND_EV_RESET, 0, 0);
	ex1 = ex2 = 0;
	memset(ey, 0, sizeof ey);
	for (int c = 0; c < 4; c++) disc_in(c, 0);
}

static inline __attribute__((always_inline)) void make_all(void) {
	uint32_t t = tick_now();
	if (at_zero) return;
	wsg_make(t + 1);
	eng_make(t);
	disc_make(t);
	if (!slots || !(dquiet & 8)) c4_make(t);        // (at rest, the slots' time doesn't matter)
	uint32_t to = eng_tick + ENG_M48;   // the ring's samples all have made
	if ((int32_t)(disc_tick - to) < 0) to = disc_tick;
	if (!slots && (int32_t)(c4_tick - to) < 0) to = c4_tick;
	m48_box(to);
}

void ppsnd_update(void) {
	TAP(PPSND_EV_UPDATE, 0, 0);
	make_all();
}

void ppsnd_enable(int state) {
	TAP(PPSND_EV_ENABLE, 0, state);
	wsg_enable = state != 0;            // sound_enable_w: no update first, as MAME
	if (!state) {                       // clson_w: the engine's registers to 0
		uint32_t t = tick_now();
		if (!at_zero) eng_make(t);
		eng_lsb = eng_msb = 0;
		eng_enable = false;
		eng_rate();
	}
}

void ppsnd_wsg(int offset, uint8_t d) {
	TAP(PPSND_EV_WSG, offset, d);
	offset &= 0x3f;
	if (wregs[offset] == d) return;
	uint32_t t = tick_now();
	if (!at_zero) wsg_make(t + 1);
	wregs[offset] = d;
	wsg_voice((offset & 0x1f) >> 2);
}

void ppsnd_engine(int msb, uint8_t d) {
	TAP(PPSND_EV_ENGINE, msb, d);
	uint32_t t = tick_now();
	if (!at_zero) eng_make(t);
	if (msb) eng_msb = d & 63;
	else { eng_lsb = d & 62; eng_enable = d & 1; }
	eng_rate();
}

void ppsnd_52xx(uint8_t d) {
	TAP(PPSND_EV_52XX, 0, d);
	disc_in(3, d & 15);
}

void ppsnd_54xx(int n, uint8_t d) {
	TAP(PPSND_EV_54XX, n, d);
	disc_in(n, d & 15);
}

// ---------------------------------------------------------------------------
// the output

void ppsnd_rate_q8(uint32_t hz_q8) {
	if (hz_q8 < (800u << 8)) hz_q8 = 800u << 8;        // a bin's ticks: 4 to 255
	if (hz_q8 > (48000u << 8)) hz_q8 = 48000u << 8;
	bin_step = ((uint64_t)TICK_HZ << 40) / hz_q8;      // from the next bin
}

static uint32_t bins_done(void) {      // the bins every stream has passed
	uint32_t done = wsg_cur.k;
	if ((int32_t)(m48_cur.k - done) < 0) done = m48_cur.k;
	return done;
}

static inline float bin_value(uint32_t k) { return bin_acc[k & NBM] * (32767.0f / (float)bin_len[k & NBM]); }

static inline int16_t to16(float v) {
	v += v < 0 ? -0.5f : 0.5f;
	return (int16_t)(v > 32767.0f ? 32767 : v < -32767.0f ? -32767 : (int32_t)v);
}

static void bin_drop(void) {            // bin_read taken
	bin_acc[bin_read & NBM] = 0;
	bin_read++;
}

static inline void bin_drop_t(void) {   // the same, bin_t0 kept
	bin_t0 += bin_len[bin_read & NBM];
	bin_drop();
}

static int take_bins(int16_t *out, int max, uint32_t done) {
	int n = 0;
	while (n < max && (int32_t)(done - bin_read) > 0) {
		float v = bin_value(bin_read);
		bin_drop();
		out[n++] = to16(v);
	}
	return n;
}

static uint32_t bins_t0(void) {        // bin_read's first tick, from the 48 kHz streams' bin's end
	uint32_t t = m48_cur.end, k = m48_cur.k;
	if ((int32_t)(k - bin_read) < 0)    // (bins lost: near enough)
		return t + (bin_read - k - 1) * bin_len[bin_read & NBM];
	for (; k != bin_read - 1; k--) t -= bin_len[k & NBM];
	return t;
}

int ppsnd_take(int16_t *out, int max) {
	make_all();
	return take_bins(out, max, bins_done());
}

// While the 52xx talks, a list's take is the slots in a window a list long
// (from ow and of: a tick and its fraction << 16), which ends SP_LEAD ticks
// or more before the bins done. Each slot: CHANL4's mean over it, and the
// rest (the bins: the WSG, the engine, the 54xx's) as the bin at its middle
// has it, at rest_f. The run loop's player makes the samples
// PPSND_SPEECH_GAIN_Q8 / 256 louder, so rest_f = 256 / that is the rest at
// its usual level (where it starts and ends) and SP_REST half of it.
#define LIST_TQ16 ((uint32_t)(60036ull * TICK_HZ * 65536u / 3579545u))     // a list in ticks << 16
#define T_TICK_Q16 1221818u             // the console's T in a tick << 16 (3579545 / 192000)
#define SP_LEAD 80
#define SP_HANG (TICK_HZ * 3 / 10)      // the speech goes on 0.3 s after CHANL4 is at rest
#define SP_EDGE (256.0f / PPSND_SPEECH_GAIN_Q8)
#define SP_REST (0.5f * SP_EDGE)
#define SP_RAMP ((SP_EDGE - SP_REST) / 128)     // a slot: down or up in a list
#define SP_C4 (DGAIN * 32767.0f / 6)            // a slot's six samples' sum to the output's units
static uint32_t ow;
static uint16_t of;
static float rest_f;

static int take_speech(int16_t *out, uint16_t *pos, uint32_t done) {
	uint32_t h = bin_t0 = bins_t0();    // the bins done end here
	for (uint32_t k = bin_read; k != done; k++) h += bin_len[k & NBM];
	uint32_t e = (uint32_t)of + LIST_TQ16, w1 = ow + (e >> 16);
	uint16_t f1 = (uint16_t)e;
	if ((int32_t)(w1 - (h - SP_LEAD)) > 0) { w1 = h - SP_LEAD; f1 = 0; }       // (the first: shorter)
	uint32_t start = ow + (of != 0), stop = w1 + (f1 != 0);       // the slots that start in it
	while ((int32_t)(s8_t - start) < 0) { s8[s8_k & S8M] = 0; s8_k++; s8_t += 24; }
	float target = ending ? SP_EDGE : SP_REST, rv = 0;
	uint32_t rk = bin_read - 1;
	uint64_t p = ((((uint64_t)(s8_t - ow)) << 16) - of) * T_TICK_Q16;     // the slot's place, T << 32
	int n = 0;
	while ((int32_t)(s8_t - stop) < 0 && n < PPSND_SPEECH_MAX) {
		uint32_t mid = s8_t + 12;
		while ((int32_t)(done - bin_read) > 1 && (int32_t)(mid - (bin_t0 + bin_len[bin_read & NBM])) >= 0) bin_drop_t();
		if (rk != bin_read) { rk = bin_read; rv = bin_value(rk); }
		if (rest_f < target) rest_f = fminf(rest_f + SP_RAMP, target);
		else if (rest_f > target) rest_f = fmaxf(rest_f - SP_RAMP, target);
		float *slot = &s8[s8_k & S8M];
		out[n] = to16(*slot * SP_C4 + rv * rest_f);
		*slot = 0;
		pos[n++] = (uint16_t)(p >> 32);
		p += (24ull << 16) * T_TICK_Q16;
		s8_k++;
		s8_t += 24;
	}
	while ((int32_t)(done - bin_read) > 0 && (int32_t)(w1 - (bin_t0 + bin_len[bin_read & NBM])) >= 0) bin_drop_t();
	ow = w1;
	of = f1;
	return n;
}

int ppsnd_take_list(int16_t *out, uint16_t *pos, int max, bool *speech) {
	slots = true;
	make_all();
	uint32_t done = bins_done();
	bool live = !(dquiet & 8) && (ppsnd_parts & 8);
	if (!talking && live) {             // the 52xx has begun: from the first bin not taken
		talking = true;
		ow = bins_t0();
		of = 0;
		rest_f = SP_EDGE;
	}
	if (talking) {
		if (live) ending = false;
		else if ((int32_t)(ow - c4_rest_at) > (int32_t)SP_HANG) ending = true;
		*speech = true;
		int n = take_speech(out, pos, done);
		if (ending && rest_f >= SP_EDGE) {
			talking = ending = false;
			after = true;
		}
		return n;
	}
	*speech = false;
	if (after) {                        // the speech's windows ended SP_LEAD early: those bins dropped
		after = false;
		while ((int32_t)(done - bin_read) > max) bin_drop();
	}
	int n = take_bins(out, max, done);
	if (s8_used) {                      // CHANL4 is at rest: the slots to 0 (c4_slots moves them on)
		memset(s8, 0, sizeof s8);
		s8_used = false;
	}
	return n;
}
