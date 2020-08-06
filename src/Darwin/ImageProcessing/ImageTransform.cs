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
using System.Drawing;
using System.Text;

namespace Darwin.ImageProcessing
{
    public static class ImageTransform
    {
        public static Bitmap CropBitmap(Bitmap bitmap, Rectangle cropRect)
        {
			if (bitmap == null)
				throw new ArgumentNullException(nameof(bitmap));

			Bitmap croppedBitmap = new Bitmap(cropRect.Width, cropRect.Height);

			using (Graphics g = Graphics.FromImage(croppedBitmap))
			{
				g.DrawImage(bitmap, new Rectangle(0, 0, croppedBitmap.Width, croppedBitmap.Height),
								 cropRect,
								 GraphicsUnit.Pixel);
			}

			return croppedBitmap;
		}
    }
}
