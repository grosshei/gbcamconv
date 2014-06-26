#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <sys/mman.h>
#import <string.h>


/* A coloured pixel. */

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} pixel_t;

/* A picture. */

typedef struct  {
    pixel_t *pixels;
    size_t width;
    size_t height;
} bitmap_t;

/* Given "bitmap", this returns the pixel of bitmap at the point
 ("x", "y"). */

static pixel_t * pixel_at (bitmap_t * bitmap, int x, int y)
{
    return bitmap->pixels + bitmap->width * y + x;
}

/* Write "bitmap" to a PNG file specified by "path"; returns 0 on
 success, non-zero on error. */

static int save_png_to_file (bitmap_t *bitmap, const char *path)
{
    FILE * fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_byte ** row_pointers = NULL;
    /* "status" contains the return value of this function. At first
     it is set to a value which means 'failure'. When the routine
     has finished its work, it is set to a value which means
     'success'. */
    int status = -1;
    /* The following number is set by trial and error only. I cannot
     see where it it is documented in the libpng manual.
     */
    int pixel_size = 3;
    int depth = 8;
    
    fp = fopen (path, "wb");
    if (! fp) {
        goto fopen_failed;
    }
    
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        goto png_create_write_struct_failed;
    }
    
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {
        goto png_create_info_struct_failed;
    }
    
    /* Set up error handling. */
    
    if (setjmp (png_jmpbuf (png_ptr))) {
        goto png_failure;
    }
    
    /* Set image attributes. */
    
    png_set_IHDR (png_ptr,
                  info_ptr,
                  bitmap->width,
                  bitmap->height,
                  depth,
                  PNG_COLOR_TYPE_GRAY,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);
    
    /* Initialize rows of PNG. */
    
    row_pointers = png_malloc (png_ptr, bitmap->height * sizeof (png_byte *));
    for (y = 0; y < bitmap->height; ++y) {
        png_byte *row = png_malloc (png_ptr, sizeof (uint8_t) * bitmap->width * pixel_size);
        row_pointers[y] = row;
        for (x = 0; x < bitmap->width; ++x) {
            pixel_t * pixel = pixel_at (bitmap, x, y);
            *row++ = pixel->red;
            *row++ = pixel->green;
            *row++ = pixel->blue;
        }
    }
    
    /* Write the image data to "fp". */
    
    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    
    /* The routine has successfully written the file, so we set
     "status" to a value which indicates success. */
    
    status = 0;
    
    for (y = 0; y < bitmap->height; y++) {
        png_free (png_ptr, row_pointers[y]);
    }
    png_free (png_ptr, row_pointers);
    
png_failure:
png_create_info_struct_failed:
    png_destroy_write_struct (&png_ptr, &info_ptr);
png_create_write_struct_failed:
    fclose (fp);
fopen_failed:
    return status;
}

/* Given "value" and "max", the maximum value which we expect "value"
 to take, this returns an integer between 0 and 255 proportional to
 "value" divided by "max". */

static int pix (int value, int max)
{
    if (value < 0)
        return 0;
    return (int) (256.0 *((double) (value)/(double) max));
}

//128 × 112
#define GBPIC_WIDTH 128u
#define GBPIC_HEIGHT 112u
#define GBPIC_DEPTH 4

typedef struct {
    unsigned char pixelData[8];
} tile_byte;


//unsigned char greyscale[] = {0x00, 0x55, 0xAA, 0xFF};
unsigned char greyscale[] = {0xFF, 0xAA, 0x55, 0x00};

