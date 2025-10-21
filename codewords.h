/*
 *      codewords.h
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      // TODO: description   
 */


#include <stdint.h>
#include "uarray2.h"

/* Compression */
void printWords(UArray2_T quantInts, A2Methods_T methods);

/* Decompression */
UArray2_T readWords(uint64_t codeWords, A2Methods_T methods);


