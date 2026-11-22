// pp.h -- the Namco Pole Position board as MAME 0.288 emulates it
// (src/mame/namco/polepos.cpp and the devices it uses), for the cart: a
// Z80, two Z8002s and the four Namco customs (51xx, 52xx, 53xx, 54xx, each
// an MB88xx MCU) with the 06xx between them and the Z80, in MAME's
// timeslices, so every CPU sees the others' writes on the same cycle as in
// MAME. See pp.c.
#ifndef POLEPOS_PP_H
#define POLEPOS_PP_H

#include <stdbool.h>
#include <stdint.h>

#define PP_CLOCK 3072000u               // the Z80's and the Z8002s' (MASTER_CLOCK / 8)
#define PP_W 256                        // the picture MAME shows: x 0-255
#define PP_H 224                        //   and lines 16-239

// the ROMs, as the files are (MAME's polepos set and the namco5x MCUs)
typedef struct {
	const uint8_t *maincpu;             // 0x3000: pp3_9.6h, pp1_10b.5h
	const uint8_t *sub1_hi, *sub1_lo;   // 0x2000 each: pp3_2.8l (even bytes), pp3_1.8m (odd)
	const uint8_t *sub2_hi, *sub2_lo;   // pp3_6.4l, pp3_5.4m
	const uint8_t *mcu[4];              // 0x400 each: 51xx.bin, 52xx.bin, 53xx.bin, 54xx.bin
	const uint8_t *chars;               // 0x1000 pp3_28.1f
	const uint8_t *tiles;               // 0x1000 pp1_29.1e
	const uint8_t *sprites;             // 0x4000 pp3_25.1n, pp3_26.1m
	const uint8_t *bigsprites;          // 0x10000: pp1_17, 19, 21 at 0, 0x2000, 0x4000; pp1_18, 20, 22 at 0x8000...
	const uint8_t *road;                // 0x5000 pp1_30.3a, pp1_31.2a, pp1_32.1a
	const uint8_t *scalelut;            // 0x1000 pp1_27.1l
	const uint8_t *proms;               // 0x1040: the colour and position PROMs as MAME loads them
	const uint8_t *wave;                // 0x100 pp1-5.3b
	const uint8_t *engine;              // 0x4000 pp1_15.6a, pp1_16.5a
	const uint8_t *speech;              // 0x8000 pp2_11.2e, pp2_12.2f, pp2_13.1e (the rest 0)
} pp_roms_t;

// the inputs, as MAME's ports read (IN0 active low; the gear is IN0 bit 1)
typedef struct {
	uint8_t in0;                        // 0x02 gear, 0x10 coin 1, 0x20 coin 2, 0x40 service 1, 0x80 service
	uint8_t dswa, dswb;
	uint8_t accel, brake;               // 0-0x90
	uint8_t steer;                      // the dial, 0-255 (it wraps)
} pp_inputs_t;
#define PP_IN0 0xfb                     // nothing pressed (bit 2 is the program's auto start, not this)
#define PP_DSWA 0xff                    // poleposa's defaults (MAME's polepos uses them)
#define PP_DSWB 0x74

extern pp_inputs_t pp_in;

// the shared RAM, 16-bit words as the Z8002s see them (the Z80 sees the low bytes)
extern uint16_t pp_sprite[0x800], pp_road[0x400], pp_alpha[0x400], pp_view[0x800];
extern uint16_t pp_hscroll, pp_vscroll;
extern uint8_t pp_nvram[0x800], pp_z80ram[0x400];
extern uint8_t pp_latch, pp_chacl;
extern uint32_t pp_frames, pp_resets;
extern void (*pp_on_vblank)(void);      // at VBLANK start (MAME's frame_update), the picture is RAM as it is now

void pp_init(const pp_roms_t *r);
// SRAM that pp_init fills (the CPUs' programs): before it, the plugin's to
// use (the card's reads, a message's canvas); its size in *n
void *pp_spare(uint32_t *n);
bool pp_frame(void);                    // to the next VBLANK start; false after a watchdog reset
uint32_t pp_run_to(uint64_t until);     // until the scheduler's base time is at Z80 cycle `until`; the VBLANKs passed
uint64_t pp_time(void);                 // the base time in Z80 cycles

// statistics (tests)
typedef struct {
	uint64_t slices, runs[7], cycles[7], timers, syncs;
} pp_stats_t;
extern pp_stats_t pp_stats;

// an instruction trace (tests): before each instruction of the CPUs in pp_trace_mask
extern void (*pp_trace)(int cpu, uint32_t pc, uint64_t total);
extern uint32_t pp_trace_mask;
void pp_trace_on(void);                 // (after setting pp_trace)

// the CPUs' state for the tests (0 Z80, 1 sub1, 2 sub2, 3-6 the 51xx-54xx)
void pp_cpu_regs(int cpu, uint16_t *r, int *n);
uint8_t pp_mcu_ram(int mcu, int a);

// the sound side (ppsnd.c): the board calls these with the time of the write
typedef struct { int32_t s; int64_t a; } pp_at_t;
pp_at_t pp_now(void);

#endif
