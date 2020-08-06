/* Tcl Wavelet Laboratory
 * Symmetric Wavelet transform operations
 *
 * Mike Hilton, 
 * Fausto Espinal, 13 Jan 1996
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wlcore.h"


/******************************************************************************
 *                                                                            *
 * SYMMETRIC WAVELET TRANSFORM FUNCTIONS                                      *
 *                                                                            *
 ******************************************************************************/

/* WL_FswtVolume
 *
 * Perform a LEVELS-deep separable forward wavelet transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in DEST.  
 * DEPTH, ROWS and COLS indicate the size of the SOURCE & DEST volumes.
 * The transformed coefficients in DEST are stored in the Mallat 
 * representation.
 */   
int WL_FswtVolume(dtype ***source, dtype ***dest, 
		 int depth, int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass)
{
  dtype *tempSource, *tempDest;
  int d, c, r, level;

  /* allocate the temp arrays for columns */
  tempSource = (dtype *) malloc(sizeof(dtype) * MAX(rows,depth));
  tempDest = (dtype *) malloc(sizeof(dtype) * MAX(rows,depth));
  if ((tempSource == NULL) || (tempDest == NULL)) {
    WL_SetErrorMsg("volume fswt: unable to malloc working vector");
    return WL_ERROR;
  }
  
  /* copy the source to dest to facilitate multiple levels */
  for (d = 0; d < depth; d++)
    for (r = 0; r < rows; r++)
      memcpy(dest[d][r], source[d][r], cols*sizeof(dtype));

  for (level = 0; level < levels; level++, rows=(rows+1)/2, 
       cols=(cols+1)/2, depth=(depth+1)/2) {

    /* transform each row */
    for (d = 0; d < depth; d++){
      for (r = 0; r < rows; r++){
        if (WL_FswtVector(dest[d][r], dest[d][r], cols, 1,
		         lowpass, hipass) != WL_OK) {
	  free((char *) tempSource);
	  free((char *) tempDest);
	  return WL_ERROR;
        }
      }
    }

    /* transform each column */
    for (d = 0; d < depth; d++){
      for (c = 0; c < cols; c++){
        for (r = 0; r < rows; r++) tempSource[r] = dest[d][r][c];
        if (WL_FswtVector(tempSource,tempDest,rows, 1,lowpass,hipass) 
            != WL_OK)
        {
	  free((char *)tempSource);
	  free((char *)tempDest);
	  return WL_ERROR;
        }
        for (r = 0; r < rows; r++) dest[d][r][c] = tempDest[r];
      }
    }

    /* transform each depth line */
    for (r = 0; r < rows; r++){
      for (c = 0; c < cols; c++){
        for (d = 0; d < depth; d++) tempSource[d] = dest[d][r][c];
        if (WL_FswtVector(tempSource,tempDest,depth, 1,lowpass,hipass) 
            != WL_OK)
        {
	  free((char *)tempSource);
	  free((char *)tempDest);
	  return WL_ERROR;
        }
        for (d = 0; d < depth; d++) dest[d][r][c] = tempDest[d];
      }
    }

  }

  free((char *) tempSource);
  free((char *) tempDest);
  return WL_OK;
}

/* WL_IswtVolume
 *
 * Perform a LEVELS-deep separable inverse symmetric wavelet 
 * transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in DEST.  
 * DEPTH, ROWS and COLS indicate the size of the SOURCE & DEST volumes.
 * The transformed coefficients in SOURCE must be stored in the Mallat 
 * format.
 */   
