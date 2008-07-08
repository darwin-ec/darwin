//*******************************************************************
//   file: Transform.h
//
// author: Adam Russell
//
//   mods:
// 
//*******************************************************************

#ifndef TRANSFORM_H
#define TRANSFORM_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

// has to be ../Error.h because MSVC++ has
// another error.h somewhere that gets included otherwise
#include "types.h" // for byte & extent_t
#include "../Error.h"
#include "Histogram.h"

//////////////////////////////////////////////////////////////////////
// Some image error classes that can be thrown as exceptions.  This
// probably isn't the best place to put them, but it's convenient to
// have them here...

class ImageEmpty : public Error {
public:
	ImageEmpty()
		: Error("Image is empty.")
	{ }

	ImageEmpty(std::string s)
		: Error("Attempt to operate on an empty image in: " + s)
	{ }
};

// End of error classes.
/////////////////////////////////////////////////////////////////////

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* resizeWithBorder(const TRANSFORM_IMAGE_TYPE *srcImage, int newHeight, int newWidth);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* resizeWithBorderNN(const TRANSFORM_IMAGE_TYPE *srcImage, int newHeight, int newWidth);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* crop(const TRANSFORM_IMAGE_TYPE* srcImage, int xMin, int yMin, int xMax, int yMax);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* mapToHistogram(const TRANSFORM_IMAGE_TYPE* srcImage, const TRANSFORM_IMAGE_TYPE* controlImage);

// resize using (N)earest (N)eighbor
template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* resizeNN(const TRANSFORM_IMAGE_TYPE* srcImage, unsigned newHeight, unsigned newWidth);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* resizeNN(const TRANSFORM_IMAGE_TYPE *srcImage, float percentage);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* resize(const TRANSFORM_IMAGE_TYPE* srcImage, unsigned newHeight, unsigned newWidth);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* resize(const TRANSFORM_IMAGE_TYPE *srcImage, float percentage);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* rotate(const TRANSFORM_IMAGE_TYPE* srcImage, float angle);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* rotateXAxis(const TRANSFORM_IMAGE_TYPE* srcImage, float angle);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* rotateYAxis(const TRANSFORM_IMAGE_TYPE* srcImage, float angle);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* rotateNN(const TRANSFORM_IMAGE_TYPE* srcImage, float angle);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* translate(const TRANSFORM_IMAGE_TYPE* srcImage, int dx, int dy);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* flipHorizontally(const TRANSFORM_IMAGE_TYPE* srcImage);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* flipVertically(const TRANSFORM_IMAGE_TYPE* srcImage);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* alterBrightness(const TRANSFORM_IMAGE_TYPE* srcImage,int increment);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* enhanceContrast(const TRANSFORM_IMAGE_TYPE* srcImage, unsigned minLevel, unsigned maxLevel);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* median(const TRANSFORM_IMAGE_TYPE* srcImage);

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* smooth(const TRANSFORM_IMAGE_TYPE* srcImage);

// IMPLEMENTATION

// resizeNN
// 	Creates a new image of the specified size using the image passed to
// it.  Uses nearest neighbor type determining method for the new image's
// values, which may lead to grainy results.
//
// PARAMETERS:
// 	Image* srcImage - The image to make a resized version of.
// 	unsigned newHeight - The height for the new image
// 	unsigned newWidth - The width for the new image
//
// RETURN: Image* - The resized version of srcImage
//
template <class TRANSFORM_IMAGE_TYPE> 
TRANSFORM_IMAGE_TYPE* resizeNN(const TRANSFORM_IMAGE_TYPE* srcImage, unsigned newHeight, unsigned newWidth)
{
	if (srcImage == NULL) throw ImageEmpty("resizeNN()");

	unsigned
		oldNumRows = srcImage->getNumRows (),
		oldNumCols = srcImage->getNumCols (),
		rowPos,
		colPos;
#ifdef DEBUG
	cout << "Zoom image - height: " << newHeight
	     << " width: " << newWidth << endl;
#endif
        try {
                TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(newHeight, newWidth);

                for (unsigned r = 0; r < newHeight; r++) {
                        for (unsigned c = 0; c < newWidth; c++) {
                                rowPos = (unsigned) round((float)r / (newHeight 
- 1) * (oldNumRows - 1));
                                colPos = (unsigned) round((float)c / (newWidth -
 1) * (oldNumCols - 1));

                                (*dstImage)(r, c) = (*srcImage)(rowPos, colPos);
                        }
                }

                return dstImage;

        } catch (...) {
                throw;
        }
}

