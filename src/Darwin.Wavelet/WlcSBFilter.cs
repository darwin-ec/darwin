/* Wavelet Laboratory
 * Subband Filter I/O operations.
 *
 * Mike Hilton, 16 May 96
 * Fausto Espinal, 18 Jan. 97
 *
 * A subband filter is a compound object consisting of four filters:
 * the high and lowpass forward transform filters, and the high and
 * low inverse transform filters.  
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
    public static class WlcSBFilter
    {
        /* WL_SBFilterLoad
         *
         * Loads a subband filter from a data file.  
         */
        public static int WL_SBFilterLoad(string fileName, string filterName, WL_SubbandFilter[,] newFilter)
        {
            int length, rows, cols;
            double[] data;

            // "Pointer" into data
            int d;
            int i;
            int coef;

            /* read in the data */
            if (WaveletUtil.WL_ReadAsciiDataFile(fileName, out rows, out cols, out data) != 0)
                return 1;

            length = rows * cols;
            d = 0;

            /* create a new filter */
            WL_SubbandFilter filter = new WL_SubbandFilter
            {
                Info = new WL_Struct
                {
                    Name = filterName,
                    Type = "sbfilter",
                    PList = null
                }
            };

            /* fill in the data for the four filters */
            for (i = 0; i < 4; i++)
            {
                /* is there header data for a filter ? */
                if (length - 2 < 0)
                {
                    Trace.WriteLine("WL_SBFilterLoad : Bad descriptor file: not enough data");
                    return 1;
                }

                filter.Filters[i].Length = (int)(data[d++] + 0.5);
                filter.Filters[i].Offset = (int)(data[d++] + 0.5);
                length -= 2;
                /* is the offset legal? */
                if ((filter.Filters[i].Offset < 0) ||
                (filter.Filters[i].Offset > filter.Filters[i].Length))
                {
                    Trace.WriteLine("WL_SBFilterLoad : Bad descriptor file: not enough data");
                    return 1;
                }
                /* is there enough data left for this filter? */
                if (length < filter.Filters[i].Length)
                    throw new Exception("WL_SBFilterLoad : Bad descriptor file: not enough data");

                /* store the filter coefficients into an array */
                filter.Filters[i].Coefs = new double[filter.Filters[i].Length];

                for (coef = 0; coef < filter.Filters[i].Length; coef++)
                {
                    filter.Filters[i].Coefs[coef] = data[d++];
                    length--;
                }
            }

            return 0;
        }
    }
}
