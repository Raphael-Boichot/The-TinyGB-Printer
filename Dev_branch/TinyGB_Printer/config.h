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

#define SD_MISO 8    // SD card SPI1
#define SD_CS 9      // SD card SPI1
#define SD_SCK 10    // SD card SPI1
#define SD_MOSI 11   // SD card SPI1
#define BTN_PUSH 12  // Define a PushButton to use to Force a new file in idle mode ///BOICHOT

/* Gameboy Link Cable Mapping to Arduino Pin */
// Note: Serial Clock Pin must be attached to an interrupt pin of the arduino
//  ___________
// |  6  4  2  |
//  \_5__3__1_/   (at cable)
//
#define GBP_SO_PIN 4       // Pin 2            : Serial OUTPUT
#define GBP_SI_PIN 3       // Pin 3            : Serial INPUT
#define GBP_SC_PIN 2       // Pin 5            : Serial Clock (Interrupt)
#define LED_STATUS_PIN 16  // Internal LED blink on packet reception
#define NUMPIXELS 1        // NeoPixel ring size (just internal LED here)
Adafruit_NeoPixel pixels(NUMPIXELS, LED_STATUS_PIN, NEO_RGB);
uint8_t intensity = 60;                                    //WS2812 intensity 255 is a death ray, 10 to 15 is normal
uint32_t WS2812_Color = pixels.Color(0, intensity, 0);     //RGB triplet, default is green, turns to blue in tear mode
uint32_t WS2812_SD_crash = pixels.Color(intensity, 0, 0);  //RGB triplet, turn to red, issue with SD card
uint32_t WS2812_Idle = pixels.Color(1, 1, 1);              //RGB triplet, turn to cyan, device idle

//Offset for the 240*240 ST7789 display
#define x_ori 40
#define y_ori 96
#define BITS_PER_PIXEL 16
unsigned short int pixel_TFT_RGB565;
//grayscale 8 bits table
unsigned short int lookup_TFT_RGB565[256] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x0020, 0x0020, 0x0020, 0x0841, 0x0841, 0x0841, 0x0841, 0x0861, 0x0861, 0x0861, 0x0861,
                                              0x1082, 0x1082, 0x1082, 0x1082, 0x10A2, 0x10A2, 0x10A2, 0x10A2, 0x18C3, 0x18C3, 0x18C3, 0x18C3, 0x18E3, 0x18E3, 0x18E3, 0x18E3,
                                              0x2104, 0x2104, 0x2104, 0x2104, 0x2124, 0x2124, 0x2124, 0x2124, 0x2945, 0x2945, 0x2945, 0x2945, 0x2965, 0x2965, 0x2965, 0x2965,
                                              0x3186, 0x3186, 0x3186, 0x3186, 0x31A6, 0x31A6, 0x31A6, 0x31A6, 0x39C7, 0x39C7, 0x39C7, 0x39C7, 0x39E7, 0x39E7, 0x39E7, 0x39E7,
                                              0x4208, 0x4208, 0x4208, 0x4208, 0x4228, 0x4228, 0x4228, 0x4228, 0x4A49, 0x4A49, 0x4A49, 0x4A49, 0x4A69, 0x4A69, 0x4A69, 0x4A69,
                                              0x528A, 0x528A, 0x528A, 0x528A, 0x52AA, 0x52AA, 0x52AA, 0x52AA, 0x5ACB, 0x5ACB, 0x5ACB, 0x5ACB, 0x5AEB, 0x5AEB, 0x5AEB, 0x5AEB,
                                              0x630C, 0x630C, 0x630C, 0x630C, 0x632C, 0x632C, 0x632C, 0x632C, 0x6B4D, 0x6B4D, 0x6B4D, 0x6B4D, 0x6B6D, 0x6B6D, 0x6B6D, 0x6B6D,
                                              0x738E, 0x738E, 0x738E, 0x738E, 0x73AE, 0x73AE, 0x73AE, 0x73AE, 0x7BCF, 0x7BCF, 0x7BCF, 0x7BCF, 0x7BEF, 0x7BEF, 0x7BEF, 0x7BEF,
                                              0x8410, 0x8410, 0x8410, 0x8410, 0x8430, 0x8430, 0x8430, 0x8430, 0x8C51, 0x8C51, 0x8C51, 0x8C51, 0x8C71, 0x8C71, 0x8C71, 0x8C71,
                                              0x9492, 0x9492, 0x9492, 0x9492, 0x94B2, 0x94B2, 0x94B2, 0x94B2, 0x9CD3, 0x9CD3, 0x9CD3, 0x9CD3, 0x9CF3, 0x9CF3, 0x9CF3, 0x9CF3,
                                              0xA514, 0xA514, 0xA514, 0xA514, 0xA534, 0xA534, 0xA534, 0xA534, 0xAD55, 0xAD55, 0xAD55, 0xAD55, 0xAD75, 0xAD75, 0xAD75, 0xAD75,
                                              0xB596, 0xB596, 0xB596, 0xB596, 0xB5B6, 0xB5B6, 0xB5B6, 0xB5B6, 0xBDD7, 0xBDD7, 0xBDD7, 0xBDD7, 0xBDF7, 0xBDF7, 0xBDF7, 0xBDF7,
                                              0xC618, 0xC618, 0xC618, 0xC618, 0xC638, 0xC638, 0xC638, 0xC638, 0xCE59, 0xCE59, 0xCE59, 0xCE59, 0xCE79, 0xCE79, 0xCE79, 0xCE79,
                                              0xD69A, 0xD69A, 0xD69A, 0xD69A, 0xD6BA, 0xD6BA, 0xD6BA, 0xD6BA, 0xDEDB, 0xDEDB, 0xDEDB, 0xDEDB, 0xDEFB, 0xDEFB, 0xDEFB, 0xDEFB,
                                              0xE71C, 0xE71C, 0xE71C, 0xE71C, 0xE73C, 0xE73C, 0xE73C, 0xE73C, 0xEF5D, 0xEF5D, 0xEF5D, 0xEF5D, 0xEF7D, 0xEF7D, 0xEF7D, 0xEF7D,
                                              0xF79E, 0xF79E, 0xF79E, 0xF79E, 0xF7BE, 0xF7BE, 0xF7BE, 0xF7BE, 0xFFDF, 0xFFDF, 0xFFDF, 0xFFDF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };

