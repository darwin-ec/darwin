/*
 * IntensityContour.h
 * 
 * Author: Scott Hale
 * Date: 19-Aug-2005
 * 
 * Contrust a Contour based on the intensity image (GrayImage) of a color image.
 * 
 * Added in change 101AT
 */


#ifndef _INTENSITYCONTOUR_H_
#define _INTENSITYCONTOUR_H_

#include "Contour.h"
#include "image_processing/ColorImage.h"
#include "image_processing/ColorImage.h"
#include "image_processing/BinaryImage.h"
#include "image_processing/GrayImage.h"
#include "image_processing/Histogram.h"
#include "image_processing/conversions.h"

class IntensityContour : public Contour
{
public:
	IntensityContour();
	
	//Get an intensity contour from image
	IntensityContour(ColorImage* img, Contour* ctour,
	                 int left, int top, int right, int bottom);//Add param ctour, 103AT SAH
	IntensityContour(GrayImage* img, Contour* ctour,
	                 int left, int top, int right, int bottom);//Add param ctour, 103AT SAH
	
	IntensityContour(IntensityContour* contour) : Contour(contour) {}
	IntensityContour(IntensityContour &contour) : Contour(contour){}
	//virtual ~IntensityContour();
	
private:
	//void getPointsFromGrayImage(GrayImage* imgIn, Contour* ctour);//Add param ctour, 103AT SAH
	//GrayImage* boundImage(GrayImage* img, Contour* ctour, int factor, int&, int&);//103AT SAH

	void getPointsFromGrayImage(GrayImage* imgIn, Contour* ctour,
	                            int left, int top, int right, int bottom);//***1.96
	GrayImage* boundImage(GrayImage* img, Contour* ctour, 
	                      int left, int top, int right, int bottom,
	                      int factor, int&, int&);//***1.96
};


#endif //_INTENSITYCONTOUR_H_
