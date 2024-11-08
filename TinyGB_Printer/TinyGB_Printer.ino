#include <stdint.h>  // uint8_t
#include <stddef.h>  // size_t

#include "config.h"
#include "StoreIDlib.h"
#include "gameboy_printer_protocol.h"
#include "gbp_pkt.h"
#include "gbp_serial_io.h"

//SD libraries
#include "SPI.h"
#include "FS.h"
#include "SD.h"
#define FSYS SD

#define numVersion "TinyGB_Printer 0.1"

/*******************************************************************************
*******************************************************************************/
// Dev Note: Gamboy camera sends data payload of 640 bytes usually
#define GBP_BUFFER_SIZE 650

/* Serial IO */
// This circular buffer contains a stream of raw packets from the gameboy
uint8_t gbp_serialIO_raw_buffer[GBP_BUFFER_SIZE] = { 0 };
inline void gbp_packet_capture_loop();

/*******************************************************************************
   Process Variables
*******************************************************************************/
gbp_pkt_t gbp_pktBuff = { GBP_REC_NONE, 0 };
uint8_t gbp_pktbuff[GBP_PKT_PAYLOAD_BUFF_SIZE_IN_BYTE] = { 0 };

/*******************************************************************************
   Custom Variables
*******************************************************************************/
// Button Variables
long buttonTimer = 0;
long longPressTime = 2000;  //2 seconds
boolean buttonActive = false;
boolean longPressActive = false;

//Data Controller
byte image_data[6000] = {};  //1 GBC Picute (5.874)
uint8_t chkHeader = 99;
uint8_t chkMargin = 0x00;
uint32_t img_index = 0x00;
unsigned int freeFileIndex = 0;
unsigned int dumpCount = 0;
unsigned int totalMultiImages = 1;

//States
bool bootAsPrinter;
bool isPrinting = false;
bool isWriting = false;
bool isConverting = false;
bool isFileSystemMounted = false;
bool setMultiPrint = false;

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
void serialClock_ISR(void) {
  // Serial Clock (1 = Rising Edge) (0 = Falling Edge); Master Output Slave Input (This device is slave)
#ifdef GBP_FEATURE_USING_RISING_CLOCK_ONLY_ISR
  const bool txBit = gpb_serial_io_OnRising_ISR(digitalRead(GBP_SI_PIN));
#else
  const bool txBit = gpb_serial_io_OnChange_ISR(digitalRead(GBP_SC_PIN), digitalRead(GBP_SI_PIN));
#endif
  digitalWrite(GBP_SO_PIN, txBit ? HIGH : LOW);
}

/*******************************************************************************
  Main Setup and Loop
*******************************************************************************/
void setup(void) {
  Serial.begin(115200);  // Config Serial

  //LED_init();

  /* Pin for pushbutton */
  pinMode(BTN_PUSH, INPUT);


//Blink the LED to test it. For RGB LED, must in this order: WHITE(all colors), RED, GREEN, BLUE
#ifdef LED_STATUS_PIN
  //LED_blink(LED_STATUS_PIN, 3, 50, 50);
#endif

  //Initialize FileSystem
  isFileSystemMounted = fs_setup();

  if (isFileSystemMounted) {
    ID_file_checker();  //Create/check controller file
    //setupImages();      //Get the Image Scale Factors from config file

//Check the bootMode (Printer mode or WiFi mode)
#ifdef ENABLE_WEBSERVER
    bootAsPrinter = fs_alternateBootMode();
#else
    bootAsPrinter = true;
#endif

    //Get the % of use and the next image ID. If full boot as Server
    uint8_t percUsed = fs_info();
    //If full, stucks with a message
    if (percUsed <= 10) {
      bootAsPrinter = false;
      //full();
      //delay(5000);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    //    Serial.println("Checking for dumps to convert...");
    //    freeFileIndex = get_next_ID();
    //    dumpCount = get_dumps();
    //    if ((freeFileIndex-1) > 0 && dumpCount > 0){
    //      Serial.println("Converting dumps to image files");
    //      isConverting = true;
    //      #ifdef USE_OLED
    //        oledStateChange(5); //BIN to BMP
    //      #endif
    //      ConvertFilesBMP();
    //      Serial.println("Done!");
    //    }else{
    //      Serial.println("No dumps found!");
    //    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    if (bootAsPrinter) {
      //initWifi(); //Initiate WiFi and NTP
      Serial.println("-----------------------");
      Serial.println("Booting in printer mode");
      Serial.println("-----------------------");

      freeFileIndex = get_next_ID();
      dumpCount = get_dumps();

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

      gbp_pkt_init(&gbp_pktBuff);

    }
#ifdef ENABLE_WEBSERVER
    else {
      Serial.println("-----------------------");
      Serial.println("Booting in server mode");
      Serial.println("-----------------------");

      /////////////////////////////////////////////////////////////////////////////////////////////////////
      Serial.println("Checking for dumps to convert...");
      freeFileIndex = get_next_ID();
      dumpCount = get_dumps();
      if ((freeFileIndex - 1) > 0 && dumpCount > 0) {
        Serial.println("Converting dumps to image files");
        isConverting = true;
        ConvertFilesBMP();
        Serial.println("Done!");
      } else {
        Serial.println("No dumps found!");
      }
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
    }
#endif

#ifdef LED_STATUS_PIN
    //LED_blink(LED_STATUS_PIN, 3, 100, 100);
#endif

  } else {
#ifdef LED_STATUS_PIN
    //LED_blink(LED_STATUS_PIN, 3, 200, 200);
#endif
  }
}