unsigned int Next_ID, Next_dir;                           //for directories and filenames
unsigned char printer_memory_buffer_core_0[9 * 640 + 1];  //Game Boy printer buffer of 9*640 bytes (maximum possible + margin in case of buffer overflow), core 0
unsigned char printer_memory_buffer_core_1[9 * 640 + 1];  //Game Boy printer buffer of 9*640 bytes (maximum possible + margin in case of buffer overflow), core 1
unsigned char PNG_image_color[144 * 160];                 //"color" RGB 2bbp data, cannot be more than a full Game Boy screen at once
char png_storage_file_name[64];                           //character string to store images
char tmp_storage_file_name[64];                           //character string to store images
unsigned char inner_palette;                              //inner palette to use for core 1
unsigned char inner_lower_margin;                         //inner margin to use for core 1
//This array contains preformated pixels for 2bbp png mode, 4 pixels per bytes, assuming a 4x upscaling factor and so 4 consecutive pixels identical stored per bytes
unsigned char PNG_compress_4x[4] = { 0b00000000, 0b01010101, 0b10101010, 0b11111111 };  //lookup table for PNG 2 bpp format. 1 byte = 4 identical pixels on a line
//unsigned char PNG_palette_RGB[12] = { 123, 129, 16, 89, 121, 66, 73, 90, 40, 46, 69, 54 };//DMG palette
//unsigned char PNG_palette_RGB[12] = { 197, 202, 165, 140, 146, 107, 105, 108, 90, 24, 24, 24 };//GBpocket palette
//unsigned char PNG_palette_RGB[12] = { 255, 255, 255, 127, 253, 55, 0, 100, 198, 0, 0, 0 };//GBcolor palette
unsigned char PNG_palette_RGB[12] = { 0xFF, 0xFF, 0xFF,
                                      0xAA, 0xAA, 0xAA,
                                      0x55, 0x55, 0x55,
                                      0x00, 0x00, 0x00 };  //RGB colors as they will appear in the PNG file
unsigned char image_palette[4];                            //2 bpp local color palette sent by the Game Boy
unsigned char DATA_packet_counter = 0;                     //counter for packets transmitted
unsigned char DATA_packet_to_print = 0;                    //counter for packets transmitted for core 1
unsigned char local_byte_LSB = 0;                          //storage byte for conversion
unsigned char local_byte_MSB = 0;                          //storage byte for conversion
unsigned char pixel_level = 0;                             //storage byte for conversion
unsigned int DATA_bytes_counter = 0;                       //counter for data bytes
unsigned int IMAGE_bytes_counter = 0;                      //counter for data bytes
unsigned int tile_column, tile_line, pixel_line = 0;       //storage variables for conversion
unsigned int offset_x = 0;                                 //local variable for decoder
unsigned int max_tile_line = 0;                            //local variable for decoder
unsigned int max_pixel_line = 0;                           //local variable for decoder
unsigned int max_files_per_folder = 1024;                  //FAT32 limits the number of entries, so better be carefull
unsigned long lines_in_image_file = 0;                     //to keep tack of image file length
unsigned long myTime;                                      //timer for PNG encoder
unsigned long FILE_number = 0;                             //counter of file per session
bool SDcard_READY = 0;                                     //self explanatory
bool PRINT_flag = 0;                                       //self explanatory
bool TEAR_mode = 0;                                        //self explanatory
bool skip_byte_on_display = 1;                             //renders the serial less verbose
bool BITBOY_mode = 0;                                      //mimicks the folder strategy of the BitBoy (100 files per folder)