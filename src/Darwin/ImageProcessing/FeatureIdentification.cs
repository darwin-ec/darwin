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
using System.Text;

namespace Darwin.ImageProcessing
{
    public class Feature
    {
        public int Area { get; set; }
        public DirectBitmap Mask { get; set; }
    }

    public class FeatureIdentification
    {
        public static Feature FindLargestFeature(DirectBitmap bitmap)
        {
            Feature largest = null;
            Feature current = null;

            //Feature* largest = NULL, *current = NULL;
            //BinaryImage imgCpy(this);

            // TODO: Should we modify the FindBlob method so it doesn't try to operate
            // on the bitmap parameter
            DirectBitmap imgCpy = new DirectBitmap(bitmap.Bitmap);

            for (int r = 10; r < bitmap.Height - 10; r++)
            {
                if (r % 100 == 0)
                    Trace.Write("."); //***1.96a - progress feedback for user

                for (int c = 10; c < bitmap.Width - 10; c++)
                {
                    if (imgCpy.GetPixel(c, r).GetIntensity() == 0)
                    {
                        current = FindBlob(imgCpy, r, c);

                        if (largest == null || current.Area > largest.Area)
                        {
                            largest = current;
                        }
                    }
                }
            }

            return largest;
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
        public static Feature FindBlob(DirectBitmap srcImg, int seedrow, int seedcol)
        {
            //Based on code from deburekr
            Stack<Darwin.PointF> nbr_stack = new Stack<Darwin.PointF>();
            int cr, cc; // removed done & j
            int l, r, t, b;  /*left, right, top , bottom neighbor */
            int area;
            int map_val = 0;

            DirectBitmap map = new DirectBitmap(srcImg.Width, srcImg.Height, System.Drawing.Imaging.PixelFormat.Format8bppIndexed);

            for (int x = 0; x < map.Width; x++)
            {
                for (int y = 0; y < map.Height; y++)
                {
                    map.SetPixelByte(x, y, ColorExtensions.WhiteByte);
                }
            }

            //using (var graphics = Graphics.FromImage(map.Bitmap))
            //{
            //    graphics.FillRectangle(Brushes.White, 0, 0, map.Width, map.Height);
            //}

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
    }
}
