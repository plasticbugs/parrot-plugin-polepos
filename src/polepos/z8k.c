// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Ernesto Corvi
//
// z8k.c -- see z8k.h. MAME 0.288's z8002_device (z8000.cpp) in C: the
// operand fetches, the memory and port helpers, Interrupt(), the reset,
// execute_set_input and execute_run. The instructions are z8k_ops.h
// (tools/z8kgen.py). MAME's license is in README.md.
//
// For speed a timeslice is one function, z8k_run: the instructions Pole
// Position runs (z8kgen.py's HOT) and their helpers are inlined into it,
// each ending by fetching the next and jumping to its code (GCC's computed
// goto, through z8k_pre: each ROM word's instruction, looked up at the
// start). The other instructions are functions of their own. The state the
// instructions use most lives in registers: on the M33 (GCC) seven core
// registers and s16 are given to it for this file (Z8K_GREGS; GCC would
// spill some of it otherwise), elsewhere it is a local struct. The rest (the
// registers, the interrupt state, the page tables) stays in the z8k_t. The
// state goes back to the z8k_t when the timeslice ends, and when the board
// is called (its handlers may call z8k_done, z8k_abort, z8k_set_input).
#include "z8k.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#if defined(__GNUC__) && !defined(__clang__) && defined(__thumb2__) && !defined(Z8K_NO_GREGS)
#define Z8K_GREGS 1
// pc, icount (less Z8K_BIAS), fcw, the opcode word, c->pre, the CPU and
// c->ops, and Z8K_BASE in s16. All callee-saved: the board's functions keep
// them, and z8k_run keeps its caller's. (Not r7: GCC 9 uses it anyway.)
register uint32_t z8k_pc asm("r4");
register int z8k_icount asm("r5");
register uint32_t z8k_fcw asm("r6");
register uint32_t z8k_op0 asm("r8");
register const uint16_t *z8k_prep asm("r9");      // c->pre
register z8k_t *z8k_c asm("r10");
register const uint16_t *z8k_ops asm("r11");      // c->ops
#ifdef __ARM_FP
#define Z8K_FBASE 1
register float z8k_basef asm("s16");              // Z8K_BASE
#endif
#endif

#if defined(PICO_ON_DEVICE) && PICO_ON_DEVICE
#define Z8K_HOT __attribute__((section(".time_critical.z8k")))
#else
#define Z8K_HOT
#endif
#define Z8K_AI inline __attribute__((always_inline))
#define Z8K_COLD __attribute__((noinline, cold))

typedef struct {
	uint16_t beg, end, step, op;        // MAME's Z8000_init: the opcodes beg..end by step run instruction op
} z8k_init_t;

// the instructions' state, z8k_run's (S)
typedef struct {
#ifndef Z8K_GREGS
	z8k_t *c;
	uint32_t pc;
	int icount;                         // the rest of the timeslice less c->bank, less Z8K_BIAS
	uint32_t fcw;                       // (16 bits: as wide as a register, for the compiler)
	uint32_t op0;
#endif
	uint32_t op[4];                     // (1-3: the operand words of the instruction running)
	uint32_t op_valid;
} z8k_st_t;

#ifdef Z8K_GREGS
#define Z8K_C      z8k_c
#define Z8K_PC     z8k_pc
#define Z8K_ICOUNT z8k_icount
#define Z8K_FCW    z8k_fcw
#define Z8K_OPW    z8k_op0
#else
#define Z8K_C      S->c
#define Z8K_PC     S->pc
#define Z8K_ICOUNT S->icount
#define Z8K_FCW    S->fcw
#define Z8K_OPW    S->op0
#endif

static z8k_t *Zp;                       // the CPU running
static bool running;
void (*z8k_trace)(const z8k_t *c);

// The fast path (z8k_run) takes the next instruction when icount > 0. To
// make the next boundary look at the interrupts, the cycles left go to
// c->bank and icount to -Z8K_BANK, far enough down that no instruction can
// bring it back above 0; the boundary puts them back. Z8K_ICOUNT is icount
// less Z8K_BIAS (the time is up when it is negative).
#define Z8K_BANK 0x40000000
#define Z8K_BIAS 1
#define z8k_look() (Z8K_C->bank += Z8K_ICOUNT + Z8K_BANK, Z8K_ICOUNT = -Z8K_BANK)

