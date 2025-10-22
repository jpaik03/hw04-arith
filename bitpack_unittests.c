#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "bitpack.h"
#include "except.h"

/*
 * A simple testing framework.
 * test_assert checks a condition and prints a message.
 * test_report prints a summary of test results.
 */
static int tests_total = 0;
static int tests_failed = 0;

#define RED   "\033[0;31m"
#define GREEN "\033[0;32m"
#define NC    "\033[0m" // No Color

static void test_assert(bool condition, const char *message) {
    tests_total++;
    if (!condition) {
        tests_failed++;
        printf("%sFAILED%s: %s\n", RED, NC, message);
    }
}

static void test_report() {
    printf("\n--- Test Summary ---\n");
    printf("Total tests: %d\n", tests_total);
    if (tests_failed == 0) {
        printf("%sAll tests passed!%s\n", GREEN, NC);
    } else {
        printf("%sTests failed: %d%s\n", RED, tests_failed, NC);
    }
}


/* --- Test functions for each part of the Bitpack interface --- */

void test_fitsu() {
    printf("--- Testing Bitpack_fitsu ---\n");
    // Spec edge cases: width = 0
    test_assert(Bitpack_fitsu(0, 0) == true, "fitsu(0, 0) should be true");
    test_assert(Bitpack_fitsu(1, 0) == false, "fitsu(1, 0) should be false");

    // Spec edge cases: width = 64
    test_assert(Bitpack_fitsu(0xFFFFFFFFFFFFFFFF, 64) == true, "fitsu(max_uint, 64) should be true");
    test_assert(Bitpack_fitsu(0, 64) == true, "fitsu(0, 64) should be true");

    // Boundary conditions
    test_assert(Bitpack_fitsu(255, 8) == true, "fitsu(255, 8) should be true (2^8 - 1)");
    test_assert(Bitpack_fitsu(256, 8) == false, "fitsu(256, 8) should be false (2^8)");
    uint64_t max_31 = (1UL << 31) - 1;
    test_assert(Bitpack_fitsu(max_31, 31) == true, "fitsu(2^31 - 1, 31) should be true");
    test_assert(Bitpack_fitsu(max_31 + 1, 31) == false, "fitsu(2^31, 31) should be false");
    
    // NEW: Width = 1 edge cases
    test_assert(Bitpack_fitsu(0, 1) == true, "fitsu(0, 1) should be true");
    test_assert(Bitpack_fitsu(1, 1) == true, "fitsu(1, 1) should be true");
    test_assert(Bitpack_fitsu(2, 1) == false, "fitsu(2, 1) should be false");
    
    // NEW: Powers of 2 boundaries
    for (int w = 1; w < 64; w++) {
        uint64_t max_val = (w == 63) ? 0x7FFFFFFFFFFFFFFFULL : ((1ULL << w) - 1);
        test_assert(Bitpack_fitsu(max_val, w) == true, "fitsu should accept max value for width");
        if (w < 63) {
            test_assert(Bitpack_fitsu(max_val + 1, w) == false, "fitsu should reject max+1 for width");
        }
    }
    
    // NEW: Width = 63 boundary
    test_assert(Bitpack_fitsu(0x7FFFFFFFFFFFFFFFULL, 63) == true, "fitsu(2^63-1, 63) should be true");
    test_assert(Bitpack_fitsu(0x8000000000000000ULL, 63) == false, "fitsu(2^63, 63) should be false");
    
    // NEW: Very large numbers
    test_assert(Bitpack_fitsu(0xFFFFFFFFFFFFFFFEULL, 64) == true, "fitsu(max-1, 64) should be true");
    test_assert(Bitpack_fitsu(0x8000000000000000ULL, 64) == true, "fitsu(2^63, 64) should be true");
}

