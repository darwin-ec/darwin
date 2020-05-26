//*******************************************************************
//   file: IntensityContour.cxx
//
// author: Scott Hale
//
//   mods: J H Stewman (8/2/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include "IntensityContour.h"
#include <iostream> //for debug purposes
#include "image_processing/transform.h"//used for resizeNN

using namespace std;

//#define DEBUG

/*
 * See IntensityContour::IntensityContour(GrayImage*)
 */
 //Add param Contour* ctour 103AT SAH
IntensityContour::IntensityContour(ColorImage* img, Contour* ctour,
	int left, int top, int right, int bottom)
	: Contour()
{
	//***1.0LK - original code created memory leak - replaced with following 
	//getPointsFromGrayImage((convColorToGray(img)));
	GrayImage* tempImage = convColorToGray(img);
	getPointsFromGrayImage(tempImage, ctour, left, top, right, bottom);//Add param ctour 103AT SAH
	delete tempImage;
}

/*
 * Constructs a Contour based on the outline automatically extracted from
 * GrayImage img
 *
 * @param img The image to analyize for a fin outline
 * @param ctour The contour formed by the user specified start and end points. (Added in 103AT SAH)
 */
IntensityContour::IntensityContour(GrayImage* img, Contour* ctour,
	int left, int top, int right, int bottom)
	: Contour()
{
	getPointsFromGrayImage(img, ctour, left, top, right, bottom);
}

/* 103AT SAH
 * Crop the image using the user specified start and end points
 */
GrayImage* IntensityContour::boundImage(GrayImage* img, Contour* ctour,
	int left, int top, int right, int bottom,
	int factor, int& xoffset, int& yoffset)
{
	/*
	//***1.96 - not needed now, simply crop to indicated bounds

	int rows = img->mRows;
	int cols = img->mCols;

	int top=0;
	int bottom = rows-1;
	int left = 0;
	int right = cols-1;

	int stx =(*ctour)[0].x/factor;
	int sty =(*ctour)[0].y/factor;
	int endx = (*ctour)[1].x/factor;
	int endy = (*ctour)[1].y/factor;

	int var = 100;

	left = _MAX(stx-var,0);
	right = _MIN(endx+var,cols-1);

	bottom = _MAX(sty,endy);
	top = _MIN(sty,endy) - 1.5*abs(stx-endx);
	top = _MAX(top,0);
	*/

	//***1.96 - simply scale the bounds
	left = (int)((float)left / factor);
	top = (int)((float)top / factor);
	right = (int)((float)right / factor);
	bottom = (int)((float)bottom / factor);

	xoffset = left;
	yoffset = top;

	return crop(img, left, top, right, bottom);
}


/*
 * The main algorithm to extract a fin outline from an image.
 * This method is called from all of this class's constructors, which are called in
 * TraceWindow.cxx (on_traceButtonImageOK_clicked).
 *
 * There are 9 stages:
 *
 * 1) Construct, analyize histogram
 * 2) Threshold GrayImage to create BinaryImage
 * 3) Clean up / get a cleanner edge through morphological processes
 * 		a) open (erode, dialate)
 * 		b) erosion with high coefficient to clean up noise, but leave the fin intact
 * 		c) AND with the orginal BinaryImage (#3) to restore the fin shape
 * 4) Feature recognition to select the largest Feature / blob
 * 5) Get a one pixel outline through one erosion and XORing with #4
 * 6) Feature recognition to slect the largest outline (feature /blob) (same code as #4)
 * 7) Find the start point
 * 		A valid start point (p1) must be in math quadrant III
 * 		and be followed by two points (p2,p3)
 * 		such that p1.row>p2.row>p3.row && p1.col<p2.col<p3.col
 * 8) Walk the outline from the starting point, recording a pixel in the contour every 3 pixels
 * 9) Walk ends when no more black pixels may be found. The outline is recouresed backwards until
 * 		a valid end point (p1) is found. A valid end point must be in math quadrant IV
 * 		and be prefaced by two points (p2,p3) such that p1.row>p2.row>p3.row && p1.col>p2.col>p3.col
 *
 * The Contour represented by this object has a length of 0 if at anytime the algorithm
 * 		cannot continue. This often occurs for images of poor contrast.
 *
 * The Contour represented by this object is adjusted with the snake code in TraceWindow.cxx
 * 		if its length is greater than 0.
 *
 */
 /*private*/
