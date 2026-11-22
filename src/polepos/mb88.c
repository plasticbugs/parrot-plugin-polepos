// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
//
// mb88.c -- see mb88.h. MAME 0.288's mb88_cpu_device in C: device_reset,
// serial_timer, write_pla, execute_set_input, pio_enable, increment_timer,
// burn_cycles and execute_run, around the instructions of mb88_ops.h
// (tools/mb88gen.py). The MCU that runs is M. MAME's license is in
// README.md.
#include "mb88.h"
#include <string.h>

#if defined(PICO_ON_DEVICE) && PICO_ON_DEVICE
#define MB88_HOT __attribute__((section(".time_critical.mb88")))
#else
#define MB88_HOT
#endif

typedef uint8_t u8;
typedef uint16_t u16;

static mb88_t *M;                       // the one running
void (*mb88_trace)(const mb88_t *m);

// mb88xx.cpp's constants and macros
#define SERIAL_PRESCALE         6
#define TIMER_PRESCALE          32
#define SERIAL_DISABLE_THRESH   1000
#define INT_CAUSE_SERIAL        0x01
#define INT_CAUSE_TIMER         0x02
#define INT_CAUSE_EXTERNAL      0x04

#define READOP(a)           (m->rom[(a) & 0x3ff])     // 10-bit program space (m: the MCU in mb88_run)
#define RDMEM(a)            (m->ram[(a) & 0x3f] & 0x0f)
#define WRMEM(a,v)          (m->ram[(a) & 0x3f] = (u8)((v) & 0x0f))
#define TEST_ST()           (m_st & 1)
#define TEST_ZF()           (m_zf & 1)
#define TEST_CF()           (m_cf & 1)
#define TEST_VF()           (m_vf & 1)
#define TEST_SF()           (m_sf & 1)
#define TEST_IF()           (m_if & 1)
#define UPDATE_ST_C(v)      m_st = (v & 0x10) ? 0 : 1
#define UPDATE_ST_Z(v)      m_st = (v == 0) ? 0 : 1
#define UPDATE_CF(v)        m_cf = ((v & 0x10) == 0) ? 0 : 1
#define UPDATE_ZF(v)        m_zf = (v != 0) ? 0 : 1
// PC and PA are one register in mb88_run, pcw_ = PA << 6 | PC (INCPC's carry into PA is its + 1)
#define GETPC()             ((int)pcw_)
#define GETEA()             ((m_X << 4) + m_Y)
#define INCPC()             (pcw_++)
#define SET_PC(v)           (pcw_ = (pcw_ & ~0x3fu) | ((unsigned)(v) & 0x3f))
#define SET_PA(v)           (pcw_ = (pcw_ & 0x3fu) | (unsigned)(u8)(v) << 6)

void mb88_reset(mb88_t *m) {
	mb88_wake(m);
	m->pz = m->pw = 0;
	m->PC = 0;
	m->PA = 0;
	m->SP[0] = m->SP[1] = m->SP[2] = m->SP[3] = 0;
	m->SI = 0;
	m->A = 0;
	m->X = 0;
	m->Y = 0;
	m->st = 1;
	m->zf = 0;
	m->cf = 0;
	m->vf = 0;
	m->sf = 0;
	m->pio = 0;
	m->TH = 0;
	m->TL = 0;
	m->TP = 0;
	m->SB = 0;
	m->SBcount = 0;
	m->pending_irq = 0;
	m->in_irq = 0;
}

MB88_HOT void mb88_serial_timer(mb88_t *m) {
	mb88_wake(m);
	m->SBcount++;
	if (m->SBcount >= SERIAL_DISABLE_THRESH)
		mb88_serial_adjust(m->id, false);
	if (!m->sf) {
		m->SB = (u8)((m->SB >> 1) | (mb88_read_si(m->id) ? 8 : 0));
		if (m->SBcount >= 4) {
			m->sf = 1;
			m->pending_irq |= INT_CAUSE_SERIAL;
		}
	}
}

