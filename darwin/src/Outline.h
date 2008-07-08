//*******************************************************************
//   file: Outline.h
//
// author: J H Stewman & K R Debure
//
//   mods:
//
// A class which encompasses a float contour, a chain and feature points
//
//*******************************************************************

#ifndef OUTLINE_H
#define OUTLINE_H

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <vector>
#include <iostream> 
#include "Contour.h" 
#include "FloatContour.h" 
#include "Chain.h" 

// Feature Point Types for get, set and findFeaturePoint functions
#define NO_FEATURE          -1
#define LE_BEGIN             1
#define LE_END               2
#define TIP                  3
#define NOTCH                4
#define POINT_OF_INFLECTION  5

class Outline
{
	public:
		Outline(Contour* c, double radius);
		Outline(FloatContour* fc);
    Outline(Outline* outline);
		~Outline();

	void mapOutlineTo (Outline* target);

	double getLEAngle() const;

	FloatContour* getFloatContour() const; 
    FloatContour* getRemappedFloatContour() const;
    Chain* getChain() const;

    bool userSetFeaturePoint(int type);
	int getFeaturePoint(int type) const;
    point_t& getFeaturePointCoords (int type) const;

    int findClosestFeaturePoint (point_t p) const;

	void setFeaturePoint(int type, int index);
    void setFloatContour(FloatContour* fc);
	void setLEAngle(double theta, bool compute);

    //***008OL next two functions moved from Chain class
    point_t& Outline::getSavedPoint(int pointNum);
    const point_t& Outline::getSavedPoint(int pointNum) const;

    int length() { return mChain->length(); } //***008OL -- use chain length ?


	private:
		FloatContour* mChainPoints;
		Chain* mChain;

    //int mNumPoints; //***008OL remove & use chain length ?

		FloatContour* mRemappedChainPoints;

    int    // indices into chain or fl contour 
		mTipPos, 
		mNotchPos,
		mBeginLE,
		mEndLE,
		mEndTE;
    		
    double mLEAngle; // I think we want this here - JHS

    
    bool 
		mUserSetTip,
		mUserSetNotch,
		mUserSetBeginLE,
		mUserSetEndLE,
		mUserSetEndTE;
                     
		int findTip();
		int findNotch();
		int findBeginLE();
		int findEndLE();
		int findPointOfInflection();
		double findLEAngle() const;


};

//***008OL following two functions moved here from Chain class
inline point_t& Outline::getSavedPoint(int pointNum)
{
	//if (pointNum < 0 || pointNum >= mNumPoints) 008OL removed
	if (pointNum < 0 || pointNum >= mChain->length()) //***008OL
		throw BoundsError("Chain::getSavedPoint");

  return (*mChainPoints)[pointNum];
}

inline const point_t& Outline::getSavedPoint(int pointNum) const
{
	//if (pointNum < 0 || pointNum >= mNumPoints) 008OL removed
	if (pointNum < 0 || pointNum >= mChain->length()) //***008OL
		throw BoundsError("Chain::getSavedPoint");

  return (*mChainPoints)[pointNum];
}

#endif
