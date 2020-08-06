/* Tcl Wavelet Laboratory
 * Symmetric Wavelet transform operations
 *
 * Mike Hilton, 
 * Fausto Espinal, 13 Jan 1996
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
using System.Text;

namespace Darwin.Wavelet
{
    /******************************************************************************
     *                                                                            *
     * SYMMETRIC WAVELET TRANSFORM FUNCTIONS                                      *
     *                                                                            *
     ******************************************************************************/
    public static class WlcSWavelet
    {
        /* WL_FswtVolume
         *
         * Perform a LEVELS-deep separable forward wavelet transform of SOURCE using 
         * the filters HIPASS and LOWPASS,  storing results in DEST.  
         * DEPTH, ROWS and COLS indicate the size of the SOURCE & DEST volumes.
         * The transformed coefficients in DEST are stored in the Mallat 
         * representation.
         */
        public static int WL_FswtVolume(double[,,] source, ref double[,,] dest,
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

            for (level = 0; level < levels; level++, rows = (rows + 1) / 2,
                 cols = (cols + 1) / 2, depth = (depth + 1) / 2)
            {

                /* transform each row */
                for (d = 0; d < depth; d++)
                {
                    for (r = 0; r < rows; r++)
                    {
                        var extract = WaveletUtil.Extract1DArray(dest, d, r, cols);
                        var result = WL_FswtVector(extract, ref extract, cols, 1, lowpass, hipass);
                        WaveletUtil.Patch1DArray(extract, ref dest, d, r, cols);
                        if (result != 0)
                        {
                            return 1;
                        }
                    }
                }

                /* transform each column */
                for (d = 0; d < depth; d++)
                {
                    for (c = 0; c < cols; c++)
                    {
                        for (r = 0; r < rows; r++) tempSource[r] = dest[d,r,c];
                        if (WL_FswtVector(tempSource, ref tempDest, rows, 1, lowpass, hipass)
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
                        for (d = 0; d < depth; d++) tempSource[d] = dest[d,r,c];
                        if (WL_FswtVector(tempSource, ref tempDest, depth, 1, lowpass, hipass)
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

        /* WL_IswtVolume
         *
         * Perform a LEVELS-deep separable inverse symmetric wavelet 
         * transform of SOURCE using 
         * the filters HIPASS and LOWPASS,  storing results in DEST.  
         * DEPTH, ROWS and COLS indicate the size of the SOURCE & DEST volumes.
         * The transformed coefficients in SOURCE must be stored in the Mallat 
         * format.
         */
        public static int WL_IswtVolume(double[,,] source, ref double[,,] dest,
                 int depth, int rows, int cols, int levels,
                 WL_Filter lowpass, WL_Filter hipass)
        {
            int[] dSplit;
            int[] rSplit;
            int[] cSplit;
            double[] tempSource;
            double[] tempDest;
            int d, c, r, level;

            rSplit = new int[levels + 1];
            cSplit = new int[levels + 1];
            dSplit = new int[levels + 1];

            /* allocate the temp arrays for columns */
            tempSource = new double[Math.Max(depth, rows)];
            tempDest = new double[Math.Max(depth, rows)];

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

            /* Determine the way the dimensions of the signal is partitioned. */
            for (level = levels, c = cols, r = rows, d = depth; level > 0; level--)
            {
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

            rows = rSplit[0];
            cols = cSplit[0];
            depth = dSplit[0];

            for (level = 1; level < levels + 1; level++)
            {

                rows += (rSplit[level]);
                cols += (cSplit[level]);
                depth += (dSplit[level]);

                /* transform each row */
                for (d = 0; d < depth; d++)
                {
                    for (r = 0; r < rows; r++)
                    {
                        var extract = WaveletUtil.Extract1DArray(dest, d, r, cols);
                        var result = WL_IswtVector(extract, ref extract, cols, 1, lowpass, hipass);
                        WaveletUtil.Patch1DArray(extract, ref dest, d, r, cols);

                        if (result != 0)
                        {
                            return 1;
                        }
                    }
                }

                /* transform each column */
                for (d = 0; d < depth; d++)
                {
                    for (c = 0; c < cols; c++)
                    {
                        for (r = 0; r < rows; r++)
                            tempSource[r] = dest[d,r,c];

                        if (WL_IswtVector(tempSource, ref tempDest, rows, 1, lowpass, hipass)
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
                        for (d = 0; d < depth; d++) tempSource[d] = dest[d,r,c];
                        if (WL_IswtVector(tempSource, ref tempDest, depth, 1, lowpass, hipass)
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


        /* WL_FswtMatrix
         *
         * Perform a LEVELS-deep separable forward wavelet transform of SOURCE using 
         * the filters HIPASS and LOWPASS,  storing results in DEST.  
         * ROWS and COLS indicate the size of the SOURCE & DEST vectors.
         * The transformed coefficients in DEST are stored in the Mallat 
         * representation.
         */
        public static int WL_FswtMatrix(double[,] source, ref double[,] dest,
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
            {
                for (int colNum = 0; colNum < cols; colNum++)
                {
                    dest[r, colNum] = source[r, colNum];
                }
            }

            for (level = 0; level < levels; level++, rows = (rows + 1) / 2, cols = (cols + 1) / 2)
            {
                /* transform each row */
                for (r = 0; r < rows; r++)
                {
                    var extract = WaveletUtil.Extract1DArray(dest, r, cols);
                    var result = WL_FswtVector(extract, ref extract, cols, 1, lowpass, hipass);
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
                    if (WL_FswtVector(tempSource, ref tempDest, rows, 1, lowpass, hipass)
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


        /* WL_IswtMatrix
         *
         * Perform a LEVELS-deep separable inverse symmetric wavelet 
         * transform of SOURCE using 
         * the filters HIPASS and LOWPASS,  storing results in DEST.  
         * ROWS and COLS indicate the size of the SOURCE & DEST vectors.
         * The transformed coefficients in SOURCE must be stored in the Mallat 
         * format.
         */
        public static int WL_IswtMatrix(double[,] source, ref double[,] dest,
                 int rows, int cols, int levels,
                 WL_Filter lowpass, WL_Filter hipass)
        {
            int[] rSplit;
            int[] cSplit;
            double[] tempSource;
            double[] tempDest;
            int c, r, level;

            rSplit = new int[levels + 1];
            cSplit = new int[levels + 1];

            /* allocate the temp arrays for columns */
            tempSource = new double[rows];
            tempDest = new double[rows];

            /* copy the source to dest to facilitate multiple levels */
            for (r = 0; r < rows; r++)
            {
                for (int colNum = 0; colNum < cols; colNum++)
                {
                    dest[r, colNum] = source[r, colNum];
                }
            }

            /* Determine the way the dimensions of the signal is partitioned. */
            for (level = levels, c = cols, r = rows; level > 0; level--)
            {
                rSplit[level] = r / 2;
                r = (r + 1) / 2;
                cSplit[level] = c / 2;
                c = (c + 1) / 2;
            }
            rSplit[0] = r;
            cSplit[0] = c;

            rows = rSplit[0];
            cols = cSplit[0];

            for (level = 1; level < levels + 1; level++)
            {
                rows += (rSplit[level]);
                cols += (cSplit[level]);

                /* transform each column */
                for (c = 0; c < cols; c++)
                {
                    for (r = 0; r < rows; r++) tempSource[r] = dest[r,c];
                    if (WL_IswtVector(tempSource, ref tempDest, rows, 1, lowpass, hipass)
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
                    var result = WL_IswtVector(extract, ref extract, cols, 1, lowpass, hipass);
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

        /* WL_FswtVector
         *
         * Perform a LEVELS-deep symmetric forward wavelet transform of SOURCE using 
         * the filters HIPASS and LOWPASS,  storing results in DEST.  
         * LENGTH is the length of the SOURCE & DEST vectors.
         * The transformed coefficients in DEST are stored in the Mallat 
         * representation.
         */
        public static int WL_FswtVector(
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

            for (level = 0; level < levels; level++)
            {
                /* make symmetric extension of dest vector */
                if (lowsize % 2 == 1)
                {
                    /* odd length filter (WSS filter), use E(1,1) extension */
                    j = 0;
                    for (i = left; i > 0; i--) temp[j++] = dest[i];
                    for (i = 0; i < length; i++) temp[j++] = dest[i];
                    for (i = length - 2; i > length - 2 - right; i--) temp[j++] = dest[i];
                }
                else
                {
                    /* even length filter (HS filter), use E(2,2) extension */
                    j = 0;
                    for (i = left - 1; i >= 0; i--) temp[j++] = dest[i];
                    for (i = 0; i < length; i++) temp[j++] = dest[i];
                    for (i = length - 1; i >= length - right; i--) temp[j++] = dest[i];
                }

                /* filter the temp vector */
                lowresult = 0;
                hiresult = (length + 1) / 2;
                lowdata = left - lowpass.Offset;
                hidata = left - hipass.Offset;
                for (i = 0; i < length - 1; i += 2)
                {
                    hisum = lowsum = 0;
                    for (j = 0; j < lowsize; j++) lowsum += temp[lowdata + j] * lowcoefs[j];
                    for (j = 0; j < hisize; j++) hisum += temp[hidata + j] * hicoefs[j];
                    dest[lowresult++] = lowsum;
                    dest[hiresult++] = hisum;
                    lowdata += 2;
                    hidata += 2;
                }

                /* Compute extra low-pass coefficient if signal is odd */
                if ((length % 2) != 0)
                {
                    lowsum = 0;
                    for (j = 0; j < lowsize; j++) lowsum += temp[lowdata + j] * lowcoefs[j];
                    dest[lowresult] = lowsum;
                }
                length = (length + 1) / 2;
            }

            return 0;
        }

        /* WL_IswtVector
         *
         * Perform a LEVELS-deep inverse symmetric wavelet transform of SOURCE using 
         * the filters HIPASS and LOWPASS,  storing results in DEST.  
         * LENGTH is the length of the SOURCE & DEST vectors.
         * The coefficients in SOURCE are expected to be in the Mallat format.
         */
        public static int WL_IswtVector(
            double[] source,
            ref double[] dest,
            int length,
            int levels,
            WL_Filter lowpass,
            WL_Filter hipass
        )
        {
            int[] sgnpart;
            int wBound;       /* Current wavelet coefficient boundary */
            int tmp;
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

            sgnpart = new int[levels + 1];

            /* allocate memory for the extended copies of source */
            upLow = new double[length + left + right];
            upHi = new double[length + left + right];

            /* Determine the way the transformed signal is partitioned. */
            for (level = levels, tmp = length; level > 0; level--)
            {
                sgnpart[level] = tmp / 2;
                tmp = (tmp + 1) / 2;
            }
            sgnpart[0] = tmp;

            /* copy the lowest frequency portion to dest to facilitate multiple levels */
            for (i = 0; i < sgnpart[0] + sgnpart[1]; i++) dest[i] = source[i];
            wBound = sgnpart[0];

            for (level = 0; level < levels; level++)
            {

                /* upsample the source data */
                for (i = 0, j = left; i < wBound; i++)
                {
                    upLow[j] = dest[i];
                    j += 1;
                    upLow[j] = 0;
                    j += 1;
                }
                for (i = 0, j = left; i < sgnpart[level + 1]; i++)
                {
                    upHi[j] = source[wBound + i];
                    j += 1;
                    upHi[j] = 0;
                    j += 1;
                }

                length = wBound + sgnpart[level + 1];

                /* edge extension options */
                if (lowsize % 2 == 1)
                {
                    /* odd length filter (WSS filter) */
                    if (length % 2 == 0)
                    {
                        /* even length signal */
                        /* use E(1,2) for low */
                        for (j = 0, i = 2 * left; i > left; i--) upLow[j++] = upLow[i];
                        for (j = left + length, i = left + length - 2; i > left + length - 2 - right; i--)
                            upLow[j++] = upLow[i];
                        /* use E(2,1) for hi */
                        upHi[left - 1] = 0;
                        for (j = 0, i = 2 * left - 2; i >= left; i--) upHi[j++] = upHi[i];
                        for (j = length + left, i = left + length - 4; i > left + length - 4 - right; i--)
                            upHi[j++] = upHi[i];
                    }
                    else
                    {
                        /* odd length signal */
                        /* use E(1,1) for low, symmetric */
                        for (j = left - 1, i = left + 1; i <= 2 * left; i++) upLow[j--] = upLow[i];
                        for (j = length + left, i = length + left - 2; i > left + length - 2 - right; i--)
                            upLow[j++] = upLow[i];
                        /* use E(2,2) for hi, symmetric */
                        upHi[left - 1] = 0;
                        for (j = left - 2, i = left; i < 2 * left - 1; i++) upHi[j--] = upHi[i];
                        upHi[length + left - 1] = upHi[length + left - 3];
                        for (j = length + left, i = length + left - 4; i > left + length - 4 - right; i--)
                            upHi[j++] = upHi[i];
                    }
                }
                else
                {
                    /* even length filter (HS filter) */
                    if (length % 2 == 0)
                    {
                        /* even length signal */
                        /* use E(2,2) for low */
                        upLow[left - 1] = 0;
                        for (j = 0, i = 2 * left - 2; i >= left; i--) upLow[j++] = upLow[i];
                        for (j = left + length, i = left + length - 2; i > left + length - 2 - right; i--)
                            upLow[j++] = upLow[i];
                        /* use E(2,2) for hi, antisymmetric */
                        upHi[left - 1] = 0;
                        for (j = 0, i = 2 * left - 2; i >= left; i--) upHi[j++] = -upHi[i];
                        for (j = left + length, i = left + length - 2; i > left + length - 2 - right; i--)
                            upHi[j++] = -upHi[i];
                    }
                    else
                    {
                        /* odd length signal */
                        /* use E(2,1) for low */
                        upLow[left - 1] = 0;
                        for (j = left - 2, i = left; i < 2 * left - 1; i++) upLow[j--] = upLow[i];
                        for (j = length + left, i = length + left - 2; i > left + length - 2 - right; i--)
                            upLow[j++] = upLow[i];
                        /* use E(2,1) for hi, antisymmetric */
                        upHi[left - 1] = 0;
                        for (j = left - 2, i = left; i < 2 * left - 1; i++) upHi[j--] = -upHi[i];
                        upHi[length + left - 1] = -upHi[length + left - 5];
                        for (j = length + left, i = length + left - 6; i > left + length - 6 - right; i--)
                            upHi[j++] = -upHi[i];
                    }
                }

#if false
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

                wBound += (sgnpart[level + 1]);
            }

            return 0;
        }
    }
}
