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

void png_upscaler(char BMP_input[], char PNG_output[], unsigned int scale_factor, unsigned char BMP_palette[]) {

  //Serial.print("Trying to load: ");
  //Serial.println(input);
  File file = SD.open(BMP_input);
  if (!file) {
    //Serial.print(input);
    //Serial.println(": this file does not exist");
  }
  //Serial.println("Upscaling to indexed PNG");
  byte header[54];  //read the image source header
  file.read(header, 54);
  unsigned long BMP_size = ((header[5] << 24) | (header[4] << 16) | (header[3] << 8) | (header[2]));
  Serial.print("BMPsize=");
  Serial.println(BMP_size, DEC);
  unsigned long STARToffset = ((header[13] << 24) | (header[12] << 16) | (header[11] << 8) | (header[10]));
  Serial.print("STARToffset=");
  Serial.println(STARToffset, DEC);
  unsigned int BMP_w = ((header[21] << 24) | (header[20] << 16) | (header[19] << 8) | (header[18]));
  Serial.print("w=");
  Serial.println(BMP_w, DEC);
  unsigned int BMP_h = -((header[25] << 24) | (header[24] << 16) | (header[23] << 8) | (header[22]));
  Serial.print("h=");
  Serial.println(BMP_h, DEC);
  unsigned int PNG_width = BMP_w * scale_factor;
  unsigned int PNG_height = BMP_h * scale_factor;
  //We choose to encode an indexed png, palette is 3*0xFF long, this is the default mode in this library
  uint8_t PNG_Palette[768] = { BMP_palette[0], BMP_palette[0], BMP_palette[0], BMP_palette[1], BMP_palette[1], BMP_palette[1], BMP_palette[2], BMP_palette[2], BMP_palette[2], BMP_palette[3], BMP_palette[3], BMP_palette[3] };  // palette entered by RGB triplets
  uint8_t PNG_Line[PNG_width];
  uint8_t pixel;
  uint8_t octet;
  unsigned long index;
  file.seek(STARToffset);
  png.open(PNG_output, myOpen, myClose, myRead, myWrite, mySeek);
  png.encodeBegin(PNG_width, PNG_height, PNG_PIXEL_INDEXED, 8, PNG_Palette, 3);
  for (unsigned int y = 0; y < BMP_h; y++) {  //treat a line
    index = 0;
    for (unsigned int x = 0; x < BMP_w; x++) {  //start a line
      pixel = file.read();
      if (pixel == BMP_palette[3]) octet = 0x03;
      if (pixel == BMP_palette[2]) octet = 0x02;
      if (pixel == BMP_palette[1]) octet = 0x01;
      if (pixel == BMP_palette[0]) octet = 0x00;
      for (unsigned int i = 0; i < scale_factor; i++) {
        PNG_Line[index] = octet;  //stack identical pixels
        index = index + 1;
      }
    }
    for (unsigned int j = 0; j < scale_factor; j++) {
      png.addLine(PNG_Line);  //stack identical lines
    }
  }
  png.close();   //close PNG file
  file.close();  //close BMP file
}
