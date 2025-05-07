/*************************************************************************
 *
 * GAMEBOY PRINTER EMULATION PROJECT V3.2.1 (Arduino)
 * Copyright (C) 2022 Brian Khuu
 *
 * PURPOSE: To capture gameboy printer images without a gameboy printer
 *          via the arduino platform. (Tested on the arduino nano)
 *          This version is to investigate gameboy behaviour.
 *          This was originally started on 2017-4-6 but updated on 2020-08-16
 * LICENCE:
 *   This file is part of Arduino Gameboy Printer Emulator.
 *
 *   Arduino Gameboy Printer Emulator is free software:
 *   you can redistribute it and/or modify it under the terms of the
 *   GNU General Public License as published by the Free Software Foundation,
 *   either version 3 of the License, or (at your option) any later version.
 *
 *   Arduino Gameboy Printer Emulator is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Arduino Gameboy Printer Emulator.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include "gameboy_printer_protocol.h"
#include "gbp_serial_io.h"

/////////////Specific to TinyGB Printer//////////////
#include <hardware/gpio.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>  //for SD
#include <SD.h>   //for SD
#include "Upscalerlib.h"
#include "config.h"
#include "splash.h"
#include <TFT_eSPI.h>  // Hardware-specific library
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();            // Invoke custom library
TFT_eSprite img = TFT_eSprite(&tft);  // Create Sprite object "img" with pointer to "tft" object
                                      // the pointer is used by pushSprite() to push it onto the TFT
//Please read this topic: https://github.com/Bodmer/TFT_eSPI/issues/3476
//in TFT_eSPI_RP2040.h, you have to comment a command to allow this particular 240x240 display to work :
//#define SET_BUS_READ_MODE  spi_set_format(SPI_X,  8, (spi_cpol_t)0, (spi_cpha_t)0, SPI_MSB_FIRST)
//must be replaced by:
//#define SET_BUS_READ_MODE  // spi_set_format(SPI_X,  8, (spi_cpol_t)0, (spi_cpha_t)0, SPI_MSB_FIRST)
//It's a trap for who never reads the code comments ^^
/////////////Specific to TinyGB Printer//////////////

#define GBP_BUFFER_SIZE 650  //maximal size of data buffer 640 bytes + commands
#define GBP_FEATURE_PARSE_PACKET_MODE
#define GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
#include "gbp_pkt.h"

/*******************************************************************************
*******************************************************************************/
/* Serial IO */
// This circular buffer contains a stream of raw packets from the gameboy
uint8_t gbp_serialIO_raw_buffer[GBP_BUFFER_SIZE] = { 0 };

#ifdef GBP_FEATURE_PARSE_PACKET_MODE
/* Packet Buffer */
gbp_pkt_t gbp_pktState = { GBP_REC_NONE, 0 };
uint8_t gbp_pktbuff[GBP_PKT_PAYLOAD_BUFF_SIZE_IN_BYTE] = { 0 };
uint8_t gbp_pktbuffSize = 0;
#ifdef GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
gbp_pkt_tileAcc_t tileBuff = { 0 };
#endif
#endif

#ifdef GBP_FEATURE_PARSE_PACKET_MODE
inline void gbp_parse_packet_loop();
#endif

/*******************************************************************************
  Utility Functions
*******************************************************************************/

const char* gbpCommand_toStr(int val) {
  switch (val) {
    case GBP_COMMAND_INIT: return "INIT";
    case GBP_COMMAND_PRINT: return "PRNT";
    case GBP_COMMAND_DATA: return "DATA";
    case GBP_COMMAND_BREAK: return "BREK";
    case GBP_COMMAND_INQUIRY: return "INQY";
    default: return "?";
  }
}

/*******************************************************************************
  Interrupt Service Routine using direct GPIO
*******************************************************************************/
void gpio_irq_callback(uint gpio, uint32_t events) {
  if (gpio == GBP_SC_PIN) {
    bool so_bit = gpio_get(GBP_SO_PIN);  // Read the bit from the master (SO = Serial Output from Master)
    bool tx_bit;
#ifdef GBP_FEATURE_USING_RISING_CLOCK_ONLY_ISR
    tx_bit = gpb_serial_io_OnRising_ISR(so_bit);
#else
    bool sc_level = gpio_get(GBP_SC_PIN);
    tx_bit = gpb_serial_io_OnChange_ISR(sc_level, so_bit);
#endif
    gpio_put(GBP_SI_PIN, tx_bit);  // Write the response bit back to the master
  }
}