// I'm assuming the percentage is in the form 100.0 to keep the same
// size, 50.0 to half the size, etc.
template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* resizeNN(const TRANSFORM_IMAGE_TYPE *srcImage, float percentage)
{
	if (NULL == srcImage)
		throw ImageEmpty("resize(TRANSFORM_IMAGE_TYPE, float)");
	
	int newWidth, newHeight;
	float scale = percentage / 100.0;
	newWidth = (int) round(srcImage->getNumCols() * scale);
	newHeight = (int) round(srcImage->getNumRows() * scale);
	
	TRANSFORM_IMAGE_TYPE *dstImage = resizeNN(srcImage, newHeight, newWidth);
	
	return dstImage;
}

// I'm assuming the percentage is in the form 100.0 to keep the same
// size, 50.0 to half the size, etc.
template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* resize(const TRANSFORM_IMAGE_TYPE *srcImage, float percentage)
{
	if (NULL == srcImage)
		throw ImageEmpty("resize(TRANSFORM_IMAGE_TYPE, float)");
	
	int newWidth, newHeight;
	float scale = percentage / 100.0;
	newWidth = (int) round(srcImage->getNumCols() * scale);
	newHeight = (int) round(srcImage->getNumRows() * scale);
	
	TRANSFORM_IMAGE_TYPE *dstImage = resize(srcImage, newHeight, newWidth);
	
	return dstImage;
}


template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* resizeWithBorder(const TRANSFORM_IMAGE_TYPE *srcImage, int newHeight, int newWidth)
{
	if (NULL == srcImage)
		throw ImageEmpty("resizeWithBorder()");

	try {
		float heightRatio, widthRatio, aspectRatio;
		
		heightRatio = srcImage->getNumRows() / (float)newHeight;
		widthRatio = srcImage->getNumCols() / (float)newWidth;
		aspectRatio = srcImage->getNumRows() / (float)srcImage->getNumCols();
		
		int tmpHeight, tmpWidth;
		
		if (widthRatio > heightRatio) {
			tmpHeight = (int)round(aspectRatio * newWidth);
	       		tmpWidth = newWidth;
		} else {
			tmpHeight = newHeight;
			tmpWidth = (int)round(newHeight / aspectRatio);
		}

		TRANSFORM_IMAGE_TYPE *tmpImage = resize(srcImage, tmpHeight, tmpWidth);
		TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(newHeight, newWidth);

		int heightOffset, widthOffset, bytesPerRow, offset;
		widthOffset = ((newWidth - tmpWidth) / 2) * dstImage->bytesPerPixel();
		bytesPerRow = tmpWidth * dstImage->bytesPerPixel();
		heightOffset = ((newHeight - tmpHeight) / 2) * newWidth * dstImage->bytesPerPixel();
		offset = widthOffset + heightOffset;

		int bytesPerRowOriginal = newWidth * dstImage->bytesPerPixel();

		for (int i = 0; i < tmpHeight; i++)
			memcpy(
				(char*)dstImage->getData() + offset + bytesPerRowOriginal * i,
				(char*)tmpImage->getData() + bytesPerRow * i,
				bytesPerRow
				);

		delete tmpImage;
		return dstImage;
	} catch (...) {
		throw;
	}
}

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* resizeWithBorderNN(const TRANSFORM_IMAGE_TYPE *srcImage, int newHeight, int newWidth)
{
	if (NULL == srcImage)
		throw ImageEmpty("resizeWithBorderNN()");

	try {
		float heightRatio, widthRatio, aspectRatio;
		
		heightRatio = srcImage->getNumRows() / (float)newHeight;
		widthRatio = srcImage->getNumCols() / (float)newWidth;
		aspectRatio = srcImage->getNumRows() / (float)srcImage->getNumCols();
		
		int tmpHeight, tmpWidth;
		
		if (widthRatio > heightRatio) {
			tmpHeight = (int)round(aspectRatio * newWidth);
	       		tmpWidth = newWidth;
		} else {
			tmpHeight = newHeight;
			tmpWidth = (int)round(newHeight / aspectRatio);
		}

		TRANSFORM_IMAGE_TYPE *tmpImage = resizeNN(srcImage, tmpHeight, tmpWidth);
		TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(newHeight, newWidth);

		int heightOffset, widthOffset, bytesPerRow, offset;
		widthOffset = ((newWidth - tmpWidth) / 2) * dstImage->bytesPerPixel();
		bytesPerRow = tmpWidth * dstImage->bytesPerPixel();
		heightOffset = ((newHeight - tmpHeight) / 2) * newWidth * dstImage->bytesPerPixel();
		offset = widthOffset + heightOffset;

		int bytesPerRowOriginal = newWidth * dstImage->bytesPerPixel();

		for (int i = 0; i < tmpHeight; i++)
			memcpy(
				(char*)dstImage->getData() + offset + bytesPerRowOriginal * i,
				(char*)tmpImage->getData() + bytesPerRow * i,
				bytesPerRow
				);

		delete tmpImage;
		return dstImage;
	} catch (...) {
		throw;
	}
}


