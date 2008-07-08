//*******************************************************************
//   file: conversions.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (7/25/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include "transform.h"
#include "conversions.h"
#include "ColorImage.h"
#include "GrayImage.h"
#include "Image.h"
#include "Pixel.h"
#include <list>
#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

using namespace std;


void convColorToPixmapString(
		const ColorImage *srcImage,
		int desiredPixHeight,
		int desiredPixWidth,
		char **&pix,
		int &rows)
{
	ColorImage *temp = resizeWithBorderNN(srcImage, desiredPixHeight, desiredPixWidth);
	convColorToPixmapString(temp, pix, rows);
	delete temp;
}


//*******************************************************************
//
// void incrementColorString(string &c)
// 
//    utility function just to increment a color for convColorToPixmapString. 
//    no error checking.. always assumes it's two characters long :(
//
void incrementColorString(string &c)
{
	if ((int)c[1] == 127) {
		c[1] = '!';
		c[0] += 1;

		if (c[0] == '"')
			c[0] += 1;
	} else {
		c[1] += 1;
		if (c[1] == '"')
			c[1] += 1;
	}
}


//*******************************************************************
//
// GrayImage* convColorToGray(const ColorImage* srcImage)
//
//    Converts color image to grayscale image.
//
GrayImage* convColorToGray(const ColorImage* srcImage)
{
	unsigned numRows = srcImage->getNumRows();
	unsigned numCols = srcImage->getNumCols();

	if (numRows == 0 || numCols == 0) throw InvalidImage();

	GrayImage *newImage = new GrayImage(numRows, numCols);

	for (unsigned r = 0; r < numRows; r++)
		for (unsigned c = 0; c < numCols; c++)
			(*newImage)(r, c) = (*srcImage)(r, c);

	return newImage;
}

//102AT, 103AT
//*******************************************************************
//
// GrayImage* convColorToCyan(const ColorImage* srcImage)
//
//    Converts color image to grayscale image representing the cyan channel of srcImage.
//
GrayImage* convColorToCyan(const ColorImage* srcImage)
{
	unsigned numRows = srcImage->getNumRows();
	unsigned numCols = srcImage->getNumCols();

	if (numRows == 0 || numCols == 0) throw InvalidImage();

	GrayImage *newImage = new GrayImage(numRows, numCols);

	for (unsigned r = 0; r < numRows; r++)
		for (unsigned c = 0; c < numCols; c++)
			(*newImage)(r, c) = ((*srcImage)(r, c)).getCyan();

	return newImage;
}

//*******************************************************************
//
// void convColorToPixmapString(const ColorImage* srcImage, char **&pix, int &rows)
//
//    Purpose is ...
//
void convColorToPixmapString(const ColorImage* srcImage, char **&pix, int &rows)
{
	unsigned numRows = srcImage->getNumRows();
	unsigned numCols = srcImage->getNumCols();

	if (numRows == 0 || numCols == 0) throw InvalidImage();

	// because of the stupid way I indexed the colors with only
	// three letters, an image much bigger than this could cause
	// major problems.
	if (numRows * numCols > 8000)
		throw Error("convColorToPixmapString can't handle images this big.");

	list<ColorPixel> pixels;

	// first, we're going to find all unique colors in the image...
	// this is a very inefficient way to do it, btw.  Should have
	// used a hash table or something  (there's just no hash table
	// in STL right now :[, glib wouldn't like the ColorPixel
	// class, and I don't feel like writing my own implementation
	// right now
	bool found;
	for (unsigned r = 0; r < numRows; r++) {
		for (unsigned c = 0; c < numCols; c++) {
			list<ColorPixel>::iterator it = pixels.begin();

			found = false;
			while (it != pixels.end()) {
				if ((*it) == (*srcImage)(r, c)) {
					found = true;
					break;
				}
				++it;
			}

			if (!found)
				pixels.push_back((*srcImage)(r, c));
		}
	}

	rows = numRows + pixels.size() + 1;
	pix = new char*[rows];

	char topRow[100];
	sprintf(topRow, "%d %d %d 2", numCols, numRows, pixels.size());
	pix[0] = new char[strlen(topRow) + 1];
	strcpy(pix[0], topRow);

	int count = 1;

	// stupid msvc++ doesn't do scoping right, obviously.
	list<ColorPixel>::iterator colorIt = pixels.begin();
	list<string> colorStrings;
		
	string curColor = "!!";
	string triplet;
	
	while (colorIt != pixels.end()) {
		
		pix[count] = new char[curColor.length() + 11];
		triplet = convertTripletToHexString(
				colorIt->getRed(),
				colorIt->getGreen(),
				colorIt->getBlue());

		sprintf(pix[count], "%s c #%s", curColor.c_str(), triplet.c_str());

		colorStrings.push_back(curColor);
		++colorIt;
		count++;
		incrementColorString(curColor);
	}

	string rowString;
	for (unsigned i = 0; i < numRows; i++) {
		rowString = "";

		for (unsigned j = 0; j < numCols; j++) {
			list<ColorPixel>::iterator pIt = pixels.begin();
			list<string>::iterator sIt = colorStrings.begin();

			// shouldn't *really* need a test case here,
			// but...
			while (pIt != pixels.end()) {
				if ((*pIt) == (*srcImage)(i, j)) {
					rowString += (*sIt);
					break;
				}
				++pIt;
				++sIt;
			}	
		}

		pix[count + i] = new char[rowString.length() + 1];
		strcpy(pix[count + i], rowString.c_str());
	}
}


///////////////////////////////////////////////////////////////////////////
//
// This is an experiment to try and create double-sized pixmaps from existing ones
//
// If it works we will go the other way, too.
//
void makeDoubleSizePixmapString(char **pixIn, char **&pixOut, int &rowsOut)
{
	int numRows, numCols, numPixels, bytesPerPixel;


	sscanf(pixIn[0],"%d %d %d %d", &numCols,&numRows,&numPixels,&bytesPerPixel);

	rowsOut = 1 + numPixels + 2 * numRows;
	pixOut = new char*[rowsOut];

	char topRow[100];
	sprintf(topRow, "%d %d %d 2", numCols*2, numRows*2, numPixels);
	pixOut[0] = new char[strlen(topRow) + 1];
	strcpy(pixOut[0], topRow);

	// copy color strings (the colormap)
	for (int count=1; count <=numPixels; count++)
	{
		pixOut[count] = new char[strlen(pixIn[count])+1];
		strcpy(pixOut[count],pixIn[count]);
	}

	// copy the pixel data (doubling # of columns & rows)
	string rowString;
	for (unsigned i = 0; i < numRows; i++) {

		rowString = "";

		for (unsigned j = 0; j < 2 * numCols; j+=2) {

			rowString += pixIn[1+numPixels+i][j];
			rowString += pixIn[1+numPixels+i][j+1];

			rowString += pixIn[1+numPixels+i][j];
			rowString += pixIn[1+numPixels+i][j+1];

		}	

		pixOut[1+numPixels+(i*2)] = new char[2*strlen(pixIn[1+numPixels+i])+1];
		strcpy(pixOut[1+numPixels+(i*2)],rowString.c_str());

		pixOut[1+numPixels+(i*2)+1] = new char[2*strlen(pixIn[1+numPixels+i])+1];
		strcpy(pixOut[1+numPixels+(i*2)+1],rowString.c_str());

	}

	/* for initial debugging - JHS 
	FILE *fp;
	fp = fopen("testPix.txt","w");
	for (int r=0; r<rowsOut; r++)
		fprintf(fp,"%s\n",pixOut[r]);
	fclose(fp);
	exit(99);
	*/
}
