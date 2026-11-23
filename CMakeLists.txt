# polepos (POLEPOS.CVP): Namco's 1982 Pole Position arcade game on the
# ColecoVision. The cart runs the arcade board (a Z80, two Z8002s, four
# Namco MCUs and the sound: src/polepos/pp.c, from MAME 0.288) from the user's ROMs
# on the card (/POLEPOS), and ppcv sends the picture and the sound to the
# console as the Game Boy kernel's lists. RESET goes back to the menu.
#
# The ROMs (212K) and the code are more than SRAM holds, so this is a flash
# binary run in place (XIP) from its slot, with the code a list runs in
# SRAM: PP_HOT, Z80_HOT, Z8K_HOT, MB88_HOT, PPVID_HOT, PPCV_HOT and
# PPSND_HOT put it in .time_critical, which crt0 copies there. The ROMs'
# store and a message's canvas are in the PSRAM (cvpsram.h); pp_init copies
# the CPUs' programs (64K) into SRAM. The picture's and the sound's ROMs
# are read from the PSRAM through the XIP cache.
#
#   cmake -B build -G Ninja -DPICO_SDK_PATH=$HOME/pico-sdk
#   ninja -C build                      -> build/POLEPOS.CVP
#
# Copy POLEPOS.CVP into the card's /PLUGINS folder.
cmake_minimum_required(VERSION 3.13)

# the cart's SDK: the copy in sdk/ (sdk/README.md)
set(PARROTCART_SDK ${CMAKE_CURRENT_LIST_DIR}/sdk/firmware/revc)
set(PICO_BOARD_HEADER_DIRS ${PARROTCART_SDK}/cvcart_revc/boards)
set(PICO_BOARD cvcart_revc)
set(PICO_PLATFORM rp2350-arm-s)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
# the USB host: sdk/tinyusb (TinyUSB 0.21 and its fixes since), not the pico-sdk's 0.18
include(${PARROTCART_SDK}/sdk/cvpad_tinyusb.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/pico_sdk_import.cmake)

project(polepos C CXX ASM)
pico_sdk_init()
include(${PARROTCART_SDK}/sdk/cvplugin.cmake)

# The same source makes the same file, whenever it's built: the compiler's
# __DATE__ and __TIME__ (the build date in the binary info) come from
# SOURCE_DATE_EPOCH, fixed here at the tested build's day, 2026-11-21.
set(BUILD_EPOCH 1795262400)
set(CMAKE_C_COMPILER_LAUNCHER ${CMAKE_COMMAND} -E env SOURCE_DATE_EPOCH=${BUILD_EPOCH})
set(CMAKE_CXX_COMPILER_LAUNCHER ${CMAKE_COMMAND} -E env SOURCE_DATE_EPOCH=${BUILD_EPOCH})

# the plugin's code (the cart's firmware/revc/polepos/, with plugin/ beside it)
set(S ${CMAKE_CURRENT_LIST_DIR}/src/polepos)

set(OURS ${S}/polepos.c ${S}/pp.c ${S}/z80.c ${S}/z8k.c ${S}/mb88.c ${S}/ppsnd.c ${S}/ppvid.c ${S}/ppcv.c ${S}/pprun.c ${S}/ppcard.c ${S}/pproms.c
    ${PARROTCART_SDK}/sdk/cvscale.c ${CMAKE_CURRENT_LIST_DIR}/src/starwars/swzip.c)
set_source_files_properties(${OURS} PROPERTIES COMPILE_OPTIONS "-Wall;-Wno-unused-function;-Wno-unused-but-set-variable;-fstack-usage")
# the emulation's state is many small statics: in one section each, every
# use needs a separate address loaded, so not
set_source_files_properties(${S}/pp.c ${S}/z8k.c ${S}/mb88.c PROPERTIES COMPILE_OPTIONS
                            "-Wall;-Wno-unused-function;-Wno-unused-but-set-variable;-fstack-usage;-fno-data-sections;-O3")
# the Z80's one big body runs as fast built for size (test/armcount.sh) and is 6K smaller
set_source_files_properties(${S}/z80.c PROPERTIES COMPILE_OPTIONS
                            "-Wall;-Wno-unused-function;-Wno-unused-but-set-variable;-fstack-usage;-fno-data-sections;-Os")
# the card's code and the start's run once: built for size
set_source_files_properties(${S}/polepos.c ${S}/ppcard.c ${S}/pproms.c ${CMAKE_CURRENT_LIST_DIR}/src/starwars/swzip.c PROPERTIES COMPILE_OPTIONS
                            "-Wall;-Wno-unused-function;-fstack-usage;-Os")
set_source_files_properties(${PARROTCART_SDK}/sdplayer/fsro.c PROPERTIES COMPILE_OPTIONS "-Os")

add_executable(polepos ${OURS} ${PARROTCART_SDK}/midihello/vdp.c)
target_include_directories(polepos PRIVATE ${S} ${PARROTCART_SDK}/midihello
                           ${PARROTCART_SDK}/sdk)
# core 0's stack: all of scratch Y (4K); core 1's as before (scratch X also holds its bus loop)
target_compile_definitions(polepos PRIVATE PICO_PRINTF_SUPPORT_FLOAT=0 PICO_PRINTF_SUPPORT_EXPONENTIAL=0 PP_NO_TRACE PPROMS_PSRAM
    PICO_STACK_SIZE=0x1000 PICO_CORE1_STACK_SIZE=0x800)
cvframes_use(polepos)
cvframes_serve2(polepos)     # core 1's bus loop acts on two samples of the pins that agree (sdk/cvfserve.S)
cvsd_use(polepos)
cvpsram_use(polepos)
cvpad_use(polepos)                           # USB game pads: the port is a host
pico_enable_stdio_usb(polepos 0)
pico_enable_stdio_uart(polepos 0)
cvcart_plugin(polepos ID parrotcart.polepos NAME "Pole Position" VERSION 0.1.3 BY "Parrot Cart"
              HOME /POLEPOS USES usb-host ICON src/polepos/polepos_icon.txt FILE POLEPOS.CVP README CART-README.md XIP)
