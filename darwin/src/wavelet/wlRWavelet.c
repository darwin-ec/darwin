/* Tcl Wavelet Laboratory
 * Redundant (Non-subsampled) Wavelet transform operations
 *
 * Mike Hilton, 30 July 1996
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "wlcore.h"

#if 0
int WL_FrwtMatrix(Tcl_Interp * interp, dtype ** source, dtype *** dest,
		  int rows, int cols, int levels,
		  WL_Filter * lowpass, WL_Filter * hipass);
int WL_IrwtVolume(
	dtype *** source,
	dtype ** dest,
	int rows,
	int cols,
	int levels,
	WL_Filter *lowpass,
	WL_Filter * hipass
);
#endif

/* DILATE_FILTER
 *
 * Dilates the input vector SOURCE by INDEX, storing the result in DEST.  
 * The length of the SOURCE vector is LENGTH.
 */
void Dilate_filter(dtype * source, dtype * dest, int length, int index)
{
    int i, j, k, power;
    power = WL_pow2(index);

    for (i = 0, j = 0; i < length; i++) {
	dest[j++] = source[i];
	for (k = 1; k < power; k++)
	    dest[j++] = (dtype) 0;
    }
}

/********************************************************************
 *
 * Redundant Wavelet Transforms for Vectors
 *
 *******************************************************************/

/* _VectorFrwt
 *
 * Forward redundant wavelet transform of the command vector using
 * <sbfilter>.
 * Syntax:
 *   fat <sbfilter> <levels> 
 *
 * Note: the output is in the form of a matrix of <levels+1> row's, each
 * row being of the same size as the input vector.
 */
#if 0
static int _VectorFrwt(Tcl_Interp * interp, ClientData cd,
		       int argc, char **argv)
{
    WL_Vector *vector = (WL_Vector *) cd;
    WL_SubbandFilter *filter;
    WL_Matrix *result;		/* result is a matrix */
    int levels;
    int temp;

    HELP
	("<sbfilter> <levels>\nReturns a matrix containing the redundant wavelet"
	 "transform of\nthe command vector");

    /* check out args */
    if (argc != 2) {
	interp->result = "vector frwt: wrong # args";
	return 0;
    }
    if (WL_GetObject(interp, argv[0], "sbfilter", (void **) &filter) != 1)
	return 0;
    if (Tcl_GetInt(interp, argv[1], &levels) != 1)
	return 0;
    if (levels < 1) {
	interp->result = "vector frwt: levels must be > 0";
	return 0;
    }

    /* make sure the source vector is diadic */
    if (WL_isDyadic(vector->length) != TRUE) {
	interp->result = "vector frwt: vector must be dyadic";
	return 0;
    }

    /* make sure the source vector is longer than the filter */
    temp = WL_pow2(WL_log2(vector->length) - levels);
    if ((temp < filter->filters[0].length) ||
	(temp < filter->filters[1].length)) {
	interp->result =
	    "vector frwt: vector smaller than filter at some level";
	return 0;
    }

    /* make a result matrix */
    if (WL_MakeMatrix(interp, "", levels + 1, vector->length, &result)
	!= 1)
	return 0;

    /* transform vector */
    strcpy(WL_scratch, interp->result);
    Tcl_ResetResult(interp);
    if (WL_FrwtVector(interp, vector->data, result->data, vector->length,
		      levels, &(filter->filters[0]), &(filter->filters[1]))
	!= 1) {
	Tcl_DeleteCommand(interp, WL_scratch);
	return 0;
    }
    strcpy(interp->result, WL_scratch);

    return 1;
}
#endif

/* WL_FrwtVector
 *
 * Perform a levels-deep forward redundant transform of vector SOURCE using 
 * the filters HIPASS and LOWPASS, storing results in a matrix DEST. 
 * LENGTH is the length of the SOURCE vector & the rows in DEST.printf("here pow2:%d level:%d\n",pow2,level);

 * The transformed coefficients in DEST are stored using an overcomplete
 * representation, each row representing a level in the multiresolution
 * pyramid with coarse approximation in zeroth row and the detail signals
 * at level i in the ith row.
 */