MB88_HOT void mb88_set_input(mb88_t *m, int inputnum, int state) {
	state = state ? 1 : 0;
	if (inputnum == MB88_IRQ_LINE && state == m->iff) return;  // (no edge: nothing changes)
	mb88_wake(m);
	switch (inputnum) {
	case MB88_IRQ_LINE:
		if (!m->iff && state && (m->pio & INT_CAUSE_EXTERNAL))
			m->pending_irq |= INT_CAUSE_EXTERNAL;
		m->iff = (u8)state;
		break;
	case MB88_TC_LINE:
		if (m->ctr && !state && (m->pio & 0x40)) {
			m->TL = (m->TL + 1) & 0x0f;             // increment_timer
			if (m->TL == 0) {
				m->TH = (m->TH + 1) & 0x0f;
				if (m->TH == 0) {
					m->vf = 1;
					m->pending_irq |= INT_CAUSE_TIMER;
				}
			}
		}
		m->ctr = (u8)state;
		break;
	default:
		break;
	}
}

// ---------------------------------------------------------------------------
// execute_run, with the MCU's registers in locals: the m_ names of
// mb88_ops.h are these (or the MCU's fields, for the ones seldom used). A
// call to the board can read the cycle count (pp.c's now()) or end the
// timeslice (mb88_abort), so the count goes to M->icount before it and
// comes back after.

#define m_A A_
#define m_X X_
#define m_Y Y_
#define m_st st_
#define m_zf m->zf
#define m_cf m->cf
#define m_pio pio_
#define m_pending_irq pend_
#define m_in_irq m->in_irq
#define m_icount icount_
#define m_SP m->SP
#define m_SI m->SI
#define m_vf m->vf
#define m_sf m->sf
#define m_if m->iff
#define m_TH m->TH
#define m_TL m->TL
#define m_TP m->TP
#define m_SB m->SB
#define m_SBcount m->SBcount
#define m_o_output m->o_output

#define SYNC_OUT() (M->icount = icount_)
#define SYNC_IN() (icount_ = M->icount)
// (a write that changes a port is counted: a periodic wait has none)
#define mb88_write_r(id, n, d) do { \
	const int n_ = (n) & 3; const u8 d_ = (u8)(d); \
	if (m->outr[n_] != d_) { m->outr[n_] = d_; m->outs++; } \
	SYNC_OUT(); mb88_write_r(id, n_, d_); SYNC_IN(); \
} while (0)
#define mb88_write_p(id, d) do { \
	const u8 d_ = (u8)(d); \
	if (m->outp != d_) { m->outp = d_; m->outs++; } \
	SYNC_OUT(); mb88_write_p(id, d_); SYNC_IN(); \
} while (0)
#define mb88_serial_on() (SYNC_OUT(), mb88_serial_adjust(M->id, true), SYNC_IN(), SLOW())

// write_pla, 8-bit PLA mode (no PLA data)
#define write_pla(index) do { \
	const u8 i_ = (u8)(index), sh_ = (i_ & 0x10) ? 4 : 0, mk_ = (u8)(0xf << sh_); \
	const u8 o_ = (u8)((m_o_output & ~mk_) | ((i_ << sh_) & mk_)); \
	if (o_ != m_o_output) m->outs++; \
	m_o_output = o_; \
	SYNC_OUT(); mb88_write_o(M->id, m_o_output, mk_); SYNC_IN(); \
} while (0)

// pio_enable
#define pio_enable(newpio) do { \
	const u8 np_ = (u8)(newpio); \
	if ((pio_ ^ np_) & 0x30) { \
		SYNC_OUT(); \
		if ((np_ & 0x30) == 0x20) mb88_serial_adjust(M->id, true); \
		else mb88_serial_adjust(M->id, false);   /* (0x10 and 0x30 are MAME's fatalerror; never done) */ \
		SYNC_IN(); \
	} \
	pio_ = np_; \
	SLOW(); \
} while (0)

#define INCREMENT_TIMER() do { \
	m_TL = (m_TL + 1) & 0x0f; \
	if (m_TL == 0) { \
		m_TH = (m_TH + 1) & 0x0f; \
		if (m_TH == 0) { m_vf = 1; pend_ |= INT_CAUSE_TIMER; } \
	} \
} while (0)

