//*******************************************************************
//   file: snake.h
//
// author: Mark C. Allen   10/23/95
//
//   mods:
//         -- modified for X-Win by Dan Wilkin
//
//         -- modified further by Adam, of course...
//
// Header file for newsnake.c;  is intended to provide function prototypes, 
// type defs. for the new snake algorithm.
// 
//*******************************************************************


#ifndef SNAKE_H
#define SNAKE_H

#include "Contour.h"
#include "image_processing/GrayImage.h"

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
);

bool moveContour(
		Contour *contour,	// The contour we'd like to move
		
		const GrayImage *edgeImage,	// Edge strength image used for energy calculations
		
		int neighborhoodSize,	// Neighborhood window size
					// neighborhoodSize x neighborhoodSize
		
		const float energyWeights[3]	// Energy Weights (!)
);

inline bool moveContour(
		Contour *contour,
		const GrayImage *edgeImage,
		const float energyWeights[3]
)
{
	return moveContour(contour, edgeImage, 3, energyWeights);
}

#endif
