using Darwin.Utilities;
using Darwin.Wavelet;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace Darwin.Features
{
    public enum FeatureSetType
    {
        DorsalFin = 0,
        Bear = 1
    }

    public abstract class FeatureSet : INotifyPropertyChanged
    {
        protected const int TransformLevels = 5;
        protected const double LETrimAmount = 0.05;
        protected const double CutoffPercent = 0.10;

        public FeatureSetType FeatureSetType { get; set; }

        public Dictionary<FeatureType, Feature> Features { get; set; }
        public Dictionary<FeaturePointType, FeaturePoint> FeaturePoints { get; set; }

        //public FeaturePoint this[FeaturePointType featurePointType]
        //{
        //    get
        //    {
        //        if (FeaturePoints == null || !FeaturePoints.ContainsKey(featurePointType))
        //            return FeaturePoint.Empty;

        //        return FeaturePoints[featurePointType];
        //    }
        //    set
        //    {
        //        FeaturePoints[featurePointType] = value;
        //        RaisePropertyChanged("FeaturePoints");
        //    }
        //}

        public List<int> FeaturePointPositions
        {
            get
            {
                if (FeaturePoints == null)
                    return new List<int>();

                return FeaturePoints
                    .Where(fp => !fp.Value.Ignore)
                    .Select(fp => fp.Value.Position)
                    .ToList();
            }
        }
        
        public event PropertyChangedEventHandler PropertyChanged;

        public static FeatureSet Create(FeatureSetType featuresType)
        {
            switch (featuresType)
            {
                case FeatureSetType.DorsalFin:
                    return new FinFeatureSet();

                case FeatureSetType.Bear:
                    return new BearFeatureSet();

                default:
                    throw new NotImplementedException();
            }
        }

        public static FeatureSet Create(FeatureSetType featuresType, Chain chain, FloatContour chainPoints)
        {
            switch (featuresType)
            {
                case FeatureSetType.DorsalFin:
                    if (chain == null || chainPoints == null)
                        return new FinFeatureSet();

                    return new FinFeatureSet(chain, chainPoints);

                case FeatureSetType.Bear:
                    if (chain == null || chainPoints == null)
                        return new BearFeatureSet();

                    return new BearFeatureSet(chain, chainPoints);

                default:
                    throw new NotImplementedException();
            }
        }

        public abstract void SetFeaturePointPosition(FeaturePointType featurePointType, int position);

        //*******************************************************************
        //
        // int Outline::findTip()
        //
        //    Finds the tip as the index into the chain array
        //
        public int FindTip(Chain chain, FloatContour chainPoints)
        {
            if (chain == null)
                throw new Exception("findTip() [*_chain]");

            // TODO
            //if (_userSetTip)
            //    throw new Exception("findNotch() [User selected tip in use]");

            int numPoints = chain.Length;

            // First, make a copy without the first value in the chain,
            // since the first value skews the rest of the chain and is
            // unnecessary for our purposes here
            double[] src = new double[numPoints - 1];

            Array.Copy(chain.Data, 1, src, 0, numPoints - 1);

            int nextPowOfTwo = MathHelper.NextPowerOfTwo(numPoints - 1);
            // Now set up the variables needed to perform a wavelet
            // transform on the chain
            double[,] continuousResult = new double[TransformLevels + 1, nextPowOfTwo];

            // Now perform the transformation
            WIRWavelet.WL_FrwtVector(src,
                    ref continuousResult,
                    numPoints - 1,
                    TransformLevels,
                    WaveletUtil.MZLowPassFilter,
                    WaveletUtil.MZHighPassFilter);

            int
                tipPosition = 0,
                level = TransformLevels;

            //***0041TIP - JHS new section below 
            // finding higest point on fin
            // remember image is upside down so high point has minimum Y value
            int highPointId = -1;
            float highPointY = 10000.0f;

            for (int ptId = 0; ptId < numPoints; ptId++)
                if (chainPoints[ptId].Y < highPointY)
                {
                    highPointY = chainPoints[ptId].Y;
                    highPointId = ptId;
                }
                else if (chainPoints[ptId].Y < highPointY + 3.0)
                {
                    // until we drop more than 3 units back down the fin
                    // keep advancing the highPointId.  That way, we
                    // find the rightmost "essentially highest" point
                    // on the outline
                    highPointId = ptId;
                }

            /*
            * while (!tipPosition && level > 0) { // Find the maxima of the
            * coefficients double *modMax = new double[numPoints - 1];
            * 
            * for (int k = 0; k < numPoints - 1; k++) continuousResult[level][k] 
            * *= normalizationCoeff(level);
            * 
            * modulusMaxima(continuousResult[level], modMax, numPoints - 1);
            * 
            * // Now, find the largest positive max, which we'll // assume is
            * the tip of the fin.
            * 
            * double max = modMax[0]; for (int i = 1; i < numPoints - 1; i++) {
            * if (modMax[i] > max) { max = modMax[i]; tipPosition = i; } }
            * 
            * level--; delete[] modMax; } 
            */

            while (level > 1)
            {
                // Find the maxima of the coefficients
                double[] modMax = new double[numPoints - 1];

                for (int k = 0; k < numPoints - 1; k++)
                    continuousResult[level, k] *= WaveletUtil.NormalizationCoeff(level);

                double[] continousExtract = WaveletUtil.Extract1DArray(continuousResult, level, numPoints - 1);
                WaveletUtil.ModulusMaxima(continousExtract, ref modMax, numPoints - 1);

                // Now, find the largest positive max, which we'll
                // assume is the tip of the fin.

                if (tipPosition == 0)
                {
                    // original loop control is two line below
                    //double max = modMax[0]; //***0041TIP-JHS restricting range of search
                    //for (int i = 1; i < numPoints - 1; i++) { //***0041TIP-JHS restricting range of search
                    //***0041TIP - we now restrict the range for initial detection of the tip
                    // start at high point on fin and search within 150 point range along fin
                    //***1.6 - NOTE: there is an unintended consequence of the limmits imposed
                    // below.  If outlines are shortened to 50% or so of full size by having
                    // leading or trailing edges missing, the range of +/- 75 points is not
                    // a large enough range to capture tips of "MissingTip" fins in a consistent
                    // manner.  We need to rethink this, and possibly go back to the original
                    // and retest against a database of fins traced without this limit.
                    // The new "Iterative, Tip Shifting" mapping approach probably conpensates
                    // for tip placement better than this limit on detection does. -- JHS (8/2/2006)
                    double max = modMax[highPointId - 75]; //***1.6 - removed temporarily for tests
                    for (int i = highPointId - 75; i < highPointId + 75; i++)
                    { //***1.6 - removed temporarily for tests
                        if (modMax[i] > max)
                        {
                            max = modMax[i];
                            tipPosition = i;
                        }
                    }
                }
                else
                    tipPosition =
                        FindClosestMax(modMax, numPoints - 1, tipPosition);

                level--;
            }

            // ... and clean up

            // Note that the actual tip position is +1 because we
            // ignored the first value 
            Trace.WriteLine("Tip position: " + (tipPosition + 1));

            if ((tipPosition + 1) < 5)
                Trace.WriteLine("Probable bad tip position.");

            return tipPosition + 1;
        }

        // findLEBegin
        // 	
        // 	Attempts to find the first point on the leading edge where the
        // 	angle stabilizes.  Does this by computing a threshold which will
        // 	maximize the between class variance of two sets of angles (the
        // 	outliers at the beginning of the edge and the main part of the
        // 	edge, itself).  (This is Otsu's method of automatic
        // 	thresholding.  In this case, we assume the "foreground" is the
        // 	main part of the fin while the "background" is the outlying
        // 	section.)
        //
        // 	Return: int representing the index into the chain of the cutoff
        // 	point.
        public int FindBeginLE(Chain chain, int tipPos)
        {
            if (null == chain)
                throw new ArgumentNullException(nameof(chain));

            if (tipPos <= 2)
                throw new Exception("findLECutoff() [int _tipPos <= 2]");

            // TODO
            //if (_userSetBeginLE)
            //    throw new Exception("findLECutoff() [attempt to modify uset set LEbegin");

            int numPoints = (tipPos - 1) / 2;

            double
                min = chain.Min(0, numPoints),
                max = chain.Max(0, numPoints);

            int roundedMin = (int)Math.Round(min);
            int numLevels = (int)Math.Round(max) - roundedMin;

            if (numLevels <= 1)
                return 0;

            int[] levels = new int[numLevels];

            //memset((void*)levels, 0, numLevels * sizeof(int));

            int i, idx;
            for (i = 0; i < numPoints; i++)
            {
                idx = (int)Math.Round(chain[i]) - roundedMin;

                if (idx < 0)
                    idx = 0;
                if (idx >= numLevels)
                    idx = numLevels - 1;
                ++levels[idx];
            }

            float[] relativeFreq = new float[numLevels];

            for (i = 0; i < numLevels; i++)
                relativeFreq[i] = (float)levels[i] / numPoints;

            float
                curVariance,    // variance calculation for the current threshhold

                maxVariance,    // the maximum value for the variance so far

                fgAvgBrightness, // average level of the foreground

                bgAvgBrightness, // average level of the background

                fgArea;     // percentage of the chain which is foreground after
                            // the chain has been threshholded.  the area of the
                            // background will then be 1 - fgArea

            // set up the initial values for the variables
            fgArea = fgAvgBrightness = relativeFreq[0];
            bgAvgBrightness = 0;

            // compute the average level of the background
            for (int freqPos = 1; freqPos < numLevels; freqPos++)
                bgAvgBrightness += (freqPos + 1) * relativeFreq[freqPos];

            maxVariance = fgArea * (1 - fgArea) * (fgAvgBrightness - bgAvgBrightness)
                    * (fgAvgBrightness - bgAvgBrightness);

            int optThreshold = 0;

            for (int lev = 1; lev < numLevels; lev++)
            {
                fgAvgBrightness *= fgArea;
                bgAvgBrightness *= (1 - fgArea);

                fgArea += relativeFreq[lev];

                if (fgArea > 1) // if there's a rounding error
                    fgArea = 1;

                fgAvgBrightness += (lev + 1) * relativeFreq[lev];
                bgAvgBrightness -= (lev + 1) * relativeFreq[lev];

                if (bgAvgBrightness < 0)  //if there's a rounding error
                    bgAvgBrightness = 0;

                if (fgArea == 0f)  // make sure we're not dividing by zero
                    fgAvgBrightness = 0;
                else
                    fgAvgBrightness /= fgArea;

                if (fgArea == 1)  // make sure we're not dividing by zero
                    bgAvgBrightness = 0;
                else
                    bgAvgBrightness /= 1 - fgArea;

                curVariance = fgArea * (1 - fgArea) * (fgAvgBrightness - bgAvgBrightness)
                    * (fgAvgBrightness - bgAvgBrightness);

                if (curVariance > maxVariance)
                {
                    maxVariance = curVariance;
                    optThreshold = lev;
                }
            }

            double cutoffVal = (double)optThreshold + roundedMin;

            for (i = 0; i < numPoints - 1; i++)
            {
                if (chain[i] < cutoffVal)
                    return i;
            }

            return 0;
        }

        public int FindEndLE(Chain chain, int beginLE, int tipPos)
        {
            if (null == chain)
                throw new Exception("findLEEnd() [data member *_chain]");

            if (tipPos <= 2)
                throw new Exception("findLEEnd() [data member _tipPos <= 2]");

            // TODO
            //if (_userSetEndLE)
            //    throw new Exception("findLEEnd() [attempt to change user specified EndLE]");

            double
                min = chain.Min(beginLE, tipPos),
                max = chain.Max(beginLE, tipPos);

            int roundedMin = (int)Math.Round(min);
            int numLevels = (int)Math.Round(max) - roundedMin;

            if (numLevels <= 1)
                return 0;

            int[] levels = new int[numLevels];

            //memset((void*)levels, 0, numLevels * sizeof(int));

            int i, idx;

            for (i = beginLE; i < tipPos; i++)
            {
                idx = (int)Math.Round(chain[i]) - roundedMin;

                if (idx < 0)
                    idx = 0;
                else if (idx >= numLevels)
                    idx = numLevels - 1;

                ++levels[idx];
            }

            float[] relativeFreq = new float[numLevels];

            for (i = 0; i < numLevels; i++)
                relativeFreq[i] = (float)levels[i] / (tipPos - beginLE);

            float
                curVariance,    // variance calculation for the current threshhold

                maxVariance,    // the maximum value for the variance so far

                fgAvgBrightness, // average level of the foreground

                bgAvgBrightness, // average level of the background

                fgArea;     // percentage of the chain which is foreground after
                            // the chain has been threshholded.  the area of the
                            // background will then be 1 - fgArea

            // set up the initial values for the variables
            fgArea = fgAvgBrightness = relativeFreq[0];
            bgAvgBrightness = 0;

            // compute the average level of the background
            for (int freqPos = 1; freqPos < numLevels; freqPos++)
                bgAvgBrightness += (freqPos + 1) * relativeFreq[freqPos];

            maxVariance = fgArea * (1 - fgArea) * (fgAvgBrightness - bgAvgBrightness)
                    * (fgAvgBrightness - bgAvgBrightness);

            int optThreshold = 0;

            for (int lev = 1; lev < numLevels; lev++)
            {
                fgAvgBrightness *= fgArea;
                bgAvgBrightness *= (1 - fgArea);

                fgArea += relativeFreq[lev];

                if (fgArea > 1) // if there's a rounding error
                    fgArea = 1;

                fgAvgBrightness += (lev + 1) * relativeFreq[lev];
                bgAvgBrightness -= (lev + 1) * relativeFreq[lev];

                if (bgAvgBrightness < 0)  //if there's a rounding error
                    bgAvgBrightness = 0;

                if (fgArea == 0f)  // make sure we're not dividing by zero
                    fgAvgBrightness = 0;
                else
                    fgAvgBrightness /= fgArea;

                if (fgArea == 1)  // make sure we're not dividing by zero
                    bgAvgBrightness = 0;
                else
                    bgAvgBrightness /= 1 - fgArea;

                curVariance = fgArea * (1 - fgArea) * (fgAvgBrightness - bgAvgBrightness)
                    * (fgAvgBrightness - bgAvgBrightness);

                if (curVariance > maxVariance)
                {
                    maxVariance = curVariance;
                    optThreshold = lev;
                }
            }

            double cutoffVal = (double)optThreshold + roundedMin;

            for (i = tipPos - 1; i > beginLE + 1; i--)
            {
                if (chain[i] < cutoffVal)
                    return i;
            }

            return beginLE + 1;
        }

        public int FindPointOfInflection(Chain chain, int tipPos)
        {
            int numPoints = chain.Length - tipPos - 1;

            Chain smoothChain = new Chain(chain);
            smoothChain.Smooth15();

            double[] der = new double[numPoints - 1];

            for (int i = tipPos + 1, j = 0; i < smoothChain.Length - 1; i++, j++)
                der[j] = smoothChain.RelativeData[i + 1] - smoothChain.RelativeData[i];

            List<ZeroCrossing> zeroCrossings = FindZeroCrossings(der, numPoints - 1);

            // Fallback if there was a problem finding zero crossings
            if (zeroCrossings == null || zeroCrossings.Count < 1)
                return chain.Length - 1;

            return zeroCrossings[zeroCrossings.Count - 1].Position + tipPos;
        }

        public double FindLEAngle(Chain chain, int tipPos, int endLE)
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));
            if (tipPos <= 1)
                throw new Exception("findLEAngle() [data member _tipPos <= 1]");

            // JHS - does it make more sense to use END of leading edge
            // rather that TIP.  I think so.
            // original code 7/25/2005
            //int numPoints = (int)round((_tipPos - 1) * (1.0 - 2.0 * LE_TRIM_AMOUNT));
            //int startPos = (int) round((_tipPos - 1) * LE_TRIM_AMOUNT);
            // new code 7/25/2005
            int numPoints = (int)Math.Round((endLE - 1) * (1.0 - 2.0 * LETrimAmount));
            int startPos = (int)Math.Round((endLE - 1) * LETrimAmount);

            Chain smoothChain = new Chain(chain);
            smoothChain.Smooth5();

            double[] smooth = new double[numPoints];

            Array.Copy(smoothChain.Data, startPos, smooth, 0, numPoints);

            Array.Sort(smooth);

            double angle;

            if (numPoints % 2 == 0)
                angle = smooth[numPoints / 2];

            else
            {
                double pos = (double)numPoints / 2;
                angle = (smooth[(int)Math.Floor(pos)] + smooth[(int)Math.Ceiling(pos)]) / 2;
            }

            return angle;
        }

        // return of -1 indicates inability to find a closest max
        public int FindClosestMax(double[] modmax, int len, int prevPosition)
        {
            if (modmax == null)
                throw new ArgumentNullException(nameof(modmax));

            if (prevPosition < 0 || prevPosition >= len)
                throw new ArgumentOutOfRangeException(nameof(prevPosition));

            if (len <= 0)
                throw new ArgumentOutOfRangeException(nameof(len));


            if (modmax[prevPosition] > 0)
                return prevPosition;

            int lowDist = 0,
             highDist = 0;
            int lowMax = -1,
             highMax = -1;

            for (int i = prevPosition - 1; i >= 0 && lowMax == -1; i--, lowDist++)
            {
                if (modmax[i] > 0.0)
                    lowMax = i;
            }

            for (int j = prevPosition + 1; j < len && highMax == -1;
             j++, highDist++)
            {
                if (modmax[j] > 0.0)
                    highMax = j;
            }

            // don't really need to test whether they're both -1 here.  if
            // they are, -1 will be returned, indicating that we haven't
            // found a closest max
            if (highMax == -1)
                return lowMax;

            if (lowMax == -1)
                return highMax;

            if (lowDist > highDist)
                return highMax;

            if (highDist > lowDist)
                return lowMax;

            // if we get here, highDist == lowDist
            // we'll return the larger max

            if (modmax[lowMax] > modmax[highMax])
                return lowMax;

            return highMax;
        }

        // return of -1 indicates inability to find a closest min
        public int FindClosestMin(int dist, double[] modmax, int len, int prevPosition)
        {
            if (modmax == null)
                throw new ArgumentNullException(nameof(modmax));

            if (prevPosition < 0 || prevPosition >= len)
                throw new ArgumentOutOfRangeException(nameof(prevPosition));

            if (len <= 0)
                throw new ArgumentOutOfRangeException(nameof(len));

            if (modmax[prevPosition] < 0.0)
                return prevPosition;

            int
            lowDist = 0,
            highDist = 0;
            int
            lowMin = -1,
                highMin = -1;

            for (int i = prevPosition - 1; i >= 0 && i >= prevPosition - dist && lowMin == -1; i--, lowDist++)
            {
                if (modmax[i] < 0.0)
                    lowMin = i;
            }

            for (int j = prevPosition + 1; j < len && j <= prevPosition + dist && highMin == -1; j++, highDist++)
            {
                if (modmax[j] < 0.0)
                    highMin = j;
            }

            // don't really need to test whether they're both -1 here.  if
            // they are, -1 will be returned, indicating that we haven't
            // found a closest min
            if (highMin == -1)
                return lowMin;

            if (lowMin == -1)
                return highMin;

            if (lowDist > highDist)
                return highMin;

            if (highDist > lowDist)
                return lowMin;

            // if we get here, highDist == lowDist
            // we'll return the smaller min

            if (modmax[lowMin] < modmax[highMin])
                return lowMin;

            return highMin;
        }

        static List<ZeroCrossing> FindZeroCrossings(double[] src, int numPoints)
        {
            if (src == null)
                throw new ArgumentNullException(nameof(src));

            if (numPoints < 3)
                throw new ArgumentOutOfRangeException(nameof(numPoints));

            List<ZeroCrossing> zeroCrossings = new List<ZeroCrossing>();

            double[] modmax = new double[numPoints];
            WaveletUtil.ModulusMaxima(src, ref modmax, numPoints);

            for (int i = 0; i < numPoints - 1; i++)
            {
                if ((src[i] < 0.0 && src[i + 1] > 0.0)
                || (src[i] > 0.0 && src[i + 1] < 0.0) || src[i] == 0.0)
                {
                    ZeroCrossing zc = new ZeroCrossing
                    {
                        Position = i,
                        LeftMag = FindLeftExtremum(modmax, numPoints, i),
                        RightMag = FindRightExtremum(modmax, numPoints, i)
                    };
                    zc.Position = i;

                    zeroCrossings.Add(zc);
                }

                if (src[i] == 0.0)
                {
                    i++;
                }
            }

            return zeroCrossings;
        }

        static double FindLeftExtremum(double[] src, int numPoints, int startPos)
        {
            if (src == null)
                throw new ArgumentNullException(nameof(src));

            if (numPoints < 2 || startPos >= numPoints)
                throw new ArgumentOutOfRangeException(nameof(numPoints));

            if (startPos < 0)
                throw new ArgumentOutOfRangeException(nameof(startPos));

            for (int i = startPos - 1; i >= 0; i--)
                if (src[i] != 0.0)
                    return src[i];

            return 0;
        }

        static double FindRightExtremum(double[] src, int numPoints, int startPos)
        {
            if (src == null)
                throw new ArgumentNullException(nameof(src));

            if (numPoints < 2 || startPos >= numPoints)
                throw new ArgumentOutOfRangeException(nameof(numPoints));

            if (startPos < 0)
                throw new ArgumentOutOfRangeException(nameof(startPos));


            for (int i = startPos + 1; i < numPoints; i++)
                if (src[i] != 0.0)
                    return src[i];

            return numPoints - 1;
        }

        private static double AlphaK(int level1, int level2, double mag1, double mag2)
        {
            if (level1 == level2)
                return 0.0;
            double
                ord1 = Math.Abs(mag1),
                ord2 = Math.Abs(mag2);

            if (ord1 <= 0.0)
                return 0.0;

            if (ord2 <= 0.0)
                return 0.0;

            return (Math.Log(ord2, 2) - Math.Log(ord1, 2)) / (level2 - level1);
        }

        protected virtual void RaisePropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
