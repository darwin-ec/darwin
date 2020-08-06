//*******************************************************************
//   file: Chain.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (7/22/2005)
//         -- reformatting of code and addition of comment blocks
//         -- changes to incorporate Outline CLASS in project
//
//*******************************************************************

#include <cmath>
#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <vector>
#include <list>
#include "Chain.h"
#include "Error.h"
#include "utility.h"

using namespace std;

static const double CLOSE_TO_ZERO = 1e-10;


//***008OL - NOTE: The Chain class will no longer conatin the Contour
// or FloatContour mChainPoints or mSavedPoints.  This point information
// is now part of the Outline class as the mChainPoints member
// variable.


/////////////////////////////////
// Utility function prototypes //
/////////////////////////////////

static void copy_point(point_t original, point_t * copy);
static double dot(point_t vector1, point_t vector2);
static double norm(point_t vector);
static void vsubtract(point_t vector1, point_t vector2, point_t * temp);
static void vadd(point_t vector1, point_t vector2, point_t * temp);
static void vscalar(double t, point_t vector, point_t * temp);

////////////////////////////////////////
// End of utility function prototypes //
////////////////////////////////////////

/*****************************************************************************/
/*
 * COPY POINT COORDINATES FROM ONE STRUCT TO ANOTHER, WORKS AS POINT = POINT
 */
static void copy_point(point_t original, point_t * copy)
{
    copy->x = original.x;
    copy->y = original.y;
    copy->z = original.z;
}

/*****************************************************************************/
/*
 * RETURN INT DOT PRODUCT VALUE OF TWO VECTORS IN 2D
 */
static double dot(point_t vector1, point_t vector2)
{
    double temp = 0;

    temp = (vector1.x * vector2.x) + (vector1.y * vector2.y);
    return temp;
}

/*****************************************************************************/
/*
 * RETURN THE NORMAL (LENGTH) OF A 2D VECTOR
 */
static double norm(point_t vector)
{
    double temp = 0;

    temp = dot(vector, vector);
    return sqrt(temp);
}

/*****************************************************************************/
/*
 * RETURNS VECTOR1 - VECTOR2, REGULAR VECTOR SUBTRACTION
 */
static void vsubtract(point_t vector1, point_t vector2, point_t * temp)
{
    temp->x = vector1.x - vector2.x;
    temp->y = vector1.y - vector2.y;
    temp->z = 1;
}

/*****************************************************************************/
/*
 * RETURNS VECTOR1 + VECTOR2, REGULAR VECTOR ADDITION
 */
static void vadd(point_t vector1, point_t vector2, point_t * temp)
{
    temp->x = vector1.x + vector2.x;
    temp->y = vector1.y + vector2.y;
    temp->z = 1;
}

/*****************************************************************************/
/*
 * RETURNS NEW COORDINATES OF A SCALAR TIMES A VECTOR
 */
static void vscalar(double t, point_t vector, point_t * temp)
{
    temp->x = t * vector.x;
    temp->y = t * vector.y;
    temp->z = 1;
}

/***008OL following functions removed (no longer needed)
 *
 * Chain::Chain(Contour *contour, double radius)
 * Chain::Chain(FloatContour *contour, double radius)
 * void Chain::computeChainPoints(point_t *p, int pLength, double radius)
 * double Chain::getRadius() const
 * FloatContour* Chain::getChainPoints()  // how to pass this back out??
 * void Chain::getXYPointNum(int num, int &x, int &y)
 *
 */


