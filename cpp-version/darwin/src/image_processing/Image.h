//*******************************************************************
//   file: Image.h
//
// author: Adam Russel?
//
//   mods:
//
//*******************************************************************

#ifndef IMAGE_H
#define IMAGE_H

#include "../Error.h"
#include "Matrix.h"
#include "Pixel.h"
#include "../utility.h"
#include <math.h>

class InvalidImage : public Error {
public:
	InvalidImage() : Error("Invalid image.")
	{ }

	InvalidImage(std::string s) : Error("Invalid image passed to: " + s)
	{ }
};

class ImageSizeMismatch : public Error {
public:
	ImageSizeMismatch() : Error("Image sizes do not match.")
	{ }

	ImageSizeMismatch(std::string s) : Error("Image sizes do not match in: " + s)
	{ }
};

template <class PIXEL_TYPE>
class Image : public Matrix<PIXEL_TYPE> {
public:
	Image(unsigned nrows, unsigned ncols);
	Image(unsigned nrows, unsigned ncols, PIXEL_TYPE* data);
	Image();

// Stupid MSVC++ - I don't understand why it thinks the syntax on these operators is wrong.
//	friend Image operator&<PIXEL_TYPE>(const Image& first, const Image& second);
//	friend Image operator/<PIXEL_TYPE>(const Image& numerator, const Image& denominator);
	virtual unsigned getColorDepth() const;

	PIXEL_TYPE* getData();
	const PIXEL_TYPE* getData() const;
	float findAngle() const;
	unsigned findXCentroid() const;
	unsigned findYCentroid() const;
	void findMaxExtents(unsigned& xMin, unsigned& xMax, unsigned& yMin, unsigned& yMax) const;
	void findMinMaxLevels(unsigned &min, unsigned &max) const;

	void rescale (float *imageTemp, int oldMin, int oldMax);

	size_t bytesPerPixel() const;

	// allow access to base class members without resolution
	using Matrix<PIXEL_TYPE>::mData;
	using Matrix<PIXEL_TYPE>::mRows;
	using Matrix<PIXEL_TYPE>::mCols;
	using Matrix<PIXEL_TYPE>::mInitialized;
};

template <class PIXEL_TYPE>
inline Image<PIXEL_TYPE>::Image(unsigned nrows, unsigned ncols)
	: Matrix<PIXEL_TYPE>(nrows, ncols)
{ }

template <class PIXEL_TYPE>
Image<PIXEL_TYPE>::Image(unsigned nrows, unsigned ncols, PIXEL_TYPE* data)
	: Matrix<PIXEL_TYPE>()
{
	mData = data;
	mRows = nrows;
	mCols = ncols;
	mInitialized = true;	
}

template <class PIXEL_TYPE>
inline Image<PIXEL_TYPE>::Image()
	: Matrix<PIXEL_TYPE>()
{ }

template <class PIXEL_TYPE>
inline PIXEL_TYPE* Image<PIXEL_TYPE>::getData()
{
	return mData;
}

template <class PIXEL_TYPE>
inline const PIXEL_TYPE* Image<PIXEL_TYPE>::getData() const
{
	return mData;
}

template <class PIXEL_TYPE>
Image<PIXEL_TYPE> operator&(const Image<PIXEL_TYPE>& first, const Image<PIXEL_TYPE>& second)
{
	if (first.mRows != second.mRows || first.mCols != second.mCols)
		throw ImageSizeMismatch("operator&");

	Image<PIXEL_TYPE> temp(first.mRows, first.mCols);

	unsigned numRows = first.mRows;
	unsigned numCols = first.mCols;
	
	for (unsigned r = 0; r < numRows; r++)
		for (unsigned c = 0; c < numCols; c++) {
			if (first.mData[r * numCols + c] > 0 && second.mData[r * numCols + c] > 0)
				temp.mData[r * numCols + c] = MAX_PIXEL_INTENSITY;
		}
	return temp;
}

template <class PIXEL_TYPE>
Image<PIXEL_TYPE> operator/(const Image<PIXEL_TYPE>& numerator, const Image<PIXEL_TYPE>& denominator)
{
	unsigned numRows = numerator.mRows;
	unsigned numCols = numerator.mCols;
	
	if (numRows != denominator.mRows || numCols != denominator.mCols)
		throw ImageSizeMismatch("operator/");
	
	float *imageTemp;

	//allocate enough room to store the image in a float array
	imageTemp = new float[numRows * numCols];
	
	for (unsigned r = 0; r < numRows; r++) {
		for (unsigned c = 0; c < numCols; c++) {
			if (denominator.mData[r * numCols + c] == 0) // we don't want to be dividing by zero
				imageTemp[r * numCols + c] = (float) numerator.mData[r * numCols + c].getIntensity();
			else
				imageTemp[r * numCols + c] = (float) numerator.mData[r * numCols + c].getIntensity() / denominator.mData[r * numCols + c].getIntensity();
		}
	}
	
	unsigned oldMin, oldMax;
	numerator.findMinMaxLevels(oldMin, oldMax);

	Image<PIXEL_TYPE> temp(numRows, numCols);
	temp.rescale(imageTemp, oldMin, oldMax);

        delete[] imageTemp;

	return temp;
}

