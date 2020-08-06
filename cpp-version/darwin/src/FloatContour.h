//*******************************************************************
//   file: FloatContour.h
//
// author: Adam Russell
//
//   mods: J H Stewman 
//         ***06FC major mods to FloatContour
//
// Complete FloatContour class, revised beginning 3/12/2004 to replace 
// most uses of Contour.  FloatContours will now always be EVENLY spaced.  
// -- JHS
//
//*******************************************************************

#ifndef FLOATCONTOUR_H
#define FLOATCONTOUR_H

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <vector>
#include <iostream> //***005DB for debugging
#include "Contour.h" //***005DB
#include "Point.h" // ***06FC for point_t type

/***06FC
this type is being replaced with point_t declared in Chain.h
typedef struct {
	float x, y;
} floatContourPointType;
*/

class FloatContour
{
	public:
		FloatContour(); //***006FC moved implementation
		FloatContour(const FloatContour& fc); //***006FC new
		~FloatContour();

		FloatContour& operator=(FloatContour& fc); //***006FC new

		int length() const;

		void popFront(unsigned numPops); //***005DB
		FloatContour* evenlySpaceContourPoints(int space); //***005DB
		//void normalizeContour(); //***06FC removed FloatContour::

		//Find the X,Y Location Closest to desx, desy
		int FindIndexFromChainPoint(float desx, float desy); //***005DB

		//Convert Contour *c into *this* FloatContour
		void ContourToFloatContour(Contour *c);	//***005DB
		Contour* FloatContourToContour(FloatContour *fc); //***005DB

		// Returns position of Contour point closest to (x,y)
		bool findPositionOfClosestPoint(float x, float y, int &position) const;

		void addPoint(float x, float y);

		point_t& operator[](int pos); //***06FC return type now point_t
		const point_t& operator[](int pos) const; //***06FC return type now point_t

		float minX();
		float minY();
		float maxX();
		float maxY();

    //***005DB new function
  	void print(); //***006FC moved implementation

	private:
		std::vector<point_t> mPointVector; //***006FC changed vector type

};

#endif
