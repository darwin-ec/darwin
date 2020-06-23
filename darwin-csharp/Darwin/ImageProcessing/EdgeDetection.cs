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
*       University of South Florida
*       heath@csee.usf.edu
*
* DATE: 2/15/96
*
* Modified: 5/17/96 - To write out a floating point RAW headerless file of
*                     the edge gradient "up the edge" where the angle is
*                     defined in radians counterclockwise from the x direction.
*                     (Mike Heath)
*******************************************************************************/

using Darwin.Extensions;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace Darwin.ImageProcessing
{
    public class EdgeDetection
    {
        const float BoostBlurFactor = 90.0f;

        /*******************************************************************************
        * PROCEDURE: canny
        * PURPOSE: To perform canny edge detection.
        * NAME: Mike Heath
        * DATE: 2/15/96
        *******************************************************************************/
        public static DirectBitmap CannyEdgeDetection(DirectBitmap image,
                        out DirectBitmap magImage,
                        bool saveMagImage,
                        float sigma,
                        float tlow,
                        float thigh)
        {
            short[] smoothedim;	/* The image after gaussian smoothing.      */
            short[] delta_x;			/* The first devivative image, x-direction. */
            short[] delta_y;			/* The first derivative image, y-direction. */
            // float[] dir_radians;	/* Gradient direction image.                */
            int num_rows = image.Height;
            int num_cols = image.Width;

            /****************************************************************************
            * Perform gaussian smoothing on the image using the input standard
            * deviation.
            ****************************************************************************/
            smoothedim = GaussianSmooth(image, sigma);

            /****************************************************************************
            * Compute the first derivative in the x and y directions.
            ****************************************************************************/
            DerivativeXY(smoothedim, num_rows, num_cols, out delta_x, out delta_y);


            ///****************************************************************************
            //* This option to write out the direction of the edge gradient was added
            //* to make the information available for computing an edge quality figure
            //* of merit.
            //****************************************************************************/
            //if (fname != NULL)
            //{
            //    /*************************************************************************
            //    * Compute the direction up the gradient, in radians that are
            //    * specified counteclockwise from the positive x-axis.
            //    *************************************************************************/
            //    radian_direction(delta_x, delta_y, num_rows, num_cols,
            //             &dir_radians, -1, -1);

            //    /*************************************************************************
            //    * Write the gradient direction image out to a file.
            //    *************************************************************************/
            //    if ((fpdir = fopen(fname, "wb")) == NULL)
            //    {
            //        fprintf(stderr, "Error opening the file %s for writing.\n",
            //            fname);
            //        exit(1);
            //    }
            //    fwrite(dir_radians, sizeof(float), num_rows * num_cols, fpdir);
            //    fclose(fpdir);
            //    free(dir_radians);
            //}

            /****************************************************************************
            * Compute the magnitude of the gradient.
            ****************************************************************************/
            /* The magnitude of the gadient image.      */
            short[] magnitude = MagnitudeXY(delta_x, delta_y, num_rows, num_cols);

            if (saveMagImage)
                magImage = ConvertMagnitude(magnitude, num_rows, num_cols);
            else
                magImage = null;

            /****************************************************************************
            * Perform non-maximal suppression.
            ****************************************************************************/
            DirectBitmap nms = Hysteresis.NonMaxSupp(magnitude, delta_x, delta_y, num_rows, num_cols);

            /****************************************************************************
            * Use hysteresis to mark the edge pixels.
            ****************************************************************************/
            DirectBitmap edge = Hysteresis.ApplyHysteresis(magnitude, nms, num_rows, num_cols, tlow, thigh);

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
        private static float[] RadianDirection(short[] delta_x, short[] delta_y, int rows,
                        int cols, int xdirtag,
                        int ydirtag)
        {
            int r, c, pos;
            double dx, dy;

            /****************************************************************************
            * Allocate an image to store the direction of the gradient.
            ****************************************************************************/
            float[] dir_radians = new float[rows * cols];

            for (r = 0, pos = 0; r < rows; r++)
            {
                for (c = 0; c < cols; c++, pos++)
                {
                    dx = (double)delta_x[pos];
                    dy = (double)delta_y[pos];

                    if (xdirtag == 1)
                        dx = -dx;
                    if (ydirtag == -1)
                        dy = -dy;

                    dir_radians[pos] = (float)AngleRadians(dx, dy);
                }
            }

            return dir_radians;
        }


        /*******************************************************************************
        * FUNCTION: angle_radians
        * PURPOSE: This procedure computes the angle of a vector with components x and
        * y. It returns this angle in radians with the answer being in the range
        * 0 <= angle <2*PI.
        *******************************************************************************/
        private static double AngleRadians(double x, double y)
        {
            double xu, yu, ang;

            xu = Math.Abs(x);
            yu = Math.Abs(y);

            if ((xu == 0) && (yu == 0))
                return (0);

            ang = Math.Atan(yu / xu);

            if (x >= 0)
            {
                if (y >= 0)
                    return (ang);
                else
                    return (2 * Math.PI - ang);
            }
            else
            {
                if (y >= 0)
                    return (Math.PI - ang);
                else
                    return (Math.PI + ang);
            }
        }


        /*******************************************************************************
        * PROCEDURE: magnitude_x_y
        * PURPOSE: Compute the magnitude of the gradient. This is the square root of
        * the sum of the squared derivative values.
        * NAME: Mike Heath
        * DATE: 2/15/96
        *******************************************************************************/
        private static short[] MagnitudeXY(short[] delta_x, short[] delta_y, int rows,
                    int cols)
        {
            int r, c, pos, sq1, sq2;

            /****************************************************************************
            * Allocate an image to store the magnitude of the gradient.
            ****************************************************************************/
            short[] magnitude = new short[rows * cols];

            for (r = 0, pos = 0; r < rows; r++)
            {
                for (c = 0; c < cols; c++, pos++)
                {
                    sq1 = (int)delta_x[pos] * (int)delta_x[pos];
                    sq2 = (int)delta_y[pos] * (int)delta_y[pos];
                    magnitude[pos] = (short)(0.5 + Math.Sqrt((float)sq1 + (float)sq2));
                }
            }

            return magnitude;
        }

        //*******************************************************************
        //
        // GrayImage *convertMagnitude(short int **magnitude, int rows, int cols)
        //
        //    Converts a magnitude representation (which is an array of shorts) to
        //    a GrayImage which is made up of bytes.
        //
        private static DirectBitmap ConvertMagnitude(short[] magnitude, int rows, int cols)
        {
            DirectBitmap dstImage = new DirectBitmap(cols, rows, System.Drawing.Imaging.PixelFormat.Format8bppIndexed);

            int pos, r, c;
            int max = magnitude[0];
            int min = max;

            for (r = 0, pos = 0; r < rows; r++)
            {
                for (c = 0; c < cols; c++, pos++)
                {
                    if (magnitude[pos] < min)
                        min = magnitude[pos];
                    if (magnitude[pos] > max)
                        max = magnitude[pos];
                }
            }

            float scaleFactor = (float)256 / (max - min);

            for (r = 0, pos = 0; r < rows; r++)
            {
                for (c = 0; c < cols; c++, pos++)
                {
                    var rawValue = Math.Round(magnitude[pos] * scaleFactor);

                    // We'll occassionally round to +1 or -1 of byte value range
                    byte value = 0;
                    if (rawValue <= Byte.MinValue)
                        value = Byte.MinValue;
                    else if (rawValue >= Byte.MaxValue)
                        value = Byte.MaxValue;
                    else
                        value = (byte)rawValue;

                    dstImage.SetPixel(c, r, Color.FromArgb(value, value, value));
                }
            }

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
        private static void DerivativeXY(short[] smoothedim, int rows, int cols,
                    out short[] delta_x, out short[] delta_y)
        {
            int r, c, pos;

            /****************************************************************************
            * Allocate images to store the derivatives.
            ****************************************************************************/
            delta_x = new short[rows * cols];
            delta_y = new short[rows * cols];

            /****************************************************************************
            * Compute the x-derivative. Adjust the derivative at the borders to avoid
            * losing pixels.
            ****************************************************************************/
            for (r = 0; r < rows; r++)
            {
                pos = r * cols;
                delta_x[pos] = Convert.ToInt16(smoothedim[pos + 1] - smoothedim[pos]);
                pos++;
                for (c = 1; c < (cols - 1); c++, pos++)
                {
                    delta_x[pos] = Convert.ToInt16(smoothedim[pos + 1] - smoothedim[pos - 1]);
                }
            delta_x[pos] = Convert.ToInt16(smoothedim[pos] - smoothedim[pos - 1]);
            }

            /****************************************************************************
            * Compute the y-derivative. Adjust the derivative at the borders to avoid
            * losing pixels.
            ****************************************************************************/
            for (c = 0; c < cols; c++)
            {
                pos = c;
                delta_y[pos] = Convert.ToInt16(smoothedim[pos + cols] - smoothedim[pos]);
                pos += cols;
                for (r = 1; r < (rows - 1); r++, pos += cols)
                {
                    delta_y[pos] = Convert.ToInt16(smoothedim[pos + cols] - smoothedim[pos - cols]);
                }
            delta_y[pos] = Convert.ToInt16(smoothedim[pos] - smoothedim[pos - cols]);
            }
        }


        /*******************************************************************************
        * PROCEDURE: gaussian_smooth
        * PURPOSE: Blur an image with a gaussian filter.
        * NAME: Mike Heath
        * DATE: 2/15/96
        *******************************************************************************/
        private static short[] GaussianSmooth(DirectBitmap image, float sigma)
        {
            int r, c, rr, cc,       /* Counter variables. */
                windowsize,     /* Dimension of the gaussian kernel. */
                center;            /* Half of the windowsize. */
            float[] tempim;      /* Buffer for separable filter gaussian smoothing. */
            float dot;			/* Dot product summing variable. */
            float sum;          /* Sum of the kernel weights variable. */

            int nRows = image.Height;
            int nCols = image.Width;

            /****************************************************************************
            * Create a 1-dimensional gaussian smoothing kernel.
            ****************************************************************************/
            /* A one dimensional gaussian kernel. */
            float[] kernel = MakeGaussianKernel(sigma, out windowsize);
            center = windowsize / 2;

            /****************************************************************************
            * Allocate a temporary buffer image and the smoothed image.
            ****************************************************************************/
            tempim = new float[nRows * nCols];
            short[] smoothedim = new short[nRows * nCols];

            /****************************************************************************
            * Blur in the x - direction.
            ****************************************************************************/
            for (r = 0; r < (int)nRows; r++)
            {
                for (c = 0; c < (int)nCols; c++)
                {
                    dot = 0.0f;
                    sum = 0.0f;
                    for (cc = (-center); cc <= center; cc++)
                    {
                        if (((c + cc) >= 0) && ((c + cc) < (int)nCols))
                        {
                            dot += (float)image.GetIntensity(c + cc, r) * kernel[center + cc];
                            sum += kernel[center + cc];
                        }
                    }
                    tempim[r * nCols + c] = dot / sum;
                }
            }

            /****************************************************************************
            * Blur in the y - direction.
            ****************************************************************************/
            for (c = 0; c < (int)nCols; c++)
            {
                for (r = 0; r < (int)nRows; r++)
                {
                    sum = 0.0f;
                    dot = 0.0f;
                    for (rr = (-center); rr <= center; rr++)
                    {
                        if (((r + rr) >= 0) && ((r + rr) < (int)nRows))
                        {
                            dot += tempim[(r + rr) * nCols + c] * kernel[center + rr];
                            sum += kernel[center + rr];
                        }
                    }

                    smoothedim[r * nCols + c] = Convert.ToInt16(dot * BoostBlurFactor / sum + 0.5);
                }
            }
            return smoothedim;
        }

        /*******************************************************************************
        * PROCEDURE: make_gaussian_kernel
        * PURPOSE: Create a one dimensional gaussian kernel.
        * NAME: Mike Heath
        * DATE: 2/15/96
        *******************************************************************************/
        private static float[] MakeGaussianKernel(float sigma, out int windowsize)
        {
            int i, center;
            float x, fx, sum = 0.0f;

            windowsize = 1 + 2 * (int)Math.Ceiling(2.5 * sigma);
            center = windowsize / 2;

            float[] kernel = new float[windowsize];

            for (i = 0; i < windowsize; i++)
            {
                x = (float)(i - center);
                fx = (float)(Math.Pow(2.71828, -0.5 * x * x / (sigma * sigma)) / (sigma * Math.Sqrt(6.2831853)));
                kernel[i] = fx;
                sum += fx;
            }

            for (i = 0; i < windowsize; i++)
                kernel[i] /= sum;

            return kernel;
        }
    }
}
