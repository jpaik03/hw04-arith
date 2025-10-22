/*
 *      codewords.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      Implementation for the final stage of the compression process. This
 *      module handles packing quantized integers into 32-bit codewords,
 *      printing them to standard output, and reading codewords from a
 *      compressed file to unpack them back into integers. This module handles
 *      the C4 and (C4)' steps.  
 */

#include <stdlib.h>
#include <assert.h>

#include "a2methods.h"
#include "blockOperation.h"
#include "codewords.h"
#include "bitpack.h"

#define BLOCKSIZE 2

/* Initialize helper functions, see function contracts below */
static void applyPrintWord(int col, int row, A2Methods_UArray2 quantInts,
                           void *elem, void *cl);
static uint64_t packBits(uint64_t a, int64_t b, int64_t c, int64_t d, 
                         uint64_t indexbpb, uint64_t indexbpr);
static uint64_t readCodeword(FILE *input);
static struct quantized unpackCodeword();

/******** printWords ********
 *
 * Prints the compressed image header and all codewords to standard output.
 *
 * Parameters:
 *      UArray2_T quantInts:    An array of 'quantized' structs to be packed
 *      A2Methods_T methods:    The method suite for array operations
 * Returns:
 *      Nothing.s
 * Expects:
 *      quantInts and methods are not NULL.
 * Notes:
 *      Throws a CRE if quantInts or methods is NULL.
 ************************/
void printWords(UArray2_T quantInts, A2Methods_T methods)
{
        assert(quantInts != NULL);
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        /* Print the header with original image's trimmed dimensions */
        unsigned width = methods->width(quantInts) * BLOCKSIZE;
        unsigned height = methods->height(quantInts) * BLOCKSIZE;
        printf("COMP40 Compressed image format 2\n%u %u\n", width, height);
        
        /* Map over the array of quantized ints, printing each as a codeword */
        map(quantInts, applyPrintWord, NULL);
}

/******** applyPrintWord ********
 *
 * Apply function that packs a single 'quantized' struct into a 32-bit codeword
 * and prints its four bytes to standard output in big-endian order.
 *
 * Parameters:
 *      int col:                     Column index of the current block (unused)
 *      int row:                     Row index of the current block (unused)
 *      A2Methods_UArray2 quantInts: The array being mapped over (unused)
 *      void *elem:                  Pointer to the current 'quantized' struct
 *      void *cl:                    Closure pointer (unused)
 * Returns:
 *      Nothing.
 * Expects:
 *      elem is not NULL.
 ************************/
static void applyPrintWord(int col, int row, A2Methods_UArray2 quantInts,
                           void *elem, void *cl)
{       
        (void) col;
        (void) row;
        (void) quantInts;
        (void) cl;

        struct quantized *originalQuant = elem;

        uint64_t a = originalQuant->a;
        int64_t b = originalQuant->b;
        int64_t c =  originalQuant->c;
        int64_t d = originalQuant->d;
        uint64_t indexbpb = originalQuant->indexbpb;
        uint64_t indexbpr = originalQuant->indexbpr;

        /* Pack the integer fields from the struct into a single 64-bit word */
        uint64_t word = packBits(a, b, c, d, indexbpb, indexbpr);
        
        /* Mask to isolate one byte at a time */
        uint64_t mask = 0xFF;

        /* Print the 4 bytes of the codeword in big-endian order */
        putchar((word >> 24) & mask);
        putchar((word >> 16) & mask);
        putchar((word >> 8) & mask);
        putchar(word & mask);
}       

/******** packBits ********
 *
 * Packs the integer fields from a 'quantized' struct into a single 32-bit
 * codeword using the Bitpack interface.
 *
 * Parameters:
 *      struct quantized *quant: Pointer to the struct with integer data
 * Returns:
 *      A 64-bit word containing the packed 32-bit codeword.
 * Expects:
 *      quant is not NULL.
 ************************/
static uint64_t packBits(uint64_t a, int64_t b, int64_t c, int64_t d, 
                         uint64_t indexbpb, uint64_t indexbpr)
{        
        uint64_t word = 0;

        word = Bitpack_newu(word, 9, 23, a);
        word = Bitpack_news(word, 5, 18, b);
        word = Bitpack_news(word, 5, 13, c);
        word = Bitpack_news(word, 5, 8, d);
        word = Bitpack_newu(word, 4, 4, indexbpb);
        word = Bitpack_newu(word, 4, 0, indexbpr);    

        return word;
}

