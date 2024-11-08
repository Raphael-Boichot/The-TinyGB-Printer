/*********************************
 * GAMEBOY LINK CABLE DEFINITIONS
 *********************************/
// Note: Serial Clock Pin must be attached to an interrupt pin
//  ___________
// |  6  4  2  |
//  \_5__3__1_/   (at cable, Game Boy Side)
//
#define PICO_CLK 2
#define PICO_SOUT 3
#define PICO_SIN 4

/*****************************
 * SD CARD MODULE DEFINITIONS 
 *****************************/
//    SD card attached to SPI bus as follows on RP2040:
//   ************ SPI0 ************
//   ** MISO (AKA RX) - pin 0, 4, or 16
//   ** MOSI (AKA TX) - pin 3, 7, or 19
//   ** CS            - pin 1, 5, or 17
//   ** SCK           - pin 2, 6, or 18
//   ************ SPI1 ************
//   ** MISO (AKA RX) - pin  8 or 12
//   ** MOSI (AKA TX) - pin 11 or 15
//   ** CS            - pin  9 or 13
//   ** SCK           - pin 10 or 14

#define SD_CS   9
#define SD_SCK  10
#define SD_MOSI 11
#define SD_MISO 8

/************************************
 * PUSH BUTTON AND LED DEFINITIONS 
 ************************************/
#define BTN_PUSH 12             //Define a PushButton to use to Force a new file in idle mode
#define LED_WS2812 16           //Pi pico waveshare zero RGB LED PIN
