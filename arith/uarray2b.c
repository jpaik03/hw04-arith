/**************************************************************
 *
 *      uarray2b.c
 *
 *      Assignment: locality
 *      Authors:    Kevin lu (klu07), Aidan Liwanag (aliwan01)
 *      Date:       10/07/25
 *
 *      This file contains the implementation of UArray2b. This 
 *      UArray2b will represent a blocked 2 dimensional unboxed
 *      array by using computation to have mapping done for
 *      all elements in a block before moving on to the next.
 *      This interface will allow for the user to call the 
 *      -block-major flag traversal order at command-line.
 *
 **************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "assert.h"
#include "uarray2b.h"
#include "uarray.h"
#include "uarray2.h"

/**************************************************************
 *
 *      struct UArray2b_T
 *
 *      This struct represents a blocked 2D unboxed array. It 
 *      contains the width, height, element size, block size, 
 *      blocked-width, blocked-height, and a 2D UArray 
 *      to hold the actual data. Structs of this kind will be used for
 *      the pixel data in the ppmtrans program, specifically when
 *      the -block-major flag is used.
 *
 **************************************************************/
struct UArray2b_T {
        int width; 
        int height; 
        int size; 
        int blocksize; 
        int blocked_width;
        int blocked_height; 
        UArray2_T blocks; 
};

/*
 * UArray2b_new
 *
 * Description: This function creates a new UArray2b.
 *
 * Parameters:
 *      int width: indicates the width of the new UArray2b
 *      int height: indicates the height of the new UArray2b
 *      int size: indicates the size of each element in the new UArray2b
 *      int blocksize: indicates the side length of a single block (e.g.
 *                     blocksize of 4 produces blocks that are 4 x 4 slots)
 *
 * Returns: An empty initialized UArray2b struct
 *
 * Expects: 
 *     Non-negative width and height, size and blocksize > 0 (verified by 
 *     assertions)
 *
 * Notes:
 *      Uses Hanson's UArrays
 *      Uses the solution implementation of UArray2
 *      Uses malloc to create, must be freed by the caller
 *      May terminate program if assertion is not met
 *      Caller is responsible for freeing
 */
UArray2b_T UArray2b_new(int width, int height, int size, int blocksize)
{       
        /* Asserting parameters */
        assert(width >= 0);
        assert(height >= 0);
        assert(size > 0);
        assert(blocksize > 0);

        /* Allocating memory for the struct */
        UArray2b_T array2b = malloc(sizeof(*array2b));
        assert(array2b != NULL);

        /* Initializing struct fields */
        array2b->width = width;
        array2b->height = height;
        array2b->size = size;
        array2b->blocksize = blocksize;

        /* Calculating blocked dimensions */
        array2b->blocked_width  = (width  + blocksize - 1) / blocksize;
        array2b->blocked_height = (height + blocksize - 1) / blocksize;

        array2b->blocks = UArray2_new(array2b->blocked_width,
                                        array2b->blocked_height,
                                        sizeof(UArray_T));

        /* Creating new block for each outer UArray2 element*/
        for (int br = 0; br < array2b->blocked_height; br++) {
                for (int bc = 0; bc < array2b->blocked_width; bc++) {
                        UArray_T *block_ptr = UArray2_at(array2b->blocks, bc,
                                                                br);
                        assert(block_ptr != NULL);
                        *block_ptr = UArray_new(blocksize * blocksize, size);
                }
        }

        return array2b;
}

/*
 * UArray2b_new_64K_block
 *
 * Description: This function creates a new UArray2b and automatically chooses
 *              the largest blocksize that will allow a block to fit in 64KB
 *              of RAM.
 *
 * Parameters:
 *      int width: indicates the width of the new UArray2b
 *      int height: indicates the height of the new UArray2b
 *      int size: indicates the size of each element in the new UArray2b
 *
 * Returns: An empty initialized UArray2b struct
 *
 * Expects: Non-negative width and height, size > 0 (verified by assertions)
 *
 * Notes:
 *      May terminate program if assertion is not met
 */
