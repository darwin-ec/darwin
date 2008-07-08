//*******************************************************************
//   file: Outline.cxx
//
// author: J H Stewman & K R Debure
//
// This class consolidates in one place what was previously passed
// about as a collection of Contours, FloatContours, Chains, ...
//
//*******************************************************************

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Outline.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <list>

#include "waveletUtil.h"
#include "wavelet/wlcore.h"

#include "math/gaussj.h"
#include "Error.h"
#include "utility.h"

using namespace std;

static const double NORMALIZATION_COEFFICIENTS[5] =
    { 1.50, 1.12, 1.03, 1.01, 1.0 };
static const int TRANSFORM_LEVELS = 5;
typedef struct _zeroCrossingTypeStruct {
    int position;
    double leftMag,
     rightMag;
} zeroCrossingType;

static const double LE_TRIM_AMOUNT = 0.05;
static const double CUTOFF_PERCENT = 0.10;

static const int NOTCH_NUM_MINS_TO_TRACK = 5;

/////////////////////////////////
// Utility function prototypes //
/////////////////////////////////

// Function to compare doubles for qsort
static int compareDoubles(const void *a, const void *b);

// return of -1 indicates inability to find a closest max
static int findClosestMax(const double *modmax, int len, int prevPosition);
static int findClosestMin(int dist, const double *modmax, int len, int prevPosition);

static list<zeroCrossingType> findZeroCrossings(const double *src, int numPoints);
static double findRightExtremum(const double *src, int numPoints, int startPos);
static double findLeftExtremum(const double *src, int numPoints, int startPos);

static double alphaK(int level1, int level2, double mag1, double mag2);

FloatContour* mapContour(FloatContour *c,
		point_t p1, point_t p2, point_t p3,
		point_t desP1, point_t desP2, point_t desP3);
