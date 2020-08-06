//*******************************************************************
//   file: IntensityContour.cxx
//
// author: Scott Hale
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

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

using Darwin.Extensions;
using System.Drawing;

namespace Darwin
{
    public class IntensityContourCyan : IntensityContour
    {
        public IntensityContourCyan(Bitmap bitmap, Contour contour,
            int left, int top, int right, int bottom)
            : base(bitmap, contour, left, top, right, bottom)
        {
            DirectBitmap cyanImage = new DirectBitmap(bitmap);
            cyanImage.ToCyanIntensity();
            GetPointsFromBitmap(ref cyanImage, contour, left, top, right, bottom);
        }
    }
}
