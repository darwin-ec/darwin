/* Tcl Wavelet Laboratory
 * Wavelet transform operations
 *
 * Mike Hilton, 8 Mar 1995
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wlcore.h"

/******************************************************************************
 *                                                                            *
 * PERIODIC WAVELET TRANSFORM FUNCTIONS                                       * 
 *                                                                            *
 ******************************************************************************/

/* WL_FwtVolume
 *
 * Perform a LEVELS-deep separable forward wavelet transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in DEST.  
 * ROWS and COLS indicate the size of the SOURCE & DEST volume.
 * The transformed coefficients in DEST are stored in the Mallat 
 * representation.
 *
 * The edge treatment is to perform periodic extension. 
 */   
int WL_FwtVolume(dtype ***source, dtype ***dest, 
		 int depth, int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass)
{
  dtype *tempSource, *tempDest;
  int d, c, r, level;

  /* allocate the temp arrays for columns */
  tempSource = (dtype *) malloc(sizeof(dtype) * MAX(rows,depth));
  tempDest = (dtype *) malloc(sizeof(dtype) * MAX(rows,depth));
  if ((tempSource == NULL) || (tempDest == NULL)) {
    WL_SetErrorMsg("volume fpwt: unable to malloc working vector");
    return WL_ERROR;
  }
  
  /* copy the source to dest to facilitate multiple levels */
  for (d = 0; d < depth; d++)
    for (r = 0; r < rows; r++)
      memcpy(dest[d][r], source[d][r], cols*sizeof(dtype));

  for (level = 0; level < levels; level++, rows /= 2, cols /= 2, depth /= 2) {

    /* transform each row */
    for (d = 0; d < depth; d++) {
      for (r = 0; r < rows; r++){
        if (WL_FwtVector(dest[d][r], dest[d][r], cols, 1,
                  lowpass, hipass) != WL_OK) {
	  free((char *) tempSource);
	  free((char *) tempDest);
	  return WL_ERROR;
        }
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
    for (d = 0; d < depth; d++) {
      for (c = 0; c < cols; c++){
        for (r = 0; r < rows; r++) tempSource[r] = dest[d][r][c];
        if (WL_FwtVector(tempSource, tempDest, rows, 1, lowpass, hipass)
            != WL_OK){
	  free((char *)tempSource);
	  free((char *)tempDest);
	  return WL_ERROR;
        }
        for (r = 0; r < rows; r++) dest[d][r][c] = tempDest[r];
      }
    }

    /* transform each depth line */
    for (r = 0; r < rows; r++) {
      for (c = 0; c < cols; c++){
        for (d = 0; d < depth; d++) tempSource[d] = dest[d][r][c];
        if (WL_FwtVector(tempSource, tempDest, depth, 1, lowpass, hipass)
            != WL_OK){
	  free((char *)tempSource);
	  free((char *)tempDest);
	  return WL_ERROR;
        }
        for (d = 0; d < depth; d++) dest[d][r][c] = tempDest[d];
      }
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


/* WL_IwtVolume
 *
 * Perform a LEVELS-deep separable inverse wavelet transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in DEST.  
 * ROWS and COLS indicate the size of the SOURCE & DEST volume.
 * The transformed coefficients in SOURCE must be stored in the Mallat 
 * format.
 *
 * The edge treatment is to perform periodic extension. 
 */   
int WL_IwtVolume(dtype ***source, dtype ***dest, 
		 int depth, int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass)
{
  dtype *tempSource, *tempDest;
  int d, c, r, level;

  /* allocate the temp arrays for columns */
  tempSource = (dtype *) malloc(sizeof(dtype) * MAX(rows,depth));
  tempDest = (dtype *) malloc(sizeof(dtype) * MAX(rows,depth));
  if ((tempSource == NULL) || (tempDest == NULL)) {
    WL_SetErrorMsg("volume ipwt: unable to malloc working vector");
    return WL_ERROR;
  }

  /* copy the source to dest to facilitate multiple levels */
  for (d = 0; d < depth; d++)
    for (r = 0; r < rows; r++)
      memcpy(dest[d][r], source[d][r], cols*sizeof(dtype));
  
  rows = rows/WL_pow2(levels-1);
  cols = cols/WL_pow2(levels-1);
  depth = depth/WL_pow2(levels-1);

  for (level = 0; level < levels; level++, rows *= 2, cols *= 2, depth *= 2) {

    /* transform each column */
    for (d = 0; d < depth; d++) {
      for (c = 0; c < cols; c++){
        for (r = 0; r < rows; r++) tempSource[r] = dest[d][r][c];
        if (WL_IwtVector(tempSource, tempDest, rows, 1, lowpass, hipass) 
            != WL_OK)
        {
	  free((char *)tempSource);
	  free((char *)tempDest);
	  return WL_ERROR;
        }
        for (r = 0; r < rows; r++) dest[d][r][c] = tempDest[r];
      }
    }

    /* transform each row */
    for (d = 0; d < depth; d++) {
      for (r = 0; r < rows; r++){
        if (WL_IwtVector(dest[d][r], dest[d][r], cols, 1,
		         lowpass, hipass) != WL_OK) {
	  free((char *) tempSource);
	  free((char *) tempDest);
	  return WL_ERROR;
        }
      }
    }

    /* transform each depth line */
    for (r = 0; r < rows; r++) {
      for (c = 0; c < cols; c++){
        for (d = 0; d < depth; d++) tempSource[d] = dest[d][r][c];
        if (WL_IwtVector(tempSource, tempDest, depth, 1, lowpass, hipass)
            != WL_OK){
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

/* WL_FwtMatrix
 *
 * Perform a LEVELS-deep separable forward wavelet transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in DEST.  
 * ROWS and COLS indicate the size of the SOURCE & DEST vectors.
 * The transformed coefficients in DEST are stored in the Mallat 
 * representation.
 *
 * The edge treatment is to perform periodic extension. 
 */   
int WL_FwtMatrix(dtype **source, dtype **dest, 
		 int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass)
{
  dtype *tempSource, *tempDest;
  int c, r, level;

  /* allocate the temp arrays for columns */
  tempSource = (dtype *) malloc(sizeof(dtype) * rows);
  tempDest = (dtype *) malloc(sizeof(dtype) * rows);
  if ((tempSource == NULL) || (tempDest == NULL)) {
    WL_SetErrorMsg("matrix fpwt: unable to malloc working vector");
    return WL_ERROR;
  }
  
  /* copy the source to dest to facilitate multiple levels */
  for (r = 0; r < rows; r++)
    memcpy(dest[r], source[r], cols*sizeof(dtype));

  for (level = 0; level < levels; level++, rows /= 2, cols /= 2) {

    /* transform each row */
    for (r = 0; r < rows; r++){
      if (WL_FwtVector(dest[r], dest[r], cols, 1,
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
      if (WL_FwtVector(tempSource, tempDest, rows, 1, lowpass, hipass)
          != WL_OK){
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

/* WL_IwtMatrix
 *
 * Perform a LEVELS-deep separable inverse wavelet transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in DEST.  
 * ROWS and COLS indicate the size of the SOURCE & DEST matrix.
 * The transformed coefficients in SOURCE must be stored in the Mallat 
 * format.
 *
 * The edge treatment is to perform periodic extension. 
 */   
int WL_IwtMatrix(dtype **source, dtype **dest, 
		 int rows, int cols, int levels, 
		 WL_Filter *lowpass, WL_Filter *hipass)
{
  dtype *tempSource, *tempDest;
  int c, r, level;

  /* allocate the temp arrays for columns */
  tempSource = (dtype *) malloc(sizeof(dtype) * rows);
  tempDest = (dtype *) malloc(sizeof(dtype) * rows);
  if ((tempSource == NULL) || (tempDest == NULL)) {
    WL_SetErrorMsg("matrix ipwt: unable to malloc working vector");
    return WL_ERROR;
  }

  /* copy the source to dest to facilitate multiple levels */
  for (r = 0; r < rows; r++)
    memcpy(dest[r], source[r], cols*sizeof(dtype));
  
  rows = rows/WL_pow2(levels-1);
  cols = cols/WL_pow2(levels-1);

  for (level = 0; level < levels; level++, rows *= 2, cols *= 2) {

    /* transform each column */
    for (c = 0; c < cols; c++){
      for (r = 0; r < rows; r++) tempSource[r] = dest[r][c];
      if (WL_IwtVector(tempSource, tempDest, rows, 1, lowpass, hipass) 
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
    for (r = 0; r < rows; r++){
      if (WL_IwtVector(dest[r], dest[r], cols, 1,
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

  }

  free((char *) tempSource);
  free((char *) tempDest);
  return WL_OK;

}

/* WL_FwtVector
 *
 * Perform a LEVELS-deep forward wavelet transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in DEST.  Optional
 * arguments indicating how edges should be treated are in ARGV.
 * LENGTH is the length of the SOURCE & DEST vectors.
 * The transformed coefficients in DEST are stored in the Mallat 
 * representation.
 *
 * The default edge treatment is periodic extension. The option
 */   
int WL_FwtVector(
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
  int left = MAX(lowpass->offset, hipass->offset);
  int right = MAX(lowpass->length - lowpass->offset - 1,
		  hipass->length - hipass->offset - 1);
  dtype *hiresult, *lowresult;
  dtype *hidata, *lowdata;

  /* allocate memory for the extended copy of source */
  temp = (dtype *) malloc(sizeof(dtype) * (length + left + right));
  if (temp == NULL){
    WL_SetErrorMsg("vector fpwt: unable to malloc working vector");
    return WL_ERROR;
  }

  /* copy source to dest to support doing multiple level transforms */
  memcpy(dest, source, length * sizeof(dtype));

  for (level = 0; level < levels; level++, length /= 2) {

    j = 0;
    for (i = length - left; i < length; i++) temp[j++] = dest[i];
    for (i = 0; i < length; i++) temp[j++] = dest[i];
    for (i = 0; i < right; i++) temp[j++] = dest[i];

#ifdef DEBUG
    fprintf(stderr, "level %d  temp: ", level);
    for(i=0; i< length+left+right; i++)
      fprintf(stderr, "%f ", temp[i]);
    fprintf(stderr, "\n");
#endif

    lowresult = dest;
    hiresult = dest + length/2;
    lowdata = temp + (left - lowpass->offset);
    hidata = temp + (left - hipass->offset);

    for (i = 0; i < length; i += 2, lowdata += 2, hidata += 2) {
      hisum = lowsum = 0;
      for (j = 0; j < lowsize; j++) lowsum += lowdata[j] * lowcoefs[j];
      for (j = 0; j < hisize; j++) hisum += hidata[j] * hicoefs[j];
      *lowresult++ = lowsum;
      *hiresult++ = hisum;
    }

#ifdef DEBUG
    fprintf(stderr, "level %d result: ", level);
    for (i=0; i < length; i++) fprintf(stderr, "%f ", dest[i]);
    fprintf(stderr, "\n");
#endif
  }

  free((char *)temp);
  return WL_OK;
}

/* WL_IwtVector
 *
 * Perform a LEVELS-deep inverse wavelet transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in DEST.  Optional
 * arguments indicating how edges should be treated are in ARGV.
 * LENGTH is the length of the SOURCE & DEST vectors.
 * The coefficients in SOURCE are expected to be in the Mallat format.
 */   
int WL_IwtVector(dtype *source, dtype *dest, int length,
		 int levels, WL_Filter *lowpass, WL_Filter *hipass)
{
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

  /* allocate memory for the extended copies of source */
  upLow = (dtype *) malloc(sizeof(dtype) * (length + left + right));
  upHi = (dtype *) malloc(sizeof(dtype) * (length + left + right));
  if ((upLow == NULL) || (upHi == NULL)){
    WL_SetErrorMsg("vector ipwt: unable to malloc working vector");
    return WL_ERROR;
  }

  length = length/WL_pow2(levels-1);
  /* copy the lowest frequency portion to dest to facilitate multiple levels */
  for (i = 0; i < length; i++) dest[i] = source[i];

  for (level = 0; level < levels; level++, length *= 2){

    /* upsample the source data */
    for (i = 0, j = left; i < length/2; i++) {
      upLow[j] = dest[i];
      upHi[j] = source[length/2+i];
      j += 1;
      upLow[j] = 0;
      upHi[j] = 0;
      j += 1;
    }

    j = 0;
    for (i = length - left; i < length; i++, j++){
	upLow[j] = upLow[left+i];
	upHi[j] = upHi[left+i];
    }
    for (i = 0; i < length; i++, j++) {
	upLow[j] = upLow[left+i];
	upHi[j] = upHi[left+i];
    }
    for (i = 0; i < right; i++, j++){
	upLow[j] = upLow[left+i];
	upHi[j] = upHi[left+i];
    }

#ifdef DEBUG
    fprintf(stderr, "level %d  upLow: ", level);
    for(i=0; i< length+left+right; i++)
      fprintf(stderr, "%f ", upLow[i]);
    fprintf(stderr, "\n");
    fprintf(stderr, "level %d  upHi: ", level);
    for(i=0; i< length+left+right; i++)
      fprintf(stderr, "%f ", upHi[i]);
    fprintf(stderr, "\n");
#endif

    lowdata = upLow + (left - lowpass->offset);
    hidata = upHi + (left - hipass->offset);
    result = dest;
    for (i = 0; i < length; i++, lowdata++, hidata++) {
      sum = 0;
      for (j = 0; j < lowsize; j++) sum += lowdata[j] * lowcoefs[j];
      for (j = 0; j < hisize; j++) sum += hidata[j] * hicoefs[j];
      *result++ = sum;
    }

#ifdef DEBUG
    fprintf(stderr, "level %d result: ", level);
    for (i=0; i < length; i++) fprintf(stderr, "%f ", dest[i]);
    fprintf(stderr, "\n");
#endif

  }

  free((char *)upLow);
  free((char *)upHi);
  return WL_OK;
}

/******************************************************************************
 *
 * MALLAT STORAGE FORMAT DATA STRUCTURE UTILITIES
 *
 ******************************************************************************/

/* BoundingBox
 *
 * Calculates the left, right, top, and bottom boundary of the
 * detail signal for the described image.
 *
 *  ROWS   the number of rows in the full image
 *  COLS   the number of columns in the full image
 *  LEVEL  the level of the detail signal in question
 *  DETAIL the id of the detail signal in question
 *  LEFT, RIGHT, TOP, BOTTOM   the sides of the detail signal, calculated
 *                             by this function
 */
int WL_BoundingBox(int rows, int cols, int level, int detail, 
                   int *left, int *right, int *top, int *bottom)
{
  int rowSize;        /* size of each row at this level */
  int colSize;        /* size of each column at this level */

  /* compute bounding box of gamma quadrant */
  rowSize = cols / WL_pow2(level);
  colSize = rows / WL_pow2(level);

  switch (detail) {
  case 0:
    /* upper right quadrant */
    *left = rowSize;
    *right = 2 * rowSize;
    *top = 0;
    *bottom = colSize;
    break;
  case 1:
    /* lower left quadrant */
    *left = 0;
    *right = rowSize;
    *top = colSize;
    *bottom = 2 * colSize;
    break;
  case 2:
    /* lower right quadrant */
    *left = rowSize;
    *right = 2 * rowSize;
    *top = colSize;
    *bottom = 2 * colSize;
    break;
  default:
    WL_SetErrorMsg("WL_BoundingBox: unexpected detail value.");
    return WL_ERROR;
  }
  return WL_OK;
}
 

/* WL_ExtractDetail
 *
 * Extracts a detail signal from a matrix in Mallat storage format.
 */
int WL_ExtractDetail(dtype **matrix, int rows, int cols, 
		     int level, int detail, dtype ***out)
{
  int left, right, top, bottom;
  dtype **data;
  int i, j, r, c;                      /* loop indices */

  if (WL_BoundingBox(rows, cols, level, detail, 
		     &left, &right, &top, &bottom) != WL_OK) 
    return WL_ERROR;
  if (WL_MakeTempMatrix(bottom - top, right - left, &data) != WL_OK)
    return WL_ERROR;
  *out = data;
  
  for (i = 0, r = top; r < bottom; r++, i++)
    for (j = 0, c = left; c < right; c++, j++)
      data[i][j] = matrix[r][c];

  return WL_OK;
}