// resize
// 	Creates a new image of the specified size using the image passed to
// it.  Uses nearest neighbor type determining method for the new image's
// values, which may lead to grainy results.
//
// PARAMETERS:
// 	Image* srcImage - The image to make a resized version of.
// 	unsigned newHeight - The height for the new image
// 	unsigned newWidth - The width for the new image
//
// RETURN: Image* - The resized version of srcImage
//
template <class TRANSFORM_IMAGE_TYPE> 
TRANSFORM_IMAGE_TYPE* resize(const TRANSFORM_IMAGE_TYPE* srcImage, unsigned newHeight, unsigned newWidth)
{
	if (srcImage == NULL) throw ImageEmpty("resize()");

	unsigned	
		numCols = srcImage->getNumCols(),
		numRows = srcImage->getNumRows(),
		lowX,
		highX,
		lowY,
		highY;

	float
		fracX,
		fracY,
		origX,
		origY;	

#ifdef DEBUG
	cout << "Resizing image - height: " << newHeight
	     << " width: " << newWidth << endl;
#endif
	TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(newHeight, newWidth);

	for (unsigned r = 0; r < newHeight; r++) {
		for (unsigned c = 0; c < newWidth; c++) {
			origX = (float) c / newWidth * numCols;
			origY = (float) r / newHeight * numRows;  

			if(origX >= 0 && origX < (numCols - 1) && origY >= 0 && origY < (numRows - 1)) {
				lowX = (int)origX;
				highX = (int)origX + 1;
				lowY = (int)origY;
				highY = (int)origY + 1;
						
				fracX = origX - lowX;
				fracY = origY - lowY;		     
						
				(*dstImage)(r,c) = ((*srcImage)(lowY, highX) - (*srcImage)(lowY, lowX))*fracX
					+ ((*srcImage)(highY, lowX) - (*srcImage)(lowY, lowX))*fracY
					+ ((*srcImage)(highY, highX) + (*srcImage)(lowY, lowX) 
			        	- (*srcImage)(highY, lowX) - (*srcImage)(lowY, highX))*(fracX*fracY)
					+ (*srcImage)(lowY, lowX);
			}	
		}
	}
	return dstImage;	
}