void IntensityContour::getPointsFromGrayImage(
	GrayImage* imgIn,
	Contour* ctour,    //Add param ctour 103AT SAH
	int left, int top, int right, int bottom)  //***1.96 - Add cropping bounds JHS
{
#ifdef DEBUG
	cout << "IntensityContour::getPointsFromGrayImage(GrayImage* img)" << endl;
#endif

	//resample image to lower resolution
	//int width = imgIn->getNumCols();
	int width = right - left; //***1.96
	int factor = 1;
	while (width / (float)factor > /*800*/1024) { //***1.96 - changed magic num JHS
		factor *= 2;
	}

	//***1.0LK - memory leak - since img is now pointed at NEW resized 
	//           copy, that copy must be deleted before function return
	GrayImage* img = imgIn;
	if (factor > 1)
		img = resizeNN(imgIn, 100.0 / factor); //***1.96 - *img is now LOCAL image

	// note: the image created above must be deleted AFTER the cropped
	// version is created below, otherwise a mem leak occurs

	int xoffset, yoffset;//103AT SAH
	GrayImage* tempImg = img; //***1.96 - JHS
	//img = boundImage(tempImg/*img*/,ctour,factor,xoffset,yoffset);//offsets are passed back //103AT SAH
	img = boundImage(tempImg/*img*/, ctour,
		left, top, right, bottom,
		factor, xoffset, yoffset);//offsets are passed back //103AT SAH
	if (factor > 1)
		delete tempImg; //***1.96 - since it is a resized LOCAL copy of *imgIn (JHS)

	//end image resize


	//Phase one analyize histogram

	Range* lowestRange, * nextRange;
	int r, c;//because it complained about multiple inilizations

	Histogram<GrayImage>* histo = img->getHistogram();

	lowestRange = histo->findNextValley(0);

	nextRange =
		histo->findNextValley(lowestRange->end);

	int totalPixles = img->getNumRows() * img->getNumCols();
	if (nextRange->tip - lowestRange->end < 25 &&
		nextRange->highestValue > totalPixles / 256) {
		//cout << "Two tone fin" << endl;
		lowestRange->end = nextRange->end;
		//TODO: update other values in strut as needed
	}

	//low contrast check
	if (lowestRange->pixleCount > totalPixles * .7 || lowestRange->end > 150
		|| lowestRange->pixleCount < totalPixles * .2) {
		//exceeds 90% of images
		//cout << "Low Contrast Warning..." << endl;
		//TODO: Do something
	}

	//use the range to threshold
	BinaryImage* binImg = img->doThreshold(lowestRange);

	//***1.96 - done with LOCAL img so delete it here, that way all return
	// paths from function are mem leak free
	delete img; //***1.96
	//***1.96 - also done with these so delete them here
	delete lowestRange;
	delete nextRange;
	delete histo;

	BinaryImage orgImg(binImg);//Copy of orginal

 /* ----------------------------------------------------
  * Open the image (erode and dialate)
  * ----------------------------------------------------
  */

	int iterations = 4; //Default number of iterations
	int ecount = 0, dcount = 1;
	int itcount = 0;
	int i;

	for (i = 0; i < iterations; i++) {
		ecount = binImg->doErode(i % 2);
	}
	while (ecount != 0) {//Shrink all other regions with less than 5 neighbor black pixles
		ecount = binImg->doErode(5);
		itcount++;
	}

	//binImg->save("c:\\users\\adamr\\desktop\\AfterErode.png");
	for (i = 0; i < iterations; i++) {
		dcount = binImg->doDialate(i % 2);
	}
	//binImg->save("c:\\users\\adamr\\desktop\\AfterDilate.png");

	//AND opended image with orginal
	binImg->doAnd(orgImg);

	//binImg->save("c:\\users\\adamr\\desktop\\AfterAnd.png");

	//Feature recognize (This is the longest part of the process)

#ifdef DEBUG
	cout << "Starting feature recognition..." << endl;
#endif

	//cout << "Looking for 1st Fin Candidate: ";

	Feature* largestFeature = binImg->getLargestFeature();

#ifdef DEBUG
	cout << "Feature recognition complete." << endl;
#endif

	delete binImg; //***1.0LK - delete existing binImage before reassigning ptr
	if (NULL == largestFeature)//103AT SAH
		return;//103AT SAH

	//cout << " found one!\n";

	binImg = largestFeature->mask;

	//Get a one pixel outline by eroding once and XORing with the previous image

	BinaryImage outline(binImg);
	binImg->save("c:\\users\\adamr\\desktop\\outline.png");
	ecount = 0;
	for (i = 0; i < 1; i++) {
		ecount = outline.doErode(0);
	}
	outline.save("c:\\users\\adamr\\desktop\\erode.png");

	binImg->doXor(outline);
	binImg->save("c:\\users\\adamr\\desktop\\xor.png");
	//***1.0LK - a bit of a mess - JHS
	// at this point binImg points to the largestFeature->mask (eroded and XORed).
	// We want to use this binImg to find a new largestFeature, but we cannot
	// delete the current largestFeature until after the call to binImg->getLargestFeature()
	// because binImg will be wiped out by the deletion of the current largestFeature
	// We must delete the current largestFeature at some point OTHEWISE we have a 
	// memory leak.

	//get rid of outlines of inner features (e.g. glare spots) by selecting largest outline

	//cout << "Looking for 2nd Fin Candidate: ";

	Feature* oldLargest = largestFeature; //***1.0LK 
	largestFeature = binImg->getLargestFeature();
	delete oldLargest; //***1.0LK 

	if (largestFeature == NULL) {
#ifdef DEBUG
		cout << "largestFeature NULL, aborting. No fin outline determined";
#endif
		return;
	}
	// now binImg can be reset to the mask of the NEW largestFeature
	binImg = largestFeature->mask;

	//cout << " found one!\n";

	//binImg->save("GrayOutline.png");

	int row = 0, col = 0;
	int rows = binImg->getNumRows(), cols = binImg->getNumCols();
	bool done = false;

	width = 15;

	//cout << "Finding first point..." << endl;
	Point* p1 = NULL, * p2 = NULL, * p3 = NULL;
	Contour_point_t pt = (*ctour)[0];
	//Contour::addPoint(pt.x,pt.y);//Add User start
	int stx = pt.x / factor - xoffset;
	int sty = pt.y / factor - yoffset;

	pt = (*ctour)[1];
	int endy = pt.y / factor - yoffset;
	int endx = pt.x / factor - xoffset;



	//find starting point (midx,midy)
	int midx;
	int midy;

	int maxy = (sty > endy) ? sty : endy; //Math.max(sty,endy);
	midx = ((endx + stx) * .5);

	//find black pixel closest to bottom in column midx
	row = maxy;
	while (!done && row >= 0) {
		if ((*binImg)(row, midx).getIntensity() == 0) {
			midy = row;
			done = true;
		}
		else {
			row--;
		}
	}


	if (!done) {
#ifdef DEBUG
		cout << "no starting point found." << endl;
#endif
		cout << "\nNo outline intersected the bisector of the secant line formed by user supplied start and end points!\n";
		return;

#ifdef DEBUG
	}
	else {
		cout << "have starting point (" << midx << "," << midy << ")" << endl;
		cout << "fyi ending point (" << endy << "," << endx << ")" << endl;
#endif
	}

	row = midy;
	col = midx;

	int st2y = row;
	int st2x = col;
	bool returnedOnce = false;

	delete p1;
	p1 = NULL;
	delete p2;
	p2 = NULL;
	delete p3;
	p3 = NULL;

	//walk from point (row,col)
	(*binImg)(row, col).setIntensity(1);


	/* Prioritize direction of movement

			4 | 3 | 2
			-- --  --
			5 | * | 1
			-- --  --
			4 | 3 | 2
	*/



	i = 0;
	bool foundPoint = true;
	bool prepend = false;
	done = false;
	while (!done) {

		/* Prioritize direction of movement

				4 | 3 | 2
				-- --  --
				5 | * | 1
				-- --  --
				4 | 3 | 2
		*/


		if (col + 1 < cols && (*binImg) (row, col + 1).getIntensity() == 0) {//E
			foundPoint = true;
			col = col + 1;
		}
		else if (row - 1 >= 0 && (*binImg) (row - 1, col).getIntensity() == 0) {//N
			foundPoint = true;
			row = row - 1;
		}
		else if (row + 1 < rows && (*binImg) (row + 1, col).getIntensity() == 0) {//S
			foundPoint = true;
			row = row + 1;
		}
		else if (col - 1 >= 0 && (*binImg) (row, col - 1).getIntensity() == 0) {//W
			foundPoint = true;
			col = col - 1;
		}
		else {
			if (prepend) {
				done = true;
				break;
			}
			else {
				prepend = true;
				row = midy;
				col = midx;
				continue;
			}
		}

		if (foundPoint /*&& i%3==0*/) {
			if (prepend) {
				Contour::addPoint(factor * (col + xoffset), factor * (row + yoffset), 0);//prepend
			}
			else {
				Contour::addPoint(factor * (col + xoffset), factor * (row + yoffset));
			}
			(*binImg)(row, col).setIntensity(128);
			if (row == maxy) {
				//done with this direction
				if (prepend) {
					done = true;
					break;
				}
				else {
					prepend = true;
					row = midy;
					col = midx;
					continue;
				}
			}
		}
		i++;
	}//end loop until done




	Contour::trimAndReorder((*ctour)[0], (*ctour)[1]);

#ifdef DEBUG
	cout << "IntensityContour::getPointsFromGrayImage(GrayImage* img) COMPLETE" << endl;
#endif


	delete largestFeature;


}