//*******************************************************************
//
// Chain::Chain(FloatContour *fc)  //***008OL entire function added
//
//    Used when FloatContour is read from DB file.
//    
//    Open Question: Should updateRelativeChain() be used here?
//
//    PRE:  fc is normalized and evenly spaced
//    POST: Chain indices correspond to fc indices
// 
Chain::Chain(FloatContour *fc)
	:  
	  mChain(NULL),
	  mRelativeChain(NULL)
{
	if (NULL == fc)
		throw EmptyArgumentError("Chain ctor");

	mNumPoints = fc->length();

	vector<double> chain, relativeChain;

	// NOTE: the Chain angle at each pooint is ALWAYS the angle of 
	// the INCOMING edge FROM the previous contour point - JHS
 
	if (mNumPoints > 1)
	{
		// initial angle of chain is angle of first point relative to (0,0)
		//chain.push_back(rtod(atan2((*fc)[0].y, (*fc)[0].x))); //***2.1mt
		//***2.01 - now just duplicate the first relative angle
		chain.push_back(rtod(atan2((*fc)[1].y - (*fc)[0].y, (*fc)[1].x - (*fc)[0].x))); //***2.01 

		chain.push_back(rtod(atan2((*fc)[1].y - (*fc)[0].y, (*fc)[1].x - (*fc)[0].x)));

		float lastAngle = chain[1]; //***2.1mt - use this to prevent any quadrant discontinuity 

		// initial angle in relativeChain is angle change across point[0]
		//***1.2 - I think initial angle is meaningless and the
		// initial meaningful angle is actually angle change across point[1]
		relativeChain.push_back(0.0); //***1.2 - added this

		// without line above the relative chain length & indexing are off by one
		relativeChain.push_back(chain[0] - chain[1]);

		for (int i = 2; i < mNumPoints; i++) 
		{
			double angle = rtod(atan2((*fc)[i].y - (*fc)[i-1].y, (*fc)[i].x - (*fc)[i-1].x)); //***2.01

			//***2.01 - prevent ANY discontinuity of greater than 180 degrees
			if (angle - lastAngle > 180.0)
				chain.push_back(angle - 360.0);
			else if (angle - lastAngle < -180.0)
				chain.push_back(angle + 360.0);
			else
				chain.push_back(angle);

			lastAngle = chain[i];

			//***2.01 - end

			//////////////////////////// begin old ///////////////////////
			relativeChain.push_back(chain[i-1] - chain[i]);
			//////////////////////// end old ///////////////////////////
		}
	}

	// NOTE: there is NO useful change in angle across either
	// the first point or the last point in fc

	// now copy vectors to double arrays

	mChain = new double[chain.size()];
	mRelativeChain = new double[relativeChain.size()];

	vector<double>::iterator cIt = chain.begin();

	for (int cnt = 0; cIt != chain.end(); cnt++, cIt++)
		mChain[cnt] = *cIt;

	vector<double>::iterator rCIt = relativeChain.begin();

	for (int rc = 0; rCIt != relativeChain.end(); rc++, rCIt++)
		mRelativeChain[rc] = *rCIt;
}


//*******************************************************************
//
// Chain::Chain(const Chain &c)
//
//    COPY Constructor (makes copy of "referenced" original).
//
Chain::Chain(const Chain &c)
{
	mNumPoints = c.mNumPoints;
	//mRadius = c.mRadius; removed 008OL
	mChain = new double[mNumPoints];
	mRelativeChain = new double[mNumPoints];
	//mSavedPoints = new point_t[mNumPoints]; removed 008OL
  //mChainPoints = new FloatContour((const FloatContour &)c.mChainPoints);  //**006DF,008OL removed JHS

	memcpy(mChain, c.mChain, mNumPoints * sizeof(double));
	memcpy(mRelativeChain, c.mRelativeChain, mNumPoints * sizeof(double));
	//memcpy(mSavedPoints, c.mSavedPoints, mNumPoints * sizeof(point_t)); removed 008OL
}


//*******************************************************************
//
// Chain::Chain(const Chain* c)  //***008OL new function
//
//    Alternative COPY Constructor (makes copy of "pointed to" original).
//    Called from Outline constructor.
//
Chain::Chain(const Chain* c)
{
	mNumPoints = c->mNumPoints;
	//mRadius = c->mRadius; removed 008OL
	mChain = new double[mNumPoints];
	mRelativeChain = new double[mNumPoints];
	//mChainPoints = new FloatContour((const FloatContour &)c->mChainPoints);  //**006DF,008OL removed JHS

	memcpy(mChain, c->mChain, mNumPoints * sizeof(double));
	memcpy(mRelativeChain, c->mRelativeChain, mNumPoints * sizeof(double));
}


//*******************************************************************
//
// Chain::~Chain()
//
//    Class DESTRUTOR
//
Chain::~Chain()
{
	delete[] mChain;
	delete[] mRelativeChain;
	//delete[] mSavedPoints; removed 008OL
  //delete mChainPoints;  //**006DF,008OL removed JHS
}