/*******************************************************************************
  Main Setup and Loop
*******************************************************************************/

void setup(void) {
#ifdef DEBUG_MODE
  Serial.begin(115200);
#endif
  Tiny_printer_preparation();  //switches in Tiny Printer mode
  gpio_init(BTN_PUSH);         // Configure BTN_PUSH as input
  gpio_set_dir(BTN_PUSH, GPIO_IN);
  gpio_init(GBP_SC_PIN);  // Configure SC and SO as inputs too
  gpio_set_dir(GBP_SC_PIN, GPIO_IN);
  gpio_init(GBP_SO_PIN);
  gpio_set_dir(GBP_SO_PIN, GPIO_IN);
  gpio_init(GBP_SI_PIN);  // Configure SI as output
  gpio_set_dir(GBP_SI_PIN, GPIO_OUT);
  gpio_put(GBP_SI_PIN, 0);  // start LOW
  LED_WS2812_state(WS2812_Idle, 1);

  /* Setup */
  gpb_serial_io_init(sizeof(gbp_serialIO_raw_buffer), gbp_serialIO_raw_buffer);

  /* Attach ISR / Enable GPIO interrupt with callback */
#ifdef GBP_FEATURE_USING_RISING_CLOCK_ONLY_ISR
  gpio_set_irq_enabled_with_callback(GBP_SC_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_irq_callback);
#else
  gpio_set_irq_enabled_with_callback(GBP_SC_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);
#endif

  /* Packet Parser */
#ifdef GBP_FEATURE_PARSE_PACKET_MODE
  gbp_pkt_init(&gbp_pktState);
#endif

#define VERSION_STRING "V3.2.1 (Copyright (C) 2022 Brian KHUU)"
#define TINY_VERSION_STRING "V2.0.3 (Copyright (C) 2024 Raphaël BOICHOT)"
#ifdef DEBUG_MODE
  Serial.println(F("// Game Boy Printer Emulator for Arduino " VERSION_STRING));
  Serial.println(F("// TinyGB Printer converter add-on " TINY_VERSION_STRING));
  Serial.flush();
#endif
}  // setup()

void loop() {
  static uint16_t sioWaterline = 0;

#ifdef GBP_FEATURE_PARSE_PACKET_MODE
  gbp_parse_packet_loop();
#endif

  // Trigger Timeout and reset the printer if byte stopped being received.
  static uint32_t last_millis = 0;
  uint32_t curr_millis = millis();
  if (curr_millis > last_millis) {
    uint32_t elapsed_ms = curr_millis - last_millis;
    if (gbp_serial_io_timeout_handler(elapsed_ms)) {
#ifdef DEBUG_MODE
      Serial.print("Core 0 -> ");
      Serial.print("Completed ");
      Serial.print("(Memory Waterline: ");
      Serial.print(gbp_serial_io_dataBuff_waterline(false));
      Serial.print("B out of ");
      Serial.print(gbp_serial_io_dataBuff_max());
      Serial.println("B)");
#endif
      gbp_pkt_reset(&gbp_pktState);
      tileBuff.count = 0;
      /////////////Specific to TinyGB Printer//////////////
      //incomplete packet due to abort command or transmission stopped inbetween DATA packets
      if (!(DATA_bytes_counter % 640 == 0) | (gbp_pktState.command == GBP_COMMAND_DATA)) {
#ifdef DEBUG_MODE
        Serial.println("Core 0 -> Incomplete packet detected due to abort command, flush printer");
#endif
        SD.remove(tmp_storage_file_name);  //remove any previous failed attempt
        lines_in_image_file = 0;           //resets the number of lines
        DATA_bytes_counter = 0;            //reset
        DATA_packet_counter = 0;           //reset
        DATA_packet_to_print = 0;          //reset
        LED_WS2812_state(WS2812_Idle, 1);
      }
      /////////////Specific to TinyGB Printer//////////////
      Serial.flush();
    }
  }
  last_millis = curr_millis;
}  // loop()