// crop
//
// PARAMETERS:
// 	Image* srcImage - The image to make a cropped version of.
// 	unsigned xMin - The place on the x-axis to start cropping (CAN START AT ZERO)
// 	unsigned xMax - The place on x-axis to stop crop (UP TO cols - 1)
// 	unsigned yMin - Start on y-axis (CAN START AT ZERO)
// 	unsigned yMax - Stop on y-axis (UP TO rows - 1)
//
// RETURN: Image* - The cropped version of srcImage.
//
template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* crop(const TRANSFORM_IMAGE_TYPE* srcImage, int xMin, int yMin, int xMax, int yMax)
{
	if (srcImage == NULL) throw ImageEmpty ("crop()");

	unsigned
		numRows,	// number of rows in the destination image
		numCols;	// number of columns in the destination image

	numRows = yMax - yMin + 1;
	numCols = xMax - xMin + 1;

	// Check validity of image dimensions
	if (numRows > srcImage->getNumRows () || numCols > srcImage->getNumCols ())
		return NULL;

	// allocate enough space to store the destination image
	TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(numRows, numCols);

	// perform the crop
	for (int row = yMin; row <= yMax; row++)
		for (int col = xMin; col <= xMax; col++)
			(*dstImage)(row - yMin, col - xMin) = (*srcImage)(row, col);

	return dstImage;
}

// rotate
//
// PARAMETERS:
// 	Image* srcImage - the image to make a rotated version of.
// 	float angle - The amount to rotate the image IN RADIANS.
//
// RETURN: Image* - The rotated version of srcImage.

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* rotate(const TRANSFORM_IMAGE_TYPE* srcImage, float angle)
{
	if (srcImage == NULL) throw ImageEmpty("rotate()");
#ifdef DEBUG
	cout << "Rotating image " << angle << " radians." << endl;
#endif
	
	unsigned
		numCols = srcImage->getNumCols(),
		numRows = srcImage->getNumRows(),
		centerCol = numCols/2,
		centerRow = numRows/2,
		lowX,
		highX,
		lowY,
		highY;
		
	float
		fracX,
		fracY,
		origX,
		origY,
		p_sin = (float)sin(angle),	// precomputed sin of angle to save time
                p_cos = (float)cos(angle); 	// and the same for cosine

	
	TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE (numRows, numCols);
	//memset(dstImage->getData(), 255, numRows*numCols);
	for (register unsigned r=0; r < numRows; r++) {
		for (register unsigned c=0; c < numCols; c++) {
			origX = (c*p_cos - r*p_sin + (centerCol*(1-p_cos) + (centerRow*p_sin)));
			origY = (c*p_sin + r*p_cos + (centerRow*(1-p_cos) - (centerCol*p_sin)));
						
			//check to see whether the points are in bounds
			if(origX >= 0 && origX < (numCols - 1) && origY >= 0 && origY < (numRows - 1)) {
				lowX = (int)origX;
				highX = (int)origX + 1;
				lowY = (int)origY;
				highY = (int)origY + 1;
						
				fracX = origX - lowX;
				fracY = origY - lowY;
						
				(*dstImage)(r,c) = ((*srcImage)(lowY, highX) - (*srcImage)(lowY, lowX))*fracX
					+ ((*srcImage)(highY, lowX) - (*srcImage)(lowY, lowX))*fracY
					+ ((*srcImage)(highY, highX) + (*srcImage)(lowY, lowX) 
			        	- (*srcImage)(highY, lowX) - (*srcImage)(lowY, highX))*(fracX*fracY)
					+ (*srcImage)(lowY, lowX);
			}	
		}
	}
	return dstImage;
}

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* rotateYAxis(const TRANSFORM_IMAGE_TYPE* srcImage, float angle)
{
	if (NULL == srcImage)
		throw EmptyArgumentError("rotateYAxis [*srcImage]");
	
	try {
		float
			pCos = cos(angle),
			pSin = sin(angle);

		TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(srcImage->getNumRows(), srcImage->getNumCols());
		int x;
		for (int r = 0; r < srcImage->getNumRows(); r++) {
			for (int c = 0; c < srcImage->getNumCols(); c++) {
				x = (int) round(c * pCos + pSin);

				if (x >= 0 && x < srcImage->getNumCols())
					(*dstImage)(r, x) = (*srcImage)(r, c);
			}
		}

		return dstImage;
		
	} catch (...) {
		throw;
	}
}

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* rotateXAxis(const TRANSFORM_IMAGE_TYPE* srcImage, float angle)
{
	if (NULL == srcImage)
		throw EmptyArgumentError("rotateYAxis [*srcImage]");
	
	try {
		float
			pCos = cos(angle),
			pSin = sin(angle);

		TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(srcImage->getNumRows(), srcImage->getNumCols());
		int y;
		for (int r = 0; r < srcImage->getNumRows(); r++) {
			for (int c = 0; c < srcImage->getNumCols(); c++) {
				y = (int) round(r * pCos - pSin);

				if (y >= 0 && y < srcImage->getNumCols())
					(*dstImage)(y, c) = (*srcImage)(r, c);
			}
		}

		return dstImage;
		
	} catch (...) {
		throw;
	}
}

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* rotateNN(const TRANSFORM_IMAGE_TYPE* srcImage, float angle)
{
	if (srcImage == NULL) throw ImageEmpty("rotate()");
#ifdef DEBUG
	cout << "Rotating image " << angle << " radians." << endl;
#endif
	
	unsigned
		numCols = srcImage->getNumCols(),
		numRows = srcImage->getNumRows(),
		centerCol = numCols/2,
		centerRow = numRows/2,
		origX,
		origY;	
	float
		p_sin = (float)sin(angle),	// precomputed sin of angle to save time
                p_cos = (float)cos(angle); 	// and the same for cosine

	
	TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(numRows, numCols);

	for (unsigned r = 0; r < numRows; r++) {
		for (unsigned c = 0; c < numCols; c++) {
			origX = (int) round((c*p_cos - r*p_sin + (centerCol*(1-p_cos) + (centerRow*p_sin))));
			origY = (int) round((c*p_sin + r*p_cos + (centerRow*(1-p_cos) - (centerCol*p_sin))));
						
			//check to see whether the points are in bounds
			if(origX >= 0 && origX < numCols && origY >= 0 && origY < numRows)
				(*dstImage)(r,c) = (*srcImage)(origY, origX);
		}
	}
	return dstImage;
}