// touching m_irq_req makes the next boundary look at it. (m_halt needs no
// look: HALT also ends the timeslice, after it compares icount, which then is
// the whole count, nothing having banked it in the instruction.) It keeps the
// opcode word in c->op[0] too: a trap's Interrupt pushes it, and the fetch
// that finds the time is up has the next one in Z8K_OPW (z8k_run).
static Z8K_AI uint8_t *z8k_irq_req_(z8k_st_t *S) {
	(void)S;
	z8k_look();
	Z8K_C->op[0] = Z8K_OPW;
	return &Z8K_C->irq_req;
}

#define m_op       S->op
#define m_op0      Z8K_OPW
#define m_ppc      Z8K_C->ppc
#define m_pc       Z8K_PC
#define m_psapseg  Z8K_C->psapseg
#define m_psapoff  Z8K_C->psapoff
#define m_fcw      Z8K_FCW
#define m_refresh  Z8K_C->refresh
#define m_nspseg   Z8K_C->nspseg
#define m_nspoff   Z8K_C->nspoff
#define m_irq_req  (*z8k_irq_req_(S))
#define m_irq_vec  Z8K_C->irq_vec
#define m_op_valid S->op_valid
#define m_regs     Z8K_C->regs
#define m_nmi_state Z8K_C->nmi_state
#define m_irq_state Z8K_C->irq_state
#define m_mi       Z8K_C->mi
#define m_halt     Z8K_C->halt
#define m_icount   Z8K_ICOUNT       // (only added to and taken from; and m_icount_end)
#define m_icount_end() (Z8K_ICOUNT >= 0 ? (void)(Z8K_ICOUNT = -Z8K_BIAS) : (void)0)      // if (m_icount > 0) m_icount = 0

// z8000.h's constants
#define Z8000_EPU     0x80
#define Z8000_TRAP    0x40
#define Z8000_NMI     0x20
#define Z8000_SEGTRAP 0x10
#define Z8000_NVI     0x08
#define Z8000_VI      0x04
#define Z8000_SYSCALL 0x02
#define Z8000_RESET   0x01
#define NVI_LINE Z8K_NVI_LINE
#define VI_LINE  Z8K_VI_LINE
#define NMI_LINE Z8K_NMI_LINE
#define CLEAR_LINE 0
#define ASSERT_LINE 1
#define m_vector_mult 1

// the spaces: polepos's Z8002s have program and I/O only, so data and
// stack are the program space (init_spaces)
enum { m_program, m_data = m_program, m_stack = m_program, m_io = 1, m_sio = 1 };

// little-endian hosts (util::BYTE*_XOR_BE)
#define BYTE_XOR_BE(a)  ((a) ^ 1)
#define BYTE4_XOR_BE(a) ((a) ^ 3)
#define BYTE8_XOR_BE(a) ((a) ^ 7)
#define BIT(x, n) (((x) >> (n)) & 1)
#define Z8K_NOLOG(...) ((void)0)

static uint8_t z8000_zsp[256];

// The dispatch: MAME's z8000_exec[] gives each opcode its entry in
// z8k_table. Here the ROM's opcodes (0x0000-0x3fff, which nothing writes)
// are looked up once at the start: z8k_pre has each word's instruction as
// the offset of its code in z8k_run (from z8k_run's start, in halfwords).
// An opcode anywhere else is looked up when it runs (far:, Z8K_FAR).
#define Z8K_NENT ((int)(sizeof z8k_table / sizeof z8k_table[0]) - 1)
#define Z8K_ROMW 0x2000
static uint16_t z8k_pre[2][1 + Z8K_ROMW];       // ([-1]: ended, the time being up)
#define Z8K_BASE ((uintptr_t)z8k_run & ~(uintptr_t)1)

// pc's word in the ROM: min(pc, 0x3fff) / 2 (for pc under 2^31, as it is:
// the instructions set 16 bits, and only fetching runs past 0xffff)
#if defined(__GNUC__) && !defined(__clang__) && defined(__thumb2__)
#define Z8K_WORD(x) ({ uint32_t r_; __asm__("usat %0, #13, %1, asr #1" : "=r"(r_) : "r"(x)); r_; })
#else
#define Z8K_WORD(x) (((x) < 0x3fffu ? (x) : 0x3fffu) >> 1)
#endif

