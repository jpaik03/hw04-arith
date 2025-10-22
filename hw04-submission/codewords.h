/*
 *      codewords.h
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      Interface for the final stage of the compression process. This module
 *      handles the C4 and (C4)' steps, which involve packing quantized integers
 *      into 32-bit codewords, printing them to standard output, and reading
 *      codewords from a compressed file to unpack them back into integers.  
 */

#include <stdio.h>
#include <stdint.h>

#include "uarray2.h"
#include "a2methods.h"

/* Compression */
void printWords(UArray2_T quantInts, A2Methods_T methods);

/* Decompression */
UArray2_T readWords(FILE *input, A2Methods_T methods, unsigned width, 
                    unsigned height);