int WL_FrwtVector(
	dtype *source,
	dtype **dest,
	int length,
	int levels,
	WL_Filter *lowpass,
	WL_Filter * hipass
)
{
    dtype *temp;	/* extended source vector */
    int i, j, level, left, right, pow2;
    int hisize, lowsize;
    dtype *hicoefs, *lowcoefs;
    dtype *hiresult, *lowresult;
    dtype *hidata, *lowdata;
    int lowoffset, hioffset;
//printf("here1\n");

    /* allocate memory for the extended copy of source */
    pow2 = WL_pow2(levels - 1);
    lowoffset = lowpass->offset * pow2;
    hioffset = hipass->offset * pow2;
    lowsize = (lowpass->length - 1) * pow2 + 1;
    hisize = (hipass->length - 1) * pow2 + 1;
    left = MAX(lowoffset, hioffset);
    right = MAX(lowsize - lowoffset - 1, hisize - hioffset - 1);
    temp =
	(dtype *) malloc(sizeof(dtype) * (length + (left + right) * pow2));
    if (temp == NULL) {
		printf("WL_FrwtVector: unable to malloc working vector\n");
		return 0;
    }
//printf("here2\n");
    /* allocate memory for the lowpass & highpass filter coefficients */
    hicoefs = (dtype *) malloc((hisize * pow2) * sizeof(dtype));
    lowcoefs = (dtype *) malloc((lowsize * pow2) * sizeof(dtype));
    if ((lowcoefs == NULL) || (hicoefs == NULL)) {
		printf("WL_FrwtVector: unable to malloc filter coefficients\n");
		return 0;
    }
//printf("here3 %d %d %d\n",dest, &source,length);
    /* copy source to dest to support doing multiple level transforms */
    memcpy(dest[0], source, length * sizeof(dtype));

    for (pow2 = 1, level = 0; level < levels; level++, pow2 *= 2) {
	//printf("here pow2:%d level:%d\n",pow2,level);

		/* dilate the filters */
		Dilate_filter(lowpass->coefs, lowcoefs, lowpass->length, level);
		Dilate_filter(hipass->coefs, hicoefs, hipass->length, level);
		lowoffset = lowpass->offset * pow2;
		hioffset = hipass->offset * pow2;
		lowsize = (lowpass->length - 1) * pow2 + 1;
		hisize = (hipass->length - 1) * pow2 + 1;
		left = MAX(lowoffset, hioffset);
		right = MAX(lowsize - lowoffset - 1, hisize - hioffset - 1);

		/* make periodic extension of dest vector */
		j = 0;
		for (i = length - left; i < length; i++)
			temp[j++] = dest[0][i];

		for (i = 0; i < length; i++)
			temp[j++] = dest[0][i];

		for (i = 0; i < right; i++)
			temp[j++] = dest[0][i];

		/* convolve the data */
		lowresult = dest[0];
		hiresult = dest[level + 1];
		lowdata = temp + left - lowoffset;
		hidata = temp + left - hioffset;
		for (i = 0; i < length; i++, lowdata++, hidata++) {
			double hisum = 0, lowsum = 0;
			for (j = 0; j < lowsize; j++)
			lowsum += lowdata[j] * lowcoefs[j];
			for (j = 0; j < hisize; j++)
			hisum += hidata[j] * hicoefs[j];
			*lowresult++ = lowsum;
			*hiresult++ = hisum;
		}
    }

    free((char *) lowcoefs);
    free((char *) hicoefs);
    free((char *) temp);
    return 1;
}

/* WL_IrwtMatrix
 *
 * Perform a LEVELS-deep inverse redundant wavelet transform of matrix SOURCE 
 * using the filters HIPASS and LOWPASS, storing results in vector 
 * DEST.
 * The coefficients in SOURCE are expected to be stored in the 
 * overcomplete representation, each row representing a level in the 
 * multiresolution pyramid with coarse approximation in zeroth row and 
 * the detail signals at level i in the ith row.
 * LENGTH is the length of the rows of SOURCE & of the vector DEST.
 */
