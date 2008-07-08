/* ******************************************************************
//   file: hysteresis.h
//
// author: Mike Heath?
//
//   mods:
//
// This code was re-written by Mike Heath from original code obtained 
// indirectly from Michigan State University. heath@csee.usf.edu 
// (Re-written in 1996).
*/

#ifndef HYSTERESIS_H
#define HYSTERESIS_H

#include "GrayImage.h"

GrayImage* applyHysteresis(
		short int *mag,
		GrayImage *nms,
		int rows,
		int cols,
		float tlow,
		float thigh
);

GrayImage* nonMaxSupp(
		short *mag,
		short *gradx,
		short *grady,
		int nrows,
		int ncols
);

#endif
