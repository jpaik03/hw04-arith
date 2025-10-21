/*
 *      pixelOperation.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      Implementation for converting pixel data between the scaled integer RGB
 *      color space and the floating-point based Component Video Color Space
 *      (CVCS). This module handles the C2 and (C2)' steps.
 */

#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "pixelOperation.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "a2methods.h"

#define T UArray2b_T
#define BLOCKSIZE 2

/* Our chosen denominator */
const unsigned DENOMINATOR = 255;

/* Initialize helper functions, see function contracts below */
static void applyPixelToCompVid(int col, int row, A2Methods_UArray2 pixels, 
                             void *elem, void *cl);
static void applyCompVidToPixel(int col, int row, A2Methods_UArray2 pixels, 
                             void *elem, void *cl);

/******** applyPixelToCompVidClosure struct ********
 *
 * A closure struct passed into the mapping function that converts a PPM
 * image with integer RGB pixels to a UArray2b of floating-point CVCS data.
 *
 * Fields:
 *      T RGBInfo:              The destination array of pixInfo structs
 *      Pnm_ppm img:            The source Pnm_ppm image
 *      A2Methods_T methods:    The method suite for array operations
 ************************/
struct applyPixelToCompVidClosure
{
        T RGBInfo;
        Pnm_ppm img;
        A2Methods_T methods;
};

/******** applyCompVidToPixelClosure struct ********
 *
 * A closure struct passed into the mapping function that converts a UArray2b of
 * CVCS data back to a PPM image with integer RGB pixels.
 *
 * Fields:
 *      Pnm_ppm pixmap:         The destination Pnm_ppm struct being populated
 *      T RGBInfo:              The source array of pixInfo structs
 *      A2Methods_T methods:    The method suite for array operations
 ************************/
struct applyCompVidToPixelClosure
{
        Pnm_ppm pixmap;
        T RGBInfo;
        A2Methods_T methods;
};

/******** getRGBCompVid ********
 *
 * Converts a Pnm_ppm image (with scaled integer RGB pixels) into a new UArray2b
 * of structs containing floating-point CVCS data.
 *
 * Parameters:
 *      Pnm_ppm img:            A pointer to the source Pnm_ppm image
 *      A2Methods_T methods:    The method suite for array operations
 * Returns:
 *      A UArray2b_T (defined as T) where each element is a pixInfo struct.
 * Expects:
 *      img and methods are not NULL.
 * Notes:
 *      Throws a CRE if img or methods is NULL.
 *      Throws a CRE if map function fails.
 *      Throws a CRE if memory allocation fails.
 *      Allocates memory for a new UArray2b, which the caller must free.
 ************************/
T getRGBCompVid(Pnm_ppm img, A2Methods_T methods)
{
        assert(img != NULL);
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);
        
        /* Create the destination array to hold floating-point CVCS data */
        T RGBInfo = methods->new_with_blocksize(img->width, img->height, 
                                                sizeof(struct pixInfo), 
                                                BLOCKSIZE);
        assert(RGBInfo != NULL);

        /* Set up the closure with source and destination arrays */
        struct applyPixelToCompVidClosure closure = {RGBInfo, img, methods};

        /* Map over the source image, applying the conversion to each pixel */
        map(img->pixels, applyPixelToCompVid, &closure);

        return RGBInfo;
}

/******** applyPixelToCompVid ********
 *
 * Apply function that converts a single Pnm_rgb pixel into a pixInfo struct. It
 * scales the integer RGB values to floats [0,1], then applies the linear
 * transformation to get Y, Pb, and Pr values.
 *
 * Parameters:
 *      int col:                        Column index of the current pixel
 *      int row:                        Row index of the current pixel
 *      A2Methods_UArray2 pixels:       The array being mapped over (unused)
 *      void *elem:                     Pointer to current Pnm_rgb pixel
 *      void *cl:                       Pointer to applyPixelToCompVidClosure
 * Returns:
 *      Nothing.
 * Expects:
 *      elem and cl are not NULL.
 * Notes:
 *      Modifies the destination array (RGBInfo) stored in the closure.
 ************************/
static void applyPixelToCompVid(int col, int row, A2Methods_UArray2 pixels, 
                                void *elem, void *cl) {
                        
        (void) pixels;
        struct applyPixelToCompVidClosure *closure = cl;
        Pnm_rgb pixel = elem;

        unsigned denom = closure->img->denominator;

        /* Convert scaled integers to floating-point values in [0,1] */
        float r = (float) pixel->red / denom;
        float g = (float) pixel->green / denom;
        float b = (float) pixel->blue / denom;

        /* Apply linear transformation from RGB to CVCS */
        float y = 0.299 * r + 0.587 * g + 0.114 * b;
        float pb = 0.5 * b - 0.168736 * r - 0.331264 * g;
        float pr = 0.5 * r - 0.418688 * g - 0.081312 * b;

        /* Get a pointer to the destination element in the new array */
        struct pixInfo *destVals = closure->methods->at(closure->RGBInfo, 
                                                        col, row);
        assert(destVals != NULL);

        /* Store the calculated CVCS values */
        *destVals = (struct pixInfo){y, pb, pr};
}