UArray2b_T UArray2b_new_64K_block(int width, int height, int size)
{
        /* Asserting parameters */
        assert(width >= 0);
        assert(height >= 0);
        assert(size > 0);
        
        const int MAX_BYTES = 64 * 1024; /* 64KB */
        int blocksize;

        /* Blocksize is 1 if size is past 64kb */
        if (size >= MAX_BYTES) {
                blocksize = 1;

        /* Blocksize calculation */
        } else {
                blocksize = (int)(sqrt((double)(MAX_BYTES / size)));

                /* Ensuring blocksize is at least 1 */
                if (blocksize < 1) {
                        blocksize = 1;
                }
        }

        return UArray2b_new(width, height, size, blocksize);
}

/*
 * free_block
 *
 * Description: This function frees a single block in a UArray2b
 *
 * Parameters:
 *      int col; unused
 *      int row: unused
 *      UArray2_T blocks: unused
 *      void *elem: points to the block that will be freed
 *      void *cl: unused
 *
 * Returns: N/A
 *
 * Expects: Non-null pointer to an element (verified by assertion)
 *
 * Notes:
 *      May terminate program if assertion is not met
 *      Uses Hanson's UArray_free
 *      Caller relenquishes ownership
 */
void free_block(int col, int row, UArray2_T blocks, void *elem, void *cl)
{
        /* Unused parameters */
        (void) col;
        (void) row;
        (void) blocks;
        (void) cl;

        /* Safety check to make sure out pointer is not null */
        UArray_T *block_ptr = elem;
        assert(block_ptr != NULL);

        /* Freeing the inner UArray */
        if (*block_ptr != NULL) {
                UArray_free(block_ptr);
        }
}

/*
 * UArray2b_free
 *
 * Description: This function frees an entire UArray2b.
 *
 * Parameters:
 *      UArray2b_T *array2b: a pointer to the UArray2b that will be freed
 *
 * Returns: N/A
 *
 * Expects: Non-null pointer to an element (verified by assertion)
 *
 * Notes:
 *      May terminate program if assertion is not met
 *      Uses solution UArray2 implementation
 *      Uses Hanson's UArray_free
 *      Caller relenquishes ownership
 */
void UArray2b_free(UArray2b_T *array2b)
{
        assert(array2b != NULL);
        assert(*array2b != NULL);

        /* Freeing all the blocks */
        UArray2_map_row_major((*array2b)->blocks, free_block, NULL);

        /* Freeing the outer UArray2 */
        UArray2_free(&((*array2b)->blocks));

        /* Freeing the struct and null the pointer */
        free(*array2b);
        *array2b = NULL;
}

/*
 * UArray2b_width
 *
 * Description: This function returns the width of a UArray2b.
 *
 * Parameters:
 *      UArray2_T array2b: A UArray2b struct whose members will be accessed
 *
 * Returns: An int representing the width of a UArray2b
 *
 * Expects: A non-NULL UArray2b (verified by assertion)
 *
 * Notes: May terminate program if assertion is not met
 */
int UArray2b_width (UArray2b_T array2b)
{
        assert(array2b != NULL);
        return array2b->width;
}

/*
 * UArray2b_height
 *
 * Description: This function returns the height of a UArray2b.
 *
 * Parameters:
 *      UArray2_T array2b: A UArray2b struct whose members will be accessed
 *
 * Returns: An int representing the height of a UArray2b
 *
 * Expects: A non-NULL UArray2b (verified by assertion)
 *
 * Notes: May terminate program if assertion is not met
 */
int UArray2b_height (UArray2b_T array2b)
{
        assert(array2b != NULL);
        return array2b->height;
}

/*
 * UArray2b_size
 *
 * Description: This function returns the size of an element in a UArray2b.
 *
 * Parameters:
 *      UArray2_T array2b: A UArray2b struct whose members will be accessed
 *
 * Returns: An int representing the size of an element in a UArray2b
 *
 * Expects: A non-NULL UArray2b (verified by assertion)
 *
 * Notes: May terminate program if assertion is not met
 */
int UArray2b_size (UArray2b_T array2b)
{
        assert(array2b != NULL);
        return array2b->size;;
}

/*
 * UArray2b_blocksize
 *
 * Description: This function returns the size of a block in a UArray2b.
 *
 * Parameters:
 *      UArray2_T array2b: A UArray2b struct whose members will be accessed
 *
 * Returns: An int representing the blocksize of a UArray2b
 *
 * Expects: A non-NULL UArray2b (verified by assertion)
 *
 * Notes: May terminate program if assertion is not met
 */
