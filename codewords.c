/*
 *      codewords.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      // TODO: description   
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "a2blocked.h"
#include "a2methods.h"
#include "blockOperation.h"
#include "codewords.h"
#include "arith40.h"
#include "bitpack.h"
#include "pnm.h"

#define BLOCKSIZE 2

static void applyPrintWord(int col, int row, A2Methods_UArray2 quantInts,
                           void *elem, void *cl);
static uint64_t packBits(uint64_t a, int64_t b, int64_t c, int64_t d, 
                         uint64_t indexbpb, uint64_t indexbpr);

static uint64_t readCodeword(FILE *input);

static struct quantized unpackCodeword();


void printWords(UArray2_T quantInts, A2Methods_T methods)
{
        assert(quantInts != NULL);
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        unsigned width = methods->width(quantInts) * BLOCKSIZE;
        unsigned height = methods->height(quantInts) * BLOCKSIZE;
        
        printf("COMP40 Compressed image format 2\n%u %u\n", width, height);
        
        map(quantInts, applyPrintWord, NULL);
}

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

        uint64_t word = packBits(a, b, c, d, indexbpb, indexbpr);
        
        uint64_t mask = 0xFF;

        putchar((word >> 24) & mask);
        putchar((word >> 16) & mask);
        putchar((word >> 8) & mask);
        putchar(word & mask);
}       

//TODO: FUNC CONTRACT
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

//TODO: FUNC CONTRACT (TAKES FILE POINTER BUT ITS NOT POSITIONED AT THE START OF CODEWORDS)
UArray2_T readWords(FILE *input, A2Methods_T methods, unsigned width, 
                    unsigned height)
{ 
        assert(input != NULL);
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        UArray2_T quantInts = methods->new(width / BLOCKSIZE, 
                                           height / BLOCKSIZE, 
                                           sizeof(struct quantized));

        /* Nested for loops to get the placement in quantized uarray */
        for (int row = 0; row < (int) height / BLOCKSIZE; row++) {
                for (int col = 0; col < (int) width / BLOCKSIZE; col++) {
                        
                        /* read each codeword */
                        uint64_t word = readCodeword(input); //TODO: IMPLEMENT READCODEWORK HELPER

                        /* unpack codeword into quantized vals */
                        struct quantized quant = unpackCodeword(word); //TODO: IMPLEMENT UNPACKCODEWORD HELPER

                        /* store quantized vals into array */
                        struct quantized *destQuant = methods->at(quantInts,
                                                                   col, row);
                        *destQuant = quant;
                }
        }

        return quantInts;
}

//TODO: FUNC CONTRACT
static uint64_t readCodeword(FILE *input) 
{
        assert(input != NULL);

        int byte1 = getc(input);        
        int byte2 = getc(input);
        int byte3 = getc(input);
        int byte4 = getc(input);

        if (byte1 == EOF || byte2 == EOF || byte3 == EOF || byte4 == EOF) {
                //TODO: WHAT TYPE OF ERROR HADNLIGN EHRE??
        }

        uint64_t word = 0;
        word |= ((uint64_t) byte1) << 24;
        word |= ((uint64_t) byte2) << 16;
        word |= ((uint64_t) byte3) << 8;
        word |= ((uint64_t) byte4);

        return word;
}

//TODO: FUNC CONTRACT
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
