﻿//*******************************************************************
//   file: FloatContour.cxx
//
// author: 
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

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

        //********************************************************************
        // FloatContour()
        //
        public FloatContour()  //***006FC revised
        {
        }

        //********************************************************************
        // FloatContour()
        //
        public FloatContour(FloatContour contour)  //***006FC new
        {
            _points = new ObservableNotifiableCollection<PointF>();

            foreach (var p in contour.Points)
                _points.Add(new PointF(p.X, p.Y, p.Z));
        }

        //********************************************************************
        // operator=()
        //
        //FloatContour& FloatContour::operator=(FloatContour& fc)
        //{ //***006FC new

        //    if (this == &fc)
        //        return *this;

        //    // empty this mPointVector
        //    mPointVector.erase(mPointVector.begin(), mPointVector.end());

        //    // copy items over from fc.mPointVector
        //    vector<point_t>::iterator it;
        //    it = fc.mPointVector.begin();
        //    while (it != fc.mPointVector.end())
        //        mPointVector.push_back(*it);

        //    return *this;
        //}


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

        //********************************************************************
        // FindIndexFromChainPoint()
        // -- Given an <X,Y> Location from the chain, returns the closest
        //    index into the Contour
        //
        public int FindIndexFromChainPoint(float desx, float desy)
        {

            float x = 0.0f, y = 0.0f;
            double diff = 0.0, ldiff = 0.0;
            int lowest = 0;

            for (var i = 0; i < Length; i++)
            {
                x = _points[i].X;
                y = _points[i].Y;
                diff = MathHelper.GetDistance(desx, desy, x, y);
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
        private FloatContour EvenlySpaceContourPoints(int space)
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

        public FloatContour MapContour(
            PointF p1,
            PointF p2,
            PointF p3,
            PointF desP1,
            PointF desP2,
            PointF desP3
        )
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

            //FloatContour *dstContour = NULL; removed 008OL
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
                b[i, i] = a[i, i];
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
                b[i, i] = a[i, i];

            // and, obviously, this time, we're going to solve for the second row of
            // coefficients, so we'll put the desired y values in the augmented
            // section
            b[0, 3] = (float)desP1.Y;
            b[1, 3] = (float)desP2.Y;
            b[2, 3] = (float)desP3.Y;

            LinearAlgebra.GaussJ(ref a, 3, ref b, 4);

            // Save the transform coefficients
            for (int i = 0; i < 3; i++)
                transformCoeff[1, i] = b[i, 3];

            // Now that we have all the required coefficients, we'll transform
            // the points in the original Contour, and store them in a new one


            FloatContour dstContour = new FloatContour(); //***008OL
            int numPoints = Length;
            //int cx, cy; removed 008OL
            float cx, cy; //***008OL
            float x, y;
            for (int i = 0; i < numPoints; i++)
            {
                cx = this[i].X;
                cy = this[i].Y;

                x = transformCoeff[0, 0] * cx
                    + transformCoeff[0, 1] * cy
                    + transformCoeff[0, 2];
                y = transformCoeff[1, 0] * cx
                    + transformCoeff[1, 1] * cy
                    + transformCoeff[1, 2];

                dstContour.AddPoint(x, y);
            }

            return dstContour;
        }

        //***006FC next function moved from header

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
