/*
 *      readWriteImage.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      Implementation for reading a PPM image from a file, trimming it to even
 *      dimensions, and writing a PPM image back to standard output.
 */

#include <stdlib.h>
#include <assert.h>

#include "readWriteImage.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "a2methods.h"

#define BLOCKSIZE 2

/* Initialize helper functions, see function contracts below */
static Pnm_ppm trimImage(Pnm_ppm img, A2Methods_mapfun *map);
static void copyPixel(int col, int row, A2Methods_UArray2 newArray, void *elem,
                      void *cl);

/******** copyClosure struct ********
 *
 * A struct to pass necessary data (a closure) into the apply function of a
 * mapping function. Used for trimming the image.
 *
 * Fields:
 *      Pnm_ppm oldImg: A pointer to the original, untrimmed image from which
 *                        pixels will be copied.
 ************************/
struct copyClosure
{
        Pnm_ppm oldImg;
};

/******** readImage ********
 *
 * Opens and reads a PPM image file, then trims it to have even width and
 * height.
 *
 * Parameters:
 *      FILE *fp:       A file pointer to the input PPM image stream
 * Returns:
 *      A Pnm_ppm struct pointer containing the trimmed image data.
 * Expects:
 *      fp is not NULL.
 *      fp points to a valid, open PPM image file.
 * Notes:
 *      Throws a CRE if fp is NULL.
 *      Throws a CRE if methods or map functions fail.
 *      Throws a CRE if Pnm_ppmread fails (e.g., NULL image).
 *      Calls helper function trimImage, which handles trimming and freeing the
 *        original image memory if trimming occurs.
 ************************/
Pnm_ppm readImage(FILE *fp)
{
        assert(fp != NULL);
        
        /* Default to blocked methods to prepare for 2x2 pixel blocks */
        A2Methods_T methods = uarray2_methods_blocked;
        assert(methods != NULL);
        
        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);
        
        /* Read the image using the pnm interface */
        Pnm_ppm img = Pnm_ppmread(fp, methods);
        assert(img != NULL);

        /* Call helper function to get even dimensions */
        return trimImage(img, map);
}

/******** trimImage ********
 *
 * Trims an image to the largest possible even width and height. If the image
 * already has even dimensions, it is returned unmodified. Otherwise, a new,
 * smaller image is created, the pixels are copied, and the original image is
 * freed.
 *
 * Parameters:
 *      Pnm_ppm oldImg:         A pointer to the original image structure
 *      A2Methods_mapfun *map:  The mapping function to use for copying pixels
 * Returns:
 *      A Pnm_ppm struct pointer to the (potentially new) trimmed image.
 * Expects:
 *      oldImg is not NULL and contains valid PPM data.
 *      map is not NULL.
 * Notes:
 *      Throws a CRE if memory allocation for newImg fails.
 *      Throws a CRE if the pixel array of newImg fails.
 *      If trimming is necessary, this function allocates memory for a new
 *        Pnm_ppm struct and its pixel array, which the caller is responsible
 *        for freeing.
 *      This function will trim at most 1 row and 1 column from the original
 *        image.
 *      Frees the original oldImg and its pixel array if a new image is created.
 ************************/
static Pnm_ppm trimImage(Pnm_ppm oldImg, A2Methods_mapfun *map)
{
        assert(oldImg != NULL);
        assert(oldImg->methods != NULL);
        assert(map != NULL);

        int newWidth = ((int) oldImg->width / BLOCKSIZE) * BLOCKSIZE;
        int newHeight = ((int) oldImg->height / BLOCKSIZE) * BLOCKSIZE;
        /* If dimensions are unchanged, no trimming is needed */
        if (newWidth == (int) oldImg->width &&
            newHeight == (int) oldImg->height) {
                return oldImg;
        }

        /* Create new image to hold the trimmed version */
        Pnm_ppm newImg = malloc(sizeof(*newImg));
        assert(newImg != NULL);
        
        /* Populate new image's struct from old image, but use new dimensions */
        newImg->denominator = oldImg->denominator;
        newImg->methods = oldImg->methods;
        newImg->width = newWidth;
        newImg->height = newHeight;
        newImg->pixels = oldImg->methods->new(newWidth, newHeight, 
                                              sizeof(struct Pnm_rgb));
        assert(newImg->pixels != NULL);

        /* Set up closure with pointer to the old image for copying */
        struct copyClosure closure = { .oldImg = oldImg };
        
        /* Copy the old image, ignoring ("trimming") the last odd row/column */
        map(newImg->pixels, copyPixel, &closure);

        /* Free old image's pixel and struct */
        oldImg->methods->free(&(oldImg->pixels));
        free(oldImg);

        return newImg;
}

/******** copyPixel ********
 *
 * Apply function used by trimImage. Copies a single pixel from the original
 * (larger) image to the corresponding position in the new (smaller) destination
 * array.
 *
 * Parameters:
 *      int col:        Column index of current pixel in new array
 *      int row:        Row index of current pixel in new array
 *      A2Methods_UArray2 newArray:     Pointer to the new (destination) array
 *      void *elem:     Pointer to curr pixel (Pnm_rgb) in newArray
 *      void *cl:       Closure w/ copyClosure struct w/ old image data
 * Returns:
 *      Nothing.
 * Expects:
 *      cl is not NULL and points to a valid copyClosure struct.
 *      elem is not NULL and points to a valid Pnm_rgb pixel.
 *      The oldImg inside cl is valid and contains a pixel at (col, row).
 * Notes:
 *      Modifies the destination array (via elem) by copying the pixel value
 *        from the old image.
 *      Throws a CRE if the pixel at (col, row) in the old image is NULL.
 ************************/
static void copyPixel(int col, int row, A2Methods_UArray2 newArray, 
                       void *elem, void *cl)
{
        (void) newArray;
        assert(elem != NULL);
        assert(cl != NULL);

        /* Get the source image using closure */
        struct copyClosure *closure = cl;
        assert(closure->oldImg != NULL);
        assert(closure->oldImg->methods != NULL);
        assert(closure->oldImg->pixels != NULL);

        Pnm_rgb newPixel = elem;
        
        /* Performs the pixel map */
        Pnm_rgb oldPixel = closure->oldImg->methods->at(closure->oldImg->pixels,
                                                        col, row);
        assert(oldPixel != NULL);
        
        /* Copy the RGB values directly */
        *newPixel = *oldPixel;
}

/******** writeImage ********
 *
 * Writes a Pnm_ppm image to standard output in the PPM plain format.
 *
 * Parameters:
 *      Pnm_ppm pixmap: A pointer to the Pnm_ppm struct to be written
 * Returns:
 *      Nothing.
 * Expects:
 *      pixmap is not NULL and contains valid image data.
 * Notes:
 *      Wraps the Pnm_ppmwrite function.
 ************************/
void writeImage(Pnm_ppm pixmap)
{
        assert(pixmap != NULL);
        assert(pixmap->methods != NULL);
        assert(pixmap->pixels != NULL);

        Pnm_ppmwrite(stdout, pixmap);
}