// translate
// 
// Note: We don't check to see whether dx and dy will translate image data out
// 	of the viewable range; we just let it go...
//
// PARAMETERS:
// 	Image* srcImage - The image we'd like to make a translated version of.
// 	int dx - Amount to move along the x-axis.
// 	int dy - Amount to move along the y-axis.
// 
// RETURN: Image* - The translated version of srcImage
template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* translate(const TRANSFORM_IMAGE_TYPE* srcImage, int dx, int dy)
{
	if (srcImage == NULL) throw ImageEmpty("translate()");

#ifdef DEBUG
	cout << "Translating image " << dx << " x, " << dy << " y." << endl;
#endif
	unsigned numRows = srcImage->getNumRows();
	unsigned numCols = srcImage->getNumCols();

	TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(numRows, numCols);

	// Rather than doing extra work later on by looking through the
	// entire image, we'll calculate where we need to stop and end right now.
	unsigned rowStart = (dy < 0) ? 0 : dy;
	unsigned rowEnd = numRows - abs(dy) + rowStart; 
	unsigned colStart = (dx < 0) ? 0 : dx;
	unsigned colEnd = numCols - abs(dx) + colStart;

	for (unsigned r = rowStart; r < rowEnd; r++)
		for (unsigned c = colStart; c < colEnd; c++)
			(*dstImage)(r, c) = (*srcImage)(r - dy, c - dx);

	return dstImage;
}

