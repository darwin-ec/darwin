/*******************************************************************************
* FILE: hysteresis.c
* This code was re-written by Mike Heath from original code obtained indirectly
* from Michigan State University. heath@csee.usf.edu (Re-written in 1996).
*******************************************************************************/

#include "hysteresis.h"
#include <stdio.h>
#include <stdlib.h>

#define NOEDGE 0
#define POSSIBLE_EDGE 128
#define EDGE 255 

/*******************************************************************************
* PROCEDURE: follow_edges
* PURPOSE: This procedure edges is a recursive routine that traces edgs along
* all paths whose magnitude values remain above some specifyable lower
* threshhold.
* NAME: Mike Heath
* DATE: 2/15/96
*******************************************************************************/
void follow_edges(unsigned char *edgemapptr, short *edgemagptr, short lowval,
	     int cols)
{
    short *tempmagptr;
    unsigned char *tempmapptr;
    int i;
    int x[8] = { 1, 1, 0, -1, -1, -1, 0, 1 }, y[8] = {
    0, 1, 1, 1, 0, -1, -1, -1};

    for (i = 0; i < 8; i++) {
	tempmapptr = edgemapptr - y[i] * cols + x[i];
	tempmagptr = edgemagptr - y[i] * cols + x[i];

	if ((*tempmapptr == POSSIBLE_EDGE) && (*tempmagptr > lowval)) {
	    *tempmapptr = (unsigned char) EDGE;
	    follow_edges(tempmapptr, tempmagptr, lowval, cols);
	}
    }
}

/*******************************************************************************
* PROCEDURE: apply_hysteresis
* PURPOSE: This routine finds edges that are above some high threshhold or
* are connected to a high pixel by a path of pixels greater than a low
* threshold.
* NAME: Mike Heath
* DATE: 2/15/96
*******************************************************************************/
GrayImage *applyHysteresis(
		short int *mag,
		GrayImage *nms,
		int rows,
		int cols,
		float tlow,
		float thigh
)
{
    int r, c, pos, numedges, highcount, lowthreshold, highthreshold,
	hist[32768];
    short int maximum_mag = 0;

    // This GrayImage will hold the edge detection image
    GrayImage *edge = new GrayImage(rows, cols);

   /****************************************************************************
   * Initialize the edge map to possible edges everywhere the non-maximal
   * suppression suggested there could be an edge except for the border. At
   * the border we say there can not be an edge because it makes the
   * follow_edges algorithm more efficient to not worry about tracking an
   * edge off the side of the image.
   ****************************************************************************/
    for (r = 0; r < rows; r++) {
	for (c = 0; c < cols; c++) {
	    if ((*nms)(r, c) == POSSIBLE_EDGE)
		(*edge)(r, c) = POSSIBLE_EDGE;
	    else
		(*edge)(r, c) = NOEDGE;
	}
    }

    for (r = 0; r < rows; r++) {
	(*edge)(r, 0) = NOEDGE;
	(*edge)(r, cols - 1) = NOEDGE;
    }

    for (c = 0; c < cols; c++) {
	(*edge)(0, c) = NOEDGE;
	(*edge)(rows - 1, c) = NOEDGE;
    }

   /****************************************************************************
   * Compute the histogram of the magnitude image. Then use the histogram to
   * compute hysteresis thresholds.
   ****************************************************************************/
    for (r = 0; r < 32768; r++)
	hist[r] = 0;
    for (r = 0, pos = 0; r < rows; r++) {
	for (c = 0; c < cols; c++, pos++) {
	    if ((*edge)(r, c) == POSSIBLE_EDGE)
		hist[mag[pos]]++;
	}
    }

   /****************************************************************************
   * Compute the number of pixels that passed the nonmaximal suppression.
   ****************************************************************************/
    for (r = 1, numedges = 0; r < 32768; r++) {
	if (hist[r] != 0)
	    maximum_mag = r;
	numedges += hist[r];
    }

    highcount = (int) (numedges * thigh + 0.5);

   /****************************************************************************
   * Compute the high threshold value as the (100 * thigh) percentage point
   * in the magnitude of the gradient histogram of all the pixels that passes
   * non-maximal suppression. Then calculate the low threshold as a fraction
   * of the computed high threshold value. John Canny said in his paper
   * "A Computational Approach to Edge Detection" that "The ratio of the
   * high to low threshold in the implementation is in the range two or three
   * to one." That means that in terms of this implementation, we should
   * choose tlow ~= 0.5 or 0.33333.
   ****************************************************************************/
    r = 1;
    numedges = hist[1];
    while ((r < (maximum_mag - 1)) && (numedges < highcount)) {
	r++;
	numedges += hist[r];
    }
    highthreshold = r;
    lowthreshold = (int) (highthreshold * tlow + 0.5);

   /****************************************************************************
   * This loop looks for pixels above the highthreshold to locate edges and
   * then calls follow_edges to continue the edge.
   ****************************************************************************/
    for (r = 0, pos = 0; r < rows; r++) {
	for (c = 0; c < cols; c++, pos++) {
	    if (((*edge)(r, c) == POSSIBLE_EDGE)
		&& (mag[pos] >= highthreshold)) {
		(*edge)(r, c) = EDGE;
		follow_edges((unsigned char *)(edge->getData() + pos), (mag + pos), lowthreshold,
			     cols);
	    }
	}
    }

   /****************************************************************************
   * Set all the remaining possible edges to non-edges.
   ****************************************************************************/
    for (r = 0; r < rows; r++) {
	for (c = 0; c < cols; c++)
	    if ((*edge)(r, c) != EDGE)
		(*edge)(r, c) = NOEDGE;
    }

    return edge;
}