void test_fitss() {
    printf("--- Testing Bitpack_fitss ---\n");
    // Spec edge cases: width = 0
    test_assert(Bitpack_fitss(0, 0) == true, "fitss(0, 0) should be true");
    test_assert(Bitpack_fitss(1, 0) == false, "fitss(1, 0) should be false");
    test_assert(Bitpack_fitss(-1, 0) == false, "fitss(-1, 0) should be false");

    // Spec edge cases: width = 1
    test_assert(Bitpack_fitss(0, 1) == true, "fitss(0, 1) should be true");
    test_assert(Bitpack_fitss(-1, 1) == true, "fitss(-1, 1) should be true");
    test_assert(Bitpack_fitss(1, 1) == false, "fitss(1, 1) should be false");
    test_assert(Bitpack_fitss(-2, 1) == false, "fitss(-2, 1) should be false");

    // Boundary conditions for 8 bits: [-128, 127]
    test_assert(Bitpack_fitss(127, 8) == true, "fitss(127, 8) should be true (max pos)");
    test_assert(Bitpack_fitss(128, 8) == false, "fitss(128, 8) should be false (max pos + 1)");
    test_assert(Bitpack_fitss(-128, 8) == true, "fitss(-128, 8) should be true (min neg)");
    test_assert(Bitpack_fitss(-129, 8) == false, "fitss(-129, 8) should be false (min neg - 1)");

    // Boundary conditions for 64 bits
    test_assert(Bitpack_fitss(0x7FFFFFFFFFFFFFFF, 64) == true, "fitss(max_int64, 64) should be true");
    test_assert(Bitpack_fitss(-1, 64) == true, "fitss(-1, 64) should be true");
    int64_t min_int64 = (int64_t)0x8000000000000000ULL;
    test_assert(Bitpack_fitss(min_int64, 64) == true, "fitss(min_int64, 64) should be true");
    
    // NEW: Width = 2 edge cases
    test_assert(Bitpack_fitss(-2, 2) == true, "fitss(-2, 2) should be true");
    test_assert(Bitpack_fitss(1, 2) == true, "fitss(1, 2) should be true");
    test_assert(Bitpack_fitss(2, 2) == false, "fitss(2, 2) should be false");
    test_assert(Bitpack_fitss(-3, 2) == false, "fitss(-3, 2) should be false");
    
    // NEW: Width = 5 (used in arith for b,c,d)
    test_assert(Bitpack_fitss(15, 5) == true, "fitss(15, 5) should be true");
    test_assert(Bitpack_fitss(16, 5) == false, "fitss(16, 5) should be false");
    test_assert(Bitpack_fitss(-16, 5) == true, "fitss(-16, 5) should be true");
    test_assert(Bitpack_fitss(-17, 5) == false, "fitss(-17, 5) should be false");
    
    // NEW: Width = 32 (common size)
    test_assert(Bitpack_fitss(0x7FFFFFFF, 32) == true, "fitss(2^31-1, 32) should be true");
    test_assert(Bitpack_fitss(-0x7FFFFFFF - 1, 32) == true, "fitss(-2^31, 32) should be true");
    test_assert(Bitpack_fitss(0x80000000LL, 32) == false, "fitss(2^31, 32) should be false");
    test_assert(Bitpack_fitss(-0x80000001LL, 32) == false, "fitss(-2^31-1, 32) should be false");
    
    // NEW: Width = 63
    test_assert(Bitpack_fitss(0x3FFFFFFFFFFFFFFFLL, 63) == true, "fitss(2^62-1, 63) should be true");
    test_assert(Bitpack_fitss(-0x4000000000000000LL, 63) == true, "fitss(-2^62, 63) should be true");
    test_assert(Bitpack_fitss(0x4000000000000000LL, 63) == false, "fitss(2^62, 63) should be false");
    test_assert(Bitpack_fitss(-0x4000000000000001LL, 63) == false, "fitss(-2^62-1, 63) should be false");
    
    // NEW: Test every width from 1 to 64
    for (int w = 1; w <= 64; w++) {
        if (w < 64) {
            int64_t max_pos = (1LL << (w - 1)) - 1;
            int64_t min_neg = -(1LL << (w - 1));
            test_assert(Bitpack_fitss(max_pos, w) == true, "fitss should accept max positive");
            test_assert(Bitpack_fitss(min_neg, w) == true, "fitss should accept min negative");
            test_assert(Bitpack_fitss(max_pos + 1, w) == false, "fitss should reject max+1");
            test_assert(Bitpack_fitss(min_neg - 1, w) == false, "fitss should reject min-1");
        }
    }
}

