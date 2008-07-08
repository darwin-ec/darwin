/* 
 * BinaryImage.h
 * 
 * Author: Scott Hale
 * Date: 19-Aug-2005
 * 
 * This file provides two classes (BinaryImage and Point) and a struct (Feature)
 * 
 * BinaryImage
 * 	A BinaryImage extends ImageFile and is thus siblings with ColorImage and GrayImage
 * 	By definition, a binary image may only have pixel values of 0 or some max (255 in this case);
 * 	however, a BinaryImage as implemented allows a pixel to have any grayscale value.
 * 	However, in the binary operations provided all non-zero values are treated as white.
 * 
 * Feature
 * 	A struct containing a pointer to a BinaryImage and the number of pixels containted in 
 * 	that image. A Feature* is returned by BinaryImage::getLargetFeature.
 * 
 * Point
 * 	This class should likely be declared in its own file (possbily as a template class);
 * 	however, a point.h already exists with a structure with members of type float.
 * 	Point represents an (x,y) or (row,col) pair.
 * 	
 * 
 * Added in change 101AT
 */

#ifndef BINARYIMAGE_HH
#define BINARYIMAGE_HH

#include<stack>

#include "ImageFile.h"
#include "Pixel.h"

/*struct*/ class Feature;

class BinaryImage : public ImageFile<GrayPixel>
{
public:
	BinaryImage(unsigned nRows, unsigned nCols);
	BinaryImage(const std::string &filename);
	BinaryImage(const BinaryImage *image);
	
//	BinaryImage& operator=(const Image<GrayPixel>& i);
	virtual unsigned getColorDepth() const;
	bool isColor() const;
	
	//Feature Recognition
	Feature* getLargestFeature();
	
	//Binary Operators ! & | ^
	void doNot();
	void operator!();
	
	void doAnd(BinaryImage img);
	void operator&(BinaryImage img);
	
	void doOr(BinaryImage img);
	void operator|(BinaryImage img);
	
	void doXor(BinaryImage img);
	void operator^(BinaryImage img);
	
	//Morphological Operators
	int doErode(int coeff);
	int doDialate(int coeff);
	double getPixularity(); //103AT SAH
	
private:
	/******************************************************************
	 * FIND BLOB (taken from deburekr)
	 *    this function finds all pixels of the same value which are
	 *    connected (4 neighbor) to pixel wr,wc
	 *    The function prints the area of the blob and returns the new 
	 *    image with just the blob as its result.
	 *
	 * INPUTS: srcImg - the Image object upon which to operate (may be this).
	 *         seedrow, seedcol - the row and column of the seed pixel
	 ******************************************************************/
	Feature* find_blob(BinaryImage &srcImg, int seedrow, int seedcol);
	
};

//***1.0LK - added constructor and destructor to prevent memory leaks
/*struct*/ class Feature {
public :
	int area;
	BinaryImage* mask;
	Feature () : mask(NULL), area(0) {}
	~Feature() { if (mask) delete mask; }
};

inline
BinaryImage::BinaryImage(const std::string &filename)
	: ImageFile<GrayPixel>(filename)
{ }

inline
BinaryImage::BinaryImage(unsigned nRows, unsigned nCols)
	: ImageFile<GrayPixel>(nRows,nCols)
{ }

inline
BinaryImage::BinaryImage(const BinaryImage *image) 
	: ImageFile<GrayPixel>()
{
	mRows = image->mRows;
	mCols = image->mCols;

	mData = new GrayPixel[mRows * mCols];

	memcpy(mData, image->getData(), mRows * mCols * sizeof(GrayPixel));

	mInitialized = true;
}

inline unsigned BinaryImage::getColorDepth() const
{
	return 256; //ACK!
}

inline
bool BinaryImage::isColor() const
{
	return false;
}

//internal class (Should probably be it's own file, but there is already a point.h using floats.
//perhaps this class should be a template class and used throughout the application
class Point {
public:
	Point() {
		row=0;
		col=0;
	}