// flipHorizontally
//
//	Flips the image horizontally, and returns the new image
//
// PARAMETERS:
// 	Image* srcImage  //Original image
//
// RETURNS:
// 	Image*   //New, flipped image
template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* flipHorizontally(const TRANSFORM_IMAGE_TYPE* srcImage)
{
	if (srcImage == NULL) throw ImageEmpty("flipHorizontally()");

#ifdef DEBUG
	cout << "Flipping image horizontally\n";
#endif //DEBUG

	unsigned
		numCols = srcImage->getNumCols(),    //# of columns in image
		numRows = srcImage->getNumRows();    //# of rows in image

	//line of symmetry
	unsigned symCol = numCols / 2;

	TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(numRows,numCols);

	try {
		for(register unsigned r = 0; r < numRows; r++) {
			for(register unsigned c = 0; c < symCol; c++) {
				(*dstImage)(r, c) = (*srcImage)(r, numCols-1-c);
				(*dstImage)(r, numCols-1-c) = (*srcImage)(r, c);

			}
		}
		
		if (numCols % 2 != 0) {
			for (unsigned r = 0; r < numRows; r++)
				(*dstImage)(r, symCol) = (*srcImage)(r, symCol);
		}
	}
	catch ( ... ) {
		throw;
	}
	return dstImage;
}


// flipVertically
//
//   Flips image vertically, then returns new image
//
//   PARAMETERS:
//   	TRANSFORM_IMAGE_TYPE* srcImage     //Original image
//
//   Returns: new, flipped image
template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* flipVertically(const TRANSFORM_IMAGE_TYPE* srcImage)
{
	if (srcImage == NULL) throw ImageEmpty("flipVertically()");
		
#ifdef DEBUG
	cout << "Flipping image vertically\n";

#endif

	unsigned 
		numCols = srcImage->getNumCols(),
		numRows = srcImage->getNumRows();
	
	//line of symmetry
	unsigned symRow = numRows/2;
	
	TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(numRows, numCols);

	try {
		for(unsigned c = 0; c < numCols; c++) {
			for(unsigned r = 0; r < symRow; r++){
				(*dstImage)(r, c) = (*srcImage)(numRows-1-r, c);
				(*dstImage)(numRows-1-r, c) = (*srcImage)(r, c);
			}
		}

		if (numRows % 2 != 0)
			memcpy(&((*dstImage)(symRow, 0)), &((*srcImage)(symRow, 0)), numCols * dstImage->bytesPerPixel());
	} catch (...) {
		throw;
	}

	return dstImage;
}


// alterBrightness
//
//  adjusts the brightness of the image
//
// PARAMETERS:
// 	Image* srcImage    //Original Image
// 	int increment      //Increment to adjust intensity
//
// RETURN: Image
template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* alterBrightness(const TRANSFORM_IMAGE_TYPE* srcImage, int increment)
{
	if (srcImage == NULL) throw ImageEmpty("alterBrightness()");
		
#ifdef DEBUG
	cout << "Incrementing brightness by " << increment << endl;

#endif //DEBUG
	
	unsigned
		numCols = srcImage->getNumCols(),
		numRows = srcImage->getNumRows();
	
	TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE (numRows, numCols);
	
	*dstImage = *srcImage;
	unsigned tempBrightness;
	try {
		for(register unsigned c = 0; c < numCols; c++) {
		       for(register unsigned r = 0; r < numRows; r++) {
			       if(increment > 0) {
					if (((*srcImage)(r,c) >= 0) &&
					    ((*srcImage)(r,c) <= (255-increment))) 
					        tempBrightness = (*dstImage)(r,c).getIntensity()+increment;
					if ((*srcImage)(r,c) > (255 - increment)) 
						tempBrightness = 255;
					(*dstImage)(r,c).setIntensity((byte) (tempBrightness));
			       }
			       else if(increment < 0) {
					if (((*srcImage)(r,c) >= 0-increment) && 
					    ((*srcImage)(r,c) <= (255))) 
						tempBrightness = (*dstImage)(r,c).getIntensity()+increment;
					if((*srcImage)(r,c) < (0 - increment))
						tempBrightness = 0;
					(*dstImage)(r,c).setIntensity((byte) (tempBrightness));
				}
				else {
					(*dstImage)(r,c) = (*srcImage)(r,c);
				}
			}		
		}
	}
	catch (...) {
		throw;
	}
	return dstImage;
}	


