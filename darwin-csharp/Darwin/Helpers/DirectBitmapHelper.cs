using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Text;
using Darwin.Extensions;

namespace Darwin.Helpers
{
    public static class DirectBitmapHelper
    {
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

        public static DirectBitmap CropBitmap(DirectBitmap bmp, int left, int top, int right, int bottom)
        {
            Rectangle cropRect = new Rectangle(left, top, right - left, bottom - top);

            DirectBitmap croppedImage = new DirectBitmap(cropRect.Width, cropRect.Height);

            using (Graphics g = Graphics.FromImage(croppedImage.Bitmap))
            {
                g.DrawImage(bmp.Bitmap,
                    new Rectangle(0, 0, croppedImage.Width, croppedImage.Height),
                    cropRect,
                    GraphicsUnit.Pixel);
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
