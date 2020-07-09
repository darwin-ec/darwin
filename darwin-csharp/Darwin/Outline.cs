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
using System.Linq;

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

        public Outline(FloatContour fc, FeatureSetType featuresType, FeatureSet features)
        {
            // called prior to reading indices of each feature pt and the
            // user mod bits

            // Note, doesn't pass chain & chainpoints in, since we don't want to find them again
            FeatureSet = features;

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

        public void RediscoverFeaturePoints(FeatureSetType featuresType)
        {
            FeatureSet = FeatureSet.Create(featuresType, _chain, _chainPoints);
        }

        public bool ContainsAllFeaturePointTypes(List<FeaturePointType> featurePointTypes)
        {
            if (featurePointTypes == null || featurePointTypes.Count < 1)
                return true;

            if (FeatureSet == null || FeatureSet.FeaturePoints == null)
                return false;

            foreach (var type in featurePointTypes)
            {
                if (!FeatureSet.FeaturePoints.ContainsKey(type))
                    return false;
            }

            return true;
        }

        // inspectors for feature points
        public int GetFeaturePoint(FeaturePointType type)
        {
            if (type == FeaturePointType.NoFeature)
                throw new ArgumentOutOfRangeException(nameof(type));

            if (!FeatureSet.FeaturePoints.ContainsKey(type))
                return 0;

            return FeatureSet.FeaturePoints[type].Position;
        }

        // inspectors for feature points
        public PointF GetFeaturePointCoords(FeaturePointType type)
        {
            if (type == FeaturePointType.NoFeature)
                throw new ArgumentOutOfRangeException(nameof(type));

            return _chainPoints[FeatureSet.FeaturePoints[type].Position];
        }

        public PointF GetRemappedFeaturePointCoords(FeaturePointType type)
        {
            if (type == FeaturePointType.NoFeature)
                throw new ArgumentOutOfRangeException(nameof(type));

            if (_remappedChainPoints != null)
                return _remappedChainPoints[FeatureSet.FeaturePoints[type].Position];

            return _chainPoints[FeatureSet.FeaturePoints[type].Position];
        }

        // returns the type of feature point closest to the given point
        public OutlineFeaturePoint FindClosestFeaturePoint(PointF p)
        {
            return FindClosestFeaturePoint(p.X, p.Y);
        }

        public OutlineFeaturePoint FindClosestFeaturePoint(System.Drawing.Point p)
        {
            return FindClosestFeaturePoint(p.X, p.Y);
        }

        public void GetNeighboringFeaturePositions(FeaturePointType type, out int previous, out int next)
        {
            if (FeatureSet == null || FeatureSet.FeaturePoints == null || !FeatureSet.FeaturePoints.ContainsKey(type))
            {
                previous = 0;
                next = Length - 1;
                return;
            }
            
            int currentPosition = GetFeaturePoint(type);

            previous = FeatureSet.FeaturePoints.Values
                .Where(fp => !fp.Ignore && fp.Type != type && fp.Position < currentPosition)
                .Select(fp => fp.Position)
                .DefaultIfEmpty()
                .Max();

            next = FeatureSet.FeaturePoints.Values
                .Where(fp => !fp.Ignore &&  fp.Type != type && fp.Position > currentPosition)
                .Select(fp => fp.Position)
                .DefaultIfEmpty()
                .Min();

            if (next == 0)
                next = Length - 1;
        }

        public OutlineFeaturePoint FindClosestFeaturePoint(float X, float Y)
        {
            var minFeature = OutlineFeaturePoint.Empty;
            double? minDist = null;

            foreach (var f in FeatureSet.FeaturePoints.Values.Where(fp => !fp.Ignore))
            {
                var coords = GetFeaturePointCoords(f.Type);
                var distToFeature = MathHelper.GetDistance(X, Y, coords.X, coords.Y);

                if (minDist == null || distToFeature < minDist)
                {
                    minDist = distToFeature;
                    minFeature = f;
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