int WL_IrwtMatrix(dtype ** source,
		  dtype * dest,
		  int length,
		  int levels, WL_Filter * lowpass, WL_Filter * hipass)
{
    int i, j, level, left, right, pow2;
    int hisize, lowsize;
    int lowoffset, hioffset;
    dtype *hitemp, *lowtemp;	/* extended source vector */
    dtype *hicoefs, *lowcoefs;
    dtype *hidata, *lowdata;
    dtype *result;

    /* allocate memory for the extended copies of hi & low source vectors */
    pow2 = WL_pow2(levels - 1);
    lowoffset = lowpass->offset * pow2;
    hioffset = hipass->offset * pow2;
    lowsize = (lowpass->length - 1) * pow2 + 1;
    hisize = (hipass->length - 1) * pow2 + 1;
    left = MAX(lowoffset, hioffset);
    right = MAX(lowsize - lowoffset - 1, hisize - hioffset - 1);
    hitemp =
	(dtype *) malloc(sizeof(dtype) * (length + (left + right) * pow2));
    lowtemp =
	(dtype *) malloc(sizeof(dtype) * (length + (left + right) * pow2));
    if (hitemp == NULL || lowtemp == NULL) {
	printf("WL_IrwtMatrix: unable to malloc working vectors\n");
	return 0;
    }

    /* allocate memory for the lowpass & highpass filter coefficients */
    hicoefs = (dtype *) malloc((hisize * pow2) * sizeof(dtype));
    lowcoefs = (dtype *) malloc((lowsize * pow2) * sizeof(dtype));

    if ((lowcoefs == NULL) || (hicoefs == NULL)) {
	printf("WL_IrwtMatrix: unable to malloc filter coefficients\n");
	return 0;
    }

    /* copy the lowpass signal to dest to facilitate multiple levels */
    for (i = 0; i < length; i++)
	dest[i] = source[0][i];

    for (pow2 = WL_pow2(levels - 1), level = levels - 1; level >= 0;
	 level--, pow2 /= 2) {

	/* dilate the filters */
	Dilate_filter(lowpass->coefs, lowcoefs, lowpass->length, level);
	Dilate_filter(hipass->coefs, hicoefs, hipass->length, level);
	lowoffset = lowpass->offset * pow2;
	hioffset = hipass->offset * pow2;
	lowsize = (lowpass->length - 1) * pow2 + 1;
	hisize = (hipass->length - 1) * pow2 + 1;
	left = MAX(lowoffset, hioffset);
	right = MAX(lowsize - lowoffset - 1, hisize - hioffset - 1);

	/* make periodic extension of dest vector */
	j = 0;
	for (i = length - left; i < length; i++, j++) {
	    lowtemp[j] = dest[i];
	    hitemp[j] = source[level + 1][i];
	}
	for (i = 0; i < length; i++, j++) {
	    lowtemp[j] = dest[i];
	    hitemp[j] = source[level + 1][i];
	}
	for (i = 0; i < right; i++, j++) {
	    lowtemp[j] = dest[i];
	    hitemp[j] = source[level + 1][i];
	}

	/* convolve the data */
	result = dest;
	lowdata = lowtemp + left - lowoffset;
	hidata = hitemp + left - hioffset;
	for (i = 0; i < length; i++, lowdata++, hidata++) {
	    double hisum = 0, lowsum = 0;
	    for (j = 0; j < lowsize; j++)
		lowsum += lowdata[j] * lowcoefs[j];
	    for (j = 0; j < hisize; j++)
		hisum += hidata[j] * hicoefs[j];
	    *result++ = (lowsum + hisum) * 0.5;
	}

    }

    free((char *) lowcoefs);
    free((char *) hicoefs);
    free((char *) lowtemp);
    free((char *) hitemp);
    return 1;
}

/***************************************************************
 *
 * Redundant Wavelet Transforms for Matrices
 *
 **************************************************************/

/* WL_FrwtMatrix
 *
 * Perform a LEVELS-deep separable forward redundant wavelet transform of 
 * SOURCE using the filters HIPASS and LOWPASS, storing results in volume 
 * DEST. 
 * ROWS and COLS indicate the size of the SOURCE matrix and rows and
 * columns of volume DEST.
 * The transformed coefficients in DEST are stored using an overcomplete
 * representation: each ROWSxCOLS matrix representing a level in the 
 * multiresolution pyramid with coarse approximation in zeroth matrix and 
 * the 3 detail signals at level i in matrix 3*i+j, where 1<=j<=3.
 * The output volume DEST has depth 3*levels+1.
 */
