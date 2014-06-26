#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <sys/mman.h>
#import <string.h>


//128 × 112
#define GBPIC_WIDTH 128
#define GBPIC_HEIGHT 112
#define GBPIC_DEPTH 2

unsigned char *savedata;

typedef struct {
    unsigned char pixelData[8];
} tile_byte;


tile_byte proc_pixelData(unsigned char byte1, unsigned char byte2) {

    unsigned char a, b, c;
    tile_byte tile = {};

    for(int i = 0 ; i < 8; i++){

        a = byte1 & (unsigned char)0x80;
        b = byte2 & (unsigned char)0x80;

        a = a >> 7;
        b = b >> 6;

        c = a | b;
        tile.pixelData[i] = ( c ^ (unsigned char)0x03 ); //tile data is inverse of what png expects

        byte1 = byte1 << 1;
        byte2 = byte2 << 1;
    }


    return tile;
}

int imageAtAddressToFilePath(size_t image_address, char *out_path) {

    FILE *out_file = fopen(out_path, "wb");

    png_structp png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop pngInfo = png_create_info_struct (png);



    png_set_IHDR (png,                  pngInfo, GBPIC_WIDTH, GBPIC_HEIGHT, GBPIC_DEPTH,
                  PNG_COLOR_TYPE_GRAY,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);

    unsigned char byte1, byte2;

    //setup pixel data structure
    png_bytepp row_pointers = png_malloc (png, GBPIC_HEIGHT * sizeof (png_byte *));
    for (int y = 0; y < GBPIC_HEIGHT; ++y) {
        png_byte *row = png_malloc (png, sizeof (uint8_t) * GBPIC_WIDTH);
        row_pointers[y] = row;
    }

    for(int ytile = 0; ytile < 14; ytile++){
        for(int xtile = 0; xtile < 16; xtile++){

            for(int y = 0; y < 8; y++){
//                for(int x = 0; x < 8; x += 4){

                    byte1 = savedata[image_address++];
                    byte2 = savedata[image_address++];
                    tile_byte tile = proc_pixelData(byte1, byte2);

                memcpy(&row_pointers[(ytile * 8) + y][(xtile * 8)], &tile.pixelData, 8);

//                    for(int i = 0; i < 8; i++) {
//                        row_pointers[(ytile * 8) + y][(xtile * 8) +  i] = tile.pixelData[i];
//                    }

//                }
            }

        }
    }

    /* Write the image data to "fp". */

    png_init_io (png, out_file);
    png_set_rows (png, pngInfo, row_pointers);
    png_write_png (png, pngInfo, PNG_TRANSFORM_PACKING, NULL);
//    png_write_png(png, pngInfo, PNG_TRANSFORM_IDENTITY, NULL);

    return 0;
}

int main ()
{
    char *in_path = "GAMEBOYCAMERA.sav";

    //    if ((fdin = open (argv[1], O_RDONLY)) < 0)
    int fdin = open(in_path, O_RDONLY);

    struct stat statbuf;
    fstat(fdin, &statbuf);

    savedata = mmap(0, (size_t)statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0);


    size_t savefilep = 0x2000;

    imageAtAddressToFilePath(savefilep, "test.png");

    return 0;
}