tile_byte proc_pixelData(unsigned char byte1, unsigned char byte2) {

    unsigned char a, b, c;
    tile_byte tile;

    for(int i = 0 ; i < 8; i++){

        a = byte1 & (unsigned char)0x80;
        b = byte2 & (unsigned char)0x80;

        a = a >> 7;
        b = b >> 6;

        c = a | b;
        tile.pixelData[i] = greyscale[c];

//        printf("%x %x | %u %u\n", byte1, byte2, a, b);

        byte1 = byte1 << 1;
        byte2 = byte2 << 1;
    }

//    unsigned char orig = byte1;
//    tile_byte a;
//    unsigned char t = byte1;
//    unsigned char asdf = 0x0;
//    for(int i = 0; i < 4; i++){
//        t = byte1 & (unsigned char)0xC0;
//        asdf = t;
//        t = t >> 6;
//        //printf("%02x | %u\n", input, t);
//        a.pixelData[i] = t;
//        byte1 = byte1 << 2;
//         //t & 0x3F
//    }
//        printf("%02x | %u %u %u %u %u\n", orig, a.pixelData[0], a.pixelData[1],a.pixelData[2],a.pixelData[3], asdf);


//    a.pixelData[0] =
    return tile;
}

int foo(){
    char *in_path = "GAMEBOYCAMERA.sav";
//    FILE *in_file = fopen(in_path, "r");

//    if ((fdin = open (argv[1], O_RDONLY)) < 0)
    int fdin = open(in_path, O_RDONLY);

    struct stat statbuf;
    fstat(fdin,&statbuf);

    unsigned char *savedata =  mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0);



    char *out_path = "test.png";

    FILE *out_file = fopen(out_path, "wb");

    png_structp png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop pngInfo = png_create_info_struct (png);



    png_set_IHDR (png,                  pngInfo, GBPIC_WIDTH, GBPIC_HEIGHT, GBPIC_DEPTH,
                  PNG_COLOR_TYPE_GRAY,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);

//    png_byte *row = png_malloc (png, sizeof (uint8_t) * GBPIC_WIDTH * GBPIC_HEIGHT);

//    unsigned char greyscale[] = {0xFF, 0xAA, 0x55, 0x00};

    size_t savefilep = 0x2000;
    unsigned char byte1, byte2;

    png_bytepp row_pointers = png_malloc (png, GBPIC_HEIGHT * sizeof (png_byte *));
    for (int y = 0; y < GBPIC_HEIGHT; ++y) {
        png_byte *row = png_malloc (png, sizeof (uint8_t) * GBPIC_WIDTH);
        row_pointers[y] = row;
        unsigned char tmp = 0xFF;
//        for (int x = 0; x < GBPIC_WIDTH; x = x+4) {
//            b = savedata[savefilep++];
//
//            tile_byte tile = proc_pixelData(b);
//
////            row[x] = greyscale[x % 4];
//            row[x] = tile.pixelData[0];
//            row[x + 1] = tile.pixelData[1];
//            row[x + 2] = tile.pixelData[2];
//            row[x + 3] = tile.pixelData[3];
//
//        }
    }

    for(int ytile = 0; ytile < 14; ytile++){
        for(int xtile = 0; xtile < 16; xtile++){

            for(int y = 0; y < 8; y++){
//                for(int x = 0; x < 8; x += 4){

                    byte1 = savedata[savefilep++];
                    byte2 = savedata[savefilep++];
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
    png_write_png (png, pngInfo, PNG_TRANSFORM_IDENTITY, NULL);

    return 0;
}

int main ()
{
    bitmap_t fruit;
    int x;
    int y;
    
    /* Create an image. */
    
    fruit.width = 100;
    fruit.height = 100;
    
    fruit.pixels = calloc (sizeof (pixel_t), fruit.width * fruit.height);
    
    for (y = 0; y < fruit.height; y++) {
        for (x = 0; x < fruit.width; x++) {
            pixel_t * pixel = pixel_at (& fruit, x, y);
            pixel->red = pix (x, fruit.width);
            pixel->green = pix (y, fruit.height);
        }
    }
    
    /* Write the image to a file 'fruit.png'. */
    
//    save_png_to_file (& fruit, "fruit.png");

    foo();

    return 0;
}
