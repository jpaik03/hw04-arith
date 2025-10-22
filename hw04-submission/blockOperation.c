/*
 *      blockOperation.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      Implementation for block-level operations in the image compression
 *      process. This module handles chroma averaging, Discrete Cosine Transform
 *      (DCT), and quantization. It corresponds to the C3 and (C3)' steps.
 */

#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "a2blocked.h"
#include "a2methods.h"
#include "blockOperation.h"
#include "pixelOperation.h"
#include "arith40.h"

#define BLOCKSIZE 2

/* Initialize helper functions, see function contracts below */
static void applyCompVidToDCT(int col, int row, A2Methods_UArray2 pixels, 
                              void *elem, void *cl);
static void applyDCTToPixel(int col, int row, A2Methods_UArray2 pixels, 
                            void *elem, void *cl);
static void applyQuantize(int col, int row, A2Methods_UArray2 pixels,
                          void *elem, void *cl);
static int quantizeBCD(float coefficient);
static void applyDequantize(int col, int row, A2Methods_UArray2 pixels, 
                            void *elem, void *cl);
static float dequantizeBCD(int quantizedCoeff);

/******** DCTVals struct ********
 *
 * A struct to hold the floating-point results of the DCT and chroma averaging
 * for a single 2x2 block.
 *
 * Fields:
 *      float a, b, c, d:       The four DCT coefficients.
 *      float bpb, bpr:         The averaged chroma values for the block.
 ************************/
struct DCTVals
{
        float a, b, c, d, bpb, bpr;
};

/******** applyCompVidToDCTClosure struct ********
 *
 * A closure passed to the apply function that converts CVCS pixel data into DCT
 * block data.
 *
 * Fields:
 *      UArray2_T DCTSpace:     The destination array for DCT block data
 *      A2Methods_T bMethods:   The blocked method suite for the source array
 *      A2Methods_T pMethods:   The plain method suite for the destination array
 *      UArray2b_T RGBCompVid:  The source array of CVCS pixel data
 ************************/
struct applyCompVidToDCTClosure
{
        UArray2_T DCTSpace;
        A2Methods_T bMethods;
        A2Methods_T pMethods;
        UArray2b_T RGBCompVid;
};

/******** applyDCTToPixelClosure struct ********
 *
 * A closure passed to the apply function that converts DCT block data back into
 * CVCS pixel data.
 *
 * Fields:
 *      UArray2b_T RGBFloats:   The destination array for CVCS pixel data
 *      A2Methods_T pMethods:   The plain method suite for the source array
 *      A2Methods_T bMethods:   The blocked method suite for the destination
 *                                array
 *      UArray2_T DCTSpace:     The source array of DCT block data
 ************************/
struct applyDCTToPixelClosure
{
        UArray2b_T RGBFloats;
        A2Methods_T pMethods;
        A2Methods_T bMethods;
        UArray2_T DCTSpace;
};

/******** applyQuantizeClosure struct ********
 *
 * A closure passed to the apply function that quantizes floating-point DCT
 * values into integers.
 *
 * Fields:
 *      UArray2_T quantInts:    The destination array for quantized integers
 *      A2Methods_T methods:    The method suite for array operations
 *      UArray2_T DCTSpace:     The source array of floating-point DCT data
 ************************/
struct applyQuantizeClosure
{
        UArray2_T quantInts;
        A2Methods_T methods;
        UArray2_T DCTSpace;
};

/******** applyDequantizeClosure struct ********
 *
 * A closure passed to the apply function that dequantizes integers back into
 * floating-point DCT values.
 *
 * Fields:
 *      UArray2_T dequantFloats:        The destination array for dequantized
 *                                        floats
 *      A2Methods_T methods:            The method suite for array operations
 *      UArray2_T DCTSpace:             The source array of quantized int data
 ************************/
struct applyDequantizeClosure
{
        UArray2_T dequantFloats;
        A2Methods_T methods;
        UArray2_T DCTSpace;
};

