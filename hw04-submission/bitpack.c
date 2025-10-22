/* 
 *      bitpack.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      Implementation of the Bitpack interface for creating and manipulating
 *      bits within a 64-bit word. This file provides functions to check if
 *      values fit, extract fields, and create new words with updated fields.
 */

#include <assert.h>
#include <stdint.h>

#include "bitpack.h"
#include "except.h"

Except_T Bitpack_Overflow = { "Overflow packing bits" };

/* A constant for the value 1 as a 64-bit unsigned integer */
const uint64_t UINT64_1 = 1;

/* Initialize helper functions, see function contracts below */
static uint64_t shiftLeft(uint64_t u, unsigned shift);
static uint64_t shiftRightU(uint64_t u, unsigned shift);

/******** Bitpack_fitsu ********
 *
 * Checks if a given unsigned 64-bit integer can be represented in 'width' bits.
 *
 * Parameters:
 *      uint64_t n:     The unsigned integer to check
 *      unsigned width: The number of bits available for representation
 * Returns:
 *      A boolean indicating true if 'n' fits, false otherwise.
 * Expects:
 *      width is less than or equal to 64.
 * Notes:
 *      Throws a CRE if width > 64.
 ************************/
bool Bitpack_fitsu(uint64_t n, unsigned width) 
{
        assert(width <= 64);

        if (width == 64) {
                return true;
        }
        
        /* Valid unsigned range is [0, 2^width - 1] */
        uint64_t max_val = shiftLeft(UINT64_1, width) - 1;
        return n <= max_val;
}

/******** Bitpack_fitss ********
 *
 * Checks if a given signed 64-bit integer can be represented in 'width' bits
 * using two's complement.
 *
 * Parameters:
 *      int64_t n:      The signed integer to check
 *      unsigned width: The number of bits available for representation
 * Returns:
 *      A boolean indicating true if 'n' fits, false otherwise.
 * Expects:
 *      width is less than or equal to 64.
 * Notes:
 *      Throws a CRE if width > 64.
 ************************/
bool Bitpack_fitss(int64_t n, unsigned width) 
{
        assert(width <= 64);

        if (width == 64) {
                return true;
        }

        if (width == 0) {
                return n == 0;
        }
        
        /* Valid signed range is [-2^(width-1), 2^(width-1) - 1] */
        uint64_t powTwo = shiftLeft(UINT64_1, width - UINT64_1);

        int64_t min = -(int64_t) powTwo;
        int64_t max = (int64_t) powTwo - UINT64_1;

        return n >= min && n <= max;
}

/******** Bitpack_getu ********
 *
 * Extracts an unsigned value of 'width' bits starting at 'lsb' from a word.
 *
 * Parameters:
 *      uint64_t word:  The 64-bit word to extract from
 *      unsigned width: The width of the field to extract
 *      unsigned lsb:   The least significant bit of the field
 * Returns:
 *      The extracted value as a uint64_t.
 * Expects:
 *      width and lsb are valid and the field is within the 64-bit word.
 * Notes:
 *      Throws a CRE if width > 64.
 *      Throws a CRE if lsb + width > 64.
 *      
 ************************/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= 64);
        assert(lsb + width <= 64);

        if (width == 0) {
                return 0;
        }

        /* Create a mask of 'width' ones to isolate the field */
        uint64_t mask = shiftLeft(UINT64_1, width) - UINT64_1;
        
        /* Shift the word to bring the field to the LSB, then apply the mask */
        word = shiftRightU(word, lsb);
        return word & mask;
}

/******** Bitpack_gets ********
 *
 * Extracts a signed value of 'width' bits starting at 'lsb' from a word.
 *
 * Parameters:
 *      uint64_t word:  The 64-bit word to extract from
 *      unsigned width: The width of the field to extract
 *      unsigned lsb:   The least significant bit of the field
 * Returns:
 *      The extracted value as an int64_t, correctly sign-extended.
 * Expects:
 *      width and lsb are valid and the field is within the 64-bit word.
 * Notes:
 *      Throws a CRE if width > 64.
 *      Throws a CRE if lsb + width > 64.
 ************************/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= 64);
        assert(lsb + width <= 64);
        
        if (width == 0) {
                return 0;
        }

        /* Get the raw bit pattern of the field */
        uint64_t uVal = Bitpack_getu(word, width, lsb);

        /* Identify the sign bit for this field */
        uint64_t signBit = shiftLeft(UINT64_1, width - 1);

        /* Check the sign bit and return accordingly */
        if ((uVal & signBit) == 0) {
                /* Positive; unsigned value is the correct representation */
                return (int64_t) uVal;
        } else {
                /* Negative; Fill in higher bits with 1s */
                uint64_t mask = ~0;
                mask = shiftLeft(mask, width);
                return (int64_t) (mask | uVal);
        }
}

