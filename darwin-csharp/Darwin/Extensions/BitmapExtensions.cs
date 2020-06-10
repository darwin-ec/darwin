using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin.Extensions
{
    public static class BitmapExtensions
    {
        public static Bitmap AlterBrightness(this Bitmap bitmap, int brightness)
        {
            float brightnessScaled = (float)brightness / 255.0f;
            
            float[][] brightnessTransform = {
                     new float[] { 1, 0, 0, 0, 0 },
                     new float[] { 0, 1, 0, 0, 0 },
                     new float[] { 0, 0, 1, 0, 0 },
                     new float[] { 0, 0, 0, 1, 0 },
                     new float[] { brightnessScaled, brightnessScaled, brightnessScaled, 0, 1 }
            };

            ColorMatrix colorMatrix = new ColorMatrix(brightnessTransform);
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

        public static Bitmap EnhanceContrast(this Bitmap bitmap, byte minLevel, byte maxLevel)
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

        private static void ConvertPositionToRowCol(int position, int bitmapWidth, out int row, out int col)
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
