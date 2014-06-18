/// \file
#include <png.h>
#include <stdio.h>
#include "pixeltypes.h"
/// Given "bitmap", this returns the pixel of bitmap at the point ("x", "y").
pixel_t __attribute__((const,pure)) * pixel_at (bitmap_t * bitmap, const unsigned  int x, const unsigned int y)
{
    return bitmap->pixels + bitmap->width * y + x;
}

/// Write "bitmap" to a PNG file specified by "path"
int save_png_to_file (bitmap_t *bitmap, const char *path)
{
    FILE * fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    unsigned int x, y;
    png_byte ** row_pointers = NULL;
    /* The following number is set by trial and error only. I cannot
       see where it it is documented in the libpng manual.
    */
    unsigned int pixel_size = 3;
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
                  (png_uint_32)bitmap->width,
                  (png_uint_32)bitmap->height,
                  depth,
                  PNG_COLOR_TYPE_RGB,
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
    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    for (y = 0; y < bitmap->height; y++) {
        png_free (png_ptr, row_pointers[y]);
    }
    png_free (png_ptr, row_pointers);
//cleanup
 png_failure:
 png_create_info_struct_failed:
    png_destroy_write_struct (&png_ptr, &info_ptr);
 png_create_write_struct_failed:
    fclose (fp);
 fopen_failed:
    return 0;
}