// enhanceContrast
//
//   adjusts contrast of the image
//
// PARAMETERS:
// 	Image* srcImage
//
// RETURN: Image*
template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* enhanceContrast(const TRANSFORM_IMAGE_TYPE* srcImage, unsigned minLevel, unsigned maxLevel)
{
	if(srcImage == NULL) throw ImageEmpty("enhanceContrast()");

#ifdef DEBUG
	cout << "increasing contrast of image\n";
#endif	

	unsigned
		numCols = srcImage->getNumCols(),
		numRows = srcImage->getNumRows();

	TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(numRows, numCols);
	
	*dstImage = *srcImage;	
	int range = maxLevel - minLevel;
	unsigned tempIntensity;	
	if ((maxLevel!=255) || (minLevel!=0)){
		try {
			for(register unsigned c = 0; c < numCols; c++) {
				for(register unsigned r = 0; r < numRows; r++) {
					tempIntensity = (int)round ((((*srcImage)(r,c).getIntensity()-minLevel)*255)/(float) range);
					if (((*srcImage)(r,c).getIntensity()) < minLevel)
						tempIntensity = 0;
					if (tempIntensity < 1)
						tempIntensity = 0;
					if (tempIntensity > 254)
						tempIntensity = 255;
					
					(*dstImage)(r,c).setIntensity((byte) (tempIntensity));
				
				//	(*dstImage)(r,c) = (int)round ((((*srcImage)(r,c)-minLevel)*255)/(float) range);
				}
			}	
		}
		catch ( ... ) {
			throw;
		}
	}		
	else {
		*dstImage = *srcImage;
	}
	return dstImage;
}


template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* median(const TRANSFORM_IMAGE_TYPE* srcImage) {
	if(srcImage == NULL) throw ImageEmpty("median()");

	unsigned filterSize = 3;
	
	unsigned border = filterSize/2;
	unsigned numNeighbors = filterSize*filterSize;
	int  j, temp;
	
#ifdef DEBUG
	cout << "median filtering\n";
#endif //DEBUG

	unsigned
		numCols = srcImage->getNumCols(),
		numRows = srcImage->getNumRows();

	TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(numRows, numCols);

	int *range = new int[numNeighbors];
	//*dstImage = *srcImage;
	//unsigned tempFilter;

	try {
		//cout << "median filter" << endl;
		unsigned r, c;
		for(r = border; r < (numRows-1); r++) {
			for(c = border; c < (numCols-1); c++) { 
				int count = 0;
				for(register unsigned sr = (r-1); sr <= (r+1); sr++) {
					for(register unsigned sc = (c-1); sc <= (c+1); sc++) {	
						range [count] = (*srcImage)(sr,sc).getIntensity();
						count++;	
					}
				}
				for(register unsigned i = 1; i < numNeighbors; i++) {
					if((range [i]) < (range [i-1])) {
						j = i;
						while(range [j] <= range [j-1]) {
							temp = range[j];
							range[j] = range[j-1];
							range[j-1] = temp;
							j--;
						}
					}
				}	
			//	tempFilter = range [numNeighbors/2];	
			//	(*dstImage)(r,c).setIntensity((byte) (tempFilter));
				(*dstImage)(r,c) = range [numNeighbors/2];
			}
		}
		for (r = 0; r < border; r++)
			for(c = 0; c < numCols; c++)
				(*dstImage)(r,c) = (*srcImage)(r,c);

		for (r = (numRows - 2); r < numRows; r++)
			for (c = 0; c < numCols; c++)
				(*dstImage)(r,c) = (*srcImage)(r,c);

		for (c = 0; c < border; c++)
			for(unsigned r = 0; r < numRows; r++)
				(*dstImage)(r,c) = (*srcImage)(r,c);

		for (c = (numCols - 2); c < numCols; c++)
			for(unsigned r = 0; r < numRows; r++)
				(*dstImage)(r,c) = (*srcImage)(r,c);

	}
       	catch (...) {
		delete[] range;
		throw;
	}
	delete[] range;
	return dstImage;
}

