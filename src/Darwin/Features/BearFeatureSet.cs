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

using Darwin.Database;
using Darwin.Helpers;
using Darwin.ML;
using Darwin.Utilities;
using Darwin.Wavelet;
using MathNet.Numerics.Interpolation;
using MathNet.Numerics.LinearAlgebra.Double;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Text;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;

namespace Darwin.Features
{
    public class BearFeatureSet : FeatureSet
    {
        private const float DesiredAngleForTipDetection = -18.0f;

        private const int TipNotchMinDistance = 25;

        private const int NasionTransformLevels = 6;
        private const float NasionPaddingPercentage = 0.25f;

        private const int UnderNoseDentNumMinsToTrack = 5;
        private const float UnderNoseDentAfterTipPadding = 0.10f;

        private const int UpperLipNumTransformsForMin = 6;
        private const float UpperLipEndPadding = 0.50f;
        private const float UpperLipDesiredAngle = -2;
        private const int UpperLipTipPadding = 40;

        private const int BrowBeforeMidpointPadding = 20;
        private const int BrowBeforeNasionPadding = 20;

        private const int NumChinMaxesToTrack = 5;

        private const float CurvatureLengthNormalizationLength = 250;
        private const int CurvatureEvenSpace = 3;
        private const int CurvatureTipPadding = 70;

        private static Dictionary<FeatureType, string> FeatureNameMapping = new Dictionary<FeatureType, string>()
        {
            { FeatureType.HasMouthDent, "Has Mouth Dent" },
            { FeatureType.BrowCurvature, "Brow Curvature" },
            { FeatureType.NasionDepth, "Nasion Depth" }
        };

        private static Dictionary<FeaturePointType, string> CoordinateFeaturePointNameMapping = new Dictionary<FeaturePointType, string>()
        {
            { FeaturePointType.Eye, "Eye" },
            { FeaturePointType.NasalLateralCommissure, "Nasal Lateral Commissure" }
        };

        private static Dictionary<FeaturePointType, string> FeaturePointNameMapping = new Dictionary<FeaturePointType, string>()
        {
            { FeaturePointType.LeadingEdgeBegin, "Beginning of Head" },
            { FeaturePointType.Brow, "Brow" },
            { FeaturePointType.LeadingEdgeEnd, "End of Leading Edge" },
            { FeaturePointType.Nasion, "Nasion" },
            { FeaturePointType.Tip, "Tip of Nose" },
            { FeaturePointType.Notch, "Under Nose Dent" },
            { FeaturePointType.UpperLip, "Upper Lip" },
            { FeaturePointType.BottomLipProtrusion, "Bottom Lip Protrusion" },
            //{ FeaturePointType.Chin, "Chin" },
            { FeaturePointType.PointOfInflection, "End of Jaw" }
        };

