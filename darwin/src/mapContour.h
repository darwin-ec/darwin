//*******************************************************************
//   file: mapContour.h
//
// author: ?
//
//   mods: 
//
//*******************************************************************

#ifndef MAPCONTOUR_H
#define MAPCONTOUR_H

#include "Chain.h"
#include "Contour.h"
#include "FloatContour.h"

FloatContour* mapContour(
		//Contour *c, // The contour we're mapping obviously - removed 008OL
		FloatContour *c, //***008OL
		// These are the three feature points on the
		// contour we're mapping
		point_t p1,
		point_t p2,
		point_t p3,
		
		// These are the three feature points on the
		// target contour
		point_t desP1,
		point_t desP2,
		point_t desP3
);

//***008OL remove this function for now - if needed use two Outlines
/*
FloatContour* autoMapContour(
		//Contour *c1, // The contour we're going to map - removed 008OL
		//Contour *c2  // The contour we're going to map it to - removed 008OL
    FloatContour *c1, //***008OL The contour we're going to map
    FloatContour *c2  //***008OL The contour we're going to map it to 
);
*/

#endif
