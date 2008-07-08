//*******************************************************************
//   file: edge_detect.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (7/25/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

// The canny edge detection code in this file is based on Mike Heath's
// code.  His comments follow:

/*******************************************************************************
* PROGRAM: canny_edge
* PURPOSE: This program implements a "Canny" edge detector. The processing
* steps are as follows:
*
*   1) Convolve the image with a separable gaussian filter.
*   2) Take the dx and dy the first derivatives using [-1,0,1] and [1,0,-1]'.
*   3) Compute the magnitude: sqrt(dx*dx+dy*dy).
*   4) Perform non-maximal suppression.
*   5) Perform hysteresis.
*
* The user must input three parameters. These are as follows:
*
*   sigma = The standard deviation of the gaussian smoothing filter.
*   tlow  = Specifies the low value to use in hysteresis. This is a 
*           fraction (0-1) of the computed high threshold edge strength value.
*   thigh = Specifies the high value to use in hysteresis. This fraction (0-1)
*           specifies the percentage point in a histogram of the gradient of
*           the magnitude. Magnitude values of zero are not counted in the
*           histogram.
*
* NAME: Mike Heath
*       Computer Vision Laboratory
*       University of South Floeida
*       heath@csee.usf.edu
*
* DATE: 2/15/96
*
* Modified: 5/17/96 - To write out a floating point RAW headerless file of
*                     the edge gradient "up the edge" where the angle is
*                     defined in radians counterclockwise from the x direction.
*                     (Mike Heath)
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include <fstream> 
#include "edge_detect.h"
#include "hysteresis.h"
#include "../utility.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef DEBUG
#include <iostream>
#endif

#define BOOSTBLURFACTOR 90.0

///////////////////////////
// Local utility functions

void radian_direction(short int *delta_x, short int *delta_y, int rows,
		      int cols, float **dir_radians, int xdirtag,
		      int ydirtag);

double angle_radians(double x, double y);

void derivative_x_y(short int *smoothedim, int rows, int cols,
		    short int **delta_x, short int **delta_y);

void magnitude_x_y(short int *delta_x, short int *delta_y, int rows,
		   int cols, short int **magnitude);

void gaussian_smooth(const GrayImage * image, float sigma,
		     short int **smoothedim);

void make_gaussian_kernel(float sigma, float **kernel, int *windowsize);

GrayImage *convertMagnitude(short int **magnitude, int rows, int cols);


//*******************************************************************
// 
// edgeDetect_sobel
//
//    This is based on Mark Allen's implementation from the original Darwin.
//
GrayImage *edgeDetect_sobel(
		const GrayImage * srcImage,	// The image to be edge detected
		GrayImage ** directionImage,	// Direction result image, if
			    			// the caller would like to keep
			    			// it around
		bool saveDirImage
)
{
    int magnitude;
    double arctangent;		/*holds the value of the arctangent */
    int tempx, tempy;
    double denompiover2 = .6366198;	/*one divided by pi over 2 */

    unsigned numRows = srcImage->getNumRows();
    unsigned numCols = srcImage->getNumCols();

    GrayImage *dstImage = new GrayImage(numRows, numCols);	// Where we're going to put the edge
    							   	// magnitude image
   
    if (saveDirImage)
	    *directionImage = new GrayImage(numRows, numCols);

    for (unsigned r = 1; r < numRows - 1; r++) {
	for (unsigned c = 1; c < numCols - 1; c++) {
	    tempx =
		 -1 * (*srcImage)(r - 1, c - 1)
		 + -2 * (*srcImage)(r - 1, c)
		 + -1 * (*srcImage)(r - 1, c + 1)
		 + 1 * (*srcImage)(r + 1, c - 1)
		 + 2 * (*srcImage)(r + 1, c)
		 + 1 * (*srcImage)(r + 1, c + 1);

	    tempy =
		-1 * (*srcImage)(r - 1, c - 1)
		+ -2 * (*srcImage)(r, c - 1)
		+ -1 * (*srcImage)(r + 1, c - 1)
		+ 1 * (*srcImage)(r - 1, c + 1)
	       	+ 2 * (*srcImage)(r, c + 1)
		+ 1 * (*srcImage)(r + 1, c + 1);

	    magnitude = abs(tempx) + abs(tempy);
	    
	    if (tempy == 0)	// Make sure we're not going to be dividing by zero
		tempy = 1;
	    if (tempx == 0)
		tempx = 1;

	    arctangent = atan2((double) tempy, (double) tempx);	// Arc tangent for the direction calculation

	    if (saveDirImage) {
	    	if (arctangent >= 0)
			(**directionImage)(r, c) =
		 	   	(unsigned char) ceil(arctangent * denompiover2 * 128);
	    	else if (arctangent < 0)
			(**directionImage)(r, c) =
			    (unsigned char) ceil(((PI + arctangent) * denompiover2) * 128);
	    }

	    if (magnitude >= 255)
		(*dstImage)(r, c) = 255;
	    else
		(*dstImage)(r, c) = magnitude;
	}
    }

    return dstImage;
}


