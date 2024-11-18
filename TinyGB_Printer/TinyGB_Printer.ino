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

//Dev notes
//Parse mode and decompressor are forced by default
//Parse mode uses tons of variable to assess the state of the printer so it's easier to interface with an inner decoder
#define CORE_0_SPEAKING false  //for debug, better let only one core speaking at a time
#define CORE_1_SPEAKING false  //for debug, better let only one core speaking at a time

#include <stdint.h>  // uint8_t
#include <stddef.h>  // size_t
#include "gameboy_printer_protocol.h"
#include "gbp_serial_io.h"
#include "gbp_pkt.h"  //PARSE MODE forced by default

/////////////////////////////////////BOICHOT
#include <Adafruit_NeoPixel.h>
#include <SPI.h>  //for SD
#include <SD.h>   //for SD
#include "config.h"
// #include "pico/stdlib.h"
// #include "hardware/gpio.h"
/////////////////////////////////////

#define GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
#define GBP_BUFFER_SIZE 400  //parce mode does not store the 640 bytes of payload

/* Gameboy Link Cable Mapping to Arduino Pin */
// Note: Serial Clock Pin must be attached to an interrupt pin of the arduino
//  ___________
// |  6  4  2  |
//  \_5__3__1_/   (at cable)
//
//                  | Arduino Pin | Gameboy Link Pin  |
#define GBP_SC_PIN 2  // Pin 5            : Serial Clock (Interrupt)
#define GBP_SI_PIN 3  // Pin 3            : Serial INPUT
#define GBP_SO_PIN 4  // Pin 2            : Serial OUTPUT

///////////////////////////////////////////BOICHOT
Adafruit_NeoPixel pixels(NUMPIXELS, LED_WS2812, NEO_RGB);
uint32_t WS2812_Color = pixels.Color(0, intensity, 0);  //RGB triplet
///////////////////////////////////////////

/*******************************************************************************
*******************************************************************************/
/* Serial IO */
// This circular buffer contains a stream of raw packets from the gameboy
uint8_t gbp_serialIO_raw_buffer[GBP_BUFFER_SIZE] = { 0 };


/* Packet Buffer */
gbp_pkt_t gbp_pktState = { GBP_REC_NONE, 0 };
uint8_t gbp_pktbuff[GBP_PKT_PAYLOAD_BUFF_SIZE_IN_BYTE] = { 0 };
uint8_t gbp_pktbuffSize = 0;
#ifdef GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
gbp_pkt_tileAcc_t tileBuff = { 0 };
#endif

inline void gbp_parse_packet_loop();


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

void setup() {
  // Config Serial
  // Has to be fast or it will not transfer the image fast enough to the computer
  Serial.begin(115200);

  // Wait for Serial to be ready
  //while (!Serial) { ; }  //no need for autonomous design with the RP2040
  Tiny_printer_preparation();  //switches in Tiny Printer mode

  /* Pins from gameboy link cable */
  pinMode(GBP_SC_PIN, INPUT);
  pinMode(GBP_SO_PIN, INPUT);
  pinMode(GBP_SI_PIN, OUTPUT);

  /* Default link serial out pin state */
  digitalWrite(GBP_SI_PIN, LOW);

  /* Setup */
  gpb_serial_io_init(sizeof(gbp_serialIO_raw_buffer), gbp_serialIO_raw_buffer);

  /* Attach ISR */
#ifdef GBP_FEATURE_USING_RISING_CLOCK_ONLY_ISR
  attachInterrupt(digitalPinToInterrupt(GBP_SC_PIN), serialClock_ISR, RISING);  // attach interrupt handler
#else
  attachInterrupt(digitalPinToInterrupt(GBP_SC_PIN), serialClock_ISR, CHANGE);  // attach interrupt handler
#endif

  /* Packet Parser */
  gbp_pkt_init(&gbp_pktState);

#define VERSION_STRING "V0.1 (Copyright (C) 2022/2024 Brian KHUU/Raphaël BOICHOT)"

  /* Welcome Message */
  Serial.println(F("// Tiny Printer Emulator " VERSION_STRING));
  Serial.flush();
}  // setup()

void setup1() {
  // nothing here
}  // setup1()