/////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
// Outline(Contour* c, double radius)
//
// Called from TraceWindow for initial creation of outline
// 
//  PRE: the Contour is already evenly spaced, normalized and 
//       without knots
// POST: all members are initialized
//
Outline::Outline(Contour *c, double radius) :
  mUserSetTip(false),
  mUserSetNotch(false),
  mUserSetBeginLE(false),
  mUserSetEndLE(false),
  mUserSetEndTE(false),
	mRemappedChainPoints(NULL)
{
  mChainPoints = new FloatContour(); // ***008OL
 	mChainPoints->ContourToFloatContour(c); //***008OL
  mChain = new Chain(mChainPoints);
  mTipPos = findTip();
  mNotchPos = findNotch();
  mBeginLE = findBeginLE();
  mEndLE = findEndLE();
  mEndTE = findPointOfInflection();
  mLEAngle = findLEAngle();
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
Outline::Outline(FloatContour *fc) :
	mChainPoints(new FloatContour(*fc)), //***008OL
	mChain(new Chain(mChainPoints)),
  mTipPos(0),
  mNotchPos(0),
  mBeginLE(0),
  mEndLE(0),
  mEndTE(0),
  mLEAngle(0),
  mUserSetTip(false),
  mUserSetNotch(false),
  mUserSetBeginLE(false),
  mUserSetEndLE(false),
  mUserSetEndTE(false),
	mRemappedChainPoints(NULL) 
{
  // called prior to reading indices of each feature pt and the
  // user mod bits
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
Outline::Outline(Outline* outline) :
  mTipPos(outline->mTipPos),
  mNotchPos(outline->mNotchPos),
  mBeginLE(outline->mBeginLE),
  mEndLE(outline->mEndLE),
  mEndTE(outline->mEndTE),
  mUserSetTip(outline->mUserSetTip),
  mUserSetNotch(outline->mUserSetNotch),
  mUserSetBeginLE(outline->mUserSetBeginLE),
  mUserSetEndLE(outline->mUserSetEndLE),
  mUserSetEndTE(outline->mUserSetEndTE),
  mLEAngle(outline->mLEAngle),
  mRemappedChainPoints(NULL)
{
  mChain = new Chain(outline->mChain);
  mChainPoints = new FloatContour(*(outline->mChainPoints)); //***008OL
}

Outline::~Outline()
{
  delete mChainPoints;
  delete mRemappedChainPoints;
	delete mChain;
}

//***008OL new function implementation
FloatContour* Outline::getFloatContour() const
{
  return mChainPoints;
}

//***008OL new function implementation
FloatContour* Outline::getRemappedFloatContour() const
{
  return mRemappedChainPoints;
}

//***008OL new function implementation
Chain* Outline::getChain() const
{
  return mChain;
}

void Outline::mapOutlineTo(Outline* target)
{
    	FloatContour *mappedContour = mapContour(
                        mChainPoints,
                        getFeaturePointCoords(TIP),
                        getFeaturePointCoords(LE_BEGIN),
                        getFeaturePointCoords(NOTCH),
                        target->getFeaturePointCoords(TIP),
                        target->getFeaturePointCoords(LE_BEGIN),
                        target->getFeaturePointCoords(NOTCH));

        // At this point you need a new outline that contains a remapped
        // float contour for display.  But, if the new outline is
        // ultimately added to the database, you want it to contain the
        // unmapped float contour (the chainPoints).
        mRemappedChainPoints = mappedContour;

}

double Outline::getLEAngle() const
{
  return mLEAngle;
}

//    If compute = true then ignore theta and call findLEAngle()
//    else use theta as the angle value
void Outline::setLEAngle(double theta, bool compute)
{
	if (compute)
		mLEAngle = findLEAngle();
	else
		mLEAngle = theta;
}

// Attempts to determine the angle of the fin's leading edge.
double Outline::findLEAngle() const
{
	if (NULL == mChain)
		throw EmptyArgumentError("findLEAngle() [data member *mChain]");
	if (mTipPos <= 1)
		throw InvalidArgumentError("findLEAngle() [data member mTipPos <= 1]");
	
	// JHS - does it make more sense to use END of leading edge
	// rather that TIP.  I think so.
	// original code 7/25/2005
	//int numPoints = (int)round((mTipPos - 1) * (1.0 - 2.0 * LE_TRIM_AMOUNT));
	//int startPos = (int) round((mTipPos - 1) * LE_TRIM_AMOUNT);
	// new code 7/25/2005
	int numPoints = (int)round((mEndLE - 1) * (1.0 - 2.0 * LE_TRIM_AMOUNT));
	int startPos = (int) round((mEndLE - 1) * LE_TRIM_AMOUNT);

	Chain *smoothChain = new Chain(*mChain);
	smoothChain->smooth5();
	
	double *smooth = new double[numPoints];
	memcpy(smooth, &(*smoothChain)[startPos], numPoints * sizeof(double));
	delete smoothChain;

	qsort(smooth, numPoints, sizeof(double), compareDoubles);

	double angle;

	if (numPoints % 2 == 0)
		angle = smooth[numPoints / 2];

	else {
		double pos = (double)numPoints / 2;
		angle = (smooth[(int)floor(pos)] + smooth[(int)ceil(pos)]) / 2;
	}
	delete[] smooth;

	return angle;
}


// inspectors for feature points
int Outline::getFeaturePoint(int type) const
{
	if (type < LE_BEGIN || type > POINT_OF_INFLECTION)
		throw InvalidArgumentError("setTip() [int type]");

	switch (type)
	{
	case TIP: 
		return mTipPos;
		break;
	case NOTCH: 
		return mNotchPos;
		break;
	case POINT_OF_INFLECTION:
		return mEndTE;
		break;
	case LE_BEGIN:
		return mBeginLE;
		break;
	case LE_END:
		return mEndLE;
		break;
	default: //***1.95
		throw Error("getFeaturePoint(): invalid feature type");
		break;
	}
}

// inspectors for feature points
point_t& Outline::getFeaturePointCoords(int type) const
{
	if (type < LE_BEGIN || type > POINT_OF_INFLECTION)
		throw InvalidArgumentError("setTip() [int type]");

	switch (type)
	{
	case TIP: 
		return (*mChainPoints)[mTipPos];
		break;
	case NOTCH: 
		return (*mChainPoints)[mNotchPos];
		break;
	case POINT_OF_INFLECTION:
		return (*mChainPoints)[mEndTE];
		break;
	case LE_BEGIN:
		return (*mChainPoints)[mBeginLE];
		break;
	case LE_END:
		return (*mChainPoints)[mEndLE];
		break;
	default: //***1.95
		throw Error("getFeaturePointCoords(): invalid feature type");
		break;
	}
}

// returns the type of feature point closest to the given point
int Outline::findClosestFeaturePoint(point_t p) const
{
    int i, minFeature;
    double distToFeature, minDist;


    point_t feature = getFeaturePointCoords(LE_BEGIN);
    minDist = distance (p.x, p.y, feature.x, feature.y);
    minFeature = LE_BEGIN;
    for (i = LE_END; i <= POINT_OF_INFLECTION; i++){
        point_t feature = getFeaturePointCoords(i);
        distToFeature = distance(p.x, p.y, feature.x, feature.y);
        if (distToFeature < minDist){
            minDist = distToFeature; 
            minFeature = i;
        }
    }
    return minFeature;
}

// mutators for feature points
void Outline::setFeaturePoint(int type, int position) 
{
        if (position < 0 || position >= mChain->length())
            throw InvalidArgumentError("setTip() [int position]");
        if (type < LE_BEGIN || type > POINT_OF_INFLECTION)
            throw InvalidArgumentError("setTip() [int type]");

        switch (type){
        case TIP: 
	   mTipPos = position;
           mUserSetTip = true;
           break;
        case NOTCH: 
	   mNotchPos = position;
           mUserSetNotch = true;
           break;
        case POINT_OF_INFLECTION:
           mEndTE = position;
           mUserSetEndTE = true;
           break;
        case LE_BEGIN:
           mBeginLE = position;
           mUserSetBeginLE = true;
           break;
        case LE_END:
           mEndLE = position;
           mUserSetEndLE = true;
           break;
        };
}

// mutators for outline
void Outline::setFloatContour(FloatContour* fc)
{
    cout << "setFloatContour: not yet implemented" << endl;
}

//*******************************************************************
//
// int Outline::findTip()
//
//    Finds the tip as the index into the chain array
//
int Outline::findTip()
{
    if (NULL == mChain)
	    throw EmptyArgumentError("findTip() [*mChain]");
    if (mUserSetTip)  
	throw InvalidArgumentError("findNotch() [User selected tip in use]");

    try {
	int numPoints = mChain->length();

	// First, make a copy without the first value in the chain,
	// since the first value skews the rest of the chain and is
	// unnecessary for our purposes here
	double *src = new double[numPoints - 1];

	memcpy(src, &((*mChain)[1]), (numPoints - 1) * sizeof(double));
	// Now set up the variables needed to perform a wavelet
	// transform on the chain
	double **continuousResult;
	continuousResult = (double **) WL_Calloc2Dmem(TRANSFORM_LEVELS + 1,
						      nextPowerOfTwo
						      (numPoints - 1),
						      sizeof(double));

	// Now perform the transformation
	WL_FrwtVector(src,
		      continuousResult,
		      numPoints - 1,
		      TRANSFORM_LEVELS,
		      getMZLowPassFilter(), getMZHighPassFilter());

	int
		tipPosition = 0,
		level = TRANSFORM_LEVELS;

   //***0041TIP - JHS new section below 
   // finding higest point on fin
   // remember image is upside down so high point has minimum Y value
   int highPointId = -1;
   float highPointY = 10000.0f;

   for (int ptId = 0; ptId < numPoints; ptId++)
      if ((*mChainPoints)[ptId].y < highPointY)
      {
         highPointY = (*mChainPoints)[ptId].y;
         highPointId = ptId;
      }
      else if ((*mChainPoints)[ptId].y < highPointY+3.0)
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


	while (level > 1) {
	    // Find the maxima of the coefficients
       double *modMax = new double[numPoints - 1];

	    for (int k = 0; k < numPoints - 1; k++)
		continuousResult[level][k] *= normalizationCoeff(level);

	    modulusMaxima(continuousResult[level], modMax, numPoints - 1);

	    // Now, find the largest positive max, which we'll
	    // assume is the tip of the fin.

	    if (tipPosition == 0) {
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
			for (int i = highPointId - 75; i < highPointId + 75; i++) { //***1.6 - removed temporarily for tests
				if (modMax[i] > max) {
				max = modMax[i];
				tipPosition = i;
				}
			}
	    } else
		tipPosition =
		    findClosestMax(modMax, numPoints - 1, tipPosition);

	    level--;
	    delete[]modMax;
	}

	// ... and clean up
	WL_Free2Dmem(continuousResult);
	delete[]src;

	// Note that the actual tip position is +1 because we
	// ignored the first value 
#ifdef DEBUG
	cout << "Tip position: " << tipPosition + 1 << endl;

	if ((tipPosition + 1) < 5)
	    cout << "Probable bad tip position." << endl;
#endif
	return tipPosition + 1;

    } catch(...) {
	throw;
    }
}

int Outline::findNotch()
{
    if (NULL == mChain)
	throw EmptyArgumentError("findNotch(): [const Chain *chain]");

    if (mTipPos < 0 || mTipPos > mChain->length())
	    throw InvalidArgumentError("findNotch() [int tipPosition]");

    if (mUserSetNotch)
	    throw InvalidArgumentError("findNotch() [User selected notch in use]");


    double *src = NULL;
    double **modMax = NULL;
    double *minVals = NULL;
    int *positions = NULL;

    try {
	// We're only going to be looking on the trailing edge
	// of the fin for the most significant notch, so we'll
	// build a source vector from that area
	int numTrailingEdgePts = mChain->length() - mTipPos - 1 - 5;
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
	memcpy(src, &((*mChain)[mTipPos + 1]), numTrailingEdgePts * sizeof(double));

	//***1.95 - code to convert negative angles
	for (int di = 0; di < numTrailingEdgePts; di++)
	{
		if (src[di] < -90) 
			src[di] += 360.0; // force negative angles to be positive
		//printf ("%5.2f\n",src[di]);
	}

	// Now set up the variables needed to perform a wavelet
	// transform on the chain
	double **continuousResult;
	continuousResult = (double **) WL_Calloc2Dmem(TRANSFORM_LEVELS + 1,
						      nextPowerOfTwo(numTrailingEdgePts),
						      sizeof(double));

	// Now perform the transformation
	WL_FrwtVector(
			src,
			continuousResult,
			numTrailingEdgePts,
			TRANSFORM_LEVELS,
			getMZLowPassFilter(),
			getMZHighPassFilter());

	int i;
	for (i = 1; i <= TRANSFORM_LEVELS; i++)
		for (int j = 0; j < numTrailingEdgePts; j++)
	    		continuousResult[i][j] *= normalizationCoeff(i);

	modMax = new double*[TRANSFORM_LEVELS];
	
	// ..and find its local minima and maxima
	for (i = 0; i < TRANSFORM_LEVELS; i++) {
		modMax[i] = new double[numTrailingEdgePts];
		modulusMaxima(continuousResult[i + 1], modMax[i], numTrailingEdgePts);
	}
	WL_Free2Dmem(continuousResult);

	int level = TRANSFORM_LEVELS / 2;

	if (level < 1) level = 1;

	// First, we'll find some local minima at an intermediate level
	// to track.
	int notchPosition = 0;
	list<int> mins;

	while (level > 0) {
		for (i = 0; i < numTrailingEdgePts; i++)
			if (modMax[level][i] < 0.0)
				mins.push_back(i);

		if (mins.size() <= 0) {
			level--;
			continue;
		}

		list<int>::iterator it = mins.begin();

		if (mins.size() == 1) {
			notchPosition = *it;

			if (0 == notchPosition)
				notchPosition = mChain->length() - mTipPos - 1;

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
		notchPosition = mChain->length() - mTipPos - 1;

	if (!notchPosition) {

		// Now, we'll take the lowest few mins, and look at how
		// they change over the transform levels.

		minVals = new double[mins.size()];
	
		list<int>::iterator it = mins.begin();
		for (i = 0; it != mins.end(); i++, it++)
			minVals[i] = modMax[level][*it];

		qsort(minVals, mins.size(), sizeof(double), compareDoubles);
		
		int numMinsToTrack;
		
		if ((int)mins.size() < NOTCH_NUM_MINS_TO_TRACK)
			numMinsToTrack = mins.size();
		else
			numMinsToTrack = NOTCH_NUM_MINS_TO_TRACK;
		
		positions = new int[numMinsToTrack];
		
		for (int count = 0; count < numMinsToTrack; count++) {
			it = mins.begin();
			for (i = 0; i < (int)mins.size() && it != mins.end(); i++) {
				if (minVals[count] == modMax[level][*it]) {
					positions[count] = *it;
					break;
				}
				++it;
			}
		}

		// Ok, now that we've got the few lowest mins,
		// let's find their corresponding positions in
		// a coarser level
		int coarserLevel = TRANSFORM_LEVELS - 2;
		
		int correspondingPos;
		//double alphaMax, alpha;
		double difMax = 0.0, dif;
		bool firstRun = true;
		
		for (i = 0; i < numMinsToTrack; i++) {
			correspondingPos = findClosestMin(
					5,
					modMax[coarserLevel],
					numTrailingEdgePts,
					positions[i]);
		
			// If we found a corresponding min in a coarser
			// level...
			if (-1 != correspondingPos) {
				
				//alpha = alphaK(
				//		level + 1,
				//		coarserLevel + 1,
				//		modMax[level][positions[i]],
				//		modMax[coarserLevel][correspondingPos]);
				dif = fabs(modMax[coarserLevel][correspondingPos]);
					//+ fabs(modMax[level][positions[i]]);
				
				if (firstRun) {
					firstRun = false;
					difMax = dif;
					notchPosition = positions[i];
				} else if (dif > difMax) {
					difMax = dif;
					notchPosition = positions[i];
				}
			}
		}

		if (firstRun)
			notchPosition = mChain->length() - 1 - mTipPos;

		delete[] minVals;
		minVals = NULL;
		delete[] positions;
		positions = NULL;
	}

	for (i = 0; i < TRANSFORM_LEVELS; i++) {
		delete[] modMax[i];
		modMax[i] = NULL;
	}

	delete[] modMax;
	modMax = NULL;

	/*
	list<zeroCrossingType> zeroCrossings =
	    findZeroCrossings(continuousResult[level], numTrailingEdgePts);

	list<zeroCrossingType>::iterator it = zeroCrossings.begin();

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

	delete[] src;
	src = NULL;

	return notchPosition + mTipPos;

    } catch (...) {
	delete[] src;

	for (int i = 0; i < TRANSFORM_LEVELS; i++)
		delete[] modMax[i];
	delete[] modMax;

	delete[] positions;
	delete[] minVals;

	throw;
    }
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
int Outline::findBeginLE()
{
	if (NULL == mChain)
		throw EmptyArgumentError("findLECutoff() [mChain]");

	if (mTipPos <= 2)
		throw InvalidArgumentError("findLECutoff() [int mTipPos <= 2]");

        if (mUserSetBeginLE)
		throw InvalidArgumentError("findLECutoff() [attempt to modify uset set LEbegin");
		
	int numPoints = (mTipPos - 1) / 2;

	double
		min = mChain->min(0, numPoints),
		max = mChain->max(0, numPoints);

	int roundedMin = (int) round(min);
	int numLevels = (int) round(max) - roundedMin;

	if (numLevels <= 1)
		return 0;

	int *levels = new int[numLevels];

	memset((void *)levels, 0, numLevels * sizeof(int));

	int i, idx;
	for (i = 0; i < numPoints; i++) {
		idx = (int)round((*mChain)[i]) - roundedMin;

		if (idx < 0)
			idx = 0;
		if (idx >= numLevels)
			idx = numLevels - 1;
		++levels[idx];
	}

	float *relativeFreq = new float[numLevels];
	
	for (i = 0; i < numLevels; i++)
		relativeFreq[i] = (float) levels[i] / numPoints;

	float
		curVariance,	// variance calculation for the current threshhold

		maxVariance,	// the maximum value for the variance so far

		fgAvgBrightness, // average level of the foreground

		bgAvgBrightness, // average level of the background

		fgArea;		// percentage of the chain which is foreground after
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

	for (int lev = 1; lev < numLevels; lev++) {
		fgAvgBrightness *= fgArea;
		bgAvgBrightness *= (1 - fgArea);

		fgArea += relativeFreq[lev];

		if (fgArea > 1) // if there's a rounding error
			fgArea = 1;

		fgAvgBrightness += (lev + 1) * relativeFreq[lev];
		bgAvgBrightness -= (lev + 1) * relativeFreq[lev];

		if (bgAvgBrightness < 0)  //if there's a rounding error
			bgAvgBrightness = 0;

		if (!fgArea)  // make sure we're not dividing by zero
			fgAvgBrightness = 0;
		else
			fgAvgBrightness /= fgArea;

		if (fgArea == 1)  // make sure we're not dividing by zero
			bgAvgBrightness = 0;
		else
			bgAvgBrightness /= 1 - fgArea;

		curVariance = fgArea * (1 - fgArea) * (fgAvgBrightness - bgAvgBrightness)
			* (fgAvgBrightness - bgAvgBrightness);

		if (curVariance > maxVariance) {
			maxVariance = curVariance;
			optThreshold = lev;
		}
	}

	delete[] levels;
	delete[] relativeFreq;

	double cutoffVal = (double)optThreshold + roundedMin;

	for (i = 0; i < numPoints - 1; i++) {
		if ((*mChain)[i] < cutoffVal)
			return i;
	}

	return 0;
}

int Outline::findEndLE()
{
	if (NULL == mChain)
		throw EmptyArgumentError("findLEEnd() [data member *mChain]");

	if (mTipPos <= 2)
		throw InvalidArgumentError("findLEEnd() [data member mTipPos <= 2]");
		
	if (mUserSetEndLE)
		throw InvalidArgumentError("findLEEnd() [attempt to change user specified EndLE]");
		
	double
		min = mChain->min(mBeginLE, mTipPos),
		max = mChain->max(mBeginLE, mTipPos);

	int roundedMin = (int) round(min);
	int numLevels = (int) round(max) - roundedMin;
	
	if (numLevels <= 1)
		return 0;

	int *levels = new int[numLevels];

	memset((void *)levels, 0, numLevels * sizeof(int));

	int i, idx;

	for (i = mBeginLE; i < mTipPos; i++) {
		idx = (int)round((*mChain)[i]) - roundedMin;

		if (idx < 0)
			idx = 0;
		else if (idx >= numLevels)
			idx = numLevels - 1;

		++levels[idx];
	}

	float *relativeFreq = new float[numLevels];
	
	for (i = 0; i < numLevels; i++)
		relativeFreq[i] = (float) levels[i] / (mTipPos - mBeginLE);

	float
		curVariance,	// variance calculation for the current threshhold

		maxVariance,	// the maximum value for the variance so far

		fgAvgBrightness, // average level of the foreground

		bgAvgBrightness, // average level of the background

		fgArea;		// percentage of the chain which is foreground after
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

	for (int lev = 1; lev < numLevels; lev++) {
		fgAvgBrightness *= fgArea;
		bgAvgBrightness *= (1 - fgArea);

		fgArea += relativeFreq[lev];

		if (fgArea > 1) // if there's a rounding error
			fgArea = 1;

		fgAvgBrightness += (lev + 1) * relativeFreq[lev];
		bgAvgBrightness -= (lev + 1) * relativeFreq[lev];

		if (bgAvgBrightness < 0)  //if there's a rounding error
			bgAvgBrightness = 0;

		if (!fgArea)  // make sure we're not dividing by zero
			fgAvgBrightness = 0;
		else
			fgAvgBrightness /= fgArea;

		if (fgArea == 1)  // make sure we're not dividing by zero
			bgAvgBrightness = 0;
		else
			bgAvgBrightness /= 1 - fgArea;

		curVariance = fgArea * (1 - fgArea) * (fgAvgBrightness - bgAvgBrightness)
			* (fgAvgBrightness - bgAvgBrightness);

		if (curVariance > maxVariance) {
			maxVariance = curVariance;
			optThreshold = lev;
		}
	}

	delete[] levels;
	delete[] relativeFreq;

	double cutoffVal = (double)optThreshold + roundedMin;

	for (i = mTipPos - 1; i > mBeginLE + 1; i--) {
		if ((*mChain)[i] < cutoffVal)
			return i;
	}

	return mBeginLE + 1;
}


int Outline::findPointOfInflection()
{
	int numPoints = mChain->length() - mTipPos - 1;
	
	Chain *smoothChain = new Chain(*mChain);
	smoothChain->smooth15();

	double *der = new double[numPoints - 1];
	
	for (int i = mTipPos + 1, j = 0; i < smoothChain->length() - 1; i++, j++)
		der[j] = (*smoothChain)(i + 1) - (*smoothChain)(i);

	delete smoothChain;
	list<zeroCrossingType> zeroCrossings =
		findZeroCrossings(der, numPoints - 1);

	delete[] der;

	list<zeroCrossingType>::iterator it = zeroCrossings.end();

	--it;
	return it->position + mTipPos;	
}

// Function to compare doubles for qsort
static int compareDoubles(const void *a, const void *b)
{
	double aVal, bVal;

	aVal = *((double *) a);
	bVal = *((double *) b);

	if (aVal < bVal)
		return -1;

	return 1;
}

// return of -1 indicates inability to find a closest max
static int findClosestMax(const double *modmax, int len, int prevPosition)
{
    if (NULL == modmax)
	throw EmptyArgumentError("findClosestMax()");

    if (prevPosition < 0 || len <= 0 || prevPosition >= len)
	throw InvalidArgumentError("findClosestMax()");


    if (modmax[prevPosition] > 0)
	return prevPosition;

    int lowDist = 0,
     highDist = 0;
    int lowMax = -1,
     highMax = -1;

    for (int i = prevPosition - 1; i >= 0 && lowMax == -1; i--, lowDist++) {
	if (modmax[i] > 0.0)
	    lowMax = i;
    }

    for (int j = prevPosition + 1; j < len && highMax == -1;
	 j++, highDist++) {
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
static int findClosestMin(int dist, const double *modmax, int len, int prevPosition)
{
    if (NULL == modmax)
	throw EmptyArgumentError("findClosestMin()");

    if (prevPosition < 0 || len <= 0 || prevPosition >= len)
	throw InvalidArgumentError("findClosestMin()");

    if (modmax[prevPosition] < 0.0)
	return prevPosition;

    int
	lowDist = 0,
	highDist = 0;
    int
	lowMin = -1,
    	highMin = -1;

    for (int i = prevPosition - 1; i >= 0 && i >= prevPosition - dist && lowMin == -1; i--, lowDist++) {
	if (modmax[i] < 0.0)
	    lowMin = i;
    }

    for (int j = prevPosition + 1; j < len && j <= prevPosition + dist && highMin == -1; j++, highDist++) {
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

static list<zeroCrossingType> findZeroCrossings(const double *src, int numPoints)
{
    if (NULL == src)
	throw
	    EmptyArgumentError("findZeroCrossings(): [const double *src]");

    if (numPoints < 3)
	throw InvalidArgumentError("findZeroCrossings(): [int numPoints]");

    try {
	list < zeroCrossingType > zeroCrossings;

	double *modmax = new double[numPoints];
	modulusMaxima(src, modmax, numPoints);

	for (int i = 0; i < numPoints - 1; i++) {
	    if ((src[i] < 0.0 && src[i + 1] > 0.0)
		|| (src[i] > 0.0 && src[i + 1] < 0.0)) {
		zeroCrossingType zc;
		zc.position = i;

		zc.leftMag = findLeftExtremum(modmax, numPoints, i);
		zc.rightMag = findRightExtremum(modmax, numPoints, i);

		zeroCrossings.push_back(zc);

	    } else if (src[i] == 0.0) {
		zeroCrossingType zc;
		zc.position = i;

		zc.leftMag = findLeftExtremum(modmax, numPoints, i);
		zc.rightMag = findRightExtremum(modmax, numPoints, i);

		zeroCrossings.push_back(zc);

		i++;
	    }
	}

	delete[]modmax;

	return zeroCrossings;

    }
    catch(...) {
	throw;
    }
}

static double findLeftExtremum(const double *src, int numPoints, int startPos)
{
    if (NULL == src)
	throw EmptyArgumentError();

    if (numPoints < 2 || startPos < 0 || startPos >= numPoints)
	throw InvalidArgumentError();

    try {
	for (int i = startPos - 1; i >= 0; i--)
	    if (src[i] != 0.0)
		return src[i];

	return 0;

    }
    catch(...) {
	throw;
    }
}

static double findRightExtremum(const double *src, int numPoints, int startPos)
{
    if (NULL == src)
	throw EmptyArgumentError();

    if (numPoints < 2 || startPos < 0 || startPos >= numPoints)
	throw InvalidArgumentError();

    try {
	for (int i = startPos + 1; i < numPoints; i++)
	    if (src[i] != 0.0)
		return src[i];

	return numPoints - 1;

    }
    catch(...) {
	throw;
    }
}

static double alphaK(int level1, int level2, double mag1, double mag2)
{
	if (level1 == level2)
		return 0.0;
	double
		ord1 = fabs(mag1),
		ord2 = fabs(mag2);
	
	if (ord1 <= 0.0)
		return 0.0;
	
	if (ord2 <= 0.0)
		return 0.0;

	return (log2(ord2) - log2(ord1)) / (level2 - level1);
}

//***008OL - Don't know why this is here. It is defined in MapContour.*
/* remove for now 
FloatContour* mapContour(
		FloatContour *c,
		point_t p1,
		point_t p2,
		point_t p3,
		point_t desP1,
		point_t desP2,
		point_t desP3
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

	FloatContour *dstContour = NULL;
	float **a = NULL, **b = NULL;
	int i;

	try {
		a = new float*[3];
		b = new float*[3];
	
		for (i = 0; i < 3; i++) {
			a[i] = new float[3];
			a[i][2] = 1.0f;
			b[i] = new float[4];
		}

		a[0][0] = p1.x;
		a[0][1] = p1.y;

		a[1][0] = p2.x;
		a[1][1] = p2.y;
	
		a[2][0] = p3.x;
		a[2][1] = p3.y;

		size_t rowSize = 3 * sizeof(float);
	
		for (i = 0; i < 3; i++)
			memcpy(b[i], a[i], rowSize);

		b[0][3] = (float)desP1.x;
		b[1][3] = (float)desP2.x;
		b[2][3] = (float)desP3.x;

		// First, we'll find the transformation coefficients
		// which will map the contour.
		gaussj((float **)a, 3, (float **)b, 4);

		float transformCoeff[2][3];
		for (i = 0; i < 3; i++)
			transformCoeff[0][i] = b[i][3];

		// Ok, now we have one row of coefficients.  We now need to reinitialize
		// the a and b matrices since the gaussj() solver function has changed
		// their values
		for (i = 0; i < 3; i++)
			a[i][2] = 1.0f;

		a[0][0] = p1.x;
		a[0][1] = p1.y;

		a[1][0] = p2.x;
		a[1][1] = p2.y;
	
		a[2][0] = p3.x;
		a[2][1] = p3.y;

		for (i = 0; i < 3; i++)
			memcpy(b[i], a[i], rowSize);

		// and, obviously, this time, we're going to solve for the second row of
		// coefficients, so we'll put the desired y values in the augmented
		// section
		b[0][3] = (float)desP1.y;
		b[1][3] = (float)desP2.y;
		b[2][3] = (float)desP3.y;

		gaussj((float **)a, 3, (float **)b, 4);

		// Save the transform coefficients
		for (i = 0; i < 3; i++)
			transformCoeff[1][i] = b[i][3];

		// Now that we have all the required coefficients, we'll transform
		// the points in the original Contour, and store them in a new one
	
		for (i = 0; i < 3; i++) {
			delete[] a[i];
			a[i] = NULL;
			delete[] b[i];
			b[i] = NULL;
		}

		delete[] a;
		a = NULL;
		delete[] b;
		b = NULL;

		dstContour = new FloatContour();
		int numPoints = c->length();
		float cx, cy;
		float x, y;
		for (i = 0; i < numPoints; i++) {
			cx = (*c)[i].x;
			cy = (*c)[i].y;
		
			x =	transformCoeff[0][0] * cx
				+ transformCoeff[0][1] * cy
				+ transformCoeff[0][2];
			y =	transformCoeff[1][0] * cx
				+ transformCoeff[1][1] * cy
				+ transformCoeff[1][2];

			dstContour->addPoint(x, y);
		}

		return dstContour;

	} catch (...) {
		for (i = 0; i < 3; i++) {
			delete[] a[i];
			delete[] b[i];
		}

		delete[] a;
		delete[] b;
		
		delete dstContour;

		throw;
	}
}
*/