#define PRESCALE(c) do { \
	if (pio_ & 0x80) { \
		m_TP += (c); \
		while (m_TP >= TIMER_PRESCALE) { m_TP -= TIMER_PRESCALE; INCREMENT_TIMER(); } \
	} \
} while (0)

// the serial timer's ticks for the cycles just run, after the interrupt
// check as MAME's timer comes between instructions (mb88_serial_timer)
#define SERIAL_TICKS(n) do { \
	for (int k_ = (n); k_ > 0 && m->ser_on; k_--) { \
		if (++m_SBcount >= SERIAL_DISABLE_THRESH) m->ser_on = 0; \
		if (!m_sf) { \
			m_SB = (u8)((m_SB >> 1) | (mb88_read_si(m->id) ? 8 : 0)); \
			if (m_SBcount >= 4) { m_sf = 1; pend_ |= INT_CAUSE_SERIAL; } \
		} \
	} \
} while (0)

// a period being checked: this instruction's end
#define PZ_MARK() do { \
	const uint32_t o_ = m->cyc + (uint32_t)(m->cycles_running - icount_) - m->pz_cyc - 1; \
	if (o_ < 128) m->pz_ends[o_ >> 5] |= 1u << (o_ & 31); \
	else m->pz = 0;                     /* (too long: another from the next check) */ \
} while (0)

// burn_cycles, with its interrupt (whose burn_cycles(3) takes no other)
// (slow_: the prescaler on, an interrupt pending, the serial port or a
// period being checked; else an instruction only counts its cycles)
#define SLOW() (slow_ = (u8)((pio_ & 0x80) | (pend_ & pio_) | m->ser_on | m->pz | m->stop_rti))
#define BURN(c) do { \
	icount_ -= (c); \
	if (slow_) { BURN_SLOW(c); SLOW(); } \
} while (0)
#define BURN_SLOW(c) do { \
	const int ic0_ = icount_ + (c); \
	PRESCALE(c); \
	if ((pend_ & pio_) && !m->in_irq) { \
		m->in_irq = true; \
		m->irqs++; \
		if (m->stop_rti == 1) { memcpy(m->irq_snap, m->ram, 64); m->stop_rti = 2; } \
		u16 intpc_ = (u16)GETPC(); \
		m_SP[m_SI] = (u16)(intpc_ | TEST_CF() << 15 | TEST_ZF() << 14 | TEST_ST() << 13); \
		m_SI = (m_SI + 1) & 3; \
		if (pend_ & pio_ & INT_CAUSE_EXTERNAL) pcw_ = 0x02; \
		else if (pend_ & pio_ & INT_CAUSE_TIMER) pcw_ = 0x04; \
		else if (pend_ & pio_ & INT_CAUSE_SERIAL) pcw_ = 0x06; \
		else pcw_ &= 0x3f; \
		st_ = 1; \
		pend_ = 0; \
		icount_ -= 3; \
		PRESCALE(3); \
	} \
	if (m->ser_on | m->pz) { \
		if (m->ser_on) SERIAL_TICKS(ic0_ - icount_); \
		if (m->pz) PZ_MARK(); \
	} \
	if (m->stop_rti == 2 && !m->in_irq) {   /* its RTI: the run ends here */ \
		m->stop_rti = 3; \
		if (icount_ > 0) { m->cycles_stolen += icount_; m->cycles_running -= icount_; icount_ = 0; } \
	} \
} while (0)

