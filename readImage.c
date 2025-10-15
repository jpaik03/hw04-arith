/*
 *      readImage.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      // TODO: description
 */

// TODO: INCLUDES (MOST SHOULD BE HERE)
#include "readImage.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pnm.h"
#include "a2methods.h"

// TODO: FUNCTION CONTRACT
Pnm_ppm readImage(FILE *fp) {
        assert(fp != NULL);
        A2Methods_T uarray2_methods_blocked = uarray2_methods_blocked;
        Pnm_ppm img = Pnm_ppmread(fp, uarray2_methods_blocked);
        trimImage(img);
        return img;
}



// TODO: FUNCTION CONTRACT
void trimImage(Pnm_ppm img) {
        assert(img != NULL);

        (void)img;
}