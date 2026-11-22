// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
//
// z80.c -- see z80.h. MAME 0.288's z80_device (z80.cpp, z80.h, z80.inc) in
// C for one CPU: its registers, the helpers z80.lst calls, device_start,
// device_reset, execute_set_input and execute_run. The instructions are
// ../pacman/z80_ops.h (../pacman/tools/z80gen.py). This is Pac-Man's with
// the NMI line added. Left out, as Pole Position has none of them: the
// daisy chain, BUSREQ, WAIT, the refresh and no-MREQ callbacks, and the
// variants (NSC800, R800, Z80N, T6A84). MAME's license is in README.md.
#include "z80.h"

#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;

#if defined(PICO_ON_DEVICE) && PICO_ON_DEVICE
#define Z80_HOT __attribute__((section(".time_critical.z80")))
#else
#define Z80_HOT
#endif

typedef union { struct { u8 l, h; } b; u16 w; } pair16_t;

static struct {
	pair16_t prvpc, pc, sp, af, bc, de, hl, ix, iy, wz, af2, bc2, de2, hl2;
	u8 r, r2, halt, im, i, nmi_state, irq_state, busack_state, service_attention;
	bool iff1, iff2;
	int busreq_state;
	u16 ea;
	struct {
		u8 s_val, z_val, yx_val, h_val, pv_val;
		bool n, c;
		u8 q, qtemp;
	} f;
	int icount, tmp_irq_vector;
	pair16_t shared_data, shared_data2;
	u32 ref;
	int cycles_running, cycles_stolen;
} Z;

const u8 *z80_rd_page[256];
u8 *z80_wr_page[256];
void (*z80_trace)(void);

// z80.inc
#define HAS_LDAIR_QUIRK 0
#define CF  0x01
#define NF  0x02
#define PF  0x04
#define VF  PF
#define HF  0x10
#define YXF 0x28
#define ZF  0x40
#define SF  0x80
#define PRVPC  Z.prvpc.w
#define PC     Z.pc.w
#define SP     Z.sp.w
#define Q      Z.f.q
#define QT     Z.f.qtemp
#define I      Z.i
#define R      Z.r
#define R2     Z.r2
#define AF     Z.af.w
#define A      Z.af.b.h
#define F      Z.af.b.l
#define BC     Z.bc.w
#define B      Z.bc.b.h
#define C      Z.bc.b.l
#define DE     Z.de.w
#define D      Z.de.b.h
#define E      Z.de.b.l
#define HL     Z.hl.w
#define H      Z.hl.b.h
#define L      Z.hl.b.l
#define IX     Z.ix.w
#define HX     Z.ix.b.h
#define LX     Z.ix.b.l
#define IY     Z.iy.w
#define HY     Z.iy.b.h
#define LY     Z.iy.b.l
#define WZ     Z.wz.w
#define WZ_H   Z.wz.b.h
#define WZ_L   Z.wz.b.l
#define TDAT   Z.shared_data.w
#define TDAT2  Z.shared_data2.w
#define TDAT_H Z.shared_data.b.h
#define TDAT_L Z.shared_data.b.l
#define TDAT8  Z.shared_data.b.l

// z80.h: the service attention bits and the flags' accessors
#define SA_BUSREQ      0
#define SA_NMI_PENDING 1
#define SA_IRQ_ON      2
#define SA_HALT        3
#define SA_AFTER_EI    4
#define SA_AFTER_LDAIR 5
#define SET_SA(bit, state) ((state) ? (Z.service_attention |= (u8)(1u << (bit))) : (Z.service_attention &= (u8)~(1u << (bit))))
#define GET_SA(bit) ((Z.service_attention >> (bit)) & 1)
#define F_S()  (Z.f.s_val & 0x80)
#define F_Z()  (Z.f.z_val ? 0 : 0x40)
#define F_YX() (Z.f.yx_val & 0x28)
#define F_H()  (Z.f.h_val & 0x10)
#define F_PV() flag_pv(Z.f.pv_val)
#define CLEAR_LINE 0

// a standard Z80's bus timing (z80_device's m_m1_cycles, m_mreq_cycles, m_iorq_cycles)
#define Z80_M1   4
#define Z80_MREQ 3
#define Z80_IORQ 4

// no refresh or no-MREQ callback is set (pacman's), so nothing is called
#define refresh_en 0
#define nomreq_en 0
#define NOCB(...) ((void)0)

