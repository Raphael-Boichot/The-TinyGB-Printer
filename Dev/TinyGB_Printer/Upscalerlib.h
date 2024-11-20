#include <PNGenc.h> //for png encoding
PNG png; // static instance of the PNG encoder class
File myfile;

void * myOpen(const char *filename) {
  //Serial.printf("Attempting to open %s\n", filename);
  // IMPORTANT!!! - don't use FILE_WRITE because it includes O_APPEND
  // this will cause the file to be written incorrectly
  myfile = SD.open(filename, "w+");
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

void png_upscaler(char input[], char output[], int scale_factor) {
  File file = SD.open(input);
  byte header[54];//read the image source header
  file.read(header, 54);
  unsigned long BMPsize = ((header[5] << 24) | (header[4] << 16) | (header[3] << 8) | (header[2]));
  unsigned long STARToffset = ((header[13] << 24) | (header[12] << 16) | (header[11] << 8) | (header[10]));
  unsigned w = ((header[21] << 24) | (header[20] << 16) | (header[19] << 8) | (header[18]));
  unsigned h = -((header[25] << 24) | (header[24] << 16) | (header[23] << 8) | (header[22]));
  unsigned up_w = w * scale_factor;
  unsigned up_h = h * scale_factor;
  unsigned WIDTH = up_w;
  unsigned HEIGHT = up_h;
  uint8_t ucPal[768] = {0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xFF, 0xFF, 0xFF}; // palette entered by triplets
  int rc, iDataSize, x, y;
  uint8_t ucLine[WIDTH];
  byte junk;
  byte pixel;
  byte octet;
  byte normal_line[w];
  unsigned long index;

  rc = png.open(output, myOpen, myClose, myRead, myWrite, mySeek);
  if (rc == PNG_SUCCESS) {
    rc = png.encodeBegin(WIDTH, HEIGHT, PNG_PIXEL_INDEXED, 8, ucPal, 3);
    if (rc == PNG_SUCCESS) {
      for (int y = 0; y < HEIGHT && rc == PNG_SUCCESS; y++) {
        index = 0;
        for (unsigned k = 0; k < w; k++) {
          pixel = file.read();
          junk = file.read();
          junk = file.read();
          if (pixel == 0xFF) octet = 0x03;
          if (pixel == 0xAA) octet = 0x02;
          if (pixel == 0x55) octet = 0x01;
          if (pixel == 0x00) octet = 0x00;
          for (unsigned i = 0; i < scale_factor; i++) {
            ucLine[index] = octet;
            index = index + 1;
          }
        }
        for (unsigned j = 0; j < scale_factor; j++) {
          rc = png.addLine(ucLine);
        }
      }
      iDataSize = png.close();
    }
  } 
  file.close();
}

