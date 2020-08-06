//*******************************************************************
//   file: snake.cxx
//
// author: Mark Allen
//
//   mods: 
//
// Much of this code is from Active Contour code from USF and other 
// sources.
//
//*******************************************************************
 
#include "snake.h"
#include "image_processing/Matrix.h"
#include <fstream>
#include <math.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef DEBUG
#include <iostream>
#endif

using namespace std;

const float BIG = (float) 1.0e20;   // largest representable number by a float *cough*

float energyCalc(
	const GrayImage* edgeImage,

	const float energyWeights[3],
	int xprevpt,
	int yprevpt,
	int xcurrpt,
	int ycurrpt,
	int xnextpt, 
	int ynextpt,
	float averageDistance
)
{
    if (NULL == edgeImage)
	return BIG;

    float
    	energyCont,
	energyLin,
	energyEdgeStrength,
	energyContDiff;

    int numRows = edgeImage->getNumRows();
    int numCols = edgeImage->getNumCols();

    //  First thing to do is to check image boundaries.  To keep
    //  a point from moving off the image, any neighborhood point
    //  out-of-bounds gets its energy set very large.  Since the
    //  whole idea is to minimize a point's energy, the point
    //  will never take the out-of-bounds route.

    if (xcurrpt < 0 || xcurrpt >= numCols || ycurrpt < 0 || ycurrpt >= numRows)
	    return BIG;	// no moving past image boundaries

	// energyCont => continuity; how evenly spaced the points on the
	// contour are.  The distance between the point and the point  
	// behind it is compared to averageDistance.  The closer, the lower the   
	// energy.

	// Determines distance from current to previous point, then squares it

	// Calculates distance between the point and the point after it, compare to averageDistance
	energyContDiff = averageDistance - (float) sqrt((double)
			    ((xcurrpt-xprevpt)
			     *(xcurrpt-xprevpt)
			     +(ycurrpt-yprevpt)
			     *(ycurrpt-yprevpt)));

	// Determines energy continuity to be a sum of the distance from the current to the
	// previous point with the deviation of the distance from the average point spacing 
	// multiplied by some scaling factor

	energyCont = energyContDiff * energyContDiff;	// squares the Average distance difference

	// energyLin => linearality; how well the point, plus the points on
	// either side, make a straight line.  The straighter, the lower  
	// the energy.
	energyLin = (float)(xprevpt - 2 * xcurrpt + xnextpt)
		* (float)(xprevpt - 2 * xcurrpt + xnextpt)
		+ (float)(yprevpt - 2 * ycurrpt + ynextpt) 
		* (float) (yprevpt - 2 * ycurrpt + ynextpt);


	energyEdgeStrength = -(float) (*edgeImage)(ycurrpt, xcurrpt).getIntensity();
	
	// Now let's add up those weighted energies
	return energyWeights[0] * energyCont
		+ energyWeights[1] * energyLin
		+ energyWeights[2] * energyEdgeStrength;
}

bool moveContour(
		Contour *contour,	// The contour we'd like to move
		
		const GrayImage *edgeImage,	// Edge strength image used for energy calculations
		
		int neighborhoodSize,	// Neighborhood window size
						// neighborhoodSize x neighborhoodSize
		
		const float energyWeights[3]	// Energy Weights (!)
)
{
	if (NULL == contour || NULL == edgeImage || neighborhoodSize <= 0)
		return false;

	int	
		minPosition = 0,	// keeps track of the value of k (the current node)
					// corresponding to the minimum energy found so far
					// in the search using k over all neighbors of the
					// current node
					// It's initialized to zero because there's a (very)
					// slight chance that it could be used unitialized
					// otherwise
 
		numNeighbors;
	
	float
		minEnergy,	// keeps track of the minimum energy found so far
				// in the search using k over all neighbors of the
				// current node. 
	
		energy;		// is used in the search over the neighbors of the
				// current node as a temporary storage of the energy computed
		
	Contour_point_t
		nextNode,	// contains the coordinates, for the neighbor of the
				// next node indexed by j. These coordinates are used
				// only to compute the energy associated with the
				// associated choice of snaxel positions
	
		currNode,	// contains the coordinates, for the neighbor of the
				// current node indexed by k. These coordinates are
				// used only to compute the energy associated with the
				// associated choice of snaxel positions
		*neighbor;
	
	numNeighbors = neighborhoodSize * neighborhoodSize;

	int numPoints = contour->length();

	if (numPoints <= 2)
		return false;

	Matrix<float> energyMtx(numPoints - 1, numNeighbors);
	
	Matrix<int> posMtx(numPoints - 1, numNeighbors);

	// memset would be better, but alkfjsdjldsfajlksfdal
	neighbor = new Contour_point_t[numNeighbors];

	for (int a = 0; a < numNeighbors; a++) {
	    neighbor[a].x = (a % neighborhoodSize) - (neighborhoodSize - 1) / 2;
	    neighbor[a].y = (a / neighborhoodSize) - (neighborhoodSize - 1) / 2;
	}

	// initialize first column of energy matrix
	// This block of code simply sets the first column of the energy
	// matrix to zero, since there is no energy associated solely
	// with the first snaxel; energy is based on distance, and there
	// is no distance associated with a single snaxel. It is only when
	// we get to the second snaxel, that we have some measure of distance
	for (int l = 0; l < numNeighbors; l++) {
		energyMtx(0, l) = 0.0;
		posMtx(0, l) = 0;
	}

	// Find the average distance between points
	float distSum = 0.0;

	for (int d = 1; d < numPoints; d++)
		distSum += sqrt((double)
				((*contour)[d].x - (*contour)[d - 1].x)
				* ((*contour)[d].x - (*contour)[d - 1].x)
				+ (double)((*contour)[d].y - (*contour)[d - 1].y)
				* ((*contour)[d].y - (*contour)[d - 1].y));

	float averageDistance = distSum	/ numPoints;

	for (int i = 1; i < numPoints - 1; i++) {
		for (int j = 0; j < numNeighbors; j++) { // for all neighbors of next node
			minEnergy = BIG;
			
			nextNode.x = (*contour)[i + 1].x + neighbor[j].x;
			nextNode.y = (*contour)[i + 1].y + neighbor[j].y;
	
			for (int k = 0; k < numNeighbors; k++) { // for all neighors of curr node
				currNode.x = (*contour)[i].x + neighbor[k].x;
				currNode.y = (*contour)[i].y + neighbor[k].y;

				energy = energyMtx(i - 1, k) +
					 energyCalc(
						edgeImage,
						energyWeights,
						(*contour)[i - 1].x + neighbor[posMtx(i - 1, k)].x,
						(*contour)[i - 1].y + neighbor[posMtx(i - 1, k)].y,
						currNode.x, currNode.y,
						nextNode.x, nextNode.y,
						averageDistance	
					);
				if (energy < minEnergy) {
					minEnergy = energy;
					minPosition = k;
				}
			}
			
			// Store minimum energy into table matrix
			energyMtx(i, j) = minEnergy;
			posMtx(i, j) = minPosition;
		}
	}

	int pos = posMtx(numPoints - 2, 4);

	// search backwards through table to find optimum positions
	for (int k = numPoints - 2; k > 0; k--) {
		(*contour)[k].x = (*contour)[k].x + neighbor[pos].x;
		(*contour)[k].y = (*contour)[k].y + neighbor[pos].y;

		pos = posMtx(k - 1, pos);
	}

	delete[] neighbor;

	return true;
}
