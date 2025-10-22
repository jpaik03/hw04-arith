/*
 *      blockOperation.h
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      Interface for block-level operations, including DCT, chroma averaging,
 *      and quantization. This module is responsible for converting pixel data
 *      into a block-based representation of quantized coefficients.
 */

#include "a2methods.h"
#include "uarray2b.h"
#include "uarray2.h"

/******** quantized struct ********
 *
 * A public struct to hold the final quantized integer representations for a
 * single 2x2 block, ready for bitpacking.
 *
 * Fields:
 *      unsigned a:             9-bit unsigned integer for coefficient a
 *      unsigned indexbpb:      4-bit index for the quantized Pb value
 *      unsigned indexbpr:      4-bit index for the quantized Pr value
 *      int b, c, d:            5-bit signed integers for coefficients b, c, d
 *
 * Notes:
 *      This struct is defined publicly in the header so that the bitpacking
 *      module can directly access its integer fields to create a codeword.
 ************************/
struct quantized
{
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