/******** pixelsToDCTBlock ********
 *
 * Converts a UArray2b of CVCS pixel data into a UArray2 of DCT block data.
 * Iterates over 2x2 blocks of pixels, calculates averaged chroma and DCT
 * coefficients for each, and stores them.
 *
 * Parameters:
 *      UArray2b_T RGBCompVid:  Source array of pixInfo structs
 *      A2Methods_T bMethods:   Blocked method suite for source array
 *      A2Methods_T pMethods:   Plain method suite for destination array
 * Returns:
 *      A UArray2_T where each element is a DCTVals struct.
 * Expects:
 *      All parameters are not NULL.
 *      RGBCompVid has even width and height.
 * Notes:
 *      Allocates memory for the returned UArray2_T, which the caller must free.
 *      Throws a CRE if any parameter is NULL or if allocation fails.
 ************************/
UArray2_T pixelsToDCTBlock(UArray2b_T RGBCompVid, A2Methods_T bMethods,
                           A2Methods_T pMethods)
{
        assert(RGBCompVid != NULL);
        assert(pMethods != NULL);
        assert(bMethods != NULL);

        A2Methods_mapfun *map = pMethods->map_default;
        assert(map != NULL);

        /* Verify source image has compatible dimensions for blocking */
        assert(bMethods->width(RGBCompVid) % BLOCKSIZE == 0);
        assert(bMethods->height(RGBCompVid) % BLOCKSIZE == 0);

        /* Destination array dimensions will be scaled down by BLOCKSIZE */
        int blockedWidth = bMethods->width(RGBCompVid) / BLOCKSIZE;
        int blockedHeight = bMethods->height(RGBCompVid) / BLOCKSIZE;

        /* Create a plain array to hold the new block-based data */
        UArray2_T DCTSpace = pMethods->new(blockedWidth, blockedHeight,
                                           sizeof(struct DCTVals));
        assert(DCTSpace != NULL);

        /* Set up closure with all necessary data for the mapping */
        struct applyCompVidToDCTClosure closure = {DCTSpace, bMethods, 
                                                   pMethods, RGBCompVid};

        /* Map over the destination array, processing block at a time */
        map(DCTSpace, applyCompVidToDCT, &closure);

        return DCTSpace;
}

/******** applyCompVidToDCT ********
 *
 * Apply function for pixelsToDCTBlock. For each element in the destination
 * (block) array, it reads the corresponding 2x2 pixel block from the source,
 * computes the averaged chroma and DCT coefficients, and stores them.
 *
 * Parameters:
 *      int col:                        Column index of the current block
 *      int row:                        Row index of the current block
 *      A2Methods_UArray2 pixels:       The array being mapped over (unused)
 *      void *elem:                     Pointer to curr DCTVals element (unused)
 *      void *cl:                       Pointer to the applyCompVidToDCTClosure
 * Returns:
 *      Nothing.
 * Expects:
 *      cl is not NULL and its contents are valid.
 * Notes:
 *      Modifies the DCTVals element at (col, row) in the DCTSpace array.
 *      Reads four pixInfo elements from the RGBCompVid array.
 ************************/
static void applyCompVidToDCT(int col, int row, A2Methods_UArray2 pixels,
                              void *elem, void *cl)
{
        (void) pixels;
        (void) elem;
        assert(cl != NULL);

        struct applyCompVidToDCTClosure *closure = cl;

        assert(closure->bMethods != NULL);
        assert(closure->pMethods != NULL);
        assert(closure->RGBCompVid != NULL);
        assert(closure->DCTSpace != NULL);
        assert(closure->bMethods->at != NULL);
        assert(closure->pMethods->at != NULL);

        /* Get pointers to the four pixels in the 2x2 source block */
        struct pixInfo *pix1 = closure->bMethods->at(closure->RGBCompVid, 
                                                     col * BLOCKSIZE,
                                                     row * BLOCKSIZE);
        assert(pix1 != NULL);
        struct pixInfo *pix2 = closure->bMethods->at(closure->RGBCompVid, 
                                                     col * BLOCKSIZE + 1,
                                                     row * BLOCKSIZE);
        assert(pix2 != NULL);
        struct pixInfo *pix3 = closure->bMethods->at(closure->RGBCompVid,
                                                     col * BLOCKSIZE,
                                                     row * BLOCKSIZE + 1);
        assert(pix3 != NULL);
        struct pixInfo *pix4 = closure->bMethods->at(closure->RGBCompVid,    
                                                     col * BLOCKSIZE + 1,
                                                     row * BLOCKSIZE + 1);
        assert(pix4 != NULL);

        /* Calculate average chroma for the block */
        float pb_bar = (pix1->pb + pix2->pb + pix3->pb + pix4->pb) / 4.0;
        float pr_bar = (pix1->pr + pix2->pr + pix3->pr + pix4->pr) / 4.0;

        /* Calculate DCT coefficients a, b, c, d for the block */
        float a = (pix4->y + pix3->y + pix2->y + pix1->y) / 4.0;
        float b = (pix4->y + pix3->y - pix2->y - pix1->y) / 4.0;
        float c = (pix4->y - pix3->y + pix2->y - pix1->y) / 4.0;
        float d = (pix4->y - pix3->y - pix2->y + pix1->y) / 4.0;

        /* Store the results in the destination block array */
        struct DCTVals *destDCT = closure->pMethods->at(closure->DCTSpace,
                                                        col, row);
        *destDCT = (struct DCTVals){a, b, c, d, pb_bar, pr_bar};
}

