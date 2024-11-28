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

// See /WEBUSB.md for details
#if USB_VERSION == 0x210
#include <WebUSB.h>
WebUSB WebUSBSerial(1, "herrzatacke.github.io/gb-printer-web/#/webusb");
#define Serial WebUSBSerial
#endif

/*Dev notes Raphaël BOICHOT 2024/11/28
  The Game boy Printer emulator runs on core 0 in parse mode with decompression by default, with Printer feature disabled. These mode are still available, I've removed nothing
  I've tried to modify as little as possible the emulator part in order to allow easy update with the emulator, just in case. The TinyGB printer justs needs small chunks of code on core 0.
  Surprisingly, the only necessary step to make it compile on the RP2040 Arduino core is... nothing. The while (!Serial) must be commented to make it work without serial
  Compared to Arduino, the RP2040 "hard resets" the USB port, so any serial based converter will probably have minor connection issues with this version.
  The Printer interface part is still working with GNU Octave as well as the Matlab decoder, but not the Android interface.
  The Tiny Printer runs on core 1. It assumes an upscaling factor of 4 by default. This allows me to store compressed 2bbp data as soon as the decoder step, very easily with a lookup table.
  Temporary data are stored on the SD card as image length can be in theory infinite. I only store in ram at maximum 9 packets of 40 tiles (9*16*40 bytes).
  Packets are written on SD card only when a print command is received because the palette is sent last. This means that storing data on SD card every packet makes no sense.
  Games uses a 2 bpp palette, 0xE4 in 80% of the case but not always. A palette can be different for each packet of 40 tiles within an image and use less than 4 colors on purpose.
  I never saw packets with other than 40 tiles (height of the printer head) but it is in theory possible. Images are always 160 pixels width (20 tiles).
  I initially used a BMP output for images in order to debug but I rapidely switched to indexed PNG. The storage container file on SD card contains preformatted data for the PNG decoder.
  I use the PNGenc library because it works. It requires some not-that-obvious memory settings and you're basically alone to configure it. It is used in the NeoGB printer too.
  This library creates micro-stallings on core 0 for unknown reason (it runs on core 1 !), so the need for overclocking the device to support double speed mode in Photo!
  Best would be to rewrite a custom PNG encoder (like with zero compression to ease the thing) and downclock back the device to 133 MHz, maybe one day.
  As every project of mine, this will be a code of theseus so stay tuned.
  That said, enjoy the device !
*/

#define GAME_BOY_PRINTER_MODE false      // to use with https://github.com/Mraulio/GBCamera-Android-Manager and https://github.com/Raphael-Boichot/PC-to-Game-Boy-Printer-interface
#define GBP_OUTPUT_RAW_PACKETS false     // by default, packets are parsed. if enabled, output will change to raw data packets for parsing and decompressing later
#define GBP_USE_PARSE_DECOMPRESSOR true  // embedded decompressor can be enabled for use with parse mode but it requires fast hardware (SAMD21, SAMD51, ESP8266, ESP32)

#include <stdint.h>
#include <stddef.h>
#include "gameboy_printer_protocol.h"
#include "gbp_serial_io.h"

/////////////Specific to TinyGB Printer//////////////
#include <Adafruit_NeoPixel.h>
#include <SPI.h>  //for SD
#include <SD.h>   //for SD
#include "Upscalerlib.h"
#include "config.h"
/////////////Specific to TinyGB Printer//////////////

#if GBP_OUTPUT_RAW_PACKETS
#define GBP_FEATURE_PACKET_CAPTURE_MODE
#else
#define GBP_FEATURE_PARSE_PACKET_MODE
#if GBP_USE_PARSE_DECOMPRESSOR
#define GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
#endif
#endif

#ifdef GBP_FEATURE_PARSE_PACKET_MODE
#include "gbp_pkt.h"
#endif

/* Gameboy Link Cable Mapping to Arduino Pin */
// Note: Serial Clock Pin must be attached to an interrupt pin of the arduino
//  ___________
// |  6  4  2  |
//  \_5__3__1_/   (at cable)
//

