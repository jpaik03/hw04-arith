/**************************************************************
 *
 *      a2plain.c
 *
 *      Assignment: locality
 *      Authors:    Kevin lu (klu07), Aidan Liwanag (aliwan01)
 *      Date:       10/07/25
 *
 *      This file contains the implementation of the a2plain methods suite.
 *      This acts as the implemenation of the a2methods interface, using
 *      UArray2 as the underlying data structure. This will allow the client
 *      to call the -row-major and -col-major flags at command-line.
 *
 **************************************************************/

#include <string.h>
#include <a2plain.h>
#include <stdio.h>
#include "uarray2.h"

/*
 * new
 *
 * Description: This function creates a new UArray2.
 *
 * Parameters:
 *      int width: indicates the width of the new UArray2
 *      int height: indicates the height of the new UArray2
 *      int size: indicates the size of each element in the new UArray2
 *
 * Returns: An initialized UArray2
 *
 * Expects: N/A
 *
 * Notes: Uses UArray2
 */
static A2Methods_UArray2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

/*
 * new_with_blocksize
 *
 * Description: This function creates a new UArray2 with specified blocksize.
 *
 * Parameters:
 *      int width: indicates the width of the new UArray2
 *      int height: indicates the height of the new UArray2
 *      int size: indicates the size of each element in the new UArray2
 *      int blocksize: indicates the side length of a single block in the new
 *                     UArray2
 *
 * Returns: An initialized UArray2
 *
 * Expects: N/A
 *
 * Notes: Uses UArray2
 */
static A2Methods_UArray2 new_with_blocksize(int width, int height, int size,
                                            int blocksize)
{
        (void)blocksize;
        return UArray2_new(width, height, size);
}

/*
 * a2_free
 *
 * Description: This function frees a UArray2.
 *
 * Parameters:
 *      A2Methods_UArray2 *a2p: a pointer to a UArray2 to be freed
 *
 * Returns: N/A
 *
 * Expects: N/A
 *
 * Notes: Uses UArray2
 */
static void a2free(A2Methods_UArray2 *a2p) {
        UArray2_free((UArray2_T *)a2p);
}

/*
 * width
 *
 * Description: This function returns the width of a UArray2.
 *
 * Parameters:
 *      A2Methods_UArray2 a2: a UArray2 whose data will be accessed
 *
 * Returns: An int representing the width of a UArray2
 *
 * Expects: N/A
 *
 * Notes: Uses UArray2
 */
static int width(A2Methods_UArray2 a2) {
        return UArray2_width(a2);
}

/*
 * height
 *
 * Description: This function returns the height of a UArray2.
 *
 * Parameters:
 *      A2Methods_UArray2 a2: a UArray2 whose data will be accessed
 *
 * Returns: An int representing the height of a UArray2
 *
 * Expects: N/A
 *
 * Notes: Uses UArray2
 */
static int height(A2Methods_UArray2 a2) {
        return UArray2_height(a2);
}

/*
 * size
 *
 * Description: This function returns the size of an element in a UArray2.
 *
 * Parameters:
 *      A2Methods_UArray2 a2: a UArray2 whose data will be accessed
 *
 * Returns: An int representing the size of an element in a UArray2
 *
 * Expects: N/A
 *
 * Notes: Uses UArray2
 */
static int size(A2Methods_UArray2 a2) {
        return UArray2_size(a2);
}

/*
 * blocksize
 *
 * Description: This function returns the size of a block in a UArray2.
 *
 * Parameters:
 *      A2Methods_UArray2 a2: a UArray2 whose data will be accessed
 *
 * Returns: An int representing the blocksize of a UArray2
 *
 * Expects: N/A
 *
 * Notes:
 *      Uses UArray2
 *      Because plain UArray2s do not utilize blocking, each element exists
 *      independently and has a theoretical blocksize of 1
 */
static int blocksize(A2Methods_UArray2 a2) {

        // plain arrays have blocksize 1
        (void)a2;
        return 1; 
}

