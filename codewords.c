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

void printWords(UArray2_T quantInts, A2Methods_T methods)
{
        assert(quantInts != NULL);
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        unsigned width = methods->width(quantInts) * BLOCKSIZE;
        unsigned height = methods->height(quantInts) * BLOCKSIZE;
        
        printf("COMP40 Compressed image format 2\n%u %u", width, height);
        
        map(quantInts, applyPrintWord, NULL);
        printf("\nend of codewords printing\n");
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

//TODO: FUNC CONTRACTS
UArray2_T readWords(uint64_t codeWords, A2Methods_T methods) {
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        UArray2_T quantInts = methods->new(methods->width(codeWords), 
                                              methods->height(codeWords), 
                                              sizeof(struct quantized));

        struct applyReadWordClosure closure = {quantInts, methods, codeWords};

        map(quantInts, applyReadWord, &closure);

        return quantInts;

}
