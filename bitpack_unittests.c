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
    int64_t min_int64 = -0x7FFFFFFFFFFFFFFF - 1;
    test_assert(Bitpack_fitss(min_int64, 64) == true, "fitss(min_int64, 64) should be true");
}

void test_getu() {
    printf("--- Testing Bitpack_getu ---\n");
    test_assert(Bitpack_getu(0x3f4, 6, 2) == 61, "getu(0x3f4, 6, 2) should be 61");
    test_assert(Bitpack_getu(0xFF, 8, 0) == 0xFF, "getu should get value from LSB");
    test_assert(Bitpack_getu(0xFF00000000000000, 8, 56) == 0xFF, "getu should get value from MSB side");
    test_assert(Bitpack_getu(0xAAAAAAAAAAAAAAAA, 0, 32) == 0, "getu with width=0 should return 0");
    test_assert(Bitpack_getu(0x123456789ABCDEF0, 64, 0) == 0x123456789ABCDEF0, "getu should get a full 64-bit word");
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
    word = Bitpack_news(initial_word, 8, 32, -1); // Insert -1 (0xFF) in the middle
    uint64_t expected_word = 0x000000FF00000000ULL;
    test_assert(word == expected_word, "news should not touch bits outside the field");
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
    word = Bitpack_news(0, 8, 20, -1); // -1 is 0xFF
    test_assert(Bitpack_getu(word, 8, 20) == 0xFF, "getu should return raw bits (0xFF) for -1");
    
    // Insert unsigned, get signed
    word = Bitpack_newu(0, 8, 40, 0xFF); // 0xFF has sign bit set
    test_assert(Bitpack_gets(word, 8, 40) == -1, "gets should interpret bit pattern 0xFF as -1");
}

void test_overflow() {
    printf("--- Testing Bitpack overflow exceptions ---\n");
    bool exception_caught;

    // Test newu overflow
    exception_caught = false;
    TRY
        Bitpack_newu(0, 4, 0, 16); // 16 does not fit in 4 unsigned bits
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
        Bitpack_news(0, 5, 0, 16); // 16 does not fit in 5 signed bits
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
        Bitpack_news(0, 5, 0, -17); // -17 does not fit in 5 signed bits
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

    test_report();
    return (tests_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