int WL_IswtVolume(dtype ***source, dtype ***dest, 
		 int depth, int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass)
{
  int   *dSplit, *rSplit, *cSplit;
  dtype *tempSource, *tempDest;
  int d, c, r, level;

  rSplit = (int *) malloc(sizeof(int) * (levels + 1));
  cSplit = (int *) malloc(sizeof(int) * (levels + 1));
  dSplit = (int *) malloc(sizeof(int) * (levels + 1));
  if ((rSplit == NULL) || (cSplit == NULL) || (dSplit == NULL)) {
    WL_SetErrorMsg("volume iswt: unable to malloc working vector");
    return WL_ERROR;
  }
  /* allocate the temp arrays for columns */
  tempSource = (dtype *) malloc(sizeof(dtype) * MAX(depth,rows));
  tempDest = (dtype *) malloc(sizeof(dtype) * MAX(depth,rows));
  if ((tempSource == NULL) || (tempDest == NULL)) {
    WL_SetErrorMsg("volume iswt: unable to malloc working vector");
    return WL_ERROR;
  }

  /* copy the source to dest to facilitate multiple levels */
  for (d = 0; d < depth; d++)
    for (r = 0; r < rows; r++)
      memcpy(dest[d][r], source[d][r], cols*sizeof(dtype));
  
  /* Determine the way the dimensions of the signal is partitioned. */
  for (level = levels, c=cols, r=rows, d=depth; level > 0; level--){
      rSplit[level] = r / 2;
      r = (r + 1) / 2;
      cSplit[level] = c / 2;
      c = (c + 1) / 2;
      dSplit[level] = d / 2;
      d = (d + 1) / 2;
  }
  rSplit[0] = r;
  cSplit[0] = c;
  dSplit[0] = d;

  rows  = rSplit[0];
  cols  = cSplit[0];
  depth = dSplit[0];

  for (level = 1; level < levels+1; level++) {

    rows  += (rSplit[level]);
    cols  += (cSplit[level]);
    depth += (dSplit[level]);

    /* transform each row */
    for (d = 0; d < depth; d++){
      for (r = 0; r < rows; r++){
        if (WL_IswtVector(dest[d][r], dest[d][r], cols, 1,
		         lowpass, hipass) != WL_OK) {
	  free((char *) tempSource);
	  free((char *) tempDest);
	  return WL_ERROR;
        }
      }
    }

    /* transform each column */
    for (d = 0; d < depth; d++){
      for (c = 0; c < cols; c++){
        for (r = 0; r < rows; r++) tempSource[r] = dest[d][r][c];
        if (WL_IswtVector(tempSource,tempDest,rows, 1,lowpass,hipass) 
            != WL_OK)
        {
	  free((char *)tempSource);
	  free((char *)tempDest);
	  return WL_ERROR;
        }
        for (r = 0; r < rows; r++) dest[d][r][c] = tempDest[r];
      }
    }

    /* transform each depth line */
    for (r = 0; r < rows; r++){
      for (c = 0; c < cols; c++){
        for (d = 0; d < depth; d++) tempSource[d] = dest[d][r][c];
        if (WL_IswtVector(tempSource,tempDest,depth, 1,lowpass,hipass) 
            != WL_OK)
        {
	  free((char *)tempSource);
	  free((char *)tempDest);
	  return WL_ERROR;
        }
        for (d = 0; d < depth; d++) dest[d][r][c] = tempDest[d];
      }
    }

  }

  free((int *) rSplit);
  free((int *) cSplit);
  free((int *) dSplit);
  free((char *) tempSource);
  free((char *) tempDest);
  return WL_OK;

}


/* WL_FswtMatrix
 *
 * Perform a LEVELS-deep separable forward wavelet transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in DEST.  
 * ROWS and COLS indicate the size of the SOURCE & DEST vectors.
 * The transformed coefficients in DEST are stored in the Mallat 
 * representation.
 */   
int WL_FswtMatrix(dtype **source, dtype **dest, 
		 int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass)
{
  dtype *tempSource, *tempDest;
  int c, r, level;

  /* allocate the temp arrays for columns */
  tempSource = (dtype *) malloc(sizeof(dtype) * rows);
  tempDest = (dtype *) malloc(sizeof(dtype) * rows);
  if ((tempSource == NULL) || (tempDest == NULL)) {
    WL_SetErrorMsg("matrix fwt: unable to malloc working vector");
    return WL_ERROR;
  }
  
  /* copy the source to dest to facilitate multiple levels */
  for (r = 0; r < rows; r++)
    memcpy(dest[r], source[r], cols*sizeof(dtype));

  for (level = 0; level < levels; level++, rows=(rows+1)/2, cols=(cols+1)/2) 
  {
    /* transform each row */
    for (r = 0; r < rows; r++){
      if (WL_FswtVector(dest[r], dest[r], cols, 1,
		       lowpass, hipass) != WL_OK) {
	free((char *) tempSource);
	free((char *) tempDest);
	return WL_ERROR;
      }
    }

#ifdef DEBUG_MATRIX
    fprintf(stderr, "level %d ROW:\n", level);
    for (r = 0; r < rows; r++){
      for (c = 0; c < cols; c++) fprintf(stderr, "%f ", dest[r][c]);
      fprintf(stderr, "\n");
    }
#endif

    /* transform each column */
    for (c = 0; c < cols; c++){
      for (r = 0; r < rows; r++) tempSource[r] = dest[r][c];
      if (WL_FswtVector(tempSource, tempDest, rows, 1, lowpass, hipass) 
          != WL_OK)
      {
	free((char *)tempSource);
	free((char *)tempDest);
	return WL_ERROR;
      }
      for (r = 0; r < rows; r++) dest[r][c] = tempDest[r];
    }

#ifdef DEBUG_MATRIX
    fprintf(stderr, "level %d COL:\n", level);
    for (r = 0; r < rows; r++){
      for (c = 0; c < cols; c++) fprintf(stderr, "%f ", dest[r][c]);
      fprintf(stderr, "\n");
    }
#endif

  }

  free((char *) tempSource);
  free((char *) tempDest);
  return WL_OK;
}


