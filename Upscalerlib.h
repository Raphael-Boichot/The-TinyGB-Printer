//Very important: in /src/PNGenc.h, increase PNG_FILE_BUF_SIZE to 16384 intead of the 2048 by default which is way too short !
//modify this manually in your local install of the PNGenc library, there is no call to do this automatically
#include <PNGenc.h>  //for png encoding
PNG png;             // static instance of the PNG encoder class
File myfile;

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

//Upscaling factor MUST be 4 for the moment
void png_upscaler(char BMP_input[], char PNG_output[], unsigned int upscaling_factor, unsigned char PNG_palette[], unsigned long lines_in_bmp_file) {
  unsigned long myTime;
  bool skip = 0;
  File BMP_file = SD.open(BMP_input);
  if (!BMP_file) {
    bool skip = 1;
    Serial.println("Core 1 -> This BMP file does not exist, skipping png fabrication");
  }

  if (skip == 0) {
    //Serial.print("Core 1 -> Creating ");
    //Serial.println(PNG_output);
    myTime = millis();
    int rc, iDataSize;
    unsigned int PNG_width = 160 * upscaling_factor;
    unsigned int PNG_height = lines_in_bmp_file * upscaling_factor;
    //We choose to encode an indexed png, palette is 3*0xFF long, this is the default mode here
    //It's possible to pass colors but it's easier to use another software then
    uint8_t PNG_Palette[768] = { PNG_palette[0], PNG_palette[0], PNG_palette[0],
                                 PNG_palette[1], PNG_palette[1], PNG_palette[1],
                                 PNG_palette[2], PNG_palette[2], PNG_palette[2],
                                 PNG_palette[3], PNG_palette[3], PNG_palette[3] };  // palette entered by RGB triplets
    uint8_t PNG_Line[PNG_width];
    uint8_t pixel_gray_level;
    uint8_t color_index;
    uint8_t Compression_level = 3;  //1 least=fast, 9 most=slow
    uint8_t bits_per_pixel = 2;  //assuming an upscaling factor of 4, 4 pixels are stored for each byte;
    unsigned long index;
    rc = png.open(PNG_output, myOpen, myClose, myRead, myWrite, mySeek);  //mandatory call
    rc = png.encodeBegin(PNG_width, PNG_height, PNG_PIXEL_INDEXED, bits_per_pixel, PNG_Palette, Compression_level);
    //format per se, documentation here: https://github.com/bitbank2/PNGenc/wiki/API
    //png.setAlphaPalette(ucAlphaPal);                                                    //left empty
    for (unsigned int y = 0; y < lines_in_bmp_file; y++) {  //treats a line
      //each line in BMP image is yet a full 4x line in 2bbp
      BMP_file.read(PNG_Line, 160);

      for (unsigned int j = 0; j < upscaling_factor; j++) {  //stacks n identical lines for upscaling
        rc = png.addLine(PNG_Line);                          //the library is made to work line by line, which is cool regarding memory management
      }
    }
    BMP_file.close();         //closes BMP file
    iDataSize = png.close();  //closes PNG file
    //Serial.print("Core 1 -> PNG closed, encoding time (ms): ");
    //Serial.println(millis() - myTime, DEC);
  }
}
