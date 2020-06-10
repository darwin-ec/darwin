using Darwin.Extensions;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
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

            return ResizeBitmap(bmp, newHeight, newWidth, InterpolationMode.NearestNeighbor);
        }

        public static Bitmap ResizeBitmap(Bitmap bmp, int newWidth, int newHeight, InterpolationMode interpolationMode = InterpolationMode.HighQualityBicubic)
        {
            if (bmp == null)
                throw new ArgumentNullException(nameof(bmp));

            if (newWidth < 1)
                throw new ArgumentOutOfRangeException(nameof(newWidth));

            if (newHeight < 1)
                throw new ArgumentOutOfRangeException(nameof(newHeight));

            var resizedImage = new Bitmap(newWidth, newHeight);

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
    }
}
