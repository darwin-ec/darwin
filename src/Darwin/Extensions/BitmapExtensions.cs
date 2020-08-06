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

using SixLabors.ImageSharp;
using SixLabors.ImageSharp.Formats.Png;
using SixLabors.ImageSharp.PixelFormats;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Rectangle = System.Drawing.Rectangle;
using Color = System.Drawing.Color;
using ColorMatrix = System.Drawing.Imaging.ColorMatrix;

namespace Darwin.Extensions
{
    public static class BitmapExtensions
    {
        public static Bitmap ConvertTo24bppRgb(Bitmap bmp)
        {
            var result = new Bitmap(bmp.Width, bmp.Height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);
            using (var gr = Graphics.FromImage(result))
                gr.DrawImage(bmp, new Rectangle(0, 0, bmp.Width, bmp.Height));
            return result;
        }

        public static Bitmap AlterBrightness(this Bitmap bitmap, int brightness)
        {
            float brightnessScaled = (float)brightness / 255.0f;
            
            ColorMatrix colorMatrix = new ColorMatrix(new float[][] {
                     new float[] { 1, 0, 0, 0, 0 },
                     new float[] { 0, 1, 0, 0, 0 },
                     new float[] { 0, 0, 1, 0, 0 },
                     new float[] { 0, 0, 0, 1, 0 },
                     new float[] { brightnessScaled, brightnessScaled, brightnessScaled, 0, 1 }
            });

            ImageAttributes imageAttributes = new ImageAttributes();
            imageAttributes.SetColorMatrix(colorMatrix);

            var destRect = new Rectangle(0, 0, bitmap.Width, bitmap.Height);
            Bitmap updatedBrightnessBitmap = new Bitmap(bitmap.Width, bitmap.Height);
            using (Graphics graphics = Graphics.FromImage(updatedBrightnessBitmap))
            {
                graphics.DrawImage(bitmap, destRect,
                    0, 0, bitmap.Width, bitmap.Height,
                    GraphicsUnit.Pixel,
                    imageAttributes);
            }

            return updatedBrightnessBitmap;
        }

        public static Bitmap EnhanceContrastMinMax(this Bitmap bitmap, byte minLevel, byte maxLevel)
        {
            if (maxLevel == 255 && minLevel == 0)
                return new Bitmap(bitmap);

            int range = maxLevel - minLevel;
            int tempIntensity;
            Bitmap enhancedContrastBitmap = new Bitmap(bitmap.Width, bitmap.Height);

            for (int c = 0; c < bitmap.Width; c++)
            {
                for (int r = 0; r < bitmap.Height; r++)
                {
                    var sourcePixel = bitmap.GetPixel(c, r);
                    tempIntensity = (int)Math.Round(((sourcePixel.GetIntensity() - minLevel) * 255) / (float)range);

                    if (tempIntensity < 1 || sourcePixel.GetIntensity() < minLevel)
                        tempIntensity = 0;

                    if (tempIntensity > 254)
                        tempIntensity = 255;

                    enhancedContrastBitmap.SetPixel(c, r, sourcePixel.SetIntensity((byte)tempIntensity));
                }
            }

            return enhancedContrastBitmap;
        }

        public static void SaveAsCompressedPng(this Bitmap bitmap, string filename)
        {
            using (FileStream fs = File.Create(filename))
            {
                using (var memoryStream = new MemoryStream())
                {
                    bitmap.Save(memoryStream, System.Drawing.Imaging.ImageFormat.Png);

                    memoryStream.Seek(0, SeekOrigin.Begin);

                    using (var imageSharpImage = SixLabors.ImageSharp.Image.Load<Bgr24>(memoryStream))
                    {
                        var encoder = new PngEncoder
                        {
                            CompressionLevel = PngCompressionLevel.BestCompression
                        };

                        imageSharpImage.SaveAsPng(fs, encoder);
                    }
                }
            }
        }

