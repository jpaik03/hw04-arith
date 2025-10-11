/*
 *      ppmdiff.c
 *      Kevin Lu (klu07), Justin Paik (jpaik03)
 *      October 21, 2025
 *      arith
 * 
 *      Implementation of ppmdiff, which calculates a numerical representation
 *      of error between two ppm files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pnm.h"
#include "assert.h"
// TODO other includes??

/******** main ********
 *
 * Driver function to run ppmdiff program. All storage variable pointers and
 * program logic follows within this function. 
 *
 * Parameters:
 *      int argc:       argument count
 *      char *argv[]:   argument array
 * Return: 
 *      0 as main function
 * Expects:
 *      user to compile program
 * Notes:
 *      Throws a CRE if any unexpected behavior is encountered.
 ************************/
int main(int argc, char *argv[])
{
        
        return 0;
}

/******** processImage ********
 *
 * Handles the entire image transformation process including reading,
 * rotating, timing, and writing the output image.
 *
 * Parameters:
 *      int argc:               argument count from command line
 *      char *argv[]:           argument array containing command line args
 *      int i:                  index of filename argument in argv
 *      char *time_file_name:   filename to write timing data to (NULL if none)
 *      int rotation:           rotation angle (0, 90, 180, or 270 degrees)
 *      A2Methods_T methods:    method suite for array operations
 *      A2Methods_mapfun *map:  mapping function to use for transformation
 * Return: 
 *      nothing
 * Expects:
 *      methods is not NULL
 *      map is not NULL
 *      rotation is 0, 90, 180, or 270
 *      if time_file_name is not NULL, it is a valid writable file path
 * Notes:
 *      throws a CRE if methods is NULL
 *      throws a CRE if ppmImage cannot be read
 *      throws a CRE if timer allocation fails when timing is requested
 *      frees all dynamically allocated memory before returning
 ************************/
void processImage(int argc, char *argv[], int i, char *time_file_name,
                  int rotation, A2Methods_T methods, A2Methods_mapfun *map)
{           
        double time_elapsed = 0.0;
        CPUTime_T timer = NULL;

        /* if timing was requested create timer */
        if (time_file_name != NULL) {
                timer = CPUTime_New();
                assert(timer != NULL);
        }

        /* open user provided image */
        Pnm_ppm ppmImage = openReadImage(argc, argv, i, methods);

        /* create a struct holding image info to pass into our mapping func */
        unsigned width;
        unsigned height;
        struct passedInfo helperStruct =
        initRotatedArray(ppmImage, rotation, methods, &width, &height);

        /* check we are in bounds and destination array and current image
           arrays are valid */
        assert(width > 0 && height > 0);
        assert(helperStruct.destArr != NULL && ppmImage->pixels != NULL);  

        /* start timer if timing was requested, map our data structure,
           then stop timer when mapping is complete */
        if (timer != NULL) {
                CPUTime_Start(timer);
        }

        map(ppmImage->pixels, rotateppm, &helperStruct);

        if (timer != NULL) {
                time_elapsed = CPUTime_Stop(timer);
        }

        /* create and fill finalImage struct that holds transformed image */
        struct Pnm_ppm finalImage;
        finalImage.width = width;
        finalImage.height = height;
        finalImage.denominator = ppmImage->denominator;
        finalImage.pixels = helperStruct.destArr;
        finalImage.methods = methods;

        /* write to stdout*/
        Pnm_ppmwrite(stdout, &finalImage);

        /* call helper function to write/append time data info to a file */
        if (timer != NULL) {
                writeTiming(time_file_name, ppmImage->width, 
                            ppmImage->height, time_elapsed, rotation, map, 
                            methods);
                CPUTime_Free(&timer);
        }
        /* free memory */
        Pnm_ppmfree(&ppmImage);
        methods->free(&helperStruct.destArr);
}

/******** openReadImage ********
 *
 * Opens and reads a PPM image file, returning a Pnm_ppm struct containing
 * the image data.
 *
 * Parameters:
 *      int argc:               argument count from command line
 *      char *argv[]:           argument array containing command line args
 *      int i:                  index of filename argument in argv
 *      A2Methods_T methods:    method suite to use for storing pixel data
 * Return: 
 *      a Pnm_ppm struct pointer containing the read image data
 * Expects:
 *      methods is not NULL
 *      if i < argc, argv[i] is a valid readable file path
 *      the file contains valid PPM format data
 * Notes:
 *      throws a CRE if file pointer is NULL
 *      throws a CRE if ppmImage is NULL after reading
 *      throws a CRE if pixel array is NULL
 *      throws a CRE if width or height is not positive
 *      closes file if not reading from stdin
 ************************/
Pnm_ppm openReadImage(int argc, char *argv[], int i, A2Methods_T methods)
{
        FILE *fp = openFile(argc, argv, i);
        assert(fp != NULL);

        /* read in a pnm_ppm image and check its valid */
        Pnm_ppm ppmImage = Pnm_ppmread(fp, methods);
        assert(ppmImage != NULL && ppmImage->pixels != NULL);
        assert(ppmImage->width > 0 && ppmImage->height > 0);

        /* close file if one has been opened */
        if (fp != stdin) {
                fclose(fp);
        }

        return ppmImage;
}
