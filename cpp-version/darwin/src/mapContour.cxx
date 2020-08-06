//*******************************************************************
//   file: mapContour.cxx
//
// author: 
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and addition of comment blocks
// 
//
//*******************************************************************

#include "mapContour.h"
#include "feature.h"
#include "math/gaussj.h"
#include "utility.h"

using namespace std;

//*******************************************************************
//
//
FloatContour* mapContour(
		//Contour *c, removed 008OL
		FloatContour *c, //***008OL
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

	//FloatContour *dstContour = NULL; removed 008OL
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

		FloatContour *dstContour = new FloatContour(); //***008OL
		int numPoints = c->length();
		//int cx, cy; removed 008OL
		float cx,cy; //***008OL
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
		
		//delete dstContour; removed 008OL

		throw;
	}
}

//***008OL remove this function -- if needed, rewrite using two Outlines
/*

//*******************************************************************
//
//
FloatContour* autoMapContour(
		//Contour *c1, // The contour we're going to map - removed 008OL
		//Contour *c2  // The contour we're going to map it to - removed 008OL
		FloatContour *c1, //***008OL The contour we're going to map
		FloatContour *c2  //***008OL The contour we're going to map it to
)
{
	Chain
		*chain1 = new Chain(c1, 3.0),
		*chain2 = new Chain(c2, 3.0);

	int c1TipPosition, c1BeginLE, c1NotchPosition;
	c1TipPosition = findTip(chain1);
	c1BeginLE = findLECutoff(chain1, c1TipPosition);
	c1NotchPosition = findNotch(chain1, c1TipPosition);

	point_t
		c1TipPositionPoint,
		c1BeginLEPoint,
		c1NotchPositionPoint;

	c1TipPositionPoint = chain1->getSavedPoint(c1TipPosition);
	c1BeginLEPoint = chain1->getSavedPoint(c1BeginLE);
	c1NotchPositionPoint = chain1->getSavedPoint(c1NotchPosition);

	int c2TipPosition, c2BeginLE, c2NotchPosition;
	c2TipPosition = findTip(chain2);
	c2BeginLE = findLECutoff(chain2, c2TipPosition);
	c2NotchPosition = findNotch(chain2, c2TipPosition);

	point_t
		c2TipPositionPoint,
		c2BeginLEPoint,
		c2NotchPositionPoint;
	
	c2TipPositionPoint = chain2->getSavedPoint(c2TipPosition);
	c2BeginLEPoint = chain2->getSavedPoint(c2BeginLE);
	c2NotchPositionPoint = chain2->getSavedPoint(c2NotchPosition);

	FloatContour *mappedContour = mapContour(
			c1,
			c1TipPositionPoint,
			c1BeginLEPoint,
			c1NotchPositionPoint,
			c2TipPositionPoint,
			c2BeginLEPoint,
			c2NotchPositionPoint);

	delete chain1;
	delete chain2;
	
	return mappedContour;
}
*/