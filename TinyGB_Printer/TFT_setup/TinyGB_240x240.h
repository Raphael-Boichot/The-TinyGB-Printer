//                            USER DEFINED SETTINGS
//   Set driver type, fonts to be loaded, pins used and SPI control method etc
//
//   See the User_Setup_Select.h file if you wish to be able to define multiple
//   setups and then easily select which setup file is used by the compiler.
//
//   If this file is edited correctly then all the library example sketches should
//   run without the need to make any more changes for a particular hardware setup!
//   Note that some sketches are designed for a particular TFT pixel width/height
//SETUP for the 128x160 version
//#define ST7735_DRIVER      // Define additional parameters below for this display
//#define TFT_RGB_ORDER TFT_RGB  // Colour order Red-Green-Blue
//#define TFT_WIDTH  128
//#define TFT_HEIGHT 160
//#define ST7735_REDTAB

//SETUP for the 240x240 version
#define ST7789_DRIVER      // Full configuration option, define additional parameters below for this display
#define TFT_RGB_ORDER TFT_BGR  // Colour order Blue-Green-Red
#define TFT_WIDTH  240 // ST7789 240 x 240 and 240 x 320
#define TFT_HEIGHT 240 // ST7789 240 x 240
//#define RP2040_PIO_SPI

//other universal parameters
#define USER_SETUP_ID 60
#define TFT_MISO  99  // not used here, just to avoid a compiling message
#define TOUCH_CS  99  // not used here, just to avoid a compiling message
#define TFT_SCLK  6  // Display SCK
#define TFT_MOSI  7  // Display SDA/MOSI
#define TFT_CS    1  // Chip select control pin
#define TFT_DC    0  // Display DC (RS/AO) - Data Command control pin
#define TFT_RST   5  // Display RESET - Reset pin (could connect to Arduino RESET pin)
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts
#define SMOOTH_FONT
#define SPI_FREQUENCY  70000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