//*******************************************************************
//
// Chain& Chain::operator=(const Chain &c)
//
//    Overloaded ASSIGNMENT operator
//
Chain& Chain::operator=(const Chain &c)
{
	// check if this is a self-assignment
	if (this == &c)
		return *this;

	mNumPoints = c.mNumPoints;
	// mRadius = c.mRadius; removed 008OL
	mChain = new double[mNumPoints];
	mRelativeChain = new double[mNumPoints];
	//mSavedPoints = new point_t[mNumPoints]; removed 008OL
  //mChainPoints = new FloatContour((const FloatContour &)c.mChainPoints);  //**006DF,008OL removed JHS

	memcpy(mChain, c.mChain, mNumPoints * sizeof(double));
	memcpy(mRelativeChain, c.mRelativeChain, mNumPoints * sizeof(double));
	//memcpy(mSavedPoints, c.mSavedPoints, mNumPoints * sizeof(point_t)); removed 008OL

	return *this;
}


//*******************************************************************
//
// int Chain::length() const
//
//    Returns number of points in chain.
//
int Chain::length() const
{
	return mNumPoints;
}


//*******************************************************************
//
// double* Chain::getData()
//
//    Returns copy of PRIVATE pointer mChain (the array of doubles containing
//    the absolute chain angles). 
//
double* Chain::getData()
{
	return mChain;
}


//*******************************************************************
//
// const double* Chain::getData()
//
//    Returns CONSTANT copy of PRIVATE pointer mChain (the array of doubles 
//    containing the absolute chain angles). 
//
const double* Chain::getData() const
{
	return mChain;
}


//*******************************************************************
//
// double* Chain::getRelativeData()
//
//    Returns copy of PRIVATE pointer mrelativeChain (the array of doubles 
//    containing the relative chain angles). 
//
double* Chain::getRelativeData()
{
	return mRelativeChain;
}


//*******************************************************************
//
// const double* Chain::getRelativeData()
//
//    Returns copy of PRIVATE pointer mRelativeChain (the array of doubles 
//    containing the relative chain angles). 
//
const double* Chain::getRelativeData() const
{
	return mRelativeChain;
}


//*******************************************************************
//
// double& Chain::getRelativeAngle(int angleNum)
//
//    Returns PUBLIC reference to specified relative angle.
//
//    PRE:  0 < angleNum < mNumPoints
//
double& Chain::getRelativeAngle(int angleNum)
{
	if (angleNum < 0 || angleNum >= mNumPoints)
		throw BoundsError("Chain::getRelativeAngle");

	return mRelativeChain[angleNum];
}


//*******************************************************************
//
// const double& Chain::getRelativeAngle(int angleNum)
//
//    Returns CONST PUBLIC reference to specified relative angle.
//
//    PRE:  0 < angleNum < mNumPoints
//
const double& Chain::getRelativeAngle(int angleNum) const
{
	if (angleNum < 0 || angleNum >= mNumPoints)
		throw BoundsError("Chain::getRelativeAngle");

	return mRelativeChain[angleNum];
}


//*******************************************************************
//
// double& Chain::getAngle(int angleNum)
//
//    Returns PUBLIC reference to specified absolute angle.
//
//    PRE:  0 < angleNum < mNumPoints
//
double& Chain::getAngle(int angleNum)
{
	if (angleNum < 0 || angleNum >= mNumPoints)
		throw BoundsError("Chain::getAngle");

	return mChain[angleNum];
}

//*******************************************************************
//
// const double& Chain::getAngle(int angleNum)
//
//    Returns CONST PUBLIC reference to specified absolute angle.
//
//    PRE:  0 < angleNum < mNumPoints
//
const double& Chain::getAngle(int angleNum) const
{
	if (angleNum < 0 || angleNum >= mNumPoints)
		throw BoundsError("Chain::getAngle");

	return mChain[angleNum];
}

//*******************************************************************
//
// double Chain::min() const
//
//    Returns MINIMUM chain angle value.  Performs linear search to
//    determine value.
//
double Chain::min() const
{
	if (mNumPoints == 0)
		return 0.0;

	double minVal = mChain[0];
	for (int i = 1; i < mNumPoints; i++) {
		if (mChain[i] < minVal)
			minVal = mChain[i];
	}

	return minVal;
}