#define swap(a, b) do { __typeof__(a) t_ = (a); (a) = (b); (b) = t_; } while (0)
#define SWAP16(a, b) swap(a, b)

#define Z80_INLINE static inline __attribute__((always_inline))

Z80_INLINE u8 flag_pv(u8 val) {
	val ^= val >> 4;
	val ^= val << 2;
	val ^= val >> 1;
	return ~val & 0x04;
}

// ---- memory and I/O ------------------------------------------------------------------------
// (ic, the cycles left, is the execute function's own: the board's code
// sees it in Z.icount, and its abort_timeslice changes it there)
#define RD(a) rd(&ic, (u16)(a))
#define WR(a, v) wr(&ic, (u16)(a), (u8)(v))
#define RD_OP() RD(PC)
#define RD_ARG() RD(PC)
#define IO_IN(p) io_in(&ic, (u16)(p))
#define IO_OUT(p, v) io_out(&ic, (u16)(p), (u8)(v))

Z80_INLINE u8 rd(int *ic, u16 a) {
	const u8 *p = z80_rd_page[a >> 8];
	if (__builtin_expect(p != 0, 1)) return p[a & 0xff];
	Z.icount = *ic;
	return z80_board_rd(a);
}
Z80_INLINE void wr(int *ic, u16 a, u8 v) {
	u8 *p = z80_wr_page[a >> 8];
	if (__builtin_expect(p != 0, 1)) { p[a & 0xff] = v; return; }
	Z.icount = *ic;
	z80_board_wr(a, v);
	*ic = Z.icount;
}
Z80_INLINE u8 io_in(int *ic, u16 p) { Z.icount = *ic; return z80_board_in(p); }
Z80_INLINE void io_out(int *ic, u16 p, u8 v) { Z.icount = *ic; z80_board_out(p, v); *ic = Z.icount; }
#define z80_irq_ack() z80_board_irq_ack()
// Pole Position's main loop (0x00b1: call 0x0278; jr 0x00b1) waits for the
// Z8002s, reading 0x4010 and 0x4018 of their sprite RAM and writing the
// same values back each time round. Once it has gone round once with
// nothing to interrupt it, every round after is the same, so the Z80 waits
// there without running: a timeslice's cycles are counted against the
// loop's steps (where MAME's core can stop, learned once by running it), so
// each ends where MAME's would. It runs again, from where the steps say it
// is (with what it read then), when a line changes, when 0x4010 or 0x4018
// hold something new, or when the tests ask for its registers. (Not while
// tracing.)
typedef struct {
	int len, m1;                        // a round's cycles, and its opcode fetches (R)
	bool known;
	uint8_t nb[113];                    // nb[x]: the first place it can stop at or past x cycles from the top
} z80_loop_t;
static z80_loop_t loops[2];             // 0: (0x4010) isn't 0x73 (ld (0x4018),0); 1: it is, and (0x4018) isn't 0 (ret nz)
static int lz = -1;                     // the loop it waits in (-1: running)
static int lz_off;                      // how far past the top it is (1..len)
static uint32_t lz_rounds;              // and the whole rounds since the real state (at the top)
static uint8_t lz_m10, lz_m18;          // what the loop read
static bool lz_quiet;                   // running the loop to learn it or to catch up
static uint64_t z80_clock;              // cycles run, all timeslices
static uint64_t last_top = ~0ull >> 1;  // where it last came to the top
static int last_path = -1;
unsigned z80_skipped;                   // the cycles not run (tests)
unsigned z80_wakes[4];                  // why it ran again: a line, new values, the tests, a reset (tests)

static void execute(void);

static int loop_path(void) {
	if (z80_board_rd(0x4010) != 0x73) return 0;
	if (z80_board_rd(0x4018) != 0) return 1;
	return -1;
}

static void learn(z80_loop_t *l, int len, int m1) {
	typeof(Z) saved = Z;
	saved.ref = 0xffff00;               // (at an instruction's start)
	l->len = len;
	l->m1 = m1;
	lz_quiet = true;
	for (int c = 1; c <= len; c++) {
		Z = saved;
		Z.cycles_running = Z.icount = c;
		Z.cycles_stolen = 0;
		execute();
		l->nb[c] = (uint8_t)(c - Z.icount);
	}
	lz_quiet = false;
	Z = saved;
	l->known = true;
}