int WL_FrwtMatrix(dtype ** source,
		  dtype *** dest,
		  int rows,
		  int cols,
		  int levels, WL_Filter * lowpass, WL_Filter * hipass)
{
    dtype *tempSource, **RowMatrix, **ColMatrix;
    int c, r, level;
    int hori, vert, diag;

    /* allocate the temp arrays to hold 1-D results */
    if (WL_MakeTempMatrix(2, cols, &RowMatrix) != 1) {
	printf("WL_FrwtMatrix: unable to malloc working matrix\n");
	return 0;
    }
    if (WL_MakeTempMatrix(2, rows, &ColMatrix) != 1) {
	printf("WL_FrwtMatrix: unable to malloc working matrix\n");
	WL_FreeTempMatrix(RowMatrix);
	return 0;
    }
    tempSource = (dtype *) malloc(sizeof(dtype) * rows);
    if (tempSource == NULL) {
	printf("WL_FrwtMatrix: unable to malloc working matrix\n");
	WL_FreeTempMatrix(RowMatrix);
	WL_FreeTempMatrix(ColMatrix);
	return 0;
    }

    /* copy the source to dest to facilitate multiple levels */
    for (r = 0; r < rows; r++)
	memcpy(dest[0][r], source[r], cols * sizeof(dtype));

    for (level = 1; level <= levels; level++) {

	/* setup indices to horizontal,vertical & diagonal detail signals */
	vert = 3 * (level - 1) + 1;
	hori = vert + 1;
	diag = hori + 1;

	/* transform each row */
	for (r = 0; r < rows; r++) {
	    if (WL_FrwtVector(dest[0][r], RowMatrix, cols,
			      level, lowpass, hipass) != 1) {
		WL_FreeTempMatrix(RowMatrix);
		WL_FreeTempMatrix(ColMatrix);
		free((char *) tempSource);
		return 0;
	    }
	    memcpy(dest[0][r], RowMatrix[0], cols * sizeof(dtype));
	    memcpy(dest[hori][r], RowMatrix[1], cols * sizeof(dtype));
	}

	/* transform each column */
	/* first, run the transform on the lowpass output from the */
	/* frwt algorithm run on the rows */
	for (c = 0; c < cols; c++) {
	    for (r = 0; r < rows; r++)
		tempSource[r] = dest[0][r][c];
	    if (WL_FrwtVector(tempSource, ColMatrix, rows,
			      level, lowpass, hipass) != 1) {
		WL_FreeTempMatrix(RowMatrix);
		WL_FreeTempMatrix(ColMatrix);
		free((char *) tempSource);
		return 0;
	    }
	    for (r = 0; r < rows; r++) {
		dest[0][r][c] = ColMatrix[0][r];
		dest[vert][r][c] = ColMatrix[1][r];
	    }
	}

	/* now, for the high pass output */
	for (c = 0; c < cols; c++) {
	    for (r = 0; r < rows; r++)
		tempSource[r] = dest[hori][r][c];
	    if (WL_FrwtVector(tempSource, ColMatrix, rows,
			      level, lowpass, hipass) != 1) {
		WL_FreeTempMatrix(RowMatrix);
		WL_FreeTempMatrix(ColMatrix);
		free((char *) tempSource);
		return 0;
	    }
	    for (r = 0; r < rows; r++) {
		dest[hori][r][c] = ColMatrix[0][r];
		dest[diag][r][c] = ColMatrix[1][r];
	    }
	}
    }

    /* free up the temporary storage */
    WL_FreeTempMatrix(RowMatrix);
    WL_FreeTempMatrix(ColMatrix);
    free((char *) tempSource);
    return 1;
}

/* WL_IrwtVolume
 *
 * Perform a LEVELS-deep separable inverse redundant transform of SOURCE using 
 * the filters HIPASS and LOWPASS,  storing results in matrix DEST. 
 * ROWS and COLS indicate the size of the rows and columns of volume SOURCE 
 * and the size of matrix DEST.
 * The transformed coefficients in SOURCE are stored using an overcomplete
 * representation, each ROWSxCOLS matrix representing a level in the 
 * multiresolution pyramid with coarse approximation in zeroth matrix and 
 * the 3 detail signals at level i in matrix 3*i+j, where 1<=j<=3.
 * The input volume SOURCE has depth levels+1.
 */
