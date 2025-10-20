/*
 *      pixelOperation.c
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

#include "pixelOperation.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "a2methods.h"

// TODO: ASK IF THIS WE SHOULD DO THIS AND WHY THERE ISNT UNDEF
#define T UArray2b_T


/* Const vals for clarity and reuse */
const int PIXEL_BLOCKSIZE = 2;
const unsigned DENOMINATOR = 255;

static void applyPixelToCompVid(int col, int row, A2Methods_UArray2 pixels, 
                             void *elem, void *cl);
static void applyCompVidToPixel(int col, int row, A2Methods_UArray2 pixels, 
                             void *elem, void *cl);

struct pixInfo
{
        float y, pb, pr;
};

struct applyPixelToCompVidClosure
{
        T RGBInfo;
        Pnm_ppm img;
        A2Methods_T methods;
};

T getRGBCompVid(Pnm_ppm img, A2Methods_T methods)
{
        assert(img != NULL);
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);
        
        T RGBInfo = methods->new_with_blocksize(img->width, img->height, 
                                                sizeof(struct pixInfo), 
                                                PIXEL_BLOCKSIZE);

        struct applyPixelToCompVidClosure closure = {RGBInfo, img, methods};

        map(img->pixels, applyPixelToCompVid, &closure);

        return RGBInfo;
}

static void applyPixelToCompVid(int col, int row, A2Methods_UArray2 pixels, 
                             void *elem, void *cl) {
                        
        (void)pixels;
        struct applyPixelToCompVidClosure *closure = cl;
        Pnm_rgb pixel = elem;

        unsigned denom = closure->img->denominator;

        /* Scaled Ints to floats */
        float r = (float) pixel->red / denom;
        float g = (float) pixel->green / denom;
        float b = (float) pixel->blue / denom;

        /* RGB floats to CVCS */
        float y = 0.299 * r + 0.587 * g + 0.114 * b;
        float pb = 0.5 * b - 0.168736 * r - 0.331264 * g;
        float pr = 0.5 * r - 0.418688 * g - 0.081312 * b;

        struct pixInfo *destVals = closure->methods->at(closure->RGBInfo, 
                                                          col, row);
        *destVals = (struct pixInfo){y, pb, pr};
}

struct applyCompVidToPixelClosure
{
        Pnm_ppm pixmap;
        T RGBInfo;
        A2Methods_T methods;
};

Pnm_ppm getRGBInts(T RGBInfo, A2Methods_T methods)
{
        assert(RGBInfo != NULL);
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        Pnm_ppm pixmap = malloc(sizeof(*pixmap));
        assert(pixmap);

        pixmap->width = methods->width(RGBInfo);
        pixmap->height = methods->height(RGBInfo);
        pixmap->denominator = DENOMINATOR;
        pixmap->methods = methods;
        pixmap->pixels = methods->new(pixmap->width, pixmap->height, 
                                      sizeof(struct Pnm_rgb));

        struct applyCompVidToPixelClosure closure = {pixmap, RGBInfo, methods};
        map(pixmap->pixels, applyCompVidToPixel, &closure);
        
        return pixmap;
}

static void applyCompVidToPixel(int col, int row, A2Methods_UArray2 pixels, 
                             void *elem, void *cl) {
                        
        (void) pixels;
        struct applyCompVidToPixelClosure *closure = cl; 
        Pnm_rgb destPixel = elem;

        struct pixInfo *srcVals = closure->methods->at(closure->RGBInfo,
                                                       col, row);
        
        float y = srcVals->y;
        float pb = srcVals->pb;
        float pr = srcVals->pr;

        /* CVCS to RGB floats */
        float r = 1.0 * y + 0.0 * pb + 1.402 * pr;
        float g = 1.0 * y - 0.344136 * pb - 0.714136 * pr;
        float b = 1.0 * y + 1.772 * pb + 0. * pr;

        /* Keeping rgb values in the range of 0.0 to 0.1 */
        r = keepInRange(r, 0.0, 1.0);
        g = keepInRange(g, 0.0, 1.0);
        b = keepInRange(b, 0.0, 1.0);

        /* RGB floats to RGB scaled ints */
        destPixel->red   = (int) round(r * DENOMINATOR);
        destPixel->green = (int) round(g * DENOMINATOR);
        destPixel->blue  = (int) round(b * DENOMINATOR);
}

float keepInRange(float val, float min, float max) {
    if (val < min) {
        return min;
    }
    if (val > max) { 
        return max;
    }
    return val;
}

float getPb(pixInfo p) {
        assert(p != NULL);
        return p->pb;
}

float getPr(pixInfo p) {
        assert(p != NULL);
        return p->pr;
}

float getY(pixInfo p) {
        assert(p != NULL);
        return p->y;
}

void setPb(pixInfo p, float pb) {
        assert(p != NULL);
        p->pb = pb;
}

void setPr(pixInfo p, float pr) {
        assert(p != NULL);
        p->pr = pr;
}

void setY(pixInfo p, float y) {
        assert(p != NULL);
        p->y = y;
}

size_t getpixInfoSize() {
        return sizeof(struct pixInfo);
}

#undef T
