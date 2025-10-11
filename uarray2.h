/**************************************************************
 *     uarray2.h
 *     Assignment: locality
 *     Authors: Aidan Liwanag (aliwan01) & Kevin (klu07)
 *     Date: 10/7/25
 *
 *     This file contains the interface of UArray2, which will allow a client
 *     to use a 2D array of elements. The client will be able to create the
 *     array by specifying dimensions and element type. The array will have
 *     accessable width, height, and element size, as well as accessable
 *     individual elements. The client will also be able to map over the whole
 *     array in either column- or row-major order, applying a function of their
 *     choosing to each element. The client can free the array when finished.
 **************************************************************/

#ifndef UARRAY2_H
#define UARRAY2_H

typedef struct UArray2 *UArray2_T;

/* Constructor/destructor */
UArray2_T UArray2_new(int width, int height, int size);
void UArray2_free(UArray2_T *uarr2_ptr);

/* Data access */
int UArray2_width(UArray2_T uarr2);
int UArray2_height(UArray2_T uarr2);
int UArray2_size(UArray2_T uarr2);
void *UArray2_at(UArray2_T uarr2, int col, int row);

/* Mapping functions */
void UArray2_map_col_major(UArray2_T uarr2,
    void apply(int col, int row, UArray2_T uarr2, void *elem, void *cl),
    void *cl);
void UArray2_map_row_major(UArray2_T uarr2,
    void apply(int col, int row, UArray2_T uarr2, void *elem, void *cl),
    void *cl);

#endif