// The customs wait for the 06xx in loops that read nothing that can change
// while they wait, but the IRQ line, the 51xx's TC count and the serial
// port, which come from outside (mb88_set_input, mb88_serial_timer). So
// once a loop has gone round once with no interrupt it can take, the rounds
// after are all the same, and the MCU waits there without running: each
// timeslice's cycles are counted against the loop's instructions, so it
// ends where MAME's would, and the prescaler is counted the same way (a
// timeslice that would reach the timer's overflow, which interrupts, runs
// as usual). It runs again, from where the count says, at whatever comes
// from outside, or when the tests read it (mb88_wake). The loops, found in
// the program:
//   1: tsti; jmp self                       (2 cycles; while IRQ low)
//   2: tsti; jmp x; jmp back                (3 cycles; while IRQ high)
//   3: ttla; c; jpl x; jmp back             (5 cycles; while M[X,Y] = TL, the prescaler off)
//   4: lyi n; l; ci k; jmp back             (4 cycles; while M[X,n] != k: the 52xx waiting
//                                           for its timer's interrupt to change it)
enum { IDLE_NONE, IDLE_TSTI1, IDLE_TSTI2, IDLE_TTLA, IDLE_POLL };
unsigned mb88_skipped[4];               // the cycles not run (tests)
static u8 idle_len[] = { 0, 2, 3, 5, 4 };
// nb[t][x]: the first instruction's end at or past x cycles from the top (x 1..len)
static u8 idle_nb[5][6] = { { 0 }, { 0, 1, 2 }, { 0, 1, 2, 3 }, { 0, 1, 2, 4, 4, 5 }, { 0, 1, 2, 3, 4 } };
static bool quiet;                      // catching up: no waiting

void mb88_find_idle(mb88_t *m) {
	for (int pc = 0; pc < 0x400; pc++) {
		const u8 *r = m->rom;
		u8 t = IDLE_NONE;
		if (r[pc] == 0x25 && (pc & 0x3f) <= 0x3e && r[pc + 1] == (0xc0 | (pc & 0x3f)))
			t = IDLE_TSTI1;
		else if (r[pc] == 0x25 && (pc & 0x3f) <= 0x3d && r[pc + 1] >= 0xc0 && r[pc + 2] == (0xc0 | (pc & 0x3f)))
			t = IDLE_TSTI2;
		else if (r[pc] == 0x16 && (pc & 0x3f) <= 0x3b && r[pc + 1] == 0x2e && (r[pc + 2] & 0xf8) == 0x68 &&
		         r[pc + 4] == (0xc0 | (pc & 0x3f)))
			t = IDLE_TTLA;
		else if ((r[pc] & 0xf0) == 0x80 && (pc & 0x3f) <= 0x3c && r[pc + 1] == 0x0d && (r[pc + 2] & 0xf0) == 0xb0 &&
		         r[pc + 3] == (0xc0 | (pc & 0x3f)))
			t = IDLE_POLL;
		m->idle[pc] = t;
	}
	m->lz = 0;
}

// x cycles on from off (1..len, or 0 at the top) in loop t: where it stops,
// as rounds and off; the cycles that takes
MB88_HOT static inline int lz_step(int t, int off, int x, uint32_t *rounds, u8 *noff) {
	int len = idle_len[t];
	int y = off + x, k = (y - 1) / len;
	int end = k * len + idle_nb[t][y - k * len];
	int q = (end - 1) / len;
	*rounds += (uint32_t)q;
	*noff = (u8)(end - q * len);
	return end - off;
}

// A loop that its timer interrupts may come back from each interrupt to
// the same state, with nothing written to a port that changes it (the
// 52xx, silent: its interrupt writes the DAC's middle every 64 cycles).
// Then each period after is the same as the last, until something comes
// from outside, and the MCU waits in the state of a period's start without
// running: the cycles are counted against the period's instructions (their
// ends were marked as it ran), and mb88_wake runs it on from the period's
// start as far as the count says. The check is at the loop's top, the
// first time after an interrupt; the board gives the MCUs whose inputs all
// come through mb88_set_input or mb88_wake a buffer for it (pz_snap). The
// period may take more than one interrupt (the 52xx's two: they come at
// one place in its 2-cycle loop, then the other); up to 128 cycles.

// a period being checked: the ends of c cycles of loop t's rounds, from its
// top o cycles into the period
MB88_HOT static void pz_rounds(mb88_t *m, int t, uint32_t o, int c) {
	int len = idle_len[t];
	if (o + (uint32_t)c > 128) { m->pz = 0; return; }
	for (int k = 0, r = 1; k < c; k++, o++, r = r == len ? 1 : r + 1)
		if (idle_nb[t][r] == r) m->pz_ends[o >> 5] |= 1u << (o & 31);
}