/////////////Specific to TinyGB Printer//////////////
void loop1()  //core 1 loop deals with images, written by Raphaël BOICHOT, november 2024
{
  if (PRINT_flag == 1) {
    PRINT_flag = 0;
    if (DATA_bytes_to_print == 320) {  //fix for Blarble 1290 homebrew, only game using "half packets", not supported by default
      memcpy(printer_memory_buffer_core_1, printer_memory_buffer_core_0, 320);
    } else {                                                                                           //regular game, packets are 640 bytes long, 40 tiles, 16 pixels height
      memcpy(printer_memory_buffer_core_1, printer_memory_buffer_core_0, 640 * DATA_packet_to_print);  //this can also be done by core 0
    }
    LED_WS2812_state(WS2812_Color, 1);
    if (inner_palette == 0x00) {  //4 games uses this palette
      inner_palette = 0xE4;       //see Game Boy Programming manual, palette 0x00 is a default palette interpreted as 0xE4 or 0b11100100
#ifdef DEBUG_MODE
      Serial.println("Core 1 -> Palette 0x00, fixed to 0xE4");
#endif
    }
    if (inner_palette == 0xE1) {  //1 game uses this wrong palette
      inner_palette = 0xD2;       //palette fix for Disney's Tarzan, inverts DG and LG
#ifdef DEBUG_MODE
      Serial.println("Core 1 -> Palette 0xE1 (Disney's Tarzan), fixed to 0xD2");
#endif
    }
    if (inner_palette == 0x2D) {  //1 game uses this wrong palette
      inner_palette = 0x1E;       //palette fix for Trade & Battle: Card Hero, inverts DG and LG. Text will become hard to read but at least creatures are well depicted
#ifdef DEBUG_MODE
      Serial.println("Core 1 -> Palette 0x2D (Trade & Battle: Card Hero), fixed to 0xE1");
#endif
    }
    //0xE4 = 0b11100100 = 3-2-1-0 intensity of colors for printer (3 = black, 0 = white), for Game Boy pixel encoded values 0-1-2-3
    image_palette[0] = bitRead(inner_palette, 0) + 2 * bitRead(inner_palette, 1);
    image_palette[1] = bitRead(inner_palette, 2) + 2 * bitRead(inner_palette, 3);
    image_palette[2] = bitRead(inner_palette, 4) + 2 * bitRead(inner_palette, 5);
    image_palette[3] = bitRead(inner_palette, 6) + 2 * bitRead(inner_palette, 7);

#ifdef DEBUG_MODE
    Serial.print("Core 1 -> I received ");
    Serial.print(DATA_packet_to_print, DEC);
    Serial.print(" packets to print with palette ");
    Serial.print(inner_palette, HEX);
    Serial.print(" and ");
    Serial.print(inner_lower_margin, DEC);
    Serial.println(" after margin");
#endif

    //All the meat to decode the 2bpp Game Boy Tile Format is explained here (among other sources): https://www.huderlem.com/demos/gameboy2bpp.html
    //the image data are a simple one dimensional array because there is no gain to have a 2D array, in particular when burning data to SD card
    //the issue here is to transform a tile-based system (8*8 pixels) to a pixel-based system in lines/columns.
    //we convert image as a whole, not packet per packet (no need)
    IMAGE_bytes_counter = 0;
    pixel_line = 0;
    offset_x = 0;

    if (DATA_bytes_to_print == 320) {  //fix for Blarble 1290 homebrew, only game using "half packets", not supported by default
      max_tile_line = 1;               //a DATA packet is 2 tiles high
      max_pixel_line = 8;              //a DATA packet is 16 pixel high
    } else {
      max_tile_line = DATA_packet_to_print * 2;    //a DATA packet is 2 tiles high
      max_pixel_line = DATA_packet_to_print * 16;  //a DATA packet is 16 pixel high
    }

    for (tile_line = 0; tile_line < max_tile_line; tile_line++) {                  //this part fills 8 lines of pixels
      IMAGE_bytes_counter = 16 * 20 * tile_line;                                   //a tile is 16 bytes, a line screen is 20 tiles (160 pixels width)
      for (int i = 0; i < 8; i++) {                                                // This part fills a line of pixels
        offset_x = pixel_line * 160;                                               //x stands for the position in the image vector containing compressed data
        for (tile_column = 0; tile_column < 20; tile_column++) {                   //we progress along 20 column tiles
          local_byte_LSB = printer_memory_buffer_core_1[IMAGE_bytes_counter];      //here we get data for a line of 8 pixels (2 bytes)
          local_byte_MSB = printer_memory_buffer_core_1[IMAGE_bytes_counter + 1];  //here we get data for a line of 8 pixels
          for (int posx = 0; posx < 8; posx++) {
            pixel_level = bitRead(local_byte_LSB, 7 - posx) + 2 * bitRead(local_byte_MSB, 7 - posx);  //here we get pixel value along 8 pixels horizontally
            PNG_image_color[offset_x + posx] = PNG_compress_4x[image_palette[pixel_level]];           //here we store 4 2bbp pixels per byte for next step (2bpp indexed png upscaler)
          }                                                                                           //this is a bit aggressive as pixel decoder and PNG compression is within the same line of code, but efficient
          IMAGE_bytes_counter = IMAGE_bytes_counter + 16;                                             //jumps to the next tile in byte
          offset_x = offset_x + 8;                                                                    //jumps to the next tile in pixels
        }                                                                                             //This part fills a line of pixels
        IMAGE_bytes_counter = IMAGE_bytes_counter - 16 * 20 + 2;                                      //shifts to the next two bytes among 16 per tile, so the next line of pixels in a tile
        pixel_line = pixel_line + 1;                                                                  //jumps to the next line
      }                                                                                               //This part fills 8 lines of pixels
    }                                                                                                 //this part fills the entire image

    if ((TEAR_mode == 0) && (inner_lower_margin > 0)) {  //just to make a fancy display with clear image separation
      max_pixel_line = max_pixel_line + TFT_offset;
    }

    //the TFT diplay can hold 15 data packets simultaneously
    //Step 1: copy previous data with a vertical upper translation corresponding to future printing area
    for (int x = 0; x < 160; x++) {
      for (int y = max_pixel_line; y < 240; y++) {
        TFT_memory_buffer[x + (y - max_pixel_line) * 160] = TFT_memory_buffer[x + y * 160];
      }
    }
    //Step 2: print the new image data after the previous ones at the bottom of display area
    for (int x = 0; x < 160; x++) {
      for (int y = 0; y < max_pixel_line; y++) {
        TFT_memory_buffer[x + (240 - max_pixel_line + y) * 160] = 0xFF - PNG_image_color[x + y * 160];  //color are inverter compared to the PNG format
      }
    }

    if ((TEAR_mode == 0) && (inner_lower_margin > 0)) {  //just to make a fancy display with clear image separation
      //Step 3: overwite the end of buffer with cool pixels
      for (int x = 0; x < 160; x++) {
        for (int y = 0; y < TFT_offset; y++) {
          TFT_memory_buffer[x + (y + 240 - TFT_offset) * 160] = tearsymbol[x + y * 160];
        }
      }
    }

    //converts tft buffer into a giant sprite covering a whole strip
    for (int x = 0; x < 160; x++) {
      for (int y = 0; y < 240; y++) {
        img.drawPixel(x, y, lookup_TFT_RGB565[TFT_memory_buffer[x + y * 160]]);
      }
    }
    img.pushSprite(x_ori, 0);  //dump image to display

    File Datafile = SD.open(tmp_storage_file_name, FILE_WRITE);  //in any case, if PRINT is received, write to a file (yet existing or not)
    if (DATA_bytes_to_print == 320) {                            //fix for Blarble 1290 homebrew, only game using "half packets", not supported by default
      Datafile.write(PNG_image_color, 160 * 8);                  //writes half the data to SD card
    } else {
      Datafile.write(PNG_image_color, 160 * 16 * DATA_packet_to_print);  //writes the data to SD card
    }
    Datafile.close();

    if (DATA_bytes_to_print == 320) {                 //fix for Blarble 1290 homebrew, only game using "half packets", not supported by default
      lines_in_image_file = lines_in_image_file + 8;  //keep track of the number of lines stored
    } else {
      lines_in_image_file = lines_in_image_file + 16 * DATA_packet_to_print;  //keep track of the number of lines stored
    }
    DATA_packet_to_print = 0;

    if ((inner_lower_margin > 0) & (TEAR_mode == 0)) {  //the printer asks to feed paper, end of file, except in TEAR mode
      Next_ID++;                                        //increment file number
      store_next_ID("/tiny.sys", Next_ID, Next_dir);
      sprintf(png_storage_file_name, "/%05d/%07d.png", Next_dir, Next_ID);
#ifdef DEBUG_MODE
      Serial.print("Core 1 -> Encoding ");
      Serial.print(png_storage_file_name);
      Serial.print(" due to feed paper signal, with ");
      Serial.print(lines_in_image_file, DEC);
      Serial.println(" lines in image file");
#endif
      myTime = millis();
      png_upscaler(tmp_storage_file_name, png_storage_file_name, PNG_palette_RGB, lines_in_image_file);
#ifdef DEBUG_MODE
      Serial.print("Core 1 -> PNG file closed, encoding time (ms): ");
      Serial.println(millis() - myTime, DEC);
#endif
      FILE_number = FILE_number + 1;
#ifdef DEBUG_MODE
      Serial.print("Core 1 -> Number of file for this session: ");
      Serial.println(FILE_number, DEC);
#endif
      if (BITBOY_mode == 0) {  // in this mode the folders are incremented at each boot
        if (FILE_number % max_files_per_folder == 0) {
          Next_dir++;
          store_next_ID("/tiny.sys", Next_ID, Next_dir);  //store next folder #immediately
        }
      } else {  // in this mode the folders must contain 100 files
        if ((Next_ID % 100) == 0) {
          Next_dir++;
          store_next_ID("/tiny.sys", Next_ID, Next_dir);  //store next folder #immediately
        }
      }

      lines_in_image_file = 0;           //resets the number of lines
      SD.remove(tmp_storage_file_name);  //a bit aggressive and maybe not optimal but I'm sure the old data disappears
    }
    LED_WS2812_state(WS2812_Idle, 1);
  }

  //in TEAR mode, a file is never closed unless you push a button
  if ((TEAR_mode == 1) & (gpio_get(BTN_PUSH)) & (lines_in_image_file > 0)) {  //in tear mode, a button push only can close file, whatever the printer (non empty) state
    LED_WS2812_state(WS2812_Color, 1);
    Next_ID++;  //increment file number
    store_next_ID("/tiny.sys", Next_ID, Next_dir);
    sprintf(png_storage_file_name, "/%05d/%07d.png", Next_dir, Next_ID);
#ifdef DEBUG_MODE
    Serial.print("Core 1 -> Encoding ");
    Serial.print(png_storage_file_name);
    Serial.print(" due to push button, with ");
    Serial.print(lines_in_image_file, DEC);
    Serial.println(" lines in image file");
#endif
    myTime = millis();
    png_upscaler(tmp_storage_file_name, png_storage_file_name, PNG_palette_RGB, lines_in_image_file);
#ifdef DEBUG_MODE
    Serial.print("Core 1 -> PNG file closed, encoding time (ms): ");
    Serial.println(millis() - myTime, DEC);
#endif
    FILE_number = FILE_number + 1;
#ifdef DEBUG_MODE
    Serial.print("Core 1 -> Number of file for this session: ");
    Serial.println(FILE_number, DEC);
#endif

    if (BITBOY_mode == 0) {
      if (FILE_number % max_files_per_folder == 0) {
        Next_dir++;
        store_next_ID("/tiny.sys", Next_ID, Next_dir);  //store next folder #immediately
      }
    } else {
      if ((Next_ID % 100) == 0) {
        Next_dir++;
        store_next_ID("/tiny.sys", Next_ID, Next_dir);  //store next folder #immediately
      }
    }

    lines_in_image_file = 0;           //resets the number of lines
    SD.remove(tmp_storage_file_name);  //a bit aggressive and maybe not optmal but I'm sure the old data disappears

    memset(TFT_memory_buffer, 255, sizeof(TFT_memory_buffer));  //clears the image buffer
    img.fillScreen(TFT_WHITE);                                  //clears the display
    img.pushSprite(x_ori, 0);                                   //dumps white image to display

    LED_WS2812_state(WS2812_Idle, 1);
  }
}  // loop1()
/////////////Specific to TinyGB Printer//////////////