// at the top of the loop, ic cycles left: true if it waits from here (ic then the timeslice's end)
static Z80_HOT bool idle(int *ic) {
	if (lz_quiet) return false;
	int p = loop_path();
	uint64_t at = z80_clock + (uint64_t)(Z.cycles_running - *ic);
	bool again = p >= 0 && p == last_path && at - last_top == (uint64_t)(p ? 84 : 112);
	last_top = at;
	last_path = p;
	u8 sa = Z.service_attention;
	if (!again || !(sa == 0 || (sa == (1u << SA_IRQ_ON) && !Z.iff1))) return false;
	z80_loop_t *l = &loops[p];
	if (!l->known) learn(l, p ? 84 : 112, p ? 8 : 10);
	int x = *ic, k = (x - 1) / l->len;
	int end = k * l->len + l->nb[x - k * l->len];
	lz = p;
	lz_m10 = z80_board_rd(0x4010);
	lz_m18 = z80_board_rd(0x4018);
	lz_rounds = (uint32_t)((end - 1) / l->len);         // (a round's end is len, not 0: MAME's core
	lz_off = end - (int)lz_rounds * l->len;             //  stops in the jr there, before its last step)
	*ic -= end;
	z80_skipped += (unsigned)end;
	return true;
}

// the real state to where the loop has got to
void z80_wake(void) {
	if (lz < 0) return;
	const z80_loop_t *l = &loops[lz];
	R = (u8)(R + lz_rounds * (uint32_t)l->m1);
	int off = lz_off;
	lz = -1;
	last_top = ~0ull >> 1;
	{                                   // the last round so far, as it read then (any new values came after)
		u8 m10 = z80_board_rd(0x4010), m18 = z80_board_rd(0x4018);
		z80_board_wr(0x4010, lz_m10);
		z80_board_wr(0x4018, lz_m18);
		lz_quiet = true;
		Z.cycles_running = Z.icount = off;
		Z.cycles_stolen = 0;
		execute();
		lz_quiet = false;
		z80_board_wr(0x4010, m10);
		z80_board_wr(0x4018, m18);
	}
}

#ifdef PP_NO_TRACE
#define Z80_HOOK() do { if (PC == 0x00b1 && idle(&ic)) { Z.ref = 0xffff00; goto suspend; } } while (0)
#else
#define Z80_HOOK() do { if (z80_trace) { Z.icount = ic; z80_trace(); } \
                        else if (PC == 0x00b1 && idle(&ic)) { Z.ref = 0xffff00; goto suspend; } } while (0)
#endif

// ---- z80.cpp's helpers ---------------------------------------------------------------------
static u8 get_f(void) {
	u8 f = 0;
	f |= F_S();
	f |= F_Z();
	f |= F_YX();
	f |= F_H();
	f |= F_PV();
	f |= Z.f.n ? NF : 0;
	f |= Z.f.c ? CF : 0;
	return f;
}

static void set_f(u8 f) {
	Z.f.s_val = f;
	Z.f.z_val = !(f & ZF);
	Z.f.yx_val = f;
	Z.f.h_val = f;
	Z.f.pv_val = !(f & PF);
	Z.f.n = f & NF;
	Z.f.c = f & CF;
}

Z80_INLINE void halt(void) {
	if (!Z.halt) {
		Z.halt = 1;
		SET_SA(SA_HALT, 1);
	}
}

Z80_INLINE void leave_halt(void) {
	if (Z.halt) {
		Z.halt = 0;
		SET_SA(SA_HALT, 0);
	}
}

Z80_INLINE void inc(u8 *r) {
	++*r;
	QT = 0;
	// keep C
	Z.f.s_val = Z.f.z_val = Z.f.yx_val = *r;
	Z.f.pv_val = *r != 0x80;
	Z.f.h_val = (*r & 0x0f) == 0x00 ? HF : 0;
	Z.f.n = 0;
}

Z80_INLINE void dec(u8 *r) {
	--*r;
	QT = 0;
	// keep C
	Z.f.s_val = Z.f.z_val = Z.f.yx_val = *r;
	Z.f.pv_val = *r != 0x7f;
	Z.f.h_val = (*r & 0x0f) == 0x0f ? HF : 0;
	Z.f.n = 1;
}

Z80_INLINE void rlca(void) {
	A = (A << 1) | (A >> 7);
	QT = 0;
	// keep SZP
	Z.f.yx_val = A;
	Z.f.h_val = Z.f.n = 0;
	Z.f.c = A & 0x01;
}

Z80_INLINE void rrca(void) {
	const u8 a0 = A;
	A = (a0 >> 1) | (a0 << 7);
	QT = 0;
	// keep SZP
	Z.f.yx_val = A;
	Z.f.h_val = Z.f.n = 0;
	Z.f.c = a0 & 0x01;
}