/******** Bitpack_newu ********
 *
 * Returns a new word identical to the old, but with the field of 'width' at
 * 'lsb' replaced by the given unsigned 'value'.
 *
 * Parameters:
 *      uint64_t word:  The original 64-bit word
 *      unsigned width: The width of the field to update
 *      unsigned lsb:   The least significant bit of the field
 *      uint64_t value: The new unsigned value for the field
 * Returns:
 *      A new 64-bit word with the updated field.
 * Expects:
 *      The field is within the word.
 * Notes:
 *      Throws a CRE if width > 64.
 *      Throws a CRE if lsb + width > 64.
 *      Raises Bitpack_Overflow if 'value' does not fit in 'width' bits.
 ************************/

uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value)
{       
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

        /* Create a mask to clear the target field */
        uint64_t mask;
        mask = shiftLeft(UINT64_1, width) - 1;
        mask = ~(shiftLeft(mask, lsb));

        /* Clear the field in the original word, then OR in new shifted value */
        return shiftLeft(value, lsb) | (mask & word);
}

/******** Bitpack_news ********
 *
 * Returns a new word identical to the old, but with the field of 'width' at
 * 'lsb' replaced by the given signed 'value'.
 *
 * Parameters:
 *      uint64_t word:  The original 64-bit word
 *      unsigned width: The width of the field to update
 *      unsigned lsb:   The least significant bit of the field
 *      int64_t value:  The new signed value for the field
 * Returns:
 *      A new 64-bit word with the updated field.
 * Expects:
 *      The field is within the word.
 * Notes:
 *      Throws a CRE if width > 64.
 *      Throws a CRE if lsb + width > 64.
 *      Raises Bitpack_Overflow if 'value' does not fit in 'width' bits.
 ************************/
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
        
        /* Get the 'width'-bit representation of the signed value */
        uint64_t mask = (shiftLeft(UINT64_1, width) - 1);
        mask = ((uint64_t) value) & mask;

        return Bitpack_newu(word, width, lsb, mask);
}

/******** shiftLeft ********
 *
 * Safely performs a left shift on a 64-bit unsigned integer.
 *
 * Parameters:
 *      uint64_t u:       The unsigned integer to shift
 *      unsigned shift:   The number of bits to shift left
 * Returns:
 *      The result of the left shift.
 * Expects:
 *      shift is less than or equal to 64.
 * Notes:
 *      Throws a CRE if shift > 64.
 *      Handles the C undefined behavior of shifting by >= word size. If shift
 *      is 64 or greater, it returns 0, as all bits would be shifted off.
 ************************/
static uint64_t shiftLeft(uint64_t u, unsigned shift) 
{
        assert(shift <= 64);

        if (shift == 64) {
                return 0;
        }

        return u << shift;
}

/******** shiftRightU ********
 *
 * Safely performs an unsigned right shift on a 64-bit unsigned integer.
 *
 * Parameters:
 *      uint64_t u:       The unsigned integer to shift
 *      unsigned shift:   The number of bits to shift right
 * Returns:
 *      The result of the right shift, with zero-filling from the left.
 * Expects:
 *      shift is less than or equal to 64.
 * Notes:
 *      Throws a CRE if shift > 64.
 *      Handles the C undefined behavior of shifting by >= word size. If shift
 *      is 64 or greater, it returns 0, as all bits would be shifted off.
 ************************/
static uint64_t shiftRightU(uint64_t u, unsigned shift) 
{
        assert(shift <= 64);

        if (shift == 64) {
                return 0;
        }

        return u >> shift;
}
