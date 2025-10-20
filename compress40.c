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
#include "a2plain.h"
#include "uarray2.h"
#include "a2blocked.h"
#include "a2methods.h"

#include "compress40.h"
#include "readWriteImage.h"
#include "pixelOperation.h"
#include "blockOperation.h"

/* reads PPM, writes compressed image */
extern void compress40(FILE *input)
{       
        assert(input != NULL);

        A2Methods_T bMethods = uarray2_methods_blocked;
        assert(bMethods != NULL);

        A2Methods_T pMethods = uarray2_methods_plain;
        assert(pMethods != NULL);

        /* This is C1 */
        Pnm_ppm img = readImage(input);

        /* This is C2*/
        UArray2b_T RGBCompVid = getRGBCompVid(img, bMethods);
        img->methods->free(&(img->pixels));
        free(img);

        /* This is C3 */
        UArray2_T DCTSpace = pixelsToDCTBlock(RGBCompVid, bMethods, 
                                              pMethods);
        bMethods->free((A2Methods_UArray2 *) &RGBCompVid);
        
        UArray2_T quantInts = quantizeValues(DCTSpace, pMethods);
        pMethods->free((A2Methods_UArray2 *) &DCTSpace);

        // TODO: UNCOMMENT WHEN DECOMPRESSION STEPS ARE MOVED
        // pMethods->free(&quantInts);

        /* TODO: MOVE EVERYTHING BELOW TO DECOMPRESSION */

        // A2Methods_T bMethods = uarray2_methods_blocked;
        // assert(bMethods != NULL);

        // A2Methods_T pMethods = uarray2_methods_plain;
        // assert(pMethods != NULL);

        /* This is (C3)' */
        
        UArray2_T dequantFloats = dequantizeValues(quantInts, 
                                                       pMethods);

        UArray2b_T deRGBCompVid = DCTBlockToPixels(dequantFloats, 
                                                   pMethods,
                                                   bMethods);
        pMethods->free((A2Methods_UArray2 *)&dequantFloats);
        
        /* This is (C2)' */
        Pnm_ppm newImg = getRGBInts(deRGBCompVid, bMethods);
        bMethods->free((A2Methods_UArray2 *)&deRGBCompVid);

        /* This is (C1)' */
        writeImage(newImg);

        newImg->methods->free(&(newImg->pixels));
        free(newImg);


        pMethods->free((A2Methods_UArray2 *)&quantInts);
} 

extern void decompress40(FILE *input)
{
        (void)input;
}