//*******************************************************************
//
// double Chain::max() const
//
//    Returns MAXIMUM chain angle value.  Performs linear search to
//    determine value.
//
double Chain::max() const
{
	if (mNumPoints == 0)
		return 0.0;

	double maxVal = mChain[0];
	for (int i = 1; i < mNumPoints; i++) {
		if (mChain[i] > maxVal)
			maxVal = mChain[i];
	}

	return maxVal;
}

//*******************************************************************
//
// double Chain::min(int start, int end) const
//
//    Returns MINIMUM chain angle value within specified portion of
//    chain.  Performs linear search within range [start, end) to
//    determine value.
// 
double Chain::min(int start, int end) const
{
  // NOTE: returns minimum value in range [start..end)

	if (mNumPoints == 0)
		return 0.0;

	if (start < 0 || start >= mNumPoints || start > end)
		throw InvalidArgumentError("Chain::min [invalid int start]");

	if (end < 0 || end > mNumPoints)
		throw InvalidArgumentError("Chain::min [invalid int end]");

	double minVal = mChain[0];
	for (int i = start; i < end; i++) {
		if (mChain[i] < minVal)
			minVal = mChain[i];
	}

	return minVal;
}

//*******************************************************************
//
// double Chain::max(int start, int end) const
//
//    Returns MAXIMUM chain angle value within specified portion of
//    chain.  Performs linear search within range [start, end) to
//    determine value.
// 
double Chain::max(int start, int end) const
{
  // NOTE: returns maximum value in range [start..end)
	
  if (mNumPoints == 0)
		return 0.0;

	if (start < 0 || start >= mNumPoints || start > end)
		throw InvalidArgumentError("Chain::max [invalid int start]");

	if (end < 0 || end > mNumPoints)
		throw InvalidArgumentError("Chain::max [invalid int end]");

	double maxVal = mChain[0];
	for (int i = start; i < end; i++) {
		if (mChain[i] > maxVal)
			maxVal = mChain[i];
	}

	return maxVal;
}


//*******************************************************************
//
// double Chain::minRelative() const
//
//    Returns MINIMUM relative chain angle value.  Performs linear search to
//    determine value.
//
double Chain::minRelative() const
{
  //***008OL NOTE: Since ther is NO useful angle at first or
  // last point along contour, this returns the minimum
  // relative angle in range [1..mNumPoints-1)

	if (mNumPoints == 0)
		return 0.0;

	//***008OL changed range of search below
  double minVal = mRelativeChain[1];
	for (int i = 2; i < mNumPoints-1; i++) {
		if (mRelativeChain[i] < minVal)
			minVal = mRelativeChain[i];
	}

	return minVal;
}


//*******************************************************************
//
// double Chain::maxRelative() const
//
//    Returns MAXIMUM relative chain angle value.  Performs linear search to
//    determine value.
//
double Chain::maxRelative() const
{
  //***008OL NOTE: Since ther is NO useful angle at first or
  // last point along contour, this returns the minimum
  // relative angle in range [1..mNumPoints-1)

	if (mNumPoints == 0)
		return 0.0;

	//***008OL changed range of search below
	double maxVal = mRelativeChain[1];
	for (int i = 2; i < mNumPoints-1; i++) {
		if (mRelativeChain[i] >maxVal)
			maxVal = mRelativeChain[i];
	}

	return maxVal;
}


