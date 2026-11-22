# What in this repository came from elsewhere

The plugin's own code, in `src/`, is under the BSD 3-Clause license
(`LICENSE`). The copy of the cart's SDK in `sdk/` is under the MIT license
(`sdk/LICENSE`). These parts came from elsewhere and keep their own terms.

| Where | What | License |
|---|---|---|
| `sdk/firmware/revc/sdk/tinyusb/` | [TinyUSB](https://github.com/hathach/tinyusb) (Ha Thach and contributors), with the cart's fixes, listed in its `VENDORED.md` | MIT (`sdk/firmware/revc/sdk/tinyusb/LICENSE`) |
| `cmake/pico_sdk_import.cmake` | from the [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) | BSD 3-Clause |

The built plugin also contains the Raspberry Pi Pico SDK's runtime (BSD
3-Clause) and TinyUSB (MIT).

## MAME

Parts of the code in `src/` are ports of MAME's code, or were written with
MAME's code as the reference. That code is under the BSD 3-Clause license
(`LICENSE`). It comes from MAME (<https://github.com/mamedev/mame>, tag
`mame0288`), and its copyright holders are these.

| Here | From MAME | Copyright holders |
|---|---|---|
| `src/polepos/pp.c`: the memory maps, the inputs, the interrupts, the steering, the customs' wiring | `src/mame/namco/polepos.cpp` | Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles, Nicola Salmoria |
| `src/polepos/pp.c`: the 06xx bus controller and the 51xx, 52xx, 53xx and 54xx customs' glue | `src/mame/namco/namco06.cpp`, `namco51.cpp`, `namco52.cpp`, `namco53.cpp`, `namco54.cpp` | Mike Harris, Aaron Giles |
| `src/polepos/pp.c`: the latch, the watchdog, the pedals' A/D converter | `src/devices/machine/74259.cpp`, `watchdog.cpp`, `adc0804.cpp` | AJR; Aaron Giles |
| `src/polepos/pp.c`: timers, timeslices, VBLANK, the scanline timer | `src/emu/schedule.cpp`, `diexec.cpp`, `screen.cpp`, `timer.cpp` | Aaron Giles |
| `src/polepos/ppvid.c`: the palette, the tilemaps, the road and the sprites | `src/mame/namco/polepos_v.cpp` | Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles, Nicola Salmoria |
| `src/polepos/ppsnd.c`: the WSG | `src/devices/sound/namco.cpp` | Nicola Salmoria, Aaron Giles |
| `src/polepos/ppsnd.c`: the engine and the circuits after the 52xx and 54xx | `src/mame/namco/polepos_a.cpp`, `src/devices/sound/discrete.cpp`, `disc_*.hxx` | Derrick Renaud; K. Wilkins, Couriersud, Derrick Renaud, Frank Palazzolo |
| `src/polepos/z80.c`, `src/pacman/z80_ops.h`, `src/pacman/tools/z80gen.py` | `src/devices/cpu/z80` (`z80.cpp`, `z80.lst`) | Juergen Buchmueller, Andrei I. Holub |
| `src/polepos/z8k.c`, `z8k_ops.h`, `tools/z8kgen.py` | `src/devices/cpu/z8000` | Juergen Buchmueller, Ernesto Corvi |
| `src/polepos/mb88.c`, `mb88_ops.h`, `tools/mb88gen.py` | `src/devices/cpu/mb88xx` | Ernesto Corvi |

The zip reader in `src/starwars/swzip.c` follows RFC 1951 and the method of
zlib's `contrib/puff` by Mark Adler (zlib license). It was written afresh,
for the cart's Star Wars plugin.

Pole Position is a trademark of Bandai Namco. Parrot Cart isn't connected
with them. This plugin doesn't include any game ROMs.

The 8x8 screen font's letter shapes come from the 1982 arcade game Mr. Do!