/******** quantizeValues ********
 *
 * Converts an array of floating-point DCT/chroma values into an array of
 * quantized integers ready for bitpacking.
 *
 * Parameters:
 *      UArray2_T DCTSpace:     An array where each element is a DCTVals struct
 *      A2Methods_T methods:    The method suite to use
 * Returns:
 *      A UArray2_T where each element is a quantized struct.
 * Expects:
 *      DCTSpace and methods are not NULL.
 * Notes:
 *      Allocates memory for the returned UArray2_T, which the caller must free.
 ************************/
UArray2_T quantizeValues(UArray2_T DCTSpace, A2Methods_T methods)
{
        assert(DCTSpace != NULL);
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        /* Create a new array of the same dimensions for the int results */
        UArray2_T quantInts = methods->new(methods->width(DCTSpace), 
                                           methods->height(DCTSpace), 
                                           sizeof(struct quantized));
        assert(quantInts != NULL);

        struct applyQuantizeClosure closure = {quantInts, methods, DCTSpace};

        map(DCTSpace, applyQuantize, &closure);
        
        return quantInts;
}

/******** applyQuantize ********
 *
 * Apply function for quantizeValues. Reads the float coefficients from a source
 * DCT block, converts them to their specified integer representations, and
 * stores them in the destination array.
 *
 * Parameters:
 *      int col:                        Column index of the current block
 *      int row:                        Row index of the current block
 *      A2Methods_UArray2 pixels:       The array being mapped over (unused)
 *      void *elem:                     Pointer to curr DCTVals element (unused)
 *      void *cl:                       Pointer to applyQuantizeClosure
 * Returns:
 *      Nothing.
 * Expects:
 *      cl is not NULL and its contents are valid.
 * Notes:
 *      Modifies the quantized element at (col, row) in the quantInts array.
 *      Reads from the DCTVals element at (col, row) in the DCTSpace array.
 ************************/
static void applyQuantize(int col, int row, A2Methods_UArray2 pixels, 
                          void *elem, void *cl)
{      
        (void) pixels;
        (void) elem;
        assert(cl != NULL);
        
        struct applyQuantizeClosure *closure = cl;

        assert(closure->methods != NULL);
        assert(closure->quantInts != NULL);
        assert(closure->DCTSpace != NULL);
        assert(closure->methods->at != NULL);

        struct DCTVals *srcDCT = closure->methods->at(closure->DCTSpace,
                                                      col, row);

        /* Quantize DCT coefficient 'a' to a 9-bit unsigned integer */
        unsigned a = (unsigned) round(srcDCT->a * 511);

        /* Quantize coefficients 'b', 'c', and 'd' to 5-bit signed integers */
        int b = quantizeBCD(srcDCT->b);
        int c = quantizeBCD(srcDCT->c);
        int d = quantizeBCD(srcDCT->d);

        /* Quantize chroma values to 4-bit indices */
        unsigned indexbpb = Arith40_index_of_chroma(srcDCT->bpb);
        unsigned indexbpr = Arith40_index_of_chroma(srcDCT->bpr);

        /* Get pointer to destination and store the quantized values */
        struct quantized *destQuant = closure->methods->at(closure->quantInts,
                                                           col, row);
        *destQuant = (struct quantized){a, indexbpb, indexbpr, b, c, d};
}

/******** quantizeBCD ********
 *
 * Helper to quantize a single b, c, or d coefficient. Forces the float value to
 * the range [-0.3, 0.3] and then scales it to a 5-bit signed integer in the
 * range [-15, 15].
 *
 * Parameters:
 *      float coefficient:      The floating-point coefficient to quantize
 * Returns:
 *      An integer in the range [-15, 15].
 * Expects:
 *      Nothing.
 * Notes:
 *      Forces the float to the range [-0.3, 0.3].
 ************************/