/******************************************************************************/
inline void gbp_parse_packet_loop(void) {
  const char nibbleToCharLUT[] = "0123456789ABCDEF";
  for (int i = 0; i < gbp_serial_io_dataBuff_getByteCount(); i++) {
    if (gbp_pkt_processByte(&gbp_pktState, (const uint8_t)gbp_serial_io_dataBuff_getByte(), gbp_pktbuff, &gbp_pktbuffSize, sizeof(gbp_pktbuff))) {
      if (gbp_pktState.received == GBP_REC_GOT_PACKET) {
#ifdef DEBUG_MODE
        Serial.print("Core 0 -> ");
        Serial.print((char)'{');
        Serial.print("\"command\":\"");
        Serial.print(gbpCommand_toStr(gbp_pktState.command));
        Serial.print("\"");
#endif
        if (!skip_byte_on_display) {
          if (gbp_pktState.command == GBP_COMMAND_INQUIRY) {
#ifdef DEBUG_MODE
            Serial.print(", \"status\":{");
            Serial.print("\"LowBat\":");
            Serial.print(gpb_status_bit_getbit_low_battery(gbp_pktState.status) ? '1' : '0');
            Serial.print(",\"ER2\":");
            Serial.print(gpb_status_bit_getbit_other_error(gbp_pktState.status) ? '1' : '0');
            Serial.print(",\"ER1\":");
            Serial.print(gpb_status_bit_getbit_paper_jam(gbp_pktState.status) ? '1' : '0');
            Serial.print(",\"ER0\":");
            Serial.print(gpb_status_bit_getbit_packet_error(gbp_pktState.status) ? '1' : '0');
            Serial.print(",\"Untran\":");
            Serial.print(gpb_status_bit_getbit_unprocessed_data(gbp_pktState.status) ? '1' : '0');
            Serial.print(",\"Full\":");
            Serial.print(gpb_status_bit_getbit_print_buffer_full(gbp_pktState.status) ? '1' : '0');
            Serial.print(",\"Busy\":");
            Serial.print(gpb_status_bit_getbit_printer_busy(gbp_pktState.status) ? '1' : '0');
            Serial.print(",\"Sum\":");
            Serial.print(gpb_status_bit_getbit_checksum_error(gbp_pktState.status) ? '1' : '0');
            Serial.print((char)'}');
#endif
          }
        }

        if (gbp_pktState.command == GBP_COMMAND_PRINT) {
          //!{"command":"PRNT","sheets":1,"margin_upper":1,"margin_lower":3,"pallet":228,"density":64 }
#ifdef DEBUG_MODE
          Serial.print(", \"sheets\":");
          Serial.print(gbp_pkt_printInstruction_num_of_sheets(gbp_pktbuff));
          Serial.print(", \"margin_upper\":");
          Serial.print(gbp_pkt_printInstruction_num_of_linefeed_before_print(gbp_pktbuff));
          Serial.print(", \"margin_lower\":");
          Serial.print(gbp_pkt_printInstruction_num_of_linefeed_after_print(gbp_pktbuff));
          Serial.print(", \"pallet\":");
          Serial.print(gbp_pkt_printInstruction_palette_value(gbp_pktbuff));
          Serial.print(", \"density\":");
          Serial.print(gbp_pkt_printInstruction_print_density(gbp_pktbuff));
#endif
          ///////////////////////specific to the TinyGB Printer////////////////////////
          DATA_packet_to_print = DATA_packet_counter;
          DATA_bytes_to_print = DATA_bytes_counter;                                                //to detect anormal packets not 640 bytes long
          inner_palette = gbp_pkt_printInstruction_palette_value(gbp_pktbuff);                     //this can also be done by core 1
          inner_lower_margin = gbp_pkt_printInstruction_num_of_linefeed_after_print(gbp_pktbuff);  //this can also be done by core 1
          DATA_bytes_counter = 0;                                                                  //counter for data bytes
          DATA_packet_counter = 0;                                                                 //counter for packets transmitted
          PRINT_flag = 1;                                                                          //triggers stuff on core 1, from now core 1 have plenty of time to convert image
          ///////////////////////specific to the TinyGB Printer////////////////////////
        }

        if (gbp_pktState.command == GBP_COMMAND_DATA) {
          LED_WS2812_state(WS2812_Color, 1);

#ifdef GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
#ifdef DEBUG_MODE
          Serial.print(", \"compressed\":0");  // Already decompressed by us, so no need to do so
#endif
#else
#ifdef DEBUG_MODE
          Serial.print(", \"compressed\":");
          Serial.print(gbp_pktState.compression);
#endif
#endif
#ifdef DEBUG_MODE
          Serial.print(", \"more\":");
          Serial.print((gbp_pktState.dataLength != 0) ? '1' : '0');
#endif
          LED_WS2812_state(WS2812_Color, 0);
        }
#ifdef DEBUG_MODE
        Serial.println((char)'}');
#endif
        ///////////////////////specific to the TinyGB Printer////////////////////////
        if (gbp_pktState.command == GBP_COMMAND_BREAK) {
#ifdef DEBUG_MODE
          Serial.println("Core 0 -> BREAK command detected, flush printer");
#endif
          SD.remove(tmp_storage_file_name);  //remove any previous failed attempt
          lines_in_image_file = 0;           //resets the number of lines
          DATA_bytes_counter = 0;            //reset
          DATA_packet_counter = 0;           //reset
          DATA_packet_to_print = 0;          //reset
          LED_WS2812_state(WS2812_Idle, 1);
        }
        ///////////////////////specific to the TinyGB Printer////////////////////////

        Serial.flush();
      } else {
#ifdef GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
        // Required for more complex games with compression support
        while (gbp_pkt_decompressor(&gbp_pktState, gbp_pktbuff, gbp_pktbuffSize, &tileBuff)) {
          if (gbp_pkt_tileAccu_tileReadyCheck(&tileBuff)) {
            // Got Tile
            for (int i = 0; i < GBP_TILE_SIZE_IN_BYTE; i++) {
              const uint8_t data_8bit = tileBuff.tile[i];

              if (!skip_byte_on_display) {
                if (i == GBP_TILE_SIZE_IN_BYTE - 1) {
#ifdef DEBUG_MODE
                  Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
                  Serial.println((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);  // use println on last byte to reduce serial calls
#endif
                } else {
#ifdef DEBUG_MODE
                  Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
                  Serial.print((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);
                  Serial.print((char)' ');
#endif
                }
              }

              ///////////////////////specific to the TinyGB Printer////////////////////////
              printer_memory_buffer_core_0[DATA_bytes_counter] = data_8bit;  //we get the graphic data here
              DATA_bytes_counter++;
              if (DATA_bytes_counter % 640 == 0) {  //we count the data packets here (16 bytes*40 tiles)
                DATA_packet_counter++;
              }
              if (DATA_bytes_counter > 9 * 640) {  //overflow protection, image will be glitched at worse but device continues to run.
                DATA_bytes_counter = 0;            //buffer loops on itself
                DATA_packet_counter = 0;           //data are sacrified to avoid crash
              }                                    //this overflow can happen with fast protocol (multi-print) when SD card is full like an egg. Commands may be missed. Results becomes unpredictable and the Pico crashes
              ///////////////////////specific to the TinyGB Printer////////////////////////
            }
            Serial.flush();
          }
        }
#endif
      }
    }
  }
}

void Tiny_printer_preparation() {
  //Set up the display
  tft.init();
  tft.setTextSize(2);
  img.setColorDepth(BITS_PER_PIXEL);  // Set colour depth first
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  if (gpio_get(BTN_PUSH)) {
    WS2812_Color = pixels.Color(0, 0, intensity);  //RGB triplet, turn to blue
    TEAR_mode = 1;                                 //idle mode with tear paper
    tft.fillScreen(TFT_NAVY);                      //this blue looks cool
#ifdef DEBUG_MODE
    Serial.println("// Tear mode, push button to close an image (tear paper)");
#endif
  } else {
    TEAR_mode = 0;
    tft.fillScreen(TFT_DARKGREEN);  //this green looks cool
#ifdef DEBUG_MODE
    Serial.println("// Margin mode, images will be closed automatically");
#endif
  }

  img.createSprite(160, 240);  // then create the giant sprite that will be an image of our video ram buffer

  for (int x = 0; x < 160; x++) {
    for (int y = 0; y < 240; y++) {
      TFT_memory_buffer[x + y * 160] = splashscreen[x + y * 160];
    }
  }

  for (int x = 0; x < 160; x++) {
    for (int y = 0; y < 240; y++) {
      img.drawPixel(x, y, lookup_TFT_RGB565[TFT_memory_buffer[x + y * 160]]);
    }
  }
  img.pushSprite(x_ori, 0);  //the Bodmer TFT uses DMA so nothing is faster than this library

  // Ensure the SPI pinout the SD card is connected to / is configured properly
  SPI1.setRX(SD_MISO);  //see config.h to see SPI groups
  SPI1.setTX(SD_MOSI);
  SPI1.setSCK(SD_SCK);
  if (SD.begin(SD_CS, SPI1)) {  //SD.begin(SD_CS) for SPI0, SPI.setRX, etc. because...
    SDcard_READY = 1;
#ifdef DEBUG_MODE
    Serial.println("// SD card detected, now switch to emulator mode");
#endif
  } else {
    SDcard_READY = 0;
#ifdef DEBUG_MODE
    Serial.println("// SD card not detected, images will not be stored. SD card can still be inserted now");
#endif
    while (!SD.begin(SD_CS, SPI1)) {
      tft.fillScreen(TFT_RED);
      tft.setTextColor(TFT_BLACK);
      tft.drawString("SD failure !", 0, 96, 4);
      LED_WS2812_state(WS2812_SD_crash, 1);
      delay(1000);
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_RED);
      tft.drawString("SD failure !", 0, 96, 4);
      LED_WS2812_state(WS2812_SD_crash, 0);
    }
  }
  for (int i = 0; i < 40; i++) {  //flashes the internal RGB LED, long enough to not discover the easter egg immediately
    LED_WS2812_state(WS2812_Color, 1);
    delay(25);
    LED_WS2812_state(WS2812_Color, 0);
    delay(25);
  }
  ID_file_creator("/tiny.sys");          //create a file on SD card that stores a unique file ID from 1 to 2^32 - 1 (in fact 1 to 99999)
  Next_ID = get_next_ID("/tiny.sys");    //get the file number on SD card
  Next_dir = get_next_dir("/tiny.sys");  //get the folder/session number on SD card
  if (BITBOY_mode == 0) {
    Next_dir++;  //new folder per session or not, if not, max_files_per_folder is taken
  }
  sprintf(tmp_storage_file_name, "/buffer.tmp");
  SD.remove(tmp_storage_file_name);               //remove any previous failed attempt
  store_next_ID("/tiny.sys", Next_ID, Next_dir);  //store next folder #immediately

  //now time for an easter egg
  if (gpio_get(BTN_PUSH)) {
    for (int i = 0; i < 40 * 16; i++) {  // inject the dummy data packet
      printer_memory_buffer_core_0[i] = dummy_packet[i];
    }
    DATA_packet_to_print = 1;
    inner_palette = 0x00;
    inner_lower_margin = 0x01;
    PRINT_flag = 1;
  }
}

void LED_WS2812_state(uint32_t WS2812_Color, bool state) {
  if (state) {
    pixels.setPixelColor(0, WS2812_Color);
    pixels.show();   // Send the updated pixel colors to the hardware.
  } else {           //flop
    pixels.clear();  // Set all pixel colors to 'off'
    pixels.show();   // Send the updated pixel colors to the hardware.
  }
}

void ID_file_creator(const char* path) {  //from fresh SD, device needs a "secret" binary storage file
                                          //this file may never be erased and is accessed frequently as it counts all images recorded with a unique ID
  uint8_t buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  if (!SD.exists(path)) {  //start from a fresh install on SD
    File Datafile = SD.open(path, FILE_WRITE);
    Datafile.write(buf, 8);
    Datafile.close();
#ifdef DEBUG_MODE
    Serial.println("// Creating a new configuration file");
#endif
  } else {
#ifdef DEBUG_MODE
    Serial.println("// Configuration file yet existing");
#endif
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
