# Cambridge pico Chippy

![picoChippy](docs/picoChippy.png)

A VGM player using a Raspberry Pi Pico, a YM2151 (OPM) sound chip and an I2S DAC.

Re-uses the same hardware as the sister project [picoX21H](https://github.com/SloeComputers/picoX21H)

[Block Diagram](docs/block_diagram.pdf)

## Status

WIP ... just started!!!

Working...
   + VGM decode (but only for supported chips)
   + YM2151 hardware driver and YMDAC to I2S translator
   + Following emulations mixed into the main I2S stream...
   + SegaPCM
   + SN76489
   + OKIM5295

## Hardware

![prototype](https://github.com/SloeComputers/Hardware/raw/main/picoX21H/docs/breadboard.jpg)

[Schematic](https://github.com/SloeComputers/Hardware/blob/main/picoX21H/docs/schematic.pdf) for the above.

See [README](https://github.com/SloeComputers/Hardware/blob/main/picoX21H/README.md) for more information.

## Software

### Checkout

This repo uses git sub-modules, so checkout using --recurse to clone all the
dependent source...

    git clone --recurse-submodules https://github.com/SloeComputers/picoChippy.git

or

    git clone --recurse-submodules ssh://git@github.com/SloeComputers/picoChippy.git

### Software dependencies

+ https://github.com/SloeComputers/PDK
+ arm-none-eabi-gcc
+ cmake via UNIX make or auto detection of ninja if installed
+ Python3
+ SDL2 (only for native test target)

### Build

Being developed on MacOS but should build on Linux too.

Indirect build for supported target rpipico2 with cmake and make (or ninja)...

    make

Build a single hardware target e.g. rpipico2 using cmake...

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release -DPLT_TARGET=rp2350 -DCMAKE_TOOLCHAIN_FILE=PDK/MTL/rp2350/target/toolchain.cmake ..
    make

flashable images will be found under the build sub-directory here...

    build/rpipico2/picoChippy_RPIPICO_I2S_DAC.uf2

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgements

The VGM rippers