void test_getu() {
    printf("--- Testing Bitpack_getu ---\n");
    test_assert(Bitpack_getu(0x3f4, 6, 2) == 61, "getu(0x3f4, 6, 2) should be 61");
    test_assert(Bitpack_getu(0xFF, 8, 0) == 0xFF, "getu should get value from LSB");
    test_assert(Bitpack_getu(0xFF00000000000000, 8, 56) == 0xFF, "getu should get value from MSB side");
    test_assert(Bitpack_getu(0xAAAAAAAAAAAAAAAA, 0, 32) == 0, "getu with width=0 should return 0");
    test_assert(Bitpack_getu(0x123456789ABCDEF0, 64, 0) == 0x123456789ABCDEF0, "getu should get a full 64-bit word");
    
    // NEW: Test at every bit position for width=1
    for (int lsb = 0; lsb < 64; lsb++) {
        uint64_t word = 1ULL << lsb;
        test_assert(Bitpack_getu(word, 1, lsb) == 1, "getu(single bit set, 1, lsb) should be 1");
        test_assert(Bitpack_getu(~word, 1, lsb) == 0, "getu(single bit clear, 1, lsb) should be 0");
    }
    
    // NEW: Test extracting fields at word boundaries
    test_assert(Bitpack_getu(0xFFFFFFFFFFFFFFFF, 32, 0) == 0xFFFFFFFF, "getu lower 32 bits");
    test_assert(Bitpack_getu(0xFFFFFFFFFFFFFFFF, 32, 32) == 0xFFFFFFFF, "getu upper 32 bits");
    
    // NEW: Test width + lsb = 64 (edge of word)
    test_assert(Bitpack_getu(0x8000000000000000ULL, 1, 63) == 1, "getu MSB only");
    test_assert(Bitpack_getu(0xFFFFFFFFFFFFFFFFULL, 63, 1) == 0x7FFFFFFFFFFFFFFFULL, "getu 63 bits at lsb=1");
    
    // NEW: Alternating bit patterns
    test_assert(Bitpack_getu(0xAAAAAAAAAAAAAAAAULL, 4, 0) == 0xA, "getu alternating bits (0xA)");
    test_assert(Bitpack_getu(0x5555555555555555ULL, 4, 0) == 0x5, "getu alternating bits (0x5)");
    
    // NEW: Extract byte-aligned fields at various positions
    for (int byte_pos = 0; byte_pos < 8; byte_pos++) {
        uint64_t word = 0xFFULL << (byte_pos * 8);
        test_assert(Bitpack_getu(word, 8, byte_pos * 8) == 0xFF, "getu byte-aligned field");
    }
    
    // NEW: Width = 9 (used in arith for 'a')
    test_assert(Bitpack_getu(0x1FF << 23, 9, 23) == 0x1FF, "getu 9-bit field at lsb=23");
    
    // NEW: Multiple adjacent fields
    uint64_t multi_field = 0;
    multi_field |= (0xFULL << 0);  // 4 bits at 0
    multi_field |= (0xFULL << 4);  // 4 bits at 4
    multi_field |= (0xFULL << 8);  // 4 bits at 8
    test_assert(Bitpack_getu(multi_field, 4, 0) == 0xF, "getu first 4-bit field");
    test_assert(Bitpack_getu(multi_field, 4, 4) == 0xF, "getu second 4-bit field");
    test_assert(Bitpack_getu(multi_field, 4, 8) == 0xF, "getu third 4-bit field");
    
    // NEW: Extract full word with lsb=0
    test_assert(Bitpack_getu(0x0123456789ABCDEFULL, 64, 0) == 0x0123456789ABCDEFULL, "getu full word");
    
    // NEW: Zero word extractions
    test_assert(Bitpack_getu(0, 32, 16) == 0, "getu from zero word");
}

void test_gets() {
    printf("--- Testing Bitpack_gets ---\n");
    test_assert(Bitpack_gets(0x0000000000000020, 3, 4) == 2, "should be 2");
    test_assert(Bitpack_gets(0x3f4, 6, 2) == -3, "gets(0x3f4, 6, 2) should be -3");
    test_assert(Bitpack_gets(0xFFFFFFFFFFFFFFFF, 8, 0) == -1, "gets should get -1 from LSB");
    test_assert(Bitpack_gets(0x8000000000000000, 8, 56) == -128, "gets should get -128 from MSB side");
    test_assert(Bitpack_gets(0xAAAAAAAAAAAAAAAA, 0, 32) == 0, "gets with width=0 should return 0");
    test_assert(Bitpack_gets(0xFFFFFFFFFFFFFFFF, 64, 0) == -1, "gets should get a full 64-bit word");
    test_assert(Bitpack_gets(0x7FFFFFFFFFFFFFFF, 64, 0) == 0x7FFFFFFFFFFFFFFF, "gets should get max positive 64-bit word");
    
    // NEW: Test sign extension for width=1 at every position
    for (int lsb = 0; lsb < 64; lsb++) {
        uint64_t word = 1ULL << lsb;
        test_assert(Bitpack_gets(word, 1, lsb) == -1, "gets(single bit set, 1, lsb) should be -1");
        test_assert(Bitpack_gets(~word, 1, lsb) == 0, "gets(single bit clear, 1, lsb) should be 0");
    }
    
    // NEW: Width = 5 signed values (arith b,c,d range)
    test_assert(Bitpack_gets(0x0F << 8, 5, 8) == 15, "gets(15, 5, 8) should be 15");
    test_assert(Bitpack_gets(0x10 << 8, 5, 8) == -16, "gets(0x10, 5, 8) should be -16");
    test_assert(Bitpack_gets(0x1F << 8, 5, 8) == -1, "gets(0x1F, 5, 8) should be -1");
    
    // NEW: Width = 8, all possible signed values
    for (int val = -128; val <= 127; val++) {
        uint64_t word = ((uint64_t)(uint8_t)val) << 10;
        test_assert(Bitpack_gets(word, 8, 10) == val, "gets should correctly extract 8-bit signed value");
    }
    
    // NEW: Width = 32 signed values
    test_assert(Bitpack_gets(0x7FFFFFFFULL, 32, 0) == 0x7FFFFFFF, "gets(max 32-bit pos, 32, 0)");
    test_assert(Bitpack_gets(0x80000000ULL, 32, 0) == -0x80000000LL, "gets(min 32-bit neg, 32, 0)");
    test_assert(Bitpack_gets(0xFFFFFFFFULL, 32, 0) == -1, "gets(0xFFFFFFFF, 32, 0) should be -1");
    
    // NEW: Sign bit position
    test_assert(Bitpack_gets(0x80, 8, 0) == -128, "gets sign bit for 8-bit field");
    test_assert(Bitpack_gets(0x40, 8, 0) == 64, "gets no sign bit for 8-bit field");
    
    // NEW: Multiple fields with different signs
    uint64_t mixed = 0;
    mixed |= (0x1FULL << 0);   // -1 in 5 bits
    mixed |= (0x0FULL << 5);   // 15 in 5 bits
    mixed |= (0x10ULL << 10);  // -16 in 5 bits
    test_assert(Bitpack_gets(mixed, 5, 0) == -1, "gets first field: -1");
    test_assert(Bitpack_gets(mixed, 5, 5) == 15, "gets second field: 15");
    test_assert(Bitpack_gets(mixed, 5, 10) == -16, "gets third field: -16");
    
    // NEW: Width = 63 with sign extension
    test_assert(Bitpack_gets(0x7FFFFFFFFFFFFFFFULL, 64, 0) == 0x7FFFFFFFFFFFFFFFLL, "gets max 64-bit positive");
    test_assert(Bitpack_gets(0x8000000000000000ULL, 64, 0) == (int64_t)0x8000000000000000ULL, "gets min 64-bit negative");
    
    // NEW: Small widths at high positions
    test_assert(Bitpack_gets(0x8000000000000000ULL, 1, 63) == -1, "gets(MSB, 1, 63) should be -1");
    test_assert(Bitpack_gets(0x4000000000000000ULL, 1, 62) == 0, "gets(0, 1, 62) should be 0");
}

