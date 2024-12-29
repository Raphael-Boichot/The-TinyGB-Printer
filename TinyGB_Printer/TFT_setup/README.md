## How to configure the Bodmer TFT library

- Locate the TFT_eSPI library: **\Arduino\libraries\TFT_eSPI** folder in your Arduino libraries
- copy the configuration file for the TFT display in this folder.
- edit the User_Setup_Select.h and modify line 29:
    **#include <TinyGB_240x240.h> // Default setup is root library folder**
  
- edit the TFT_eSPI_RP2040.h and modify line 52
   **#define SET_BUS_READ_MODE  // spi_set_format(SPI_X,  8, (spi_cpol_t)0, (spi_cpha_t)0, SPI_MSB_FIRST)**

This correspond to this issue with this particulat TFT display: [https://github.com/Bodmer/TFT_eSPI/issues/3476](https://github.com/Bodmer/TFT_eSPI/issues/3476)

## How to configure the PNGenc

- Locate the PNGenc library **\Arduino\libraries\PNGenc\src\PNGenc.h** folder in your Arduino libraries
- edit the line 59:
   **#define PNG_FILE_BUF_SIZE 16384**

Now you're ready to compile !
