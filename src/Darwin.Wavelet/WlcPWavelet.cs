/* Tcl Wavelet Laboratory
 * Wavelet transform operations
 *
 * Mike Hilton, 8 Mar 1995
 */

// This file is part of DARWIN.
// Copyright (C) 1994 - 2020
//
// DARWIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DARWIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DARWIN.  If not, see<https://www.gnu.org/licenses/>.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace Darwin.Wavelet
{
    /******************************************************************************
     *                                                                            *
     * PERIODIC WAVELET TRANSFORM FUNCTIONS                                       * 
     *                                                                            *
     ******************************************************************************/
    public static class WlcPWavelet
    {
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
        public static int WL_FwtVolume(double[,,] source, ref double[,,] dest,
                 int depth, int rows, int cols, int levels,
                 WL_Filter lowpass, WL_Filter hipass)
        {
            double[] tempSource;
            double[] tempDest;
            int d, c, r, level;

            /* allocate the temp arrays for columns */
            tempSource = new double[Math.Max(rows, depth)];
            tempDest = new double[Math.Max(rows, depth)];

            /* copy the source to dest to facilitate multiple levels */
            for (d = 0; d < depth; d++)
            {
                for (r = 0; r < rows; r++)
                {
                    for (int colNum = 0; colNum < cols; colNum++)
                    {
                        dest[d, r, colNum] = source[d, r, colNum];
                    }
                }
            }

            for (level = 0; level < levels; level++, rows /= 2, cols /= 2, depth /= 2)
            {
                /* transform each row */
                for (d = 0; d < depth; d++)
                {
                    for (r = 0; r < rows; r++)
                    {
                        var extract = WaveletUtil.Extract1DArray(dest, d, r, cols);
                        var result = WL_FwtVector(extract, ref extract, cols, 1, lowpass, hipass);
                        WaveletUtil.Patch1DArray(extract, ref dest, d, r, cols);

                        if (result != 0)
                        {
                            return 1;
                        }
                    }
                }

//# ifdef DEBUG_MATRIX
//                fprintf(stderr, "level %d ROW:\n", level);
//                for (r = 0; r < rows; r++)
//                {
//                    for (c = 0; c < cols; c++) fprintf(stderr, "%f ", dest[r][c]);
//                    fprintf(stderr, "\n");
//                }
//#endif

                /* transform each column */
                for (d = 0; d < depth; d++)
                {
                    for (c = 0; c < cols; c++)
                    {
                        for (r = 0; r < rows; r++) tempSource[r] = dest[d,r,c];
                        if (WL_FwtVector(tempSource, ref tempDest, rows, 1, lowpass, hipass)
                            != 0)
                        {
                            return 1;
                        }
                        for (r = 0; r < rows; r++) dest[d,r,c] = tempDest[r];
                    }
                }

                /* transform each depth line */
                for (r = 0; r < rows; r++)
                {
                    for (c = 0; c < cols; c++)
                    {
                        for (d = 0; d < depth; d++) tempSource[d] = dest[d, r, c];
                        if (WL_FwtVector(tempSource, ref tempDest, depth, 1, lowpass, hipass)
                            != 0)
                        {
                            return 1;
                        }
                        for (d = 0; d < depth; d++)
                            dest[d,r,c] = tempDest[d];
                    }
                }

//# ifdef DEBUG_MATRIX
//                fprintf(stderr, "level %d COL:\n", level);
//                for (r = 0; r < rows; r++)
//                {
//                    for (c = 0; c < cols; c++) fprintf(stderr, "%f ", dest[r][c]);
//                    fprintf(stderr, "\n");
//                }
//#endif

            }

            return 0;
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
        public static int WL_IwtVolume(double[,,] source, ref double[,,] dest,
                 int depth, int rows, int cols, int levels,
                 WL_Filter lowpass, WL_Filter hipass)
        {
            double[] tempSource;
            double[] tempDest;
            int d, c, r, level;

            /* allocate the temp arrays for columns */
            tempSource = new double[Math.Max(rows, depth)];
            tempDest = new double[Math.Max(rows, depth)];

            /* copy the source to dest to facilitate multiple levels */
            for (d = 0; d < depth; d++)
            {
                for (r = 0; r < rows; r++)
                {
                    for (int colNum = 0; colNum < cols; colNum++)
                    {
                        dest[d, r, colNum] = source[d, r, colNum];
                    }
                }
            }

            rows = rows / WaveletUtil.WL_pow2(levels - 1);
            cols = cols / WaveletUtil.WL_pow2(levels - 1);
            depth = depth / WaveletUtil.WL_pow2(levels - 1);

            for (level = 0; level < levels; level++, rows *= 2, cols *= 2, depth *= 2)
            {

                /* transform each column */
                for (d = 0; d < depth; d++)
                {
                    for (c = 0; c < cols; c++)
                    {
                        for (r = 0; r < rows; r++) tempSource[r] = dest[d, r, c];
                        if (WL_IwtVector(tempSource, ref tempDest, rows, 1, lowpass, hipass)
                            != 0)
                        {
                            return 1;
                        }
                        for (r = 0; r < rows; r++) dest[d,r, c] = tempDest[r];
                    }
                }

                /* transform each row */
                for (d = 0; d < depth; d++)
                {
                    for (r = 0; r < rows; r++)
                    {
                        var extract = WaveletUtil.Extract1DArray(dest, d, r, cols);
                        var result = WL_IwtVector(extract, ref extract, cols, 1, lowpass, hipass);
                        WaveletUtil.Patch1DArray(extract, ref dest, d, r, cols);

                        if (result != 0)
                        {
                            return 1;
                        }
                    }
                }

                /* transform each depth line */
                for (r = 0; r < rows; r++)
                {
                    for (c = 0; c < cols; c++)
                    {
                        for (d = 0; d < depth; d++) tempSource[d] = dest[d,r,c];
                        if (WL_IwtVector(tempSource, ref tempDest, depth, 1, lowpass, hipass)
                            != 0)
                        {
                            return 1;
                        }
                        for (d = 0; d < depth; d++) dest[d,r,c] = tempDest[d];
                    }
                }

            }

            return 0;
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
        public static int WL_FwtMatrix(double[,] source, ref double[,] dest,
                 int rows, int cols, int levels,
                 WL_Filter lowpass, WL_Filter hipass)
        {
            double[] tempSource;
            double[] tempDest;
            int c, r, level;

            /* allocate the temp arrays for columns */
            tempSource = new double[rows];
            tempDest = new double[rows];

            /* copy the source to dest to facilitate multiple levels */
            for (r = 0; r < rows; r++)
                Array.Copy(source, r * cols, dest, r * cols, cols);

            for (level = 0; level < levels; level++, rows /= 2, cols /= 2)
            {

                /* transform each row */
                for (r = 0; r < rows; r++)
                {
                    var extract = WaveletUtil.Extract1DArray(dest, r, cols);
                    var result = WL_FwtVector(extract, ref extract, cols, 1, lowpass, hipass);
                    WaveletUtil.Patch1DArray(extract, ref dest, r, cols);

                    if (result != 0)
                    {
                        return 1;
                    }
                }

//# ifdef DEBUG_MATRIX
//                fprintf(stderr, "level %d ROW:\n", level);
//                for (r = 0; r < rows; r++)
//                {
//                    for (c = 0; c < cols; c++) fprintf(stderr, "%f ", dest[r][c]);
//                    fprintf(stderr, "\n");
//                }
//#endif

                /* transform each column */
                for (c = 0; c < cols; c++)
                {
                    for (r = 0; r < rows; r++) tempSource[r] = dest[r,c];
                    if (WL_FwtVector(tempSource, ref tempDest, rows, 1, lowpass, hipass)
                        != 0)
                    {
                        return 1;
                    }
                    for (r = 0; r < rows; r++) dest[r,c] = tempDest[r];
                }

//# ifdef DEBUG_MATRIX
//                fprintf(stderr, "level %d COL:\n", level);
//                for (r = 0; r < rows; r++)
//                {
//                    for (c = 0; c < cols; c++) fprintf(stderr, "%f ", dest[r][c]);
//                    fprintf(stderr, "\n");
//                }
//#endif
            }

            return 0;
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
        public static int WL_IwtMatrix(double[,] source, double[,] dest,
                 int rows, int cols, int levels,
                 WL_Filter lowpass, WL_Filter hipass)
        {
            double[] tempSource;
            double[] tempDest;
            int c, r, level;

            /* allocate the temp arrays for columns */
            tempSource = new double[rows];
            tempDest = new double[rows];

            /* copy the source to dest to facilitate multiple levels */
            for (r = 0; r < rows; r++)
                Array.Copy(source, r * cols, dest, r * cols, cols);

            rows = rows / WaveletUtil.WL_pow2(levels - 1);
            cols = cols / WaveletUtil.WL_pow2(levels - 1);

            for (level = 0; level < levels; level++, rows *= 2, cols *= 2)
            {

                /* transform each column */
                for (c = 0; c < cols; c++)
                {
                    for (r = 0; r < rows; r++)
                        tempSource[r] = dest[r, c];

                    if (WL_IwtVector(tempSource, ref tempDest, rows, 1, lowpass, hipass)
                        != 0)
                    {
                        return 1;
                    }
                    for (r = 0; r < rows; r++) dest[r,c] = tempDest[r];
                }

//# ifdef DEBUG_MATRIX
//                fprintf(stderr, "level %d COL:\n", level);
//                for (r = 0; r < rows; r++)
//                {
//                    for (c = 0; c < cols; c++) fprintf(stderr, "%f ", dest[r][c]);
//                    fprintf(stderr, "\n");
//                }
//#endif


                /* transform each row */
                for (r = 0; r < rows; r++)
                {
                    var extract = WaveletUtil.Extract1DArray(dest, r, cols);
                    var result = WL_IwtVector(extract, ref extract, cols, 1, lowpass, hipass);
                    WaveletUtil.Patch1DArray(extract, ref dest, r, cols);
                    if (result != 0)
                    {
                        return 1;
                    }
                }

//# ifdef DEBUG_MATRIX
//                fprintf(stderr, "level %d ROW:\n", level);
//                for (r = 0; r < rows; r++)
//                {
//                    for (c = 0; c < cols; c++) fprintf(stderr, "%f ", dest[r][c]);
//                    fprintf(stderr, "\n");
//                }
//#endif

            }

            return 0;
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
        public static int WL_FwtVector(
            double[] source,
            ref double[] dest,
            int length,
            int levels,
            WL_Filter lowpass,
            WL_Filter hipass
        )
        {
            double[] temp;        /* extended source vector */
            int i, j, level;
            int hisize = hipass.Length;
            int lowsize = lowpass.Length;
            double[] hicoefs = hipass.Coefs;
            double[] lowcoefs = lowpass.Coefs;
            double hisum, lowsum;
            int left = Math.Max(lowpass.Offset, hipass.Offset);
            int right = Math.Max(lowpass.Length - lowpass.Offset - 1,
                    hipass.Length - hipass.Offset - 1);

            // These are "pointers" into arrays
            int hiresult;
            int lowresult;
            int hidata;
            int lowdata;

            /* allocate memory for the extended copy of source */
            temp = new double[length + left + right];

            /* copy source to dest to support doing multiple level transforms */
            Array.Copy(source, dest, length);

            for (level = 0; level < levels; level++, length /= 2)
            {
                j = 0;
                for (i = length - left; i < length; i++) temp[j++] = dest[i];
                for (i = 0; i < length; i++) temp[j++] = dest[i];
                for (i = 0; i < right; i++) temp[j++] = dest[i];

//# ifdef DEBUG
//                fprintf(stderr, "level %d  temp: ", level);
//                for (i = 0; i < length + left + right; i++)
//                    fprintf(stderr, "%f ", temp[i]);
//                fprintf(stderr, "\n");
//#endif

                lowresult = 0;
                hiresult = length / 2;
                lowdata = left - lowpass.Offset;
                hidata = left - hipass.Offset;

                for (i = 0; i < length; i += 2, lowdata += 2, hidata += 2)
                {
                    hisum = lowsum = 0;
                    for (j = 0; j < lowsize; j++) lowsum += temp[lowdata + j] * lowcoefs[j];
                    for (j = 0; j < hisize; j++) hisum += temp[hidata + j] * hicoefs[j];
                    dest[lowresult++] = lowsum;
                    dest[hiresult++] = hisum;
                }

//# ifdef DEBUG
//                fprintf(stderr, "level %d result: ", level);
//                for (i = 0; i < length; i++) fprintf(stderr, "%f ", dest[i]);
//                fprintf(stderr, "\n");
//#endif
            }

            return 0;
        }

        /* WL_IwtVector
         *
         * Perform a LEVELS-deep inverse wavelet transform of SOURCE using 
         * the filters HIPASS and LOWPASS,  storing results in DEST.  Optional
         * arguments indicating how edges should be treated are in ARGV.
         * LENGTH is the length of the SOURCE & DEST vectors.
         * The coefficients in SOURCE are expected to be in the Mallat format.
         */
        public static int WL_IwtVector(double[] source, ref double[] dest, int length,
                 int levels, WL_Filter lowpass, WL_Filter hipass)
        {
            double[] upLow;        /* extended upsampled low frequency vector */
            double[] upHi;         /* extended upsampled high frequency vector */
            int i, j, level;
            int hisize = hipass.Length;
            int lowsize = lowpass.Length;
            double[] hicoefs = hipass.Coefs;
            double[] lowcoefs = lowpass.Coefs;
            double sum;
            int left = Math.Max(lowpass.Offset, hipass.Offset);
            int right = Math.Max(lowpass.Length - lowpass.Offset - 1,
                    hipass.Length - hipass.Offset - 1);

            // These are "pointers" into arrays
            int result;
            int hidata;
            int lowdata;

            /* allocate memory for the extended copies of source */
            upLow = new double[length + left + right];
            upHi = new double[length + left + right];

            length = length / WaveletUtil.WL_pow2(levels - 1);
            /* copy the lowest frequency portion to dest to facilitate multiple levels */
            for (i = 0; i < length; i++) dest[i] = source[i];

            for (level = 0; level < levels; level++, length *= 2)
            {

                /* upsample the source data */
                for (i = 0, j = left; i < length / 2; i++)
                {
                    upLow[j] = dest[i];
                    upHi[j] = source[length / 2 + i];
                    j += 1;
                    upLow[j] = 0;
                    upHi[j] = 0;
                    j += 1;
                }

                j = 0;
                for (i = length - left; i < length; i++, j++)
                {
                    upLow[j] = upLow[left + i];
                    upHi[j] = upHi[left + i];
                }
                for (i = 0; i < length; i++, j++)
                {
                    upLow[j] = upLow[left + i];
                    upHi[j] = upHi[left + i];
                }
                for (i = 0; i < right; i++, j++)
                {
                    upLow[j] = upLow[left + i];
                    upHi[j] = upHi[left + i];
                }

//# ifdef DEBUG
//                fprintf(stderr, "level %d  upLow: ", level);
//                for (i = 0; i < length + left + right; i++)
//                    fprintf(stderr, "%f ", upLow[i]);
//                fprintf(stderr, "\n");
//                fprintf(stderr, "level %d  upHi: ", level);
//                for (i = 0; i < length + left + right; i++)
//                    fprintf(stderr, "%f ", upHi[i]);
//                fprintf(stderr, "\n");
//#endif

                lowdata = left - lowpass.Offset;
                hidata = left - hipass.Offset;
                result = 0;

                for (i = 0; i < length; i++, lowdata++, hidata++)
                {
                    sum = 0;
                    for (j = 0; j < lowsize; j++)
                        sum += upLow[lowdata + j] * lowcoefs[j];

                    for (j = 0; j < hisize; j++)
                        sum += upHi[hidata + j] * hicoefs[j];

                    dest[result++] = sum;
                }

//# ifdef DEBUG
//                fprintf(stderr, "level %d result: ", level);
//                for (i = 0; i < length; i++) fprintf(stderr, "%f ", dest[i]);
//                fprintf(stderr, "\n");
//#endif

            }

            return 0;
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
        public static int WL_BoundingBox(int rows, int cols, int level, int detail,
                           out int left, out int right, out int top, out int bottom)
        {
            int rowSize;        /* size of each row at this level */
            int colSize;        /* size of each column at this level */

            /* compute bounding box of gamma quadrant */
            rowSize = cols / WaveletUtil.WL_pow2(level);
            colSize = rows / WaveletUtil.WL_pow2(level);

            left = right = top = bottom = 0;
            switch (detail)
            {
                case 0:
                    /* upper right quadrant */
                    left = rowSize;
                    right = 2 * rowSize;
                    top = 0;
                    bottom = colSize;
                    break;
                case 1:
                    /* lower left quadrant */
                    left = 0;
                    right = rowSize;
                    top = colSize;
                    bottom = 2 * colSize;
                    break;
                case 2:
                    /* lower right quadrant */
                    left = rowSize;
                    right = 2 * rowSize;
                    top = colSize;
                    bottom = 2 * colSize;
                    break;
                default:
                    Trace.WriteLine("WL_BoundingBox: unexpected detail value.");
                    return 1;
            }
            return 0;
        }


        /* WL_ExtractDetail
         *
         * Extracts a detail signal from a matrix in Mallat storage format.
         */
        public static int WL_ExtractDetail(double[,] matrix, int rows, int cols,
                     int level, int detail, ref double[,] output)
        {
            int left, right, top, bottom;
            int i, j, r, c;                      /* loop indices */

            if (WL_BoundingBox(rows, cols, level, detail,
                       out left, out right, out top, out bottom) != 0)
                return 1;

            output = new double[bottom - top, right - left];

            for (i = 0, r = top; r < bottom; r++, i++)
                for (j = 0, c = left; c < right; c++, j++)
                    output[i, j] = matrix[r, c];

            return 0;
        }
    }
}