// x cycles on from pw_off: the cycles to the first instruction's end at or
// past it; pw_off then that end
MB88_HOT static int pw_step(mb88_t *m, int x) {
	int len = m->pw_len, y = m->pw_off + x;
	int q = (y - 1) / len, b = y - q * len - 1;         // the bit for y in its period
	int i = b >> 5;
	uint32_t w = m->pz_ends[i] & (~0u << (b & 31));
	while (!w) w = m->pz_ends[++i];                     // (bit len - 1 is set)
	int end = q * len + (i << 5) + __builtin_ctz(w) + 1;
	int ran = end - m->pw_off;
	m->pw_off = (u16)(end % len);
	return ran;
}

MB88_HOT void mb88_wake(mb88_t *m) {
	m->pz = 0;
	if (m->pw) {                        // from the period's start, as far as the count
		int off = m->pw_off;
		m->pw = 0;
		if (off) {
			quiet = true;
			mb88_run(m, off);
			quiet = false;
		}
		return;
	}
	if (!m->lz) return;
	int t = m->lz, off = m->lz_off;
	m->lz = 0;
	if (m->pio & 0x80) {                // the prescaler, for the whole rounds
		int c = m->TP + (int)m->lz_rounds * idle_len[t];
		int v = (m->TH << 4 | m->TL) + c / TIMER_PRESCALE;
		m->TP = (u8)(c % TIMER_PRESCALE);
		m->TH = (u8)(v >> 4);
		m->TL = (u8)(v & 15);
	}
	quiet = true;                       // the last round so far, run
	mb88_run(m, off);
	quiet = false;
}

// at the loop's top (after the jmp that came back to it), icount_ left:
// true if it waits from here (icount_ then the timeslice's end)
static MB88_HOT bool lz_enter(mb88_t *m, int t, int *icount_, u8 pio_) {
	uint32_t r = 0;
	u8 off;
	int end = lz_step(t, 0, *icount_, &r, &off);
	int ovf = (TIMER_PRESCALE - m->TP) + TIMER_PRESCALE * (255 - (m->TH << 4 | m->TL));
	if ((pio_ & 0x80) && (end >= ovf || m->pz)) return false;      // (a period being checked: its rounds
	                                                                // are run or skipped in the timeslice)
	m->lz = (u8)t;
	m->pz = 0;                          // (no interrupt to come back from)
	m->lz_rounds = r;
	m->lz_off = off;
	m->lz_ovf = ovf;
	mb88_skipped[m->id] += (unsigned)end;
	*icount_ -= end;
	return true;
}