// Z8K_BASE where z8k_run has it
#ifdef Z8K_FBASE
#define Z8K_BASE_SET() __asm__ volatile("vmov %0, %1" : "=t"(z8k_basef) : "r"(Z8K_BASE))
#define Z8K_BASEV ({ uintptr_t b_; __asm__ volatile("vmov %0, %1" : "=r"(b_) : "t"(z8k_basef)); b_; })
#else
#define Z8K_BASE_SET() const uintptr_t base = Z8K_BASE
#define Z8K_BASEV base
#endif

#define get_segmented_mode() 0

// ---- memory (z8000.cpp) --------------------------------------------------------------------
// The board's handlers may look at the time (z8k_done), end the timeslice
// (z8k_abort) or change the interrupt lines (z8k_set_input, which reads
// fcw): pc, fcw and the time left go to the z8k_t for them, and the time
// left comes back banked (z8k_look), so the next boundary looks at the
// interrupts. (Out of line: they are many and seldom run.)
#ifdef Z8K_GREGS
static Z8K_COLD uint16_t z8k_board_rd_(uint16_t a) {
	z8k_t *c = z8k_c;
	c->pc = z8k_pc;
	c->fcw = (uint16_t)z8k_fcw;
	c->icount = z8k_icount + Z8K_BIAS + c->bank;
	c->bank = 0;
	uint16_t d = z8k_board_rd(c->id, a);
	c->bank = c->icount - Z8K_BIAS + Z8K_BANK;
	z8k_icount = -Z8K_BANK;
	return d;
}

static Z8K_COLD void z8k_board_wr_(uint16_t a, uint16_t d, uint16_t mask) {
	z8k_t *c = z8k_c;
	c->pc = z8k_pc;
	c->fcw = (uint16_t)z8k_fcw;
	c->icount = z8k_icount + Z8K_BIAS + c->bank;
	c->bank = 0;
	z8k_board_wr(c->id, a, d, mask);
	c->bank = c->icount - Z8K_BIAS + Z8K_BANK;
	z8k_icount = -Z8K_BANK;
}
#else
static Z8K_COLD uint16_t z8k_board_rd_(z8k_t *c, uint16_t a) {
	c->icount += c->bank;
	c->bank = 0;
	uint16_t d = z8k_board_rd(c->id, a);
	c->bank = c->icount - Z8K_BIAS + Z8K_BANK;
	return d;
}

static Z8K_COLD void z8k_board_wr_(z8k_t *c, uint16_t a, uint16_t d, uint16_t mask) {
	c->icount += c->bank;
	c->bank = 0;
	z8k_board_wr(c->id, a, d, mask);
	c->bank = c->icount - Z8K_BIAS + Z8K_BANK;
}

#define z8k_board_out() (Z8K_C->pc = Z8K_PC, Z8K_C->fcw = (uint16_t)Z8K_FCW, Z8K_C->icount = Z8K_ICOUNT + Z8K_BIAS)
#endif

static Z8K_AI uint16_t mem_rw_(z8k_st_t *S, uint16_t a) {
	(void)S;
	const uint16_t *p = Z8K_C->rd[a >> 11];
	if (__builtin_expect(p != 0, 1)) return p[(a & 0x7ff) >> 1];
#ifdef Z8K_GREGS
	return z8k_board_rd_(a);
#else
	z8k_board_out();
	uint16_t d = z8k_board_rd_(Z8K_C, a);
	Z8K_ICOUNT = -Z8K_BANK;
	return d;
#endif
}
#define mem_rw(a) mem_rw_(S, a)

static Z8K_AI void mem_ww_(z8k_st_t *S, uint16_t a, uint16_t d, uint16_t mask) {
	(void)S;
	uint16_t *p = Z8K_C->wr[a >> 11];
	if (__builtin_expect(p != 0, 1)) {
		uint16_t *w = &p[(a & 0x7ff) >> 1];
		*w = (uint16_t)((*w & ~mask) | (d & mask));
		return;
	}
#ifdef Z8K_GREGS
	z8k_board_wr_(a, d, mask);
#else
	z8k_board_out();
	z8k_board_wr_(Z8K_C, a, d, mask);
	Z8K_ICOUNT = -Z8K_BANK;
#endif
}
#define mem_ww(a, d, mask) mem_ww_(S, a, d, mask)

static inline uint32_t addr_add(uint32_t addr, uint32_t addend) {
	return (addr & 0xffff0000) | ((addr + addend) & 0xffff);
}

static inline uint32_t addr_sub(uint32_t addr, uint32_t subtrahend) {
	return (addr & 0xffff0000) | ((addr - subtrahend) & 0xffff);
}

