/*
 *      compress40.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      Implementation of the main compression and decompression driver
 *      functions. This module calls other modules to perform the full image
 *      transformation process in either direction.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "uarray2.h"
#include "a2methods.h"
#include "a2blocked.h"
#include "a2plain.h"
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
 *      FILE *input:    A file pointer to the source PPM image
 * Returns:
 *      Nothing.
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
} 

/******** decompress40 ********
 *
 * Reads a compressed binary image from an input stream, decompresses it, and
 * writes the resulting PPM image to standard output.
 *
 * Parameters:
 *      FILE *input:    A file pointer to the source compressed image
 * Expects:
 *      input is not NULL.
 *      input points to a valid, open compressed file.
 * Notes:
 *      Throws a CRE if input is NULL.
 *      Manages the entire decompression pipeline and frees all intermediate
 *        data structures.
 ************************/
extern void decompress40(FILE *input)
{
        assert(input != NULL);

        /* Initialize method suites */
        A2Methods_T bMethods = uarray2_methods_blocked;
        assert(bMethods != NULL);
        A2Methods_T pMethods = uarray2_methods_plain;
        assert(pMethods != NULL);

        unsigned width, height;
        
        /* Read header to get image dimensions */
        readCompressedHeader(input, &width, &height);

        /* Step (C4)': Bit Codeword Operations
         *      Read codewords and unpack into an array of quantized int structs
         */
         
        UArray2_T quantInts = readWords(input, pMethods, width, height);

        /* Step (C3)': Block-level Operations
         *      Convert bit-representation integers to CV Pixels 
         *      Intermediate steps shown for clarity
         */
        
        /* Part 1: Dequantize integers to float DCT values */
        UArray2_T dequantFloats = dequantizeValues(quantInts, pMethods);
        pMethods->free((A2Methods_UArray2 *) &quantInts);

        /* Part 2: Convert DCT blocks back to float Component Video pixels */
        UArray2b_T deRGBCompVid = DCTBlockToPixels(dequantFloats, 
                                                   pMethods,
                                                   bMethods);
        pMethods->free((A2Methods_UArray2 *) &dequantFloats);

        /* Step (C2)': Pixel-level Operations
         *      Convert float Component Video pixels to integer RGB
         */

        Pnm_ppm newImg = getRGBInts(deRGBCompVid, bMethods);
        bMethods->free((A2Methods_UArray2 *) &deRGBCompVid);

        /* Step (C1)': Image Operations
         *      Write the final PPM image to standard output
         */

        writeImage(newImg);
        newImg->methods->free(&(newImg->pixels));
        free(newImg);
        
}

/******** readCompressedHeader ********
 *
 * Reads the required header from a compressed image file.
 *
 * Parameters:
 *      FILE *input:            File pointer to the compressed image
 *      unsigned *width:        Pointer to store the read width
 *      unsigned *height:       Pointer to store the read height
 * Returns:
 *      Nothing.
 * Expects:
 *      All parameters are not NULL.
 *      The input file has a correctly formatted header.
 * Notes:
 *      Throws a CRE if the header format does not match the spec.
 ************************/
static void readCompressedHeader(FILE *input, unsigned *width, unsigned *height)
{
        assert(input != NULL);
        assert(width != NULL);
        assert(height != NULL);

        /* Use fscanf with the provided string to read the header */
        int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", 
                          width, height);
        assert(read == 2);

        /* Verify final newline character */
        int c = getc(input);
        assert(c == '\n');
}