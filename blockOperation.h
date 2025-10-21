/*
 *      blockOperation.h
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      // TODO: description   
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "uarray2.h"
#include "uarray2b.h"
#include "stdint.h"

// TODO: struct contract
struct quantized {
        unsigned a, indexbpb, indexbpr;
        int b, c, d;
};

/* Compression */
UArray2_T pixelsToDCTBlock(UArray2b_T RGBCompVid, A2Methods_T bMethods, 
                           A2Methods_T pMethods);
UArray2_T quantizeValues(UArray2_T DCTSpace, A2Methods_T methods);

/* Decompression */
UArray2b_T DCTBlockToPixels(UArray2_T DCTSpace, A2Methods_T pMethods, 
                            A2Methods_T bMethods);
UArray2_T dequantizeValues(UArray2_T DCTSpace, A2Methods_T methods);
