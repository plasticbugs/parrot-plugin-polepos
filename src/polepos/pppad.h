// pppad.h -- the controls: the ColecoVision controller or a USB pad as Pole
// Position's wheel, pedals and gear lever. Portable, header-only.
//
// The cabinet's wheel is an optical encoder (MAME's STEER dial): the game
// steers by how fast it turns, not where it is. So the wheel here is a
// position that wraps, turned a little each frame (the Pocket core's feel):
//   the joystick or d-pad   2 counts a frame, 4 once held half a second
//   a pad's left stick      in proportion to how far it is pushed, up to
//                           3.5 counts a frame, fractions carried over
// The pedals go 0 to 0x90 (MAME's range) in 16s, a frame at a time, as
// MAME's keys move them. The gear lever is a switch with two places, HI
// and LO, and stays where it was last put.
//
//   ColecoVision   joystick left and right the wheel, up HI, down LO; right
//                  fire accelerator, left fire brake; keypad * or # coin
//   USB pad        d-pad left and right or the left stick the wheel; R
//                  shoulder HI, L shoulder LO; A (right face) or ZR
//                  accelerator, B (bottom) or ZL brake; - coin
//
// There is no start button on the cabinet: a coin, then the accelerator.
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "../sdk/gamepad.h"
#include "pp.h"

// the ColecoVision keypad's codes, as the kernel reports them
enum { PPKEY_0 = 0xA, PPKEY_1 = 0xD, PPKEY_STAR = 0x9, PPKEY_HASH = 0x6, PPKEY_NONE = 0xF };

typedef struct {
    uint8_t steer;              // the wheel, as the board reads it
    uint8_t held;               // frames a direction has been held (to 32)
    uint8_t frac;               // the stick's part of a count, in 256ths
    uint8_t accel, brake;       // 0..0x90
    bool gear_hi;
    uint8_t coin_left;          // frames of the coin pulse still to give the game
    uint8_t coin_wait;          // frames the coin key must still be up before a press counts
} pppad_t;

#define PPPAD_COIN     6        // a coin is a pulse this long however long the key is held,
#define PPPAD_COIN_UP  3        // and another needs the key up this long
#define PPPAD_DPAD     2        // counts a frame, twice that after
#define PPPAD_DPAD_2X  32       // frames held
#define PPPAD_DEAD     256      // a stick this near its middle (of 2047) is at rest
#define PPPAD_STICK    896      // full lock: 3.5 counts a frame, in 256ths
#define PPPAD_PEDAL    0x10
#define PPPAD_PEDAL_MAX 0x90

static inline uint8_t pppad_pedal(uint8_t v, bool down) {
    if (down) return v >= PPPAD_PEDAL_MAX - PPPAD_PEDAL ? PPPAD_PEDAL_MAX : (uint8_t)(v + PPPAD_PEDAL);
    return v <= PPPAD_PEDAL ? 0 : (uint8_t)(v - PPPAD_PEDAL);
}

// One TV frame's inputs. joy, fire, key: the console's controller as
// cvf_input() gives it (low = held: joy bit 0 up, 1 right, 2 down, 3 left;
// fire bit 0 left, 1 right; key a PPKEY_ code). pad: a USB pad, or NULL.
// Sets in's in0 (the gear and the coin), accel, brake and steer.
static inline void pppad_frame(pppad_t *s, uint8_t joy, uint8_t fire, uint8_t key, const gamepad_t *pad, pp_inputs_t *in) {
    int dir = (!(joy & 2) ? 1 : 0) - (!(joy & 8) ? 1 : 0);
    bool acc = !(fire & 2), brk = !(fire & 1);
    bool coin = key == PPKEY_STAR || key == PPKEY_HASH;
    if (!(joy & 1)) s->gear_hi = true;
    if (!(joy & 4)) s->gear_hi = false;
    int stick = 0;
    if (pad) {
        uint32_t b = pad->buttons;
        if (b & GP_RIGHT) dir = 1;
        if (b & GP_LEFT) dir = -1;
        if (b & (GP_EAST | GP_ZR)) acc = true;
        if (b & (GP_SOUTH | GP_ZL)) brk = true;
        if (b & GP_MINUS) coin = true;
        if (b & GP_R) s->gear_hi = true;
        if (b & GP_L) s->gear_hi = false;
        int x = pad->lx;
        if (x >= PPPAD_DEAD || x <= -PPPAD_DEAD) stick = x;
    }
    if (stick) {                                    // the stick: its rate, with the fraction carried
        int m = stick > 0 ? stick : -stick;
        uint32_t add = (uint32_t)(m - PPPAD_DEAD) * PPPAD_STICK / (2047 - PPPAD_DEAD) + s->frac;
        int whole = (int)(add >> 8);
        s->frac = (uint8_t)add;
        s->steer = (uint8_t)(stick > 0 ? s->steer + whole : s->steer - whole);
        s->held = 0;
    } else if (dir) {
        s->frac = 0;
        if (s->held < PPPAD_DPAD_2X) s->held++;
        int n = s->held >= PPPAD_DPAD_2X ? 2 * PPPAD_DPAD : PPPAD_DPAD;
        s->steer = (uint8_t)(s->steer + dir * n);
    } else {
        s->held = 0;
        s->frac = 0;
    }
    s->accel = pppad_pedal(s->accel, acc);
    s->brake = pppad_pedal(s->brake, brk);
    if (coin && !s->coin_wait) s->coin_left = PPPAD_COIN;
    if (coin) s->coin_wait = PPPAD_COIN_UP;
    else if (s->coin_wait) s->coin_wait--;
    uint8_t v = PP_IN0;
    if (s->gear_hi) v &= (uint8_t)~0x02;            // the lever in HI (MAME's gear bit low: its toggle on)
    if (s->coin_left) { v &= (uint8_t)~0x10; s->coin_left--; }
    in->in0 = v;
    in->accel = s->accel;
    in->brake = s->brake;
    in->steer = s->steer;
}
