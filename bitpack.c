/* 
 *      bitpack.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      // TODO: description   
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "bitpack.h"
#include "arith40.h"
#include "compress40.h"

#include "except.h"
Except_T Bitpack_Overflow = { "Overflow packing bits" };

const uint64_t UINT64_1 = 1;

static uint64_t shiftLeft(uint64_t u, unsigned shift);
static uint64_t shiftRightU(uint64_t u, unsigned shift);

//TODO: FUNC CONTRACT
bool Bitpack_fitsu(uint64_t n, unsigned width) 
{
        assert(width <= 64);

        if (width == 64) {
                return true;
        }
        
        uint64_t max = shiftLeft(UINT64_1, width) - 1;

        return n <= max;
}

//TODO: FUNC CONTRACT
bool Bitpack_fitss(int64_t n, unsigned width) 
{
        assert(width <= 64);

        if (width == 64) {
                return true;
        }

        if (width == 0) {
                return n == 0;
        }
        
        uint64_t powTwo = shiftLeft(UINT64_1, width - UINT64_1);

        int64_t min = -(int64_t) powTwo;
        int64_t max = (int64_t) powTwo - UINT64_1;

        return n >= min && n <= max;
}

uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= 64);
        assert(lsb + width <= 64);

        if (width == 0) {
                return 0;
        }

        word = shiftRightU(word, lsb);
        
        uint64_t mask = shiftLeft(UINT64_1, width) - UINT64_1;
        
        return word & mask;
}

int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= 64);
        assert(lsb + width <= 64);
        
        if (width == 0) {
                return 0;
        }

        uint64_t uVal = Bitpack_getu(word, width, lsb);

        /* Getting the sign bit validator */
        uint64_t signBit = shiftLeft(UINT64_1, width - 1);

        if ((uVal & signBit) == 0) {
                return (int64_t) uVal;
        } else {
                uint64_t mask = ~0;
                mask = shiftLeft(mask, width);
                return (int64_t) (mask | uVal);
        }
}

uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value)
{       
        /* Defensive checking */
        assert(width <= 64);
        assert(lsb + width <= 64);
        
        if (!Bitpack_fitsu(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        if (width == 0) {
                return word;
        }
        if (width == 64) {
                return value;
        }

        /* Creating 0 mask */
        uint64_t mask;
        mask = shiftLeft(UINT64_1, width) - 1;
        mask = ~(shiftLeft(mask, lsb));
        
        return shiftLeft(value, lsb) | (mask & word);
}

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,
                      int64_t value)
{
        assert(width <= 64);
        assert(lsb + width <= 64);

        if (!Bitpack_fitss(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        if (width == 0) {
                return word;
        }
        if (width == 64) {
                return (uint64_t) value;
        }
        
        uint64_t mask = (shiftLeft(UINT64_1, width) - 1);
        mask = ((uint64_t) value) & mask;

        return Bitpack_newu(word, width, lsb, mask);
}

static uint64_t shiftLeft(uint64_t u, unsigned shift) 
{
        assert(shift <= 64);
        if (shift == 64) {
                return 0;
        }

        return u << shift;
}

static uint64_t shiftRightU(uint64_t u, unsigned shift) 
{
        assert(shift <= 64);
        if (shift == 64) {
                return 0;
        }

        return u >> shift;
}
