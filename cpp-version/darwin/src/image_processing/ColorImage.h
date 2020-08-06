//*******************************************************************
//   file: ColorImage.h
//
// author: Adam Russell
//
//   mods: J H Stewman (7/22/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#ifndef COLORIMAGE_H
#define COLORIMAGE_H

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <map>

#include "GrayImage.h"
#include "ImageFile.h"
#include "Pixel.h"
#include "Error.h"

class ColorImage : public ImageFile<ColorPixel>
{
	public:
		ColorImage(unsigned nRows, unsigned nCols);
		ColorImage(const std::string &filename);
		ColorImage(const ColorImage *image);
		ColorImage(unsigned nRows, unsigned nCols, ColorPixel *data);
		ColorImage(unsigned nRows, unsigned nCols, GrayPixel *data);

		//~ColorImage();

		ColorImage& operator=(const Image<ColorPixel>& i);
		// friend ColorImage operator/(const ColorImage& numerator, const ColorImage& denominator);

		virtual unsigned getColorDepth() const;
		bool isColor() const;

		// These functions return an image representing one
		// entire color from this ColorImage.
		GrayImage* redImage() const;
		GrayImage* greenImage() const;
		GrayImage* blueImage() const;

		void findMinMaxRed(unsigned& min, unsigned& max) const;
		void findMinMaxGreen(unsigned& min, unsigned& max) const;
		void findMinMaxBlue(unsigned& min, unsigned& max) const;

		void rescale(float *redTemp, float *greenTemp, float *blueTemp,
		unsigned redMin, unsigned redMax,
		unsigned greenMin, unsigned greenMax,
		unsigned blueMin, unsigned blueMax);

		void createDefaultImageFromXPM(char **xpm);
};


//*******************************************************************
//
// inline ColorImage::ColorImage(unsigned nRows, unsigned nCols)
//
//    CONSTRUCTOR - Only effect is to allocate memory for the underlying
//    Matrix class object.
//
inline ColorImage::ColorImage(unsigned nRows, unsigned nCols)
	: ImageFile<ColorPixel>(nRows, nCols)
{ }


//*******************************************************************
//
// inline ColorImage::ColorImage(unsigned nRows, unsigned nCols, ColorPixel *data)
//
//    CONSTRUCTOR - Create ColorImage using passed ColorPixel data.  NO new
//    memory is allocated.  The data passed into this function becomes the
//    data of the created ColorImage.
//
inline ColorImage::ColorImage(unsigned nRows, unsigned nCols, ColorPixel *data)
	: ImageFile<ColorPixel>(nRows, nCols, data)
{ }


//*******************************************************************
//
// inline ColorImage::ColorImage(const ColorImage *image)
//
//    COPY CONSTRUCTOR - Creates complete copy of "pointed to" image.
//
inline ColorImage::ColorImage(const ColorImage *image)
	: ImageFile<ColorPixel>() //***1.0LK
{
	mRows = image->getNumRows();
	mCols = image->getNumCols();

	mData = new ColorPixel[mRows * mCols];

	memcpy(mData, image->getData(), mRows * mCols * sizeof(ColorPixel));

	mNormScale = image->mNormScale; //***1.5

	mImageMods = image->mImageMods; //***1.8
	mOriginalImageFilename = image->mOriginalImageFilename; //***1.8

	mInitialized = true;
}


//*******************************************************************
//
// inline unsigned ColorImage::getColorDepth() const
//
//    Returns 256 as depth of image.
//
//    NOTE: This will need changing at some future time in order to support
//    additional image formats.
//
inline unsigned ColorImage::getColorDepth() const
{
	// Yup. This is bad.
	return 256;
}


//*******************************************************************
//
// inline bool ColorImage::isColor() const
//
//    Returns true to indicate the image IS COLOR.
//
inline bool ColorImage::isColor() const
{
	return true;
}


//*******************************************************************
//
// inline ColorImage& ColorImage::operator=(const Image<ColorPixel>& i)
//
//    ASSIGNMENT operator
//
inline ColorImage& ColorImage::operator=(const Image<ColorPixel>& i)
{
	// Don't really need to test for self assignment, I don't think.
	if (!i.isInitialized()) {
		mInitialized = false;
		mRows = mCols = 0;
		mData = NULL;
	} else {
		if (!mInitialized || mRows != i.getNumRows() || mCols != i.getNumCols()) {
				mRows = i.getNumRows ();
				mCols = i.getNumCols ();
				
				if (mInitialized)
					delete[] mData;
		
				mData = new ColorPixel[mRows * mCols];
		}

		memcpy (mData, &i(0,0), sizeof(ColorPixel) * mRows * mCols);
		
		mInitialized = true;

		mNormScale = 1.0f; //***1.5
	}
	return *this;
}

#endif