template <class PIXEL_TYPE>
inline unsigned Image<PIXEL_TYPE>::getColorDepth() const
{
	// FIX ME!
	return 0;
}

// Non-inlined implementation

template <class PIXEL_TYPE>
float Image<PIXEL_TYPE>::findAngle() const
{
	int
		x_centroid = findXCentroid (),
		y_centroid = findYCentroid (),
		product_total = 0,
		x_squared = 0,
		y_squared = 0;

	float   b_over_ac = 0;
    
	for (unsigned r = 0; r < mRows; r++) {
		for(unsigned c = 0; c < mCols; c++) {
			if (mData[r * mCols + c] > 0) {
				product_total+=((c-x_centroid)*(r-y_centroid));
				x_squared+=((c-x_centroid)*(c-x_centroid));
				y_squared+=((r-y_centroid)*(r-y_centroid));
			}
		}
	}
	b_over_ac = (float)2*product_total/(float)(x_squared-y_squared);

	return ((float) atan ((double)b_over_ac)/(float)2);
}

template <class PIXEL_TYPE>
unsigned Image<PIXEL_TYPE>::findXCentroid() const
{
	int
		pixels = 0,
		pictureTotal = 0;
    
	for (unsigned r = 0; r < mRows; r++) {
		for (unsigned c = 0; c < mCols; c++) {
			if (mData[r * mCols + c] > 0) {
				pictureTotal += c;
				pixels++;
			}
		}
	}
	unsigned xCentroid = (unsigned)round((float)pictureTotal/pixels);
	return xCentroid;
}

template <class PIXEL_TYPE>
unsigned Image<PIXEL_TYPE>::findYCentroid () const
{
	int
		pixels = 0,
		pictureTotal = 0;
    
	for (unsigned c = 0; c < mCols; c++) {
		for (unsigned r = 0; r < mRows; r++) {
			if (mData[r * mCols + c] > 0) {
				pictureTotal += r;
				pixels++;
			}
		}
	}
	unsigned yCentroid = (unsigned)round((float)pictureTotal/pixels);
	return yCentroid;
}

template <class PIXEL_TYPE>
void Image<PIXEL_TYPE>::findMaxExtents (unsigned& xMin, unsigned& xMax, unsigned& yMin, unsigned& yMax) const
{                         
	xMax = yMax = 0;	// The lazy way of doing things
	xMin = yMin = 999999;	// NEEDS TO BE FIXED AT SOME POINT =/

	for (unsigned r = 0; r < mRows; r++)
		for (unsigned c = 0; c < mCols; c++) {
			if (mData[r * mCols + c] > 0) { 
				if (r < yMin) yMin = r;
				if (r > yMax) yMax = r;
				if (c < xMin) xMin = c;
				if (c > xMax) xMax = c;
			}
		}
}

template <class PIXEL_TYPE>
void Image<PIXEL_TYPE>::findMinMaxLevels(unsigned& min, unsigned& max) const
{
	max = min = (unsigned)mData[0].getIntensity();

	for (unsigned r  = 0; r < mRows; r++) {
		for (unsigned c = 0; c < mCols; c++) {
			if (mData[r * mCols + c] > max)
				max = (unsigned)mData[r * mCols + c].getIntensity();

			if (mData[r * mCols + c] < min)
				min = (unsigned)mData[r * mCols + c].getIntensity();
		}
	}
}

template <class PIXEL_TYPE>
void Image<PIXEL_TYPE>::rescale(float *imageTemp, int oldMin, int oldMax)
{
	float
		newMin,
		newMax,
		scale;

	newMin = newMax = imageTemp[0];

	unsigned r, c;

	for (r = 0; r < mRows; r++) {
		for (c = 0; c < mCols; c++) {
			if( imageTemp[r * mCols + c] > newMax )
				newMax = imageTemp[r * mCols + c];
			if( imageTemp[r * mCols + c] < newMin )
				newMin = imageTemp[r * mCols + c];
		}
	}

	scale = (float)(oldMax - oldMin) / (newMax - newMin);

	for (r = 0; r < mRows; r++)
		for (c = 0; c < mCols; c++)
                        mData[r * mCols + c] = round(scale * imageTemp[r * mCols + c]);
}

template <class PIXEL_TYPE>
inline size_t Image<PIXEL_TYPE>::bytesPerPixel() const
{
	return sizeof(PIXEL_TYPE);
}

#endif