	Point(int r, int c) {
		row=r;
		col=c;
	}

	//virtual ~Point();
	
	int getRow() const {
		return row;
	}

	int getCol() const {
		return col;
	}
	
	void setRow(int r) {
		row=r;
	}

	void setCol(int c ) {
		col = c;
	}
	
private:
	int row;
	int col;	
};

//103AT SAH
//Pixularity metric array --whoever formatted the dilate and erosion arrays feel free to format this if you like, but I don't think it accomplishes anything.
//Yes I declared them int since I can't think of ever using the value in the image and I want to sum them to a number exceeding unsigned.
static int pixularity [512] = {1,4,5,4,4,7,4,3,5,4,10,6,8,7,9,5,6,9,5,6,9,13,6,7,5,6,7,4,9,10,8,5,5,8,10,9,4,7,6,5,9,8,15,11,8,7,11,7,5,9,7,8,6,10,4,5,5,6,9,6,6,7,6,3,4,7,8,7,7,10,7,6,4,3,9,5,7,6,8,4,9,13,9,10,13,17,10,11,6,7,8,5,10,11,9,6,8,11,13,12,7,10,9,8,8,7,14,10,7,6,10,6,9,13,11,12,10,14,8,9,6,7,10,7,7,8,7,4,5,8,9,8,8,11,8,7,10,9,15,11,13,12,14,10,5,9,5,6,9,13,6,7,7,8,9,6,11,12,10,7,10,13,15,14,9,12,11,10,15,14,21,17,14,13,17,13,7,11,9,10,8,12,6,7,9,10,13,10,10,11,10,7,4,7,8,7,7,10,7,6,6,5,11,7,9,8,10,6,6,10,6,7,10,14,7,8,4,5,6,3,8,9,7,4,9,12,14,13,8,11,10,9,11,10,17,13,10,9,13,9,8,12,10,11,9,13,7,8,6,7,10,7,7,8,7,4,4,7,8,7,7,10,7,6,8,7,13,9,11,10,12,8,9,13,9,10,13,17,10,11,9,10,11,8,13,14,12,9,4,7,9,8,3,6,5,4,8,7,14,10,7,6,10,6,6,10,8,9,7,11,5,6,6,7,10,7,7,8,7,4,7,10,11,10,10,13,10,9,7,6,12,8,10,9,11,7,13,17,13,14,17,21,14,15,10,11,12,9,14,15,13,10,7,10,12,11,6,9,8,7,7,6,13,9,6,5,9,5,10,14,12,13,11,15,9,10,7,8,11,8,8,9,8,5,4,7,8,7,7,10,7,6,9,8,14,10,12,11,13,9,6,10,6,7,10,14,7,8,8,9,10,7,12,13,11,8,6,9,11,10,5,8,7,6,11,10,17,13,10,9,13,9,4,8,6,7,5,9,3,4,6,7,10,7,7,8,7,4,3,6,7,6,6,9,6,5,5,4,10,6,8,7,9,5,7,11,7,8,11,15,8,9,5,6,7,4,9,10,8,5,5,8,10,9,4,7,6,5,7,6,13,9,6,5,9,6,5,9,7,8,6,10,4,5,3,4,7,4,4,5,4,1};