//*******************************************************************
//
// void Chain::smooth7()
//
//    SMOOTHES mChain using an interval of 7.  A temporary chain copy
//    is created and is padded on leading end [0,3] with mChain[1] (the first
//    meaningful angle) and on the traling end [mNumPoints+3,mNumPoints+5] 
//    with mChain[mNumpoints-1] (the last meaningful angle.  Each angle in
//    the NEW mChain is the average (mean) of 7 values centered on the
//    same location as the value it replaced.
//
void Chain::smooth7()
{
    if (mNumPoints == 0 || mChain == NULL || mRelativeChain == NULL)
	    return;

    int
	chI,
    	sI;

    double *tempChain;

    tempChain = new double[mNumPoints + 6];

    memcpy(&tempChain[3], mChain, mNumPoints * sizeof(double));

    tempChain[0] = mChain[1]; //005CH
    tempChain[1] = mChain[1]; //005CH
    tempChain[2] = mChain[1]; //005CH
    tempChain[3] = mChain[1]; //005CH
    tempChain[mNumPoints + 3] = mChain[mNumPoints - 1];
    tempChain[mNumPoints + 4] = mChain[mNumPoints - 1];
    tempChain[mNumPoints + 5] = mChain[mNumPoints - 1];

    chI = 3; //005CH

    double *smoothChain = new double[mNumPoints];

    while (chI < mNumPoints + 3) { //005CH
	sI = chI - 3;
	smoothChain[sI] = (tempChain[chI - 3]
			     + tempChain[chI - 2]
			     + tempChain[chI - 1]
			     + tempChain[chI]
			     + tempChain[chI + 1]
			     + tempChain[chI + 2]
			     + tempChain[chI + 3])
				 / 7.0;
	chI++;
    }
    delete[] tempChain;
    delete[] mChain;

    mChain = smoothChain;

    Chain::updateRelativeChain();
}


//*******************************************************************
//
// void Chain::smooth5()
//
//    SMOOTHES mChain using an interval of 5.  A temporary chain copy
//    is created and is padded on leading end [0,2] with mChain[1] (the first
//    meaningful angle) and on the traling end [mNumPoints+2,mNumPoints+3] 
//    with mChain[mNumpoints-1] (the last meaningful angle.  Each angle in
//    the NEW mChain is the average (mean) of 5 values centered on the
//    same location as the value it replaced.
//
void Chain::smooth5()
{
    if (mNumPoints == 0 || mChain == NULL || mRelativeChain == NULL)
	    return;

    int
	chI,
    	sI;

    double *tempChain = new double[mNumPoints + 4];

    memcpy(&tempChain[2], mChain, mNumPoints * sizeof(double));

    tempChain[0] = mChain[1]; //005CH
    tempChain[1] = mChain[1]; //005CH
    tempChain[2] = mChain[1]; //005CH
    tempChain[mNumPoints + 2] = mChain[mNumPoints - 1];
    tempChain[mNumPoints + 3] = mChain[mNumPoints - 1];
    chI = 2;

    double *smoothChain = new double[mNumPoints];

    while (chI < mNumPoints + 2) {
	sI = chI - 2;
	smoothChain[sI] =
		 (tempChain[chI - 2]
		+ tempChain[chI - 1]
		+ tempChain[chI]
		+ tempChain[chI + 1]
		+ tempChain[chI + 2])
		/ 5.0;
	chI++;
    }
    delete[] tempChain;
    delete[] mChain;

    mChain = smoothChain;

    Chain::updateRelativeChain();
}


//*******************************************************************
//
// void Chain::smooth3()
//
//    SMOOTHES mChain using an interval of 3.  A temporary chain copy
//    is created and is padded on leading end [0,1] with mChain[1] (the first
//    meaningful angle) and on the traling end [mNumPoints+1] 
//    with mChain[mNumpoints-1] (the last meaningful angle.  Each angle in
//    the NEW mChain is the average (mean) of 3 values centered on the
//    same location as the value it replaced.
//
void Chain::smooth3()
{
    if (mNumPoints == 0 || mChain == NULL || mRelativeChain == NULL)
	    return;

    int chI,
     sI;

    double *tempChain;

    tempChain = new double[mNumPoints + 2];

    memcpy(&tempChain[1], mChain, mNumPoints * sizeof(double));

    tempChain[0] = mChain[1]; //005CH
    tempChain[1] = mChain[1]; //005CH
    tempChain[mNumPoints + 1] = mChain[mNumPoints - 1];
    chI = 1;

    double *smoothChain = new double[mNumPoints];

    while (chI < mNumPoints + 1) {
	sI = chI - 1;
	smoothChain[sI] = tempChain[chI - 1]
	    + tempChain[chI] + tempChain[chI + 1];
	smoothChain[sI] /= 3.0;
	chI++;
    }

    delete[]tempChain;
    delete[] mChain;
    mChain = smoothChain;

    Chain::updateRelativeChain();
}


