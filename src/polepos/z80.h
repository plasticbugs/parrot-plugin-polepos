// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
//
// z80.h -- the Zilog Z80 as MAME 0.288 emulates it (src/devices/cpu/z80:
// z80.cpp, z80.h, z80.inc and z80.lst), in C, for one CPU: Pac-Man's
// (../pacman/z80.c) with the NMI line added and the board's functions
// renamed. The instructions are ../pacman/z80_ops.h, made from MAME's
// z80.lst by ../pacman/tools/z80gen.py. An instruction can stop at any of MAME's steps when the
// cycles run out and carry on there in the next call, as MAME's does.
//
// The board supplies the memory and I/O below. Reads and writes in a page
// of z80_rd_page / z80_wr_page go straight to it; the rest call the
// board's z80_board_rd / z80_board_wr. MAME's license (BSD-3-Clause) is in README.md.
#ifndef Z80_H
#define Z80_H

#include <stdbool.h>
#include <stdint.h>

extern const uint8_t *z80_rd_page[256];
extern uint8_t *z80_wr_page[256];

// the board's side (pp.c)
uint8_t z80_board_rd(uint16_t a);
void z80_board_wr(uint16_t a, uint8_t d);
uint8_t z80_board_in(uint16_t port);
void z80_board_out(uint16_t port, uint8_t d);
uint8_t z80_board_irq_ack(void);           // the vector the board puts on the bus

void z80_start(void);               // device_start: power on
void z80_reset(void);               // device_reset
int z80_run(int cycles);            // a timeslice: MAME's execute_run; returns the cycles run
void z80_abort(void);               // abort_timeslice: the rest of the timeslice taken back
int z80_done(void);                 // cycles run so far in this timeslice (m_cycles_running - m_icount)
void z80_set_irq(int state);        // INPUT_LINE_IRQ0: ASSERT_LINE (1) or CLEAR_LINE (0)
void z80_set_nmi(int state, bool ran);  // INPUT_LINE_NMI; ran: total_cycles() isn't 0

typedef struct {
	uint16_t pc, sp, af, bc, de, hl, ix, iy, af2, bc2, de2, hl2, wz;
	uint8_t r, i, im, iff1, iff2, halt;
} z80_regs_t;
void z80_regs(z80_regs_t *r);
void z80_wake(void);                // run the main loop's waiting to where it has got to (z80.c)       // as MAME's state interface gives them (F from the flags, R from r and r2)
extern void (*z80_trace)(void);     // before each instruction (MAME's debugger_instruction_hook), tests only

#endif
