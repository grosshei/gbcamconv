#define _GNU_SOURCE
#include <stdio.h>

#include <png.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>


//128 × 112
#define GBPIC_WIDTH 128
#define GBPIC_HEIGHT 112
#define GBPIC_DEPTH 2

unsigned char *savedata;

typedef struct {
    unsigned char data[8];
} tile_row;


tile_row tile_bytes_to_row_data(unsigned char byte1, unsigned char byte2) {

    unsigned char a, b, c;
    tile_row tile = {};

    for(int i = 0 ; i < 8; i++){

        a = byte1 & (unsigned char)0x80;
        b = byte2 & (unsigned char)0x80;

        a = a >> 7;
        b = b >> 6;

        c = a | b;
        tile.data[i] = ( c ^ (unsigned char)0x03 ); //tile data is inverse of what png expects

        byte1 = byte1 << 1;
        byte2 = byte2 << 1;
    }

    return tile;
}

int save_gameboy_image(size_t image_address, char *out_path) {

    FILE *out_file = fopen(out_path, "wb");

    png_structp png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop pngInfo = png_create_info_struct (png);

    png_set_IHDR(png, pngInfo, GBPIC_WIDTH, GBPIC_HEIGHT, GBPIC_DEPTH,
        PNG_COLOR_TYPE_GRAY,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    unsigned char byte1, byte2;

    //setup pixel data structure
    png_bytepp row_pointers = png_malloc (png, GBPIC_HEIGHT * sizeof(png_byte *));
    for (int y = 0; y < GBPIC_HEIGHT; ++y) {
        png_byte *row = png_malloc (png, sizeof(unsigned char) * GBPIC_WIDTH);
        row_pointers[y] = row;
    }

    //populate png data by iterating over tiles
    for (int ytile = 0; ytile < 14; ytile++) {
        for (int xtile = 0; xtile < 16; xtile++) {

            for (int y = 0; y < 8; y++) {
                byte1 = savedata[image_address++];
                byte2 = savedata[image_address++];
                tile_row tile = tile_bytes_to_row_data(byte1, byte2);
                memcpy(&row_pointers[(ytile * 8) + y][(xtile * 8)], &tile.data, 8);
            }
        }
    }

    png_init_io (png, out_file);
    png_set_rows (png, pngInfo, row_pointers);
    png_write_png (png, pngInfo, PNG_TRANSFORM_PACKING, NULL);
//    png_write_png(png, pngInfo, PNG_TRANSFORM_IDENTITY, NULL);

    return 0;
}

int main(int argc, char *argv[]) {
    char *in_path;

    if(argc > 1) {
        in_path = argv[1];
    }else{
        printf("Usage: %s <gameboy camera save file>\n", argv[0]);
        exit(-1);
    }

    int fdin = open(in_path, O_RDONLY);
    if(fdin < 0){
        printf("Error: cannot open %s\n", in_path);
        exit(-1);
    }

    struct stat statbuf;
    fstat(fdin, &statbuf);

    savedata = mmap(0, (size_t)statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0);

    //loop over all images 1-30
    //0x200 - 0x1f000
    //exit if pointer exceeds file size (truncated save file, etc)

    for(size_t savefilep = 0x2000, i = 0; savefilep < statbuf.st_size && savefilep <= 0x1f000; savefilep += 0x1000, i++) {
        char *file_path;
        asprintf(&file_path, "out_%02zu.png", i );
        save_gameboy_image(savefilep, file_path);
        printf("Image %02u @ %x saved as %s\n", (unsigned int)(i + 1), (unsigned int)savefilep, file_path);
    }

    return 0;
}
