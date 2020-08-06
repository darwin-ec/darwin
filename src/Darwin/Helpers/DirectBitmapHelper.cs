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
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Text;
using Darwin.Extensions;

namespace Darwin.Helpers
{
    public static class DirectBitmapHelper
    {
        public static DirectBitmap ConvertToDirectBitmapGrayscale(Bitmap bmp)
        {
            var result = new DirectBitmap(bmp.Width, bmp.Height, System.Drawing.Imaging.PixelFormat.Format8bppIndexed);

            //create the grayscale ColorMatrix
            ColorMatrix colorMatrix = new ColorMatrix(new float[][] {
                new float[] {.299f, .299f, .299f, 0, 0},
                new float[] {.587f, .587f, .587f, 0, 0},
                new float[] {.114f, .114f, .114f, 0, 0},
                new float[] {0, 0, 0, 1, 0},
                new float[] {0, 0, 0, 0, 1}
            });

            ImageAttributes imageAttributes = new ImageAttributes();
            imageAttributes.SetColorMatrix(colorMatrix);

            DirectBitmap grayscale24BitTemp = new DirectBitmap(bmp.Width, bmp.Height);

            using (var graphics = Graphics.FromImage(grayscale24BitTemp.Bitmap))
            {
                graphics.DrawImage(bmp, new Rectangle(0, 0, bmp.Width, bmp.Height), 0, 0,
                    bmp.Width, bmp.Height, GraphicsUnit.Pixel, imageAttributes);
            }

            for (int x = 0; x < grayscale24BitTemp.Width; x++)
            {
                for (int y = 0; y < grayscale24BitTemp.Height; y++)
                {
                    result.SetPixelByte(x, y, grayscale24BitTemp.GetIntensity(x, y));
                }
            }
     
            return result;
        }

        public static void ConvertToGrayscale(ref DirectBitmap bitmap)
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

        public static void ThresholdRange(ref DirectBitmap bitmap, Range range)
        {
            for (int x = 0; x < bitmap.Width; x++)
            {
                for (int y = 0; y < bitmap.Height; y++)
                {
                    var intensity = bitmap.GetPixel(x, y).GetIntensity();

                    var pixelValue = Color.FromArgb(0, 0, 0);
                    if (intensity > range.End || intensity < range.Start)
                        pixelValue = Color.FromArgb(255, 255, 255);

                    bitmap.SetPixel(x, y, pixelValue);
                }
            }
        }

        public static DirectBitmap ResizePercentageNearestNeighbor(DirectBitmap bmp, float percentage)
        {
            if (bmp == null)
                throw new ArgumentNullException(nameof(bmp));

            if (percentage < 0)
                throw new ArgumentOutOfRangeException(nameof(percentage));

            if (bmp.BitsPerPixel != 8)
                return new DirectBitmap(BitmapHelper.ResizePercentageNearestNeighbor(bmp.Bitmap, percentage));

            // Below is for 8 bpp grayscale
            float scale = percentage / 100.0f;
            int newWidth = Convert.ToInt32(Math.Round(bmp.Width * scale));
            int newHeight = Convert.ToInt32(Math.Round(bmp.Height * scale));

            DirectBitmap result = new DirectBitmap(newWidth, newHeight, PixelFormat.Format8bppIndexed);

            for (int x = 0; x < newWidth; x++)
            {
                for (int y = 0; y < newHeight; y++)
                {
                    int oldX = (int)Math.Round(x / scale);
                    int oldY = (int)Math.Round(y / scale);
                    if (oldX < 0)
                        oldX = 0;
                    else if (oldX >= bmp.Width)
                        oldX = bmp.Width - 1;

                    if (oldY < 0)
                        oldY = 0;
                    else if (oldY >= bmp.Height)
                        oldY = bmp.Height - 1;

                    result.SetPixelByte(x, y, bmp.GetPixel(oldX, oldY).R);
                }
            }

            return result;
        }

        public static DirectBitmap CropBitmap(DirectBitmap bmp, int left, int top, int right, int bottom)
        {
            Rectangle cropRect = new Rectangle(left, top, right - left, bottom - top);

            DirectBitmap croppedImage;

            // We can't use Graphics to draw into an indexed bitmap
            if (bmp.Bitmap.PixelFormat == PixelFormat.Format8bppIndexed)
            {
                croppedImage = new DirectBitmap(cropRect.Width, cropRect.Height, PixelFormat.Format8bppIndexed);

                for (int x = 0; x < cropRect.Width; x++)
                {
                    for (int y = 0; y < cropRect.Height; y++)
                    {
                        croppedImage.SetPixelByte(x, y, bmp.GetIntensity(x, y));
                    }
                }
            }
            else
            {
                croppedImage = new DirectBitmap(cropRect.Width, cropRect.Height);

                using (Graphics g = Graphics.FromImage(croppedImage.Bitmap))
                {
                    g.DrawImage(bmp.Bitmap,
                        new Rectangle(0, 0, croppedImage.Width, croppedImage.Height),
                        cropRect,
                        GraphicsUnit.Pixel);
                }
            }

            return croppedImage;
        }

        public static DirectBitmap ApplyBounds(DirectBitmap bitmap, int left, int top, int right, int bottom, int factor, out int xoffset, out int yoffset)
        {
            xoffset = Convert.ToInt32((float)left / factor);
            yoffset = Convert.ToInt32((float)top / factor);

            return CropBitmap(bitmap,
                xoffset,
                yoffset,
                Convert.ToInt32((float)right / factor),
                Convert.ToInt32((float)bottom / factor));
        }
    }
}
