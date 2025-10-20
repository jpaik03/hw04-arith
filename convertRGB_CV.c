/*
 *      convertRGB_CV.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      // TODO: description
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "convertRGB_CV.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "a2methods.h"

// TODO: ASK IF THIS WE SHOULD DO THIS AND WHY THERE ISNT UNDEF
#define T UArray2b_T

const int PIXEL_BLOCKSIZE = 2;

static void applyPixelToFloats(int col, int row, A2Methods_UArray2 pixels, 
                             void *elem, void *cl);

struct pixelInfo {
        float r, g, b;
};

struct applyPixelToFloatsClosure {
        T RGBFloats;
        Pnm_ppm img;
        A2Methods_T methods;
};

T getRGBFloats(Pnm_ppm img)
{
        assert(img != NULL);

        A2Methods_T methods = uarray2_methods_blocked;
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        T RGBFloats = UArray2b_new(img->width, img->height, sizeof(struct pixelInfo), 
                                   PIXEL_BLOCKSIZE);

        struct applyPixelToFloatsClosure closure = {RGBFloats, img, methods};

        map(img->pixels, applyPixelToFloats, &closure);

        return RGBFloats;
}

static void applyPixelToFloats(int col, int row, A2Methods_UArray2 pixels, 
                             void *elem, void *cl) {
                        
        (void) pixels;
        struct applyPixelToFloatsClosure *closure = cl;
        Pnm_rgb pixel = elem;

        unsigned denom = closure->img->denominator;

        float rFloat = (float) pixel->red / denom;
        float gFloat = (float) pixel->green / denom;
        float bFloat = (float) pixel->blue / denom;

        printf("r:%f\ng:%f\nb:%f\n", rFloat, gFloat, bFloat);

        struct pixelInfo *destVals = closure->methods->at(closure->RGBFloats, col, row);
        *destVals = (struct pixelInfo){rFloat, gFloat, bFloat};
}

#undef T
