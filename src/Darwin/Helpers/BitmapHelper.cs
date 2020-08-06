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
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Darwin.Helpers
{
    public static class BitmapHelper
    {
        private const PixelOffsetMode _PixelOffsetMode = PixelOffsetMode.Default;
        private const CompositingQuality _CompositingQuality = CompositingQuality.Default;

        public static Bitmap ResizePercentageNearestNeighbor(Bitmap bmp, float percentage)
        {
            if (bmp == null)
                throw new ArgumentNullException(nameof(bmp));

            if (percentage < 0)
                throw new ArgumentOutOfRangeException(nameof(percentage));

            float scale = percentage / 100.0f;
            int newWidth = Convert.ToInt32(Math.Round(bmp.Width * scale));
            int newHeight = Convert.ToInt32(Math.Round(bmp.Height * scale));

            return ResizeBitmap(bmp, newWidth, newHeight, InterpolationMode.NearestNeighbor);
        }

        public static Bitmap ResizeKeepAspectRatio(Bitmap bmp, int maxWidth, int maxHeight)
        {
            // Figure out the ratio
            double ratioX = (double)maxWidth / (double)bmp.Width;
            double ratioY = (double)maxHeight / (double)bmp.Height;

            // Use whichever multiplier is smaller
            double ratio = ratioX < ratioY ? ratioX : ratioY;

            int newHeight = Convert.ToInt32(bmp.Height * ratio);
            int newWidth = Convert.ToInt32(bmp.Width * ratio);

            return ResizeBitmap(bmp, newWidth, newHeight);
        }

        public static Bitmap ResizeBitmap(Bitmap bmp, int newWidth, int newHeight, InterpolationMode interpolationMode = InterpolationMode.HighQualityBicubic)
        {
            if (bmp == null)
                throw new ArgumentNullException(nameof(bmp));

            if (newWidth < 1)
                throw new ArgumentOutOfRangeException(nameof(newWidth));

            if (newHeight < 1)
                throw new ArgumentOutOfRangeException(nameof(newHeight));

            Bitmap resizedImage = new Bitmap(newWidth, newHeight, PixelFormat.Format24bppRgb);

            using (var graphic = Graphics.FromImage(resizedImage))
            {
                graphic.InterpolationMode = interpolationMode;
                graphic.PixelOffsetMode = _PixelOffsetMode;
                graphic.CompositingQuality = _CompositingQuality;

                graphic.Clear(Color.Transparent); // Transparent padding, just in case
                graphic.DrawImage(bmp, 0, 0, newWidth, newHeight);
            }

            return resizedImage;
        }

        public static Bitmap CropBitmap(Bitmap bmp, int left, int top, int right, int bottom)
        {
            Rectangle cropRect = new Rectangle(left, top, right - left, bottom - top);

            Bitmap croppedImage = new Bitmap(cropRect.Width, cropRect.Height);

            using (Graphics g = Graphics.FromImage(croppedImage))
            {
                g.DrawImage(bmp,
                    new Rectangle(0, 0, croppedImage.Width, croppedImage.Height),
                    cropRect,
                    GraphicsUnit.Pixel);
            }

            return croppedImage;
        }

        public static Bitmap ApplyBounds(Bitmap bitmap, int left, int top, int right, int bottom, int factor, out int xoffset, out int yoffset)
        {
            xoffset = Convert.ToInt32((float)left / factor);
            yoffset = Convert.ToInt32((float)top / factor);

            return CropBitmap(bitmap,
                xoffset,
                yoffset,
                Convert.ToInt32((float)right / factor),
                Convert.ToInt32((float)bottom / factor));
        }

        public static Bitmap Copy8bppIndexed(Bitmap source)
        {
            int width = source.Width;
            int height = source.Height;

            BitmapData sourceData = source.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.ReadOnly, source.PixelFormat);

            var sourceStride = Math.Abs(sourceData.Stride);

            int bytes = Math.Abs(sourceStride) * height;
            byte[] sourceBytes = new byte[bytes];

            Marshal.Copy(sourceData.Scan0, sourceBytes, 0, sourceBytes.Length);

            Bitmap result = new Bitmap(width, height, PixelFormat.Format8bppIndexed);

            result.Palette = source.Palette;

            BitmapData resultData = result.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.ReadWrite, result.PixelFormat);
            var resultStride = Math.Abs(resultData.Stride);
            int resultNumBytes = Math.Abs(resultStride) * height;

            if (resultNumBytes != bytes)
                throw new Exception("There was a problem copying images, byte counts on arrays should have matched.");

            byte[] resultBytes = new byte[resultNumBytes];
            Marshal.Copy(resultData.Scan0, resultBytes, 0, resultBytes.Length);

            Array.Copy(sourceBytes, resultBytes, resultNumBytes);

            Marshal.Copy(resultBytes, 0, resultData.Scan0, resultNumBytes);

            source.UnlockBits(sourceData);
            result.UnlockBits(resultData);

            return result;
        }

        public static ImageFormat GetImageFormatFromExtension(string filename)
        {
            string extension = Path.GetExtension(filename);

            if (string.IsNullOrEmpty(extension))
                throw new ArgumentOutOfRangeException(nameof(filename));

            switch (extension.ToLower())
            {
                case ".bmp":
                    return ImageFormat.Bmp;

                case ".gif":
                    return ImageFormat.Gif;

                case ".jpg":
                case ".jpeg":
                    return ImageFormat.Jpeg;

                case ".png":
                    return ImageFormat.Png;

                case ".tif":
                case ".tiff":
                    return ImageFormat.Tiff;

                default:
                    throw new NotImplementedException();
            }
        }
    }
}
