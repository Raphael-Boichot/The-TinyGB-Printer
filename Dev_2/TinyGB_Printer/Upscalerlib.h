//PNGenc is very practical as it works line by line, avoiding using huge amount of memory in the encodig process
#include <PNGenc.h>  //for png encoding
PNG png;             // static instance of the PNG encoder class
File myfile;

void *myOpen(const char *filename) {
  myfile = SD.open(filename, "w+");  // IMPORTANT!!! - don't use FILE_WRITE because it includes O_APPEND
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

void png_upscaler(char BMP_input[], char PNG_output[], unsigned int upscaling_factor, unsigned char BMP_palette[]) {
  bool skip = 0;
  File BMP_file = SD.open(BMP_input);
  if (!BMP_file) {
    bool skip = 1;
    Serial.println(": This BMP file does not exist, skipping png fabrication");
  }

  if (skip == 0) {
    //Serial.println("Upscaling to indexed PNG");
    uint8_t header[54];  //read the image source header
    BMP_file.read(header, 54);
    //unsigned long BMP_size = ((header[5] << 24) | (header[4] << 16) | (header[3] << 8) | (header[2])); //not necessary
    unsigned long STARToffset = ((header[13] << 24) | (header[12] << 16) | (header[11] << 8) | (header[10]));
    unsigned int BMP_w = ((header[21] << 24) | (header[20] << 16) | (header[19] << 8) | (header[18]));
    unsigned int BMP_h = -((header[25] << 24) | (header[24] << 16) | (header[23] << 8) | (header[22]));
    unsigned int PNG_width = BMP_w * upscaling_factor;
    unsigned int PNG_height = BMP_h * upscaling_factor;
    //We choose to encode an indexed png, palette is 3*0xFF long, this is the default mode here
    uint8_t PNG_Palette[768] = {BMP_palette[0], BMP_palette[0], BMP_palette[0], 
                                BMP_palette[1], BMP_palette[1], BMP_palette[1], 
                                BMP_palette[2], BMP_palette[2], BMP_palette[2], 
                                BMP_palette[3], BMP_palette[3], BMP_palette[3] };  // palette entered by RGB triplets
    uint8_t PNG_Line[PNG_width];
    uint8_t pixel_gray_level;
    uint8_t color_index;
    unsigned long index;
    BMP_file.seek(STARToffset);// skip the 1072 bytes header and start from pixel_gray_level data
    png.open(PNG_output, myOpen, myClose, myRead, myWrite, mySeek);//mandatory call
    png.encodeBegin(PNG_width, PNG_height, PNG_PIXEL_INDEXED, 8, PNG_Palette, 3);//format per se, documentation here: https://github.com/bitbank2/PNGenc/wiki/API
    for (unsigned int y = 0; y < BMP_h; y++) {  //treats a line
      index = 0;
      for (unsigned int x = 0; x < BMP_w; x++) {  //starts a line
        pixel_gray_level = BMP_file.read();//one byte = one pixel_gray_level
        if (pixel_gray_level == BMP_palette[3]) color_index = 0x03;//converts BMP gray level to color PNG color index
        if (pixel_gray_level == BMP_palette[2]) color_index = 0x02;//converts BMP gray level to color PNG color index
        if (pixel_gray_level == BMP_palette[1]) color_index = 0x01;//converts BMP gray level to color PNG color index
        if (pixel_gray_level == BMP_palette[0]) color_index = 0x00;//converts BMP gray level to color PNG color index
        for (unsigned int i = 0; i < upscaling_factor; i++) {  //stacks n identical pixel_gray_levels for upscaling
          PNG_Line[index] = color_index;
          index = index + 1;
        }
      }
      for (unsigned int j = 0; j < upscaling_factor; j++) {  //stacks n identical lines for upscaling
        png.addLine(PNG_Line);//the library is made to work line by line, which is cool regarding memory management
      }
    }
    png.close();       //closes PNG file
    BMP_file.close();  //closes BMP file
  }
}