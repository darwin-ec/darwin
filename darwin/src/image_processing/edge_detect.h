//*******************************************************************
//   file: edge_detect.h
//
// author: Adam Russell
//
//   mods: J H Stewman (7/25/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#ifndef EDGE_DETECT_H
#define EDGE_DETECT_H

#include "GrayImage.h"

// Canny edge detection implementation

GrayImage* edgeDetect_canny(
		const GrayImage *image,	// The image to be edge detected
		
		GrayImage **magImage,	// The edge
					// magnitude image will be saved
					// here
		bool saveMagImage,	// If this is set to true
		
		float sigma,		// The standard deviation of
					// the gaussian smoothing filter.

		float tlow,		// Specifies the low value to use
					// in hysteresis. This is a fraction
					// (0-1) of the computed high
					// threshold edge strength value.
	
		float thigh,		// Specifies the high value to use
					// in hysteresis. This fraction (0-1)
					// specifies the percentage point
					// in a histogram of the gradient of
					// the magnitude. Magnitude values
					// of zero are not counted in the
					// histogram.

		char *fname
);

// Sobel edge detection implementation
GrayImage* edgeDetect_sobel(
		const GrayImage *srcImage,	// The image to be edge detected
		
		GrayImage **directionImage,	// Direction result image, if
						// the caller would like to keep
						// it around
		bool saveDirImage
);

inline
GrayImage* edgeDetect_canny(
		const GrayImage *image,
		float sigma,
		float tlow,
		float thigh
)
{
	return edgeDetect_canny(image, NULL, false, sigma, tlow, thigh, NULL);
}

inline
GrayImage* edgeDetect_canny(
		const GrayImage *image,
		GrayImage **magImage,
		float sigma,
		float tlow,
		float thigh
		)
{
	return edgeDetect_canny(image, magImage, true, sigma, tlow, thigh, NULL);
}

inline
GrayImage* edgeDetect_sobel(const GrayImage *srcImage)
{
	return edgeDetect_sobel(srcImage, NULL, false);
}

#endif
