/*
 *      compress40.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      // TODO: description   
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "compress40.h"
#include "readWriteImage.h"
#include "convertRGB_CV.h"

/* reads PPM, writes compressed image */
extern void compress40(FILE *input)
{
        assert(input != NULL);

        Pnm_ppm img = readImage(input);

        getRGBFloats(img);

        // TODO: move to decompress. this is (c1)'
        writeImage(img);

        img->methods->free(&(img->pixels));
        free(img);
} 

extern void decompress40(FILE *input)
{
        (void)input;
}