        /// <summary>
        /// Enhance the contrast of a Bitmap
        /// </summary>
        /// <param name="bitmap">The Bitmap on which to enhance contrast</param>
        /// <param name="level">0 for no change, between -255 and 255 to not blow the image out</param>
        /// <returns>Bitmap with enhanced contrast</returns>
        public static Bitmap EnhanceContrast(this Bitmap bitmap, int level)
        {
            float contrast = (1.0f + level / 255f) / 1.0f;
            float tValue = (1f - contrast) / 2f;

            ColorMatrix colorMatrix = new ColorMatrix(new float[][] {
                     new float[] { contrast, 0, 0, 0, 0 },
                     new float[] { 0, contrast, 0, 0, 0 },
                     new float[] { 0, 0, contrast, 0, 0 },
                     new float[] { 0, 0, 0, 1, 0 },
                     new float[] { tValue, tValue, tValue, 0, 1 }
            });

            ImageAttributes imageAttributes = new ImageAttributes();
            imageAttributes.SetColorMatrix(colorMatrix);

            var destRect = new Rectangle(0, 0, bitmap.Width, bitmap.Height);
            Bitmap updatedContrastBitmap = new Bitmap(bitmap.Width, bitmap.Height);
            using (Graphics graphics = Graphics.FromImage(updatedContrastBitmap))
            {
                graphics.DrawImage(bitmap, destRect,
                    0, 0, bitmap.Width, bitmap.Height,
                    GraphicsUnit.Pixel,
                    imageAttributes);
            }

            return updatedContrastBitmap;
        }

        public static Bitmap ToGrayscale(this Bitmap bitmap)
        {
            var result = new Bitmap(bitmap.Width, bitmap.Height, PixelFormat.Format24bppRgb);

            for (int x = 0; x < bitmap.Width; x++)
            {
                for (int y = 0; y < bitmap.Height; y++)
                {
                    var pixel = bitmap.GetPixel(x, y);
                    result.SetPixel(x, y, pixel.ToGrayscaleColor());
                }
            }

            return result;
        }

        //102AT, 103AT
        //*******************************************************************
        //
        // GrayImage* convColorToCyan(const ColorImage* srcImage)
        //
        //    Converts color image to grayscale image representing the cyan channel of srcImage.
        public static Bitmap ToCyanIntensity(this Bitmap bitmap)
        {
            var result = new Bitmap(bitmap.Width, bitmap.Height);

            for (int x = 0; x < bitmap.Width; x++)
            {
                for (int y = 0; y < bitmap.Height; y++)
                {
                    var pixel = bitmap.GetPixel(x, y);
                    result.SetPixel(x, y, pixel.ToCyanIntensity());
                }
            }

            return result;
        }

        public static Bitmap ToThreshold(this Bitmap bitmap, float threshold)
        {
            var result = new Bitmap(bitmap.Width, bitmap.Height);

            ImageAttributes attributes = new ImageAttributes();
            attributes.SetThreshold(threshold);

            var rect = new Rectangle(0, 0, bitmap.Width, bitmap.Height);

            using (var gr = Graphics.FromImage(result))
            {
                gr.DrawImage(bitmap, rect, 0, 0, bitmap.Width, bitmap.Height, GraphicsUnit.Pixel, attributes);
            }

            return result;
        }

        public static Bitmap ToThresholdGrayscale(this Bitmap bitmap, float threshold)
        {
            var result = new Bitmap(bitmap.Width, bitmap.Height);

            ImageAttributes attributes = new ImageAttributes();
            attributes.SetColorMatrix(new ColorMatrix(AppSettings.GrayscaleConversionMatrix));
            attributes.SetThreshold(threshold);

            var rect = new Rectangle(0, 0, bitmap.Width, bitmap.Height);

            using (var gr = Graphics.FromImage(result))
            {
                gr.DrawImage(bitmap, rect, 0, 0, bitmap.Width, bitmap.Height, GraphicsUnit.Pixel, attributes);
            }

            return result;
        }

        public static Bitmap ToThresholdRange(this Bitmap bitmap, Range range)
        {
            var result = new Bitmap(bitmap.Width, bitmap.Height);

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

        public static void ConvertPositionToRowCol(int position, int bitmapWidth, out int row, out int col)
        {
            row = position / bitmapWidth;
            col = position % bitmapWidth;
        }

        public static Color GetPixelByPosition(this Bitmap bitmap, int position)
        {
            if (position < 0 || position > bitmap.Width * bitmap.Height - 1)
                throw new ArgumentOutOfRangeException(nameof(position));

            int row;
            int col;

            ConvertPositionToRowCol(position, bitmap.Width, out row, out col);

            return bitmap.GetPixel(col, row);
        }

        public static void SetPixelByPosition(this Bitmap bitmap, int position, Color color)
        {
            if (position < 0 || position > bitmap.Width * bitmap.Height - 1)
                throw new ArgumentOutOfRangeException(nameof(position));

            int row;
            int col;

            ConvertPositionToRowCol(position, bitmap.Width, out row, out col);

            bitmap.SetPixel(col, row, color);
        }
    }
}