template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* smooth(const TRANSFORM_IMAGE_TYPE* srcImage) {
	if(srcImage == NULL) throw ImageEmpty("smooth()");

	unsigned filterSize = 3;
	
	unsigned border = filterSize/2;
	unsigned numNeighbors = filterSize*filterSize;
	int sum, average;
	
#ifdef DEBUG
	cout << "smooth\n";

#endif //DEBUG

	unsigned
		numCols = srcImage->getNumCols(),
		numRows = srcImage->getNumRows();

	TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(numRows, numCols);

	try {
		unsigned r, c;
		for(r = border; r < (numRows-1); r++) {
			for(c = border; c < (numCols-1); c++) {
				sum = 0;
				for(unsigned sr = (r-1); sr <= (r+1); sr++)
					for(unsigned sc = (c-1); sc <= (c+1); sc++)
						sum = sum + (*srcImage)(sr,sc).getIntensity();

				average = sum / numNeighbors;
				(*dstImage)(r,c) = average;
			}
		}
		for(r = 0; r < border; r++)
			for(unsigned c = 0; c < numCols; c++)
				(*dstImage)(r,c) = (*srcImage)(r,c);

		for(r = (numRows - 2); r < numRows; r++)
			for(unsigned c = 0; c < numCols; c++)
				(*dstImage)(r,c) = (*srcImage)(r,c);

		for(c = 0; c < border; c++)
			for(unsigned r = 0; r < numRows; r++)
				(*dstImage)(r,c) = (*srcImage)(r,c);

		for(c = (numCols - 2); c < numCols; c++)
			for(unsigned r = 0; r < numRows; r++)
				(*dstImage)(r,c) = (*srcImage)(r,c);

	} catch (...) {
		throw;
	}

	return dstImage;
}

// mapToHistogram
//
// PARAMETERS:
// 	Image* srcImage
// 	Image* controlImage
//
// RETURN: Image*
template <class TRANSFORM_IMAGE_TYPE>
TRANSFORM_IMAGE_TYPE* mapToHistogram(const TRANSFORM_IMAGE_TYPE* srcImage, const TRANSFORM_IMAGE_TYPE* controlImage)
{
	if (srcImage == NULL || controlImage == NULL)
		throw ImageEmpty("mapToHistogram()");

	Histogram<TRANSFORM_IMAGE_TYPE> oldHistogram(srcImage);
	Histogram<TRANSFORM_IMAGE_TYPE> wantHistogram(controlImage);

	unsigned histSize = oldHistogram.getSize();

	//allocate enough space for the replacement histogram
	unsigned *replacementHist = new unsigned[histSize];

	unsigned cSum = 0;
	unsigned wSum = 0;
	unsigned cIndex = 1; // we'll start after black...

	for (unsigned wIndex = 1; wIndex < histSize; wIndex++) {
		wSum += wantHistogram.getValue (wIndex);

		while (cSum + oldHistogram.getValue (cIndex) <= wSum) {
                        cSum += oldHistogram.getValue (cIndex);
			replacementHist[cIndex] = wIndex;
			cIndex ++;
		}
	}

	// Now, use the replacement histogram to create a new image based
	// on srcImage with unsignedensity values mapped to the same range as
	// those in controlImage
	TRANSFORM_IMAGE_TYPE *dstImage = new TRANSFORM_IMAGE_TYPE(srcImage->getNumRows (), srcImage->getNumCols ());
	
	for (unsigned r = 0; r < srcImage->getNumRows (); r++)
		for (unsigned c = 0; c < srcImage->getNumCols (); c++)
			(*dstImage)(r, c) = replacementHist[(*srcImage)(r,c).getIntensity ()];

	delete[] replacementHist;

	return dstImage;
}

#endif