int UArray2b_blocksize(UArray2b_T array2b)
{
        assert(array2b != NULL);
        return array2b->blocksize;;
}

/*
 * UArray2b_at
 *
 * Description: This function returns an element at a given index in a UArray2b.
 *
 * Parameters:
 *      UArray2_T array2b: A UArray2b containing the element to be accessed
 *      int column: the column in which the element is located
 *      int row: the row in which the element is located
 *
 * Returns: A pointer to the element at the specified index
 *
 * Expects: A non-NULL UArray2b and an in-range index (verified by assertions)
 *
 * Notes: May terminate program if assertion is not met
 *        Uses Hanson's UArray and the UArray2b solution
 */
void *UArray2b_at(UArray2b_T array2b, int column, int row)
{
        /* Asserting parameters */
        assert(array2b != NULL);
        assert(column >= 0 && column < array2b->width);
        assert(row >= 0 && row < array2b->height);

        /* Finding the block containing the element */
        int blocksize = array2b->blocksize;
        int block_col = column / blocksize;
        int block_row = row / blocksize;

        /* Find index within block */
        int col_in_block = column % blocksize;
        int row_in_block = row % blocksize;

        /* Compute the index in the block (from spec) */ 
        int block_index = row_in_block * blocksize + col_in_block;

        /* Safety check to make sure out pointer is not null */
        UArray_T *block_ptr = UArray2_at(array2b->blocks,
                                         block_col,
                                         block_row);
        assert(block_ptr != NULL);   

        return UArray_at(*block_ptr, block_index);
}

/* This struct serves as the closure parameter for UArray2b_map */
struct BlockMapClosure {
        UArray2b_T array2b;
        void (*apply)(int i, int j, UArray2b_T a, void *elem, void *cl);
        void *cl;
};

/*
 * map_one_block
 *
 * Description: This function maps over a single block in a UArray2b.
 *
 * Parameters:
 *      int block_col: the column of the block of interest
 *      int block_row: the row of the block of interest
 *      UArray2_T blocks: unused
 *      void *block_ptr: a pointer to the block to be mapped over
 *      void *vcl: the closure parameter for this apply function - in this case,
 *                 a pointer to a BlockMapClosure struct
 *
 * Returns: N/A
 *
 * Expects: A non-NULL block_ptr (verified by assertion)
 *
 * Notes: May terminate program if assertion is not met
 *        Uses Hanson's UArray
 */
static void map_one_block(int block_col, int block_row,
                          UArray2_T blocks, void *block_ptr, void *vcl)
{
        /* Unused parameters */
        (void)blocks; 
        
        struct BlockMapClosure *closure = vcl;
        UArray2b_T a2b = closure->array2b;
        int blocksize = a2b->blocksize;

        assert(block_ptr != NULL);

        UArray_T block = *(UArray_T *)block_ptr;
        assert(block != NULL);

        int num_cells = blocksize * blocksize;

        /* Mapping over each cell in the block */
        for (int idx = 0; idx < num_cells; idx++) {
                int row_in_block = idx / blocksize;
                int col_in_block = idx % blocksize;

                int global_col = block_col * blocksize + col_in_block;
                int global_row = block_row * blocksize + row_in_block;

                /* Skip unused cells in edge blocks */ 
                if (global_col < a2b->width && global_row < a2b->height) {
                        void *elem_in_block = UArray_at(block, idx);
                        closure->apply(global_col, global_row, a2b, 
                                        elem_in_block, closure->cl);
                }
        }
}

/*
 * UArray2b_map
 *
 * Description: This function maps over whole UArray2b, block by block in row-
 *              major order.
 *
 * Parameters:
 *      UArray2b_T array2b: the UArray2b to be mapped over
 *      void apply: a caller-specified funciton to be applied to each element
 *      void *cl: a caller-specified closure parameter
 *
 * Returns: N/A
 *
 * Expects: A non-NULL block_ptr (verified by assertion)
 *
 * Notes: May terminate program if assertion is not met
 *        Uses Hanson's UArray
 */
void UArray2b_map(UArray2b_T array2b,
                  void apply(int i, int j, UArray2b_T a, void *elem, void *cl),
                  void *cl)
{
        assert(array2b != NULL);
        assert(apply != NULL);

        struct BlockMapClosure closure = { array2b, apply, cl };
        UArray2_map_row_major(array2b->blocks, map_one_block, &closure);
}
