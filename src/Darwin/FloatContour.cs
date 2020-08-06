//*******************************************************************
//   file: FloatContour.cxx
//
// author: 
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

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

using Darwin.Collections;
using Darwin.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin
{
    public class FloatContour
    {
        private ObservableNotifiableCollection<PointF> _points;

        public ObservableNotifiableCollection<PointF> Points
        {
            get
            {
                if (_points == null)
                    _points = new ObservableNotifiableCollection<PointF>();

                return _points;
            }
            set
            {
                _points = value;
            }
        }

        public int Length
        {
            get
            {
                return Points.Count;
            }
        }

        public FloatContour()
        {
        }

        //********************************************************************
        // FloatContour()
        //
        public FloatContour(FloatContour contour)  //***006FC new
        {
            if (contour == null)
                throw new ArgumentNullException(nameof(contour));

            _points = new ObservableNotifiableCollection<PointF>();

            foreach (var p in contour.Points)
                _points.Add(new PointF(p.X, p.Y, p.Z));
        }

        //********************************************************************
        //
        // bool Contour::findPositionOfClosestPoint(float x, float y, int &position) const
        //
        //    Sets position to the index of the contour point "closest to" (x,y)
        //    and returns true.  If there is no contour, the function returns false.
        //
        public int FindPositionOfClosestPoint(float x, float y)
        {
            if (Length == 0)
                return -1;

            double lowestDistance = MathHelper.GetDistance(x, y, _points[0].X, _points[0].Y);
            int position = 0;

            for (int i = 1; i < Length; i++)
            {

                double curDistance = MathHelper.GetDistance(x, y, _points[i].X, _points[i].Y);

                if (curDistance < lowestDistance)
                {
                    lowestDistance = curDistance;
                    position = i;
                }
            }

            return position;
        }

        /// <summary>
        /// This disables collection and item change notifications.  This is
        /// for performance on FloatContours that aren't used in the UI/etc.
        /// </summary>
        public void DisableNotificationEvents()
        {
            if (Points != null)
                Points.DisableEvents();
        }

        //***005FC next 5 functions are new
        //********************************************************************
        // popFront()
        // -- Pops *numPops elements from the front of the vector
        //
        public void PopFront(int numPops)
        {
            for (var i = 0; i < numPops; i++)
                RemovePoint(0);
        }

        public void RemovePoint(int position)
        {
            if (position < 0 || position >= Length)
                throw new ArgumentOutOfRangeException(nameof(position));

            Points.RemoveAt(position);
        }

        public FloatContour Rotate(PointF centerPoint, float degreesToRotate)
        {
            FloatContour result = new FloatContour();

            result.Points = new ObservableNotifiableCollection<PointF>();

            foreach (var p in this.Points)
                result.Points.Add(p.Rotate(centerPoint, degreesToRotate));

            return result;
        }

        public int GetNumPointsDistanceFromPoint(double distance, PointF referencePoint)
        {
            int numPoints = 0;
            foreach (var p in this.Points)
            {
                if (MathHelper.GetDistance(p.X, p.Y, referencePoint.X, referencePoint.Y) < distance)
                    break;

                numPoints += 1;
            }

            return numPoints;
        }

        public int GetEndNumPointsDistanceFromPoint(double distance, PointF referencePoint)
        {
            int numPoints = 0;
            for (var i = Points.Count - 1; i >= 0; i--)
            {
                if (MathHelper.GetDistance(Points[i].X, Points[i].Y, referencePoint.X, referencePoint.Y) < distance)
                    break;

                numPoints += 1;
            }

            return numPoints;
        }

        public FloatContour TrimBeginningToDistanceFromPoint(double distance, PointF referencePoint, out int numPointsTrimmed, bool disableEvents = false)
        {
            FloatContour result = new FloatContour();

            result.Points = new ObservableNotifiableCollection<PointF>();

            if (disableEvents)
                result.DisableNotificationEvents();

            bool foundBeginning = false;
            numPointsTrimmed = 0;

            foreach (var p in this.Points)
            {
                if (!foundBeginning && MathHelper.GetDistance(p.X, p.Y, referencePoint.X, referencePoint.Y) < distance)
                        foundBeginning = true;

                if (foundBeginning)
                    result.Points.Add(p);
                else
                    numPointsTrimmed += 1;
            }

            return result;
        }

        public FloatContour TransformTrimBeginningToDistanceFromPoint(float[,] transform, double distance, PointF referencePoint, out int numPointsTrimmed, bool disableEvents = false)
        {
            FloatContour result = new FloatContour();

            if (disableEvents)
                result.DisableNotificationEvents();

            result.Points = new ObservableNotifiableCollection<PointF>();

            bool foundBeginning = false;
            numPointsTrimmed = 0;

            foreach (var p in this.Points)
            {
                float newX = transform[0, 0] * p.X
                        + transform[0, 1] * p.Y
                        + transform[0, 2];

                float newY = transform[1, 0] * p.X
                        + transform[1, 1] * p.Y
                        + transform[1, 2];

                if (!foundBeginning && MathHelper.GetDistance(newX, newY, referencePoint.X, referencePoint.Y) < distance)
                    foundBeginning = true;

                if (foundBeginning)
                {
                    result.Points.Add(new PointF(newX, newY));
                }
                else
                {
                    numPointsTrimmed += 1;
                }
            }

            return result;
        }

        //********************************************************************
        // FindIndexFromChainPoint()
        // -- Given an <X,Y> Location from the chain, returns the closest
        //    index into the Contour
        //
        public int FindIndexFromChainPoint(float desx, float desy)
        {
            double ldiff = 0.0;
            int lowest = 0;

            for (var i = 0; i < Length; i++)
            {
                var diff = MathHelper.GetDistance(desx, desy, _points[i].X, _points[i].Y);
                if (i == 0)
                    ldiff = diff;
                else if (diff < ldiff)
                {
                    ldiff = diff;
                    lowest = i;
                }
            }

            return lowest;
        }

        //********************************************************************
        // ContourToFloatContour()
        // -- Convert a Contour into *this* FloatContour
        //
        public void ContourToFloatContour(Contour c)
        {
            //***008OL remove any existing mPointVector
            if (Length > 0)
                _points.Clear();

            for (int i = 0; i < c.Length; i++)
            {
                AddPoint(c[i].X, c[i].Y);
            }
        }

        //********************************************************************
        // FloatContourToContour()
        // -- Return a pointer to a Contour converted from a FloatContour
        //
        public Contour ToContour()
        {
            Contour c = new Contour();
            int length = Length, i, x, y;

            for (i = 0; i < length; i++)
            {

                x = (int)Math.Round(_points[i].X);
                y = (int)Math.Round(_points[i].Y);
                c.AddPoint(x, y);
            }

            return c;
        }

        //***005FC end of new function listings

        //********************************************************************
        // addPoint()
        //
        public void AddPoint(float x, float y)
        {
            Points.Add(new Darwin.PointF { X = x, Y = y });

        }

        //********************************************************************
        // operator[]()
        //
        public Darwin.PointF this[int i]
        {
            // Routing through "Points" rather than "_points" so we have the null check
            get
            {
                return Points[i];
            }
            set
            {
                Points[i] = value;
            }
        }


        //********************************************************************
        // minX()
        //
        public float MinX()
        {

            if (Length <= 0)
                return 0.0f;


            return _points.Min(x => x.X);
        }

        //********************************************************************
        // minY()
        //
        public float MinY()
        {

            if (Length <= 0)
                return 0.0f;

            return _points.Min(x => x.Y);
        }

        //********************************************************************
        // maxX()
        //
        public float MaxX()
        {

            if (Length <= 0)
                return 0.0f;

            return _points.Max(x => x.X);
        }

        //********************************************************************
        // maxY()
        //
        public float MaxY()
        {

            if (Length <= 0)
                return 0.0f;

            return _points.Max(x => x.Y);
        }

        //********************************************************************
        // evenlySpaceContourPoints()
        //
        public FloatContour EvenlySpaceContourPoints(int space)
        {
            if (space <= 0)
                return null;

            float x, y, ccx, ccy, tx, ty, vx, vy, vsx, vsy;
            int i;
            double a, b, c, sqrt_b2_4ac, t1, t2, t, tLen;

            ccx = Points[0].X;
            ccy = Points[0].Y;
            i = 0;
            bool done = false;
            FloatContour newContour = new FloatContour();

            while (!done)
            {          // Contour length >= 2

                vsx = Points[i].X;
                vsy = Points[i].Y;
                x = Points[i + 1].X;
                y = Points[i + 1].Y;
                vx = x - vsx;
                vy = y - vsy;
                tx = x - ccx;
                ty = y - ccy;
                tLen = Math.Sqrt(tx * tx + ty * ty);
                if (tLen > space)
                {    // then evaluate intersection

                    tx = vsx - ccx;
                    ty = vsy - ccy;
                    a = vx * vx + vy * vy;
                    b = 2 * (vx * tx + vy * ty);    // 2 * dot(v, t)
                    c = tx * tx + ty * ty - space * space;
                    sqrt_b2_4ac = Math.Sqrt(b * b - 4 * a * c);

                    if (sqrt_b2_4ac < 0.0)  // should never happen - JHS
                        Trace.WriteLine("Neg Rad in FloatContour::evenlySpaceContourPoints()\n");

                    t1 = (-b + sqrt_b2_4ac) / (2 * a);
                    t2 = (-b - sqrt_b2_4ac) / (2 * a);
                    if ((t1 >= 0) && (t1 <= 1))
                        t = t1;
                    else if ((t2 >= 0) && (t2 <= 1))
                        t = t2;
                    else
                        t = 0;
                    // update circle ctr for next go
                    ccx = (float)(vsx + t * vx);
                    ccy = (float)(vsy + t * vy);
                    newContour.AddPoint(ccx, ccy);
                }
                else
                {
                    i++;
                }
                if (i >= Length - 1)
                    done = true;
            }

            return newContour;
        }

        /// <summary>
        /// This will do a specific mapping with p1 and desP1 as centers and will
        /// rotate, scale, and translate so that p1 and p2 are on top of desP1 and desP2,
        /// respectively. Note: The precision on this isn't great -- would probably be better
        /// to do the linear equation solving to find a transformation matrix similar to the 
        /// regular MapContour method.
        /// </summary>
        /// <param name="p1"></param>
        /// <param name="p2"></param>
        /// <param name="desP1"></param>
        /// <param name="desP2"></param>
        /// <returns></returns>
        public FloatContour MapContour2D(
            PointF p1,
            PointF p2,
            PointF desP1,
            PointF desP2)
        {
            float currentAngle = p1.FindAngle(p2);
            float desiredAngle = desP1.FindAngle(desP2);

            float degreesToRotate = desiredAngle - currentAngle;

            FloatContour transformedContour = this.Rotate(p1, degreesToRotate);

            var p2rotated = p2.Rotate(p1, degreesToRotate);

            // Figure out the scaling ratio
            double currentDistance = MathHelper.GetDistance(p1.X, p1.Y, p2.X, p2.Y);
            double targetDistance = MathHelper.GetDistance(desP1.X, desP1.Y, desP2.X, desP2.Y);

            double scaleRatio = targetDistance / currentDistance;

            // So we've rotated the contour, let's translate it so that p1 is on top of desP1
            // and scale them at the same time.
            double xDiff = desP1.X - p1.X * scaleRatio;
            double yDiff = desP1.Y - p1.Y * scaleRatio;

            foreach (var p in transformedContour.Points)
            {
                p.X = (float)((p.X + xDiff) * scaleRatio);
                p.Y = (float)((p.Y + yDiff) * scaleRatio);
            }

            return transformedContour;
        }

        public float[,] GetMappingTransform(
            PointF p1,
            PointF p2,
            PointF p3,
            PointF desP1,
            PointF desP2,
            PointF desP3)
        {
            // Well, we're basically trying to solve two systems of linear
            // equations here.  They are:
            // 
            // x'0 = a11 * x0 + a12 * y0 + a13
            // x'1 = a11 * x1 + a12 * y1 + a13
            // x'2 = a11 * x2 + a12 * y2 + a13
            //
            // and
            //
            // y'0 = a21 * x0 + a22 * y0 + a23
            // y'1 = a21 * x1 + a22 * y1 + a23
            // y'2 = a21 * x2 + a22 * y2 + a23
            //
            // So, we're going to put our known quantities in an augmented
            // matrix form, and solve for the a(rc)'s that will transform
            // the initial four points to the desired four points.

            float[,] a = new float[3, 3];
            float[,] b = new float[3, 4];

            for (int i = 0; i < 3; i++)
                a[i, 2] = 1.0f;

            a[0, 0] = p1.X;
            a[0, 1] = p1.Y;

            a[1, 0] = p2.X;
            a[1, 1] = p2.Y;

            a[2, 0] = p3.X;
            a[2, 1] = p3.Y;

            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    b[i, j] = a[i, j];
                }
            }

            b[0, 3] = (float)desP1.X;
            b[1, 3] = (float)desP2.X;
            b[2, 3] = (float)desP3.X;

            // First, we'll find the transformation coefficients
            // which will map the contour.
            LinearAlgebra.GaussJ(ref a, 3, ref b, 4);

            float[,] transformCoeff = new float[2, 3];

            for (int i = 0; i < 3; i++)
                transformCoeff[0, i] = b[i, 3];

            // Ok, now we have one row of coefficients.  We now need to reinitialize
            // the a and b matrices since the gaussj() solver function has changed
            // their values
            for (int i = 0; i < 3; i++)
                a[i, 2] = 1.0f;

            a[0, 0] = p1.X;
            a[0, 1] = p1.Y;

            a[1, 0] = p2.X;
            a[1, 1] = p2.Y;

            a[2, 0] = p3.X;
            a[2, 1] = p3.Y;

            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    b[i, j] = a[i, j];
                }
            }

            // This time, we're going to solve for the second row of
            // coefficients, so we'll put the desired y values in the augmented
            // section
            b[0, 3] = (float)desP1.Y;
            b[1, 3] = (float)desP2.Y;
            b[2, 3] = (float)desP3.Y;

            LinearAlgebra.GaussJ(ref a, 3, ref b, 4);

            // Save the transform coefficients
            for (int i = 0; i < 3; i++)
                transformCoeff[1, i] = b[i, 3];

            return transformCoeff;
        }

        public FloatContour TransformContour(float[,] transform, bool disableNotificationEvents = false)
        {
            // Now that we have all the required coefficients, we'll transform
            // the points in the original Contour, and store them in a new one
            FloatContour dstContour = new FloatContour(); //***008OL

            if (disableNotificationEvents)
                dstContour.DisableNotificationEvents();

            for (int i = 0; i < Length; i++)
            {
                float cx = this[i].X;
                float cy = this[i].Y;

                float x = transform[0, 0] * cx
                    + transform[0, 1] * cy
                    + transform[0, 2];

                float y = transform[1, 0] * cx
                    + transform[1, 1] * cy
                    + transform[1, 2];

                dstContour.AddPoint(x, y);
            }

            return dstContour;
        }

        public FloatContour MapContour(
            PointF p1,
            PointF p2,
            PointF p3,
            PointF desP1,
            PointF desP2,
            PointF desP3,
            bool disableNotificationEvents = false
        )
        {
            float[,] transformCoeff = GetMappingTransform(p1, p2, p3, desP1, desP2, desP3);

            return TransformContour(transformCoeff, disableNotificationEvents);
        }

        public static void FitContoursToSize(double drawingWidth, double drawingHeight, FloatContour unknownContour, FloatContour dbContour,
            out Contour displayUnknownContour, out Contour displayDBContour,
            out double xOffset, out double yOffset)
        {
            if (unknownContour == null && dbContour == null)
                throw new ArgumentNullException("Both contours null!");

            float xMax, yMax, xMin, yMin;

            if (unknownContour == null)
            {
                xMax = dbContour.MaxX();
                yMax = dbContour.MaxY();
                xMin = dbContour.MinX();
                yMin = dbContour.MinY();
            }
            else if (dbContour == null)
            {
                xMax = unknownContour.MaxX();
                yMax = unknownContour.MaxY();
                xMin = unknownContour.MinX();
                yMin = unknownContour.MinY();
            }
            else
            {
                xMax = (dbContour.MaxX() > (float)unknownContour.MaxX()) ? dbContour.MaxX() : (float)unknownContour.MaxX();
                yMax = (dbContour.MaxY() > (float)unknownContour.MaxY()) ? dbContour.MaxY() : (float)unknownContour.MaxY();
                xMin = (dbContour.MinX() < (float)unknownContour.MinX()) ? dbContour.MinX() : (float)unknownContour.MinX();
                yMin = (dbContour.MinY() < (float)unknownContour.MinY()) ? dbContour.MinY() : (float)unknownContour.MinY();
            }

            float
                xRange = xMax - xMin + 8, //***1.5 - added POINT_SIZE
                yRange = yMax - yMin + 8; //***1.5 - added POINT_SIZE

            float
                heightRatio = (float)(drawingWidth / yRange),
                widthRatio = (float)(drawingHeight / xRange);

            float ratio;
            if (heightRatio < widthRatio)
            {
                ratio = heightRatio;
                xOffset = (drawingWidth - ratio * xRange) / 2 - xMin * ratio;
                yOffset = 0 - yMin * ratio;
            }
            else
            {
                ratio = widthRatio;
                xOffset = 0 - xMin * ratio;
                yOffset = (drawingHeight - ratio * yRange) / 2 - yMin * ratio;
            }

            float ratioInverse = 1 / ratio;

            if (unknownContour == null)
                displayUnknownContour = null;
            else
                displayUnknownContour = new Contour(unknownContour, ratioInverse);

            if (dbContour == null)
                displayDBContour = null;
            else
                displayDBContour = new Contour(dbContour, ratioInverse);
        }

        //********************************************************************
        // print()
        //
        public void Print()
        {
            int j = Points.Count, i;
            for (i = 0; i < j; i++)
            {
                string fmtOut = string.Format("X: {0} Y: {1}", Points[i].X, Points[i].Y);
                Trace.Write(fmtOut);
            }
        }
    }
}
