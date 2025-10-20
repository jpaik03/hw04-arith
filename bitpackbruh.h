#ifndef BITPACK_INCLUDED
#define BITPACK_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include "except.h"

extern Except_T Bitpack_Overflow;

// Can n be represented in width number of bits?
bool Bitpack_fitsu(uint64_t n, unsigned width);
bool Bitpack_fitss( int64_t n, unsigned width);

// Accessors
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb);
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb);

// Modifiers (note, we don’t modify, we create new because we don’t want to mess
// with pointers)
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value);
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,
                      int64_t value); 

#endif 

/*
Three design alternatives for the Bitpack module, any of which is acceptable:
- Implement the signed functions using the unsigned functions
- Implement the unsigned functions using the signed functions
- Implement the signed functions and the unsigned functions independently, in 
  such a way that neither is aware of the other
*/

// unsigned
// 2^width > n
// (1 << n) > n

// signed
// 2^(width - 1) > n