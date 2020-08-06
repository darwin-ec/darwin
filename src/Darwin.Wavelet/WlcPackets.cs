/* Wavelet Laboratory (WL)
 * Wavelet packet operations.
 *   Compute a full tree decomposition of a vector or a matrix.
 *
 * Fausto Espinal, 2 June 97
 *
 * A full decomposition is simply the transform taken at recursively
 * along both the low and high pass channel.
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
    public static class WlcPackets
    {
        /* WL_FswptMatrix
         *
         * Computes the full decomposition tree for the symmetric wavelet
         *  transform up to the desired level.
         */
        public static int WL_FswptMatrix(double[,] source, ref double[,] dest,
                 int rows, int cols, int levels,
                 WL_Filter lowpass, WL_Filter hipass)
        {
            int nRows, nCols;
            double[,] tmp;
            double[,] tmpDest;

            if (levels == 0)
            {
                Array.Copy(source, dest, rows * cols);
                return 0;
            }

            if (WlcSWavelet.WL_FswtMatrix(source, ref dest, rows, cols, 1, lowpass, hipass) != 0)
            {
                return 1;
            }

            nRows = rows / 2;
            nCols = cols / 2;
            tmpDest = new double[nRows, nCols];

            tmp = WaveletUtil.WL_Extract2D(dest, 0, 0, nRows, nCols);

            if (WL_FswptMatrix(tmp, ref tmpDest, nRows, nCols, levels - 1, lowpass, hipass) != 0)
            {
                return 1;
            }
            WaveletUtil.WL_Put2D(ref dest, tmpDest, 0, 0, nRows, nCols);

            /* Horizontal */
            tmp = WaveletUtil.WL_Extract2D(dest, 0, nCols, nRows, nCols);

            if (WL_FswptMatrix(tmp, ref tmpDest, nRows, nCols, levels - 1, lowpass, hipass) != 0)
            {
                return 1;
            }
            WaveletUtil.WL_Put2D(ref dest, tmpDest, 0, nCols, nRows, nCols);

            /* Diagonal */
            tmp = WaveletUtil.WL_Extract2D(dest, nRows, nCols, nRows, nCols);

            if (WL_FswptMatrix(tmp, ref tmpDest, nRows, nCols, levels - 1, lowpass, hipass) != 0)
            {
                return 1;
            }
            WaveletUtil.WL_Put2D(ref dest, tmpDest, nRows, nCols, nRows, nCols);

            /* Vertical */
            tmp = WaveletUtil.WL_Extract2D(dest, nRows, 0, nRows, nCols);

            if (WL_FswptMatrix(tmp, ref tmpDest, nRows, nCols, levels - 1, lowpass, hipass) != 0)
            {
                return 1;
            }

            WaveletUtil.WL_Put2D(ref dest, tmpDest, nRows, 0, nRows, nCols);

            return 0;
        }

        /* WL_IswptMatrix
         *
         * Computes the Inverse of the full decomposition tree for the symmetric wavelet
         *  transform up to the desired level.
         */
        public static int WL_IswptMatrix(double[,] source, ref double[,] dest,
                 int rows, int cols, int levels,
                 WL_Filter lowpass, WL_Filter hipass)
        {
            int nRows, nCols;
            double[,] tmp;
            double[,] tmpDest;
            double[,] tempSource;

            if (levels == 1)
            {
                return WlcSWavelet.WL_IswtMatrix(source, ref dest, rows, cols, 1, lowpass, hipass);
            }

            nRows = rows / 2;
            nCols = cols / 2;

            tmpDest = new double[nRows, nCols];
            tempSource = new double[rows, cols];

            tmp = WaveletUtil.WL_Extract2D(source, 0, 0, nRows, nCols);

            if (WL_IswptMatrix(tmp, ref tmpDest, nRows, nCols, levels - 1, lowpass, hipass) != 0)
            {
                return 1;
            }
            WaveletUtil.WL_Put2D(ref tempSource, tmpDest, 0, 0, nRows, nCols);

            /* Horizontal */
            tmp = WaveletUtil.WL_Extract2D(source, 0, nCols, nRows, nCols);

            if (WL_FswptMatrix(tmp, ref tmpDest, nRows, nCols, levels - 1, lowpass, hipass) != 0)
            {
                return 1;
            }
            WaveletUtil.WL_Put2D(ref tempSource, tmpDest, 0, nCols, nRows, nCols);

            /* Diagonal */
            tmp = WaveletUtil.WL_Extract2D(source, nRows, nCols, nRows, nCols);

            if (WL_FswptMatrix(tmp, ref tmpDest, nRows, nCols, levels - 1, lowpass, hipass) != 0)
            {
                return 1;
            }
            WaveletUtil.WL_Put2D(ref tempSource, tmpDest, nRows, nCols, nRows, nCols);

            /* Vertical */
            tmp = WaveletUtil.WL_Extract2D(source, nRows, 0, nRows, nCols);

            if (WL_FswptMatrix(tmp, ref tmpDest, nRows, nCols, levels - 1, lowpass, hipass) != 0)
            {
                return 1;
            }
            WaveletUtil.WL_Put2D(ref tempSource, tmpDest, nRows, 0, nRows, nCols);

            if (WlcSWavelet.WL_IswtMatrix(tempSource, ref dest, rows, cols, 1, lowpass, hipass) != 0)
            {
                return 1;
            }

            return 0;
        }

        /* WL_PacketBounding
         *
         * Computes the Bounding box of the desired Wavelet Packet.
         * The addresing scheme used is a string based one where the elements
         * of the string come from the alphabet {0,1,2,3} and the length
         * indicates the level of the packet.
         */
        public static int WL_PacketBounding(int rows, int cols, string addr,
                              out int left, out int right, out int top, out int bottom)
        {
            left = right = top = bottom = 0;

            int i;
            int levels;
            int l, r, t, b;

            l = 0;
            r = cols;
            t = 0;
            b = rows;
            levels = addr.Length;
            
            for (i = 0; i < levels; i++)
            {
                switch (addr[i])
                {
                    case '0':
                        r = r - ((r - l) / 2);
                        b = b - ((b - t) / 2);
                        break;
                    case '1':
                        l = l + ((r - l) / 2);
                        b = b - ((b - t) / 2);
                        break;
                    case '2':
                        r = r - ((r - l) / 2);
                        t = t + ((b - t) / 2);
                        break;
                    case '3':
                        l = l + ((r - l) / 2);
                        t = t + ((b - t) / 2);
                        break;
                    default:
                        return 1;
                }
            }
            left = l;
            right = r;
            top = t;
            bottom = b;
            return 0;
        }

    }
}
