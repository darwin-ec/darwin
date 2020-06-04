/* Tcl Wavelet Laboratory
 * Redundant (Non-subsampled) Wavelet transform operations
 *
 * Mike Hilton, 30 July 1996
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace Darwin.Wavelet
{
    public static class WIRWavelet
    {
		/* DILATE_FILTER
		 *
		 * Dilates the input vector SOURCE by INDEX, storing the result in DEST.  
		 * The length of the SOURCE vector is LENGTH.
		 */
		public static void Dilate_filter(double[] source, ref double[] dest, int length, int index)
		{
			int i, j, k, power;
			power = WaveletUtil.WL_pow2(index);

			for (i = 0, j = 0; i < length; i++)
			{
				dest[j++] = source[i];
				for (k = 1; k < power; k++)
					dest[j++] = (double)0;
			}
		}

		/********************************************************************
		 *
		 * Redundant Wavelet Transforms for Vectors
		 *
		 *******************************************************************/

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
		public static int WL_FrwtVector(
			double[] source,
			ref double[,] dest,
			int length,
			int levels,
			WL_Filter lowpass,
			WL_Filter hipass
		)
		{
			double[] temp;    /* extended source vector */
			int i, j, level, left, right, pow2;
			int hisize, lowsize;
			double[] hicoefs;
			double[] lowcoefs;
			int lowoffset, hioffset;

			// These are "pointers" into arrays
			int hiresult;
			int lowresult;
			int hidata;
			int lowdata;

			/* allocate memory for the extended copy of source */
			pow2 = WaveletUtil.WL_pow2(levels - 1);
			lowoffset = lowpass.Offset * pow2;
			hioffset = hipass.Offset * pow2;
			lowsize = (lowpass.Length - 1) * pow2 + 1;
			hisize = (hipass.Length - 1) * pow2 + 1;
			left = Math.Max(lowoffset, hioffset);
			right = Math.Max(lowsize - lowoffset - 1, hisize - hioffset - 1);
			temp = new double[length + (left + right) * pow2];

			//printf("here2\n");
			/* allocate memory for the lowpass & highpass filter coefficients */
			hicoefs = new double[hisize * pow2];
			lowcoefs = new double[lowsize * pow2];

			//printf("here3 %d %d %d\n",dest, &source,length);
			/* copy source to dest to support doing multiple level transforms */
			//memcpy(dest[0], source, length * sizeof(dtype));
			//TODO: Verify this is correct.  Original memcpy is above.  Dest is multi-dimensional
			for (int z = 0; z < length; z++)
				dest[0, z] = source[z];

			for (pow2 = 1, level = 0; level < levels; level++, pow2 *= 2)
			{
				//printf("here pow2:%d level:%d\n",pow2,level);

				/* dilate the filters */
				Dilate_filter(lowpass.Coefs, ref lowcoefs, lowpass.Length, level);
				Dilate_filter(hipass.Coefs, ref hicoefs, hipass.Length, level);
				lowoffset = lowpass.Offset * pow2;
				hioffset = hipass.Offset * pow2;
				lowsize = (lowpass.Length - 1) * pow2 + 1;
				hisize = (hipass.Length - 1) * pow2 + 1;
				left = Math.Max(lowoffset, hioffset);
				right = Math.Max(lowsize - lowoffset - 1, hisize - hioffset - 1);

				/* make periodic extension of dest vector */
				j = 0;
				for (i = length - left; i < length; i++)
					temp[j++] = dest[0, i];

				for (i = 0; i < length; i++)
					temp[j++] = dest[0, i];

				for (i = 0; i < right; i++)
					temp[j++] = dest[0, i];

				/* convolve the data */
				lowresult = 0;
				hiresult = 0;
				lowdata = left - lowoffset;
				hidata = left - hioffset;
				for (i = 0; i < length; i++, lowdata++, hidata++)
				{
					double hisum = 0, lowsum = 0;
					for (j = 0; j < lowsize; j++)
						lowsum += temp[lowdata + j] * lowcoefs[j];
					for (j = 0; j < hisize; j++)
						hisum += temp[hidata + j] * hicoefs[j];
					// TODO: Verify this is correct
					dest[0, lowresult++] = lowsum;
					dest[level + 1, hiresult++] = hisum;
				}
			}

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
		public static int WL_IrwtMatrix(double[,] source,
				  ref double[] dest,
				  int length,
				  int levels, WL_Filter lowpass, WL_Filter hipass)
		{
			int i, j, level, left, right, pow2;
			int hisize, lowsize;
			int lowoffset, hioffset;
			double[] hitemp;
			double[] lowtemp;    /* extended source vector */
			double[] hicoefs;
			double[] lowcoefs;

			// These are "pointers" into arrays
			int hidata;
			int lowdata;
			int result;

			/* allocate memory for the extended copies of hi & low source vectors */
			pow2 = WaveletUtil.WL_pow2(levels - 1);
			lowoffset = lowpass.Offset * pow2;
			hioffset = hipass.Offset * pow2;
			lowsize = (lowpass.Length - 1) * pow2 + 1;
			hisize = (hipass.Length - 1) * pow2 + 1;
			left = Math.Max(lowoffset, hioffset);
			right = Math.Max(lowsize - lowoffset - 1, hisize - hioffset - 1);

			hitemp = new double[length + (left + right) * pow2];
			lowtemp = new double[length + (left + right) * pow2];

			/* allocate memory for the lowpass & highpass filter coefficients */
			hicoefs = new double[hisize * pow2];
			lowcoefs = new double[lowsize * pow2];

			/* copy the lowpass signal to dest to facilitate multiple levels */
			for (i = 0; i < length; i++)
				dest[i] = source[0, i];

			for (pow2 = WaveletUtil.WL_pow2(levels - 1), level = levels - 1; level >= 0;
			 level--, pow2 /= 2)
			{

				/* dilate the filters */
				Dilate_filter(lowpass.Coefs, ref lowcoefs, lowpass.Length, level);
				Dilate_filter(hipass.Coefs, ref hicoefs, hipass.Length, level);
				lowoffset = lowpass.Offset * pow2;
				hioffset = hipass.Offset * pow2;
				lowsize = (lowpass.Length - 1) * pow2 + 1;
				hisize = (hipass.Length - 1) * pow2 + 1;
				left = Math.Max(lowoffset, hioffset);
				right = Math.Max(lowsize - lowoffset - 1, hisize - hioffset - 1);

				/* make periodic extension of dest vector */
				j = 0;
				for (i = length - left; i < length; i++, j++)
				{
					lowtemp[j] = dest[i];
					hitemp[j] = source[level + 1, i];
				}
				for (i = 0; i < length; i++, j++)
				{
					lowtemp[j] = dest[i];
					hitemp[j] = source[level + 1, i];
				}
				for (i = 0; i < right; i++, j++)
				{
					lowtemp[j] = dest[i];
					hitemp[j] = source[level + 1, i];
				}

				/* convolve the data */
				result = 0;
				lowdata = left - lowoffset;
				hidata = left - hioffset;
				for (i = 0; i < length; i++, lowdata++, hidata++)
				{
					double hisum = 0, lowsum = 0;
					for (j = 0; j < lowsize; j++)
						lowsum += lowtemp[lowdata + j] * lowcoefs[j];

					for (j = 0; j < hisize; j++)
						hisum += hitemp[hidata + j] * hicoefs[j];

					dest[result++] = (lowsum + hisum) * 0.5;
				}

			}

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
		public static int WL_FrwtMatrix(double[,] source,
				  ref double[,,] dest,
				  int rows,
				  int cols,
				  int levels, WL_Filter lowpass, WL_Filter hipass)
		{
			double[] tempSource;
			double[,] RowMatrix;
			double[,] ColMatrix;
			int c, r, level;
			int hori, vert, diag;

			/* allocate the temp arrays to hold 1-D results */
			RowMatrix = new double[2, cols];
			ColMatrix = new double[2, rows];
			
			tempSource = new double[rows];

			/* copy the source to dest to facilitate multiple levels */
			for (r = 0; r < rows; r++)
			{
				for (int colNum = 0; colNum < cols; colNum++)
				{
					dest[0, r, colNum] = source[r, colNum];
				}
			}

			for (level = 1; level <= levels; level++)
			{

				/* setup indices to horizontal,vertical & diagonal detail signals */
				vert = 3 * (level - 1) + 1;
				hori = vert + 1;
				diag = hori + 1;

				/* transform each row */
				for (r = 0; r < rows; r++)
				{
					var extract = WaveletUtil.Extract1DArray(dest, 0, r, cols);
					var result = WL_FrwtVector(extract, ref RowMatrix, cols, level, lowpass, hipass);

					if (result != 1)
					{
						return 0;
					}

					for (int colNum = 0; colNum < cols; colNum++)
					{
						dest[0, r, colNum] = RowMatrix[0, colNum];
						dest[hori, r, colNum] = RowMatrix[1, colNum];
					}
				}

				/* transform each column */
				/* first, run the transform on the lowpass output from the */
				/* frwt algorithm run on the rows */
				for (c = 0; c < cols; c++)
				{
					for (r = 0; r < rows; r++)
						tempSource[r] = dest[0, r, c];
					if (WL_FrwtVector(tempSource, ref ColMatrix, rows,
							  level, lowpass, hipass) != 1)
					{
						return 0;
					}
					for (r = 0; r < rows; r++)
					{
						dest[0, r, c] = ColMatrix[0, r];
						dest[vert, r, c] = ColMatrix[1, r];
					}
				}

				/* now, for the high pass output */
				for (c = 0; c < cols; c++)
				{
					for (r = 0; r < rows; r++)
						tempSource[r] = dest[hori, r, c];
					if (WL_FrwtVector(tempSource, ref ColMatrix, rows, level, lowpass, hipass) != 1)
					{
						return 0;
					}
					for (r = 0; r < rows; r++)
					{
						dest[hori, r, c] = ColMatrix[0, r];
						dest[diag, r, c] = ColMatrix[1, r];
					}
				}
			}

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
		public static int WL_IrwtVolume(
			double[,,] source,
			ref double[,] dest,
			int rows,
			int cols,
			int levels,
			WL_Filter lowpass,
			WL_Filter hipass
		)
		{
			double[] tempDest;
			double[,] HighMatrix;
			double[,] RowMatrix;
			double[,] ColMatrix;
			int c, r, level;
			int hori, vert, diag;

			/* allocate the temp arrays to hold 1-D results */
			RowMatrix = new double[2, cols];

			ColMatrix = new double[2, rows];

			HighMatrix = new double[rows, cols];

			tempDest = new double[rows];

			/* copy the source to dest to facilitate multiple levels */
			for (r = 0; r < rows; r++)
			{
				for (int colNum = 0; colNum < cols; colNum++)
				{
					dest[r, colNum] = source[0, r, colNum];
				}
			}

			for (level = levels; level >= 1; level--)
			{

				/* setup indices to horizontal,vertical & diagonal detail signals */
				vert = 3 * (level - 1) + 1;
				hori = vert + 1;
				diag = hori + 1;

				/* transform each column */
				/* first, run the inverse transform using the lowpass output */
				/* from the 1D forward redundant transform along the columns */
				for (c = 0; c < cols; c++)
				{
					for (r = 0; r < rows; r++)
					{
						ColMatrix[0, r] = dest[r, c];
						ColMatrix[1, r] = source[vert, r, c];
					}
					if (WL_IrwtMatrix(ColMatrix, ref tempDest, rows, level, lowpass, hipass) != 1)
					{
						return 0;
					}
					for (r = 0; r < rows; r++)
						dest[r, c] = tempDest[r];
				}

				/* now, for the high pass output */
				for (c = 0; c < cols; c++)
				{
					for (r = 0; r < rows; r++)
					{
						ColMatrix[0, r] = source[hori, r, c];
						ColMatrix[1, r] = source[diag, r, c];
					}
					if (WL_IrwtMatrix(ColMatrix, ref tempDest, rows, level, lowpass, hipass) != 1)
					{
						return 0;
					}
					for (r = 0; r < rows; r++)
						HighMatrix[r, c] = tempDest[r];
				}

				/* transform each row */
				for (r = 0; r < rows; r++)
				{
					for (int colNum = 0; colNum < cols; colNum++)
					{
						RowMatrix[0, colNum] = dest[r, colNum];
						RowMatrix[1, colNum] = HighMatrix[r, colNum];
					}

					var extract = WaveletUtil.Extract1DArray(dest, r, cols);
					var result = WL_IrwtMatrix(RowMatrix, ref extract, cols, level, lowpass, hipass);
					WaveletUtil.Patch1DArray(extract, ref dest, r, cols);

					if (result != 1)
					{
						return 0;
					}
				}
			}

			return 1;
		}
	}
}
