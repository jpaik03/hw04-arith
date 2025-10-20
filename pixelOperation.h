/*
 *      pixelOperation.h
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      // TODO: description
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "uarray2b.h"
#include "pnm.h"

typedef struct pixInfo *pixInfo;

/* Compression */
UArray2b_T getRGBCompVid(Pnm_ppm img, A2Methods_T methods);

/* Accessors and modifiers for blocked operation module */
float getPb(pixInfo p);
float getPr(pixInfo p);
float getY(pixInfo p);
size_t getpixInfoSize();

void setPb(pixInfo p, float pb);
void setPr(pixInfo p, float pr);
void setY(pixInfo p, float y);

/* Decompression */
Pnm_ppm getRGBInts(UArray2b_T RGBFloats, A2Methods_T methods);

float keepInRange(float val, float min, float max);