void loop() {
  if (isFileSystemMounted) {
    if (bootAsPrinter) {
      gbp_packet_capture_loop();

      // Trigger Timeout and reset the printer if byte stopped being received.
      static uint32_t last_millis = 0;
      uint32_t curr_millis = millis();
      if (curr_millis > last_millis) {
        uint32_t elapsed_ms = curr_millis - last_millis;
        if (gbp_serial_io_timeout_handler(elapsed_ms)) {
          //Printer Timeout
          Serial.println("Printer Timeout");

          //Reset Values
          chkHeader = 99;
          memset(image_data, 0x00, sizeof(image_data));

#ifdef LED_STATUS_PIN
          //LED_led_OFF(LED_STATUS_PIN);
#endif

          if (!setMultiPrint && totalMultiImages > 1 && !isWriting && !isPrinting) {
            ResetPrinterVariables();
          }
        }
      }
      last_millis = curr_millis;

      // Feature to detect a short press and a Long Press
      if (!isWriting) {
#ifdef BTN_INVERT
        if (digitalRead(BTN_PUSH) == LOW) {
#else
        if (digitalRead(BTN_PUSH) == HIGH) {
#endif
          if (buttonActive == false) {
            buttonActive = true;
            buttonTimer = millis();
          }
          if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {
            longPressActive = true;
            //Long press to convert to Image Files
            //if (!isConverting && !isPrinting && (freeFileIndex-1) > 0 && dumpCount > 0){
            if (!isConverting && (freeFileIndex - 1) > 0 && dumpCount > 0) {
              Serial.println("Converting to Image File");
              isConverting = true;

              //Remove the interrupt to prevent receive data from Gameboy
              detachInterrupt(digitalPinToInterrupt(GBP_SC_PIN));
              ConvertFilesBMP();


#ifdef GBP_FEATURE_USING_RISING_CLOCK_ONLY_ISR
              attachInterrupt(digitalPinToInterrupt(GBP_SC_PIN), serialClock_ISR, RISING);  // attach interrupt handler
#else
              attachInterrupt(digitalPinToInterrupt(GBP_SC_PIN), serialClock_ISR, CHANGE);  // attach interrupt handler
#endif
            }
          }
        } else {
          if (buttonActive == true) {
            if (longPressActive == true) {
              longPressActive = false;
            } else {
              delay(500);
              if ((totalMultiImages - 1) > 1 && chkMargin == 0 && isPrinting) {
                Serial.println("Getting next file ID");

#ifdef LED_STATUS_PIN
                //LED_led_ON(LED_STATUS_PIN);
#endif

                delay(1000);
                dumpCount = update_get_dumps(1);
                freeFileIndex = update_get_next_ID(1);
                ResetPrinterVariables();

#ifdef LED_STATUS_PIN
                //LED_led_OFF(LED_STATUS_PIN);
#endif
              }
            }
            buttonActive = false;
          }
        }
      }
    } else {
    }
  }
}
