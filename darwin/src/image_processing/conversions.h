//*******************************************************************
//   file: Conversions.h
//
// author: Adam Russell?
//
//   mods: 
//
//*******************************************************************

#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include "GrayImage.h"
#include "ColorImage.h"
#include "Error.h"

GrayImage* convColorToGray(const ColorImage* srcImage);
GrayImage* convColorToCyan(const ColorImage* srcImage);//AT102, 103AT

// will dynamically allocat **pix itself!
// use the utility.h function freePixmapString to free it.
// Ugly, I know.. but more convenient than the alternatives considering
// what the gdk* functions need.
//
// This is an ugly function.. I wrote it quickly.. it can't handle large
// images.
void convColorToPixmapString(const ColorImage* srcImage, char **&pix, int &rows);

void convColorToPixmapString(
		const ColorImage *srcImage,
		int desiredPixHeight,
		int desiredPixWidth,
		char **&pix,
		int &rows);

template <class IMAGE_TYPE>
void convToPixmapString(const IMAGE_TYPE* srcImage, char **&pix, int &rows)
{
	if (srcImage->isColor())
		convColorToPixmapString(srcImage, pix, rows);
	else {
		pix = NULL;
		rows = 0;
	}
}

//////////////// new stuff - JHS - 11/21/2005 ///////////////
void makeDoubleSizePixmapString(char **pixIn, char **&pixOut, int &rowsOut);


#endif
