/*********************************
 * GAMEBOY LINK CABLE DEFINITIONS
 *********************************/
// Note: Serial Clock Pin must be attached to an interrupt pin of the ESP (Pins 1 and 4 are not used. Pin 6 must be connect to the GND)
//  ___________
// |  6  4  2  |
//  \_5__3__1_/   (at cable)
//
#define ESP_MOSI_PIN 23
#define ESP_MISO_PIN 19
#define ESP_CLK_PIN 18
//#define INVERT_SERIAL_PINS //Invert pin 2 and 3 order, since the pin 2 goes o 3 in the other side of the cable, and 3 goes to 2. This is useful if you are using a breakout board for the link cable

/*****************************
 * SD CARD MODULE DEFINITIONS 
 *****************************/
//Define the SD Card Module Pins
#define SD_CS   15
#define SD_SCK  14
#define SD_MOSI 13
#define SD_MISO 26

/************************************
 * PUSH BUTTON AND IMAGE DEFINITIONS 
 ************************************/
#define BTN_PUSH 34             // Define a PushButton to use to Force a new file/Generate the images.