static Z8K_AI uint16_t RDOP_(z8k_st_t *S) {
	uint16_t res = mem_rw((uint16_t)(m_pc & 0xfffe));
	m_pc += 2;
	return res;
}
#define RDOP() RDOP_(S)

static Z8K_AI uint32_t get_operand_(z8k_st_t *S, int opnum) {
	if (opnum == 0) return m_op0;       // (the dispatch's)
	if (!(m_op_valid & (1u << opnum))) {
		m_op[opnum] = mem_rw((uint16_t)(m_pc & 0xfffe));
		m_pc += 2;
		m_op_valid |= 1u << opnum;
	}
	return m_op[opnum];
}
#define get_operand(n) get_operand_(S, n)

// (non-segmented: the address is the word)
#define get_addr_operand(n) get_operand(n)
#define get_raw_addr_operand(n) get_operand(n)

// A byte in a page is read and written as the byte (the word's other half
// is left as it is, as the mask does); the board gets MAME's word and mask.
static Z8K_AI uint8_t RDMEM_B_(z8k_st_t *S, int space, uint32_t addr) {
	(void)space;
	uint16_t a = (uint16_t)addr;
	const uint16_t *p = Z8K_C->rd[a >> 11];
	if (__builtin_expect(p != 0, 1)) return ((const uint8_t *)p)[(a & 0x7ff) ^ 1];
	uint16_t w = mem_rw((uint16_t)(addr & 0xfffe));
	return (uint8_t)((addr & 1) ? w : w >> 8);
}
#define RDMEM_B(...) RDMEM_B_(S, __VA_ARGS__)

static Z8K_AI uint16_t RDMEM_W_(z8k_st_t *S, int space, uint32_t addr) {
	(void)space;
	return mem_rw((uint16_t)(addr & 0xfffe));
}
#define RDMEM_W(...) RDMEM_W_(S, __VA_ARGS__)

static Z8K_AI uint32_t RDMEM_L_(z8k_st_t *S, int space, uint32_t addr) {
	(void)space;
	addr &= ~1u;
	uint32_t result = (uint32_t)mem_rw((uint16_t)addr) << 16;
	return result + mem_rw((uint16_t)addr_add(addr, 2));
}
#define RDMEM_L(...) RDMEM_L_(S, __VA_ARGS__)

static Z8K_AI void WRMEM_B_(z8k_st_t *S, int space, uint32_t addr, uint8_t value) {
	(void)space;
	uint16_t a = (uint16_t)addr;
	uint16_t *p = Z8K_C->wr[a >> 11];
	if (__builtin_expect(p != 0, 1)) {
		((uint8_t *)p)[(a & 0x7ff) ^ 1] = value;
		return;
	}
	uint16_t value16 = (uint16_t)(value | (value << 8));
	mem_ww((uint16_t)(addr & 0xfffe), value16, BIT(addr, 0) ? 0x00ff : 0xff00);
}
#define WRMEM_B(...) WRMEM_B_(S, __VA_ARGS__)

static Z8K_AI void WRMEM_W_(z8k_st_t *S, int space, uint32_t addr, uint16_t value) {
	(void)space;
	mem_ww((uint16_t)(addr & 0xfffe), value, 0xffff);
}
#define WRMEM_W(...) WRMEM_W_(S, __VA_ARGS__)

static Z8K_AI void WRMEM_L_(z8k_st_t *S, int space, uint32_t addr, uint32_t value) {
	(void)space;
	addr &= ~1u;
	mem_ww((uint16_t)addr, (uint16_t)(value >> 16), 0xffff);
	mem_ww((uint16_t)addr_add(addr, 2), (uint16_t)value, 0xffff);
}
#define WRMEM_L(...) WRMEM_L_(S, __VA_ARGS__)

// polepos maps nothing in the Z8002s' I/O space: reads give MAME's unmapped
// value (0) and writes go nowhere
#define RDPORT_B(mode, addr) ((void)(mode), (void)(addr), (uint8_t)0)
#define RDPORT_W(mode, addr) ((void)(mode), (void)(addr), (uint16_t)0)
#define WRPORT_B(mode, addr, value) ((void)(mode), (void)(addr), (void)(value))
#define WRPORT_W(mode, addr, value) ((void)(mode), (void)(addr), (void)(value))

#define cycles(c) (m_icount -= (c))