Z80_INLINE void rla(void) {
	u8 res = (A << 1) + Z.f.c;
	QT = 0;
	// keep SZP
	Z.f.yx_val = res;
	Z.f.h_val = Z.f.n = 0;
	Z.f.c = A & 0x80;
	A = res;
}

Z80_INLINE void rra(void) {
	u8 res = (Z.f.c << 7) | (A >> 1);
	QT = 0;
	// keep SZP
	Z.f.yx_val = res;
	Z.f.h_val = Z.f.n = 0;
	Z.f.c = A & 0x01;
	A = res;
}

Z80_INLINE void add_a(u8 value) {
	const u16 res = A + value;
	QT = 0;
	Z.f.s_val = Z.f.z_val = Z.f.yx_val = (u8)res;
	Z.f.c = res & 0x100;
	Z.f.h_val = (A & 0x0f) + (value & 0x0f);
	Z.f.pv_val = !((A ^ res) & (value ^ res) & 0x80);
	Z.f.n = 0;
	A = (u8)res;
}

Z80_INLINE void adc_a(u8 value) {
	const int c = Z.f.c;
	const u16 res = A + value + c;
	QT = 0;
	Z.f.s_val = Z.f.z_val = Z.f.yx_val = (u8)res;
	Z.f.c = res & 0x100;
	Z.f.h_val = (A & 0x0f) + (value & 0x0f) + c;
	Z.f.pv_val = !((A ^ res) & (value ^ res) & 0x80);
	Z.f.n = 0;
	A = (u8)res;
}

Z80_INLINE void sub_a(u8 value) {
	const u16 res = A - value;
	QT = 0;
	Z.f.s_val = Z.f.z_val = Z.f.yx_val = (u8)res;
	Z.f.c = res & 0x100;
	Z.f.h_val = (A & 0x0f) - (value & 0x0f);
	Z.f.pv_val = !((A ^ value) & (A ^ res) & 0x80);
	Z.f.n = 1;
	A = (u8)res;
}

Z80_INLINE void sbc_a(u8 value) {
	const int c = Z.f.c;
	const u16 res = A - value - c;
	QT = 0;
	Z.f.s_val = Z.f.z_val = Z.f.yx_val = (u8)res;
	Z.f.c = res & 0x100;
	Z.f.h_val = (A & 0x0f) - (value & 0x0f) - c;
	Z.f.pv_val = !((A ^ value) & (A ^ res) & 0x80);
	Z.f.n = 1;
	A = (u8)res;
}

Z80_INLINE void neg(void) {
	u8 value = A;
	A = 0;
	sub_a(value);
}

Z80_INLINE void daa(void) {
	u8 a = A;
	if (Z.f.n) {
		if (F_H() || ((A & 0xf) > 9)) a -= 6;
		if (Z.f.c || (A > 0x99)) a -= 0x60;
	} else {
		if (F_H() || ((A & 0xf) > 9)) a += 6;
		if (Z.f.c || (A > 0x99)) a += 0x60;
	}
	QT = 0;
	// keep N
	Z.f.s_val = Z.f.z_val = Z.f.pv_val = Z.f.yx_val = a;
	Z.f.h_val = A ^ a;
	Z.f.c = Z.f.c || A > 0x99;
	A = a;
}

Z80_INLINE void and_a(u8 value) {
	A &= value;
	QT = 0;
	Z.f.s_val = Z.f.z_val = Z.f.pv_val = Z.f.yx_val = A;
	Z.f.n = Z.f.c = 0;
	Z.f.h_val = HF;
}

Z80_INLINE void or_a(u8 value) {
	A |= value;
	QT = 0;
	Z.f.s_val = Z.f.z_val = Z.f.pv_val = Z.f.yx_val = A;
	Z.f.h_val = Z.f.n = Z.f.c = 0;
}

Z80_INLINE void xor_a(u8 value) {
	A ^= value;
	QT = 0;
	Z.f.s_val = Z.f.z_val = Z.f.pv_val = Z.f.yx_val = A;
	Z.f.h_val = Z.f.n = Z.f.c = 0;
}

Z80_INLINE void cp(u8 value) {
	const u16 res = A - value;
	QT = 0;
	Z.f.s_val = Z.f.z_val = (u8)res;
	Z.f.yx_val = value;
	Z.f.c = res & 0x100;
	Z.f.h_val = (A & 0x0f) - (value & 0x0f);
	Z.f.pv_val = !((A ^ value) & (A ^ res) & 0x80);
	Z.f.n = 1;
}