int WL_IrwtVolume(
	dtype *** source,
	dtype ** dest,
	int rows,
	int cols,
	int levels,
	WL_Filter *lowpass,
	WL_Filter * hipass
)
{
    dtype *tempDest, **HighMatrix, **RowMatrix, **ColMatrix;
    int c, r, level;
    int hori, vert, diag;

    /* allocate the temp arrays to hold 1-D results */
    if (WL_MakeTempMatrix(2, cols, &RowMatrix) != 1) {
	printf("WL_IrwtMatrix: unable to malloc working matrix\n");
	return 0;
    }
    if (WL_MakeTempMatrix(2, rows, &ColMatrix) != 1) {
	printf("WL_IrwtMatrix: unable to malloc working matrix\n");
	WL_FreeTempMatrix(RowMatrix);
	return 0;
    }
    if (WL_MakeTempMatrix(rows, cols, &HighMatrix) != 1) {
	printf("WL_IrwtMatrix: unable to malloc working matrix\n");
	WL_FreeTempMatrix(RowMatrix);
	WL_FreeTempMatrix(ColMatrix);
	return 0;
    }
    tempDest = (dtype *) malloc(sizeof(dtype) * rows);
    if (tempDest == NULL) {
	printf("WL_IrwtMatrix: unable to malloc working matrix\n");
	WL_FreeTempMatrix(RowMatrix);
	WL_FreeTempMatrix(ColMatrix);
	WL_FreeTempMatrix(HighMatrix);
	return 0;
    }


    /* copy the source to dest to facilitate multiple levels */
    for (r = 0; r < rows; r++)
	memcpy(dest[r], source[0][r], cols * sizeof(dtype));

    for (level = levels; level >= 1; level--) {

	/* setup indices to horizontal,vertical & diagonal detail signals */
	vert = 3 * (level - 1) + 1;
	hori = vert + 1;
	diag = hori + 1;

	/* transform each column */
	/* first, run the inverse transform using the lowpass output */
	/* from the 1D forward redundant transform along the columns */
	for (c = 0; c < cols; c++) {
	    for (r = 0; r < rows; r++) {
		ColMatrix[0][r] = dest[r][c];
		ColMatrix[1][r] = source[vert][r][c];
	    }
	    if (WL_IrwtMatrix
		(ColMatrix, tempDest, rows, level, lowpass, hipass) != 1) {
		WL_FreeTempMatrix(RowMatrix);
		WL_FreeTempMatrix(ColMatrix);
		WL_FreeTempMatrix(HighMatrix);
		free((char *) tempDest);
		return 0;
	    }
	    for (r = 0; r < rows; r++)
		dest[r][c] = tempDest[r];
	}

	/* now, for the high pass output */
	for (c = 0; c < cols; c++) {
	    for (r = 0; r < rows; r++) {
		ColMatrix[0][r] = source[hori][r][c];
		ColMatrix[1][r] = source[diag][r][c];
	    }
	    if (WL_IrwtMatrix
		(ColMatrix, tempDest, rows, level, lowpass, hipass) != 1) {
		WL_FreeTempMatrix(RowMatrix);
		WL_FreeTempMatrix(ColMatrix);
		WL_FreeTempMatrix(HighMatrix);
		free((char *) tempDest);
		return 0;
	    }
	    for (r = 0; r < rows; r++)
		HighMatrix[r][c] = tempDest[r];
	}

	/* transform each row */
	for (r = 0; r < rows; r++) {
	    memcpy(RowMatrix[0], dest[r], cols * sizeof(dtype));
	    memcpy(RowMatrix[1], HighMatrix[r], cols * sizeof(dtype));
	    if (WL_IrwtMatrix
		(RowMatrix, dest[r], cols, level, lowpass, hipass) != 1) {
		WL_FreeTempMatrix(RowMatrix);
		WL_FreeTempMatrix(ColMatrix);
		WL_FreeTempMatrix(HighMatrix);
		free((char *) tempDest);
		return 0;
	    }
	}
    }

    /* free up the temporary storage */
    WL_FreeTempMatrix(RowMatrix);
    WL_FreeTempMatrix(ColMatrix);
    WL_FreeTempMatrix(HighMatrix);
    free((char *) tempDest);
    return 1;
}