/* WL_IswtMatrix
 *
 * Perform a LEVELS-deep separable inverse symmetric wavelet 
 * transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in DEST.  
 * ROWS and COLS indicate the size of the SOURCE & DEST vectors.
 * The transformed coefficients in SOURCE must be stored in the Mallat 
 * format.
 */   
int WL_IswtMatrix(dtype **source, dtype **dest, 
		 int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass)
{
  int   *rSplit, *cSplit;
  dtype *tempSource, *tempDest;
  int    c, r, level;

  rSplit = (int *) malloc(sizeof(int) * (levels + 1));
  cSplit = (int *) malloc(sizeof(int) * (levels + 1));
  if ((rSplit == NULL) || (cSplit == NULL)) {
    WL_SetErrorMsg("matrix iswt: unable to malloc working vector");
    return WL_ERROR;
  }
  /* allocate the temp arrays for columns */
  tempSource = (dtype *) malloc(sizeof(dtype) * rows);
  tempDest = (dtype *) malloc(sizeof(dtype) * rows);
  if ((tempSource == NULL) || (tempDest == NULL)) {
    WL_SetErrorMsg("matrix iswt: unable to malloc working vector");
    return WL_ERROR;
  }

  /* copy the source to dest to facilitate multiple levels */
  for (r = 0; r < rows; r++)
    memcpy(dest[r], source[r], cols*sizeof(dtype));
  
  /* Determine the way the dimensions of the signal is partitioned. */
  for (level = levels, c=cols, r=rows; level > 0; level--){
      rSplit[level] = r / 2;
      r = (r + 1) / 2;
      cSplit[level] = c / 2;
      c = (c + 1) / 2;
  }
  rSplit[0] = r;
  cSplit[0] = c;

  rows = rSplit[0];
  cols = cSplit[0];

  for (level = 1; level < levels+1; level++) {
    rows+=(rSplit[level]);
    cols+=(cSplit[level]);

    /* transform each column */
    for (c = 0; c < cols; c++){
      for (r = 0; r < rows; r++) tempSource[r] = dest[r][c];
      if (WL_IswtVector(tempSource, tempDest, rows, 1, lowpass, hipass) 
          != WL_OK)
      {
	free((char *)tempSource);
	free((char *)tempDest);
	return WL_ERROR;
      }
      for (r = 0; r < rows; r++) dest[r][c] = tempDest[r];
    }

#ifdef DEBUG_MATRIX
    fprintf(stderr, "level %d COL:\n", level);
    for (r = 0; r < rows; r++){
      for (c = 0; c < cols; c++) fprintf(stderr, "%f ", dest[r][c]);
      fprintf(stderr, "\n");
    }
#endif

    /* transform each row */
    for (r = 0; r < rows; r++) {
      if (WL_IswtVector(dest[r], dest[r], cols, 1,lowpass, hipass) 
          != WL_OK) 
      {
	free((char *) tempSource);
	free((char *) tempDest);
	return WL_ERROR;
      }
    }

#ifdef DEBUG_MATRIX
    fprintf(stderr, "level %d ROW:\n", level);
    for (r = 0; r < rows; r++){
      for (c = 0; c < cols; c++) fprintf(stderr, "%f ", dest[r][c]);
      fprintf(stderr, "\n");
    }
#endif

  }

  free((int *) rSplit);
  free((int *) cSplit);
  free((char *) tempSource);
  free((char *) tempDest);
  return WL_OK;
}

/* WL_FswtVector
 *
 * Perform a LEVELS-deep symmetric forward wavelet transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in DEST.  
 * LENGTH is the length of the SOURCE & DEST vectors.
 * The transformed coefficients in DEST are stored in the Mallat 
 * representation.
 */   