//*******************************************************************
//
// void Chain::smooth15()
//
//    SMOOTHES mChain using an interval of 15.  A temporary chain copy
//    is created and is padded on leading end [0,7] with mChain[1] (the first
//    meaningful angle) and on the traling end [mNumPoints+7,mNumPoints+13] 
//    with mChain[mNumpoints-1] (the last meaningful angle.  Each angle in
//    the NEW mChain is the average (mean) of 15 values centered on the
//    same location as the value it replaced.
//
void Chain::smooth15()
{
    if (mNumPoints == 0 || mChain == NULL || mRelativeChain == NULL)
	    return;

    int
	chI,
    	sI;

    double *tempChain = new double[mNumPoints + 14];

    memcpy(&tempChain[7], mChain, mNumPoints * sizeof(double));

    int i;
    for (i = 0; i < 8; i++)     //005CH
	    tempChain[i] = mChain[1]; //005CH

    for (i = mNumPoints + 7; i < mNumPoints + 14; i++)
	    tempChain[i] = mChain[mNumPoints - 1];

    chI = 7;

    double *smoothChain = new double[mNumPoints];

    while (chI < mNumPoints + 7) { //005CH
	sI = chI - 7;
	smoothChain[sI] = (tempChain[chI - 7]
			     + tempChain[chI - 6]
			     + tempChain[chI - 5]
			     + tempChain[chI - 4]
			     + tempChain[chI - 3]
			     + tempChain[chI - 2]
			     + tempChain[chI - 1]
			     + tempChain[chI]
			     + tempChain[chI + 1]
			     + tempChain[chI + 2]
			     + tempChain[chI + 3]
			     + tempChain[chI + 4]
			     + tempChain[chI + 5]
			     + tempChain[chI + 6]
			     + tempChain[chI + 7])
	    / 15.0;
	chI++;
    }
    delete[] tempChain;
    delete[] mChain;
    mChain = smoothChain;
    Chain::updateRelativeChain();
}


//*******************************************************************
//
// void Chain::print() const
//
//    Prints a list of chain values in the CONSOLE.
//
#ifdef DEBUG
void Chain::print() const
{
	for (int i = 0; i < mNumPoints; i++)
		cout << mChain[i] << endl;
}
#endif


//*******************************************************************
//
// void Chain::updateRelativeChain()
//
//    Creates (or changes) the mRelativeChain angles to be consistent with
//    the absolute angles in mChain.
//
//    PRE:  mChain and mRelativeChain arrays exists and the memory allocated 
//          for both is the same  
//
void Chain::updateRelativeChain()
{
	if (0 == mNumPoints || NULL == mChain || NULL == mRelativeChain)
		return;

	mRelativeChain[0] = mChain[0];

	double temp;

	for (int i = 1; i < mNumPoints; i++) {

		/*
		////////////////// begin new ///////////////////////
		//***1.1ER - new update for relative chain
		// this new code more efficiently converts delta angles outside the
		// range -180 .. +180 back into that range
		//
		//double change = mChain[i-1] - mChain[i]; // this is consistent with constructor
		// it seems that the subtraction below is correct rather than the one 
		// above -- BUT this is reversed from what is done in the
		// constructor -- shouldn't they be consistent?? -- JHS
		double change = mChain[i] - mChain[i-1]; // BUT this seems to work correctly
		if (change > 180)
			change = change - 360;
		else if (change < -180)
			change = 360 - change;
		mRelativeChain[i] = change;
		///////////////// end new ////////////////////////
		*/
		//////////////////// begin old ////////////////////
		//***1.1ER
		// this is the original code - it needs to be replaced with what is above
		// in the next version - all part of fixing the problem that pulls
		// snake out of deep upturning notches and then finds the lip of the
		// notch as the hole
		temp = dtor(mChain[i] - mChain[i - 1]);

		//***008OL NOTE: the following loop corrects angles of greater than
		// 360 degrees, BUT how is it possible for the relative angle to
		// ever be more than 360 degrees?  Would this not automatically
		// indicate an error in computation somewhere?

		while (fabs(temp / PI) > 1)
			temp -= (fabs(temp) / temp) * PI;

		temp = rtod(temp);

		if (temp < CLOSE_TO_ZERO && temp > -CLOSE_TO_ZERO)
			mRelativeChain[i] = 0.0;
		else
			mRelativeChain[i] = temp;
		/////////////////////////// end old ///////////////
	}
}