void test_newu() {
    printf("--- Testing Bitpack_newu ---\n");
    uint64_t word;
    
    word = Bitpack_newu(0xAAAAAAAAAAAAAAAA, 0, 32, 0);
    test_assert(word == 0xAAAAAAAAAAAAAAAA, "newu with width=0 should not change the word");

    word = Bitpack_newu(0, 64, 0, 0x123456789ABCDEF0);
    test_assert(word == 0x123456789ABCDEF0, "newu should replace a full 64-bit word");

    uint64_t initial_word = 0xFFFFFFFFFFFFFFFF;
    word = Bitpack_newu(initial_word, 8, 32, 0);
    uint64_t expected_word = 0xFFFFFF00FFFFFFFFULL;
    test_assert(word == expected_word, "newu should not touch bits outside the field");

    word = Bitpack_newu(0, 8, 32, 0xFF);
    expected_word = 0x000000FF00000000ULL;
    test_assert(word == expected_word, "newu should not touch bits on an empty word");
    
    // NEW: Set individual bits across entire word
    word = 0;
    for (int bit = 0; bit < 64; bit++) {
        word = Bitpack_newu(word, 1, bit, 1);
    }
    test_assert(word == 0xFFFFFFFFFFFFFFFFULL, "newu setting all bits one by one");
    
    // NEW: Clear individual bits across entire word
    word = 0xFFFFFFFFFFFFFFFFULL;
    for (int bit = 0; bit < 64; bit++) {
        word = Bitpack_newu(word, 1, bit, 0);
    }
    test_assert(word == 0, "newu clearing all bits one by one");
    
    // NEW: Update byte-aligned fields
    word = 0;
    for (int byte_pos = 0; byte_pos < 8; byte_pos++) {
        word = Bitpack_newu(word, 8, byte_pos * 8, 0x11 * (byte_pos + 1));
    }
    test_assert(word == 0x8877665544332211ULL, "newu setting all bytes");
    
    // NEW: Arith codeword construction simulation
    word = 0;
    word = Bitpack_newu(word, 9, 23, 511);  // a
    word = Bitpack_newu(word, 4, 4, 15);    // indexPb
    word = Bitpack_newu(word, 4, 0, 15);    // indexPr
    test_assert((word & 0xFF800000ULL) == (511ULL << 23), "newu arith 'a' field");
    test_assert((word & 0xF0ULL) == (15ULL << 4), "newu arith indexPb field");
    test_assert((word & 0x0FULL) == 15ULL, "newu arith indexPr field");
    
    // NEW: Update same field multiple times
    word = 0;
    word = Bitpack_newu(word, 8, 16, 0xAA);
    test_assert(Bitpack_getu(word, 8, 16) == 0xAA, "newu first update");
    word = Bitpack_newu(word, 8, 16, 0x55);
    test_assert(Bitpack_getu(word, 8, 16) == 0x55, "newu second update");
    word = Bitpack_newu(word, 8, 16, 0xFF);
    test_assert(Bitpack_getu(word, 8, 16) == 0xFF, "newu third update");
    
    // NEW: Adjacent non-overlapping fields
    word = 0;
    word = Bitpack_newu(word, 4, 0, 0xF);
    word = Bitpack_newu(word, 4, 4, 0xF);
    word = Bitpack_newu(word, 4, 8, 0xF);
    test_assert(word == 0xFFF, "newu three adjacent 4-bit fields");
    
    // NEW: Width + lsb = 64 boundary
    word = 0;
    word = Bitpack_newu(word, 63, 1, 0x7FFFFFFFFFFFFFFFULL);
    test_assert(Bitpack_getu(word, 63, 1) == 0x7FFFFFFFFFFFFFFFULL, "newu 63 bits at lsb=1");
    
    // NEW: Update preserves other fields
    word = 0x123456789ABCDEF0ULL;
    word = Bitpack_newu(word, 8, 24, 0xCC);
    test_assert((word & 0xFF000000ULL) == 0xCC000000ULL, "newu updated field");
    test_assert((word & 0x00FFFFFFULL) == 0x009BDEF0ULL, "newu preserved lower bits");
    test_assert((word & 0xFF00000000000000ULL) == 0x1200000000000000ULL, "newu preserved upper bits");
}

