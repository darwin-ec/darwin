//*******************************************************************
//   file: Outline.cxx
//
// author: J H Stewman & K R Debure
//
// This class consolidates in one place what was previously passed
// about as a collection of Contours, FloatContours, Chains, ...
//
//*******************************************************************

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
        private const int TransformLevels = 5;
        private const double LETrimAmount = 0.05;
        private const double CutoffPercent = 0.10;

        private const int NotchNumMinsToTrack = 5;

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

        //int mNumPoints; //***008OL remove & use chain length ?

        private FloatContour _remappedChainPoints;
        public FloatContour RemappedChainPoints { get => _remappedChainPoints; set => _remappedChainPoints = value; }

        private int    // indices into chain or fl contour 
            _tipPos,
            _notchPos,
            _beginLE,
            _endLE,
            _endTE;

        double _LEAngle; // I think we want this here - JHS


        bool
            _userSetTip,
            _userSetNotch,
            _userSetBeginLE,
            _userSetEndLE,
            _userSetEndTE;


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
        public Outline(Contour c, double radius)
        {
            _userSetTip = false;
            _userSetNotch = false;
            _userSetBeginLE = false;
            _userSetEndLE = false;
            _userSetEndTE = false;
            _remappedChainPoints = null;
            _chainPoints = new FloatContour(); // ***008OL
            _chainPoints.ContourToFloatContour(c); //***008OL
            _chain = new Chain(_chainPoints);
            _tipPos = FindTip();
            _notchPos = FindNotch();
            _beginLE = FindBeginLE();
            _endLE = FindEndLE();
            _endTE = FindPointOfInflection();
            _LEAngle = FindLEAngle();
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
        public Outline(FloatContour fc)
        {
            // called prior to reading indices of each feature pt and the
            // user mod bits
            _chainPoints = fc;
            _chain = new Chain(_chainPoints);
            _tipPos = 0;
            _notchPos = 0;
            _beginLE = 0;
            _endLE = 0;
            _endTE = 0;
            _LEAngle = 0;
            _userSetTip = false;
            _userSetNotch = false;
            _userSetBeginLE = false;
            _userSetEndLE = false;
            _userSetEndTE = false;
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
            _tipPos = outline._tipPos;
            _notchPos = outline._notchPos;
            _beginLE = outline._beginLE;
            _endLE = outline._endLE;
            _endTE = outline._endTE;
            _LEAngle = outline._LEAngle;
            _userSetTip = outline._userSetTip;
            _userSetNotch = outline._userSetNotch;
            _userSetBeginLE = outline._userSetBeginLE;
            _userSetEndLE = outline._userSetEndLE;
            _userSetEndTE = outline._userSetEndTE;
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

        public double GetLEAngle()
        {
            return _LEAngle;
        }

        //    If compute = true then ignore theta and call findLEAngle()
        //    else use theta as the angle value
        public void SetLEAngle(double theta, bool compute)
        {
            if (compute)
                _LEAngle = FindLEAngle();
            else
                _LEAngle = theta;
        }

        // Attempts to determine the angle of the fin's leading edge.
        public double FindLEAngle()
        {
            if (_chain == null)
                throw new Exception("findLEAngle() [data member *_chain]");
            if (_tipPos <= 1)
                throw new Exception("findLEAngle() [data member _tipPos <= 1]");

            // JHS - does it make more sense to use END of leading edge
            // rather that TIP.  I think so.
            // original code 7/25/2005
            //int numPoints = (int)round((_tipPos - 1) * (1.0 - 2.0 * LE_TRIM_AMOUNT));
            //int startPos = (int) round((_tipPos - 1) * LE_TRIM_AMOUNT);
            // new code 7/25/2005
            int numPoints = (int)Math.Round((_endLE - 1) * (1.0 - 2.0 * LETrimAmount));
            int startPos = (int)Math.Round((_endLE - 1) * LETrimAmount);

            Chain smoothChain = new Chain(_chain);
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


        // inspectors for feature points
        public int GetFeaturePoint(FeaturePointType type)
        {
            if (type == FeaturePointType.NoFeature)
                throw new ArgumentOutOfRangeException(nameof(type));

            switch (type)
            {
                case FeaturePointType.Tip:
                    return _tipPos;

                case FeaturePointType.Notch:
                    return _notchPos;

                case FeaturePointType.PointOfInflection:
                    return _endTE;

                case FeaturePointType.LeadingEdgeBegin:
                    return _beginLE;

                case FeaturePointType.LeadingEdgeEnd:
                    return _endLE;

                default: //***1.95
                    throw new Exception("getFeaturePoint(): invalid feature type");
            }
        }

        // inspectors for feature points
        public PointF GetFeaturePointCoords(FeaturePointType type)
        {
            if (type == FeaturePointType.NoFeature)
                throw new ArgumentOutOfRangeException(nameof(type));

            switch (type)
            {
                case FeaturePointType.Tip:
                    return _chainPoints[_tipPos];

                case FeaturePointType.Notch:
                    return _chainPoints[_notchPos];

                case FeaturePointType.PointOfInflection:
                    return _chainPoints[_endTE];

                case FeaturePointType.LeadingEdgeBegin:
                    return _chainPoints[_beginLE];

                case FeaturePointType.LeadingEdgeEnd:
                    return _chainPoints[_endLE];

                default: //***1.95
                    throw new Exception("getFeaturePointCoords(): invalid feature type");

            }
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

            if (type < FeaturePointType.LeadingEdgeBegin || type > FeaturePointType.PointOfInflection)
                throw new Exception("setTip() [int type]");

            switch (type)
            {
                case FeaturePointType.Tip:
                    _tipPos = position;
                    _userSetTip = true;
                    break;
                case FeaturePointType.Notch:
                    _notchPos = position;
                    _userSetNotch = true;
                    break;
                case FeaturePointType.PointOfInflection:
                    _endTE = position;
                    _userSetEndTE = true;
                    break;
                case FeaturePointType.LeadingEdgeBegin:
                    _beginLE = position;
                    _userSetBeginLE = true;
                    break;
                case FeaturePointType.LeadingEdgeEnd:
                    _endLE = position;
                    _userSetEndLE = true;
                    break;
            };
        }

        // mutators for outline
        public void SetFloatContour(FloatContour fc)
        {
            throw new NotImplementedException();
        }

        //*******************************************************************
        //
        // int Outline::findTip()
        //
        //    Finds the tip as the index into the chain array
        //
        public int FindTip()
        {
            if (_chain == null)
                throw new Exception("findTip() [*_chain]");
            if (_userSetTip)
                throw new Exception("findNotch() [User selected tip in use]");

            int numPoints = _chain.Length;

            // First, make a copy without the first value in the chain,
            // since the first value skews the rest of the chain and is
            // unnecessary for our purposes here
            double[] src = new double[numPoints - 1];

            Array.Copy(_chain.Data, 1, src, 0, numPoints - 1);

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
                if (_chainPoints[ptId].Y < highPointY)
                {
                    highPointY = _chainPoints[ptId].Y;
                    highPointId = ptId;
                }
                else if (_chainPoints[ptId].Y < highPointY + 3.0)
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

        public int FindNotch()
        {
            if (_chain == null)
                throw new Exception("findNotch(): [const Chain *chain]");

            if (_tipPos < 0 || _tipPos > _chain.Length)
                throw new Exception("findNotch() [int tipPosition]");

            if (_userSetNotch)
                throw new Exception("findNotch() [User selected notch in use]");


            double[] src = null;
            double[,] modMax = null;
            double[] minVals = null;
            int[] positions = null;

            // We're only going to be looking on the trailing edge
            // of the fin for the most significant notch, so we'll
            // build a source vector from that area
            int numTrailingEdgePts = _chain.Length - _tipPos - 1 - 5;
            src = new double[numTrailingEdgePts];

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
            Array.Copy(_chain.Data, 1, src, 0, numTrailingEdgePts);

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

            modMax = new double[TransformLevels, numTrailingEdgePts];

            // ..and find its local minima and maxima
            for (i = 0; i < TransformLevels; i++)
            {
                double[] temp = new double[numTrailingEdgePts];

                double[] continousExtract = WaveletUtil.Extract1DArray(continuousResult, i + 1, numTrailingEdgePts);

                WaveletUtil.ModulusMaxima(continousExtract, ref temp, numTrailingEdgePts);

                WaveletUtil.Patch1DArray(temp, ref continuousResult, i + 1, numTrailingEdgePts);
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
                        notchPosition = _chain.Length - _tipPos - 1;

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
                notchPosition = _chain.Length - _tipPos - 1;

            if (notchPosition == 0)
            {
                // Now, we'll take the lowest few mins, and look at how
                // they change over the transform levels.

                minVals = new double[mins.Count];

                for (i = 0; i < mins.Count; i++)
                    minVals[i] = modMax[level, i];

                Array.Sort(minVals);

                int numMinsToTrack;

                if ((int)mins.Count < NotchNumMinsToTrack)
                    numMinsToTrack = mins.Count;
                else
                    numMinsToTrack = NotchNumMinsToTrack;

                positions = new int[numMinsToTrack];

                for (int count = 0; count < numMinsToTrack; count++)
                {
                    for (i = 0; i < mins.Count; i++)
                    {
                        if (minVals[count] == modMax[level, i])
                        {
                            positions[count] = i;
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
                    notchPosition = _chain.Length - 1 - _tipPos;
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

            return notchPosition + _tipPos;
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
        public int FindBeginLE()
        {
            if (null == _chain)
                throw new Exception("findLECutoff() [_chain]");

            if (_tipPos <= 2)
                throw new Exception("findLECutoff() [int _tipPos <= 2]");

            if (_userSetBeginLE)
                throw new Exception("findLECutoff() [attempt to modify uset set LEbegin");

            int numPoints = (_tipPos - 1) / 2;

            double
                min = _chain.Min(0, numPoints),
                max = _chain.Max(0, numPoints);

            int roundedMin = (int)Math.Round(min);
            int numLevels = (int)Math.Round(max) - roundedMin;

            if (numLevels <= 1)
                return 0;

            int[] levels = new int[numLevels];

            //memset((void*)levels, 0, numLevels * sizeof(int));

            int i, idx;
            for (i = 0; i < numPoints; i++)
            {
                idx = (int)Math.Round(_chain[i]) - roundedMin;

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
                if (_chain[i] < cutoffVal)
                    return i;
            }

            return 0;
        }

        public int FindEndLE()
        {
            if (null == _chain)
                throw new Exception("findLEEnd() [data member *_chain]");

            if (_tipPos <= 2)
                throw new Exception("findLEEnd() [data member _tipPos <= 2]");

            if (_userSetEndLE)
                throw new Exception("findLEEnd() [attempt to change user specified EndLE]");

            double
                min = _chain.Min(_beginLE, _tipPos),
                max = _chain.Max(_beginLE, _tipPos);

            int roundedMin = (int)Math.Round(min);
            int numLevels = (int)Math.Round(max) - roundedMin;

            if (numLevels <= 1)
                return 0;

            int[] levels = new int[numLevels];

            //memset((void*)levels, 0, numLevels * sizeof(int));

            int i, idx;

            for (i = _beginLE; i < _tipPos; i++)
            {
                idx = (int)Math.Round(_chain[i]) - roundedMin;

                if (idx < 0)
                    idx = 0;
                else if (idx >= numLevels)
                    idx = numLevels - 1;

                ++levels[idx];
            }

            float[] relativeFreq = new float[numLevels];

            for (i = 0; i < numLevels; i++)
                relativeFreq[i] = (float)levels[i] / (_tipPos - _beginLE);

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

            for (i = _tipPos - 1; i > _beginLE + 1; i--)
            {
                if (_chain[i] < cutoffVal)
                    return i;
            }

            return _beginLE + 1;
        }


        public int FindPointOfInflection()
        {
            int numPoints = _chain.Length - _tipPos - 1;

            Chain smoothChain = new Chain(_chain);
            smoothChain.Smooth15();

            double[] der = new double[numPoints - 1];

            for (int i = _tipPos + 1, j = 0; i < smoothChain.Length - 1; i++, j++)
                der[j] = smoothChain[i + 1] - smoothChain[i];

            List<ZeroCrossing> zeroCrossings = FindZeroCrossings(der, numPoints - 1);

            return zeroCrossings.Count - 1 + _tipPos;
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

    }
}