/******** getRGBInts ********
 *
 * Converts a UArray2b of floating-point CVCS data back into a new Pnm_ppm image
 * with scaled integer RGB pixels.
 *
 * Parameters:
 *      T RGBInfo:              The source array of pixInfo structs
 *      A2Methods_T methods:    The method suite for array operations
 * Returns:
 *      A Pnm_ppm struct pointer to the newly created image.
 * Expects:
 *      RGBInfo and methods are not NULL.
 * Notes:
 *      Throws a CRE if memory allocation fails.
 *      Throws a CRE if map is NULL.
 *      Allocates memory for a new Pnm_ppm struct and its pixel array,
 *        which the caller is responsible for freeing.
 ************************/
Pnm_ppm getRGBInts(T RGBInfo, A2Methods_T methods)
{
        assert(RGBInfo != NULL);
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        /* Create the destination Pnm_ppm struct */
        Pnm_ppm pixmap = malloc(sizeof(*pixmap));
        assert(pixmap);

        /* Populate the pixmap struct */
        pixmap->width = methods->width(RGBInfo);
        pixmap->height = methods->height(RGBInfo);
        pixmap->denominator = DENOMINATOR;
        pixmap->methods = methods;
        pixmap->pixels = methods->new(pixmap->width, pixmap->height, 
                                      sizeof(struct Pnm_rgb));
        assert(pixmap->pixels != NULL);

        /* Set up the closure for the mapping operation */
        struct applyCompVidToPixelClosure closure = {pixmap, RGBInfo, methods};
        
        /* Map over the destination array, pulling data from the source */
        map(pixmap->pixels, applyCompVidToPixel, &closure);
        
        return pixmap;
}

/******** applyCompVidToPixel ********
 *
 * Apply function that converts a single pixInfo struct back to a Pnm_rgb pixel.
 * It applies the inverse transformation, clamps the resulting RGB floats to the
 * valid range [0.0, 1.0], and scales them to integers.
 *
 * Parameters:
 *      int col:                        Column index of the current pixel
 *      int row:                        Row index of the current pixel
 *      A2Methods_UArray2 pixels:       The array being mapped over (unused)
 *      void *elem:                     Pointer to destination Pnm_rgb pixel
 *      void *cl:                       Pointer to applyCompVidToPixelClosure
 * Returns:
 *      Nothing.
 * Expects:
 *      elem and cl are not NULL.
 * Notes:
 *      Modifies the destination pixel (using elem) in the pixmap->pixels array.
 ************************/
static void applyCompVidToPixel(int col, int row, A2Methods_UArray2 pixels, 
                             void *elem, void *cl) {
                        
        (void) pixels;
        struct applyCompVidToPixelClosure *closure = cl; 
        Pnm_rgb destPixel = elem;

        /* Get a pointer to the source element in the CVCS array */
        struct pixInfo *srcVals = closure->methods->at(closure->RGBInfo,
                                                       col, row);
        assert(srcVals != NULL);
        
        float y = srcVals->y;
        float pb = srcVals->pb;
        float pr = srcVals->pr;

        /* Apply inverse linear transformation from CVCS to RGB */
        float r = 1.0 * y + 0.0 * pb + 1.402 * pr;
        float g = 1.0 * y - 0.344136 * pb - 0.714136 * pr;
        float b = 1.0 * y + 1.772 * pb + 0. * pr;

        /* Clamp RGB float values to the valid [0.0, 1.0] range */
        r = keepInRange(r, 0.0, 1.0);
        g = keepInRange(g, 0.0, 1.0);
        b = keepInRange(b, 0.0, 1.0);

        /* Convert floats to scaled integers by scaling and rounding */
        destPixel->red   = (int) round(r * DENOMINATOR);
        destPixel->green = (int) round(g * DENOMINATOR);
        destPixel->blue  = (int) round(b * DENOMINATOR);
}

/******** keepInRange ********
 *
 * A helper function that forces a floating-point value to a specified minimum
 * and maximum range.
 *
 * Parameters:
 *      float val:      The value to force.
 *      float min:      The minimum allowed value.
 *      float max:      The maximum allowed value.
 * Returns:
 *      The forced value.
 * Expects:
 *      Nothing.
 * Notes:
 *      This function is not declared as static, as it will also be used in
 *      blockOperation.
 ************************/
float keepInRange(float val, float min, float max) {
    if (val < min) {
        return min;
    }
    if (val > max) { 
        return max;
    }
    return val;
}

#undef T