void test_news() {
    printf("--- Testing Bitpack_news ---\n");
    uint64_t word;

    word = Bitpack_news(0, 64, 0, -1);
    test_assert(word == 0xFFFFFFFFFFFFFFFF, "news(-1) should create all 1s for full word");

    word = Bitpack_news(0, 8, 0, -1);
    test_assert(word == 0xFF, "news(-1) should create 0xFF in 8 bits");
    
    word = Bitpack_news(0, 8, 0, -128);
    test_assert(word == 0x80, "news(-128) should create 0x80 in 8 bits");
    
    uint64_t initial_word = 0;
    word = Bitpack_news(initial_word, 8, 32, -1);
    uint64_t expected_word = 0x000000FF00000000ULL;
    test_assert(word == expected_word, "news should not touch bits outside the field");
    
    // NEW: Width = 5, all values from -16 to 15
    for (int val = -16; val <= 15; val++) {
        word = Bitpack_news(0, 5, 10, val);
        test_assert(Bitpack_gets(word, 5, 10) == val, "news/gets round trip for 5-bit signed");
    }
    
    // NEW: Multiple signed fields in one word
    word = 0;
    word = Bitpack_news(word, 5, 0, -1);    // bits 0-4
    word = Bitpack_news(word, 5, 5, 15);    // bits 5-9
    word = Bitpack_news(word, 5, 10, -16);  // bits 10-14
    test_assert(Bitpack_gets(word, 5, 0) == -1, "news field 1: -1");
    test_assert(Bitpack_gets(word, 5, 5) == 15, "news field 2: 15");
    test_assert(Bitpack_gets(word, 5, 10) == -16, "news field 3: -16");
    
    // NEW: Arith b,c,d simulation
    word = 0;
    word = Bitpack_news(word, 5, 18, 15);   // b = 15
    word = Bitpack_news(word, 5, 13, -15);  // c = -15
    word = Bitpack_news(word, 5, 8, 0);     // d = 0
    test_assert(Bitpack_gets(word, 5, 18) == 15, "news b=15");
    test_assert(Bitpack_gets(word, 5, 13) == -15, "news c=-15");
    test_assert(Bitpack_gets(word, 5, 8) == 0, "news d=0");
    
    // NEW: Zero value
    word = Bitpack_news(0xFFFFFFFFFFFFFFFFULL, 8, 24, 0);
    test_assert(Bitpack_gets(word, 8, 24) == 0, "news(0) in field of all 1s");
    
    // NEW: Positive values near boundary
    word = Bitpack_news(0, 8, 0, 127);
    test_assert(Bitpack_gets(word, 8, 0) == 127, "news(127) in 8 bits");
    
    // NEW: Negative values near boundary
    word = Bitpack_news(0, 8, 0, -128);
    test_assert(Bitpack_gets(word, 8, 0) == -128, "news(-128) in 8 bits");
    
    // NEW: Width = 1 for -1 and 0
    word = Bitpack_news(0, 1, 31, -1);
    test_assert(Bitpack_gets(word, 1, 31) == -1, "news(-1, 1, 31)");
    word = Bitpack_news(0, 1, 31, 0);
    test_assert(Bitpack_gets(word, 1, 31) == 0, "news(0, 1, 31)");
    
    // NEW: Update signed field multiple times
    word = 0;
    word = Bitpack_news(word, 8, 16, -50);
    test_assert(Bitpack_gets(word, 8, 16) == -50, "news first: -50");
    word = Bitpack_news(word, 8, 16, 100);
    test_assert(Bitpack_gets(word, 8, 16) == 100, "news second: 100");
    word = Bitpack_news(word, 8, 16, -1);
    test_assert(Bitpack_gets(word, 8, 16) == -1, "news third: -1");
    
    // NEW: Width = 32 signed
    word = Bitpack_news(0, 32, 16, -1);
    test_assert(Bitpack_gets(word, 32, 16) == -1, "news(-1, 32, 16)");
    word = Bitpack_news(0, 32, 16, 0x7FFFFFFF);
    test_assert(Bitpack_gets(word, 32, 16) == 0x7FFFFFFF, "news(max 32-bit, 32, 16)");
}