// x == 0 (x up to 32 bits) as 1 or 0, without a branch (GCC would make one)
#if defined(__GNUC__) && !defined(__clang__) && defined(__thumb2__)
#define Z8K_ZERO(x) ({ uint32_t z_; __asm__("clz %0, %1" : "=r"(z_) : "r"((uint32_t)(x))); z_ >> 5; })
#else
#define Z8K_ZERO(x) ((uint32_t)((uint32_t)(x) == 0))
#endif
#define PSA_ADDR() ((uint32_t)m_psapoff)
#define F_SEG_Z8001() 0

#include "z8k_ops.h"

static uint16_t z8k_off[Z8K_NOPS + 2];  // each instruction's (and far:'s, ended:'s) offset, as in z8k_pre
static const void *const *z8k_labels;   // z8k_run's code: each instruction's, far:'s, ended:'s
#define Z8K_FAR Z8K_NOPS
#define Z8K_ENDED (Z8K_NOPS + 1)

#define PUSH_PC() PUSHW(SP, (uint16_t)m_pc)
#define GET_PC(VEC) ((uint32_t)RDMEM_W(m_program, (VEC) + 2))
#define get_reset_pc() ((uint32_t)RDMEM_W(m_program, 4))
#define GET_FCW(VEC) RDMEM_W(m_program, (VEC))
#define read_irq_vector() ((uint32_t)RDMEM_W(m_program, VEC00 + 2 * (m_irq_vec & 0xff)))

// an interrupt acknowledge reads MAME's unbound m_iack_in: 0xffff
#define IACK 0xffff

// irq_req or halt (one load: they are side by side)
_Static_assert(offsetof(z8k_t, halt) == offsetof(z8k_t, irq_req) + 1 && sizeof(bool) == 1, "z8k_t's irq_req, halt");
static Z8K_AI uint16_t z8k_stops(const z8k_t *c) {
	uint16_t w;
	memcpy(&w, &c->irq_req, 2);
	return w;
}

static Z8K_AI void Interrupt(z8k_st_t *S) {
	uint16_t fcw = m_fcw;
	const uint16_t op0 = (uint16_t)Z8K_C->op[0];      // (the trap's instruction, z8k_irq_req_)

	if (m_irq_req & Z8000_RESET) {
		m_irq_req &= Z8000_NVI | Z8000_VI;
		CHANGE_FCW(RDMEM_W(m_program, 2));
		m_pc = get_reset_pc();
	} else if (m_irq_req & Z8000_EPU) {
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());
		PUSH_PC();
		PUSHW(SP, fcw);
		PUSHW(SP, op0);
		m_irq_req &= ~Z8000_EPU;
		CHANGE_FCW(GET_FCW(EPU));
		m_pc = GET_PC(EPU);
	} else if (m_irq_req & Z8000_TRAP) {
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());
		PUSH_PC();
		PUSHW(SP, fcw);
		PUSHW(SP, op0);
		m_irq_req &= ~Z8000_TRAP;
		CHANGE_FCW(GET_FCW(TRAP));
		m_pc = GET_PC(TRAP);
	} else if (m_irq_req & Z8000_SYSCALL) {
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());
		PUSH_PC();
		PUSHW(SP, fcw);
		PUSHW(SP, op0);
		m_irq_req &= ~Z8000_SYSCALL;
		CHANGE_FCW(GET_FCW(SYSCALL));
		m_pc = GET_PC(SYSCALL);
	} else if (m_irq_req & Z8000_SEGTRAP) {
		m_irq_vec = IACK;
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());
		PUSH_PC();
		PUSHW(SP, fcw);
		PUSHW(SP, m_irq_vec);
		m_irq_req &= ~Z8000_SEGTRAP;
		CHANGE_FCW(GET_FCW(SEGTRAP));
		m_pc = GET_PC(SEGTRAP);
	} else if (m_irq_req & Z8000_NMI) {
		m_irq_vec = IACK;
		m_halt = false;
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());
		PUSH_PC();
		PUSHW(SP, fcw);
		PUSHW(SP, m_irq_vec);
		m_pc = RDMEM_W(m_program, NMI);
		m_irq_req &= ~Z8000_NMI;
		CHANGE_FCW(GET_FCW(NMI));
		m_pc = GET_PC(NMI);
	} else if ((m_irq_req & Z8000_NVI) && (m_fcw & F_NVIE)) {
		m_irq_vec = IACK;
		m_halt = false;
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());
		PUSH_PC();
		PUSHW(SP, fcw);
		PUSHW(SP, m_irq_vec);
		m_pc = GET_PC(NVI);
		m_irq_req &= ~Z8000_NVI;
		CHANGE_FCW(GET_FCW(NVI));
	} else if ((m_irq_req & Z8000_VI) && (m_fcw & F_VIE)) {
		m_irq_vec = IACK;
		m_halt = false;
		CHANGE_FCW(fcw | F_S_N | F_SEG_Z8001());
		PUSH_PC();
		PUSHW(SP, fcw);
		PUSHW(SP, m_irq_vec);
		m_pc = read_irq_vector();
		m_irq_req &= ~Z8000_VI;
		CHANGE_FCW(GET_FCW(VI));
	}
}