static int quantizeBCD(float coefficient)
{
        /* Force the float value to the specified range */
        float quantizedCoeff = keepInRange(coefficient, -0.3, 0.3);

        /* Scale and round to the nearest integer */
        return (int) round(quantizedCoeff * 50);
}

/******** dequantizeValues ********
 *
 * Converts an array of quantized integer structs back into an array of
 * floating-point DCT/chroma values.
 *
 * Parameters:
 *      UArray2_T DCTSpace:     An array where each element is a quantized
 *                                struct
 *      A2Methods_T methods:    The method suite to use
 * Returns:
 *      A UArray2_T where each element is a DCTVals struct.
 * Expects:
 *      DCTSpace and methods are not NULL.
 * Notes:
 *      Allocates memory for the returned UArray2_T, which the caller must free.
 ************************/
UArray2_T dequantizeValues(UArray2_T DCTSpace, A2Methods_T methods)
{       
        assert(DCTSpace != NULL);
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        /* Create the destination array for the dequantized float values */
        UArray2_T dequantFloats = methods->new(methods->width(DCTSpace), 
                                               methods->height(DCTSpace), 
                                               sizeof(struct DCTVals));
        assert(dequantFloats != NULL);

        struct applyDequantizeClosure closure = {dequantFloats, methods,
                                                 DCTSpace};
                                                        
        map(DCTSpace, applyDequantize, &closure);

        return dequantFloats;       
}

/******** applyDequantize ********
 *
 * Apply function for dequantizeValues. Reads quantized integers from a source
 * block, converts them back to their floating-point representations, and stores
 * them in the destination array.
 *
 * Parameters:
 *      int col:                        Column index of the current block
 *      int row:                        Row index of the current block
 *      A2Methods_UArray2 pixels:       The array being mapped over (unused)
 *      void *elem:                     Pointer to the current element (unused)
 *      void *cl:                       Pointer to the applyDequantizeClosure
 * Returns:
 *      Nothing.
 * Expects:
 *      cl is not NULL and its contents are valid.
 * Notes:
 *      Modifies the DCTVals element at (col, row) in the dequantFloats array.
 ************************/
static void applyDequantize(int col, int row, A2Methods_UArray2 pixels, 
                            void *elem, void *cl)
{        
        (void) pixels;
        (void) elem;
        assert(cl != NULL);
        
        struct applyDequantizeClosure *closure = cl;

        assert(closure->methods != NULL);
        assert(closure->dequantFloats != NULL);
        assert(closure->DCTSpace != NULL);
        assert(closure->methods->at != NULL);

        struct quantized *srcQuant = closure->methods->at(closure->DCTSpace,
                                                          col, row);

        /* Dequantize 'a' by dividing by given factor */
        float a = keepInRange(srcQuant->a / 511.0, 0, 1);

        /* Dequantize 'b', 'c', and 'd' */
        float b = dequantizeBCD(srcQuant->b);
        float c = dequantizeBCD(srcQuant->c);
        float d = dequantizeBCD(srcQuant->d);

        /* Dequantize chroma indices  */
        float pb_bar = Arith40_chroma_of_index(srcQuant->indexbpb);
        float pr_bar = Arith40_chroma_of_index(srcQuant->indexbpr);

        /* Get pointer to destination and store the float values */
        struct DCTVals *destDCT = closure->methods->at(closure->dequantFloats,
                                                       col, row);

        *destDCT = (struct DCTVals){a, b, c, d, pb_bar, pr_bar};
}

/******** dequantizeBCD ********
 *
 * Helper to dequantize a single b, c, or d coefficient from its 5-bit integer
 * representation back to a float.
 *
 * Parameters:
 *      int quantizedCoeff:     An integer in the range [-15, 15]
 * Returns:
 *      A float representing the dequantized coefficient.
 * Expects:
 *      Nothing.
 * Notes:
 *      Reverses the scaling from quantizeBCD.
 ************************/
static float dequantizeBCD(int quantizedCoeff)
{
        float coefficient = quantizedCoeff / 50.0;
        return coefficient;
}

