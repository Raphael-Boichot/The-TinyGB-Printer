# The TinyGB Printer /// A WORK IN PROGRESS

For the moment it is just the [NeoGB printer](https://github.com/zenaro147/NeoGB-Printer) less everything but the SD file system. It compiles on [RP2040 core for Arduino IDE](https://github.com/earlephilhower/arduino-pico). The goal in to move the project from the crap ESP32 platform to Raspberry Pi Pico and turns it into the most easy-to-use and simple device as possible.

## What have been done at the moment
- removed/comment all ESP32/Wifi/LED/png specific command until it compiles for the rp2040 core. Removed all unneccessary dependencies. Pins not allocated, still the same as ESP32.