// The instructions Pole Position doesn't run, and Interrupt, out of line
// (with the state in registers they need no more)
#ifdef Z8K_GREGS
static Z8K_COLD void z8k_interrupt(void) {
	z8k_st_t st;
	Interrupt(&st);
}

#define Z8K_OUT_H(num, fn, cy)
#define Z8K_OUT_C(num, fn, cy) \
	static Z8K_COLD void fn##_out(void) { \
		z8k_st_t st; \
		st.op_valid = 1; \
		fn(&st); \
	}
#define Z8K_OUT(num, fn, cy, k) Z8K_OUT_##k(num, fn, cy)
Z8K_OP0(Z8K_OUT)
Z8K_OPS(Z8K_OUT)
#undef Z8K_OUT
#undef Z8K_OUT_C
#undef Z8K_OUT_H
#endif

static void clear_internal_state(z8k_t *c) {
	c->op[0] = c->op[1] = c->op[2] = c->op[3] = 0;
	c->ppc = 0;
	c->pc = 0;
	c->psapseg = 0;
	c->psapoff = 0;
	c->fcw = 0;
	c->refresh = 0;
	c->nspseg = 0;
	c->nspoff = 0;
	c->irq_req = 0;
	c->irq_vec = 0;
	c->op_valid = 0;
	c->regs.Q[0] = c->regs.Q[1] = c->regs.Q[2] = c->regs.Q[3] = 0;
	c->nmi_state = 0;
	c->irq_state[0] = c->irq_state[1] = 0;
}

void z8k_init(void) {
	for (int i = 0; i < 256; i++)
		z8000_zsp[i] = (uint8_t)(((i == 0) ? F_Z : 0) |
		                         ((i & 128) ? F_S : 0) |
		                         ((((i >> 7) ^ (i >> 6) ^ (i >> 5) ^ (i >> 4) ^ (i >> 3) ^ (i >> 2) ^ (i >> 1) ^ i) & 1) ? 0 : F_PV));
	z8k_t labeler;                      // (z8k_run gives its code's addresses for this one)
	memset(&labeler, 0, sizeof labeler);
	labeler.id = -1;
	labeler.halt = true;
	z8k_run(&labeler, 0);
	Zp = 0;
	for (int i = 0; i < Z8K_NOPS + 2; i++) {
		uintptr_t o = (uintptr_t)z8k_labels[i] - Z8K_BASE;
		if (o >= 0x20000) __builtin_trap();     // (z8k_run is to be under 128K)
		z8k_off[i] = (uint16_t)(o / 2);
	}
}

// z8000_exec[op]: the last entry with op (z8000_init_tables fills it in order)
static Z8K_COLD uint16_t decode(uint16_t op) {
	for (int i = Z8K_NENT - 1; i > 0; i--) {
		const z8k_init_t *e = &z8k_table[i];
		if (op >= e->beg && op <= e->end && (op - e->beg) % e->step == 0) return e->op;
	}
	return z8k_table[0].op;
}

static void predecode(z8k_t *c) {
	uint16_t *pre = &z8k_pre[c->id][1];
	c->pre = pre;
	pre[-1] = z8k_off[Z8K_ENDED];
	c->rom = c->rd[0];
	for (int p = 0; p < 8; p++)
		if (!c->rd[p] || c->rd[p] != c->rd[0] + p * 0x400) c->rom = 0;
	for (int w = 0; w < Z8K_ROMW; w++) pre[w] = z8k_off[c->rom ? decode(c->rom[w]) : Z8K_FAR];
	pre[Z8K_ROMW - 1] = z8k_off[Z8K_FAR];       // (z8k_run's pc past the ROM, and the last word)
	c->ops = c->rom ? c->rom : pre;             // (no ROM: pre is all far)
}

