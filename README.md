# Pole Position

This plugin plays Namco's 1982 Pole Position arcade game on the ColecoVision. It needs the game's ROMs from MAME, which you supply.

## Setup

1. Copy `POLEPOS.CVP` into the `/PLUGINS` folder on the card.
2. Make a folder named `/POLEPOS`. Put MAME's `polepos.zip` in it as it is, along with `namco51.zip`, `namco52.zip`, `namco53.zip` and `namco54.zip`. Loose files from those zips work too. You can also put the game's zip in the card's top folder as `/POLEPOS.ZIP`.
3. Start Pole Position from the Plugins page in the menu.

The files can have any names, because the plugin knows each ROM by its size and contents. If a ROM is missing, the screen names it.

## Controls

On the ColecoVision controller:

- Left and right on the joystick turn the wheel.
- Up on the joystick shifts to HI, and down shifts to LO.
- The right fire button is the accelerator. The left fire button is the brake.
- Keypad * or # adds a coin.
- Press RESET on the console to go back to the menu.

On a USB pad, through a powered hub:

- The D-pad or the left stick turns the wheel. The stick turns it faster the further you push it.
- A or ZR is the accelerator. B or ZL is the brake.
- The R shoulder button shifts to HI, and L shifts to LO.
- Minus adds a coin.

The arcade machine has no start button. Add a coin, then press the accelerator.

The arcade wheel spins freely, and the game steers by how fast it turns. Holding the joystick turns it at a steady rate, twice as fast after half a second.

## Picture and sound

The arcade screen is 256 by 224 pixels. The plugin shows it at half size, 128 by 112, in the middle of the screen. The text rows at the top (TOP, TIME, LAP, SCORE and SPEED) and the bottom go in the borders at full size, so they stay easy to read.

Colors are the nearest of the ColecoVision's 15. The ColecoVision has one grey, so the road is lighter than on the arcade machine.

The arcade's sound, with its eight voices, the engine, the speech and the crashes, plays through the ColecoVision's sound chip as samples. While the voice talks, the plugin sends it 8,000 times a second and turns the other sounds down, so the words come through clearly.

High scores last until you go back to the menu.

## Credits

The arcade emulation is a port of MAME 0.288's code, under the BSD-3-Clause license. Its authors are:

- Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles and Nicola Salmoria, for the Pole Position driver.
- Juergen Buchmueller and Ernesto Corvi, for the Z8000 CPU.
- Ernesto Corvi, for the MB88 MCU.
- Mike Harris and Aaron Giles, for the Namco 06xx and 51xx.
- Aaron Giles, for the Namco 52xx, 53xx and 54xx, MAME's scheduler and the watchdog.
- Juergen Buchmueller, for the Z80 CPU.
- Nicola Salmoria and Aaron Giles, for the Namco sound chip.
- Derrick Renaud, for the Pole Position sound.
- K. Wilkins, Couriersud, Derrick Renaud and Frank Palazzolo, for the discrete sound circuits.
- AJR, for the 74LS259 latch.

You can find MAME at https://www.mamedev.org. The zip reader follows Mark Adler's puff, from zlib. USB support comes from TinyUSB by Ha Thach, under the MIT license.

The plugin itself is by Parrot Cart, and it's free. Its source is in this repository, https://github.com/plasticbugs/parrot-plugin-polepos.

Pole Position is a trademark of Bandai Namco. Parrot Cart isn't connected with them. This plugin doesn't include any game ROMs.

## Build it yourself

You need the Raspberry Pi Pico SDK 2.1.1, an Arm GNU toolchain, CMake,
Ninja, Python 3, sjasmplus and picotool. Releases are built with the Arm
GNU Toolchain 9-2019-q4 (GCC 9.2.1) and picotool 2.3.1. With those you get
the same file, byte for byte.

```sh
cmake -B build -G Ninja -DPICO_SDK_PATH=$HOME/pico-sdk
ninja -C build
```

This makes `build/POLEPOS.CVP`. Copy it into the `/PLUGINS` folder on the card.

On Ubuntu, `tools/ci/setup.sh` installs the same tools that GitHub uses to
build each release.

`CART-README.md` is the text the cart shows on the plugin's page. This
README starts with the same text.

## License

The plugin is under the BSD 3-Clause license (`LICENSE`). `THIRD-PARTY.md`
lists the parts that came from elsewhere, and their licenses.
