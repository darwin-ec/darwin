// feature.h

#ifndef FEATURE_H
#define FEATURE_H

#include "Chain.h"

// Attempts to determine the angle of the fin's leading edge.
// 
// Parameters:
//
// 	int tipPosition - the index into the chain array that represents
// 	the fin's tip.
double findLEAngle(const Chain *chain, int tipPosition);

// Finds the tip as the index into the chain
int findTip(const Chain *chain);

// Finds the most prominant notch on the trailing edge as an index into
// the chain
int findNotch(const Chain *chain, int tipPosition);

//int findPointOfInflection(const point_t *p, int tipPosition, int chainEnd); 
int findPointOfInflection(const Chain *chain, int tipPosition);

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
int findLECutoff(const Chain *chain, int tipPosition);

int findLEEnd(const Chain *chain, int leBegin, int tipPosition);


#endif
