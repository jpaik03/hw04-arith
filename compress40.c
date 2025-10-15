#include <stdio.h>
#include <stdlib.h>
#include "compress40.h"
#include "readImage.h"

/* reads PPM, writes compressed image */
extern void compress40(FILE *input) {
        Pnm_ppm img = readImage(input);

        // TODO: DELETE
        // Pnm_ppmwrite(stdout, img);

        


        img->methods->free(&(img->pixels));
        free(img);

} 

extern void decompress40(FILE *input) {
        (void)input;
}