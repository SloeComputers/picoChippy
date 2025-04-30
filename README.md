# picoVGM2151

![picoxVGM2151](docs/picoVGM2151.png)

A VGM player using a Raspberry Pi Pico, a YM2151 (OPM) sound chip and an I2S DAC.

[Block Diagram](docs/block_diagram.pdf)

## Status

## Hardware

![prototype](docs/breadboard.jpg)

[Schematic](docs/schematic.pdf) for the above.

## Software

### Checkout

This repo uses git sub-modules, so checkout using --recurse to clone all the
dependent source...

    git clone --recurse https://github.com/AnotherJohnH/picoVGM2151.git

ore

    git clone --recurse ssh://git@github.com/AnotherJohnH/picoVGM2151.git

### Software dependencies

+ https://github.com/AnotherJohnH/Platform
+ arm-none-eabi-gcc
+ cmake via UNIX make or auto detection of ninja if installed
+ Python3

### Build

Being developed on MacOS but should build on Linux too.

Indirect build of all supported targets, rpipico and rpipico2 with cmake and make (or ninja)...

    make

Build a single hardware target e.g. rpipico2 using cmake...

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release -DPLT_TARGET=rpipico2 -DCMAKE_TOOLCHAIN_FILE=Platform/MTL/rpipico2/toolchain.cmake ..
    make

flashable images will be found under the build sub-directory here...

    build/Source/picoVGM2151_I2S_DAC.uf2

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgements

