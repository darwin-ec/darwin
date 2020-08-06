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
using System.Drawing.Imaging;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin.Extensions
{
    public static class DirectBitmapExtensions
    {
        public static void ToGrayscale(this DirectBitmap bitmap)
        {
            for (int x = 0; x < bitmap.Width; x++)
            {
                for (int y = 0; y < bitmap.Height; y++)
                {
                    var pixel = bitmap.GetPixel(x, y);
                    bitmap.SetPixel(x, y, pixel.ToGrayscaleColor());
                }
            }
        }

        //102AT, 103AT
        //*******************************************************************
        //
        // GrayImage* convColorToCyan(const ColorImage* srcImage)
        //
        //    Converts color image to grayscale image representing the cyan channel of srcImage.
        public static void ToCyanIntensity(this DirectBitmap bitmap)
        {
            for (int x = 0; x < bitmap.Width; x++)
            {
                for (int y = 0; y < bitmap.Height; y++)
                {
                    var pixel = bitmap.GetPixel(x, y);
                    bitmap.SetPixel(x, y, pixel.ToCyanIntensity());
                }
            }
        }

        public static DirectBitmap ToThreshold(this DirectBitmap bitmap, float threshold)
        {
            var result = new DirectBitmap(bitmap.Width, bitmap.Height);

            ImageAttributes attributes = new ImageAttributes();
            attributes.SetThreshold(threshold);

            var rect = new Rectangle(0, 0, bitmap.Width, bitmap.Height);

            using (var gr = Graphics.FromImage(result.Bitmap))
            {
                gr.DrawImage(bitmap.Bitmap, rect, 0, 0, bitmap.Width, bitmap.Height, GraphicsUnit.Pixel, attributes);
            }

            return result;
        }

        public static DirectBitmap ToThresholdGrayscale(this DirectBitmap bitmap, float threshold)
        {
            var result = new DirectBitmap(bitmap.Width, bitmap.Height);

            ImageAttributes attributes = new ImageAttributes();
            attributes.SetColorMatrix(new ColorMatrix(AppSettings.GrayscaleConversionMatrix));
            attributes.SetThreshold(threshold);

            var rect = new Rectangle(0, 0, bitmap.Width, bitmap.Height);

            using (var gr = Graphics.FromImage(result.Bitmap))
            {
                gr.DrawImage(bitmap.Bitmap, rect, 0, 0, bitmap.Width, bitmap.Height, GraphicsUnit.Pixel, attributes);
            }

            return result;
        }

        public static DirectBitmap ToThresholdRange(this DirectBitmap bitmap, Range range)
        {
            var result = new DirectBitmap(bitmap.Width, bitmap.Height);

            for (int x = 0; x < bitmap.Width; x++)
            {
                for (int y = 0; y < bitmap.Height; y++)
                {
                    var intensity = bitmap.GetPixel(x, y).GetIntensity();

                    var pixelValue = Color.FromArgb(0, 0, 0);
                    if (intensity > range.End || intensity < range.Start)
                        pixelValue = Color.FromArgb(255, 255, 255);

                    result.SetPixel(x, y, pixelValue);
                }
            }

            return result;
        }

        public static Color GetPixelByPosition(this DirectBitmap bitmap, int position)
        {
            if (position < 0 || position > bitmap.Width * bitmap.Height - 1)
                throw new ArgumentOutOfRangeException(nameof(position));

            int row;
            int col;

            BitmapExtensions.ConvertPositionToRowCol(position, bitmap.Width, out row, out col);

            return bitmap.GetPixel(col, row);
        }

        public static void SetPixelByPosition(this DirectBitmap bitmap, int position, Color color)
        {
            if (position < 0 || position > bitmap.Width * bitmap.Height - 1)
                throw new ArgumentOutOfRangeException(nameof(position));

            int row;
            int col;

            BitmapExtensions.ConvertPositionToRowCol(position, bitmap.Width, out row, out col);

            bitmap.SetPixel(col, row, color);
        }
    }
}
