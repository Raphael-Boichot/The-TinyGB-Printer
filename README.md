# The TinyGB Printer /// A WORK IN PROGRESS

The tiniest possible Game Boy printer emulator storing images on SD card made with easily available parts and easy to assemble. Basically a simplified NeoGB printer. 100% compatibility with all known GB/GBC games, homebrews included.

## Libraries
- [RP2040 core for Arduino IDE](https://github.com/earlephilhower/arduino-pico)
- [Adafruit Neopixel for Arduino IDE](https://github.com/adafruit/Adafruit_NeoPixel)

## What have been done at the moment
- a PCB with all individual functions working was done (works natively with the [Arduino emulator](https://github.com/mofosyne/arduino-gameboy-printer-emulator), access to LED and SD card OK).
- Starting from the NeoGB Printer code and removed/commented all ESP32/Wifi/LED/png specific command until it compiles for the rp2040 core. Removed all unneccessary dependencies. Pins not allocated, still the same as ESP32.
- Starting now from the mofosyne code. I think it will be simplier to rebuild from it while stealing chunks of the NeoGB Printer when necessary.
- Support for the WS2812 internal LED in mofosyne code added.
- Support for the SD card in mofosyne code added.
- Main functions for making BMPs and keep track of file ID added.

## How do I want it to work ?
- boot without pressing pushbutton: margin mode (green LED), an image is closed automatically when an after margin is detected;
- boot while pressing pushbutton: idle mode (blue LED), an image is closed only when you press the pushbutton. Pressing the pushbutton is like tearing paper;
- red led at boot: the device has general SD failure or no SD card, it can only work with the USB port, but works nevertheless (or not, have to see).
- it must run on one single or two AA NiMH batteries because lithium is evil.

## Showcase
![](Tiny_GB_Printer.jpg)
