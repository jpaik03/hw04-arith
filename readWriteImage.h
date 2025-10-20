/*
 *      readWriteImage.h
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      Interface for reading, trimming, and writing PPM images. This module
 *      handles the C1 and (C1)' steps which involve reading a PPM from input,
 *      ensuring it has even dimensions, and writing a PPM to standard output.
 */

#include <stdio.h>
#include "pnm.h"

Pnm_ppm readImage(FILE *fp);
void writeImage(Pnm_ppm pixmap);