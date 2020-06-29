using Darwin.Utilities;
using Darwin.Wavelet;
using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Features
{
    public class FinFeatureSet : FeatureSet
    {
        private static Dictionary<FeaturePointType, string> FeaturePointNameMapping = new Dictionary<FeaturePointType, string>()
        {
            { FeaturePointType.Tip, "Fin Tip" },
            { FeaturePointType.Notch, "Notch" },
            { FeaturePointType.LeadingEdgeBegin, "Beginning of Leading Edge" },
            { FeaturePointType.LeadingEdgeEnd, "End of Leading Edge" },
            { FeaturePointType.PointOfInflection, "End of Trailing Edge" }
        };

        // TODO: Valid features mapping?

        private const int NotchNumMinsToTrack = 5;

        public FinFeatureSet()
        {
            FeatureSetType = FeatureSetType.DorsalFin;

            FeaturePoints = new Dictionary<FeaturePointType, ContourFeaturePoint>()
            {
                { FeaturePointType.Tip, new ContourFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Tip], Type = FeaturePointType.Tip, IsEmpty = true } },
                { FeaturePointType.Notch, new ContourFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Notch], Type = FeaturePointType.Notch, IsEmpty = true } },
                { FeaturePointType.LeadingEdgeBegin, new ContourFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.LeadingEdgeBegin], Type = FeaturePointType.LeadingEdgeBegin, IsEmpty = true } },
                { FeaturePointType.LeadingEdgeEnd, new ContourFeaturePoint { Ignore = true, Name = FeaturePointNameMapping[FeaturePointType.LeadingEdgeEnd], Type = FeaturePointType.LeadingEdgeEnd, IsEmpty = true } },
                { FeaturePointType.PointOfInflection, new ContourFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.PointOfInflection], Type = FeaturePointType.PointOfInflection, IsEmpty = true } }
            };

            Features = new Dictionary<FeatureType, Feature>()
            {
                { FeatureType.LeadingEdgeAngle, new Feature { Name = "Angle of Leading Edge", Type = FeatureType.LeadingEdgeAngle, IsEmpty = true } }
            };
        }

        public FinFeatureSet(List<ContourFeaturePoint> featurePoints)
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

        public FinFeatureSet(Chain chain, FloatContour chainPoints)
            : this()
        {
            int tipPos = FindTip(chain, chainPoints);
            int notchPos = FindNotch(chain, tipPos);
            int beginLE = FindBeginLE(chain, tipPos);
            int endLE = FindEndLE(chain, beginLE, tipPos);
            int endTE = FindPointOfInflection(chain, tipPos);

            FeaturePoints[FeaturePointType.Tip].Position = tipPos;
            FeaturePoints[FeaturePointType.Notch].Position = notchPos;
            FeaturePoints[FeaturePointType.LeadingEdgeBegin].Position = beginLE;
            FeaturePoints[FeaturePointType.LeadingEdgeEnd].Position = endLE;
            FeaturePoints[FeaturePointType.PointOfInflection].Position = endTE;

            double leAngle = FindLEAngle(chain, tipPos, endLE);
            Features[FeatureType.LeadingEdgeAngle].Value = leAngle;
        }

        public override void SetFeaturePointPosition(FeaturePointType featurePointType, int position)
        {
            if (!FeaturePoints.ContainsKey(featurePointType))
                throw new ArgumentOutOfRangeException(nameof(featurePointType));

            FeaturePoints[featurePointType].Position = position;
            FeaturePoints[featurePointType].UserSetPosition = true;
        }

        public int FindNotch(Chain chain, int tipPos)
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));

            if (tipPos < 0 || tipPos > chain.Length)
                throw new Exception("findNotch() [int tipPosition]");

            // TODO
            //if (_userSetNotch)
            //    throw new Exception("findNotch() [User selected notch in use]");

            // We're only going to be looking on the trailing edge
            // of the fin for the most significant notch, so we'll
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
            Array.Copy(chain.Data, tipPos + 1, src, 0, numTrailingEdgePts);

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
            int notchPosition = 0;
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
                    notchPosition = 1;

                    if (0 == notchPosition)
                        notchPosition = chain.Length - tipPos - 1;

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
                notchPosition = chain.Length - tipPos - 1;

            if (notchPosition == 0)
            {
                // Now, we'll take the lowest few mins, and look at how
                // they change over the transform levels.

                double[] minVals = new double[mins.Count];

                for (i = 0; i < mins.Count; i++)
                    minVals[i] = modMax[level, mins[i]];

                Array.Sort(minVals);

                int numMinsToTrack;

                if ((int)mins.Count < NotchNumMinsToTrack)
                    numMinsToTrack = mins.Count;
                else
                    numMinsToTrack = NotchNumMinsToTrack;

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

                        //alpha = alphaK(
                        //		level + 1,
                        //		coarserLevel + 1,
                        //		modMax[level][positions[i]],
                        //		modMax[coarserLevel][correspondingPos]);
                        dif = Math.Abs(modMax[coarserLevel, correspondingPos]);
                        //+ fabs(modMax[level][positions[i]]);

                        if (firstRun)
                        {
                            firstRun = false;
                            difMax = dif;
                            notchPosition = positions[i];
                        }
                        else if (dif > difMax)
                        {
                            difMax = dif;
                            notchPosition = positions[i];
                        }
                    }
                }

                if (firstRun)
                    notchPosition = chain.Length - 1 - tipPos;
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

            return notchPosition + tipPos;
        }
    }
}
