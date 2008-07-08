// feature.cxx

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "feature.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <list>

#include "waveletUtil.h"
#include "wavelet/wlcore.h"

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

/////////////////////////////////////////////////////

// Finds the tip as the index into the chain array
int findTip(const Chain *chain)
{
    if (NULL == chain)
	    throw EmptyArgumentError("findTip() [const Chain *chain]");

    try {
	int numPoints = chain->length();

	// First, make a copy without the first value in the chain,
	// since the first value skews the rest of the chain and is
	// unnecessary for our purposes here
	double *src = new double[numPoints - 1];

	memcpy(src, &((*chain)[1]), (numPoints - 1) * sizeof(double));
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
		double max = modMax[0];
		for (int i = 1; i < numPoints - 1; i++) {
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

int findNotch(const Chain *chain, int tipPosition)
{
    if (NULL == chain)
	throw EmptyArgumentError("findNotch(): [const Chain *chain]");

    if (tipPosition < 0 || tipPosition > chain->length())
	    throw InvalidArgumentError("findNotch() [int tipPosition]");

    double *src = NULL;
    double **modMax = NULL;
    double *minVals = NULL;
    int *positions = NULL;

    try {
	// We're only going to be looking on the trailing edge
	// of the fin for the most significant notch, so we'll
	// build a source vector from that area
	int numTrailingEdgePts = chain->length() - tipPosition - 1 - 5;
	src = new double[numTrailingEdgePts];

	memcpy(src, &((*chain)[tipPosition + 1]), numTrailingEdgePts * sizeof(double));
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
				notchPosition = chain->length() - tipPosition - 1;

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
		notchPosition = chain->length() - tipPosition - 1;

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
			notchPosition = chain->length() - 1 - tipPosition;

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

	return notchPosition + tipPosition;

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

// findLECutoff
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
int findLECutoff(const Chain *chain, int tipPosition)
{
	if (NULL == chain)
		throw EmptyArgumentError("findLECutoff() [Chain *chain]");

	if (tipPosition <= 2)
		throw InvalidArgumentError("findLECutoff() [int tipPosition <= 2]");
		
	int numPoints = (tipPosition - 1) / 2;

	double
		min = chain->min(0, numPoints),
		max = chain->max(0, numPoints);

	int roundedMin = (int) round(min);
	int numLevels = (int) round(max) - roundedMin;

	if (numLevels <= 1)
		return 0;

	int *levels = new int[numLevels];

	memset((void *)levels, 0, numLevels * sizeof(int));

	int i, idx;
	for (i = 0; i < numPoints; i++) {
		idx = (int)round((*chain)[i]) - roundedMin;

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
		if ((*chain)[i] < cutoffVal)
			return i;
	}

	return 0;
}

int findLEEnd(const Chain *chain, int leBegin, int tipPosition)
{
	if (NULL == chain)
		throw EmptyArgumentError("findLEEnd() [Chain *chain]");

	if (tipPosition <= 2)
		throw InvalidArgumentError("findLEEnd() [int tipPosition <= 2]");
		
	double
		min = chain->min(leBegin, tipPosition),
		max = chain->max(leBegin, tipPosition);

	int roundedMin = (int) round(min);
	int numLevels = (int) round(max) - roundedMin;
	
	if (numLevels <= 1)
		return 0;

	int *levels = new int[numLevels];

	memset((void *)levels, 0, numLevels * sizeof(int));

	int i, idx;

	for (i = leBegin; i < tipPosition; i++) {
		idx = (int)round((*chain)[i]) - roundedMin;

		if (idx < 0)
			idx = 0;
		else if (idx >= numLevels)
			idx = numLevels - 1;

		++levels[idx];
	}

	float *relativeFreq = new float[numLevels];
	
	for (i = 0; i < numLevels; i++)
		relativeFreq[i] = (float) levels[i] / (tipPosition - leBegin);

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

	for (i = tipPosition - 1; i > leBegin + 1; i--) {
		if ((*chain)[i] < cutoffVal)
			return i;
	}

	return leBegin + 1;
}

// Attempts to determine the angle of the fin's leading edge.
double findLEAngle(const Chain *chain, int tipPosition)
{
	if (NULL == chain)
		throw EmptyArgumentError("findLEAngle() [const Chain *chain]");
	if (tipPosition <= 1)
		throw InvalidArgumentError("findLEAngle() [int tipPosition <= 1]");
	
	int numPoints = (int)round((tipPosition - 1) * (1.0 - 2.0 * LE_TRIM_AMOUNT));
	int startPos = (int) round((tipPosition - 1) * LE_TRIM_AMOUNT);

	Chain *smoothChain = new Chain(*chain);
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

/*
int findPointOfInflection(const point_t *p, int tipPosition, int chainEnd)
{
	if (NULL == p)
		throw EmptyArgumentError("findPointOfInflection() [const point_t *p]");
	int firstDerLen = chainEnd - tipPosition - 2;

	// just a failsafe in case this function gets passed a very
	// small point array
	if (firstDerLen <= 2)
		return (chainEnd - tipPosition) / 2;

	double *firstDer, *secondDer;
	
	firstDer = new double[firstDerLen];

	// first, smooth the points quite a bit
	point_t *s = new point_t[firstDerLen + 1];
	memcpy(s, &p[tipPosition + 1], (firstDerLen + 1) * sizeof(point_t));
	smoothPoints(s, firstDerLen + 1);

	double num, den;
	
	for (int i = 0; i < firstDerLen; i++) {
		den = (double) s[i + 1].y - s[i].y;
	       	
		if (den == 0.0)
			firstDer[i] = 0.0;

		else {
			num = (double) s[i + 1].x - s[i].x;
			firstDer[i] = num / den;
		}
	}

//	for (int k = 0; k < firstDerLen; k++)
//		cout << firstDer[k] << endl;
	secondDer = new double[firstDerLen - 1];

	for (int j = 0; j < firstDerLen - 1; j++) {
		den = (double)s[j + 1].y - s[j].y;

		if (den == 0.0)
			secondDer[j] = 0.0;

		else {
			num = firstDer[j + 1] - firstDer[j];
			secondDer[j] = num / den;
		}
	}
	delete[] firstDer;
	delete[] s;
//	for (int k = 0; k < firstDerLen - 1; k++)
//		cout << secondDer[k] << endl;
	delete[] secondDer;

	return 0;
}
*/
int findPointOfInflection(
		const Chain *chain,
		int tipPosition
)
{
	int numPoints = chain->length() - tipPosition - 1;
	
	Chain *smoothChain = new Chain(*chain);
	smoothChain->smooth15();

	double *der = new double[numPoints - 1];
	
	for (int i = tipPosition + 1, j = 0; i < smoothChain->length() - 1; i++, j++)
		der[j] = (*smoothChain)(i + 1) - (*smoothChain)(i);

	delete smoothChain;
	list<zeroCrossingType> zeroCrossings =
		findZeroCrossings(der, numPoints - 1);

	delete[] der;

	list<zeroCrossingType>::iterator it = zeroCrossings.end();

	/*
	double maxDist = 0.0;
	int p = 0; // the point of inflection

	while (it != zeroCrossings.end()) {
		double curDist = (fabs(it->leftMag) + fabs(it->rightMag));
		if (curDist > maxDist) {
		    maxDist = curDist;
		    p = it->position;
		}
	    ++it;
	}
	// to be nice
	zeroCrossings.clear();

	if (0.0 == maxDist)
		return tipPosition + (chainEnd - tipPosition) / 2;

	return p + tipPosition;
	*/

	--it;
	return it->position + tipPosition;	
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