        public BearFeatureSet()
        {
            FeatureSetType = FeatureSetType.Bear;

            FeaturePoints = new Dictionary<FeaturePointType, OutlineFeaturePoint>()
            {
                { FeaturePointType.LeadingEdgeBegin, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.LeadingEdgeBegin], Type = FeaturePointType.LeadingEdgeBegin, IsEmpty = true } },
                { FeaturePointType.Brow, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Brow], Type = FeaturePointType.Brow, IsEmpty = true } },
                // Note the following item has Ignore = true
                { FeaturePointType.LeadingEdgeEnd, new OutlineFeaturePoint { Ignore = true, Name = FeaturePointNameMapping[FeaturePointType.LeadingEdgeEnd], Type = FeaturePointType.LeadingEdgeEnd, IsEmpty = true } },
                { FeaturePointType.Nasion, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Nasion], Type = FeaturePointType.Nasion, IsEmpty = true } },
                { FeaturePointType.Tip, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Tip], Type = FeaturePointType.Tip, IsEmpty = true } },
                { FeaturePointType.Notch, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Notch], Type = FeaturePointType.Notch, IsEmpty = true } },
                //{ FeaturePointType.Chin, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Chin], Type = FeaturePointType.Chin, IsEmpty = true } },
                { FeaturePointType.UpperLip, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.UpperLip], Type = FeaturePointType.UpperLip, IsEmpty = true } },
                { FeaturePointType.BottomLipProtrusion, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.BottomLipProtrusion], Type = FeaturePointType.BottomLipProtrusion, IsEmpty = true } },
                { FeaturePointType.PointOfInflection, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.PointOfInflection], Type = FeaturePointType.PointOfInflection, IsEmpty = true } }
            };

            Features = new Dictionary<FeatureType, Feature>()
            {
                { FeatureType.HasMouthDent, new Feature { Name = FeatureNameMapping[FeatureType.HasMouthDent], Type = FeatureType.HasMouthDent, IsEmpty = true } },
                { FeatureType.BrowCurvature, new Feature { Name = FeatureNameMapping[FeatureType.BrowCurvature], Type = FeatureType.BrowCurvature, IsEmpty = true } },
                { FeatureType.NasionDepth, new Feature { Name = FeatureNameMapping[FeatureType.NasionDepth], Type = FeatureType.NasionDepth, IsEmpty = true } }
            };

            CoordinateFeaturePoints = new Dictionary<FeaturePointType, CoordinateFeaturePoint>()
            {
                { FeaturePointType.Eye, new CoordinateFeaturePoint { Name = CoordinateFeaturePointNameMapping[FeaturePointType.Eye], Type = FeaturePointType.Eye, IsEmpty = true } },
                { FeaturePointType.NasalLateralCommissure, new CoordinateFeaturePoint { Name = CoordinateFeaturePointNameMapping[FeaturePointType.NasalLateralCommissure], Type = FeaturePointType.NasalLateralCommissure, IsEmpty = true } }
            };
        }

        public BearFeatureSet(List<OutlineFeaturePoint> featurePoints, List<CoordinateFeaturePoint> coordinateFeaturePoints, List<Feature> features)
            : this()
        {
            if (featurePoints == null)
                throw new ArgumentNullException(nameof(featurePoints));

            foreach (var fp in featurePoints)
            {
                // Note: This will ignore any features that are not in the FeaturePointNameMapping dictionary!
                if (FeaturePointNameMapping.ContainsKey(fp.Type))
                {
                    fp.Name = FeaturePointNameMapping[fp.Type];
                    FeaturePoints[fp.Type] = fp;
                }
            }

            if (coordinateFeaturePoints != null)
            {
                foreach (var cfp  in coordinateFeaturePoints)
                {
                    if (CoordinateFeaturePointNameMapping.ContainsKey(cfp.Type))
                    {
                        cfp.Name = CoordinateFeaturePointNameMapping[cfp.Type];
                        CoordinateFeaturePoints[cfp.Type] = cfp;
                    }
                }
            }

            if (features != null)
            {
                foreach (var f in features)
                {
                    if (FeatureNameMapping.ContainsKey(f.Type))
                    {
                        f.Name = FeatureNameMapping[f.Type];
                        Features[f.Type] = f;
                    }
                }
            }
        }

        public BearFeatureSet(Bitmap image, double scale, Chain chain, FloatContour chainPoints)
            : this()
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));

            if (chainPoints == null)
                throw new ArgumentNullException(nameof(chainPoints));

            if (chainPoints.Length < 5)
                throw new Exception("FloatContour isn't long enough to find feature points");

            int tipPosition = FindTipOfNose(chain, chainPoints);
            int underNoseDentPosition = FindUnderNoseDent(chain, tipPosition);

            if (underNoseDentPosition - tipPosition < TipNotchMinDistance)
            {
                // If the tip and under nose dent are too close together, it's likely we have a really curved nose indent.
                // Let's try to find the tip again with a bit of padding
                tipPosition = FindTipOfNose(chain, chainPoints, DefaultTipHighPointPadding, TipNotchMinDistance);
            }

            try
            {
                int nasionPos = FindNasion(chain, tipPosition);
                int beginLE = FindBeginLE(chain, tipPosition);
                int endLE = FindEndLE(chain, beginLE, tipPosition);
                int endTE = FindPointOfInflection(chain, tipPosition);

                bool hasMouthDent;
                int bottomLipProtrusion;
                int upperLipPosition = FindUpperLip(chain, chainPoints, underNoseDentPosition, out hasMouthDent, out bottomLipProtrusion);

                int browPosition = FindBrow(chain, beginLE, nasionPos);
                //int chinPosition = FindChin(chain, tipPosition);

                FeaturePoints[FeaturePointType.LeadingEdgeBegin].Position = beginLE;
                FeaturePoints[FeaturePointType.Brow].Position = browPosition;
                FeaturePoints[FeaturePointType.LeadingEdgeEnd].Position = endLE;
                FeaturePoints[FeaturePointType.Nasion].Position = nasionPos;
                FeaturePoints[FeaturePointType.Tip].Position = tipPosition;
                FeaturePoints[FeaturePointType.Notch].Position = underNoseDentPosition;
                FeaturePoints[FeaturePointType.UpperLip].Position = upperLipPosition;
                FeaturePoints[FeaturePointType.BottomLipProtrusion].Position = bottomLipProtrusion;
                //FeaturePoints[FeaturePointType.Chin].Position = chinPosition;
                FeaturePoints[FeaturePointType.PointOfInflection].Position = endTE;

                Features[FeatureType.HasMouthDent].Value = (hasMouthDent) ? 1.0 : 0.0;

                // Find the curvature between the brow and a spot near the nose. The reason for the padding is to try
                // to avoid taking the upturned noses some bears have into account.
                int curvatureTipPosition = tipPosition - CurvatureTipPadding;

                // Fallback to prevent an error.  We're going to get an error of 0 or close to it if we fall to this.
                if (curvatureTipPosition <= browPosition)
                {
                    curvatureTipPosition = browPosition + 1;
                    Trace.WriteLine("Fallback tip position to find curvature.");
                }
                Features[FeatureType.BrowCurvature].Value = FindCurvature(chainPoints, browPosition, curvatureTipPosition);
                Features[FeatureType.NasionDepth].Value = FindDepthOfNasion(chainPoints, browPosition, nasionPos, curvatureTipPosition);

                if (Options.CurrentUserOptions.FindCoordinateFeatures)
                {
                    var coordinates = MLSupport.PredictCoordinates(image, chainPoints, scale);

                    CoordinateFeaturePoints[FeaturePointType.Eye].Coordinate = new Point((int)Math.Round(coordinates[0]), (int)Math.Round(coordinates[1]));
                    CoordinateFeaturePoints[FeaturePointType.NasalLateralCommissure].Coordinate = new Point((int)Math.Round(coordinates[2]), (int)Math.Round(coordinates[3]));
                }

                // Fake the eye & nasal fold for right now (this at least gets the features on the display so we can move them)
                //CoordinateFeaturePoints[FeaturePointType.Eye].Coordinate = new Point((int)chainPoints[nasionPos].X - 60, (int)chainPoints[nasionPos].Y + 20);
                //CoordinateFeaturePoints[FeaturePointType.NasalLateralCommissure].Coordinate = new Point((int)chainPoints[tipPosition].X - 60, (int)chainPoints[tipPosition].Y + 60);
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex);
            }
        }

        public override void SetFeaturePointPosition(FeaturePointType featurePointType, int position)
        {
            if (!FeaturePoints.ContainsKey(featurePointType))
                throw new ArgumentOutOfRangeException(nameof(featurePointType));

            FeaturePoints[featurePointType].Position = position;
            FeaturePoints[featurePointType].UserSetPosition = true;
        }

        /// <summary>
        /// Finds the tip of the nose.  It relies mostly on the base FindTip, which also looks at the Y
        /// positions of the outline to narrow down the search area (looks for the highest points, which
        /// are the lowest Y numbers).  We want to allow the bear head to be traced sideways, but the Y
        /// coordinates of the tip of the nose won't be near the top in that case.  However, since the start
        /// of the head and the end are somewhat similar in most cases, we're going to rotate the contour
        /// so the begining and end are at a particular angle, so that the tip of the nose is at the highest
        /// point in Y coordinates.
        /// 
        /// The padding is for a second run.  Sometimes, the under nose dent is a really high max, so if we
        /// find the max way too close to our tip position after finding that point, we have likely misplaced
        /// the tip of the nose, so we run this again with padding.
        /// </summary>
        /// <param name="chain">Chain code of the outline</param>
        /// <param name="chainPoints">FloatContour points on the outline</param>
        /// <param name="highPointPaddingLeft">Padding for our search area to the "left" of the high Y point</param>
        /// <param name="highPointPaddingRight">Padding for our search area to the "right" of the high Y point</param>
        /// <returns>Index of the position of the tip of the nose</returns>
        public int FindTipOfNose(Chain chain,
                                 FloatContour chainPoints,
                                 int highPointPaddingLeft = DefaultTipHighPointPadding,
                                 int highPointPaddingRight = DefaultTipHighPointPadding)
        {
            float currentAngle = chainPoints[0].FindAngle(chainPoints.Points[chainPoints.Length - 1]);

            float degreesToRotate = DesiredAngleForTipDetection - currentAngle;

            FloatContour rotatedContour = chainPoints.Rotate(chainPoints[0], degreesToRotate);

            return FindTip(chain, rotatedContour, highPointPaddingLeft, highPointPaddingRight);
        }

        /// <summary>
        /// Tries to find the chin.  Currently doesn't work very well in most cases.
        /// Originally based on dolphin find notch, but looks for maxima instead of minima
        /// </summary>
        /// <param name="chain"></param>
        /// <param name="tipPosition"></param>
        /// <returns></returns>
        public int FindChin(Chain chain, int tipPosition)
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));

            if (tipPosition < 0 || tipPosition > chain.Length)
                throw new ArgumentOutOfRangeException(nameof(tipPosition));

            // TODO
            //if (_userSetNotch)
            //    throw new Exception("findNotch() [User selected notch in use]");

            // We're only going to be looking on the "trailing edge" 
            // (after the nose) of the bear head for the chin, so we'll
            // build a source vector from that area
            int numTrailingEdgePts = chain.Length - tipPosition - 1 - 5;
            double[] src = new double[numTrailingEdgePts];

            //***1.95 - JHS
            // The following original code copies the ABSOLUTE Chain angles into source.
            // These are probably in the range -180 .. 180.  Is this causing problems with
            // angles that cross quadrants between +180 and -180 thus producing a LARGE
            // positive change in angle rather than a small negative one.  Should the
            // angles all be SHIFTED by +180 so the range is 0..360?  NO, this does not
            // work.  However, converting ONLY the negative angles (-91..-180) to positive 
            // by adding 360 to them DOES WORK.  This way there is a continuous change in 
            // positive outline orientation throughout all leftward extending notches 
            // except narrow ones opening up and forward. In effect, we have removed potential
            // discontinuities from the chain signal being sent into the wavelet code.
            //memcpy(src, &((*_chain)[_tipPos + 1]), numTrailingEdgePts * sizeof(double));
            Array.Copy(chain.RelativeData, tipPosition + 1, src, 0, numTrailingEdgePts);

            //***1.95 - code to convert negative angles
            for (int di = 0; di < numTrailingEdgePts; di++)
            {
                if (src[di] < -90)
                    src[di] += 360.0; // force negative angles to be positive
                                      //printf ("%5.2f\n",src[di]);
            }

            // Now set up the variables needed to perform a wavelet
            // transform on the chain
            double[,] continuousResult = new double[TransformLevels + 1, MathHelper.NextPowerOfTwo(numTrailingEdgePts)];

            // Now perform the transformation
            WIRWavelet.WL_FrwtVector(
                    src,
                    ref continuousResult,
                    numTrailingEdgePts,
                    TransformLevels,
                    WaveletUtil.MZLowPassFilter,
                    WaveletUtil.MZHighPassFilter);

            int i;
            for (i = 1; i <= TransformLevels; i++)
                for (int j = 0; j < numTrailingEdgePts; j++)
                    continuousResult[i, j] *= WaveletUtil.NormalizationCoeff(i);

            double[,] modMax = new double[TransformLevels, numTrailingEdgePts];

            // ..and find its local minima and maxima
            for (i = 0; i < TransformLevels; i++)
            {
                double[] temp = new double[numTrailingEdgePts];

                double[] continousExtract = WaveletUtil.Extract1DArray(continuousResult, i + 1, numTrailingEdgePts);

                WaveletUtil.ModulusMaxima(continousExtract, ref temp, numTrailingEdgePts);

                WaveletUtil.Patch1DArray(temp, ref modMax, i, numTrailingEdgePts);
            }

            int level = TransformLevels / 2;

            if (level < 1)
                level = 1;

            // First, we'll find some local maxima at an intermediate level
            // to track.
            int chinPosition = 0;
            List<int> maxes = new List<int>();

            while (level > 0)
            {
                for (i = 0; i < numTrailingEdgePts; i++)
                    if (modMax[level, i] > 0.0)
                        maxes.Add(i);

                if (maxes.Count <= 0)
                {
                    level--;
                    continue;
                }

                if (maxes.Count == 1)
                {
                    chinPosition = 1;

                    if (0 == chinPosition)
                        chinPosition = chain.Length - tipPosition - 1;

                    break;
                }

                // yes, bad code
                break;
            }

            if (level == 0)
            {
                // Well, this really shouldn't happen: we've looked through
                // all the fine transform levels and haven't found any local
                // maxima.  So, we'll just set the chin position to the end
                // of the chain.
                chinPosition = chain.Length - tipPosition - 1;
                Trace.WriteLine("Warning: No local maxima found on trailing edge.  Set chin position to end of outline.");
            }

            if (chinPosition == 0)
            {
                // Now, we'll take the lowest few maxes, and look at how
                // they change over the transform levels.
                double[] maxVals = new double[maxes.Count];

                for (i = 0; i < maxes.Count; i++)
                    maxVals[i] = modMax[level, maxes[i]];

                Array.Sort(maxVals);

                // We want the highest maxes at the beginning of the array
                Array.Reverse(maxVals);

                int numMaxesToTrack;

                if ((int)maxes.Count < NumChinMaxesToTrack)
                    numMaxesToTrack = maxes.Count;
                else
                    numMaxesToTrack = NumChinMaxesToTrack;

                int[] positions = new int[numMaxesToTrack];

                for (int count = 0; count < numMaxesToTrack; count++)
                {
                    for (i = 0; i < maxes.Count; i++)
                    {
                        if (maxVals[count] == modMax[level, maxes[i]])
                        {
                            positions[count] = maxes[i];
                            break;
                        }
                    }
                }

                // Ok, now that we've got the few highest maxes,
                // let's find their corresponding positions in
                // a coarser level
                int coarserLevel = TransformLevels - 2;

                int correspondingPos;
                double difMax = 0.0, dif;
                bool firstRun = true;

                for (i = 0; i < numMaxesToTrack; i++)
                {
                    var extract = WaveletUtil.Extract1DArray(modMax, coarserLevel, numTrailingEdgePts);
                    correspondingPos = FindClosestMin(
                            5,
                            extract,
                            numTrailingEdgePts,
                            positions[i]);

                    // If we found a corresponding min in a coarser level...
                    if (-1 != correspondingPos)
                    {
                        dif = Math.Abs(modMax[coarserLevel, correspondingPos]);

                        if (firstRun)
                        {
                            firstRun = false;
                            difMax = dif;
                            chinPosition = positions[i];
                        }
                        else if (dif > difMax)
                        {
                            difMax = dif;
                            chinPosition = positions[i];
                        }
                    }
                }

                if (firstRun)
                    chinPosition = chain.Length - 1 - tipPosition;
            }

            return chinPosition + tipPosition;
        }

        /// <summary>
        /// Finds the Nasion, or the point on the head between the eyes.  It does this by
        /// finding the most significant minimum before the tip of the nose.
        /// </summary>
        /// <param name="chain"></param>
        /// <param name="tipPosition">Index of the tip of the nose</param>
        /// <returns>Integer index representing the position of the nasion</returns>
        public int FindNasion(Chain chain, int tipPosition)
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));

            if (tipPosition < 0 || tipPosition > chain.Length)
                throw new ArgumentOutOfRangeException(nameof(tipPosition));

            int padding = (int)Math.Round(NasionPaddingPercentage * tipPosition);

            int numPoints = chain.Length;

            int numLeadingEdgePts = tipPosition - 2 * padding;
            double[] src = new double[numLeadingEdgePts];

            Array.Copy(chain.Data, padding, src, 0, numLeadingEdgePts);

            int nextPowOfTwo = MathHelper.NextPowerOfTwo(numPoints - 1);

            // Now set up the variables needed to perform a wavelet transform on the chain
            double[,] continuousResult = new double[NasionTransformLevels + 1, nextPowOfTwo];

            // Now perform the transformation
            WIRWavelet.WL_FrwtVector(src,
                    ref continuousResult,
                    numLeadingEdgePts,
                    NasionTransformLevels,
                    WaveletUtil.MZLowPassFilter,
                    WaveletUtil.MZHighPassFilter);

            int
                nasionPosition = 0,
                level = NasionTransformLevels;

            while (level > 1)
            {
                // Find the maxima of the coefficients
                double[] modMax = new double[numLeadingEdgePts];

                for (int k = 0; k < numPoints - 1; k++)
                    continuousResult[level, k] *= WaveletUtil.NormalizationCoeff(level);

                double[] continousExtract = WaveletUtil.Extract1DArray(continuousResult, level, numLeadingEdgePts);
                WaveletUtil.ModulusMaxima(continousExtract, ref modMax, numLeadingEdgePts);

                // Now, find the largest positive max, which we'll
                // assume is the tip of the fin.

                if (nasionPosition == 0)
                {
                    // Find the greatest min
                    double min = double.MaxValue; //***1.6 - removed temporarily for tests
                    for (int i = 0; i < numLeadingEdgePts; i++)
                    { //***1.6 - removed temporarily for tests
                        if (modMax[i] < min)
                        {
                            min = modMax[i];
                            nasionPosition = i;
                        }
                    }
                }
                else
                {
                    var closestPosition = FindClosestMin(modMax, numLeadingEdgePts, nasionPosition);

                    if (closestPosition >= 0)
                        nasionPosition = closestPosition;
                }

                level--;
            }

            var correctedNasionPosition = nasionPosition + padding;
            Trace.WriteLine("Nasion position: " + correctedNasionPosition);

            if (correctedNasionPosition < 5)
                Trace.WriteLine("Probable bad nasion position.");

            return correctedNasionPosition;
        }

        /// <summary>
        /// Finds the position of the under nose dent.  Originally based on dorsal fin
        /// FindNotch function.  The regular FindNotch sometimes finds the dent created
        /// by the mouth.  We need the point to be more stable, though.  This is pretty
        /// simple, but it's looking for a minimum within UnderNoseDentAfterTipPadding
        /// percentage from the tip.  Seems to work pretty well in practice, but
        /// it's somewhat dependent on the outlines being of similar length.
        /// </summary>
        /// <param name="chain"></param>
        /// <param name="tipPosition">Index of the tip of the nose</param>
        /// <returns>Integer index representing the under nose dent</returns>
        public int FindUnderNoseDent(Chain chain, int tipPosition)
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));

            if (tipPosition < 0 || tipPosition > chain.Length)
                throw new ArgumentOutOfRangeException(nameof(tipPosition));

            // We're only going to be looking on the trailing edge
            // of the fin for the most significant notch, so we'll
            // build a source vector from that area
            //int numTrailingEdgePts = chain.Length - tipPosition - 1 - 5;
            int numTrailingEdgePts = (int)Math.Round(UnderNoseDentAfterTipPadding * chain.Length);

            if (numTrailingEdgePts > (chain.Length - tipPosition - 1 - 5))
                numTrailingEdgePts = chain.Length - tipPosition - 1 - 5;

            double[] src = new double[numTrailingEdgePts];

            //***1.95 - JHS
            // The following original code copies the ABSOLUTE Chain angles into source.
            // These are probably in the range -180 .. 180.  Is this causing problems with
            // angles that cross quadrants between +180 and -180 thus producing a LARGE
            // positive change in angle rather than a small negative one.  Should the
            // angles all be SHIFTED by +180 so the range is 0..360?  NO, this does not
            // work.  However, converting ONLY the negative angles (-91..-180) to positive 
            // by adding 360 to them DOES WORK.  This way there is a continuous change in 
            // positive outline orientation throughout all leftward extending notches 
            // except narrow ones opening up and forward. In effect, we have removed potential
            // discontinuities from the chain signal being sent into the wavelet code.
            //memcpy(src, &((*_chain)[_tipPos + 1]), numTrailingEdgePts * sizeof(double));
            Array.Copy(chain.Data, tipPosition + 1, src, 0, numTrailingEdgePts);

            //***1.95 - code to convert negative angles
            for (int di = 0; di < numTrailingEdgePts; di++)
            {
                if (src[di] < -90)
                    src[di] += 360.0; // force negative angles to be positive
                                      //printf ("%5.2f\n",src[di]);
            }

            // Now set up the variables needed to perform a wavelet
            // transform on the chain
            double[,] continuousResult = new double[TransformLevels + 1, MathHelper.NextPowerOfTwo(numTrailingEdgePts)];

            // Now perform the transformation
            WIRWavelet.WL_FrwtVector(
                    src,
                    ref continuousResult,
                    numTrailingEdgePts,
                    TransformLevels,
                    WaveletUtil.MZLowPassFilter,
                    WaveletUtil.MZHighPassFilter);

            int i;
            for (i = 1; i <= TransformLevels; i++)
                for (int j = 0; j < numTrailingEdgePts; j++)
                    continuousResult[i, j] *= WaveletUtil.NormalizationCoeff(i);

            double[,] modMax = new double[TransformLevels, numTrailingEdgePts];

            // ..and find its local minima and maxima
            for (i = 0; i < TransformLevels; i++)
            {
                double[] temp = new double[numTrailingEdgePts];

                double[] continousExtract = WaveletUtil.Extract1DArray(continuousResult, i + 1, numTrailingEdgePts);

                WaveletUtil.ModulusMaxima(continousExtract, ref temp, numTrailingEdgePts);

                WaveletUtil.Patch1DArray(temp, ref modMax, i, numTrailingEdgePts);
            }

            int level = TransformLevels / 2;

            if (level < 1) level = 1;

            // First, we'll find some local minima at an intermediate level
            // to track.
            int underNoseDentPosition = 0;
            List<int> mins = new List<int>();

            while (level > 0)
            {
                for (i = 0; i < numTrailingEdgePts; i++)
                    if (modMax[level, i] < 0.0)
                        mins.Add(i);

                if (mins.Count <= 0)
                {
                    level--;
                    continue;
                }

                if (mins.Count == 1)
                {
                    underNoseDentPosition = 1;

                    if (0 == underNoseDentPosition)
                        underNoseDentPosition = chain.Length - tipPosition - 1;

                    break;
                }

                // yes, bad code
                break;
            }

            if (level == 0)
                // Well, this really shouldn't happen: we've looked through
                // all the fine transform levels and haven't found any local
                // minima.  So, we'll just set the notch Position to the end
                // of the chain.
                underNoseDentPosition = chain.Length - tipPosition - 1;

            if (underNoseDentPosition == 0)
            {
                // Now, we'll take the lowest few mins, and look at how
                // they change over the transform levels.

                double[] minVals = new double[mins.Count];

                for (i = 0; i < mins.Count; i++)
                    minVals[i] = modMax[level, mins[i]];

                Array.Sort(minVals);

                int numMinsToTrack;

                if ((int)mins.Count < UnderNoseDentNumMinsToTrack)
                    numMinsToTrack = mins.Count;
                else
                    numMinsToTrack = UnderNoseDentNumMinsToTrack;

                int[] positions = new int[numMinsToTrack];

                for (int count = 0; count < numMinsToTrack; count++)
                {
                    for (i = 0; i < mins.Count; i++)
                    {
                        if (minVals[count] == modMax[level, mins[i]])
                        {
                            positions[count] = mins[i];
                            break;
                        }
                    }
                }

                // Ok, now that we've got the few lowest mins,
                // let's find their corresponding positions in
                // a coarser level
                int coarserLevel = TransformLevels - 2;

                int correspondingPos;
                //double alphaMax, alpha;
                double difMax = 0.0, dif;
                bool firstRun = true;

                for (i = 0; i < numMinsToTrack; i++)
                {
                    var extract = WaveletUtil.Extract1DArray(modMax, coarserLevel, numTrailingEdgePts);
                    correspondingPos = FindClosestMin(
                            5,
                            extract,
                            numTrailingEdgePts,
                            positions[i]);

                    // If we found a corresponding min in a coarser
                    // level...
                    if (-1 != correspondingPos)
                    {
                        dif = Math.Abs(modMax[coarserLevel, correspondingPos]);

                        if (firstRun)
                        {
                            firstRun = false;
                            difMax = dif;
                            underNoseDentPosition = positions[i];
                        }
                        else if (dif > difMax)
                        {
                            difMax = dif;
                            underNoseDentPosition = positions[i];
                        }
                    }
                }

                if (firstRun)
                    underNoseDentPosition = chain.Length - 1 - tipPosition;
            }

            return underNoseDentPosition + tipPosition;
        }

        /// <summary>
        /// Tries to find the position of the upper lip on the outline.  If there's a mouth dent,
        /// it'll return that (and set the output parameter hasMouthDent to true).  If there isn't,
        /// it'll return the greatest max to the right of the mouth dent.
        /// </summary>
        /// <param name="chain"></param>
        /// <param name="chainPoints"></param>
        /// <param name="underNoseDentPosition"></param>
        /// <param name="hasMouthDent"></param>
        /// <param name="mouthDentPosition"></param>
        /// <returns></returns>
        public int FindUpperLip(Chain chain, FloatContour chainPoints, int underNoseDentPosition, out bool hasMouthDent, out int bottomLipProtrusion)
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));

            if (underNoseDentPosition < 0 || underNoseDentPosition > chain.Length)
                throw new ArgumentOutOfRangeException(nameof(underNoseDentPosition));

            hasMouthDent = false;

            int numPointsAfterNoseDent = (int)Math.Round((chain.Length - underNoseDentPosition) * (1.0f - UpperLipEndPadding));

            // We're going to transform the whole chain
            int numPoints = chain.Length;

            // First, make a copy without the first value in the chain,
            // since the first value skews the rest of the chain and is
            // unnecessary for our purposes here
            double[] src = new double[numPoints - 1];

            Array.Copy(chain.Data, 1, src, 0, numPoints - 1);

            int nextPowOfTwo = MathHelper.NextPowerOfTwo(numPoints - 1);

            // Now set up the variables needed to perform a wavelet transform on the chain
            double[,] continuousResult = new double[UpperLipNumTransformsForMin + 1, nextPowOfTwo];
            WIRWavelet.WL_FrwtVector(src,
                ref continuousResult,
                numPoints - 1,
                UpperLipNumTransformsForMin,
                WaveletUtil.MZLowPassFilter,
                WaveletUtil.MZHighPassFilter);

            for (int i = 1; i <= UpperLipNumTransformsForMin; i++)
                for (int j = 0; j < numPoints - 1; j++)
                    continuousResult[i, j] *= WaveletUtil.NormalizationCoeff(i);

            double[] modMax = new double[numPoints - 1];

            for (int k = 0; k < numPoints - 1; k++)
                continuousResult[UpperLipNumTransformsForMin, k] *= WaveletUtil.NormalizationCoeff(UpperLipNumTransformsForMin);

            double[] continousExtract = WaveletUtil.Extract1DArray(continuousResult, UpperLipNumTransformsForMin, numPoints - 1);
            WaveletUtil.ModulusMaxima(continousExtract, ref modMax, numPoints - 1);

            // Now, let's see if we have any mins at the top transform level.  If we do, we probably have a mouth dent. If we don't,
            // we might not.
            for (int k = numPointsAfterNoseDent + underNoseDentPosition; k > underNoseDentPosition; k--)
            {
                if (modMax[k] < 0.0)
                {
                    hasMouthDent = true;
                    break;
                }
            }

            int mouthDentPosition = 0;

            if (hasMouthDent)
            {
                mouthDentPosition = FindNotch(chain, underNoseDentPosition + UpperLipTipPadding);

                // Check where it found the mouth dent -- if it's too close to the end, it's likely that it's
                // not the mouth, but something like the neck/etc.
                if (mouthDentPosition < underNoseDentPosition + numPointsAfterNoseDent)
                {
                    bottomLipProtrusion = FindTip(chain, mouthDentPosition + 20, mouthDentPosition - underNoseDentPosition - UpperLipTipPadding, UpperLipTipPadding);
                    return mouthDentPosition;
                }
            }

            // Fake the mouth dent position as a starting point for finding the biggest max in the area
            mouthDentPosition = underNoseDentPosition + numPointsAfterNoseDent;

            int upperLipPosition = FindTip(chain, mouthDentPosition, mouthDentPosition - underNoseDentPosition - UpperLipTipPadding, UpperLipTipPadding);

            // These are fallback positions, so they're going to be off if we hit this if statement.
            if (upperLipPosition < underNoseDentPosition)
                upperLipPosition = (int)Math.Round(mouthDentPosition + numPointsAfterNoseDent / 2.0f);

            bottomLipProtrusion = upperLipPosition;

            return upperLipPosition;
        }
        
        /// <summary>
        /// Finds the brow as the biggest max between the beginning of the leading edge and the nasion.
        /// </summary>
        /// <param name="chain"></param>
        /// <param name="beginningOfLeadingEdge"></param>
        /// <param name="nasionPosition"></param>
        /// <returns></returns>
        public int FindBrow(Chain chain, int beginningOfLeadingEdge, int nasionPosition)
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));

            if (beginningOfLeadingEdge < 0 || beginningOfLeadingEdge > nasionPosition)
                throw new ArgumentOutOfRangeException(nameof(beginningOfLeadingEdge));

            if (nasionPosition < 0 || nasionPosition >= chain.Length)
                throw new ArgumentOutOfRangeException(nameof(nasionPosition));

            int segmentLength = nasionPosition - beginningOfLeadingEdge;
            int midPoint = (int)Math.Round(segmentLength / 2.0);

            int rightPadding = nasionPosition - midPoint - BrowBeforeNasionPadding;

            if (rightPadding < 0)
                rightPadding = 50;

            return FindTip(chain, midPoint, BrowBeforeMidpointPadding, rightPadding);
        }

        /// <summary>
        /// Finds the depth of the nasion, or the distance from the nasion to the line
        /// created by the brow and the offset nose position.  The contour is scaled to
        /// a magic number first so that these distances are hopefully comparable between
        /// samples.
        /// </summary>
        /// <param name="chainPoints"></param>
        /// <param name="browPosition"></param>
        /// <param name="nasionPosition"></param>
        /// <param name="offsetTipOfNose"></param>
        /// <returns></returns>
        public double FindDepthOfNasion(FloatContour chainPoints, int browPosition, int nasionPosition, int offsetTipOfNose)
        {
            if (chainPoints == null)
                throw new ArgumentNullException(nameof(chainPoints));

            if (browPosition < 0 || browPosition > nasionPosition)
                throw new ArgumentOutOfRangeException(nameof(browPosition));

            if (nasionPosition < 0 || nasionPosition > offsetTipOfNose)
                throw new ArgumentOutOfRangeException(nameof(nasionPosition));

            if (offsetTipOfNose < 0 || offsetTipOfNose >= chainPoints.Length)
                throw new ArgumentOutOfRangeException(nameof(offsetTipOfNose));

            double currentPositionDistance = MathHelper.GetDistance(chainPoints[browPosition].X,
                chainPoints[browPosition].Y, chainPoints[offsetTipOfNose].X, chainPoints[offsetTipOfNose].Y);

            float ratio = (float)(CurvatureLengthNormalizationLength / currentPositionDistance);

            PointF scaledNasionPosition = new PointF(chainPoints[nasionPosition].X * ratio, chainPoints[nasionPosition].Y * ratio);

            FloatContour scaledContour = new FloatContour();
            for (int i = browPosition; i <= offsetTipOfNose; i++)
                scaledContour.AddPoint(chainPoints[i].X * ratio, chainPoints[i].Y * ratio);

            scaledContour = scaledContour.EvenlySpaceContourPoints(CurvatureEvenSpace);

            // Now we're going to fit a simple line to our start and end points
            float slope = (scaledContour[scaledContour.Length - 1].Y - scaledContour[0].Y) / (scaledContour[scaledContour.Length - 1].X - scaledContour[0].X);
            float intercept = scaledContour[0].Y - slope * scaledContour[0].X;

            // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
            var nasionDistance = Math.Abs(slope * scaledNasionPosition.X - scaledNasionPosition.Y + intercept) /
                Math.Sqrt(Math.Pow(slope, 2) + 1 /* (-1 squared) */);

            Trace.WriteLine("Nasion depth: " + nasionDistance);

            return nasionDistance;
        }

        /// <summary>
        /// Find a rough approximation of the curvature of a FloatContour.
        /// Scales the contour length for the first and second positions to a benchmark
        /// and then finds the mean squared error between that contour and a line fit
        /// between the first and second points.
        /// </summary>
        /// <param name="chainPoints"></param>
        /// <param name="firstPosition"></param>
        /// <param name="secondPosition"></param>
        /// <returns></returns>
        public double FindCurvature(FloatContour chainPoints, int firstPosition, int secondPosition)
        {
            if (chainPoints == null)
                throw new ArgumentNullException(nameof(chainPoints));

            if (firstPosition < 0 || firstPosition > secondPosition)
                throw new ArgumentOutOfRangeException(nameof(firstPosition));

            if (secondPosition < 0 || secondPosition >= chainPoints.Length)
                throw new ArgumentOutOfRangeException(nameof(secondPosition));

            //// Fit a spline to our points
            //var spline = CubicSpline.InterpolateNatural(
            //    chainPoints.Points.Skip(beginningOfLeadingEdge)
            //                      .Take(nasionPosition - beginningOfLeadingEdge)
            //                      .Select(p => (double)p.X),
            //    chainPoints.Points.Skip(beginningOfLeadingEdge)
            //                      .Take(nasionPosition - beginningOfLeadingEdge)
            //                      .Select(p => (double)p.Y));

            //const int column_width = 10;
            //for (int i = beginningOfLeadingEdge; i < nasionPosition; i++)
            //{
            //    double dydx = spline.Differentiate2(chainPoints[i].X);
            //}

            //CurvatureLengthNormalizationLength

            double currentPositionDistance = MathHelper.GetDistance(chainPoints[firstPosition].X,
                chainPoints[firstPosition].Y, chainPoints[secondPosition].X, chainPoints[secondPosition].Y);

            float ratio = (float)(CurvatureLengthNormalizationLength / currentPositionDistance);

            FloatContour scaledContour = new FloatContour();

            for (int i = firstPosition; i <= secondPosition; i++)
            {
                scaledContour.AddPoint(chainPoints[i].X * ratio, chainPoints[i].Y * ratio);
            }

            scaledContour = scaledContour.EvenlySpaceContourPoints(CurvatureEvenSpace);

            //double checkDistance = MathHelper.GetDistance(scaledContour[0].X, scaledContour[0].Y,
            //    scaledContour[scaledContour.Length - 1].X, scaledContour[scaledContour.Length - 1].Y);

            // Now we're going to fit a simple line to our start and end points
            float slope = (scaledContour[scaledContour.Length - 1].Y - scaledContour[0].Y) / (scaledContour[scaledContour.Length - 1].X - scaledContour[0].X);
            float intercept = scaledContour[0].Y - slope * scaledContour[0].X;

            double squaredError = 0f;

            // And find the mean squared error of the Y distance between the line and the contour
            foreach (var p in scaledContour.Points)
            {
                squaredError += Math.Pow(p.Y - (p.X * slope + intercept), 2);
            }

            double meanSquaredError = squaredError / scaledContour.Length;

            Trace.WriteLine("Curvature MSE: " + meanSquaredError.ToString("N2"));

            return meanSquaredError;
        }
    }
}
