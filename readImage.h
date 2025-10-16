/*
 *      readImage.h
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      //TODO: description   
 */

//TODO INCLUDES
#include "pnm.h"

//TODO: FUNCTIONS DECLARATIONS

Pnm_ppm readImage(FILE *fp);
Pnm_ppm trimImage(Pnm_ppm img, A2Methods_mapfun *map);
