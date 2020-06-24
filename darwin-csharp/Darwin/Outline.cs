//*******************************************************************
//   file: Outline.cxx
//
// author: J H Stewman & K R Debure
//
// This class consolidates in one place what was previously passed
// about as a collection of Contours, FloatContours, Chains, ...
//
//*******************************************************************

using Darwin.Features;
using Darwin.Utilities;
using Darwin.Wavelet;
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Darwin
{
    public class ZeroCrossing
    {
        public int Position { get; set; }
        public double LeftMag { get; set; }
        public double RightMag { get; set; }
    }

    public class Outline
    {
        private FloatContour _chainPoints;
        public FloatContour ChainPoints { get => _chainPoints; set => _chainPoints = value; }

        private Chain _chain;
        public Chain Chain { get => _chain; set => _chain = value; }

        public int Length
        {
            get
            {
                if (_chain == null)
                    return 0;

                //***008OL -- use chain length ?
                return _chain.Length;
            }
        }

        public FeatureSet FeatureSet { get; set; }

        private FloatContour _remappedChainPoints;
        public FloatContour RemappedChainPoints { get => _remappedChainPoints; set => _remappedChainPoints = value; }
        
        //public double LeadingEdgeAngle { get => _LEAngle; set => _LEAngle = value; }
        //public int TipPosition { get => _tipPos; set => _tipPos = value; }
        //public int NotchPosition { get => _notchPos; set => _notchPos = value; }
        //public int BeginLeadingEdgePosition { get => _beginLE; set => _beginLE = value; }
        //public int EndLeadingEdgePosition { get => _endLE; set => _endLE = value; }
        //public int EndTrailingEdgePosition { get => _endTE; set => _endTE = value; }

        public List<int> FeaturePointPositions
        {
            get
            {
                return FeatureSet.FeaturePointPositions;
            }
        }


        //***008OL following two functions moved here from Chain class
        PointF GetSavedPoint(int pointNum)
        {
            //if (pointNum < 0 || pointNum >= mNumPoints) 008OL removed
            if (pointNum < 0 || pointNum >= _chain.Length) //***008OL
                throw new ArgumentOutOfRangeException(nameof(pointNum));

            return _chainPoints[pointNum];
        }

        //////////////////////////////////////////////////////////////
        // Outline(Contour* c, double radius)
        //
        // Called from TraceWindow for initial creation of outline
        // 
        //  PRE: the Contour is already evenly spaced, normalized and 
        //       without knots
        // POST: all members are initialized
        //
        public Outline(Contour c, FeatureSetType featuresType)
        {
            _remappedChainPoints = null;
            _chainPoints = new FloatContour(); // ***008OL
            _chainPoints.ContourToFloatContour(c); //***008OL
            _chain = new Chain(_chainPoints);

            FeatureSet = FeatureSet.Create(featuresType, _chain, _chainPoints);

            c.SetFeaturePointPositions(FeaturePointPositions);
        }

        //////////////////////////////////////////////////////////////
        // Outline(FloatContour *fc)
        //
        // Called from DatabaseFin on read from DB file
        // 
        //  PRE: the FloatContour fc is already evenly spaced, normalized 
        //       and without knots
        // POST: all members are initialized & a copy is made of original
        //       FloatContour
        //
        public Outline(FloatContour fc, FeatureSetType featuresType)
        {
            // called prior to reading indices of each feature pt and the
            // user mod bits

            // Note, doesn't pass chain & chainpoints in, since we don't want to find them again
            FeatureSet = FeatureSet.Create(featuresType);

            _chainPoints = fc;
            _chain = new Chain(_chainPoints);
            _remappedChainPoints = null;
        }

        //////////////////////////////////////////////////////////////
        // Outline(Outline* outline)
        //
        // Called from ResultsWindow
        // 
        //  PRE: the outline has all members initialized
        // POST: all members are initialized, copies made of original
        //       Chain and FloatContour
        //
        public Outline(Outline outline)
        {
            FeatureSet = outline.FeatureSet;
            _remappedChainPoints = null;
            _chain = new Chain(outline.Chain);
            _chainPoints = new FloatContour(outline.ChainPoints); //***008OL
        }

        public void MapOutlineTo(Outline target)
        {
            FloatContour mappedContour = _chainPoints.MapContour(
                            GetFeaturePointCoords(FeaturePointType.Tip),
                            GetFeaturePointCoords(FeaturePointType.LeadingEdgeBegin),
                            GetFeaturePointCoords(FeaturePointType.Notch),
                            target.GetFeaturePointCoords(FeaturePointType.Tip),
                            target.GetFeaturePointCoords(FeaturePointType.LeadingEdgeBegin),
                            target.GetFeaturePointCoords(FeaturePointType.Notch));

            // At this point you need a new outline that contains a remapped
            // float contour for display.  But, if the new outline is
            // ultimately added to the database, you want it to contain the
            // unmapped float contour (the chainPoints).
            _remappedChainPoints = mappedContour;
        }

        //    If compute = true then ignore theta and call findLEAngle()
        //    else use theta as the angle value
        //public void SetLEAngle(double theta, bool compute)
        //{
        //    if (compute)
        //        _LEAngle = FindLEAngle();
        //    else
        //        _LEAngle = theta;
        //}

        // Attempts to determine the angle of the fin's leading edge.


        // inspectors for feature points
        public int GetFeaturePoint(FeaturePointType type)
        {
            if (type == FeaturePointType.NoFeature)
                throw new ArgumentOutOfRangeException(nameof(type));

            return FeatureSet.FeaturePoints[type].Position;
        }

        // inspectors for feature points
        public PointF GetFeaturePointCoords(FeaturePointType type)
        {
            if (type == FeaturePointType.NoFeature)
                throw new ArgumentOutOfRangeException(nameof(type));

            return _chainPoints[FeatureSet.FeaturePoints[type].Position];
        }

        // returns the type of feature point closest to the given point
        public int FindClosestFeaturePoint(PointF p)
        {
            return FindClosestFeaturePoint(p.X, p.Y);
        }

        public int FindClosestFeaturePoint(System.Drawing.Point p)
        {
            return FindClosestFeaturePoint(p.X, p.Y);
        }

        public int FindClosestFeaturePoint(float X, float Y)
        {
            int i, minFeature;
            double distToFeature, minDist;

            PointF feature = GetFeaturePointCoords(FeaturePointType.LeadingEdgeBegin);
            minDist = MathHelper.GetDistance(X, Y, feature.X, feature.Y);
            minFeature = (int)FeaturePointType.LeadingEdgeBegin;
            for (i = (int)FeaturePointType.LeadingEdgeEnd; i <= (int)FeaturePointType.PointOfInflection; i++)
            {
                PointF f = GetFeaturePointCoords((FeaturePointType)i);

                distToFeature = MathHelper.GetDistance(X, Y, f.X, f.Y);
                if (distToFeature < minDist)
                {
                    minDist = distToFeature;
                    minFeature = i;
                }
            }
            return minFeature;
        }

        // mutators for feature points
        public void SetFeaturePoint(FeaturePointType type, int position)
        {
            if (position < 0 || position >= _chain.Length)
                throw new ArgumentOutOfRangeException(nameof(position));

            //if (type < FeaturePointType.LeadingEdgeBegin || type > FeaturePointType.PointOfInflection)
            //    throw new Exception("setTip() [int type]");

            FeatureSet.SetFeaturePointPosition(type, position);
        }
    }
}