// clang-format off
#ifdef ESP8266
// Pin Setup for ESP8266 Devices
//                  | Arduino Pin | Gameboy Link Pin  |
#define GBP_VCC_PIN               // Pin 1            : 5.0V (Unused)
#define GBP_SO_PIN       13       // Pin 2            : ESP-pin 7 MOSI (Serial OUTPUT) -> Arduino 13
#define GBP_SI_PIN       12       // Pin 3            : ESP-pin 6 MISO (Serial INPUT)  -> Arduino 12
#define GBP_SD_PIN                // Pin 4            : Serial Data  (Unused)
#define GBP_SC_PIN       14       // Pin 5            : ESP-pin 5 CLK  (Serial Clock)  -> Arduino 14
#define GBP_GND_PIN               // Pin 6            : GND (Attach to GND Pin)
#define LED_STATUS_PIN    2       // Internal LED blink on packet reception
#else
// Pin Setup for Arduinos
//                  | Arduino Pin | Gameboy Link Pin  |
#define GBP_VCC_PIN               // Pin 1            : 5.0V (Unused)
#define GBP_SO_PIN        4       // Pin 2            : Serial OUTPUT
#define GBP_SI_PIN        3       // Pin 3            : Serial INPUT
#define GBP_SD_PIN                // Pin 4            : Serial Data  (Unused)
#define GBP_SC_PIN        2       // Pin 5            : Serial Clock (Interrupt)
#define GBP_GND_PIN               // Pin 6            : GND (Attach to GND Pin)
#define LED_STATUS_PIN   16       // Internal LED blink on packet reception
#endif
// clang-format on

/*******************************************************************************
*******************************************************************************/

/////////////Specific to TinyGB Printer//////////////
Adafruit_NeoPixel pixels(NUMPIXELS, LED_STATUS_PIN, NEO_RGB);
uint32_t WS2812_Color = pixels.Color(0, intensity, 0);  //RGB triplet
/////////////Specific to TinyGB Printer//////////////

// Dev Note: Gamboy camera sends data payload of 640 bytes usually

#ifdef GBP_FEATURE_PARSE_PACKET_MODE
#define GBP_BUFFER_SIZE 400
#else
#define GBP_BUFFER_SIZE 650
#endif

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

#ifdef GBP_FEATURE_PACKET_CAPTURE_MODE
inline void gbp_packet_capture_loop();
#endif
#ifdef GBP_FEATURE_PARSE_PACKET_MODE
inline void gbp_parse_packet_loop();
#endif

/*******************************************************************************
  Utility Functions
*******************************************************************************/