void test_round_trip_and_interactions() {
    printf("--- Testing round trips and interactions ---\n");
    uint64_t word = 0;
    
    // Unsigned round trip
    word = Bitpack_newu(word, 10, 5, 1023);
    test_assert(Bitpack_getu(word, 10, 5) == 1023, "Unsigned round trip failed");

    // Signed round trip (positive)
    word = Bitpack_news(word, 7, 20, 50);
    test_assert(Bitpack_gets(word, 7, 20) == 50, "Signed positive round trip failed");
    
    // Signed round trip (negative)
    word = Bitpack_news(word, 9, 40, -200);
    test_assert(Bitpack_gets(word, 9, 40) == -200, "Signed negative round trip failed");
    
    // Insert signed, get unsigned
    word = Bitpack_news(0, 8, 20, -1);
    test_assert(Bitpack_getu(word, 8, 20) == 0xFF, "getu should return raw bits (0xFF) for -1");
    
    // Insert unsigned, get signed
    word = Bitpack_newu(0, 8, 40, 0xFF);
    test_assert(Bitpack_gets(word, 8, 40) == -1, "gets should interpret bit pattern 0xFF as -1");
    
    // NEW: Comprehensive arith codeword simulation
    word = 0;
    word = Bitpack_newu(word, 9, 23, 511);   // a
    word = Bitpack_news(word, 5, 18, 15);    // b
    word = Bitpack_news(word, 5, 13, -15);   // c
    word = Bitpack_news(word, 5, 8, 0);      // d
    word = Bitpack_newu(word, 4, 4, 15);     // indexPb
    word = Bitpack_newu(word, 4, 0, 15);     // indexPr
    
    test_assert(Bitpack_getu(word, 9, 23) == 511, "arith codeword: a");
    test_assert(Bitpack_gets(word, 5, 18) == 15, "arith codeword: b");
    test_assert(Bitpack_gets(word, 5, 13) == -15, "arith codeword: c");
    test_assert(Bitpack_gets(word, 5, 8) == 0, "arith codeword: d");
    test_assert(Bitpack_getu(word, 4, 4) == 15, "arith codeword: indexPb");
    test_assert(Bitpack_getu(word, 4, 0) == 15, "arith codeword: indexPr");
    
    // NEW: Test round trips at every position for various widths
    for (int width = 1; width <= 16; width++) {
        for (int lsb = 0; lsb + width <= 64; lsb += 8) {
            uint64_t val = (1ULL << width) - 1;
            word = Bitpack_newu(0, width, lsb, val);
            test_assert(Bitpack_getu(word, width, lsb) == val, "Round trip unsigned");
            
            if (width > 1) {
                int64_t sval = (1LL << (width - 1)) - 1;
                word = Bitpack_news(0, width, lsb, sval);
                test_assert(Bitpack_gets(word, width, lsb) == sval, "Round trip signed pos");
                
                sval = -(1LL << (width - 1));
                word = Bitpack_news(0, width, lsb, sval);
                test_assert(Bitpack_gets(word, width, lsb) == sval, "Round trip signed neg");
            }
        }
    }
    
    // NEW: Law: getu(newu(word, w, lsb, val), w, lsb) == val
    word = 0xA5A5A5A5A5A5A5A5ULL;
    word = Bitpack_newu(word, 12, 20, 0xABC);
    test_assert(Bitpack_getu(word, 12, 20) == 0xABC, "Law: getu(newu(w,w,l,v),w,l) == v");
    
    // NEW: Law: if lsb2 >= w + lsb, then field doesn't overlap
    word = 0xFFFFFFFFFFFFFFFFULL;
    word = Bitpack_newu(word, 8, 8, 0);
    test_assert(Bitpack_getu(word, 8, 0) == 0xFF, "Non-overlapping field unchanged (lower)");
    test_assert(Bitpack_getu(word, 8, 16) == 0xFF, "Non-overlapping field unchanged (upper)");
    
    // NEW: Multiple updates to same word
    word = 0;
    for (int i = 0; i < 8; i++) {
        word = Bitpack_newu(word, 8, i * 8, i * 0x11);
    }
    for (int i = 0; i < 8; i++) {
        test_assert(Bitpack_getu(word, 8, i * 8) == (uint64_t)(i * 0x11), "Multiple field updates");
    }
    
    // NEW: Alternating signed/unsigned fields
    word = 0;
    word = Bitpack_newu(word, 4, 0, 15);
    word = Bitpack_news(word, 4, 4, -8);
    word = Bitpack_newu(word, 4, 8, 15);
    word = Bitpack_news(word, 4, 12, -8);
    test_assert(Bitpack_getu(word, 4, 0) == 15, "Alternating: unsigned 1");
    test_assert(Bitpack_gets(word, 4, 4) == -8, "Alternating: signed 1");
    test_assert(Bitpack_getu(word, 4, 8) == 15, "Alternating: unsigned 2");
    test_assert(Bitpack_gets(word, 4, 12) == -8, "Alternating: signed 2");
}

