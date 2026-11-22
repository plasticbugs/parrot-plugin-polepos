// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
//
// mb88.h -- the Fujitsu MB8843/MB8844 as MAME 0.288 emulates them
// (src/devices/cpu/mb88xx/mb88xx.cpp), in C, for Pole Position's four
// Namco customs (51xx, 52xx, 53xx, 54xx). mb88_ops.h, the instructions, is
// made from MAME's source by tools/mb88gen.py; mb88.c is the rest of
// mb88_cpu_device: reset, the timer, the serial port, the interrupts and
// execute_run. 1K of program, 64 nibbles of RAM, 8-bit PLA mode for O.
// MAME's license (BSD-3-Clause) is in README.md.
#ifndef MB88_H
#define MB88_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MB88_IRQ_LINE 0
#define MB88_TC_LINE 1

typedef struct mb88 {
	uint8_t PC, PA;
	uint16_t SP[4];
	uint8_t SI, A, X, Y, st, zf, cf, vf, sf, iff;
	uint8_t pio, TH, TL, TP, ctr, SB;
	uint16_t SBcount;
	uint8_t o_output, pending_irq;
	bool in_irq;
	uint8_t ram[64];                    // (PC to here: the state a periodic wait compares, MB88_PZ_LEN)
	int icount;
	int cycles_running, cycles_stolen;
	int id;                             // the board's number for it
	const uint8_t *rom;                 // 1K
	uint8_t idle[0x400];                // the waiting loops' tops (mb88_find_idle)
	uint8_t lz;                         // the loop it waits in without running (0: running; mb88.c)
	uint8_t lz_off;                     // how far past the loop's top it is (1..len)
	uint32_t lz_rounds;                 // and the whole rounds since the real state (at the top)
	int lz_ovf;                         // the cycles from the real state to the timer's overflow (prescaler on)
	uint8_t ser_on;                     // its serial port runs in mb88_run, a tick a cycle (no timer: the
	                                    // board's mb88_serial_adjust sets it, outside PP_EXACT)
	// waiting in a loop that its timer interrupts, the same each time (mb88.c)
	uint8_t *pz_snap;                   // the board's MB88_PZ_LEN bytes for it (NULL: it never waits so)
	uint8_t pz, pw;                     // a period being checked; waiting (the state of a period's start)
	uint8_t irqs, pz_irqs;              // the interrupts it has taken (wraps), as at the last check
	uint8_t outr[4], outp;              // the last R and P written
	uint16_t pz_pc, pw_len, pw_off;     // the loop's top; the period; how far into it
	uint32_t cyc, pz_cyc;               // the cycles it has run before this mb88_run; at the period's start
	uint32_t outs, pz_outs;             // the writes that changed a port; at the period's start
	uint32_t pz_ends[4];                // the instructions' ends in the period (bit k: k + 1 cycles in)
	// the board's: stop_rti 1, its RAM into irq_snap when it takes an interrupt (then 2) and
	// mb88_run ends at the RTI (then 3)
	uint8_t stop_rti;
	uint8_t *irq_snap;
} mb88_t;
#define MB88_PZ_LEN (offsetof(mb88_t, ram) + 64 - offsetof(mb88_t, PC))

// the board's side
uint8_t mb88_read_k(int id);
uint8_t mb88_read_r(int id, int n);
void mb88_write_r(int id, int n, uint8_t d);
void mb88_write_p(int id, uint8_t d);
void mb88_write_o(int id, uint8_t d, uint8_t mask);
int mb88_read_si(int id);
void mb88_serial_adjust(int id, bool on);  // its serial timer: every 6 clocks from now, or off

void mb88_reset(mb88_t *m);             // device_reset
void mb88_find_idle(mb88_t *m);         // the program's waiting loops (once, with rom set)
int mb88_run(mb88_t *m, int cycles);    // execute_run; the cycles run
void mb88_abort(void);                  // abort_timeslice, of the one running
int mb88_done(void);                    // the running one's cycles so far
void mb88_set_input(mb88_t *m, int line, int state);    // execute_set_input
void mb88_wake(mb88_t *m);              // the real state to where its waiting has got to (before reading it)
void mb88_serial_timer(mb88_t *m);      // the serial timer's callback
extern void (*mb88_trace)(const mb88_t *m);
extern unsigned mb88_skipped[4];        // the cycles each has not run (waiting, standing still)

#endif
