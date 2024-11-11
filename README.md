# The TinyGB Printer /// A WORK IN PROGRESS

For the moment it is just the [NeoGB printer](https://github.com/zenaro147/NeoGB-Printer) less everything but the SD file system. It compiles on [RP2040 core for Arduino IDE](https://github.com/earlephilhower/arduino-pico). The goal in to move the project from the crap ESP32 platform to Raspberry Pi Pico and turns it into the most easy-to-use and simple device as possible.

## What have been done at the moment
- a PCB with all individual functions working was done (works natively with the [Arduino emulator](https://github.com/mofosyne/arduino-gameboy-printer-emulator), access to LED and SD card OK).
- Starting from the NeoGB Printer code and removed/commented all ESP32/Wifi/LED/png specific command until it compiles for the rp2040 core. Removed all unneccessary dependencies. Pins not allocated, still the same as ESP32.
- Starting now from the mofosyne code to compare. I think it will be simplier to rebuild from it stealing chunks of the NeoGB Printer.

## Showcase
![](Tiny_GB_Printer.jpg)
