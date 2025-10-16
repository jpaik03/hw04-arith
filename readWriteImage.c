/*
 *      readWriteImage.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      // TODO: description
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "readWriteImage.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "a2methods.h"

static void copy_pixel(int col, int row, A2Methods_UArray2 newArray, 
                       void *elem, void *cl);

// TODO: STRUCT CONTRACT
struct copyClosure
{
        Pnm_ppm oldImg;
};

// TODO: FUNCTION CONTRACT
Pnm_ppm readImage(FILE *fp)
{
        assert(fp != NULL);
        
        A2Methods_T methods = uarray2_methods_blocked;
        assert(methods != NULL);
        
        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);
        
        Pnm_ppm img = Pnm_ppmread(fp, methods);
        assert(img != NULL);

        return trimImage(img, map);
}

// TODO: FUNCTION CONTRACT
Pnm_ppm trimImage(Pnm_ppm oldImg, A2Methods_mapfun *map)
{
        assert(oldImg != NULL);

        /* If the dimensions were already even, return */
        int newWidth = ((int) oldImg->width / 2) * 2;
        int newHeight = ((int) oldImg->height / 2) * 2;
        if (newWidth == (int) oldImg->width &&
            newHeight == (int) oldImg->height) {
                return oldImg;
        }

        /* Create new image */
        Pnm_ppm newImg = malloc(sizeof(*newImg));
        assert(newImg != NULL);
        
        /* Populate new image's struct */
        newImg->denominator = oldImg->denominator;
        newImg->methods = oldImg->methods;
        newImg->width = newWidth;
        newImg->height = newHeight;
        newImg->pixels = oldImg->methods->new(newWidth, newHeight, 
                                              sizeof(struct Pnm_rgb));
        assert(newImg->pixels != NULL);

        /* Set up closure */
        // struct copyClosure closure = { .newImg = newImg };
        struct copyClosure closure = { .oldImg = oldImg };
        
        /* Copy the old image, ignoring ("trimming") the last odd row/column */
        map(newImg->pixels, copy_pixel, &closure);

        /* Free old image's memory */
        oldImg->methods->free(&(oldImg->pixels));
        free(oldImg);

        return newImg;
}

// TODO FUNCTION CONTRACT
static void copy_pixel(int col, int row, A2Methods_UArray2 newArray, 
                       void *elem, void *cl)
{
        
        (void) newArray;
        struct copyClosure *closure = cl;
        Pnm_rgb newPixel = elem;

        /* Performs the pixel map */
        Pnm_rgb oldPixel = closure->oldImg->methods->at(closure->oldImg->pixels,
                                                        col, row);
        assert(oldPixel != NULL);
        *newPixel = *oldPixel;
}

void writeImage(Pnm_ppm pixmap)
{
        assert(pixmap != NULL);
        Pnm_ppmwrite(stdout, pixmap);
}