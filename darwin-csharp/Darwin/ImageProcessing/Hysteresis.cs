/*******************************************************************************
* FILE: hysteresis.c
* This code was re-written by Mike Heath from original code obtained indirectly
* from Michigan State University. heath@csee.usf.edu (Re-written in 1996).
*******************************************************************************/

using Darwin.Extensions;
using System;
using System.Drawing;

namespace Darwin.ImageProcessing
{
    public static class Hysteresis
    {
        private const byte NoEdge = 0;
		private static Color NoEdgePixel = Color.FromArgb(NoEdge, NoEdge, NoEdge);

        private const byte PossibleEdge = 128;
		private static Color PossibleEdgePixel = Color.FromArgb(PossibleEdge, PossibleEdge, PossibleEdge);

        private const byte Edge = 255;
		private static Color EdgePixel = Color.FromArgb(Edge, Edge, Edge);

		/*******************************************************************************
		* PROCEDURE: follow_edges
		* PURPOSE: This procedure edges is a recursive routine that traces edgs along
		* all paths whose magnitude values remain above some specifyable lower
		* threshhold.
		* NAME: Mike Heath
		* DATE: 2/15/96
		*******************************************************************************/
		public static void FollowEdges(ref DirectBitmap bitmap, int edgemapptr, int edgemagptr, short lowval,
				 int cols)
		{
			int tempmagptr;
			int tempmapptr;
			int[] x = new int[] { 1, 1, 0, -1, -1, -1, 0, 1 };
			int[] y = new int[] { 0, 1, 1, 1, 0, -1, -1, -1 };

			for (var i = 0; i < 8; i++)
			{
				tempmapptr = Convert.ToByte(edgemapptr - y[i] * cols + x[i]);
				tempmagptr = Convert.ToByte(edgemagptr - y[i] * cols + x[i]);

				if (bitmap.GetPixelByPosition(tempmapptr).GetIntensity() == PossibleEdge &&
					bitmap.GetPixelByPosition(tempmagptr).GetIntensity() > lowval)
				{
					bitmap.SetPixelByPosition(tempmapptr, EdgePixel);
					FollowEdges(ref bitmap, tempmapptr, tempmagptr, lowval, cols);
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
		public static DirectBitmap ApplyHysteresis(
				short[] mag,
				DirectBitmap nms,
				int rows,
				int cols,
				float tlow,
				float thigh
		)
		{
			int r, c, pos, numedges, highcount, lowthreshold, highthreshold;
			int[] hist = new int[32768];

			short maximum_mag = 0;

			// This GrayImage will hold the edge detection image
			DirectBitmap edge = new DirectBitmap(cols, rows, System.Drawing.Imaging.PixelFormat.Format8bppIndexed);

			/****************************************************************************
			* Initialize the edge map to possible edges everywhere the non-maximal
			* suppression suggested there could be an edge except for the border. At
			* the border we say there can not be an edge because it makes the
			* follow_edges algorithm more efficient to not worry about tracking an
			* edge off the side of the image.
			****************************************************************************/
			for (r = 0; r < rows; r++)
			{
				for (c = 0; c < cols; c++)
				{
					if (nms.GetIntensity(c, r) == PossibleEdge)
						edge.SetPixel(c, r, PossibleEdgePixel);
					else
						edge.SetPixel(c, r, NoEdgePixel);
				}
			}

			for (r = 0; r < rows; r++)
			{
				edge.SetPixel(0, r, NoEdgePixel);
				edge.SetPixel(cols - 1, r, NoEdgePixel);
			}

			for (c = 0; c < cols; c++)
			{
				edge.SetPixel(c, 0, NoEdgePixel);
				edge.SetPixel(c, rows - 1, NoEdgePixel);
			}

			/****************************************************************************
			* Compute the histogram of the magnitude image. Then use the histogram to
			* compute hysteresis thresholds.
			****************************************************************************/
			for (r = 0; r < 32768; r++)
				hist[r] = 0;

			for (r = 0, pos = 0; r < rows; r++)
			{
				for (c = 0; c < cols; c++, pos++)
				{
					if (edge.GetIntensity(c, r) == PossibleEdge)
						hist[mag[pos]]++;
				}
			}

			/****************************************************************************
			* Compute the number of pixels that passed the nonmaximal suppression.
			****************************************************************************/
			for (r = 1, numedges = 0; r < 32768; r++)
			{
				if (hist[r] != 0)
					maximum_mag = (short)r;
				numedges += hist[r];
			}

			highcount = (int)(numedges * thigh + 0.5);

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
			while ((r < (maximum_mag - 1)) && (numedges < highcount))
			{
				r++;
				numedges += hist[r];
			}
			highthreshold = r;
			lowthreshold = (int)(highthreshold * tlow + 0.5);

			/****************************************************************************
			* This loop looks for pixels above the highthreshold to locate edges and
			* then calls follow_edges to continue the edge.
			****************************************************************************/
			for (r = 0, pos = 0; r < rows; r++)
			{
				for (c = 0; c < cols; c++, pos++)
				{
					if ((edge.GetIntensity(c, r) == PossibleEdge)
					&& (mag[pos] >= highthreshold))
					{
						edge.SetPixel(c, r, EdgePixel);
						//FollowEdges((unsigned char *)(edge->getData() + pos), (mag + pos), lowthreshold,
						// cols);
					}
				}
			}

		   /****************************************************************************
		   * Set all the remaining possible edges to non-edges.
		   ****************************************************************************/
			for (r = 0; r<rows; r++)
			{
				for (c = 0; c < cols; c++)
				{
					if (edge.GetIntensity(c, r) != Edge)
					{
						edge.SetPixel(c, r, NoEdgePixel);
					}
				}
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
		public static DirectBitmap NonMaxSupp(
				short[] mag,
				short[] gradx,
				short[] grady,
				int nrows,
				int ncols
		)
		{
			int rowcount, colcount;
			int magrowptr = ncols + 1;
			int magptr;
			int gxrowptr = ncols + 1;
			int gxptr;
			int gyrowptr = ncols + 1;
			int gyptr;
			int z1, z2;
			int m00, gx = 0, gy = 0;
			float mag1, mag2, xperp = 0.0f, yperp = 0.0f;
			int resultrowptr = ncols + 1;
			int resultptr;

			DirectBitmap dstImage = new DirectBitmap(ncols, nrows, System.Drawing.Imaging.PixelFormat.Format8bppIndexed);

			/****************************************************************************
			* Suppress non-maximum points.
			****************************************************************************/
			for (rowcount = 1; rowcount < nrows - 2;
				rowcount++, magrowptr += (short)ncols, gyrowptr += (short)ncols, gxrowptr += (short)ncols, resultrowptr += ncols) {

				for (colcount = 1, magptr = magrowptr, gxptr = gxrowptr, gyptr =
					 gyrowptr, resultptr = resultrowptr; colcount < ncols - 2;
					 colcount++, magptr++, gxptr++, gyptr++, resultptr++)
				{
					m00 = mag[magptr];
					if (m00 == 0)
					{
						dstImage.SetPixelByPosition(resultptr, NoEdgePixel);
					}
					else
					{
						xperp = -(gx = gradx[gxptr]) / ((float)m00);
						yperp = (gy = grady[gyptr]) / ((float)m00);
					}

					if (gx >= 0)
					{
						if (gy >= 0)
						{
							if (gx >= gy)
							{
								/* 111 */
								/* Left point */
								z1 = mag[magptr - 1];
								z2 = mag[magptr - ncols - 1];

								mag1 = (m00 - z1) * xperp + (z2 - z1) * yperp;

								/* Right point */
								z1 = mag[magptr + 1];
								z2 = mag[magptr + ncols + 1];

								mag2 = (m00 - z1) * xperp + (z2 - z1) * yperp;
							}
							else
							{
								/* 110 */
								/* Left point */
								z1 = mag[magptr - ncols];
								z2 = mag[magptr - ncols - 1];

								mag1 = (z1 - z2) * xperp + (z1 - m00) * yperp;

								/* Right point */
								z1 = mag[magptr + ncols];
								z2 = mag[magptr + ncols + 1];

								mag2 = (z1 - z2) * xperp + (z1 - m00) * yperp;
							}
						}
						else
						{
							if (gx >= -gy)
							{
								/* 101 */
								/* Left point */
								z1 = mag[magptr - 1];
								z2 = mag[magptr + ncols - 1];

								mag1 = (m00 - z1) * xperp + (z1 - z2) * yperp;

								/* Right point */
								z1 = mag[magptr + 1];
								z2 = mag[magptr - ncols + 1];

								mag2 = (m00 - z1) * xperp + (z1 - z2) * yperp;
							}
							else
							{
								/* 100 */
								/* Left point */
								z1 = mag[magptr + ncols];
								z2 = mag[magptr + ncols - 1];

								mag1 = (z1 - z2) * xperp + (m00 - z1) * yperp;

								/* Right point */
								z1 = mag[magptr - ncols];
								z2 = mag[magptr - ncols + 1];

								mag2 = (z1 - z2) * xperp + (m00 - z1) * yperp;
							}
						}
					}
					else
					{
						if ((gy = grady[gyptr]) >= 0)
						{
							if (-gx >= gy)
							{
								/* 011 */
								/* Left point */
								z1 = mag[magptr + 1];
								z2 = mag[magptr - ncols + 1];

								mag1 = (z1 - m00) * xperp + (z2 - z1) * yperp;

								/* Right point */
								z1 = mag[magptr - 1];
								z2 = mag[magptr + ncols - 1];

								mag2 = (z1 - m00) * xperp + (z2 - z1) * yperp;
							}
							else
							{
								/* 010 */
								/* Left point */
								z1 = mag[magptr - ncols];
								z2 = mag[magptr - ncols + 1];

								mag1 = (z2 - z1) * xperp + (z1 - m00) * yperp;

								/* Right point */
								z1 = mag[magptr + ncols];
								z2 = mag[magptr + ncols - 1];

								mag2 = (z2 - z1) * xperp + (z1 - m00) * yperp;
							}
						}
						else
						{
							if (-gx > -gy)
							{
								/* 001 */
								/* Left point */
								z1 = mag[magptr + 1];
								z2 = mag[magptr + ncols + 1];

								mag1 = (z1 - m00) * xperp + (z1 - z2) * yperp;

								/* Right point */
								z1 = mag[magptr - 1];
								z2 = mag[magptr - ncols - 1];

								mag2 = (z1 - m00) * xperp + (z1 - z2) * yperp;
							}
							else
							{
								/* 000 */
								/* Left point */
								z1 = mag[magptr + ncols];
								z2 = mag[magptr + ncols + 1];

								mag1 = (z2 - z1) * xperp + (m00 - z1) * yperp;

								/* Right point */
								z1 = mag[magptr - ncols];
								z2 = mag[magptr - ncols - 1];

								mag2 = (z2 - z1) * xperp + (m00 - z1) * yperp;
							}
						}
					}

					/* Now determine if the current point is a maximum point */

					if (mag1 > 0.0 || mag2 > 0.0)
					{
						dstImage.SetPixelByPosition(resultptr, NoEdgePixel);
					}
					else
					{
						if (mag2 == 0.0)
							dstImage.SetPixelByPosition(resultptr, NoEdgePixel);
						else
							dstImage.SetPixelByPosition(resultptr, PossibleEdgePixel);
					}
				}
			}

			return dstImage;
		}
    }
}
