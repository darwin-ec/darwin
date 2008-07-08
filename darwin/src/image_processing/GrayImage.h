//*******************************************************************
//   file: GrayImage.h
//
// author:
//
//   mods:
//
//*******************************************************************

#ifndef GRAYIMAGE_HH
#define GRAYIMAGE_HH

#include "ImageFile.h"
#include "Pixel.h"
#include "Histogram.h"
#include "BinaryImage.h"

class GrayImage : public ImageFile<GrayPixel>
{
public:
	GrayImage(unsigned nRows, unsigned nCols);
	GrayImage(const std::string &filename);
	GrayImage(unsigned nRows, unsigned nCols, GrayPixel *data);
	GrayImage(unsigned nRows, unsigned nCols, ColorPixel *data);
	GrayImage(const GrayImage *image);
	
	GrayImage& operator=(const Image<GrayPixel>& i);
	virtual unsigned getColorDepth() const;
	bool isColor() const;
	
	//Scott: Intensity outline of fin (Aug/2005) Code: 101AT
	Histogram<GrayImage>* getHistogram();//101AT
	BinaryImage* doThreshold(Range* rng);//101AT
	
};

inline
GrayImage::GrayImage(const std::string &filename)
	: ImageFile<GrayPixel>(filename)
{ }

inline
GrayImage::GrayImage(unsigned nRows, unsigned nCols)
	: ImageFile<GrayPixel>(nRows, nCols)
{ }

inline
GrayImage::GrayImage(unsigned nRows, unsigned nCols, GrayPixel *data)
	: ImageFile<GrayPixel>(nRows, nCols, data)
{ }

inline
GrayImage::GrayImage(const GrayImage *image)
{
	mRows = image->getNumRows();
	mCols = image->getNumCols();

	mData = new GrayPixel[mRows * mCols];

	memcpy(mData, image->getData(), mRows * mCols * sizeof(GrayPixel));

	mInitialized = true;
}

inline
GrayImage& GrayImage::operator=(const Image<GrayPixel>& i)
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
		
				mData = new GrayPixel[mRows * mCols];
		}

		memcpy (mData, &i(0,0), sizeof(GrayPixel) * mRows * mCols);
		
		mInitialized = true;
	}
	return *this;
}

inline unsigned GrayImage::getColorDepth() const
{
	return 256; //ACK!
}

inline
bool GrayImage::isColor() const
{
	return false;
}
#endif