/*******************************************************************************
* PROCEDURE: non_max_supp
* PURPOSE: This routine applies non-maximal suppression to the magnitude of
* the gradient image.
* NAME: Mike Heath
* DATE: 2/15/96
*******************************************************************************/
GrayImage* nonMaxSupp(
		short *mag,
		short *gradx,
		short *grady,
		int nrows,
		int ncols
)
{
    int rowcount, colcount;
    short *magrowptr, *magptr;
    short *gxrowptr, *gxptr;
    short *gyrowptr, *gyptr, z1, z2;
    short m00, gx = 0, gy = 0;
    float mag1, mag2, xperp = 0.0, yperp = 0.0;
    unsigned char *resultrowptr, *resultptr;

    GrayImage* dstImage = new GrayImage(nrows, ncols);

   /****************************************************************************
   * Suppress non-maximum points.
   ****************************************************************************/
    for (rowcount = 1, magrowptr = mag + ncols + 1, gxrowptr =
	 gradx + ncols + 1, gyrowptr = grady + ncols + 1, resultrowptr =
	 (unsigned char*)dstImage->getData() + ncols + 1; rowcount < nrows - 2;
	 rowcount++, magrowptr += ncols, gyrowptr += ncols, gxrowptr +=
	 ncols, resultrowptr += ncols) {
	for (colcount = 1, magptr = magrowptr, gxptr = gxrowptr, gyptr =
	     gyrowptr, resultptr = resultrowptr; colcount < ncols - 2;
	     colcount++, magptr++, gxptr++, gyptr++, resultptr++) {
	    m00 = *magptr;
	    if (m00 == 0) {
		*resultptr = (unsigned char) NOEDGE;
	    } else {
		xperp = -(gx = *gxptr) / ((float) m00);
		yperp = (gy = *gyptr) / ((float) m00);
	    }

	    if (gx >= 0) {
		if (gy >= 0) {
		    if (gx >= gy) {
			/* 111 */
			/* Left point */
			z1 = *(magptr - 1);
			z2 = *(magptr - ncols - 1);

			mag1 = (m00 - z1) * xperp + (z2 - z1) * yperp;

			/* Right point */
			z1 = *(magptr + 1);
			z2 = *(magptr + ncols + 1);

			mag2 = (m00 - z1) * xperp + (z2 - z1) * yperp;
		    } else {
			/* 110 */
			/* Left point */
			z1 = *(magptr - ncols);
			z2 = *(magptr - ncols - 1);

			mag1 = (z1 - z2) * xperp + (z1 - m00) * yperp;

			/* Right point */
			z1 = *(magptr + ncols);
			z2 = *(magptr + ncols + 1);

			mag2 = (z1 - z2) * xperp + (z1 - m00) * yperp;
		    }
		} else {
		    if (gx >= -gy) {
			/* 101 */
			/* Left point */
			z1 = *(magptr - 1);
			z2 = *(magptr + ncols - 1);

			mag1 = (m00 - z1) * xperp + (z1 - z2) * yperp;

			/* Right point */
			z1 = *(magptr + 1);
			z2 = *(magptr - ncols + 1);

			mag2 = (m00 - z1) * xperp + (z1 - z2) * yperp;
		    } else {
			/* 100 */
			/* Left point */
			z1 = *(magptr + ncols);
			z2 = *(magptr + ncols - 1);

			mag1 = (z1 - z2) * xperp + (m00 - z1) * yperp;

			/* Right point */
			z1 = *(magptr - ncols);
			z2 = *(magptr - ncols + 1);

			mag2 = (z1 - z2) * xperp + (m00 - z1) * yperp;
		    }
		}
	    } else {
		if ((gy = *gyptr) >= 0) {
		    if (-gx >= gy) {
			/* 011 */
			/* Left point */
			z1 = *(magptr + 1);
			z2 = *(magptr - ncols + 1);

			mag1 = (z1 - m00) * xperp + (z2 - z1) * yperp;

			/* Right point */
			z1 = *(magptr - 1);
			z2 = *(magptr + ncols - 1);

			mag2 = (z1 - m00) * xperp + (z2 - z1) * yperp;
		    } else {
			/* 010 */
			/* Left point */
			z1 = *(magptr - ncols);
			z2 = *(magptr - ncols + 1);

			mag1 = (z2 - z1) * xperp + (z1 - m00) * yperp;

			/* Right point */
			z1 = *(magptr + ncols);
			z2 = *(magptr + ncols - 1);

			mag2 = (z2 - z1) * xperp + (z1 - m00) * yperp;
		    }
		} else {
		    if (-gx > -gy) {
			/* 001 */
			/* Left point */
			z1 = *(magptr + 1);
			z2 = *(magptr + ncols + 1);

			mag1 = (z1 - m00) * xperp + (z1 - z2) * yperp;

			/* Right point */
			z1 = *(magptr - 1);
			z2 = *(magptr - ncols - 1);

			mag2 = (z1 - m00) * xperp + (z1 - z2) * yperp;
		    } else {
			/* 000 */
			/* Left point */
			z1 = *(magptr + ncols);
			z2 = *(magptr + ncols + 1);

			mag1 = (z2 - z1) * xperp + (m00 - z1) * yperp;

			/* Right point */
			z1 = *(magptr - ncols);
			z2 = *(magptr - ncols - 1);

			mag2 = (z2 - z1) * xperp + (m00 - z1) * yperp;
		    }
		}
	    }

	    /* Now determine if the current point is a maximum point */

	    if (mag1 > 0.0 || mag2 > 0.0) {
		*resultptr = (unsigned char) NOEDGE;
	    } else {
		if (mag2 == 0.0)
		    *resultptr = (unsigned char) NOEDGE;
		else
		    *resultptr = (unsigned char) POSSIBLE_EDGE;
	    }
	}
    }

    return dstImage;
}
