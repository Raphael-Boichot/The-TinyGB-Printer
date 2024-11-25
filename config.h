
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

#define SD_MISO 8      // SD card SPI1
#define SD_CS 9        // SD card SPI1
#define SD_SCK 10      // SD card SPI1
#define SD_MOSI 11     // SD card SPI1
#define BTN_PUSH 12    // Define a PushButton to use to Force a new file in idle mode ///BOICHOT
#define NUMPIXELS 1    // NeoPixel ring size (just internal LED here)

unsigned char B = 0x00;                               //palette Black
unsigned char DG = 0x55;                              //palette Dark Gray
unsigned char LG = 0xAA;                              //palette Light Gray
unsigned char W = 0xFF;                               //palette White
unsigned int PNG_upscaling_factor = 4;                //for png encoder, MUST BE 4 at the moment
unsigned int Next_ID, Next_dir;                       //for directories and filenames
unsigned char printer_memory_buffer_core_0[9 * 640];  //Game Boy printer buffer of 9*640 bytes (maximum possible), core 0
unsigned char printer_memory_buffer_core_1[9 * 640];  //Game Boy printer buffer of 9*640 bytes (maximum possible), core 1
unsigned char BMP_image_color[144 * 160];             //color RGB image for BMP, real color known from palette (maximum possible), core 1
char png_storage_file_name[64];                       //character string to store images
char tmp_storage_file_name[64];                       //character string to store images
unsigned char inner_palette;                          //inner palette to use for core 1
unsigned char inner_lower_margin;                     //inner margin to use for core 1
//This array contains preformated pixels for 2bbp png mode, 4 pixels per bytes, assuming a 4x upscaling factor and so 4 consecutive pixels identical stored per bytes
unsigned char PNG_compress_4x[4] = { 0b00000000, 0b01010101, 0b10101010, 0b11111111};// this is fortuitely the inverse of the image palette ^_^
unsigned char PNG_palette[4] = { W, LG, DG, B };
//unsigned char BMP_palette[4] = { W, LG, DG, B };      //colors as they will appear in the bmp file and display after dithering
unsigned char image_palette[4] = { 0, 0, 0, 0 };      //2 bpp colors refering to BMP_palette[4]
unsigned int DATA_bytes_counter = 0;                  //counter for data bytes
unsigned int BMP_bytes_counter = 0;                   //counter for data bytes
unsigned int offset_x = 0;                            //offsets for tile conversion in BMP file
unsigned char DATA_packet_counter = 0;                //counter for packets transmitted
unsigned char DATA_packet_to_print = 0;               //counter for packets transmitted for core 1
unsigned char local_byte_LSB = 0;                     //storage byte for conversion
unsigned char local_byte_MSB = 0;                     //storage byte for conversion
unsigned int tile_column, tile_line, pixel_line = 0;  //storage byte for conversion
unsigned char pixel_level = 0;                        //storage byte for conversion
unsigned long lines_in_image_file = 0;                  //to keep tack of image file length
uint8_t intensity = 150;                              //WS2812 intensity 255 is a death ray, 10 to 15 is normal
uint32_t SD_card_access_Color;
uint32_t BMP_decoder_color;
uint32_t PNG_decoder_color;
bool SDcard_READY = 0;
bool PRINT_flag = 0;
bool TEAR_mode = 0;

//////////////////////////////////////////////SD stuff///////////////////////////////////////////////////////////////////////////////////////////
void ID_file_creator(const char* path) {  //from fresh SD, device needs a "secret" binary storage file
                                          //this file may never be erased and is accessed frequently as it counts all images recorded with a unique ID
  uint8_t buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  if (!SD.exists(path)) {
    File Datafile = SD.open(path, FILE_WRITE);
    //start from a fresh install on SD
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
