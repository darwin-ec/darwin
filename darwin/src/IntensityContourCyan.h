/*
 * IntensityContour.h
 * 
 * Author: Scott Hale
 * Date: 22-Aug-2006
 * 
 * Contrust a Contour based on the cyan channel of a color image.
 * 
 * Added in change 102AT
 */


#ifndef _INTENSITYCONTOURCYAN_H_
#define _INTENSITYCONTOURCYAN_H_

#include "Contour.h"
#include "image_processing/ColorImage.h"
#include "image_processing/ColorImage.h"
#include "image_processing/BinaryImage.h"
#include "image_processing/GrayImage.h"
#include "image_processing/Histogram.h"
#include "image_processing/conversions.h"

class IntensityContourCyan : public Contour
{
public:
	IntensityContourCyan();
	
	//Get an intensity contour from image
	IntensityContourCyan(ColorImage* img, Contour* ctour,
	                     int left, int top, int right, int bottom); //***1.96 - added bounds
	//IntensityContourCyan(GrayImage* img);
	
	IntensityContourCyan(IntensityContourCyan* contour) : Contour(contour) {}
	IntensityContourCyan(IntensityContourCyan &contour) : Contour(contour){}
	//virtual ~IntensityContour();
	
private:
	void getPointsFromCyanImage(GrayImage* imgIn, Contour* ctour,
	                            int left, int top, int right, int bottom); //***1.96 - added bounds
	GrayImage* boundImage(GrayImage* img, Contour* ctour, 
	                      int left, int top, int right, int bottom, //***1.96 - added bounds
	                      int factor, int&, int&);
};


#endif //_INTENSITYCONTOURCYAN_H_