void test_overflow() {
    printf("--- Testing Bitpack overflow exceptions ---\n");
    bool exception_caught;

    // Test newu overflow
    exception_caught = false;
    TRY
        Bitpack_newu(0, 4, 0, 16);
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(exception_caught, "newu must raise Bitpack_Overflow for value 16 in 4 bits");
    
    // Test newu at the limit (should not overflow)
    exception_caught = false;
    TRY
        Bitpack_newu(0, 4, 0, 15); 
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(!exception_caught, "newu must NOT raise overflow for value 15 in 4 bits");

    // Test news overflow (positive)
    exception_caught = false;
    TRY
        Bitpack_news(0, 5, 0, 16);
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(exception_caught, "news must raise Bitpack_Overflow for value 16 in 5 bits");
    
    // Test news at positive limit (should not overflow)
    exception_caught = false;
    TRY
        Bitpack_news(0, 5, 0, 15);
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(!exception_caught, "news must NOT raise overflow for value 15 in 5 bits");

    // Test news overflow (negative)
    exception_caught = false;
    TRY
        Bitpack_news(0, 5, 0, -17);
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(exception_caught, "news must raise Bitpack_Overflow for value -17 in 5 bits");

    // Test news at negative limit (should not overflow)
    exception_caught = false;
    TRY
        Bitpack_news(0, 5, 0, -16);
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(!exception_caught, "news must NOT raise overflow for value -16 in 5 bits");
    
    // NEW: Test width = 1 overflow
    exception_caught = false;
    TRY
        Bitpack_newu(0, 1, 0, 2);
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(exception_caught, "newu(2, 1) should overflow");
    
    exception_caught = false;
    TRY
        Bitpack_news(0, 1, 0, 1);
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(exception_caught, "news(1, 1) should overflow");
    
    exception_caught = false;
    TRY
        Bitpack_news(0, 1, 0, -2);
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(exception_caught, "news(-2, 1) should overflow");
    
    // NEW: Test width = 9 (arith 'a' field) overflow
    exception_caught = false;
    TRY
        Bitpack_newu(0, 9, 23, 512);
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(exception_caught, "newu(512, 9) should overflow");
    
    exception_caught = false;
    TRY
        Bitpack_newu(0, 9, 23, 511);
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(!exception_caught, "newu(511, 9) should NOT overflow");
    
    // NEW: Test at width = 63
    exception_caught = false;
    TRY
        Bitpack_newu(0, 63, 0, 0x8000000000000000ULL);
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(exception_caught, "newu(2^63, 63) should overflow");
    
    exception_caught = false;
    TRY
        Bitpack_newu(0, 63, 0, 0x7FFFFFFFFFFFFFFFULL);
    EXCEPT(Bitpack_Overflow)
        exception_caught = true;
    END_TRY;
    test_assert(!exception_caught, "newu(2^63-1, 63) should NOT overflow");
    
    // NEW: Test max values for different widths
    for (int w = 1; w < 64; w++) {
        uint64_t max_val = (w == 63) ? 0x7FFFFFFFFFFFFFFFULL : ((1ULL << w) - 1);
        
        exception_caught = false;
        TRY
            Bitpack_newu(0, w, 0, max_val);
        EXCEPT(Bitpack_Overflow)
            exception_caught = true;
        END_TRY;
        test_assert(!exception_caught, "newu at max value should not overflow");
        
        if (w < 63) {
            exception_caught = false;
            TRY
                Bitpack_newu(0, w, 0, max_val + 1);
            EXCEPT(Bitpack_Overflow)
                exception_caught = true;
            END_TRY;
            test_assert(exception_caught, "newu at max+1 should overflow");
        }
    }
    
    // NEW: Test signed max values
    for (int w = 2; w <= 32; w++) {
        int64_t max_pos = (1LL << (w - 1)) - 1;
        int64_t min_neg = -(1LL << (w - 1));
        
        exception_caught = false;
        TRY
            Bitpack_news(0, w, 0, max_pos);
        EXCEPT(Bitpack_Overflow)
            exception_caught = true;
        END_TRY;
        test_assert(!exception_caught, "news at max positive should not overflow");
        
        exception_caught = false;
        TRY
            Bitpack_news(0, w, 0, max_pos + 1);
        EXCEPT(Bitpack_Overflow)
            exception_caught = true;
        END_TRY;
        test_assert(exception_caught, "news at max+1 should overflow");
        
        exception_caught = false;
        TRY
            Bitpack_news(0, w, 0, min_neg);
        EXCEPT(Bitpack_Overflow)
            exception_caught = true;
        END_TRY;
        test_assert(!exception_caught, "news at min negative should not overflow");
        
        exception_caught = false;
        TRY
            Bitpack_news(0, w, 0, min_neg - 1);
        EXCEPT(Bitpack_Overflow)
            exception_caught = true;
        END_TRY;
        test_assert(exception_caught, "news at min-1 should overflow");
    }
}

