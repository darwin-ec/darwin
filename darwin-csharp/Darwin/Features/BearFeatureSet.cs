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
        private const int NumChinMaxesToTrack = 5;
        private const float StartOfSnoutPaddingPercentage = 0.30f;
        private const int StartOfSnoutNumMinsToTrack = 5;

        private static Dictionary<FeaturePointType, string> FeaturePointNameMapping = new Dictionary<FeaturePointType, string>()
        {
            { FeaturePointType.LeadingEdgeBegin, "Beginning of Head" },
            { FeaturePointType.LeadingEdgeEnd, "End of Leading Edge" },
            { FeaturePointType.StartOfSnout, "Start of Snout" },
            { FeaturePointType.Tip, "Tip of Nose" },
            { FeaturePointType.Chin, "Chin" },
            { FeaturePointType.PointOfInflection, "End of Jaw" }
        };

        public BearFeatureSet()
        {
            FeatureSetType = FeatureSetType.Bear;

            FeaturePoints = new Dictionary<FeaturePointType, FeaturePoint>()
            {
                { FeaturePointType.LeadingEdgeBegin, new FeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.LeadingEdgeBegin], Type = FeaturePointType.LeadingEdgeBegin, IsEmpty = true } },
                { FeaturePointType.LeadingEdgeEnd, new FeaturePoint { Ignore = true, Name = FeaturePointNameMapping[FeaturePointType.LeadingEdgeEnd], Type = FeaturePointType.LeadingEdgeEnd, IsEmpty = true } },
                { FeaturePointType.StartOfSnout, new FeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.StartOfSnout], Type = FeaturePointType.StartOfSnout, IsEmpty = true } },
                { FeaturePointType.Tip, new FeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Tip], Type = FeaturePointType.Tip, IsEmpty = true } },
                { FeaturePointType.Chin, new FeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Chin], Type = FeaturePointType.Chin, IsEmpty = true } },
                { FeaturePointType.PointOfInflection, new FeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.PointOfInflection], Type = FeaturePointType.PointOfInflection, IsEmpty = true } }
            };

            Features = new Dictionary<FeatureType, Feature>();
        }

        public BearFeatureSet(List<FeaturePoint> featurePoints)
            : this()
        {
            if (featurePoints == null)
                throw new ArgumentNullException(nameof(featurePoints));

            foreach (var fp in featurePoints)
            {
                fp.Name = FeaturePointNameMapping[fp.Type];
                FeaturePoints[fp.Type] = fp;
            }
        }

        public BearFeatureSet(Chain chain, FloatContour chainPoints)
            : this()
        {
            int tipPos = FindTip(chain, chainPoints);
            int chinPos = FindChin(chain, tipPos);
            int startSnoutPos = FindStartOfSnout(chain, tipPos);
            int beginLE = FindBeginLE(chain, tipPos);
            int endLE = FindEndLE(chain, beginLE, tipPos);
            int endTE = FindPointOfInflection(chain, tipPos);

            FeaturePoints[FeaturePointType.LeadingEdgeBegin].Position = beginLE;
            FeaturePoints[FeaturePointType.LeadingEdgeEnd].Position = endLE;
            FeaturePoints[FeaturePointType.StartOfSnout].Position = startSnoutPos;
            FeaturePoints[FeaturePointType.Tip].Position = tipPos;
            FeaturePoints[FeaturePointType.Chin].Position = chinPos;
            FeaturePoints[FeaturePointType.PointOfInflection].Position = endTE;
        }

        public override void SetFeaturePointPosition(FeaturePointType featurePointType, int position)
        {
            if (!FeaturePoints.ContainsKey(featurePointType))
                throw new ArgumentOutOfRangeException(nameof(featurePointType));

            FeaturePoints[featurePointType].Position = position;
            FeaturePoints[featurePointType].UserSetPosition = true;
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

        // This is based on dolphin FindNotch, but on the leading side
        public int FindStartOfSnout(Chain chain, int tipPos)
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));

            if (tipPos < 0 || tipPos > chain.Length)
                throw new ArgumentOutOfRangeException(nameof(tipPos));

            int padding = (int)Math.Round(StartOfSnoutPaddingPercentage * tipPos);

            // TODO
            //if (_userSetNotch)
            //    throw new Exception("findNotch() [User selected notch in use]");

            // We're only going to be looking on the "leading edge"
            // of the bear head, so we're going to create an array from that data
            // with a little padding
            int numLeadingEdgePts = tipPos - 2 * padding;
            double[] src = new double[numLeadingEdgePts];

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
            Array.Copy(chain.Data, padding, src, 0, numLeadingEdgePts);

            //***1.95 - code to convert negative angles
            for (int di = 0; di < numLeadingEdgePts; di++)
            {
                if (src[di] < -90)
                    src[di] += 360.0; // force negative angles to be positive
                                      //printf ("%5.2f\n",src[di]);
            }

            // Now set up the variables needed to perform a wavelet
            // transform on the chain
            double[,] continuousResult = new double[TransformLevels + 1, MathHelper.NextPowerOfTwo(numLeadingEdgePts)];

            // Now perform the transformation
            WIRWavelet.WL_FrwtVector(
                    src,
                    ref continuousResult,
                    numLeadingEdgePts,
                    TransformLevels,
                    WaveletUtil.MZLowPassFilter,
                    WaveletUtil.MZHighPassFilter);

            int i;
            for (i = 1; i <= TransformLevels; i++)
                for (int j = 0; j < numLeadingEdgePts; j++)
                    continuousResult[i, j] *= WaveletUtil.NormalizationCoeff(i);

            double[,] modMax = new double[TransformLevels, numLeadingEdgePts];

            // ..and find its local minima and maxima
            for (i = 0; i < TransformLevels; i++)
            {
                double[] temp = new double[numLeadingEdgePts];

                double[] continousExtract = WaveletUtil.Extract1DArray(continuousResult, i + 1, numLeadingEdgePts);

                WaveletUtil.ModulusMaxima(continousExtract, ref temp, numLeadingEdgePts);

                WaveletUtil.Patch1DArray(temp, ref modMax, i, numLeadingEdgePts);
            }

            int level = TransformLevels / 2;

            if (level < 1) level = 1;

            // First, we'll find some local minima at an intermediate level
            // to track.
            int startOfSnoutPosition = 0;
            List<int> mins = new List<int>();

            while (level > 0)
            {
                for (i = 0; i < numLeadingEdgePts; i++)
                    if (modMax[level, i] < 0.0)
                        mins.Add(i);

                if (mins.Count <= 0)
                {
                    level--;
                    continue;
                }

                if (mins.Count == 1)
                {
                    startOfSnoutPosition = 1;

                    if (0 == startOfSnoutPosition)
                        startOfSnoutPosition = chain.Length - tipPos - 1;

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
                startOfSnoutPosition = chain.Length - tipPos - 1;

            if (startOfSnoutPosition == 0)
            {
                // Now, we'll take the lowest few mins, and look at how
                // they change over the transform levels.

                double[] minVals = new double[mins.Count];

                for (i = 0; i < mins.Count; i++)
                    minVals[i] = modMax[level, mins[i]];

                Array.Sort(minVals);

                int numMinsToTrack;

                if ((int)mins.Count < StartOfSnoutNumMinsToTrack)
                    numMinsToTrack = mins.Count;
                else
                    numMinsToTrack = StartOfSnoutNumMinsToTrack;

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

                double difMax = 0.0, dif;
                bool firstRun = true;

                for (i = 0; i < numMinsToTrack; i++)
                {
                    var extract = WaveletUtil.Extract1DArray(modMax, coarserLevel, numLeadingEdgePts);
                    correspondingPos = FindClosestMin(
                            5,
                            extract,
                            numLeadingEdgePts,
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
                            startOfSnoutPosition = positions[i];
                        }
                        else if (dif > difMax)
                        {
                            difMax = dif;
                            startOfSnoutPosition = positions[i];
                        }
                    }
                }

                if (firstRun)
                    startOfSnoutPosition = (int)Math.Round(tipPos * 2 * StartOfSnoutPaddingPercentage);
            }

            /*
			list<ZeroCrossing> zeroCrossings =
				findZeroCrossings(continuousResult[level], numTrailingEdgePts);

			list<ZeroCrossing>::iterator it = zeroCrossings.begin();

			double maxDist = 0.0;

			while (it != zeroCrossings.end()) {
				if (it->leftMag < 0.0) {
				double curDist = (fabs(it->leftMag) + fabs(it->rightMag));
				if (curDist > maxDist) {
					maxDist = curDist;
					notchPosition = it->position;
				}
				}
				++it;
			}

			zeroCrossings.clear();
			*/

            return startOfSnoutPosition + padding;
        }
    }
}