/******** readWords ********
 *
 * Reads all codewords from a compressed file, unpacks them, and stores the
 * resulting integer coefficients in a new UArray2.
 *
 * Parameters:
 *      FILE *input:         File pointer positioned after the header
 *      A2Methods_T methods: The method suite for array operations
 *      unsigned width:      The width of the block array to create
 *      unsigned height:     The height of the block array to create
 * Returns:
 *      A UArray2_T where each element is a 'quantized' struct.
 * Expects: 
 *      input and methods are not NULL.
 * Notes:
 *      Throws a CRE if input or methods is NULL.
 *      Throws a CRE if memory allocation fails.
 ************************/
UArray2_T readWords(FILE *input, A2Methods_T methods, unsigned width, 
                    unsigned height)
{ 
        assert(input != NULL);
        assert(methods != NULL);

        /* Create a new array to hold the unpacked integer data */
        UArray2_T quantInts = methods->new(width / BLOCKSIZE, 
                                           height / BLOCKSIZE, 
                                           sizeof(struct quantized));
        assert(quantInts != NULL);

        /* Iterate through the expected number of codewords */
        for (int row = 0; row < (int) height / BLOCKSIZE; row++) {
                for (int col = 0; col < (int) width / BLOCKSIZE; col++) {
                        /* Read the next 4 bytes and assemble each codeword */
                        uint64_t word = readCodeword(input);

                        /* Unpack the codeword into a 'quantized' struct */
                        struct quantized quant = unpackCodeword(word);

                        /* Store the unpacked struct in the array */
                        struct quantized *destQuant = methods->at(quantInts,
                                                                  col, row);
                        *destQuant = quant;
                }
        }

        return quantInts;
}

/******** readCodeword ********
 *
 * Reads four bytes from the input stream and assembles them into a single
 * 32-bit codeword, respecting big-endian byte order.
 *
 * Parameters:
 *      FILE *input:    File pointer to the compressed data
 * Returns:
 *      A 64-bit word containing the assembled 32-bit codeword.
 * Expects:
 *      input is not NULL.
 *      input contains at least 4 more bytes of data.
 * Notes:
 *      Throws a CRE if EOF is reached prematurely.
 ************************/
static uint64_t readCodeword(FILE *input) 
{
        assert(input != NULL);

        /* Read four sequential bytes from the input stream */
        int byte1 = getc(input);        
        int byte2 = getc(input);
        int byte3 = getc(input);
        int byte4 = getc(input);

        /* If any byte is EOF, the file is too short and throws a CRE */
        assert(byte1 != EOF && byte2 != EOF && byte3 != EOF && byte4 != EOF);

        /* Assemble the bytes into a 64-bit word in big-endian order */
        uint64_t word = 0;
        word = Bitpack_newu(word, 8, 24, (uint64_t) byte1);
        word = Bitpack_newu(word, 8, 16, (uint64_t) byte2);
        word = Bitpack_newu(word, 8, 8,  (uint64_t) byte3);
        word = Bitpack_newu(word, 8, 0,  (uint64_t) byte4);

        return word;
}

/******** unpackCodeword ********
 *
 * Unpacks a 32-bit codeword into its constituent integer fields and returns
 * them in a 'quantized' struct.
 *
 * Parameters:
 *      uint64_t word:  The 32-bit codeword (stored in a 64-bit integer)
 * Returns:
 *      A 'quantized' struct populated with the unpacked values.
 * Expects:
 *      Nothing.
 * Notes:
 *      Nothing.
 ************************/
static struct quantized unpackCodeword(uint64_t word) 
{
        struct quantized quant;

        quant.a = Bitpack_getu(word, 9, 23);
        quant.b = Bitpack_gets(word, 5, 18);
        quant.c = Bitpack_gets(word, 5, 13);
        quant.d = Bitpack_gets(word, 5, 8);
        quant.indexbpb = Bitpack_getu(word, 4, 4);
        quant.indexbpr = Bitpack_getu(word, 4, 0);

        return quant;
}