/*******************************************************************************
* PROCEDURE: canny
* PURPOSE: To perform canny edge detection.
* NAME: Mike Heath
* DATE: 2/15/96
*******************************************************************************/
GrayImage *edgeDetect_canny(const GrayImage * image,
			    GrayImage **magImage,
			    bool saveMagImage,
			    float sigma,
			    float tlow,
			    float thigh,
			    char *fname)
{
    FILE *fpdir = NULL;		/* File to write the gradient image to.     */
    short int *smoothedim,	/* The image after gaussian smoothing.      */
    *delta_x,			/* The first devivative image, x-direction. */
    *delta_y,			/* The first derivative image, y-direction. */
    *magnitude;			/* The magnitude of the gadient image.      */
    float *dir_radians = NULL;	/* Gradient direction image.                */
    unsigned num_rows = image->getNumRows();
    unsigned num_cols = image->getNumCols();

   /****************************************************************************
   * Perform gaussian smoothing on the image using the input standard
   * deviation.
   ****************************************************************************/
    gaussian_smooth(image, sigma, &smoothedim);

   /****************************************************************************
   * Compute the first derivative in the x and y directions.
   ****************************************************************************/
    derivative_x_y(smoothedim, num_rows, num_cols, &delta_x, &delta_y);

   /****************************************************************************
   * This option to write out the direction of the edge gradient was added
   * to make the information available for computing an edge quality figure
   * of merit.
   ****************************************************************************/
    if (fname != NULL) {
      /*************************************************************************
      * Compute the direction up the gradient, in radians that are
      * specified counteclockwise from the positive x-axis.
      *************************************************************************/
	radian_direction(delta_x, delta_y, num_rows, num_cols,
			 &dir_radians, -1, -1);

      /*************************************************************************
      * Write the gradient direction image out to a file.
      *************************************************************************/
	if ((fpdir = fopen(fname, "wb")) == NULL) {
	    fprintf(stderr, "Error opening the file %s for writing.\n",
		    fname);
	    exit(1);
	}
	fwrite(dir_radians, sizeof(float), num_rows * num_cols, fpdir);
	fclose(fpdir);
	free(dir_radians);
    }

   /****************************************************************************
   * Compute the magnitude of the gradient.
   ****************************************************************************/
    magnitude_x_y(delta_x, delta_y, num_rows, num_cols, &magnitude);

    if (saveMagImage)
    	*magImage = convertMagnitude(&magnitude, num_rows, num_cols);

   /****************************************************************************
   * Perform non-maximal suppression.
   ****************************************************************************/
    GrayImage *nms =
	nonMaxSupp(magnitude, delta_x, delta_y, num_rows, num_cols);

   /****************************************************************************
   * Use hysteresis to mark the edge pixels.
   ****************************************************************************/
    GrayImage *edge =
	applyHysteresis(magnitude, nms, num_rows, num_cols, tlow, thigh);

   /****************************************************************************
   * Free all of the memory that we allocated except for the edge image that
   * is still being used to store out result.
   ****************************************************************************/
    free(smoothedim);
    free(delta_x);
    free(delta_y);
    free(magnitude);
    delete nms;

    return edge;
}


/*******************************************************************************
* Procedure: radian_direction
* Purpose: To compute a direction of the gradient image from component dx and
* dy images. Because not all derriviatives are computed in the same way, this
* code allows for dx or dy to have been calculated in different ways.
*
* FOR X:  xdirtag = -1  for  [-1 0  1]
*         xdirtag =  1  for  [ 1 0 -1]
*
* FOR Y:  ydirtag = -1  for  [-1 0  1]'
*         ydirtag =  1  for  [ 1 0 -1]'
*
* The resulting angle is in radians measured counterclockwise from the
* xdirection. The angle points "up the gradient".
*******************************************************************************/
void radian_direction(short int *delta_x, short int *delta_y, int rows,
		      int cols, float **dir_radians, int xdirtag,
		      int ydirtag)
{
    int r, c, pos;
    float *dirim = NULL;
    double dx, dy;

   /****************************************************************************
   * Allocate an image to store the direction of the gradient.
   ****************************************************************************/
    if ((dirim = (float *) calloc(rows * cols, sizeof(float))) == NULL) {
	fprintf(stderr,
		"Error allocating the gradient direction image.\n");
	exit(1);
    }
    *dir_radians = dirim;

    for (r = 0, pos = 0; r < rows; r++) {
	for (c = 0; c < cols; c++, pos++) {
	    dx = (double) delta_x[pos];
	    dy = (double) delta_y[pos];

	    if (xdirtag == 1)
		dx = -dx;
	    if (ydirtag == -1)
		dy = -dy;

	    dirim[pos] = (float) angle_radians(dx, dy);
	}
    }
}


