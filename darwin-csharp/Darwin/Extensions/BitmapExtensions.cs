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
        // There are no octal literals in C#, so we're kinda faking it here.
        private static int o0006 = Convert.ToInt32("0006", 8);
        private static int o0666 = Convert.ToInt32("0666", 8);
        private static int o0110 = Convert.ToInt32("0110", 8);

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

            using (var gr = Graphics.FromImage(bitmap))
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

            using (var gr = Graphics.FromImage(bitmap))
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

        //Morphological Operators
        /** Algorithms doErode and doDilate from Graphics Gems (Ed. Paul S. Heckbert)**/

        /* 
         * Take a thresholded image 0 or 255, as input, and erode 
         * (shrink black regions, grow white regions).
         * 
         * @param coeff The miniumum number of white neighbors (out of 8) to be considered
         * 	an edge pixel and thus changed to white. Use 0 for classical erosion.
         * @return the number of pixels affected (changed from black to white)
         */
        public static int Erode(this Bitmap bitmap, int coeff)
        {
            /*int rows, cols;*/
            int r, c;
            // int i;
            // int pc = 0;           /* pass count */
            int count = 0;        /*deleted pixel count */
            int p, q;             /* neighborhood maps of adjacent cells */
            //int m;                /* Deletion direction mask */

            /* neighborhood maps of previous scanline */
            byte[] qb = new byte[bitmap.Width];

            /* Used for lower-right pixel	*/
            qb[bitmap.Width - 1] = 0;

            /* Scan image while deleting the pixels */

            /* Build initial previous scan buffer */
            if (bitmap.GetPixel(0, 0).GetIntensity() != 0)
                p = 1;
            else
                p = 0;

            // TODO: Some of this may be too much for a single line
            for (c = 0; c < bitmap.Width - 1; c++)
                qb[c] = Convert.ToByte(p = ((p << 1) & o0006) | ((bitmap.GetPixel(c + 1, 0).GetIntensity() != 0) ? 1 : 0));

            /* Scan image for pixel deletion candidates */
            for (r = 0; r < bitmap.Height - 1; r++)
            {
                q = qb[0];
                p = ((q << 3) & o0110) | ((bitmap.GetPixel(0, r + 1).GetIntensity() != 0) ? 1 : 0);

                for (c = 0; c < bitmap.Width - 1; c++)
                {
                    q = qb[c];
                    p = ((p << 1) & o0666) | ((q << 3) & o0110) |
                        ((bitmap.GetPixel(c + 1, r + 1).GetIntensity() != 0) ? 1 : 0);

                    // Values that are outside of byte range are possible here
                    // Note that this should be mod and shouldn't constrain the value
                    // with > max < min checks
                    qb[c] = Convert.ToByte(p % (byte.MaxValue + 1));

                    /* if p[r][c] == 0 and more than COEFF nbrs are white */
                    if (AppSettings.ErosionMatrix[p] > coeff)
                    {
                        count++;
                        bitmap.SetPixel(c, r, Color.White);
                    }
                }

                /* Process right edge pixel */
                p = (p << 1) & o0666;
                if (AppSettings.ErosionMatrix[p] > coeff)
                {
                    count++;
                    bitmap.SetPixel(bitmap.Width - 1, r, Color.White);
                }
            }

            /* Process bottom scan line */

            // Different from book
            for (c = 0; c < bitmap.Width - 1; c++)
            {
                q = qb[c];
                p = ((p << 1) & o0666) | ((q << 3) & o0110);

                if (AppSettings.ErosionMatrix[p] > coeff)
                {
                    count++;
                    bitmap.SetPixel(c, bitmap.Height - 1, Color.White);
                }
            }

            return (count);
        }

        /* 
         * Take a thresholded image 0 or 255, as input, dilate
         * (shrink white regions, grow black regions).
         * 
         * @param coeff The miniumum number of black neighbors (out of 8) to be considered
         * 	an edge pixel and thus changed to black. Use 0 for classical dilation.
         * @return the number of pixels affected (changed from white to black)
         */
        public static int Dilate(this Bitmap bitmap, int coeff)
        {
            int r, c;
            //int i;
            // int pc = 0;     /* pass count */
            int p, q;       /* neighborhood maps of adjacent cells */
            //int m;		/* Deletion direction mask */

            /* neighborhood maps of previous scanline */
            byte[] qb = new byte[bitmap.Width];

            qb[bitmap.Width - 1] = 0;

            /* Scan image while deleting the pixels */

            /*while (count && (pc<6)) {*/
            //	pc++;
            /*deleted pixel count */
            int count = 0;

            /* Build initial previous scan buffer */

            /* Build initial previous scan buffer */
            if (bitmap.GetPixel(0, 0).GetIntensity() != 0)
                p = 1;
            else
                p = 0;

            for (c = 0; c < bitmap.Width - 1; c++)
                qb[c] = Convert.ToByte(p = ((p << 1) & o0006) | ((bitmap.GetPixel(c + 1, 0).GetIntensity() != 0) ? 1 : 0));

            /* Scan image for pixel deletion candidates */
            for (r = 0; r < bitmap.Height - 1; r++)
            {
                q = qb[0];
                p = ((q << 3) & o0110) | ((bitmap.GetPixel(0, r + 1).GetIntensity() != 0) ? 1 : 0);

                for (c = 0; c < bitmap.Width - 1; c++)
                {
                    q = qb[c];
                    p = ((p << 1) & o0666) | ((q << 3) & o0110) |
                        ((bitmap.GetPixel(c + 1, r + 1).GetIntensity() != 0) ? 1 : 0);

                    // Values that are outside of byte range are possible here
                    // Note that this should be mod and shouldn't constrain the value
                    // with > max < min checks
                    qb[c] = Convert.ToByte(p % (byte.MaxValue + 1));

                    /* if p[r][c] == 1 and more than COEFF nbrs are black */
                    if (AppSettings.DilationMatrix[p] > coeff && r != 0 && c != 0)
                    {
                        count++;
                        bitmap.SetPixel(c, r, Color.Black);
                    }
                }

                /* Process right edge pixel */

                /*p = (p<<1)&0666;
                if (dilate[p] > coeff){
                   count++;
                   mData[r * mCols + mCols-1].setIntensity(0);
                }*/
            }

            /* Process bottom scan line */


            /*for (c=0; c<mCols-1; c++) {
               q = qb[c];
               p = ((p<<1) & 0666) | ((q<<3) & 0110); 

               if (dilate[p] > coeff){
                  count++;
                  mData[(mRows-1) * mCols + c].setIntensity(0);
               }
            }*/

            /*printf ("dilate: pass %d, %d white pixels deleted\n", pc, count);*/
            /*}*/

            return count;
        }

        /*
         * TODO: This comment does not appear to be accurate.
         * Change each pixels in this BinaryImage to white if the coresponding pixel in img 
         * is not black.
         * 
         * @param img the image with which to execute the operation. img is not affected
         */
        public static void And(this Bitmap bitmap, Bitmap otherBitmap)
        {
            if (otherBitmap == null)
                throw new ArgumentNullException(nameof(otherBitmap));

            if (bitmap.Width < 1 || bitmap.Height < 1)
                throw new ArgumentOutOfRangeException(nameof(bitmap));

            if (otherBitmap.Width < 1 || otherBitmap.Height < 1)
                throw new ArgumentOutOfRangeException(nameof(otherBitmap));


            if (bitmap.Width != otherBitmap.Width || bitmap.Height != otherBitmap.Height)
                throw new Exception("Mismatched size on Bitmaps");

            for (int r = 0; r < bitmap.Height; r++)
            {
                for (int c = 0; c < bitmap.Width; c++)
                {
                    if (!(bitmap.GetPixel(c, r).GetIntensity() == 0 && otherBitmap.GetPixel(c, r).GetIntensity() == 0))
                    {
                        bitmap.SetPixel(c, r, Color.White);
                    }
                }
            }
        }

        /*
         * The resulting image is black only where one (but not both) images was black.
         * 
         * @param img the image with which to execute the operation. img is not affected
         */
        public static void Xor(this Bitmap bitmap, Bitmap otherBitmap)
        {
            if (otherBitmap == null)
                throw new ArgumentNullException(nameof(otherBitmap));

            if (bitmap.Width < 1 || bitmap.Height < 1)
                throw new ArgumentOutOfRangeException(nameof(bitmap));

            if (otherBitmap.Width < 1 || otherBitmap.Height < 1)
                throw new ArgumentOutOfRangeException(nameof(otherBitmap));


            if (bitmap.Width != otherBitmap.Width || bitmap.Height != otherBitmap.Height)
                throw new Exception("Mismatched size on Bitmaps");

            for (int r = 0; r < bitmap.Height; r++)
            {
                for (int c = 0; c < bitmap.Width; c++)
                {
                    if (bitmap.GetPixel(c, r).GetIntensity() == 0 && otherBitmap.GetPixel(c, r).GetIntensity() == 0)
                    {
                        bitmap.SetPixel(c, r, Color.White);
                    }
                    else if (otherBitmap.GetPixel(c, r).GetIntensity() == 0)
                    {
                        bitmap.SetPixel(c, r, Color.Black);
                    }
                }
            }
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