int WL_FswtVector(
	dtype *source,
	dtype *dest,
	int length,
	int levels,
	WL_Filter *lowpass,
	WL_Filter *hipass
)
{
  dtype *temp;        /* extended source vector */
  int i, j, level;
  int hisize = hipass->length;
  int lowsize = lowpass->length;
  dtype *hicoefs = hipass->coefs;
  dtype *lowcoefs = lowpass->coefs;
  double hisum, lowsum;
  int left  = MAX(lowpass->offset, hipass->offset);
  int right = MAX(lowpass->length - lowpass->offset - 1,
		  hipass->length - hipass->offset - 1);
  dtype *hiresult, *lowresult;
  dtype *hidata, *lowdata;

  /* allocate memory for the extended copy of source */
  temp = (dtype *) malloc(sizeof(dtype) * (length + left + right));
  if (temp == NULL){
    WL_SetErrorMsg("FswtVector: unable to malloc working vector");
    return WL_ERROR;
  }

  /* copy source to dest to support doing multiple level transforms */
  memcpy(dest, source, length * sizeof(dtype));

  for (level = 0; level < levels; level++) {
    /* make symmetric extension of dest vector */
    if (lowsize%2  == 1) {
      /* odd length filter (WSS filter), use E(1,1) extension */
      j = 0;
      for (i = left; i > 0; i--) temp[j++] = dest[i];
      for (i = 0; i < length; i++) temp[j++] = dest[i];
      for (i = length-2; i > length - 2 - right; i--) temp[j++] = dest[i];
    } else {
      /* even length filter (HS filter), use E(2,2) extension */
      j = 0;
      for (i = left-1; i >= 0; i--) temp[j++] = dest[i];
      for (i = 0; i < length; i++) temp[j++] = dest[i];
      for (i = length-1; i >= length-right; i--) temp[j++] = dest[i];
    }

    /* filter the temp vector */
    lowresult = dest;
    hiresult = dest + (length+1)/2;
    lowdata = temp + (left - lowpass->offset);
    hidata = temp + (left - hipass->offset);
    for (i = 0; i < length-1; i += 2) {
      hisum = lowsum = 0;
      for (j = 0; j < lowsize; j++) lowsum += lowdata[j] * lowcoefs[j];
      for (j = 0; j < hisize; j++) hisum += hidata[j] * hicoefs[j];
      *lowresult++ = lowsum;
      *hiresult++ = hisum;
      lowdata+=2;
      hidata+=2;
    }
    /* Compute extra low-pass coefficient if signal is odd */
    if (length % 2) {
      lowsum = 0;
      for (j = 0; j < lowsize; j++) lowsum += lowdata[j] * lowcoefs[j];
      *lowresult = lowsum;
    }
    length = (length + 1) / 2;
  }

  free((char *)temp);
  return (WL_OK);
}

/* WL_IswtVector
 *
 * Perform a LEVELS-deep inverse symmetric wavelet transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in DEST.  
 * LENGTH is the length of the SOURCE & DEST vectors.
 * The coefficients in SOURCE are expected to be in the Mallat format.
 */   