/*******************************************************************************
* FUNCTION: angle_radians
* PURPOSE: This procedure computes the angle of a vector with components x and
* y. It returns this angle in radians with the answer being in the range
* 0 <= angle <2*PI.
*******************************************************************************/
double angle_radians(double x, double y)
{
    double xu, yu, ang;

    xu = fabs(x);
    yu = fabs(y);

    if ((xu == 0) && (yu == 0))
	return (0);

    ang = atan(yu / xu);

    if (x >= 0) {
	if (y >= 0)
	    return (ang);
	else
	    return (2 * PI - ang);
    } else {
	if (y >= 0)
	    return (PI - ang);
	else
	    return (PI + ang);
    }
}


/*******************************************************************************
* PROCEDURE: magnitude_x_y
* PURPOSE: Compute the magnitude of the gradient. This is the square root of
* the sum of the squared derivative values.
* NAME: Mike Heath
* DATE: 2/15/96
*******************************************************************************/
void magnitude_x_y(short int *delta_x, short int *delta_y, int rows,
		   int cols, short int **magnitude)
{
    int r, c, pos, sq1, sq2;

   /****************************************************************************
   * Allocate an image to store the magnitude of the gradient.
   ****************************************************************************/
    if ((*magnitude = (short *) calloc(rows * cols, sizeof(short))) ==
	NULL) {
	fprintf(stderr, "Error allocating the magnitude image.\n");
	exit(1);
    }

    for (r = 0, pos = 0; r < rows; r++) {
	for (c = 0; c < cols; c++, pos++) {
	    sq1 = (int) delta_x[pos] * (int) delta_x[pos];
	    sq2 = (int) delta_y[pos] * (int) delta_y[pos];
	    (*magnitude)[pos] = (short) (0.5 + sqrt((float) sq1 + (float) sq2));
	}
    }

}

//*******************************************************************
//
// GrayImage *convertMagnitude(short int **magnitude, int rows, int cols)
//
//    Converts a magnitude representation (which is an array of shorts) to
//    a GrayImage which is made up of bytes.
//
GrayImage *convertMagnitude(short int **magnitude, int rows, int cols)
{
    GrayImage *dstImage = new GrayImage(rows, cols);

    int pos, r, c;
    short int max = (*magnitude)[0];
    short int min = max;

    for (r = 0, pos = 0; r < rows; r++) {
	for (c = 0; c < cols; c++, pos++) {
	    if ((*magnitude)[pos] < min)
		min = (*magnitude)[pos];
	    if ((*magnitude)[pos] > max)
		max = (*magnitude)[pos];
	}
    }

    float scaleFactor = (float) 256 / (max - min);

    for (r = 0, pos = 0; r < rows; r++)
	for (c = 0; c < cols; c++, pos++)
	    
		(*dstImage) (r,
			     c).
		setIntensity((byte)
			     round((*magnitude)[pos] * scaleFactor));

    return dstImage;
}


/*******************************************************************************
* PROCEDURE: derivative_x_y
* PURPOSE: Compute the first derivative of the image in both the x any y
* directions. The differential filters that are used are:
*
*                                          -1
*         dx =  -1 0 +1     and       dy =  0
*                                          +1
*
* NAME: Mike Heath
* DATE: 2/15/96
*******************************************************************************/
void derivative_x_y(short int *smoothedim, int rows, int cols,
		    short int **delta_x, short int **delta_y)
{
    int r, c, pos;

   /****************************************************************************
   * Allocate images to store the derivatives.
   ****************************************************************************/
    if (((*delta_x) = (short *) calloc(rows * cols, sizeof(short))) ==
	NULL) {
	fprintf(stderr, "Error allocating the delta_x image.\n");
	exit(1);
    }
    if (((*delta_y) = (short *) calloc(rows * cols, sizeof(short))) ==
	NULL) {
	fprintf(stderr, "Error allocating the delta_x image.\n");
	exit(1);
    }

   /****************************************************************************
   * Compute the x-derivative. Adjust the derivative at the borders to avoid
   * losing pixels.
   ****************************************************************************/
    for (r = 0; r < rows; r++) {
	pos = r * cols;
	(*delta_x)[pos] = smoothedim[pos + 1] - smoothedim[pos];
	pos++;
	for (c = 1; c < (cols - 1); c++, pos++) {
	    (*delta_x)[pos] = smoothedim[pos + 1] - smoothedim[pos - 1];
	}
	(*delta_x)[pos] = smoothedim[pos] - smoothedim[pos - 1];
    }

   /****************************************************************************
   * Compute the y-derivative. Adjust the derivative at the borders to avoid
   * losing pixels.
   ****************************************************************************/
    for (c = 0; c < cols; c++) {
	pos = c;
	(*delta_y)[pos] = smoothedim[pos + cols] - smoothedim[pos];
	pos += cols;
	for (r = 1; r < (rows - 1); r++, pos += cols) {
	    (*delta_y)[pos] = smoothedim[pos + cols] - smoothedim[pos - cols];
	}
	(*delta_y)[pos] = smoothedim[pos] - smoothedim[pos - cols];
    }
}


