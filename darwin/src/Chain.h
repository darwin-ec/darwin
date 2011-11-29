//*******************************************************************
//   file: Chain.h
//
// author: Adam Russell
//
//
//   mods: J H Stewman (7/22/2005)
//         -- reformatting of code and addition of comment blocks
//         -- changes to incorporate Outline CLASS in project
//
//*******************************************************************


#ifndef CHAIN_H
#define CHAIN_H

#include "Point.h" //***006FC point_t definition
#include "Contour.h"
#include "FloatContour.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


class Chain
{
	public:
		Chain(FloatContour *contour);  //***006OL
		Chain(const Chain* c);         //***006OL
		Chain(const Chain &c);
		~Chain();

		Chain& operator=(const Chain &c);

		int length() const;

		double* getData();
		const double* getData() const;
		double* getRelativeData();
		const double* getRelativeData() const;
		double getRadius() const;	//***006DF

		// These return absolute angles
		double& operator[](int angleNum);
		const double& operator[](int angleNum) const;

		// These return relative angles
		double& operator()(int angleNum);
		const double& operator()(int angleNum) const;

		double& getAngle(int angleNum);
		const double& getAngle(int angleNum) const;

		double& getRelativeAngle(int angleNum);
		const double& getRelativeAngle(int angleNum) const;


		double min() const;
		double max() const;

		// These ranged functions start at index = start,
		// and end at index end - 1.
		double min(int start, int end) const;
		double max(int start, int end) const;

		double minRelative() const;
		double maxRelative() const;

		void smooth3();
		void smooth5();
		void smooth7();
		void smooth15();

#ifdef DEBUG
		void print() const;
#endif

	private:
		int mNumPoints;
		double
			*mChain,
			*mRelativeChain;

		// NOTE: point coordinates are maintained in the Outline class

		// After the absolute chain has been changed, call
		// this function to regenerate the relative chain
		void updateRelativeChain();
};


//*******************************************************************
//
// inline double& Chain::operator[](int angleNum)
//
//    Returns value of indicated absolute chain angle.
//
//    PRE:  0 < angleNum < mNumPoints
//
inline double& Chain::operator[](int angleNum)
{
	if (angleNum < 0 || angleNum >= mNumPoints)
		throw BoundsError("Chain::operator[]");

	return mChain[angleNum];
}


//*******************************************************************
//
// inline const double& Chain::operator[](int angleNum)
//
//    Returns CONST value of indicated absolute chain angle.
//
//    PRE:  0 < angleNum < mNumPoints
//
inline const double& Chain::operator[](int angleNum) const
{
	if (angleNum < 0 || angleNum >= mNumPoints)
		throw BoundsError("Chain::operator[]");

	return mChain[angleNum];
}


//*******************************************************************
//
// inline double& Chain::operator()(int angleNum)
//
//    Returns value of indicated relative chain angle.
//
//    PRE:  0 < angleNum < mNumPoints
//
inline double& Chain::operator()(int angleNum)
{
	if (angleNum < 0 || angleNum >= mNumPoints)
		throw BoundsError("Chain::operator()");

	return mRelativeChain[angleNum];
}


//*******************************************************************
//
// inline const double& Chain::operator()(int angleNum)
//
//    Returns value of indicated relative chain angle.
//
//    PRE:  0 < angleNum < mNumPoints
//
inline const double& Chain::operator()(int angleNum) const
{
	if (angleNum < 0 || angleNum >= mNumPoints)
		throw BoundsError("Chain::operator()");

	return mRelativeChain[angleNum];
}


#endif

/***008OL numerous removals due to incorporation of Outline CLASS
 *
 * Chain(Contour *contour, double radius); removed 008OL
 * Chain(FloatContour *contour, double radius); removed 008OL
 * void computeChainPoints(point_t *p, int pLength, double radius); //0005CH removed 008OL
 * point_t& getSavedPoint(int pointNum); removed 008OL
 * const point_t& getSavedPoint(int pointNum) const; removed 0089OL
 * 
 * double mRadius; 008OL removed
 * point_t *mSavedPoints; replaced by following
 * FloatContour *mChainPoints;  //***006DF removed - 008OL JHS
 * FloatContour* getChainPoints(); 008OL removed JHS
 * const FloatContour* getChainPoints() const; 008OL removed JHS
 * 
 */
