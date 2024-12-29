## How to configure the Bodmer TFT library

- Locate the TFT_eSPI library: **\Arduino\libraries\TFT_eSPI** folder in your Arduino libraries
- copy the configuration file for the TFT display in this folder.
- edit the User_Setup_Select.h and modify line 29:
    **#include <TinyGB_240x240.h> // Default setup is root library folder**
  
- edit the 


- Locate the PNGenc library **\Arduino\libraries\PNGenc\src\PNGenc.h** folder in your Arduino libraries

[https://github.com/Bodmer/TFT_eSPI/issues/3476](https://github.com/Bodmer/TFT_eSPI/issues/3476)

just read this [note](https://github.com/Raphael-Boichot/The-TinyGB-Printer/blob/59753096baee4126f26321b7315d2f6e0639d5b6/TinyGB_Printer/Upscalerlib.h#L1)