int WL_IswtVector(
	dtype *source,
	dtype *dest,
	int length,
	int levels,
	WL_Filter *lowpass,
	WL_Filter *hipass
)
{
  int   *sgnpart;
  int    wBound;       /* Current wavelet coefficient boundary */
  int    tmp;
  dtype *upLow;        /* extended upsampled low frequency vector */
  dtype *upHi;         /* extended upsampled high frequency vector */
  int i, j, level;
  int hisize = hipass->length;
  int lowsize = lowpass->length;
  dtype *hicoefs = hipass->coefs;
  dtype *lowcoefs = lowpass->coefs;
  double sum;
  int left = MAX(lowpass->offset, hipass->offset);
  int right = MAX(lowpass->length - lowpass->offset - 1,
		  hipass->length - hipass->offset - 1);
  dtype *result;
  dtype *hidata, *lowdata;

  sgnpart = (int *) malloc(sizeof(int) * (levels + 1));
  /* allocate memory for the extended copies of source */
  upLow = (dtype *) malloc(sizeof(dtype) * (length + left + right));
  upHi  = (dtype *) malloc(sizeof(dtype) * (length + left + right));
  if ((upLow == NULL) || (upHi == NULL)){
    WL_SetErrorMsg("IswtVector: unable to malloc working vector");
    return WL_ERROR;
  }

  /* Determine the way the transformed signal is partitioned. */
  for (level = levels, tmp=length; level > 0; level--){
      sgnpart[level] = tmp / 2;
      tmp = (tmp + 1) / 2;
  }
  sgnpart[0] = tmp;

  /* copy the lowest frequency portion to dest to facilitate multiple levels */
  for (i = 0; i < sgnpart[0]+sgnpart[1]; i++) dest[i] = source[i];
  wBound = sgnpart[0];

  for (level = 0; level < levels; level++) {

    /* upsample the source data */
    for (i = 0, j = left; i < wBound; i++) {
      upLow[j] = dest[i];
      j += 1;
      upLow[j] = 0;
      j += 1;
    }
    for (i = 0, j = left; i < sgnpart[level+1]; i++) {
      upHi[j] = source[wBound+i];
      j += 1;
      upHi[j] = 0;
      j += 1;
    }

    length = wBound + sgnpart[level+1];

    /* edge extension options */
    if (lowsize%2 == 1) {
      /* odd length filter (WSS filter) */
      if (length%2 == 0) {
	/* even length signal */
	/* use E(1,2) for low */
	for (j = 0, i = 2*left; i > left; i--) upLow[j++] = upLow[i];
	for (j = left+length, i = left+length-2; i > left+length-2-right; i--)
	  upLow[j++] = upLow[i];
	/* use E(2,1) for hi */
	upHi[left-1] = 0;
	for (j = 0, i = 2*left-2; i >= left; i--) upHi[j++] = upHi[i];
	for (j = length+left, i = left+length-4; i > left+length-4-right; i--)
	  upHi[j++] = upHi[i];
      } else {
	/* odd length signal */
	/* use E(1,1) for low, symmetric */
	for (j = left-1, i = left+1; i<= 2*left; i++) upLow[j--] = upLow[i];
	for (j = length+left, i = length+left-2; i > left+length-2-right; i--)
	  upLow[j++] = upLow[i];
	/* use E(2,2) for hi, symmetric */
        upHi[left-1]=0;
	for (j = left-2, i = left; i< 2*left-1; i++) upHi[j--] = upHi[i];
        upHi[length+left-1] = upHi[length+left-3];
	for (j = length+left, i = length+left-4; i > left+length-4-right; i--)
	  upHi[j++] = upHi[i];
      }
    } else {
      /* even length filter (HS filter) */
      if (length%2 == 0) {
	/* even length signal */
	/* use E(2,2) for low */
	upLow[left-1] = 0;
	for (j = 0, i = 2*left-2; i >= left; i--) upLow[j++] = upLow[i];
	for (j = left+length, i = left+length-2; i > left+length-2-right; i--)
	  upLow[j++] = upLow[i];
	/* use E(2,2) for hi, antisymmetric */
	upHi[left-1] = 0;
	for (j = 0, i = 2*left-2; i >= left; i--) upHi[j++] = -upHi[i];
	for (j = left+length, i = left+length-2; i > left+length-2-right; i--)
	  upHi[j++] = -upHi[i];
      } else {
	/* odd length signal */
	/* use E(2,1) for low */
        upLow[left-1]=0;
	for (j = left-2, i = left; i< 2*left-1; i++) upLow[j--] = upLow[i];
	for (j = length+left, i = length+left-2; i > left+length-2-right; i--)
	  upLow[j++] = upLow[i];
	/* use E(2,1) for hi, antisymmetric */
        upHi[left-1]=0;
	for (j = left-2, i = left; i< 2*left-1; i++) upHi[j--] = -upHi[i];
        upHi[length+left-1] = -upHi[length+left-5];
	for (j = length+left, i = length+left-6; i > left+length-6-right; i--)
	  upHi[j++] = -upHi[i];
      }
    }      

#if 0
printf("\n\n-------------------------\n");
printf("Length = %d   Left = %d    Right = %d\n",length,left,right);
printf("upLow = ");
for (i = 0; i < left; i++) printf("%g ", upLow[i]);
printf("\n");
for (i = left; i < length+left; i++) printf("%g ", upLow[i]);
printf("\n");
for (i = length+left; i < length+left+right; i++) printf("%g ", upLow[i]);
printf("\nupHi  = ");
for (i = 0; i < left; i++) printf("%g ", upHi[i]);
printf("\n");
for (i = left; i < length+left; i++) printf("%g ", upHi[i]);
printf("\n");
for (i = length+left; i < length+left+right; i++) printf("%g ", upHi[i]);
printf("\n--------------------------\n\n");
#endif

    /* filter the source vectors */
    lowdata = upLow + (left - lowpass->offset);
    hidata = upHi + (left - hipass->offset);
    result = dest;
    for (i = 0; i < length; i++, lowdata++, hidata++) {
      sum = 0;
      for (j = 0; j < lowsize; j++) sum += lowdata[j] * lowcoefs[j];
      for (j = 0; j < hisize; j++) sum += hidata[j] * hicoefs[j];
      *result++ = sum;
    }
    
    wBound+=(sgnpart[level+1]);
  }

  free((int *) sgnpart);
  free((char *)upLow);
  free((char *)upHi);
  return WL_OK;
}