const char *gbpCommand_toStr(int val) {
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
  Interrupt Service Routine
*******************************************************************************/

#ifdef ESP8266
void ICACHE_RAM_ATTR serialClock_ISR(void)
#else
void serialClock_ISR(void)
#endif
{
  // Serial Clock (1 = Rising Edge) (0 = Falling Edge); Master Output Slave Input (This device is slave)
#ifdef GBP_FEATURE_USING_RISING_CLOCK_ONLY_ISR
  const bool txBit = gpb_serial_io_OnRising_ISR(digitalRead(GBP_SO_PIN));
#else
  const bool txBit = gpb_serial_io_OnChange_ISR(digitalRead(GBP_SC_PIN), digitalRead(GBP_SO_PIN));
#endif
  digitalWrite(GBP_SI_PIN, txBit ? HIGH : LOW);
}


/*******************************************************************************
  Main Setup and Loop
*******************************************************************************/

void setup(void) {
  // Config Serial
  // Has to be fast or it will not transfer the image fast enough to the computer
  // Wait for Serial to be ready
  /////////////Specific to TinyGB Printer//////////////
  //Serial.begin(115200);
  //while (!Serial) { ; }
  /////////////Specific to TinyGB Printer//////////////

  Connect_to_printer();  //makes an attempt to switch in printer mode

  /////////////Specific to TinyGB Printer//////////////
  Tiny_printer_preparation();  //switches in Tiny Printer mode
  /////////////Specific to TinyGB Printer//////////////

  /* Pins from gameboy link cable */
  pinMode(GBP_SC_PIN, INPUT);
  pinMode(GBP_SO_PIN, INPUT);
  pinMode(GBP_SI_PIN, OUTPUT);

  /* Default link serial out pin state */
  digitalWrite(GBP_SI_PIN, LOW);

  /////////////Specific to TinyGB Printer//////////////
  /* LED Indicator */
  // pinMode(LED_STATUS_PIN, OUTPUT);
  // digitalWrite(LED_STATUS_PIN, LOW);
  /////////////Specific to TinyGB Printer//////////////

  /* Setup */
  gpb_serial_io_init(sizeof(gbp_serialIO_raw_buffer), gbp_serialIO_raw_buffer);

  /* Attach ISR */
#ifdef GBP_FEATURE_USING_RISING_CLOCK_ONLY_ISR
  attachInterrupt(digitalPinToInterrupt(GBP_SC_PIN), serialClock_ISR, RISING);  // attach interrupt handler
#else
  attachInterrupt(digitalPinToInterrupt(GBP_SC_PIN), serialClock_ISR, CHANGE);  // attach interrupt handler
#endif

  /* Packet Parser */
#ifdef GBP_FEATURE_PARSE_PACKET_MODE
  gbp_pkt_init(&gbp_pktState);
#endif

#define VERSION_STRING "V3.2.1 (Copyright (C) 2022 Brian Khuu)"
#define TINY_VERSION_STRING "V0.9 (Copyright (C) 2024 Raphaël BOICHOT)"

  /* Welcome Message */
#ifdef GBP_FEATURE_PACKET_CAPTURE_MODE
  Serial.println(F("// GAMEBOY PRINTER Packet Capture " VERSION_STRING));
  Serial.println(F("// Note: Each byte is from each GBP packet is from the gameboy"));
  Serial.println(F("//       except for the last two bytes which is from the printer"));
  Serial.println(F("// JS Raw Packet Decoder: https://mofosyne.github.io/arduino-gameboy-printer-emulator/GameBoyPrinterDecoderJS/gameboy_printer_js_raw_decoder.html"));
#endif
#ifdef GBP_FEATURE_PARSE_PACKET_MODE
  Serial.println(F("// GAMEBOY PRINTER Emulator " VERSION_STRING));
  Serial.println(F("// TINYGB PRINTER Converter " TINY_VERSION_STRING));
  Serial.println(F("// Note: Each hex encoded line is a gameboy tile"));
  //Serial.println(F("// JS Decoder: https://mofosyne.github.io/arduino-gameboy-printer-emulator/GameBoyPrinterDecoderJS/gameboy_printer_js_decoder.html"));
#endif
  Serial.println(F("// --- GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007 ---"));
  Serial.println(F("// This program comes with ABSOLUTELY NO WARRANTY;"));
  Serial.println(F("// This is free software, and you are welcome to redistribute it"));
  Serial.println(F("// under certain conditions. Refer to LICENSE file for detail."));
  Serial.println(F("// ---"));

  Serial.flush();
}  // setup()

void loop() {
  static uint16_t sioWaterline = 0;

#ifdef GBP_FEATURE_PACKET_CAPTURE_MODE
  gbp_packet_capture_loop();
#endif
#ifdef GBP_FEATURE_PARSE_PACKET_MODE
  gbp_parse_packet_loop();
#endif

  // Trigger Timeout and reset the printer if byte stopped being received.
  static uint32_t last_millis = 0;
  uint32_t curr_millis = millis();
  if (curr_millis > last_millis) {
    uint32_t elapsed_ms = curr_millis - last_millis;
    if (gbp_serial_io_timeout_handler(elapsed_ms)) {
      Serial.println("");
      Serial.print("Core 0 -> ");
      Serial.print("Completed ");
      Serial.print("(Memory Waterline: ");
      Serial.print(gbp_serial_io_dataBuff_waterline(false));
      Serial.print("B out of ");
      Serial.print(gbp_serial_io_dataBuff_max());
      Serial.println("B)");
      Serial.flush();
      ///////////////////////specific to the TinyGB Printer////////////////////////
      //digitalWrite(LED_STATUS_PIN, LOW);
      SD.remove(tmp_storage_file_name);  //remove any previous failed attempt
      lines_in_image_file = 0;           //resets the number of lines
      DATA_bytes_counter = 0;            //counter for data bytes
      DATA_packet_counter = 0;
      DATA_packet_to_print = 0;
      Serial.print("Core 0 -> Reset of all converter variables");
      ///////////////////////specific to the TinyGB Printer////////////////////////

#ifdef GBP_FEATURE_PARSE_PACKET_MODE
      gbp_pkt_reset(&gbp_pktState);
#ifdef GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
      tileBuff.count = 0;
#endif
#endif
    }
  }
  last_millis = curr_millis;

  // Diagnostics Console
  while (Serial.available() > 0) {
    switch (Serial.read()) {
      case '?':
        Serial.println("d=debug, ?=help");
        break;

      case 'd':
        Serial.print("waterline: ");
        Serial.print(gbp_serial_io_dataBuff_waterline(false));
        Serial.print("B out of ");
        Serial.print(gbp_serial_io_dataBuff_max());
        Serial.println("B");
        break;
    }
  };
}  // loop()

/////////////Specific to TinyGB Printer//////////////
//with the RP2040 Arduino core, the two loops runs in parallel independantly on each core and you deal with that
//they share variables and memory, which is simple and practical but requires a mental gymnastics.
//you can still use the Pico SDK commands for multicore if you wish
//core 1 could more stable for heavy duty tasks as it does not have to deal with internal interrupts like timers
void loop1()  //core 1 loop deals with images, written by Raphaël BOICHOT, november 2024
{
  if (PRINT_flag == 1) {
    PRINT_flag = 0;
    memcpy(printer_memory_buffer_core_1, printer_memory_buffer_core_0, 640 * DATA_packet_to_print);  //this can also be done by core 0
    LED_WS2812_state(WS2812_Color, 1);
    if (inner_palette == 0x00) {
      inner_palette = 0xE4;  //see Game Boy Programming manual, palette 0x00 is a default palette interpreted as 0xE4 or 0b11100100
    }
    //0xE4 = 0b11100100 = 3-2-1-0 intensity of colors for printer (3 = black, 0 = white), for Game Boy pixel encoded values 0-1-2-3
    image_palette[0] = bitRead(inner_palette, 0) + 2 * bitRead(inner_palette, 1);
    image_palette[1] = bitRead(inner_palette, 2) + 2 * bitRead(inner_palette, 3);
    image_palette[2] = bitRead(inner_palette, 4) + 2 * bitRead(inner_palette, 5);
    image_palette[3] = bitRead(inner_palette, 6) + 2 * bitRead(inner_palette, 7);

    Serial.print("Core 1 -> I received ");
    Serial.print(DATA_packet_to_print, DEC);
    Serial.print(" packets to print with palette ");
    Serial.print(inner_palette, HEX);
    Serial.print(" and ");
    Serial.print(inner_lower_margin, DEC);
    Serial.println(" after margin");

    //All the meat to decode the 2bpp Game Boy Tile Format is explained here (among other sources): https://www.huderlem.com/demos/gameboy2bpp.html
    //the image data are a simple one dimensional array because there is no gain to have a 2D array, in particular when burning data to SD card
    //the issue here is to transform a tile-based system (8*8 pixels) to a pixel-based system in lines/columns.
    IMAGE_bytes_counter = 0;
    pixel_line = 0;
    offset_x = 0;
    max_tile_line = DATA_packet_to_print * 2;                                      //a DATA packet is 2 tiles high
    max_pixel_line = DATA_packet_to_print * 16;                                    //a DATA packet is 16 pixel high
    for (tile_line = 0; tile_line < max_tile_line; tile_line++) {                  //this part fills 8 lines of pixels
      IMAGE_bytes_counter = 16 * 20 * tile_line;                                   //a tile is 16 bytes, a line screen is 20 tiles (160 pixels width)
      for (int i = 0; i < 8; i++) {                                                // This part fills a line of pixels
        offset_x = pixel_line * 160;                                               //x stands for the position in the image vector containing compressed data
        for (tile_column = 0; tile_column < 20; tile_column++) {                   //we progress along 20 column tiles
          local_byte_LSB = printer_memory_buffer_core_1[IMAGE_bytes_counter];      //here we get data for a line of 8 pixels (2 bytes)
          local_byte_MSB = printer_memory_buffer_core_1[IMAGE_bytes_counter + 1];  //here we get data for a line of 8 pixels
          for (int posx = 0; posx < 8; posx++) {
            pixel_level = bitRead(local_byte_LSB, 7 - posx) + 2 * bitRead(local_byte_MSB, 7 - posx);  //here we get pixel value along 8 pixels horizontally
            PNG_image_color[offset_x + posx] = PNG_compress_4x[image_palette[pixel_level]];           //here we store 4 2bbp pixels per byte for next step (png upscaler)
          }                                                                                           //this is a bit aggressive as pixel decoder and PNG compression is within the same line of code, but efficient
          IMAGE_bytes_counter = IMAGE_bytes_counter + 16;                                             //jumps to the next tile in byte
          offset_x = offset_x + 8;                                                                    //jumps to the next tile in pixels
        }                                                                                             //This part fills a line of pixels
        IMAGE_bytes_counter = IMAGE_bytes_counter - 16 * 20 + 2;                                      //shifts to the next two bytes among 16 per tile, so the next line of pixels in a tile
        pixel_line = pixel_line + 1;                                                                  //jumps to the next line
      }                                                                                               //This part fills 8 lines of pixels
    }                                                                                                 //this part fills the entire image

    File Datafile = SD.open(tmp_storage_file_name, FILE_WRITE);        //in any case, if PRINT is received, write to a file (yet existing or not)
    Datafile.write(PNG_image_color, 160 * 16 * DATA_packet_to_print);  //writes the data to SD card
    Datafile.close();
    lines_in_image_file = lines_in_image_file + 16 * DATA_packet_to_print;  //keep track of the number of lines stored
    DATA_packet_to_print = 0;

    if ((inner_lower_margin > 0) & (TEAR_mode == 0)) {  //the printer asks to feed paper, end of file, except in TEAR mode
      Next_ID++;                                        //increment file number
      store_next_ID("/tiny.sys", Next_ID, Next_dir);
      sprintf(png_storage_file_name, "/%05d/%07d.png", Next_dir, Next_ID);
      Serial.print("Core 1 -> Encoding ");
      Serial.print(png_storage_file_name);
      Serial.print(" due to feed paper signal, with ");
      Serial.print(lines_in_image_file, DEC);
      Serial.println(" lines in image file");
      myTime = millis();
      png_upscaler(tmp_storage_file_name, png_storage_file_name, PNG_palette, lines_in_image_file);
      Serial.print("Core 1 -> PNG file closed, encoding time (ms): ");
      Serial.println(millis() - myTime, DEC);
      lines_in_image_file = 0;           //resets the number of lines
      SD.remove(tmp_storage_file_name);  //a bit aggressive and maybe not optmal but I'm sure the old data disappears
    }
    LED_WS2812_state(WS2812_Color, 0);
  }

  //in TEAR mode, a file is never closed unless you push a button
  if ((TEAR_mode == 1) & (digitalRead(BTN_PUSH)) & (lines_in_image_file > 0)) {  //in tear mode, a button push only can close file, whatever the printer (non empty) state
    LED_WS2812_state(WS2812_Color, 1);
    Next_ID++;  //increment file number
    store_next_ID("/tiny.sys", Next_ID, Next_dir);
    sprintf(png_storage_file_name, "/%05d/%07d.png", Next_dir, Next_ID);
    Serial.print("Core 1 -> Encoding ");
    Serial.print(png_storage_file_name);
    Serial.print(" due to push button, with ");
    Serial.print(lines_in_image_file, DEC);
    Serial.println(" lines in image file");
    myTime = millis();
    png_upscaler(tmp_storage_file_name, png_storage_file_name, PNG_palette, lines_in_image_file);
    Serial.print("Core 1 -> PNG file closed, encoding time (ms): ");
    Serial.println(millis() - myTime, DEC);
    lines_in_image_file = 0;           //resets the number of lines
    SD.remove(tmp_storage_file_name);  //a bit aggressive and maybe not optmal but I'm sure the old data disappears
    LED_WS2812_state(WS2812_Color, 0);
  }
}  // loop1()
/////////////Specific to TinyGB Printer//////////////

/******************************************************************************/

#ifdef GBP_FEATURE_PARSE_PACKET_MODE
inline void gbp_parse_packet_loop(void) {
  const char nibbleToCharLUT[] = "0123456789ABCDEF";
  for (int i = 0; i < gbp_serial_io_dataBuff_getByteCount(); i++) {
    if (gbp_pkt_processByte(&gbp_pktState, (const uint8_t)gbp_serial_io_dataBuff_getByte(), gbp_pktbuff, &gbp_pktbuffSize, sizeof(gbp_pktbuff))) {
      if (gbp_pktState.received == GBP_REC_GOT_PACKET) {
        /////////////Specific to TinyGB Printer//////////////
        //digitalWrite(LED_STATUS_PIN, HIGH);
        Serial.print("Core 0 -> ");
        /////////////Specific to TinyGB Printer//////////////
        Serial.print((char)'{');
        Serial.print("\"command\":\"");
        Serial.print(gbpCommand_toStr(gbp_pktState.command));
        Serial.print("\"");
        if (!skip_byte_on_display) {  /////////////Specific to TinyGB Printer//////////////
          if (gbp_pktState.command == GBP_COMMAND_INQUIRY) {
            // !{"command":"INQY","status":{"lowbatt":0,"jam":0,"err":0,"pkterr":0,"unproc":1,"full":0,"bsy":0,"chk_err":0}}
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
          }
        }  /////////////Specific to TinyGB Printer//////////////
        if (gbp_pktState.command == GBP_COMMAND_PRINT) {
          //!{"command":"PRNT","sheets":1,"margin_upper":1,"margin_lower":3,"pallet":228,"density":64 }
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

          ///////////////////////specific to the TinyGB Printer////////////////////////
          DATA_packet_to_print = DATA_packet_counter;                                              //counter for packets transmitted to be transmitted to core 1
          inner_palette = gbp_pkt_printInstruction_palette_value(gbp_pktbuff);                     //this can also be done by core 1
          inner_lower_margin = gbp_pkt_printInstruction_num_of_linefeed_after_print(gbp_pktbuff);  //this can also be done by core 1
          DATA_bytes_counter = 0;                                                                  //counter for data bytes
          DATA_packet_counter = 0;                                                                 //counter for packets transmitted
          PRINT_flag = 1;                                                                          //triggers stuff on core 1, from now core 1 have plenty of time to convert image
          ///////////////////////specific to the TinyGB Printer////////////////////////
        }
        if (gbp_pktState.command == GBP_COMMAND_DATA) {
          //!{"command":"DATA", "compressed":0, "more":0}
          ///////////////////////specific to the TinyGB Printer////////////////////////
          LED_WS2812_state(WS2812_Color, 1);
///////////////////////specific to the TinyGB Printer////////////////////////
#ifdef GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
          Serial.print(", \"compressed\":0");  // Already decompressed by us, so no need to do so
#else
          Serial.print(", \"compressed\":");
          Serial.print(gbp_pktState.compression);
#endif
          Serial.print(", \"more\":");
          Serial.print((gbp_pktState.dataLength != 0) ? '1' : '0');
          ///////////////////////specific to the TinyGB Printer////////////////////////
          LED_WS2812_state(WS2812_Color, 0);
          ///////////////////////specific to the TinyGB Printer////////////////////////
        }
        Serial.println((char)'}');
        Serial.flush();
      } else {
#ifdef GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
        // Required for more complex games with compression support
        while (gbp_pkt_decompressor(&gbp_pktState, gbp_pktbuff, gbp_pktbuffSize, &tileBuff)) {
          if (gbp_pkt_tileAccu_tileReadyCheck(&tileBuff)) {
            // Got Tile
            for (int i = 0; i < GBP_TILE_SIZE_IN_BYTE; i++) {
              const uint8_t data_8bit = tileBuff.tile[i];

              if (!skip_byte_on_display) {  /////////////Specific to TinyGB Printer//////////////
                if (i == GBP_TILE_SIZE_IN_BYTE - 1) {
                  Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
                  Serial.println((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);  // use println on last byte to reduce serial calls
                } else {
                  Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
                  Serial.print((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);
                  Serial.print((char)' ');
                }
              }  /////////////Specific to TinyGB Printer//////////////

              ///////////////////////specific to the TinyGB Printer////////////////////////
              printer_memory_buffer_core_0[DATA_bytes_counter] = data_8bit;  //we get the graphic data here
              DATA_bytes_counter++;
              if (DATA_bytes_counter % 640 == 0) {  //we count the data packets here (16 bytes*40 tiles)
                DATA_packet_counter++;
              }
              ///////////////////////specific to the TinyGB Printer////////////////////////
            }
            Serial.flush();
          }
        }
#else
        // Simplified support for gameboy camera only application
        // Dev Note: Good for checking if everything above decompressor is working
        if (gbp_pktbuffSize > 0) {
          // Got Tile
          for (int i = 0; i < gbp_pktbuffSize; i++) {
            const uint8_t data_8bit = gbp_pktbuff[i];
            if (i == gbp_pktbuffSize - 1) {
              Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
              Serial.println((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);  // use println on last byte to reduce serial calls
            } else {
              Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
              Serial.print((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);
              Serial.print((char)' ');
            }
          }
          Serial.flush();
        }
#endif
      }
    }
  }
}
#endif

#ifdef GBP_FEATURE_PACKET_CAPTURE_MODE
inline void gbp_packet_capture_loop() {
  /* tiles received */
  static uint32_t byteTotal = 0;
  static uint32_t pktTotalCount = 0;
  static uint32_t pktByteIndex = 0;
  static uint16_t pktDataLength = 0;
  const size_t dataBuffCount = gbp_serial_io_dataBuff_getByteCount();
  if (
    ((pktByteIndex != 0) && (dataBuffCount > 0)) || ((pktByteIndex == 0) && (dataBuffCount >= 6))) {
    const char nibbleToCharLUT[] = "0123456789ABCDEF";
    uint8_t data_8bit = 0;
    for (int i = 0; i < dataBuffCount; i++) {  // Display the data payload encoded in hex
      // Start of a new packet
      if (pktByteIndex == 0) {
        pktDataLength = gbp_serial_io_dataBuff_getByte_Peek(4);
        pktDataLength |= (gbp_serial_io_dataBuff_getByte_Peek(5) << 8) & 0xFF00;
#if 0
        Serial.print("// ");
        Serial.print(pktTotalCount);
        Serial.print(" : ");
        Serial.println(gbpCommand_toStr(gbp_serial_io_dataBuff_getByte_Peek(2)));
#endif
        ///////////////////////specific to the TinyGB Printer////////////////////////
        //digitalWrite(LED_STATUS_PIN, HIGH);
        ///////////////////////specific to the TinyGB Printer////////////////////////
      }
      // Print Hex Byte
      data_8bit = gbp_serial_io_dataBuff_getByte();
      Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
      Serial.print((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);
      // Splitting packets for convenience
      if ((pktByteIndex > 5) && (pktByteIndex >= (9 + pktDataLength))) {
        //digitalWrite(LED_STATUS_PIN, LOW);
        Serial.println("");
        pktByteIndex = 0;
        pktTotalCount++;
      } else {
        Serial.print((char)' ');
        pktByteIndex++;  // Byte hex split counter
        byteTotal++;     // Byte total counter
      }
    }
    Serial.flush();
  }
}
#endif

void Connect_to_printer() {
#if GAME_BOY_PRINTER_MODE  //Printer mode
  pinMode(GBP_SC_PIN, OUTPUT);
  pinMode(GBP_SO_PIN, INPUT_PULLUP);
  pinMode(GBP_SI_PIN, OUTPUT);
  ///////////////////////specific to the TinyGB Printer////////////////////////
  //pinMode(LED_STATUS_PIN, OUTPUT);
  ///////////////////////specific to the TinyGB Printer////////////////////////
  const char INIT[] = { 0x88, 0x33, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };  //INIT command
  uint8_t junk, status;
  for (uint8_t i = 0; i < 10; i++) {
    junk = (printing(INIT[i]));
    if (i == 8) {
      status = junk;
    }
  }
  if (status == 0X81)  //Printer connected !
  {
    digitalWrite(GBP_SC_PIN, HIGH);  //acts like a real Game Boy
    digitalWrite(GBP_SI_PIN, LOW);   //acts like a real Game Boy
    Serial.println(F("// A printer is connected to the serial cable !!!"));
    Serial.println(F("// GAME BOY PRINTER I/O INTERFACE Made By Raphaël BOICHOT, 2023"));
    Serial.println(F("// Use with the GBCamera-Android-Manager: https://github.com/Mraulio/GBCamera-Android-Manager"));
    Serial.println(F("// Or with the PC-to-Game-Boy-Printer-interface: https://github.com/Raphael-Boichot/PC-to-Game-Boy-Printer-interface"));
    delay(100);
    Serial.begin(9600);
    while (!Serial) { ; }
    while (Serial.available() > 0) {  //flush the buffer from any remaining data
      Serial.read();
    }
    ///////////////////////specific to the TinyGB Printer////////////////////////
    //digitalWrite(LED_STATUS_PIN, HIGH);  //LED ON = PRINTER INTERFACE mode
    ///////////////////////specific to the TinyGB Printer////////////////////////
    while (true) {
      if (Serial.available() > 0) {
        Serial.write(printing(Serial.read()));
      }
    }
  }
#endif
}

#if GAME_BOY_PRINTER_MODE      //Printer mode
char printing(char byte_sent)  // this function prints bytes to the serial
{
  bool bit_sent, bit_read;
  char byte_read;
  for (int i = 0; i <= 7; i++) {
    bit_sent = bitRead(byte_sent, 7 - i);
    digitalWrite(GBP_SC_PIN, LOW);
    digitalWrite(GBP_SI_PIN, bit_sent);  //GBP_SI_PIN is SOUT for the printer
                                         ///////////////////////specific to the TinyGB Printer////////////////////////
    LED_WS2812_state(WS2812_Color, bit_sent);
    ///////////////////////specific to the TinyGB Printer////////////////////////
    delayMicroseconds(30);  //double speed mode
    digitalWrite(GBP_SC_PIN, HIGH);
    bit_read = (digitalRead(GBP_SO_PIN));  //GBP_SO_PIN is SIN for the printer
    bitWrite(byte_read, 7 - i, bit_read);
    delayMicroseconds(30);  //double speed mode
  }
  delayMicroseconds(0);  //optionnal delay between bytes, may be less than 1490 µs
  //  Serial.println(byte_sent, HEX);
  //  Serial.println(byte_read, HEX);
  return byte_read;
}
#endif

void Tiny_printer_preparation() {
  Serial.begin(115200);
  delay(1000);
  if (digitalRead(BTN_PUSH)) {
    WS2812_Color = pixels.Color(0, 0, intensity);  //RGB triplet
    TEAR_mode = 1;                                 //idle mode with tear paper
    Serial.println("// Tear mode, push button to close an image (tear paper)");
  } else {
    TEAR_mode = 0;
    Serial.println("// Margin mode, images will be closed automatically");
  }
  // Ensure the SPI pinout the SD card is connected to / is configured properly
  SPI1.setRX(SD_MISO);
  SPI1.setTX(SD_MOSI);
  SPI1.setSCK(SD_SCK);
  if (SD.begin(SD_CS, SPI1)) {  //SD.begin(SD_CS) for SPI0, SPI.setRX, etc. because...
    SDcard_READY = 1;
    Serial.println("// SD card detected, now switch to emulator mode");
  } else {
    SDcard_READY = 0;
    uint32_t WS2812_SD_crash = pixels.Color(intensity, 0, 0);  //RGB triplet
    Serial.println("// SD card not detected, images will not be stored. SD card can still be inserted now");
    while (!SD.begin(SD_CS, SPI1)) {
      LED_WS2812_state(WS2812_SD_crash, 1);
      delay(1000);
      LED_WS2812_state(WS2812_SD_crash, 0);
    }
  }
  for (int i = 0; i < 20; i++) {  // For each pixel..
    LED_WS2812_state(WS2812_Color, 1);
    delay(25);
    LED_WS2812_state(WS2812_Color, 0);
    delay(25);
  }
  ID_file_creator("/tiny.sys");          //create a file on SD card that stores a unique file ID from 1 to 2^32 - 1 (in fact 1 to 99999)
  Next_ID = get_next_ID("/tiny.sys");    //get the file number on SD card
  Next_dir = get_next_dir("/tiny.sys");  //get the folder/session number on SD card
  Next_dir++;
  sprintf(tmp_storage_file_name, "/buffer.tmp");
  SD.remove(tmp_storage_file_name);               //remove any previous failed attempt
  store_next_ID("/tiny.sys", Next_ID, Next_dir);  //store next folder #immediately
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
