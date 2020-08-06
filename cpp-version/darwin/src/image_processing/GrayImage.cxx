//*******************************************************************
//   file: GrayImage.cxx
//
// author: Scott Hale
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include "GrayImage.h"

using namespace std;

GrayImage::GrayImage(unsigned nRows, unsigned nCols, ColorPixel *data)
{
	if (nRows == 0 || nCols == 0)
		throw Error("Bad image dimensions in GrayImage ctor.");

	if (NULL == data)
		throw Error("NULL data in GrayImage ctor.");

	mRows = nRows;
	mCols = nCols;

	mData = new GrayPixel[mRows * mCols];

	for (unsigned r = 0; r < mRows; r++)
		for (unsigned c = 0; c < mCols; c++)
			mData[r * mCols + c] = data[r * mCols + c].getIntensity();
	
	mInitialized = true;
}

/*
 * Contrust a Histogram<GrayImage> based on the GrayImage represented by this
 * 	object.
 * 
 * @return a pointer to the Histogram<GrayImage> constructed.
 */
Histogram<GrayImage>* GrayImage::getHistogram() {
	return new Histogram<GrayImage>(this);
}


/*
 * Construct a BinaryImage by thresholding this GrayImage based on Range rng.
 * All values not within the Range representated by rng (rng->start - rng->end) (inclusive)
 * are while, while all values within range are black.
 * 
 * The orginal GrayImage is unaffected.
 *
 * @return a pointer to the BinaryImage constructed.
 */
BinaryImage* GrayImage::doThreshold(Range* rng) {

	unsigned val;
	BinaryImage *img = new BinaryImage(mRows,mCols);

   for (int r=0; r<mRows; r++){
 	for (int c=0; c<mCols; c++){
 	  val = (*this)(r,c).getIntensity();
 	  if (val > rng->end || val < rng->start ) {
	    val = 255;//MAX_PIXEL_INTENSIY;//Pixle.h
	  }
	  else {//in range
	    val=0;
	  }
 	  img->mData[r*mCols+c].setIntensity(val);
 	}
   }
	return img;
}
