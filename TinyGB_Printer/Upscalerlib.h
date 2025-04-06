#include <PNGenc.h>  //for png encoding
PNG png;             // static instance of the PNG encoder class
File myfile;

//All this part must be kept intact ! I would like to get rid of it eventually
void *myOpen(const char *filename) {
  myfile = SD.open(filename, O_READ | O_WRITE | O_CREAT);
  return &myfile;
}
void myClose(PNGFILE *handle) {
  File *f = (File *)handle->fHandle;
  f->close();
}
int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length) {
  File *f = (File *)handle->fHandle;
  return f->read(buffer, length);
}
int32_t myWrite(PNGFILE *handle, uint8_t *buffer, int32_t length) {
  File *f = (File *)handle->fHandle;
  return f->write(buffer, length);
}
int32_t mySeek(PNGFILE *handle, int32_t position) {
  File *f = (File *)handle->fHandle;
  return f->seek(position);
}


//Upscaling factor MUST be 4 for the moment, it is let as variable but it is not. I think this is the perfect value
//In any case, do not modify
void png_upscaler(char DATA_input[], char PNG_output[], unsigned char PNG_palette_RGB[], unsigned long lines_in_DATA_file) {
  unsigned long myTime;
  bool skip = 0;
  File DATA_file = SD.open(DATA_input);
  if (!DATA_file) {
    bool skip = 1;  //file not found, skip next step
  }

  if (skip == 0) {
    myTime = millis();
    int rc, iDataSize;
    unsigned int upscaling_factor = 4;
    unsigned int PNG_width = 160 * upscaling_factor;  // images have fixed width, pratical
    unsigned int PNG_height = lines_in_DATA_file * upscaling_factor;
    uint8_t PNG_Line[PNG_width];
    uint8_t pixel_gray_level;
    uint8_t color_index;
    uint8_t Compression_level = 3;  //1 least=fast, 9 most=slow
    uint8_t bits_per_pixel = 2;     //assuming an upscaling factor of 4, 4 pixels are stored for each byte. In 8 bpp, each byte is an entry in the index table (can be 2, 4 or 8)
    unsigned long index;
    // palette is BGR for indexed image (see library wiki https://github.com/bitbank2/PNGenc/wiki/API).
    // [768] in PNGenc because the PNG upscaler requires a full 3*0xFF BGR palette.
    unsigned char PNG_palette_BGR[768] = { PNG_palette_RGB[2], PNG_palette_RGB[1], PNG_palette_RGB[0],      //White
                                           PNG_palette_RGB[5], PNG_palette_RGB[4], PNG_palette_RGB[3],      //Light gray
                                           PNG_palette_RGB[8], PNG_palette_RGB[7], PNG_palette_RGB[6],      //Dark gray
                                           PNG_palette_RGB[11], PNG_palette_RGB[10], PNG_palette_RGB[9] };  //black
    rc = png.open(PNG_output, myOpen, myClose, myRead, myWrite, mySeek);
    // PNG_PIXEL_GRAYSCALE - 8-bpp grayscale - No palette needed
    // PNG_PIXEL_TRUECOLOR - 24-bpp (8x3) RGB triplets - No palette needed
    // PNG_PIXEL_INDEXED - 1 to 8-bpp palette color - a palette of BGR triplets, not RGB !!!
    // PNG_PIXEL_GRAY_ALPHA - 16-bpp (8-bit gray + 8-bit alpha) - No palette needed
    // PNG_PIXEL_TRUECOLOR_ALPHA - 32-bpp (RGB8888) - No palette needed
    rc = png.encodeBegin(PNG_width, PNG_height, PNG_PIXEL_INDEXED, bits_per_pixel, PNG_palette_BGR, Compression_level);
    //format per se, documentation here: https://github.com/bitbank2/PNGenc/wiki/API
    //png.setAlphaPalette(ucAlphaPal);                                                    //left empty
    for (unsigned int y = 0; y < lines_in_DATA_file; y++) {  //treats a line
      //each line in stored data is yet a full 4x pixels line in 2bbp, so no upscaling factor here
      DATA_file.read(PNG_Line, 160);
      //in the data buffer file, each series of 160 bytes is a pixel line 4x upscaled
      for (unsigned int j = 0; j < upscaling_factor; j++) {  //stacks 4 identical lines for upscaling
        rc = png.addLine(PNG_Line);                          //the library is made to work line by line, which is cool regarding memory management
      }
    }
    DATA_file.close();        //closes BMP file
    iDataSize = png.close();  //closes PNG file
  }
}
