/*
 *      pixelOperation.h
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      Interface for converting pixel data between RGB and Component Video
 *      Color Space (CVCS).
 */

#include "pnm.h"
#include "a2methods.h"
#include "uarray2b.h"

/******** pixInfo struct ********
 *
 * A public struct holding the floating-point values for a single pixel in the
 * Component Video Color Space.
 *
 * Fields:
 *      float y:  The luminance component.
 *      float pb: The blue-difference chroma component.
 *      float pr: The red-difference chroma component.
 * Notes:
 *      This struct is defined publicly in the header so that the blockOperation
 *      module can directly access its fields to perform calculations like
 *      averaging chroma or applying DCT to luminance.
 ************************/
struct pixInfo
{
        float y, pb, pr;
};

/* Compression */
UArray2b_T getRGBCompVid(Pnm_ppm img, A2Methods_T methods);

/* Decompression */
Pnm_ppm getRGBInts(UArray2b_T RGBFloats, A2Methods_T methods);

float keepInRange(float val, float min, float max);