//Dialation / Erosion arrays. There's some syntax error putting these inside the class.
/****************************************************************
   for image dilation -
   entries indicate for white pixels, the # of black neighbors
						black pixels, zero
   pixels with # black neigbors > threshold coeff can be deleted
*****************************************************************/
static unsigned dilate [512] = {
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  8,  7,  7,  6,    7,  6,  6,  5,    7,  6,  6,  5,    6,  5,  5,  4,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  7,  6,  6,  5,    6,  5,  5,  4,    6,  5,  5,  4,    5,  4,  4,  3,  

  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  7,  6,  6,  5,    6,  5,  5,  4,    6,  5,  5,  4,    5,  4,  4,  3,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  6,  5,  5,  4,    5,  4,  4,  3,    5,  4,  4,  3,    4,  3,  3,  2,  

  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  7,  6,  6,  5,    6,  5,  5,  4,    6,  5,  5,  4,    5,  4,  4,  3,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  6,  5,  5,  4,    5,  4,  4,  3,    5,  4,  4,  3,    4,  3,  3,  2,  

  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  6,  5,  5,  4,    5,  4,  4,  3,    5,  4,  4,  3,    4,  3,  3,  2,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  5,  4,  4,  3,    4,  3,  3,  2,    4,  3,  3,  2,    3,  2,  2,  1,  

  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  7,  6,  6,  5,    6,  5,  5,  4,    6,  5,  5,  4,    5,  4,  4,  3,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  6,  5,  5,  4,    5,  4,  4,  3,    5,  4,  4,  3,    4,  3,  3,  2,  

  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  6,  5,  5,  4,    5,  4,  4,  3,    5,  4,  4,  3,    4,  3,  3,  2,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  5,  4,  4,  3,    4,  3,  3,  2,    4,  3,  3,  2,    3,  2,  2,  1,  

  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  6,  5,  5,  4,    5,  4,  4,  3,    5,  4,  4,  3,    4,  3,  3,  2,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  5,  4,  4,  3,    4,  3,  3,  2,    4,  3,  3,  2,    3,  2,  2,  1,  

  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  5,  4,  4,  3,    4,  3,  3,  2,    4,  3,  3,  2,    3,  2,  2,  1,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  4,  3,  3,  2,    3,  2,  2,  1,    3,  2,  2,  1,    2,  1,  1,  0
};

/****************************************************************
   for image erosion -
   entries indicate for black pixels, the # of white neighbors
						white pixels, zero
   pixels with # black neigbors > threshold coeff can be deleted
*****************************************************************/
static unsigned erode[512] = {
  0,  1,  1,  2,    1,  2,  2,  3,    1,  2,  2,  3,    2,  3,  3,  4,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  1,  2,  2,  3,    2,  3,  3,  4,    2,  3,  3,  4,    3,  4,  4,  5,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  

  1,  2,  2,  3,    2,  3,  3,  4,    2,  3,  3,  4,    3,  4,  4,  5,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  2,  3,  3,  4,    3,  4,  4,  5,    3,  4,  4,  5,    4,  5,  5,  6,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  

  1,  2,  2,  3,    2,  3,  3,  4,    2,  3,  3,  4,    3,  4,  4,  5,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  2,  3,  3,  4,    3,  4,  4,  5,    3,  4,  4,  5,    4,  5,  5,  6,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  

  2,  3,  3,  4,    3,  4,  4,  5,    3,  4,  4,  5,    4,  5,  5,  6,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  3,  4,  4,  5,    4,  5,  5,  6,    4,  5,  5,  6,    5,  6,  6,  7,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  

  1,  2,  2,  3,    2,  3,  3,  4,    2,  3,  3,  4,    3,  4,  4,  5,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  2,  3,  3,  4,    3,  4,  4,  5,    3,  4,  4,  5,    4,  5,  5,  6,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  

  2,  3,  3,  4,    3,  4,  4,  5,    3,  4,  4,  5,    4,  5,  5,  6,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  3,  4,  4,  5,    4,  5,  5,  6,    4,  5,  5,  6,    5,  6,  6,  7,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  

  2,  3,  3,  4,    3,  4,  4,  5,    3,  4,  4,  5,    4,  5,  5,  6,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  3,  4,  4,  5,    4,  5,  5,  6,    4,  5,  5,  6,    5,  6,  6,  7,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  

  3,  4,  4,  5,    4,  5,  5,  6,    4,  5,  5,  6,    5,  6,  6,  7,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,  
  4,  5,  5,  6,    5,  6,  6,  7,    5,  6,  6,  7,    6,  7,  7,  8,  
  0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0
};	
#endif