/******** DCTBlockToPixels ********
 *
 * Converts a UArray2 of DCT block data back into a UArray2b of CVCS pixel data.
 * It iterates over each DCT block, applies the inverse DCT, and populates the
 * corresponding four pixels.
 *
 * Parameters:
 *      UArray2_T DCTSpace:     Source array of DCTVals structs
 *      A2Methods_T pMethods:   Plain method suite for source array
 *      A2Methods_T bMethods:   Blocked method suite for destination array
 * Returns:
 *      A UArray2b_T where each element is a pixInfo struct.
 * Expects:
 *      All parameters are not NULL.
 * Notes:
 *      Allocates memory for the returned UArray2b_T, which the caller must
 *        free.
 ************************/
UArray2b_T DCTBlockToPixels(UArray2_T DCTSpace, A2Methods_T pMethods,
                            A2Methods_T bMethods)
{
        assert(DCTSpace != NULL);
        assert(pMethods != NULL);
        assert(bMethods != NULL);

        A2Methods_mapfun *map = bMethods->map_default;
        assert(map != NULL);

        /* Calculate dimensions for the pixel array */
        int pixelW = pMethods->width(DCTSpace) * BLOCKSIZE;
        int pixelH = pMethods->height(DCTSpace) * BLOCKSIZE;

        /* Create a new blocked array to hold the pixel data */
        UArray2b_T RGBFloats = bMethods->new_with_blocksize(pixelW, pixelH,
                                                         sizeof(struct pixInfo),
                                                         BLOCKSIZE);
        assert(RGBFloats != NULL);

        struct applyDCTToPixelClosure closure = {RGBFloats, pMethods, bMethods, 
                                                 DCTSpace};

        /* Map over the destination pixel array, populating from the source */
        map(RGBFloats, applyDCTToPixel, &closure);

        return RGBFloats;
}

/******** applyDCTToPixel ********
 *
 * Apply function for DCTBlockToPixels. For each pixel in the destination array,
 * it finds the corresponding source DCT block, calculates the correct Y value
 * using the inverse DCT formulas, and populates the pixel.
 *
 * Parameters:
 *      int col:                        Column index of the current pixel
 *      int row:                        Row index of the current pixel
 *      A2Methods_UArray2 pixels:       The array being mapped over (unused)
 *      void *elem:                     Pointer to the current element (unused)
 *      void *cl:                       Pointer to the applyDCTToPixelClosure
 * Returns:
 *      Nothing.
 * Expects:
 *      cl is not NULL and its contents are valid.
 * Notes:
 *      Modifies the pixInfo element at (col, row) in the RGBFloats array.
 ************************/
static void applyDCTToPixel(int col, int row, A2Methods_UArray2 pixels,
                            void *elem, void *cl)
{
        (void) pixels;
        (void) elem;
        assert(cl != NULL);

        struct applyDCTToPixelClosure *closure = cl;

        assert(closure->pMethods != NULL);
        assert(closure->bMethods != NULL);
        assert(closure->RGBFloats != NULL);
        assert(closure->DCTSpace != NULL);
        assert(closure->pMethods->at != NULL);
        assert(closure->bMethods->at != NULL);

        /* Find the source block for the current pixel */
        struct DCTVals *srcDCT = closure->pMethods->at(closure->DCTSpace,
                                                       col / BLOCKSIZE,
                                                       row / BLOCKSIZE);

        float a = srcDCT->a;
        float b = srcDCT->b;
        float c = srcDCT->c;
        float d = srcDCT->d;
        float pb_bar = srcDCT->bpb;
        float pr_bar = srcDCT->bpr;

        float y;
        /* Apply the correct inverse DCT formula based on the pixel's position
           within the 2x2 block */
        if (col % BLOCKSIZE == 0 && row % BLOCKSIZE == 0) {
                y = a - b - c + d;
        } else if (col % BLOCKSIZE == 1 && row % BLOCKSIZE == 0) {
                y = a - b + c - d;
        } else if (col % BLOCKSIZE == 0 && row % BLOCKSIZE == 1) {
                y = a + b - c - d;
        } else {
                y = a + b + c + d;
        }

        /* Get the destination pixel and populate it with the calculated Y value
           and the block's averaged chroma values */
        struct pixInfo *destPix = closure->bMethods->at(closure->RGBFloats,
                                                        col, row);

        destPix->y = y;
        destPix->pb = pb_bar;
        destPix->pr = pr_bar;
}