void test_edge_cases_and_stress() {
    printf("--- Testing edge cases and stress tests ---\n");
    uint64_t word;
    
    // NEW: All zeros word operations
    word = 0;
    test_assert(Bitpack_getu(word, 64, 0) == 0, "getu all zeros");
    test_assert(Bitpack_gets(word, 64, 0) == 0, "gets all zeros");
    word = Bitpack_newu(word, 64, 0, 0);
    test_assert(word == 0, "newu(0) on zero word");
    
    // NEW: All ones word operations
    word = 0xFFFFFFFFFFFFFFFFULL;
    test_assert(Bitpack_getu(word, 64, 0) == 0xFFFFFFFFFFFFFFFFULL, "getu all ones");
    test_assert(Bitpack_gets(word, 64, 0) == -1, "gets all ones");
    
    // NEW: Checkerboard patterns
    word = 0xAAAAAAAAAAAAAAAAULL;
    for (int lsb = 0; lsb < 64; lsb += 2) {
        test_assert(Bitpack_getu(word, 1, lsb) == 0, "Checkerboard even bits");
        test_assert(Bitpack_getu(word, 1, lsb + 1) == 1, "Checkerboard odd bits");
    }
    
    word = 0x5555555555555555ULL;
    for (int lsb = 0; lsb < 64; lsb += 2) {
        test_assert(Bitpack_getu(word, 1, lsb) == 1, "Inverse checkerboard even bits");
        test_assert(Bitpack_getu(word, 1, lsb + 1) == 0, "Inverse checkerboard odd bits");
    }
    
    // NEW: Stress test - build word bit by bit
    word = 0;
    for (int i = 0; i < 64; i++) {
        word = Bitpack_newu(word, 1, i, i % 2);
    }
    test_assert(word == 0xAAAAAAAAAAAAAAAAULL, "Build checkerboard bit by bit");
    
    // NEW: Stress test - extract every possible 8-bit field
    word = 0x0123456789ABCDEFULL;
    uint8_t expected_bytes[] = {0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01};
    for (int i = 0; i < 8; i++) {
        test_assert(Bitpack_getu(word, 8, i * 8) == expected_bytes[i], "Extract byte");
    }
    
    // NEW: Stress test - update every byte
    word = 0;
    for (int i = 0; i < 8; i++) {
        word = Bitpack_newu(word, 8, i * 8, expected_bytes[i]);
    }
    test_assert(word == 0x0123456789ABCDEFULL, "Build word byte by byte");
    
    // NEW: Width spans multiple bytes
    word = 0xFFFFFFFFFFFFFFFFULL;
    word = Bitpack_newu(word, 24, 20, 0);
    test_assert((word & 0x00000FFFFF00000ULL) == 0, "Clear 24-bit field");
    test_assert((word & 0xFFFFF00000FFFFFULL) == 0xFFFFF00000FFFFFULL, "Preserve surrounding bits");
    
    // NEW: Prime number widths and offsets (harder to get wrong)
    word = 0;
    word = Bitpack_newu(word, 7, 11, 127);
    test_assert(Bitpack_getu(word, 7, 11) == 127, "Prime width/offset 1");
    word = Bitpack_newu(word, 13, 23, 8191);
    test_assert(Bitpack_getu(word, 13, 23) == 8191, "Prime width/offset 2");
    
    // NEW: Sign bit exactly at width boundaries
    for (int w = 2; w <= 16; w++) {
        uint64_t sign_bit_set = 1ULL << (w - 1);
        word = Bitpack_newu(0, w, 20, sign_bit_set);
        test_assert(Bitpack_gets(word, w, 20) == -(1LL << (w - 1)), "Sign bit set");
        
        uint64_t sign_bit_clear = sign_bit_set - 1;
        word = Bitpack_newu(0, w, 20, sign_bit_clear);
        test_assert(Bitpack_gets(word, w, 20) == (1LL << (w - 1)) - 1, "Sign bit clear");
    }
}

int main() {
    test_fitsu();
    test_fitss();
    test_getu();
    test_gets();
    test_newu();
    test_news();
    test_round_trip_and_interactions();
    test_overflow();
    test_edge_cases_and_stress();

    test_report();
    return (tests_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}