Z80_INLINE void exx(void) {
	swap(Z.bc, Z.bc2);
	swap(Z.de, Z.de2);
	swap(Z.hl, Z.hl2);
}

#define SHIFT_FLAGS(res, carry)                                                  \
	do {                                                                         \
		QT = 0;                                                                  \
		Z.f.s_val = Z.f.z_val = Z.f.pv_val = Z.f.yx_val = (res);                 \
		Z.f.h_val = Z.f.n = 0;                                                   \
		Z.f.c = (carry);                                                         \
	} while (0)
Z80_INLINE u8 rlc(u8 value) { const u8 res = ((value << 1) | (value >> 7)) & 0xff; SHIFT_FLAGS(res, value & 0x80); return res; }
Z80_INLINE u8 rrc(u8 value) { const u8 res = ((value >> 1) | (value << 7)) & 0xff; SHIFT_FLAGS(res, value & 0x01); return res; }
Z80_INLINE u8 rl(u8 value) { const u8 res = ((value << 1) + Z.f.c) & 0xff; SHIFT_FLAGS(res, value & 0x80); return res; }
Z80_INLINE u8 rr(u8 value) { const u8 res = ((value >> 1) | (Z.f.c << 7)) & 0xff; SHIFT_FLAGS(res, value & 0x01); return res; }
Z80_INLINE u8 sla(u8 value) { const u8 res = (value << 1) & 0xff; SHIFT_FLAGS(res, value & 0x80); return res; }
Z80_INLINE u8 sra(u8 value) { const u8 res = ((value >> 1) | (value & 0x80)) & 0xff; SHIFT_FLAGS(res, value & 0x01); return res; }
Z80_INLINE u8 sll(u8 value) { const u8 res = ((value << 1) | 0x01) & 0xff; SHIFT_FLAGS(res, value & 0x80); return res; }
Z80_INLINE u8 srl(u8 value) { const u8 res = (value >> 1) & 0xff; SHIFT_FLAGS(res, value & 0x01); return res; }

Z80_INLINE void bit(int b, u8 value) {
	QT = 0;
	Z.f.s_val = Z.f.z_val = Z.f.pv_val = value & (1 << b);
	Z.f.h_val = HF;
	Z.f.n = 0;
	Z.f.yx_val = value;
}

Z80_INLINE void bit_hl(int b, u8 value) {
	QT = 0;
	Z.f.s_val = Z.f.z_val = Z.f.pv_val = value & (1 << b);
	Z.f.h_val = HF;
	Z.f.n = 0;
	Z.f.yx_val = WZ_H;
}

Z80_INLINE void bit_xy(int b, u8 value) {
	QT = 0;
	Z.f.s_val = Z.f.z_val = Z.f.pv_val = value & (1 << b);
	Z.f.h_val = HF;
	Z.f.n = 0;
	Z.f.yx_val = Z.ea >> 8;
}

Z80_INLINE u8 res(int b, u8 value) { return value & ~(1 << b); }
Z80_INLINE u8 set(int b, u8 value) { return value | (1 << b); }

Z80_INLINE void block_io_interrupted_flags(void) {
	Z.f.yx_val = PC >> 8;
	const u8 pv_old = F_PV();
	if (Z.f.c) {
		Z.f.h_val = 0;
		if (TDAT8 & 0x80) {
			Z.f.pv_val = (B - 1) & 0x07;
			if ((B & 0x0f) == 0x00) Z.f.h_val = HF;
		} else {
			Z.f.pv_val = (B + 1) & 0x07;
			if ((B & 0x0f) == 0x0f) Z.f.h_val = HF;
		}
	} else {
		Z.f.pv_val = B & 0x07;
	}
	Z.f.pv_val = (pv_old ^ F_PV()) & PF;
}

Z80_INLINE void ei(void) {
	Z.iff1 = Z.iff2 = 1;
	SET_SA(SA_AFTER_EI, 1);
}

Z80_INLINE void illegal_1(void) {}
Z80_INLINE void illegal_2(void) {}