void loop() {  //core 0
  static uint16_t sioWaterline = 0;
  gbp_parse_packet_loop();

  // Trigger Timeout and reset the printer if byte stopped being received.
  static uint32_t last_millis = 0;
  uint32_t curr_millis = millis();
  if (curr_millis > last_millis) {
    uint32_t elapsed_ms = curr_millis - last_millis;
    if (gbp_serial_io_timeout_handler(elapsed_ms)) {
      Serial.println("");
      Serial.print("// Completed ");
      Serial.print("(Memory Waterline: ");
      Serial.print(gbp_serial_io_dataBuff_waterline(false));
      Serial.print("B out of ");
      Serial.print(gbp_serial_io_dataBuff_max());
      Serial.println("B)");
      Serial.flush();

      gbp_pkt_reset(&gbp_pktState);
#ifdef GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
      tileBuff.count = 0;
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

void loop1()  //core 1 loop written by Raphaël BOICHOT, november 2024
{
  if (PRINT_flag == 1) {
    SD_card_access_Color = pixels.Color(intensity, 0, 0);       //RGB triplet
    BMP_decoder_color = pixels.Color(0, intensity, intensity);  //RGB triplet
    delay(500);                                                 // if printing is ran immediately, it interferes with the interrupt...
    // It must be due to a voltage drop or some interrupt routines interfering, not sure
    // As we have plenty of time on core 1, it's not that a problem. Image must just be decoded before the next PRINT command
    PRINT_flag = 0;
    //preparing palette;

    if (inner_palette == 0x00) {
      inner_palette == 0xE4;  //see Game Boy Programming manual, palette 0x00 is a default palette interpreted as 0xE4 or 0b11100100
    }

    image_palette[3] = bitRead(inner_palette, 0) + 2 * bitRead(inner_palette, 1);
    image_palette[2] = bitRead(inner_palette, 2) + 2 * bitRead(inner_palette, 3);
    image_palette[1] = bitRead(inner_palette, 4) + 2 * bitRead(inner_palette, 5);
    image_palette[0] = bitRead(inner_palette, 6) + 2 * bitRead(inner_palette, 7);


    // Serial.println("");
    // Serial.print("Packets to print: ");
    // Serial.println(DATA_packet_to_print, DEC);
    // Serial.print("Palette/GB to color/after margin: ");
    // Serial.print(inner_palette, HEX);
    // Serial.print("/");
    // Serial.print(image_palette[0], HEX);
    // Serial.print(image_palette[1], HEX);
    // Serial.print(image_palette[2], HEX);
    // Serial.print(image_palette[3], HEX);
    // Serial.print("/");
    // Serial.println(inner_lower_margin, HEX);


    //BMP converter is written to decode pixels line by line in order to ease future integration with png encoder for example (not planned but possible)
    //All the meat to decode the 2bpp Game Boy Tile Format is explained here (among other sources): https://www.huderlem.com/demos/gameboy2bpp.html
    //the bmp data are a simple one dimenstional array because there is no gain to have a 2D array, in particular when burning data to SD card
    LED_WS2812_state(BMP_decoder_color, 1);
    memset(BMP_image_color, 0, sizeof(BMP_image_color));  //clean the whole image data array
    BMP_bytes_counter = 0;
    pixel_line = 0;
    int max_tile_line = DATA_packet_to_print * 2;
    // Serial.print("Max lines of tiles: ");
    // Serial.println(max_tile_line, DEC);
    int max_pixel_line = DATA_packet_to_print * 16;
    // Serial.print("Max lines of pixels to convert: ");
    // Serial.println(max_pixel_line, DEC);
    //
    for (tile_line = 0; tile_line < max_tile_line; tile_line++) {  // This part fills 8 lines of pixels
      BMP_bytes_counter = 16 * 20 * tile_line;
      for (int i = 0; i < 8; i++) {  // This part fills a line of pixels
        offset_x = pixel_line * 160;
        for (tile_column = 0; tile_column < 20; tile_column++) {  //we progress along 20 column tiles
          local_byte_LSB = printer_memory_buffer_core_1[BMP_bytes_counter];
          local_byte_MSB = printer_memory_buffer_core_1[BMP_bytes_counter + 1];
          for (int posx = 0; posx < 8; posx++) {
            pixel_level = bitRead(local_byte_LSB, 7 - posx) + 2 * bitRead(local_byte_MSB, 7 - posx);
            BMP_image_color[offset_x + posx] = BMP_palette[image_palette[pixel_level]];
          }
          BMP_bytes_counter = BMP_bytes_counter + 16;  //jump to the next tile in byte
          offset_x = offset_x + 8;                     //jump to the next tile in pixels
        }                                              // This part fills a line of pixels
        // Serial.print("BMP byte counter: ");
        // Serial.println(BMP_bytes_counter, DEC);
        // Serial.print("pixel_line: ");
        // Serial.println(pixel_line, DEC);
        BMP_bytes_counter = BMP_bytes_counter - 16 * 20 + 2;  //shift to the next two bytes among 16 per tile, so the next line of pixels in a tile
        pixel_line = pixel_line + 1;                          //jump to the next line
      }                                                       // This part fills 8 lines of pixels
      // Serial.print("BMP byte counter: ");
      // Serial.println(BMP_bytes_counter, DEC);
    }  //this part fills the entire image

    // Serial.print("BMP byte counter at the end: ");
    // Serial.println(BMP_bytes_counter, DEC);
    // Serial.print("pixel line at the end: ");
    // Serial.println(pixel_line, DEC);

    memset(printer_memory_buffer_core_1, 0, sizeof(printer_memory_buffer_core_1));  //clean the whole image data array
    //////////////////////////////////////////////end of BMP converter stuff
    LED_WS2812_state(SD_card_access_Color, 1);

    if (NEWFILE_flag == 1) {
      sprintf(storage_file_name, "/%05d/%07d.bmp", Next_dir, Next_ID);
      // Serial.println("This is a new file, Dummy BMP header is now written");
      // Serial.print("Burning: ");
      // Serial.println(storage_file_name);
      NEWFILE_flag = 0;
      Pre_allocate_bmp_header(0, 0);  //creates a dummy BMP header
      File Datafile = SD.open(storage_file_name, FILE_WRITE);
      Datafile.write(BMP_header_generic, 54);     //writes a dummy BMP header
      Datafile.write(BMP_indexed_palette, 1024);  //indexed RGB palette
      Datafile.close();
    }
    //writing loop
    //Serial.println("Writing new packets to BMP file");
    File Datafile = SD.open(storage_file_name, FILE_WRITE);
    Datafile.write(BMP_image_color, 160 * 16 * DATA_packet_to_print);  //writes the data to SD card
    Datafile.close();
    lines_in_bmp_file = lines_in_bmp_file + 16 * DATA_packet_to_print;
    // Serial.print("Current lines in BMP file: ");
    // Serial.println(lines_in_bmp_file, DEC);
    DATA_packet_to_print = 0;

    if (inner_lower_margin > 0) {  //the printer asks to feed paper, end of file
      CLOSE_flag = 1;
      //Serial.println("File will be closed after writing the current packets");
    } else {
      CLOSE_flag = 0;
      //Serial.println("Continuing on existing file for next packets...");
    }

    if (CLOSE_flag == 1) {
      //Serial.println("Closing an existing file, finalising BMP header");  // now updating the BMP header
      Pre_allocate_bmp_header(160, lines_in_bmp_file);                    //number of lines will be updated now
      File Datafile = SD.open(storage_file_name, FILE_WRITE);
      Datafile.seek(0);                        //go to the beginning of the file
      Datafile.write(BMP_header_generic, 54);  //fixed header fixed with correct lenght
      Datafile.close();
      lines_in_bmp_file = 0;
      Next_ID++;
      store_next_ID("/tiny.sys", Next_ID, Next_dir);
      NEWFILE_flag = 1;
      lines_in_bmp_file = 0;
    }
    LED_WS2812_state(SD_card_access_Color, 0);
  }
  //BMP and SD stuff will be here
}  // loop1()

/******************************************************************************/
inline void gbp_parse_packet_loop(void) {
  const char nibbleToCharLUT[] = "0123456789ABCDEF";
  for (int i = 0; i < gbp_serial_io_dataBuff_getByteCount(); i++) {
    if (gbp_pkt_processByte(&gbp_pktState, (const uint8_t)gbp_serial_io_dataBuff_getByte(), gbp_pktbuff, &gbp_pktbuffSize, sizeof(gbp_pktbuff))) {
      if (gbp_pktState.received == GBP_REC_GOT_PACKET) {
        Serial.print((char)'{');
        Serial.print("\"command\":\"");
        Serial.print(gbpCommand_toStr(gbp_pktState.command));
        Serial.print("\"");
        if (gbp_pktState.command == GBP_COMMAND_INQUIRY) {
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
        if (gbp_pktState.command == GBP_COMMAND_PRINT) {
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

          ////////////////////////////////////////////////////////////////////////BOICHOT
          DATA_packet_to_print = DATA_packet_counter;  //counter for packets transmitted to be transmitted to core 1
          //Serial.println("PRINT command received");
          //Serial.println("All packets resetted, core 1 deals with next steps");
          memcpy(printer_memory_buffer_core_1, printer_memory_buffer_core_0, 640 * DATA_packet_to_print);
          inner_palette = gbp_pkt_printInstruction_palette_value(gbp_pktbuff);
          inner_lower_margin = gbp_pkt_printInstruction_num_of_linefeed_after_print(gbp_pktbuff);
          PRINT_flag = 1;           //triggers stuff on core 1, from now core 1 have plenty of time to convert image
          DATA_bytes_counter = 0;   //counter for data bytes
          DATA_packet_counter = 0;  //counter for packets transmitted
          ///////////////////////////////////////////////////////////////////////
        }
        if (gbp_pktState.command == GBP_COMMAND_DATA) {
          LED_WS2812_state(WS2812_Color, 1);
          //!{"command":"DATA", "compressed":0, "more":0}
#ifdef GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
          Serial.print(", \"compressed\":0");  // Already decompressed by us, so no need to do so
#else
          Serial.print(", \"compressed\":");
          Serial.print(gbp_pktState.compression);
#endif
          Serial.print(", \"more\":");
          Serial.print((gbp_pktState.dataLength != 0) ? '1' : '0');
          LED_WS2812_state(WS2812_Color, 0);
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
              if (i == GBP_TILE_SIZE_IN_BYTE - 1) {
                Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
                Serial.println((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);  // use println on last byte to reduce serial calls
              } else {
                Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
                Serial.print((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);
                Serial.print((char)' ');
              }

              ////////////////////////////////////////////////////////////////////////BOICHOT
              printer_memory_buffer_core_0[DATA_bytes_counter] = data_8bit;
              DATA_bytes_counter++;
              if (DATA_bytes_counter % 640 == 0) {
                DATA_packet_counter++;
                //Serial.print("Packet added: ");
                //Serial.println(DATA_packet_counter, DEC);
              }
              ///////////////////////////////////////////////////////////////////////
            }
            Serial.flush();
          }
        }
#else   //GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR \
        // Simplified support for gameboy camera only application \
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

            ////////////////////////////////////////////////////////////////////////BOICHOT
            printer_memory_buffer_core_0[DATA_bytes_counter] = data_8bit;
            DATA_bytes_counter++;
            if (DATA_bytes_counter % 640 == 0) {
              DATA_packet_counter++;
              // Serial.println("Packet added");
              // Serial.println(DATA_packet_counter, DEC);
            }
            ///////////////////////////////////////////////////////////////////////
          }
          Serial.flush();
        }
#endif  //GBP_FEATURE_PARSE_PACKET_USE_DECOMPRESSOR
      }
    }
  }
}  //inline void gbp_parse_packet_loop(void)

void LED_WS2812_state(uint32_t WS2812_Color, bool state) {
  if (state) {
    pixels.setPixelColor(0, WS2812_Color);
    pixels.show();   // Send the updated pixel colors to the hardware.
  } else {           //flop
    pixels.clear();  // Set all pixel colors to 'off'
    pixels.show();   // Send the updated pixel colors to the hardware.
  }
}

void Tiny_printer_preparation() {
  ////////////////////////////////////////////////////////BOICHOT
  Serial.begin(115200);
  delay(1000);
  if (digitalRead(BTN_PUSH)) {
    WS2812_Color = pixels.Color(0, 0, intensity);  //RGB triplet
    TEAR_mode = 1;                                 //idle mode with tear paper
    Serial.println("// Tear mode, push button to close an image (tear paper)");
  } else {
    Serial.println("// Margin mode, images will be closed automatically");
  }
  // Ensure the SPI pinout the SD card is connected to / is configured properly
  SPI1.setRX(SD_MISO);
  SPI1.setTX(SD_MOSI);
  SPI1.setSCK(SD_SCK);
  if (SD.begin(SD_CS, SPI1)) {  //SD.begin(SD_CS) for SPI0, SPI.setRX, etc. because...
    SDcard_READY = 1;
    Serial.println("// SD card detected");
  } else {
    SDcard_READY = 0;
    WS2812_Color = pixels.Color(intensity, 0, 0);  //RGB triplet
    Serial.println("// SD card not detected, images will not be stored");
    while (1) {
      LED_WS2812_state(WS2812_Color, 1);
      delay(250);
      LED_WS2812_state(WS2812_Color, 0);
      delay(250);
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
  store_next_ID("/tiny.sys", Next_ID, Next_dir);
  ////////////////////////////////////////////////////////
}