/* Tcl Wavelet Laboratory
 * Memory allocation routines for WL.  Not all of the code uses
 *    these.
 *
 * Fausto Espinal, 2 June 1997
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wlcore.h"

/* WL_Alloc2Dmem    
 *
 * Allocates contiguous memory for a 2-D array.    
 *                                                                        
 */

void **WL_Alloc2Dmem(int r, int c, int size_elem)
{
    int  i;
    void *data;
    void *dataPtr;
    void **ptr;
    unsigned int offsetData;

    offsetData = (sizeof(void *) * r);
    dataPtr= (void *) malloc(offsetData);
    data   = (void *) malloc((size_elem*r*c));
    if (!data) {
        printf("malloc failed in WL_Alloc2Dmem [%d %d %d] (Aborting ) ...\n",
               r,c,size_elem);
        exit(-1);
    }
    ptr = (void **) dataPtr;

    for(i = 0; i < r; i++)
        ptr[i] = (void *) (((long) data) + (i*(size_elem*c)));

    return ((void ** ) dataPtr);
}

/* WL_Calloc2Dmem    
 *
 * Allocates contiguous memory for a 2-D array just like calloc.    
 */
void **WL_Calloc2Dmem(int r, int c, int size_elem)
{
    int  i;
    void *data;
    void *dataPtr;
    void **ptr;

    int voidPtrSize = sizeof(void *);
    dataPtr = (void *) calloc(r, voidPtrSize);
    data = (void *) calloc(r * c, size_elem);
    
    if (!data || !dataPtr) {
        printf("calloc failed in WL_Calloc2Dmem [%d %d %d] (Aborting ) ...\n",
               r,c,size_elem);
        exit(-1);
    }

    ptr = (void **) dataPtr;

    for (i = 0; i < r; i++)
        ptr[i] = (void *) ((long)data + (i * (size_elem * c)));
    
    return ((void **) dataPtr);
}

/* WL_Free2Dmem    
 *
 * Frees memory for a 2-D array that was allocated using
 * WL_Alloc2Dmem ......    
 */
void WL_Free2Dmem(void *memarg)
{
    void **datamem = (void **) memarg;

    if (memarg == NULL) return;
    free(datamem[0]);
    free(memarg);
    return;
}

/* WL_Alloc3Dmem    
 *
 * Allocates contiguous memory for a 3-D array.    
 */
void ***WL_Alloc3Dmem(int d1, int d2, int d3, int size_elem)
{
    void *ptrData;
    void *ptrTable;
    void *rowPtr;
    int  size_page;
    int  i,j;
    
    ptrData  = (void *) malloc((size_elem*d1*d2*d3));
    ptrTable = (void *) malloc(sizeof(void *)*(d1*d2 + d1));

    if (!ptrTable || !ptrData) {
        printf("malloc failed in WL_Alloc3Dmem (Aborting ) .....\n");
        exit(-1);
    }

    size_page = size_elem * d2*d3;
    rowPtr = (void *) ((long) ptrTable + (sizeof(void *)*d1));
    for (i=0;i<d1;i++) {
        void **ptr = (void **) ptrTable;
        void **rPtr;
        ptr[i] = (void *) ((long) rowPtr + (i*d2*sizeof(void *)));
        rPtr = (void **) ptr[i];
        for (j=0;j<d2;j++) {
            rPtr[j] = (void *) ((long) ptrData + 
                              i * size_page + j * (size_elem * d3));
        }
    }
    return ((void ***) ptrTable);
}


/* WL_Calloc3Dmem    
 *
 * Allocates contiguous memory for a 3-D array.    
 */
void ***WL_Calloc3Dmem(int d1, int d2, int d3, int size_elem)
{
    void *ptrData;
    void *ptrTable;
    void *rowPtr;
    int  size_page;
    int  i,j;
    
    ptrData  = (void *) calloc(d1*d2*d3,(size_elem));
    ptrTable = (void *) calloc((d1*d2 + d1),sizeof(void *));

    if (!ptrTable || !ptrData) {
        printf("calloc failed in WL_Calloc3Dmem (Aborting ) .....\n");
        exit(-1);
    }

    size_page = size_elem * d2*d3;
    rowPtr = (void *) ((int ) ptrTable + (sizeof(void *)*d1));
    for (i=0;i<d1;i++) {
        void **ptr = (void **) ptrTable;
        void **rPtr;
        ptr[i] = (void *) ((int ) rowPtr + (i*d2*sizeof(void *)));
        rPtr = (void **) ptr[i];
        for (j=0;j<d2;j++) {
            rPtr[j] = (void *) ((int ) ptrData + 
                              i*size_page + j*(size_elem * d3));
        }
    }
    return ((void ***) ptrTable);
}

/* WL_Free3Dmem    
 *
 * Frees memory for a 3-D array that was allocated using
 * WL_Alloc3Dmem ......    
 */
void WL_Free3Dmem(void *memarg)
{
    void ***mem = (void ***) memarg;
    if (mem) {
        free(mem[0][0]);
        free(mem);
    }
}

dtype **WL_Extract2D(dtype **data,int sR, int sC, int numR, int numC)
{
    int i,j;
    dtype **res;

    res = (dtype **) WL_Alloc2Dmem(numR,numC,sizeof(dtype));

    if (!res)
        return NULL;

    for(i = 0; i < numR; i++)
        for(j = 0; j < numC; j++)
            res[i][j] = data[i+sR][j+sC];

    return (res);
}

void WL_Put2D(dtype **data,dtype **putD,int sR, int sC, int numR, int numC)
{
    int i,j;

    for(i = 0; i < numR; i++)
        for(j = 0; j < numC; j++)
            data[i+sR][j+sC] = putD[i][j];
}

dtype ***WL_Extract3D(dtype ***data,int sD, int sR, int sC, int numD, 
                        int numR, int numC)
{
    int i,j,k;
    dtype ***res;

    res = (dtype ***) WL_Alloc3Dmem(numD,numR,numC,sizeof(dtype));
    if (!res) {
        return NULL;
    }
    for (k = 0; k < numD; k++)
        for (i = 0;i < numR; i++)
            for (j = 0; j < numC; j++)
                res[k][i][j] = data[k+sD][i+sR][j+sC];
    return (res);
}
