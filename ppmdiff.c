#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "assert.h"

#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"

FILE *openFile(const char *filename);

int main(int argc, char *argv[])
{
        /* Validate usage */
        if (argc != 3) {
                fprintf(stderr, "Usage: ./ppmdiff <file1> <file2>\n");
                exit(EXIT_FAILURE);
        }

        /* Read in images into 2D arrays */
        A2Methods_T methods = uarray2_methods_plain;
        assert(methods);
        FILE *fp1 = openFile(argv[1]);
        FILE *fp2 = openFile(argv[2]);
        Pnm_ppm img1 = Pnm_ppmread(fp1, methods);
        Pnm_ppm img2 = Pnm_ppmread(fp2, methods);

        /* If image dimensions differ by more than 1, fail */
        if (abs((int)img1->width - (int)img2->width) > 1 || 
            abs((int)img1->height - (int)img2->height) > 1) {
                fprintf(stderr, "Error: Images differ by more than 1 pixel.\n");
                printf("1.0\n");

                Pnm_ppmfree(&img1);
                Pnm_ppmfree(&img2);
                fclose(fp1);
                fclose(fp2);

                exit(EXIT_FAILURE);
        }

        /* Find smaller image */
        unsigned w;
        if (img1->width < img2->width) {
                w = img1->width;
        } else {
                w = img2->width;
        }
        unsigned h;
        if (img1->height < img2->height) {
                h = img1->height;
        } else {
                h = img2->height;
        }

        /* Divide by zero case */
        if (w == 0 || h == 0) {
                printf("0.0000\n");
        } else {
                double sum_sq_diff = 0.0;
                double denom1 = (double)img1->denominator;
                double denom2 = (double)img2->denominator;

                /* Iterate through smallest dimensions */
                for (unsigned j = 0; j < h; j++) {
                        for (unsigned i = 0; i < w; i++) {
                                Pnm_rgb p1 = (Pnm_rgb)methods->at(img1->pixels, i, j);
                                Pnm_rgb p2 = (Pnm_rgb)methods->at(img2->pixels, i, j);

                                /* Convert scaled integers to floating point values */
                                double r1 = (double)p1->red   / denom1;
                                double g1 = (double)p1->green / denom1;
                                double b1 = (double)p1->blue  / denom1;

                                double r2 = (double)p2->red   / denom2;
                                double g2 = (double)p2->green / denom2;
                                double b2 = (double)p2->blue  / denom2;
                                
                                /* Add the squared differences of each color channel to the sum */
                                sum_sq_diff += pow(r1 - r2, 2);
                                sum_sq_diff += pow(g1 - g2, 2);
                                sum_sq_diff += pow(b1 - b2, 2);
                        }
                }

                /* Apply formula for the root mean square difference */
                double e = sqrt(sum_sq_diff / (3.0 * w * h));
                printf("%.4f\n", e);
        }

        Pnm_ppmfree(&img1);
        Pnm_ppmfree(&img2);
        fclose(fp1);
        fclose(fp2);

        return 0;
}

FILE *openFile(const char *filename)
{
        FILE *fp = fopen(filename, "rb");
        if (fp == NULL) {
                fprintf(stderr, "Could not open file: %s\n", filename);
                exit(EXIT_FAILURE);
        }
        return fp;
}