void z8k_start(z8k_t *c, int id) {
	const uint16_t *rd[32];
	uint16_t *wr[32];
	memcpy(rd, c->rd, sizeof rd);
	memcpy(wr, c->wr, sizeof wr);
	memset(c, 0, sizeof *c);
	memcpy(c->rd, rd, sizeof rd);
	memcpy(c->wr, wr, sizeof wr);
	c->id = id;
	clear_internal_state(c);
	c->mi = CLEAR_LINE;
	predecode(c);
}

void z8k_reset(z8k_t *c) {
	c->irq_req |= Z8000_RESET;
	c->refresh &= 0x7fff;
	c->halt = false;
}

// execute_run. An instruction boundary where MAME's loop would find no
// interrupt request, no halt and time left is the fast path (icount > 0);
// anything that may change that (an instruction touching m_irq_req, the
// board's handlers) banks icount (z8k_look), and the next boundary goes
// through look:, which is MAME's. For a CPU with id -1 (z8k_init's) it
// gives z8k_labels instead.
int Z8K_HOT z8k_run(z8k_t *c, int cyc) {
	z8k_st_t st;
	z8k_st_t *const S = &st;
#ifdef Z8K_GREGS
	const uint32_t keep_pc = z8k_pc, keep_fcw = z8k_fcw, keep_op0 = z8k_op0;
	const int keep_icount = z8k_icount;
	z8k_t *const keep_c = z8k_c;
	const uint16_t *const keep_ops = z8k_ops, *const keep_pre = z8k_prep;
#ifdef Z8K_FBASE
	const float keep_base = z8k_basef;
#endif
#endif
	Zp = c;
	running = true;
	c->cycles_running = cyc;
	c->cycles_stolen = 0;
	c->bank = 0;
	Z8K_C = c;
	Z8K_PC = c->pc;
	Z8K_FCW = c->fcw;
	Z8K_ICOUNT = cyc - Z8K_BIAS;
	st.op_valid = 0;
#ifdef Z8K_GREGS
#define Z8K_PRE z8k_prep
#define Z8K_ROM z8k_ops
#else
	const uint16_t *pre, *ops;
#define Z8K_PRE pre
#define Z8K_ROM ops
#endif
	Z8K_PRE = c->pre;
	Z8K_ROM = c->ops;
	Z8K_BASE_SET();
	uint32_t a;
	// The next instruction: its opcode word from the ROM (ops) and its code
	// from pre, or (far) looked up; pc past the ROM is the ROM's last word,
	// which is far. Z8K_NEXT, after an instruction, goes to ended instead
	// when the time is up (Z8K_ICOUNT < 0 makes the word -1: pre[-1]), which
	// takes the fetch back.
#define Z8K_FETCH() do { \
		a = Z8K_WORD(Z8K_PC); \
		Z8K_OPW = Z8K_ROM[a]; \
		Z8K_PC += 2; \
		st.op_valid = 1; \
		goto *(void *)(Z8K_BASEV + 2u * Z8K_PRE[a]); \
	} while (0)
#define Z8K_NEXT() do { \
		a = Z8K_WORD(Z8K_PC); \
		Z8K_OPW = Z8K_ROM[a]; \
		Z8K_PC += 2; \
		st.op_valid = 1; \
		goto *(void *)(Z8K_BASEV + 2u * Z8K_PRE[(int32_t)(a | (uint32_t)(Z8K_ICOUNT >> 31))]); \
	} while (0)
	goto look;                          // (MAME's do ... while: the first runs whatever icount is)

#ifdef Z8K_GREGS
next:                                   // (the instructions out of line)
	Z8K_NEXT();
#endif
ended:
	Z8K_PC -= 2;
	Z8K_ICOUNT += Z8K_C->bank;
	Z8K_C->bank = 0;
	if (Z8K_ICOUNT < 0) goto out;
look:
	if (__builtin_expect(!z8k_stops(Z8K_C), 1)
#ifndef PP_NO_TRACE
	    && !z8k_trace
#endif
	   ) Z8K_FETCH();                   // (nothing to look at)
	if (Z8K_C->id < 0) {
#define Z8K_LABEL(num, fn, cy, k) &&op_##num,
		static const void *const labels[Z8K_NOPS + 2] = {
			Z8K_OP0(Z8K_LABEL)
			Z8K_OPS(Z8K_LABEL)
			&&far, &&ended
		};
#undef Z8K_LABEL
		z8k_labels = labels;
		goto out;
	}
	if (Z8K_C->irq_req) {
#ifdef Z8K_GREGS
		z8k_interrupt();
#else
		Interrupt(S);
#endif
		Z8K_ICOUNT += Z8K_C->bank;      // (touching m_irq_req banked it)
		Z8K_C->bank = 0;
	}
