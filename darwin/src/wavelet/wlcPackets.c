/* Wavelet Laboratory (WL)
 * Wavelet packet operations.
 *   Compute a full tree decomposition of a vector or a matrix.
 *
 * Fausto Espinal, 2 June 97
 *
 * A full decomposition is simply the transform taken at recursively
 * along both the low and high pass channel.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wlcore.h"


/* WL_FswptMatrix
 *
 * Computes the full decomposition tree for the symmetric wavelet
 *  transform up to the desired level.
 */
int WL_FswptMatrix(dtype **source, dtype **dest, 
		 int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass)
{
    int nRows, nCols;
    dtype **tmp,**tmpDest;

    if (levels==0) {
        memcpy(&(dest[0][0]),&(source[0][0]),rows*cols*sizeof(dtype));
        return WL_OK;
    }

    if (WL_FswtMatrix(source,dest,rows,cols,1,lowpass,hipass)!=WL_OK) {
        return WL_ERROR;
    }

    nRows   = rows/2;
    nCols   = cols/2;
    tmpDest = (dtype **) WL_Alloc2Dmem(nRows,nCols,sizeof(dtype));

    tmp = WL_Extract2D(dest,0,0,nRows,nCols);
    if (!tmp) {
        WL_Free2Dmem(tmpDest);
        return WL_ERROR;
    }

    if (WL_FswptMatrix(tmp,tmpDest,nRows,nCols,levels-1,lowpass,hipass)!=WL_OK) {
        WL_Free2Dmem(tmpDest);
        WL_Free2Dmem(tmp);
        return WL_ERROR;
    }
    WL_Put2D(dest,tmpDest,0,0,nRows,nCols);
    WL_Free2Dmem(tmp);

    /* Horizontal */
    tmp = WL_Extract2D(dest,0,nCols,nRows,nCols);
    if (!tmp) {
        WL_Free2Dmem(tmpDest);
        return WL_ERROR;
    }

    if (WL_FswptMatrix(tmp,tmpDest,nRows,nCols,levels-1,lowpass,hipass)!=WL_OK) {
        WL_Free2Dmem(tmpDest);
        WL_Free2Dmem(tmp);
        return WL_ERROR;
    }
    WL_Put2D(dest,tmpDest,0,nCols,nRows,nCols);
    WL_Free2Dmem(tmp);

    /* Diagonal */
    tmp = WL_Extract2D(dest,nRows,nCols,nRows,nCols);
    if (!tmp) {
        WL_Free2Dmem(tmpDest);
        return WL_ERROR;
    }

    if (WL_FswptMatrix(tmp,tmpDest,nRows,nCols,levels-1,lowpass,hipass)!=WL_OK) {
        WL_Free2Dmem(tmpDest);
        WL_Free2Dmem(tmp);
        return WL_ERROR;
    }
    WL_Put2D(dest,tmpDest,nRows,nCols,nRows,nCols);
    WL_Free2Dmem(tmp);

    /* Vertical */
    tmp = WL_Extract2D(dest,nRows,0,nRows,nCols);
    if (!tmp) {
        WL_Free2Dmem(tmpDest);
        return WL_ERROR;
    }

    if (WL_FswptMatrix(tmp,tmpDest,nRows,nCols,levels-1,lowpass,hipass)!=WL_OK) {
        WL_Free2Dmem(tmpDest);
        WL_Free2Dmem(tmp);
        return WL_ERROR;
    }
    WL_Put2D(dest,tmpDest,nRows,0,nRows,nCols);
    WL_Free2Dmem(tmp);


    WL_Free2Dmem(tmpDest);
    return WL_OK;
}


/* WL_IswptMatrix
 *
 * Computes the Inverse of the full decomposition tree for the symmetric wavelet
 *  transform up to the desired level.
 */