/*******************************************************************************
* PROCEDURE: gaussian_smooth
* PURPOSE: Blur an image with a gaussian filter.
* NAME: Mike Heath
* DATE: 2/15/96
*******************************************************************************/
void gaussian_smooth(const GrayImage * image, float sigma,
		     short int **smoothedim)
{
    int r, c, rr, cc,		/* Counter variables. */
     windowsize,		/* Dimension of the gaussian kernel. */
     center;			/* Half of the windowsize. */
    float *tempim,		/* Buffer for separable filter gaussian smoothing. */
    *kernel,			/* A one dimensional gaussian kernel. */
     dot,			/* Dot product summing variable. */
     sum;			/* Sum of the kernel weights variable. */

    unsigned nRows = image->getNumRows();
    unsigned nCols = image->getNumCols();

   /****************************************************************************
   * Create a 1-dimensional gaussian smoothing kernel.
   ****************************************************************************/
    make_gaussian_kernel(sigma, &kernel, &windowsize);
    center = windowsize / 2;

   /****************************************************************************
   * Allocate a temporary buffer image and the smoothed image.
   ****************************************************************************/
    if ((tempim = (float *) calloc(nRows * nCols, sizeof(float))) == NULL) {
	fprintf(stderr, "Error allocating the buffer image.\n");
	exit(1);
    }
    if (((*smoothedim) = (short int *) calloc(nRows * nCols,
					      sizeof(short int))) == NULL) {
	fprintf(stderr, "Error allocating the smoothed image.\n");
	exit(1);
    }

   /****************************************************************************
   * Blur in the x - direction.
   ****************************************************************************/
    for (r = 0; r < (int) nRows; r++) {
	for (c = 0; c < (int) nCols; c++) {
	    dot = 0.0;
	    sum = 0.0;
	    for (cc = (-center); cc <= center; cc++) {
		if (((c + cc) >= 0) && ((c + cc) < (int) nCols)) {
		    dot +=
			(float) (*image) (r,
					  c +
					  cc).getIntensity() *
			kernel[center + cc];
		    sum += kernel[center + cc];
		}
	    }
	    tempim[r * nCols + c] = dot / sum;
	}
    }

   /****************************************************************************
   * Blur in the y - direction.
   ****************************************************************************/
    for (c = 0; c < (int) nCols; c++) {
	for (r = 0; r < (int) nRows; r++) {
	    sum = 0.0;
	    dot = 0.0;
	    for (rr = (-center); rr <= center; rr++) {
		if (((r + rr) >= 0) && ((r + rr) < (int) nRows)) {
		    dot +=
			tempim[(r + rr) * nCols + c] * kernel[center + rr];
		    sum += kernel[center + rr];
		}
	    }

	    (*smoothedim)[r * nCols + c] =
		(short int) (dot * BOOSTBLURFACTOR / sum + 0.5);
	}
    }

    free(tempim);
    free(kernel);
}


/*******************************************************************************
* PROCEDURE: make_gaussian_kernel
* PURPOSE: Create a one dimensional gaussian kernel.
* NAME: Mike Heath
* DATE: 2/15/96
*******************************************************************************/
void make_gaussian_kernel(float sigma, float **kernel, int *windowsize)
{
    int i, center;
    float x, fx, sum = 0.0;

    *windowsize = 1 + 2 * (int) ceil(2.5 * sigma);
    center = (*windowsize) / 2;

    if ((*kernel = (float *) calloc((*windowsize), sizeof(float))) == NULL) {
	fprintf(stderr, "Error callocing the gaussian kernel array.\n");
	exit(1);
    }

    for (i = 0; i < (*windowsize); i++) {
	x = (float) (i - center);
	fx =
	    pow(2.71828,
		-0.5 * x * x / (sigma * sigma)) / (sigma *
						   sqrt(6.2831853));
	(*kernel)[i] = fx;
	sum += fx;
    }

    for (i = 0; i < (*windowsize); i++)
	(*kernel)[i] /= sum;
}
