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
#define NUMPIXELS 1  // NeoPixel ring size (just internal LED here)

unsigned int Next_ID, Next_dir;                       //for directories and filenames
unsigned char printer_memory_buffer_core_0[9 * 640];  //Game Boy printer buffer of 9*640 bytes (maximum possible), core 0
unsigned char printer_memory_buffer_core_1[9 * 640];  //Game Boy printer buffer of 9*640 bytes (maximum possible), core 1
unsigned char PNG_image_color[144 * 160];             //"color" RGB 2bbp data
char png_storage_file_name[64];                       //character string to store images
char tmp_storage_file_name[64];                       //character string to store images
unsigned char inner_palette;                          //inner palette to use for core 1
unsigned char inner_lower_margin;                     //inner margin to use for core 1
//This array contains preformated pixels for 2bbp png mode, 4 pixels per bytes, assuming a 4x upscaling factor and so 4 consecutive pixels identical stored per bytes
unsigned char PNG_compress_4x[4] = { 0b00000000, 0b01010101, 0b10101010, 0b11111111 };                        //lookup table for PNG 2 bpp format. 1 byte = 4 identical pixels on a line
//unsigned char PNG_palette_RGB[12] = { 123, 129, 16, 89, 121, 66, 73, 90, 40, 46, 69, 54 };//DMG palette
//unsigned char PNG_palette_RGB[12] = { 197, 202, 165, 140, 146, 107, 105, 108, 90, 24, 24, 24 };//GBpocket palette
//unsigned char PNG_palette_RGB[12] = { 255, 255, 57, 129, 35, 00, 29, 245, 90, 00, 00, 00 };//GBcolor palette
unsigned char PNG_palette_RGB[12] = { 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x00, 0x00, 0x00 };  //RGB colors as they will appear in the PNG file
unsigned char image_palette[4];                       //2 bpp local color palette sent by the Game Boy
unsigned char DATA_packet_counter = 0;                //counter for packets transmitted
unsigned char DATA_packet_to_print = 0;               //counter for packets transmitted for core 1
unsigned char local_byte_LSB = 0;                     //storage byte for conversion
unsigned char local_byte_MSB = 0;                     //storage byte for conversion
unsigned char pixel_level = 0;                        //storage byte for conversion
unsigned int DATA_bytes_counter = 0;                  //counter for data bytes
unsigned int IMAGE_bytes_counter = 0;                 //counter for data bytes
unsigned int tile_column, tile_line, pixel_line = 0;  //storage variables for conversion
unsigned int offset_x = 0;                            //local variable for decoder
unsigned int max_tile_line = 0;                       //local variable for decoder
unsigned int max_pixel_line = 0;                      //local variable for decoder
unsigned int max_files_per_folder = 1024;             //FAT32 limits the number of entries, so better be carefull
unsigned long lines_in_image_file = 0;                //to keep tack of image file length
unsigned long myTime;                                 //timer for PNG encoder
unsigned long FILE_number = 0;                        //counter of file per session
uint8_t intensity = 30;                               //WS2812 intensity 255 is a death ray, 10 to 15 is normal
uint32_t SD_card_access_Color;                        //mandatory structure for the WS2812 LED
bool SDcard_READY = 0;                                //self explanatory
bool PRINT_flag = 0;                                  //self explanatory
bool TEAR_mode = 0;                                   //self explanatory
bool BREAK_flag = 0;                                  //detects a broken packet, suicides the whole print
bool skip_byte_on_display = 1;                        //renders the serial less verbose
//////////////////////////////////////////////SD stuff///////////////////////////////////////////////////////////////////////////////////////////
void ID_file_creator(const char* path) {  //from fresh SD, device needs a "secret" binary storage file
                                          //this file may never be erased and is accessed frequently as it counts all images recorded with a unique ID
  uint8_t buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  if (!SD.exists(path)) {  //start from a fresh install on SD
    File Datafile = SD.open(path, FILE_WRITE);
    Datafile.write(buf, 8);
    Datafile.close();
    Serial.println("// Creating a new configuration file");
  } else {
    Serial.println("// Configuration file yet existing");
  }
}

unsigned long get_next_ID(const char* path) {  //get the next file #
  uint8_t buf[4];
  File Datafile = SD.open(path);
  Datafile.read(buf, 4);
  Next_ID = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]));
  Datafile.close();
  return Next_ID;
}

unsigned long get_next_dir(const char* path) {  //get the next directory #
  uint8_t buf[4];
  File Datafile = SD.open(path);
  Datafile.read(buf, 4);  //dumps the 4 first bytes
  Datafile.read(buf, 4);
  Next_dir = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]));
  Datafile.close();
  return Next_dir;
}

void store_next_ID(const char* path, unsigned long Next_ID, unsigned long Next_dir) {  //store current file # and directory #
  uint8_t buf[4];
  File Datafile = SD.open(path, FILE_WRITE);
  Datafile.seek(0);
  buf[3] = Next_ID >> 0;
  buf[2] = Next_ID >> 8;
  buf[1] = Next_ID >> 16;
  buf[0] = Next_ID >> 24;
  Datafile.write(buf, 4);
  buf[3] = Next_dir >> 0;
  buf[2] = Next_dir >> 8;
  buf[1] = Next_dir >> 16;
  buf[0] = Next_dir >> 24;
  Datafile.write(buf, 4);
  Datafile.close();
}
//////////////////////////////////////////////SD stuff///////////////////////////////////////////////////////////////////////////////////////////