int WL_IswptMatrix(dtype **source, dtype **dest, 
		 int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass)
{
    int nRows, nCols;
    dtype **tmp,**tmpDest, **tempSource;

    if (levels==1) {
        return (WL_IswtMatrix(source,dest,rows,cols,1,lowpass,hipass));
    }

    nRows=rows/2;
    nCols=cols/2;

    tmpDest = (dtype **) WL_Alloc2Dmem(nRows,nCols,sizeof(dtype));
    tempSource = (dtype **) WL_Alloc2Dmem(rows,cols,sizeof(dtype));

    tmp = WL_Extract2D(source,0,0,nRows,nCols);
    if (!tmp) {
        WL_Free2Dmem(tmpDest);
        return WL_ERROR;
    }

    if (WL_IswptMatrix(tmp,tmpDest,nRows,nCols,levels-1,lowpass,hipass)!=WL_OK) {
        WL_Free2Dmem(tmpDest);
        WL_Free2Dmem(tmp);
        return WL_ERROR;
    }
    WL_Put2D(tempSource,tmpDest,0,0,nRows,nCols);
    WL_Free2Dmem(tmp);

    /* Horizontal */
    tmp = WL_Extract2D(source,0,nCols,nRows,nCols);
    if (!tmp) {
        WL_Free2Dmem(tmpDest);
        return WL_ERROR;
    }

    if (WL_FswptMatrix(tmp,tmpDest,nRows,nCols,levels-1,lowpass,hipass)!=WL_OK) {
        WL_Free2Dmem(tmpDest);
        WL_Free2Dmem(tmp);
        return WL_ERROR;
    }
    WL_Put2D(tempSource,tmpDest,0,nCols,nRows,nCols);
    WL_Free2Dmem(tmp);

    /* Diagonal */
    tmp = WL_Extract2D(source,nRows,nCols,nRows,nCols);
    if (!tmp) {
        WL_Free2Dmem(tmpDest);
        return WL_ERROR;
    }

    if (WL_FswptMatrix(tmp,tmpDest,nRows,nCols,levels-1,lowpass,hipass)!=WL_OK) {
        WL_Free2Dmem(tmpDest);
        WL_Free2Dmem(tmp);
        return WL_ERROR;
    }
    WL_Put2D(tempSource,tmpDest,nRows,nCols,nRows,nCols);
    WL_Free2Dmem(tmp);

    /* Vertical */
    tmp = WL_Extract2D(source,nRows,0,nRows,nCols);
    if (!tmp) {
        WL_Free2Dmem(tmpDest);
        return WL_ERROR;
    }

    if (WL_FswptMatrix(tmp,tmpDest,nRows,nCols,levels-1,lowpass,hipass)!=WL_OK) {
        WL_Free2Dmem(tmpDest);
        WL_Free2Dmem(tmp);
        return WL_ERROR;
    }
    WL_Put2D(tempSource,tmpDest,nRows,0,nRows,nCols);
    WL_Free2Dmem(tmp);

    WL_Free2Dmem(tmpDest);

    if (WL_IswtMatrix(tempSource,dest,rows,cols,1,lowpass,hipass)!=WL_OK) {
        return WL_ERROR;
    }
    WL_Free2Dmem(tempSource);

    return WL_OK;
}



/* WL_PacketBounding
 *
 * Computes the Bounding box of the desired Wavelet Packet.
 * The addresing scheme used is a string based one where the elements
 * of the string come from the alphabet {0,1,2,3} and the length
 * indicates the level of the packet.
 */
int WL_PacketBounding(int rows, int cols, char *addr,
                      int *left, int *right, int *top, int *bottom)
{
    int i;
    int levels;
    int l,r,t,b;

    l=0;
    r=cols;
    t=0;
    b=rows;
    levels=strlen(addr);
    for(i=0;i<levels;i++) {
      switch(addr[i]) {
      case '0':
        r=r-((r-l)/2);
        b=b-((b-t)/2);
        break;
      case '1':
        l=l+((r-l)/2);
        b=b-((b-t)/2);
        break;
      case '2':
        r=r-((r-l)/2);
        t=t+((b-t)/2);
        break;
      case '3':
        l=l+((r-l)/2);
        t=t+((b-t)/2);
        break;
      default:
        return WL_ERROR;
      }
    }
    *left  = l;
    *right = r;
    *top   = t;
    *bottom= b;
    return WL_OK;
}
