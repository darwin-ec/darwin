//*******************************************************************
//   file: IntensityContour.cxx
//
// author: Scott Hale
//
//   mods: J H Stewman (8/2/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

using Darwin.Extensions;
using Darwin.Helpers;
using Darwin.ImageProcessing;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin
{
    public class IntensityContour : Contour
    {
        private static readonly Color Color1 = Color.FromArgb(1, 1, 1);
        private static readonly Color Color128 = Color.FromArgb(128, 128, 128);

        public IntensityContour(Bitmap bitmap, Contour contour,
            int left, int top, int right, int bottom)
        {
            DirectBitmap grayImage = new DirectBitmap(bitmap);
            DirectBitmapHelper.ConvertToGrayscale(ref grayImage);
            GetPointsFromBitmap(ref grayImage, contour, left, top, right, bottom);
        }

        /* TODO: References spots in old code
         * The main algorithm to extract a fin outline from an image.
         * This method is called from all of this class's constructors, which are called in
         * TraceWindow.cxx (on_traceButtonImageOK_clicked).
         * 
         * There are 9 stages:
         * 
         * 1) Construct, analyize histogram
         * 2) Threshold GrayImage to create BinaryImage
         * 3) Clean up / get a cleanner edge through morphological processes
         * 		a) open (erode, dialate)
         * 		b) erosion with high coefficient to clean up noise, but leave the fin intact
         * 		c) AND with the orginal BinaryImage (#3) to restore the fin shape
         * 4) Feature recognition to select the largest Feature / blob
         * 5) Get a one pixel outline through one erosion and XORing with #4
         * 6) Feature recognition to slect the largest outline (feature /blob) (same code as #4)
         * 7) Find the start point
         * 		A valid start point (p1) must be in math quadrant III
         * 		and be followed by two points (p2,p3) 
         * 		such that p1.row>p2.row>p3.row && p1.col<p2.col<p3.col
         * 8) Walk the outline from the starting point, recording a pixel in the contour every 3 pixels
         * 9) Walk ends when no more black pixels may be found. The outline is recouresed backwards until
         * 		a valid end point (p1) is found. A valid end point must be in math quadrant IV
         * 		and be prefaced by two points (p2,p3) such that p1.row>p2.row>p3.row && p1.col>p2.col>p3.col
         * 
         * The Contour represented by this object has a length of 0 if at anytime the algorithm
         * 		cannot continue. This often occurs for images of poor contrast.
         * 
         * The Contour represented by this object is adjusted with the snake code in TraceWindow.cxx
         * 		if its length is greater than 0.
         * 
         */
        public Contour GetPointsFromBitmap(ref DirectBitmap bmp, Contour ctour, int left, int top, int right, int bottom)
        {
            // Resample the image to lower resolution
            int width = right - left;

            int factor = 1;
            while (width / (float)factor > 1024)
            { //***1.96 - changed magic num JHS
                factor *= 2;
            }

            DirectBitmap workingBmp = null;

            if (factor > 1)
                workingBmp = new DirectBitmap(BitmapHelper.ResizePercentageNearestNeighbor(bmp.Bitmap, 100.0f / factor));
            else
                workingBmp = new DirectBitmap(bmp.Bitmap);

            int xoffset;
            int yoffset;

            workingBmp = DirectBitmapHelper.ApplyBounds(workingBmp, left, top, right, bottom, factor,
                out xoffset, out yoffset);

            IntensityHistogram histogram = new IntensityHistogram(workingBmp);

            Range lowestRange = histogram.FindNextValley(0);

            Range nextRange = histogram.FindNextValley(lowestRange.End);

            int totalPixels = workingBmp.Width * workingBmp.Height;

            // TODO: Magic numbers?
            if (nextRange.Tip - lowestRange.End < 25 && nextRange.HighestValue > totalPixels / 256)
            {
                lowestRange.End = nextRange.End;
                //TODO: update other values in strut as needed
            }

            // TODO: Magic numbers?
            // low contrast check
            if (lowestRange.PixelCount > totalPixels * .7 || lowestRange.End > 150
                || lowestRange.PixelCount < totalPixels * .2)
            {
                //exceeds 90% of images
                //cout << "Low Contrast Warning..." << endl;
                //TODO: Do something
            }

            // use the range to threshold
            DirectBitmapHelper.ThresholdRange(ref workingBmp, lowestRange);


            DirectBitmap saveBinaryBmp = new DirectBitmap(workingBmp.Bitmap);

            /* ----------------------------------------------------
             * Open the image (erode and dilate)
             * ----------------------------------------------------
             */

            int iterations = 4; // Default number of iterations
            int ecount = 0, dcount = 1;
            int itcount = 0;
            int i;

            for (i = 0; i < iterations; i++)
            {
                ecount = MorphologicalOperators.Erode(ref workingBmp, i % 2);
            }

            // Shrink all other regions with less than 5 neighbor black pixles
            while (ecount != 0)
            {
                ecount = MorphologicalOperators.Erode(ref workingBmp, 5);
                itcount++;
            }

            for (i = 0; i < iterations; i++)
            {
                dcount = MorphologicalOperators.Dilate(ref workingBmp, i % 2);;
            }

            // AND opended image with orginal
            MorphologicalOperators.And(ref workingBmp, saveBinaryBmp);

            //Trace.WriteLine("Starting feature recognition...");

            var largestFeature = FeatureIdentification.FindLargestFeature(workingBmp);

            Trace.WriteLine("Feature recognition complete.");

            // now binImg can be reset to the mask of the NEW largestFeature
            DirectBitmap binaryBmp = largestFeature.Mask;

            DirectBitmap outline = new DirectBitmap(binaryBmp.Bitmap);

            ecount = 0;
            for (i = 0; i < 1; i++)
            {
                ecount = MorphologicalOperators.Erode(ref outline, 0);
            }

            MorphologicalOperators.Xor(ref binaryBmp, outline);

            //***1.0LK - a bit of a mess - JHS
            // at this point binImg points to the largestFeature->mask (eroded and XORed).
            // We want to use this binImg to find a new largestFeature, but we cannot
            // delete the current largestFeature until after the call to binImg->getLargestFeature()
            // because binImg will be wiped out by the deletion of the current largestFeature
            // We must delete the current largestFeature at some point OTHEWISE we have a 
            // memory leak.

            //get rid of outlines of inner features (e.g. glare spots) by selecting largest outline

            //cout << "Looking for 2nd Fin Candidate: ";

            //Feature oldLargest = largestFeature; //***1.0LK 

            var finalLargestFeature = FeatureIdentification.FindLargestFeature(binaryBmp);

            if (largestFeature == null)
            {
                // TODO
                Trace.WriteLine("largestFeature NULL, aborting. No fin outline determined");
                return null;
            }

            // now binImg can be reset to the mask of the NEW largestFeature
            DirectBitmap finalWorkingBmp = largestFeature.Mask;

            int row = 0, col = 0;
            int rows = finalWorkingBmp.Height, cols = finalWorkingBmp.Width;
            bool done = false;

            width = 15;

            //cout << "Finding first point..." << endl;

            Darwin.Point pt = ctour[0];
            //Contour::addPoint(pt.x,pt.y);//Add User start
            int stx = pt.X / factor - xoffset;
            int sty = pt.Y / factor - yoffset;

            pt = ctour[1];
            int endy = pt.Y / factor - yoffset;
            int endx = pt.X / factor - xoffset;

            //find starting point (midx,midy)
            int midx;
            int midy = 0;

            int maxy = (sty > endy) ? sty : endy; // Math.max(sty,endy);
            midx = Convert.ToInt32((endx + stx) * .5);

            //find black pixel closest to bottom in column midx
            row = maxy;
            while (!done && row >= 0)
            {
                if (finalWorkingBmp.GetPixel(midx, row).GetIntensity() == 0)
                {
                    midy = row;
                    done = true;
                }
                else
                {
                    row--;
                }
            }

            if (!done)
            {
                Trace.WriteLine("no starting point found.");

                Trace.WriteLine("\nNo outline intersected the bisector of the secant line formed by user supplied start and end points!\n");

                // TODO
                return null;
            }

            Trace.WriteLine(string.Format("Have starting point ({0}, {1})", midx, midy));
            Trace.WriteLine(string.Format("FYI ending point ({0}, {1})", endy, endx));

            row = midy;
            col = midx;

            int st2y = row;
            int st2x = col;

            // Walk from point (row,col)
            finalWorkingBmp.SetPixel(col, row, Color1);

            /* Prioritize direction of movement

                    4 | 3 | 2
                    -- --  --
                    5 | * | 1
                    -- --  --
                    4 | 3 | 2
            */

            i = 0;
            bool foundPoint;
            bool prepend = false;
            done = false;
            while (!done)
            {
                /* Prioritize direction of movement

                        4 | 3 | 2
                        -- --  --
                        5 | * | 1
                        -- --  --
                        4 | 3 | 2
                */
                if (col + 1 < cols && finalWorkingBmp.GetPixel(col + 1, row).GetIntensity() == 0)
                {//E
                    foundPoint = true;
                    col = col + 1;
                }
                else if (row - 1 >= 0 && finalWorkingBmp.GetPixel(col, row - 1).GetIntensity() == 0)
                {//N
                    foundPoint = true;
                    row = row - 1;
                }
                else if (row + 1 < rows && finalWorkingBmp.GetPixel(col, row + 1).GetIntensity() == 0)
                {//S
                    foundPoint = true;
                    row = row + 1;
                }
                else if (col - 1 >= 0 && finalWorkingBmp.GetPixel(col - 1, row).GetIntensity() == 0)
                {//W
                    foundPoint = true;
                    col = col - 1;
                }
                else
                {
                    if (prepend)
                    {
                        done = true;
                        break;
                    }
                    else
                    {
                        prepend = true;
                        row = midy;
                        col = midx;
                        continue;
                    }
                }

                if (foundPoint /*&& i%3==0*/)
                {
                    if (prepend)
                    {
                        AddPoint(factor * (col + xoffset), factor * (row + yoffset), 0);//prepend
                    }
                    else
                    {
                        AddPoint(factor * (col + xoffset), factor * (row + yoffset));
                    }
                    finalWorkingBmp.SetPixel(col, row, Color128);
                    if (row == maxy)
                    {
                        //done with this direction
                        if (prepend)
                        {
                            done = true;
                            break;
                        }
                        else
                        {
                            prepend = true;
                            row = midy;
                            col = midx;
                            continue;
                        }
                    }
                }
                i++;
            } // end loop until done

            TrimAndReorder(ctour[0], ctour[1]);

            Trace.WriteLine("IntensityContour::GetPointsFromBitmap COMPLETE");

            return null;
        }
    }
}