/*
 * at
 *
 * Description: This function retrieves an element at a given index in a
 *              UArray2.
 *
 * Parameters:
 *      A2Methods_UArray2 a2: a UArray2 whose data will be accessed
 *      int i: the column in which the elemenet is located
 *      int j: the row in which the element is located
 *
 * Returns: A pointer to the elemenet at the specified index
 *
 * Expects: N/A
 *
 * Notes: Uses UArray2
 */
static A2Methods_Object *at(A2Methods_UArray2 a2, int i, int j) {
        return UArray2_at(a2, i, j);
}

/* Defines the UArray2 apply funciton */
typedef void UArray2_applyfun(int i, int j, UArray2_T array2, void *elem,
                              void *cl);

/*
 * map_row_major
 *
 * Description: This function maps over whole UArray2 in row-major order.
 *
 * Parameters:
 *      A2Methods_UArray2 uarray2: the array that will be mapped over
 *      A2Methods_applyfun apply: a caller-specfied function to be applied to
 *                                each element
 *      void *cl: a closure parameter to be specified by the caller
 *
 * Returns: N/A
 *
 * Expects: N/A
 *
 * Notes: Uses UArray2
 */
static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}

/*
 * map_col_major
 *
 * Description: This function maps over whole UArray2 in column-major order.
 *
 * Parameters:
 *      A2Methods_UArray2 uarray2: the array that will be mapped over
 *      A2Methods_applyfun apply: a caller-specfied function to be applied to
 *                                each element
 *      void *cl: a closure parameter to be specified by the caller
 *
 * Returns: N/A
 *
 * Expects: N/A
 *
 * Notes: Uses UArray2
 */
static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

/*
 * map_default
 *
 * Description: This function maps over whole UArray2 in an default major order.
 *
 * Parameters:
 *      A2Methods_UArray2 uarray2: the array that will be mapped over
 *      A2Methods_applyfun apply: a caller-specfied function to be applied to
 *                                each element
 *      void *cl: a closure parameter to be specified by the caller
 *
 * Returns: N/A
 *
 * Expects: N/A
 *
 * Notes: In this implementation, row-major is the default mapping order
 */
static void map_default(A2Methods_UArray2 a2,
                        A2Methods_applyfun apply, void *cl) {
        map_row_major(a2, apply, cl);
}

/* Stores the smaller apply function and closure */
struct small_closure {
        A2Methods_smallapplyfun *apply; 
        void                    *cl;
};

/* A simpler apply function that ignores indicies */
static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        struct small_closure *cl = vcl;
        (void)i;
        (void)j;
        (void)uarray2;
        cl->apply(elem, cl->cl);
}

/*
 * small_map_row_major
 *
 * Description: This function maps over whole UArray2 in row-major order and
 *              applies a simpler apply function without the need of knowing 
 *              the indicies of each element.
 *
 * Parameters:
 *      A2Methods_UArray2 a2: the array that will be mapped over
 *      A2Methods_smallapplyfun apply: a caller-specified small apply function
 *      void *cl: a caller-specified closure parameter
 *
 * Returns: N/A
 *
 * Expects: N/A
 *
 * Notes: Uses UArray2
 */
static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

/*
 * small_map_col_major
 *
 * Description: This function maps over whole UArray2 in column-major order and
 *              applies a simpler apply function without the need of knowing
 *              the indicies of each element.
 *
 * Parameters:
 *      A2Methods_UArray2 a2: the array that will be mapped over
 *      A2Methods_smallapplyfun apply: a caller-specified small apply function
 *      void *cl: a caller-specified closure parameter
 *
 * Returns: N/A
 *
 * Expects: N/A
 *
 * Notes: Uses UArray2
 */
static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}

/* This struct contains the functions available in the A2Methods plain suite */
static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,
        at,
        map_row_major,
        map_col_major,
        NULL,             // map_block_major 
        map_default,
        small_map_row_major,
        small_map_col_major,
        NULL,             // small_map_block_major 
        small_map_row_major
};

// finally the payoff: here is the exported pointer to the struct

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
