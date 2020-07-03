using Darwin.Utilities;
using Darwin.Wavelet;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing.Text;
using System.Text;

namespace Darwin.Features
{
    public class BearFeatureSet : FeatureSet
    {
        private const int NasionTransformLevels = 6;

        private const int TipNotchMinDistance = 25;

        private const int NumChinMaxesToTrack = 5;
        private const float StartOfSnoutPaddingPercentage = 0.25f;

        private const float DesiredAngleForTipDetection = -18.0f;

        private static Dictionary<FeaturePointType, string> FeaturePointNameMapping = new Dictionary<FeaturePointType, string>()
        {
            { FeaturePointType.LeadingEdgeBegin, "Beginning of Head" },
            { FeaturePointType.LeadingEdgeEnd, "End of Leading Edge" },
            { FeaturePointType.Nasion, "Nasion" },
            { FeaturePointType.Tip, "Tip of Nose" },
            { FeaturePointType.Notch, "Under Nose Dent" },
            //{ FeaturePointType.Chin, "Chin" },
            { FeaturePointType.PointOfInflection, "End of Jaw" }
        };

        public BearFeatureSet()
        {
            FeatureSetType = FeatureSetType.Bear;

            FeaturePoints = new Dictionary<FeaturePointType, OutlineFeaturePoint>()
            {
                { FeaturePointType.LeadingEdgeBegin, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.LeadingEdgeBegin], Type = FeaturePointType.LeadingEdgeBegin, IsEmpty = true } },
                { FeaturePointType.LeadingEdgeEnd, new OutlineFeaturePoint { Ignore = true, Name = FeaturePointNameMapping[FeaturePointType.LeadingEdgeEnd], Type = FeaturePointType.LeadingEdgeEnd, IsEmpty = true } },
                { FeaturePointType.Nasion, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Nasion], Type = FeaturePointType.Nasion, IsEmpty = true } },
                { FeaturePointType.Tip, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Tip], Type = FeaturePointType.Tip, IsEmpty = true } },
                { FeaturePointType.Notch, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Notch], Type = FeaturePointType.Notch, IsEmpty = true } },
                { FeaturePointType.PointOfInflection, new OutlineFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.PointOfInflection], Type = FeaturePointType.PointOfInflection, IsEmpty = true } }
            };

            Features = new Dictionary<FeatureType, Feature>();
        }

        public BearFeatureSet(List<OutlineFeaturePoint> featurePoints)
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
        }

        public BearFeatureSet(Chain chain, FloatContour chainPoints)
            : this()
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));

            if (chainPoints == null)
                throw new ArgumentNullException(nameof(chainPoints));

            if (chainPoints.Length < 5)
                throw new Exception("FloatContour not long enough");

            int tipPos = FindTipOfNose(chain, chainPoints);
            int notchPos = FindNotch(chain, tipPos);

            if (notchPos - tipPos < TipNotchMinDistance)
            {
                // If the tip and notch are too close together, it's likely we have a really curved nose indent.
                // Let's try to find the tip again with a bit of padding
                tipPos = FindTipOfNose(chain, chainPoints, DefaultTipHighPointPadding, TipNotchMinDistance);
            }

            try
            {
                int nasionPos = FindNasion(chain, tipPos);
                int beginLE = FindBeginLE(chain, tipPos);
                int endLE = FindEndLE(chain, beginLE, tipPos);
                int endTE = FindPointOfInflection(chain, tipPos);

                FeaturePoints[FeaturePointType.LeadingEdgeBegin].Position = beginLE;
                FeaturePoints[FeaturePointType.LeadingEdgeEnd].Position = endLE;
                FeaturePoints[FeaturePointType.Nasion].Position = nasionPos;
                FeaturePoints[FeaturePointType.Tip].Position = tipPos;
                FeaturePoints[FeaturePointType.Notch].Position = notchPos;
                FeaturePoints[FeaturePointType.PointOfInflection].Position = endTE;
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
        
        public int FindTipOfNose(Chain chain,
            FloatContour chainPoints,
            int highPointPaddingLeft = DefaultTipHighPointPadding, int highPointPaddingRight = DefaultTipHighPointPadding)
        {
            float currentAngle = chainPoints[0].FindAngle(chainPoints.Points[chainPoints.Length - 1]);

            float degreesToRotate = DesiredAngleForTipDetection - currentAngle;

            FloatContour rotatedContour = chainPoints.Rotate(chainPoints[0], degreesToRotate);

            return FindTip(chain, rotatedContour, highPointPaddingLeft, highPointPaddingRight);
        }

        public int FindChin(Chain chain, int tipPos)
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));

            if (tipPos < 0 || tipPos > chain.Length)
                throw new Exception("findNotch() [int tipPosition]");

            // TODO
            //if (_userSetNotch)
            //    throw new Exception("findNotch() [User selected notch in use]");

            // We're only going to be looking on the "trailing edge" 
            // (after the nose) of the bear head for the chin, so we'll
            // build a source vector from that area
            int numTrailingEdgePts = chain.Length - tipPos - 1 - 5;
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
            Array.Copy(chain.RelativeData, tipPos + 1, src, 0, numTrailingEdgePts);

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
                        chinPosition = chain.Length - tipPos - 1;

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
                chinPosition = chain.Length - tipPos - 1;
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
                    chinPosition = chain.Length - 1 - tipPos;
            }

            return chinPosition + tipPos;
        }

        public int FindNasion(Chain chain, int tipPos)
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));

            if (tipPos < 0 || tipPos > chain.Length)
                throw new ArgumentOutOfRangeException(nameof(tipPos));

            int padding = (int)Math.Round(StartOfSnoutPaddingPercentage * tipPos);

            int numPoints = chain.Length;

            int numLeadingEdgePts = tipPos - 2 * padding;
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
    }
}