// ---- device_start, device_reset, execute_set_input -----------------------------------------
void z80_start(void) {
	memset(&Z, 0, sizeof Z);
	lz = -1;
	z80_clock = 0;
	last_top = ~0ull >> 1;
	last_path = -1;
	PRVPC = 0; PC = 0; SP = 0; AF = 0;
	set_f(0);
	Q = 0; QT = 0;
	BC = 0; DE = 0; HL = 0; IX = 0; IY = 0; WZ = 0;
	Z.af2.w = Z.bc2.w = Z.de2.w = Z.hl2.w = 0;
	R = R2 = 0;
	Z.iff1 = Z.iff2 = 0;
	Z.halt = 0;
	Z.im = 0;
	Z.i = 0;
	Z.nmi_state = Z.irq_state = 0;
	Z.busreq_state = Z.busack_state = 0;
	Z.ea = 0;
	Z.service_attention = 0;
	IX = IY = 0xffff;       // IX and IY are FFFF after a reset!
	Z.f.z_val = 0;          // Zero flag is set
	Z.ref = 0xffff00;
}

void z80_reset(void) {
	if (lz >= 0) z80_wakes[3]++;
	z80_wake();
	leave_halt();
	Z.ref = 0xffff00;
	PC = 0;
	WZ = PC;
	Z.i = 0;
	Z.r = 0;
	Z.r2 = 0;
	Z.iff1 = 0;
	Z.iff2 = 0;
	SET_SA(SA_NMI_PENDING, 0);
	SET_SA(SA_AFTER_EI, 0);
	SET_SA(SA_AFTER_LDAIR, 0);
}

// (while it waits, a change it won't act on leaves it waiting: the loop reads neither)
void z80_set_nmi(int state, bool ran) {
	// mark an NMI pending on the rising edge (not at the same time RESET is cleared)
	if (Z.nmi_state == CLEAR_LINE && state != CLEAR_LINE && ran) {
		if (lz >= 0) z80_wakes[0]++;
		z80_wake();
		SET_SA(SA_NMI_PENDING, 1);
	}
	Z.nmi_state = (u8)state;
}

void z80_set_irq(int state) {
	if (state != CLEAR_LINE && Z.iff1) {
		if (lz >= 0) z80_wakes[0]++;
		z80_wake();
	}
	Z.irq_state = (u8)state;
	if (Z.irq_state != CLEAR_LINE) SET_SA(SA_IRQ_ON, 1);
	else SET_SA(SA_IRQ_ON, 0);
}

// ---- execute_run ---------------------------------------------------------------------------
static Z80_HOT __attribute__((noinline)) void execute(void) {
	int ic = Z.icount;
#include "../pacman/z80_ops.h"
suspend:
	Z.icount = ic;
}

Z80_HOT int z80_run(int cycles) {
	if (lz >= 0) {
		if (z80_board_rd(0x4010) == lz_m10 && z80_board_rd(0x4018) == lz_m18) {
			const z80_loop_t *l = &loops[lz];
			int x = lz_off + cycles, k = (x - 1) / l->len;
			int end = k * l->len + l->nb[x - k * l->len];
			int ran = end - lz_off;
			int q = (end - 1) / l->len;
			lz_rounds += (uint32_t)q;
			lz_off = end - q * l->len;
			Z.cycles_running = cycles;
			Z.cycles_stolen = 0;
			Z.icount = cycles - ran;
			z80_clock += (uint64_t)ran;
			z80_skipped += (unsigned)ran;
			return ran;
		}
		z80_wakes[1]++;
		z80_wake();
	}
	Z.cycles_running = cycles;
	Z.cycles_stolen = 0;
	Z.icount = cycles;
	execute();
	int ran = cycles - Z.icount - Z.cycles_stolen;
	z80_clock += (uint64_t)ran;
	return ran;
}

void z80_abort(void) {
	int delta = Z.icount;
	Z.cycles_stolen += delta;
	Z.cycles_running -= delta;
	Z.icount -= delta;
}

int z80_done(void) { return Z.cycles_running - Z.icount; }

void z80_regs(z80_regs_t *r) {
	if (lz >= 0) z80_wakes[2]++;
	z80_wake();
	r->pc = PC; r->sp = SP;
	r->af = (u16)(A << 8 | get_f());
	r->bc = BC; r->de = DE; r->hl = HL; r->ix = IX; r->iy = IY;
	r->af2 = Z.af2.w; r->bc2 = Z.bc2.w; r->de2 = Z.de2.w; r->hl2 = Z.hl2.w;
	r->wz = WZ;
	r->r = (u8)((R & 0x7f) | (R2 & 0x80));
	r->i = Z.i; r->im = Z.im; r->iff1 = Z.iff1; r->iff2 = Z.iff2; r->halt = Z.halt;
}