int MB88_HOT mb88_run(mb88_t *m, int cycles) {
	if (m->pw) {
		int ran = pw_step(m, cycles);
		m->cycles_running = cycles;
		m->cycles_stolen = 0;
		m->icount = cycles - ran;
		m->cyc += (uint32_t)ran;
		mb88_skipped[m->id] += (unsigned)ran;
		return ran;
	}
	if (m->lz) {
		uint32_t r = m->lz_rounds;
		u8 off;
		int ran = lz_step(m->lz, m->lz_off, cycles, &r, &off);
		if (!(m->pio & 0x80) || (int)r * idle_len[m->lz] + off < m->lz_ovf) {
			m->lz_rounds = r;
			m->lz_off = off;
			m->cycles_running = cycles;
			m->cycles_stolen = 0;
			m->icount = cycles - ran;
			mb88_skipped[m->id] += (unsigned)ran;
			return ran;
		}
		mb88_wake(m);                   // (it would reach the overflow)
	}
	mb88_t *prev = M;
	M = m;
	m->cycles_running = cycles;
	m->cycles_stolen = 0;
	int icount_ = cycles;
	unsigned pcw_ = (unsigned)m->PA << 6 | m->PC;
	u8 A_ = m->A, X_ = m->X, Y_ = m->Y, st_ = m->st;
	u8 pio_ = m->pio, pend_ = m->pending_irq, slow_;
	SLOW();
	const u8 *rom = m->rom;
	int idle_at = -1, idle_ic = 0;      // the loop's top a jmp came back to, and the count then
#define WRITE_BACK() do { \
	m->PC = (u8)(pcw_ & 0x3f); m->PA = (u8)(pcw_ >> 6); m->A = A_; m->X = X_; m->Y = Y_; m->st = st_; \
	m->pio = pio_; m->pending_irq = pend_; m->icount = icount_; \
} while (0)
	do {
		u8 opcode, arg, oc;
#ifndef PP_NO_TRACE
		if (mb88_trace) { WRITE_BACK(); mb88_trace(M); }
#endif
		opcode = rom[GETPC() & 0x3ff];
		INCPC();
		if (opcode >= 0xc0) {           // jmp, the most common, and how the waiting loops go round
			if (st_ & 1) SET_PC(opcode);
			st_ = 1;
			BURN(1);
			int pc = GETPC() & 0x3ff, t = m->idle[pc];
			if (t && m->irqs != m->pz_irqs && m->pz_snap && !m->in_irq && !quiet && !m->ser_on) {
				m->pz_irqs = m->irqs;   // back from an interrupt: the same as the last time?
				WRITE_BACK();
				uint32_t now = m->cyc + (uint32_t)(m->cycles_running - icount_);
				if (m->pz && m->pz_pc == pc && m->outs == m->pz_outs && now - m->pz_cyc <= 128 &&
				    !memcmp(m->pz_snap, &m->PC, MB88_PZ_LEN)) {
					m->pz = 0;
					m->pw = 1;
					m->pw_len = (u16)(now - m->pz_cyc);
					m->pw_off = 0;
					if (icount_ > 0) {
						int ran = pw_step(m, icount_);
						icount_ -= ran;
						mb88_skipped[m->id] += (unsigned)ran;
					}
					break;
				}
				if (!m->pz || m->pz_pc != pc || m->outs != m->pz_outs) {   // (else a period of two or more interrupts?)
					memcpy(m->pz_snap, &m->PC, MB88_PZ_LEN);
					m->pz = 1;
					m->pz_pc = (u16)pc;
					m->pz_cyc = now;
					m->pz_outs = m->outs;
					memset(m->pz_ends, 0, sizeof m->pz_ends);
				}
			}
			if (t && icount_ > 0) {
				if (pc == idle_at && idle_ic - icount_ == idle_len[t] && (m->in_irq || !(pend_ & pio_)) && !quiet &&
				    !m->ser_on &&
				    !(t == IDLE_TTLA && (pio_ & 0x80))) {
					if (lz_enter(m, t, &icount_, pio_)) break;
					// the timer overflows in this timeslice: the rounds before it, at once (the
					// 52xx waits here between its timer's interrupts, 64 cycles apart)
					int len = idle_len[t];
					int ovf = (TIMER_PRESCALE - m->TP) + TIMER_PRESCALE * (255 - (m->TH << 4 | m->TL));
					int can = (ovf < icount_ ? ovf : icount_) - 1;
					if (can >= len) {
						int c = can / len * len;
						int tc = m->TP + c, v = (m->TH << 4 | m->TL) + tc / TIMER_PRESCALE;
						m->TP = (u8)(tc % TIMER_PRESCALE);
						m->TH = (u8)(v >> 4);
						m->TL = (u8)(v & 15);
						if (m->pz) pz_rounds(m, t, m->cyc + (uint32_t)(m->cycles_running - icount_) - m->pz_cyc, c);
						icount_ -= c;
						mb88_skipped[m->id] += (unsigned)c;
					}
				}
				idle_at = pc;
				idle_ic = icount_;
			}
			if (t) SLOW();              // (the period's check, a skip)
			continue;
		}
		oc = 1;
		switch (opcode) {
#include "mb88_ops.h"
		}
		BURN(oc);
	} while (icount_ > 0);
	WRITE_BACK();
	M = prev;
	int ran = cycles - m->icount - m->cycles_stolen;
	m->cyc += (uint32_t)ran;
	return ran;
}

MB88_HOT void mb88_abort(void) {
	if (!M) return;
	if (M->icount > 0) {
		M->cycles_stolen += M->icount;
		M->cycles_running -= M->icount;
		M->icount = 0;
	}
}

MB88_HOT int mb88_done(void) { return M->cycles_running - M->icount; }
