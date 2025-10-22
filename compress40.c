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
#include "codewords.h"

/* Initialize helper functions, see function contracts below */
static void readCompressedHeader(FILE *input, unsigned *width, 
                                 unsigned *height);

/******** compress40 ********
 *
 * Compresses a PPM image from an input stream and writes the binary compressed
 * format to standard output.
 *
 * Parameters:
 *      FILE *input: A file pointer to the source PPM image.
 * Returns 
 * Expects:
 *      input is not NULL and points to a valid, open PPM file.
 * Notes:
 *      Throws a CRE if input is NULL
 *      Manages the entire compression pipeline and frees all intermediate data
 *        structures.
 ************************/
extern void compress40(FILE *input)
{       
        assert(input != NULL);

        /* Initialize method suites for blocked and plain arrays */
        A2Methods_T bMethods = uarray2_methods_blocked;
        assert(bMethods != NULL);
        A2Methods_T pMethods = uarray2_methods_plain;
        assert(pMethods != NULL);

        /* Step C1: Image Operations
         *      Read and trim the image to even dimensions
         */
        
        Pnm_ppm img = readImage(input);

        /* Step C2: Pixel-level Operations
         *      Convert integer RGB pixels to float Component Video
         */
        
        UArray2b_T RGBCompVid = getRGBCompVid(img, bMethods);
        img->methods->free(&(img->pixels));
        free(img);

        /* Step C3: Block-level Operations
         *      Convert CV Pixels to quantized bit-representation integers
         *      Intermediate steps shown for clarity
         */
        
        /* Part 1: Convert CV pixels to DCT blocks (float) */
        UArray2_T DCTSpace = pixelsToDCTBlock(RGBCompVid, bMethods, 
                                              pMethods);
        bMethods->free((A2Methods_UArray2 *) &RGBCompVid);
        
        /* Part 2: Quantize float DCT values to integers */
        UArray2_T quantInts = quantizeValues(DCTSpace, pMethods);
        pMethods->free((A2Methods_UArray2 *) &DCTSpace);

        /* Step C4: Bit Codeword Operations
         *      Pack integers into codewords and print to stdout
         */
        printWords(quantInts, pMethods);
        pMethods->free((A2Methods_UArray2 *) &quantInts);

        /* TODO: MOVE EVERYTHING BELOW TO DECOMPRESSION */

        // A2Methods_T bMethods = uarray2_methods_blocked;
        // assert(bMethods != NULL);

        // A2Methods_T pMethods = uarray2_methods_plain;
        // assert(pMethods != NULL);

        /* This is (C3)' */
        
        // UArray2_T dequantFloats = dequantizeValues(quantInts, 
        //                                                pMethods);

        // UArray2b_T deRGBCompVid = DCTBlockToPixels(dequantFloats, 
        //                                            pMethods,
        //                                            bMethods);
        // pMethods->free((A2Methods_UArray2 *)&dequantFloats);
        
        // /* This is (C2)' */
        // Pnm_ppm newImg = getRGBInts(deRGBCompVid, bMethods);
        // bMethods->free((A2Methods_UArray2 *)&deRGBCompVid);

        // /* This is (C1)' */
        // writeImage(newImg);

        // newImg->methods->free(&(newImg->pixels));
        // free(newImg);


        // pMethods->free((A2Methods_UArray2 *)&quantInts);
} 

/******** decompress40 ********
 *
 * Reads a compressed binary image from an input stream, decompresses it,
 * and writes the resulting PPM image to standard output.
 *
 * Parameters:
 * FILE *input: A file pointer to the source compressed image.
 * Expects:
 * input is not NULL and points to a valid, open compressed file.
 * Notes:
 * Throws a CRE if input is NULL.
 * Manages the entire decompression pipeline and frees all intermediate
 * data structures.
 ************************/
extern void decompress40(FILE *input)
{
        assert(input != NULL);

        A2Methods_T bMethods = uarray2_methods_blocked;
        assert(bMethods != NULL);

        A2Methods_T pMethods = uarray2_methods_plain;
        assert(pMethods != NULL);

        unsigned width, height;

        /* Reading header helper function */
        readCompressedHeader(input, &width, &height);

        // UArray2_T pixels = pMethods->new(width, height, 
        //                                    sizeof(struct Pnm_rgb));

        /* Struct population */
        // struct Pnm_ppm pixmap = { 
        //         .width = width, 
        //         .height = height,
        //         .denominator = 255, 
        //         .pixels = pixels,
        //         .methods = pMethods
        // };

        /* (C4)' */
        UArray2_T quantInts = readWords(input, pMethods, width, height);

        /* (C3)' */
        UArray2_T dequantFloats = dequantizeValues(quantInts, pMethods);
        pMethods->free((A2Methods_UArray2 *) &quantInts);

        UArray2b_T deRGBCompVid = DCTBlockToPixels(dequantFloats, 
                                                   pMethods,
                                                   bMethods);
        pMethods->free((A2Methods_UArray2 *) &dequantFloats);

        /* (C2)' */
        Pnm_ppm newImg = getRGBInts(deRGBCompVid, bMethods);
        bMethods->free((A2Methods_UArray2 *) &deRGBCompVid);

        /* (C1)' */
        writeImage(newImg);
        newImg->methods->free(&(newImg->pixels));
        free(newImg);
        
}

//TODO: FUNC CONTRACTS
static void readCompressedHeader(FILE *input, unsigned *width, unsigned *height)
{
        assert(input != NULL);
        assert(width != NULL);
        assert(height != NULL);

        FILE *in = input;
        int read = fscanf(in, "COMP40 Compressed image format 2\n%u %u", 
                          width, height);
        assert(read == 2);
        int c = getc(in);
        assert(c == '\n');
}