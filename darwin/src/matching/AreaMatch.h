//*******************************************************************
//   file: AreaMatch.h
//
// author: J H Stewman 2007 
//         -- addition to DARWIN version 1.85
//
// computes the error between two fin outlines by finding the
// area between the interlaced contours after mapping
//
//*******************************************************************

#ifndef AREA_MATCH_H
#define AREA_MATCH_H

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <vector>
#include <set>
#include <map>
#include <cmath>
#include "../FloatContour.h"

// new classes for area-based error calculation

class contour_segment {
	public:
		double parameter;
		char contourType; // 'd' or 'u' for database or unknown
		int startIndex; 
		bool reversed; // true means end index is startIndex - 1 rather than startIndex + 1
		std::set<int> intersectingSegs; // initially empty
		                           // contains indices of intersecting segments on other contour

	contour_segment()
		:parameter(0.0), contourType('d'), startIndex(0), reversed(false)
	{
		intersectingSegs.clear();
	}

	~contour_segment()
	{ 
		intersectingSegs.clear();
	}
};

class ref_point {
	public:
		double parameter;
		int segId; // index of segment
		int segOp; // operation (0 means add, 1 means remove from active segment list)

		ref_point()
		{}

		~ref_point()
		{}
};

class int_point {
	public:
		double paramDb, paramUnk;
		point_t pt;

		int_point () 
		{}
};

double findAreaBetweenOutlineSegments( // called for either leading or training edge
		FloatContour *c1, // mapped unknown fin 
		int begin1,
		int end1,
		FloatContour *c2, // evenly spaced database fin //***0005CM
		int begin2,
		int end2);

double areaBasedErrorBetweenOutlineSegments_NEW( // called for entire fin .. scales result
		FloatContour *c1, // mapped unknown fin 
		int begin1,
		int mid1,
		int end1,
		FloatContour *c2, // evenly spaced database fin //***0005CM
		int begin2,
		int mid2,
		int end2);

double polygonArea(const std::vector<point_t> &p);

void intersect(std::vector<contour_segment> &segments, 
			   FloatContour *c1, // unknown contour
			   FloatContour *c2, // database contour
			   int dbId, int unkId, 
			   double &s1, double &t1, point_t &p1,  // db param, point coords & unk param
			   double &s2, point_t &p2,       // db param, point coords
			   int &n);

void findSegsAndRefPointsForContour (		
		FloatContour *c1, // part of some contour (DB or UNK)
		int begin1,
		int end1,
		char conType, // 'd' or 'u'
		std::vector<contour_segment> &segments,
		std::multimap<double, ref_point> &refPts
		);

double areaBasedErrorBetweenOutlineSegments( 
		FloatContour *c1, // mapped unknown fin 
		int begin1,
		int end1,
		FloatContour *c2, // envenly spaced database fin //***0005CM
		int begin2,
		int end2);

#endif // AREA_MATCH_H