#ifndef PP_NO_TRACE
	Z8K_C->ppc = Z8K_PC;                // (only MAME's debugger reads it)
	if (z8k_trace) {
		Z8K_C->pc = Z8K_PC;
		Z8K_C->fcw = (uint16_t)Z8K_FCW;
		Z8K_C->icount = Z8K_ICOUNT + Z8K_BIAS;
		z8k_trace(Z8K_C);
	}
#endif
	if (Z8K_C->halt) {
		Z8K_ICOUNT = -Z8K_BIAS;
		goto out;
	}
	if (Z8K_C->irq_req
#ifndef PP_NO_TRACE
	    || z8k_trace
#endif
	   ) z8k_look();                    // (the next boundary looks again)
	Z8K_FETCH();

far:
	Z8K_PC -= 2;
	Z8K_OPW = mem_rw((uint16_t)(Z8K_PC & 0xfffe));
	Z8K_PC += 2;
	goto *(void *)(Z8K_BASEV + 2u * z8k_off[decode((uint16_t)Z8K_OPW)]);

	// each instruction, then the next (MAME's while (m_icount > 0))
#define Z8K_CASE_H(num, fn, cy) \
op_##num: \
	Z8K_ICOUNT -= cy; \
	fn(S); \
	Z8K_NEXT();
#ifdef Z8K_GREGS
#define Z8K_CASE_C(num, fn, cy) \
op_##num: \
	Z8K_ICOUNT -= cy; \
	fn##_out(); \
	goto next;
#else
#define Z8K_CASE_C Z8K_CASE_H
#endif
#define Z8K_CASE(num, fn, cy, k) Z8K_CASE_##k(num, fn, cy)
	Z8K_OP0(Z8K_CASE)
	Z8K_OPS(Z8K_CASE)
#undef Z8K_CASE

out:
	running = false;
	Z8K_C->pc = Z8K_PC;
	Z8K_C->fcw = (uint16_t)Z8K_FCW;
	Z8K_C->op_valid = 0;
	Z8K_C->icount = Z8K_ICOUNT + Z8K_BIAS;
	{
		int ran = Z8K_C->cycles_running - Z8K_C->icount;   // (= cyc - icount - cycles_stolen)
#ifdef Z8K_GREGS
		z8k_pc = keep_pc;
		z8k_fcw = keep_fcw;
		z8k_op0 = keep_op0;
		z8k_icount = keep_icount;
		z8k_c = keep_c;
		z8k_ops = keep_ops;
		z8k_prep = keep_pre;
#ifdef Z8K_FBASE
		z8k_basef = keep_base;
#endif
#endif
		return ran;
	}
}

void z8k_abort(void) {
	if (!running) return;
	if (Zp->icount > 0) {
		Zp->cycles_stolen += Zp->icount;
		Zp->cycles_running -= Zp->icount;
		Zp->icount = 0;
	}
}

int z8k_done(void) { return Zp->cycles_running - Zp->icount; }

void z8k_set_input(z8k_t *c, int irqline, int state) {
	if (irqline == NMI_LINE) {
		if (c->nmi_state == state) return;
		c->nmi_state = state;
		if (state != CLEAR_LINE) c->irq_req |= Z8000_NMI;
	} else if (irqline < 2) {
		c->irq_state[irqline] = state;
		if (irqline == NVI_LINE) {
			if (state == CLEAR_LINE) {
				if (!(c->fcw & F_NVIE)) c->irq_req &= ~Z8000_NVI;
			} else {
				if (c->fcw & F_NVIE) c->irq_req |= Z8000_NVI;
			}
		} else {
			if (state == CLEAR_LINE) {
				if (!(c->fcw & F_VIE)) c->irq_req &= ~Z8000_VI;
			} else {
				if (c->fcw & F_VIE) c->irq_req |= Z8000_VI;
			}
		}
	}
}

uint16_t z8k_reg(const z8k_t *c, int n) { return c->regs.W[BYTE4_XOR_BE(n)]; }
