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

bool Bitpack_fitsu(uint64_t n, unsigned width) 
{

}

bool Bitpack_fitss( int64_t n, unsigned width) 
{

}
