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

#define GAME_BOY_PRINTER_MODE false  // to use with https://github.com/Mraulio/GBCamera-Android-Manager and https://github.com/Raphael-Boichot/PC-to-Game-Boy-Printer-interface

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
  Connect_to_printer();        //makes an attempt to switch in printer mode
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
  Serial.println(F("// Note: Each hex encoded line is a gameboy tile"));
  Serial.println(F("// --- GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007 ---"));
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

void loop1()  //core 1
{
  if (PRINT_flag == 1) {
    PRINT_flag = 0;
    //preparing palette;
    image_palette[3] = bitRead(inner_palette, 0) + 2 * bitRead(inner_palette, 1);
    image_palette[2] = bitRead(inner_palette, 2) + 2 * bitRead(inner_palette, 3);
    image_palette[1] = bitRead(inner_palette, 4) + 2 * bitRead(inner_palette, 5);
    image_palette[0] = bitRead(inner_palette, 6) + 2 * bitRead(inner_palette, 7);
    if (inner_palette>0){//the printer asks to feed paper, end of file
      CLOSE_flag=1;
    }
    //now the big loop

    Serial.println("");
    Serial.println("Core 1 is ready to convert tiles to BMP");
    sprintf(storage_file_name, "/%05d/%07d.bmp", Next_dir, Next_ID);
    Serial.println(storage_file_name);
    Serial.print(inner_palette, HEX);
    Serial.print("/");
    Serial.print(image_palette[0], HEX);
    Serial.print(image_palette[1], HEX);
    Serial.print(image_palette[2], HEX);
    Serial.println(image_palette[3], HEX);
    Serial.println(inner_lower_margin, HEX);
    Serial.println("");
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

          ////////////////////////////////////////////////////////////////////////BOICHOT
          DATA_packet_to_print = DATA_packet_counter;  //counter for packets transmitted to be transmitted to core 1
          memcpy(printer_memory_buffer_core_1, printer_memory_buffer_core_0, 640 * DATA_packet_to_print);
          inner_palette = gbp_pkt_printInstruction_palette_value(gbp_pktbuff);
          inner_lower_margin = gbp_pkt_printInstruction_num_of_linefeed_after_print(gbp_pktbuff);
          PRINT_flag = 1;           //triggers stuff on core 1, from now core 1 have plenty of time to convert image
          DATA_bytes_counter = 0;   //counter for data bytes
          DATA_packet_counter = 0;  //counter for packets transmitted
          Serial.println("");
          Serial.println("PRINT command received");
          Serial.println("All packets resetted, core 1 deals with next steps");
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
              // if (i == GBP_TILE_SIZE_IN_BYTE - 1) {
              //   Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
              //   Serial.println((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);  // use println on last byte to reduce serial calls
              // } else {
              //   Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
              //   Serial.print((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);
              //   Serial.print((char)' ');
              // }

              ////////////////////////////////////////////////////////////////////////BOICHOT
              printer_memory_buffer_core_0[DATA_bytes_counter] = data_8bit;
              DATA_bytes_counter++;
              if (DATA_bytes_counter % 640 == 0) {
                DATA_packet_counter++;
                Serial.println("Packet added");
                Serial.println(DATA_packet_counter, DEC);
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
            // if (i == gbp_pktbuffSize - 1) {
            //   Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
            //   Serial.println((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);  // use println on last byte to reduce serial calls
            // } else {
            //   Serial.print((char)nibbleToCharLUT[(data_8bit >> 4) & 0xF]);
            //   Serial.print((char)nibbleToCharLUT[(data_8bit >> 0) & 0xF]);
            //   Serial.print((char)' ');
            // }

            ////////////////////////////////////////////////////////////////////////BOICHOT
            printer_memory_buffer_core_0[DATA_bytes_counter] = data_8bit;
            DATA_bytes_counter++;
            if (DATA_bytes_counter % 640 == 0) {
              DATA_packet_counter++;
              Serial.println("Packet added");
              Serial.println(DATA_packet_counter, DEC);
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


void Connect_to_printer() {
#if GAME_BOY_PRINTER_MODE  //Printer mode
  pinMode(GBP_SC_PIN, OUTPUT);
  pinMode(GBP_SO_PIN, INPUT_PULLUP);
  pinMode(GBP_SI_PIN, OUTPUT);
  LED_WS2812_state(WS2812_Color, OUTPUT);
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
    LED_WS2812_state(WS2812_Color, 1);

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
  uint32_t WS2812_Color = pixels.Color(intensity, intensity, intensity);  //RGB triplet
  bool bit_sent, bit_read;
  char byte_read;
  for (int i = 0; i <= 7; i++) {
    bit_sent = bitRead(byte_sent, 7 - i);
    digitalWrite(GBP_SC_PIN, LOW);
    digitalWrite(GBP_SI_PIN, bit_sent);  //GBP_SI_PIN is SOUT for the printer
    LED_WS2812_state(WS2812_Color, bit_sent);
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