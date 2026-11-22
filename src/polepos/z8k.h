// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Ernesto Corvi
//
// z8k.h -- the Zilog Z8002 as MAME 0.288 emulates it (src/devices/cpu/z8000:
// z8000.cpp, z8000.h, z8000cpu.h, z8000ops.hxx, z8000tbl.hxx), in C, for
// Pole Position's two. z8k_ops.h is made from MAME's sources by
// tools/z8kgen.py; z8k.c is the rest of z8002_device (its memory helpers,
// the interrupts, reset, the input lines and execute_run).
//
// The board gives each CPU its memory as pages of 2K: a page in rd[] is
// read straight (16-bit words, host order); the others call the board's
// z8k_board_rd. A page in wr[] is written straight; the others call
// z8k_board_wr with MAME's mem_mask. MAME's license (BSD-3-Clause) is in
// README.md.
#ifndef Z8K_H
#define Z8K_H

#include <stdbool.h>
#include <stdint.h>

#define Z8K_NVI_LINE 0
#define Z8K_VI_LINE 1
#define Z8K_NMI_LINE 32

typedef struct z8k {
	// z8002_device's members (z8k.c's m_ names are these; regs first, at
	// the CPU's address itself, for the instructions' indexed loads)
	union { uint8_t B[16]; uint16_t W[16]; uint32_t L[8]; uint64_t Q[4]; } regs;
	uint32_t op[4];                     // (op[0]: the last instruction's that touched irq_req, for a trap)
	uint32_t ppc;
	uint32_t pc;
	uint16_t psapseg, psapoff, fcw, refresh, nspseg, nspoff;
	uint8_t irq_req;
	bool halt;                          // (beside irq_req: z8k_run reads them as one)
	uint16_t irq_vec;
	uint32_t op_valid;
	int nmi_state;
	int irq_state[2];
	int mi;
	int icount;
	// the scheduler's (device_execute_interface)
	int cycles_running, cycles_stolen;
	// the board's
	int id;
	const uint16_t *rd[32];
	uint16_t *wr[32];
	const uint16_t *rom;                // pages 0-7 as one (0: not), for the opcodes looked up at the start
	// z8k.c's
	const uint16_t *pre;                // the ROM's opcodes looked up
	const uint16_t *ops;                // the opcode words fetched with them (rom, or pre if none)
	int bank;                           // the cycles z8k_run has put aside (z8k.c)
} z8k_t;

// the board's side
uint16_t z8k_board_rd(int id, uint16_t a);              // a word at a (even)
void z8k_board_wr(int id, uint16_t a, uint16_t d, uint16_t mask);

void z8k_init(void);                    // the tables (once)
void z8k_start(z8k_t *c, int id);       // device_start (after the ROM pages are in rd)
void z8k_reset(z8k_t *c);               // device_reset
int z8k_run(z8k_t *c, int cycles);      // a timeslice (execute_run); the cycles run
void z8k_abort(void);                   // abort_timeslice, of the one running
int z8k_done(void);                     // the running one's cycles so far in its timeslice
void z8k_set_input(z8k_t *c, int line, int state);     // execute_set_input
extern void (*z8k_trace)(const z8k_t *c);               // before each instruction (tests)
uint16_t z8k_reg(const z8k_t *c, int n);                // RW(n)

#endif
