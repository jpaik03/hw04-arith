/*
 *      blockOperation.c
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
#include "pixelOperation.h"
#include "arith40.h"

const int PIX_BLOCKSIZE = 2;

static void applyCompVidToDCT(int col, int row, A2Methods_UArray2 pixels, 
                              void *elem, void *cl);

static void applyDCTToPixel(int col, int row, A2Methods_UArray2 pixels, 
                            void *elem, void *cl);

static void applyQuantize(int col, int row, A2Methods_UArray2 pixels,
                          void *elem, void *cl);

static int quantizeBCD(float coefficient);

static void applyDequantize(int col, int row, A2Methods_UArray2 pixels, 
                            void *elem, void *cl);
                            
static float dequantizeBCD(int quantizedCoeff);

//TODO: STRUCT CONTRACT AND UNCOMMENT
struct DCTVals {
        float a, b, c, d, bpb, bpr;
};

struct quantized {
        unsigned a, indexbpb, indexbpr;
        int b, c, d;
};

struct applyCompVidToDCTClosure {
        UArray2_T DCTSpace;
        A2Methods_T bMethods;
        A2Methods_T pMethods;
        UArray2b_T RGBCompVid;
};

struct applyDCTToPixelClosure {
        UArray2b_T RGBFloats;
        A2Methods_T pMethods;
        A2Methods_T bMethods;
        UArray2_T DCTSpace;
};

struct applyQuantizeClosure {
        UArray2_T quantInts;
        A2Methods_T methods;
        UArray2_T DCTSpace;
};

struct applyDequantizeClosure {
        UArray2_T dequantFloats;
        A2Methods_T methods;
        UArray2_T DCTSpace;
};

//TODO: FUNC CONTRACT
UArray2_T pixelsToDCTBlock(UArray2b_T RGBCompVid, A2Methods_T bMethods,
                           A2Methods_T pMethods) {
        assert(RGBCompVid != NULL);
        assert(pMethods != NULL);
        assert(bMethods != NULL);

        A2Methods_mapfun *map = pMethods->map_default;
        assert(map != NULL);

        assert(bMethods->width(RGBCompVid) % 2 == 0);
        assert(bMethods->height(RGBCompVid) % 2 == 0);

        int blockedWidth = bMethods->width(RGBCompVid) / 2;
        int blockedHeight = bMethods->height(RGBCompVid) / 2;

        UArray2_T DCTSpace = pMethods->new(blockedWidth,
                                               blockedHeight,
                                               sizeof(struct DCTVals));

        struct applyCompVidToDCTClosure closure = {DCTSpace, bMethods, 
                                                   pMethods, RGBCompVid};

        map(DCTSpace, applyCompVidToDCT, &closure);

        return DCTSpace;
}

//TODO: FUNC CONTRACT
static void applyCompVidToDCT(int col, int row, A2Methods_UArray2 pixels,
                              void *elem, void *cl)
{
        (void) pixels;
        (void) elem;

        struct applyCompVidToDCTClosure *closure = cl;

        struct pixInfo *pix1 = closure->bMethods->at(closure->RGBCompVid, 
                                                     col * 2, row * 2);
        struct pixInfo *pix2 = closure->bMethods->at(closure->RGBCompVid, 
                                                     col * 2 + 1, row * 2);
        struct pixInfo *pix3 = closure->bMethods->at(closure->RGBCompVid,
                                                     col * 2, row * 2 + 1);
        struct pixInfo *pix4 = closure->bMethods->at(closure->RGBCompVid,    
                                                     col * 2 + 1, row * 2 + 1);

        float pb_bar = (getPb(pix1) + getPb(pix2) +
                        getPb(pix3) + getPb(pix4)) / 4.0;
        float pr_bar = (getPr(pix1) + getPr(pix2) +
                        getPr(pix3) + getPr(pix4)) / 4.0;

        float a, b, c, d;
        
        a = (getY(pix4) + getY(pix3) + getY(pix2) + getY(pix1)) / 4.0;
        b = (getY(pix4) + getY(pix3) - getY(pix2) - getY(pix1)) / 4.0;
        c = (getY(pix4) - getY(pix3) + getY(pix2) - getY(pix1)) / 4.0;
        d = (getY(pix4) - getY(pix3) - getY(pix2) + getY(pix1)) / 4.0;

        struct DCTVals *destDCT = closure->pMethods->at(closure->DCTSpace,
                                                              col, row);
        *destDCT = (struct DCTVals){a, b, c, d, pb_bar, pr_bar};
}

UArray2_T quantizeValues(UArray2_T DCTSpace, A2Methods_T methods) {

        assert(DCTSpace != NULL);
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        UArray2_T quantInts = methods->new(methods->width(DCTSpace), 
                                               methods->height(DCTSpace), 
                                               sizeof(struct quantized));

        struct applyQuantizeClosure closure = {quantInts, methods,
                                               DCTSpace};

        map(DCTSpace, applyQuantize, &closure);
        
        return quantInts;
        
}

static void applyQuantize(int col, int row, A2Methods_UArray2 pixels, 
                          void *elem, void *cl) {
        
        (void) pixels;
        (void) elem;
        
        struct applyQuantizeClosure *closure = cl;
        struct DCTVals *srcDCT = closure->methods->at(closure->DCTSpace,
                                                      col, row);

        unsigned a = (unsigned) round(srcDCT->a * 511);
        int b = quantizeBCD(srcDCT->b);
        int c = quantizeBCD(srcDCT->c);
        int d = quantizeBCD(srcDCT->d);

        unsigned indexbpb = Arith40_index_of_chroma(srcDCT->bpb);
        unsigned indexbpr = Arith40_index_of_chroma(srcDCT->bpr);

        struct quantized *destQuant = closure->methods->at(closure->quantInts,
                                                          col, row);

        *destQuant = (struct quantized){a, indexbpb, indexbpr, b, c, d};
}

static int quantizeBCD(float coefficient) {
        int quantizedCoeff = keepInRange((int) round(coefficient * 50),
                                         -15, 15);
        return quantizedCoeff;
}

UArray2_T dequantizeValues(UArray2_T DCTSpace, A2Methods_T methods) {
        
        assert(DCTSpace != NULL);
        assert(methods != NULL);

        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        UArray2_T dequantFloats = methods->new(methods->width(DCTSpace), 
                                                   methods->height(DCTSpace), 
                                                   sizeof(struct DCTVals));

        struct applyDequantizeClosure closure = {dequantFloats, methods,
                                                 DCTSpace};
                                                        
        map(DCTSpace, applyDequantize, &closure);

        return dequantFloats;
        
}

static void applyDequantize(int col, int row, A2Methods_UArray2 pixels, 
                            void *elem, void *cl) {
        
        (void) pixels;
        (void) elem;
        
        struct applyDequantizeClosure *closure = cl;
        struct quantized *srcQuant = closure->methods->at(closure->DCTSpace,
                                                      col, row);

        float a = keepInRange(srcQuant->a / 511.0, 0, 1);
        float b = dequantizeBCD(srcQuant->b);
        float c = dequantizeBCD(srcQuant->c);
        float d = dequantizeBCD(srcQuant->d);

        float pb_bar = Arith40_chroma_of_index(srcQuant->indexbpb);
        float pr_bar = Arith40_chroma_of_index(srcQuant->indexbpr);

        struct DCTVals *destDCT = closure->methods->at(closure->dequantFloats,
                                                          col, row);

        *destDCT = (struct DCTVals){a, b, c, d, pb_bar, pr_bar};
}

static float dequantizeBCD(int quantizedCoeff) {
        float coefficient = quantizedCoeff / 50.0;
        return coefficient;
}

UArray2b_T DCTBlockToPixels(UArray2_T DCTSpace, A2Methods_T pMethods,
                            A2Methods_T bMethods) {
        assert(DCTSpace != NULL);
        assert(pMethods != NULL);
        assert(bMethods != NULL);

        A2Methods_mapfun *map = bMethods->map_default;
        assert(map != NULL);

        int pixelWidth = pMethods->width(DCTSpace) * PIX_BLOCKSIZE;
        int pixelHeight = pMethods->height(DCTSpace) * PIX_BLOCKSIZE;

        UArray2b_T RGBFloats = bMethods->new_with_blocksize(pixelWidth,
                                                           pixelHeight,
                                                           getpixInfoSize(),
                                                           PIX_BLOCKSIZE);

        struct applyDCTToPixelClosure closure = {RGBFloats, pMethods, bMethods, 
                                                 DCTSpace};

        map(RGBFloats, applyDCTToPixel, &closure);

        return RGBFloats;
}

static void applyDCTToPixel(int col, int row, A2Methods_UArray2 pixels,
                            void *elem, void *cl) {

        (void) pixels;
        (void) elem;
        
        struct applyDCTToPixelClosure *closure = cl;

        struct DCTVals *srcDCT = closure->pMethods->at(closure->DCTSpace,
                                                      col / 2, row / 2);

        float a = srcDCT->a;
        float b = srcDCT->b;
        float c = srcDCT->c;
        float d = srcDCT->d;
        float pb_bar = srcDCT->bpb;
        float pr_bar = srcDCT->bpr;

        float y;
        if (col % 2 == 0 && row % 2 == 0) {
                y = a - b - c + d;
        } else if (col % 2 == 1 && row % 2 == 0) {
                y = a - b + c - d;
        } else if (col % 2 == 0 && row % 2 == 1) {
                y = a + b - c - d;
        } else {
                y = a + b + c + d;
        }

        struct pixInfo *destPix = closure->bMethods->at(closure->RGBFloats,
                                                        col, row);

        setY(destPix, y);
        setPb(destPix, pb_bar);
        setPr(destPix, pr_bar);
}