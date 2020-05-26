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

        public static Bitmap ResizePercentageNearestNeighboar(Bitmap bmp, float percentage)
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

        /*
         * Find all black pixels that touch (seedrow,seedcol).
         * (seedrow,seedcol) should be black, otherwise an empty image and an area of 0 are returned.
         * 
         * @param srcImg The image in which to look for contigous black pixels
         * 		NOTE: srcImg is changed. Do not pass *this
         * 		TODO: Check that the code works correctly with srcImg being changed
         * @param seedrow the row of the (row,col) pair at which to start
         * @param seedcol the column of the (row,col) pair at which to start
         * @return a pointer to a Feature containing an BinaryImage of all contigous black pixels touching (seedrow,seedcol)
         */
        public static Feature FindBlob(Bitmap srcImg, int seedrow, int seedcol)
        {
            //Based on code from deburekr
            Stack<Darwin.PointF> nbr_stack = new Stack<Darwin.PointF>();
            int cr, cc; // removed done & j
            int l, r, t, b;  /*left, right, top , bottom neighbor */
            int area;
            int map_val = 0;

            Bitmap map = new Bitmap(srcImg.Width, srcImg.Height);
            using (var graphics = Graphics.FromImage(map))
            {
                graphics.FillRectangle(Brushes.White, 0, 0, map.Width, map.Height);
            }

            nbr_stack.Push(new Darwin.PointF(seedcol, seedrow));
            area = 0;
            while (nbr_stack.Count > 0)
            {
                Darwin.PointF pt = nbr_stack.Pop();
                cr = (int)Math.Round(pt.Y);
                cc = (int)Math.Round(pt.X);

                if (srcImg.GetPixel(cc, cr).GetIntensity() != 255)
                {  /* may already have been pushed & processed */
                    srcImg.SetPixel(cc, cr, Color.White);

                    var mapPixel = Color.FromArgb(map_val, map_val, map_val);
                    map.SetPixel(cc, cr, mapPixel);

                    area++;
                    r = cc + 1;
                    l = cc - 1;
                    t = cr - 1;
                    b = cr + 1;
                    /* process 4 neighbors in R,B,L,T (ESWN) order */
                    if (r < srcImg.Width)
                    { /* border? */
                        if (srcImg.GetPixel(r, cr).GetIntensity() != 255)
                        {
                            nbr_stack.Push(new Darwin.PointF(r, cr));
                        }
                    }
                    if (b < srcImg.Height)
                    {
                        if (srcImg.GetPixel(cc, b).GetIntensity() != 255)
                        {
                            nbr_stack.Push(new Darwin.PointF(cc, b));
                        }
                    }
                    if (l >= 0)
                    {
                        if (srcImg.GetPixel(l, cr).GetIntensity() != 255)
                        {
                            nbr_stack.Push(new Darwin.PointF(l, cr));
                        }
                    }
                    if (t >= 0)
                    {
                        if (srcImg.GetPixel(cc, t).GetIntensity() != 255)
                        {
                            nbr_stack.Push(new Darwin.PointF(cc, t));
                        }
                    }
                }
            } /* end while */
            //printf("Area = %d\n", area);

            return new Feature
            {
                Area = area,
                Mask = map
            };
        }

        public static Feature FindLargestFeature(Bitmap bitmap)
        {
            Feature largest = null;
            Feature current = null;

            //Feature* largest = NULL, *current = NULL;
            //BinaryImage imgCpy(this);

            // TODO: Should we modify the FindBlob method so it doesn't try to operate
            // on the bitmap parameter
            Bitmap imgCpy = new Bitmap(bitmap);

            for (int r = 10; r < bitmap.Height - 10; r++)
            {
                if (r % 100 == 0)
                    Trace.Write("."); //***1.96a - progress feedback for user

                for (int c = 10; c < bitmap.Width - 10; c++)
                {
                    if (imgCpy.GetPixel(c, r).GetIntensity() == 0)
                    {
                        current = FindBlob(imgCpy, r, c);

                        //BinaryImage notMask((current->mask));
                        //notMask.doNot();
                        //imgCpy.doAnd(notMask);
                        if (largest == null || current.Area > largest.Area)
                            largest = current;
                    }
                }
            }

            return largest;
        }
    }
}
