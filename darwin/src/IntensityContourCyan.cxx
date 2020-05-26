//*******************************************************************
//   file: IntensityContour.cxx
//
// author: Scott Hale
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include "IntensityContourCyan.h"
#include <iostream> //for debug purposes
#include "image_processing/transform.h"//used for resizeNN

using namespace std;

//#define DEBUG
/*
 * See IntensityContour::IntensityContour(GrayImage*)
 */
IntensityContourCyan::IntensityContourCyan(ColorImage* img, Contour* ctour,
	int left, int top, int right, int bottom)
	: Contour()
{
	//***1.0LK - original code created memory leak - replaced with following 
	//getPointsFromGrayImage((convColorToGray(img)));
	GrayImage* tempImage = convColorToCyan(img);
	getPointsFromCyanImage(tempImage, ctour, left, top, right, bottom); //***1.96 - added bounds
	delete tempImage;
}

/*
 * Constructs a Contour based on the outline automatically extracted from
 * GrayImage img
 *
 * @param img The image to analyize for a fin outline
 */
 /*IntensityContour::IntensityContour(GrayImage* img) : Contour() {
	 getPointsFromGrayImage(img);
 }*/


GrayImage* IntensityContourCyan::boundImage(GrayImage* img, Contour* ctour,
	int left, int top, int right, int bottom, //***1.96
	int factor, int& xoffset, int& yoffset)
{
	/*
	//***1.96
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



/*private*/
void IntensityContourCyan::getPointsFromCyanImage(GrayImage* imgIn, Contour* ctour,
	int left, int top, int right, int bottom)
{

#ifdef DEBUG
	cout << "IntensityContourCyan::getPointsFromCyanImage(GrayImage* img)" << endl;
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

	//***1.96 - *img is now a LOCAL image pointed to LOCALLY

	// NOTE: img above must be deleted after a cropped copy of it is
	// created below - else mem leak (JHS)

	int xoffset, yoffset;
	GrayImage* tempImg = img; //***1.96
	img = boundImage(tempImg/*img*/, ctour,
		left, top, right, bottom,
		factor, xoffset, yoffset);//offsets are passed back //103AT SAH
	if (factor > 1)
		delete tempImg; //***1.96 - since it is a resized LOCAL copy of *imgIn (JHS)

	//end image resize

////////////////////////////////////////////New Code////////////////////////////////////////////////////////////////////////
#ifdef DEBUG
	img->save("001_cyan.pgm");
#endif
	Range* rng, * tmpRng;
	rng = new Range;
	rng->start = 0;
	tmpRng = new Range;
	tmpRng->start = 0;

	double p = 0;//1 is min
	double previous = 99;//21 is max	
	double diff = 0;//SAH diff previously uninialized.
	double previous_diff;
	int threshold = 0;
	bool increases = false;

	//first increase, p is always > previous (e.g. previous < p)
	//--> p-previous > 0
	while (threshold < 100) {
		previous = p;
		previous_diff = diff;


		rng->end = threshold;
		BinaryImage* cpy = img->doThreshold(rng);

		p = cpy->getPixularity();
		//cout << threshold << ": " << p << endl;

		diff = p - previous;
		//cout << "diff: " << diff << endl;

		threshold += 5;

		delete cpy;//ADD 2006-08-23 -- Just for you JHS

		if (diff < 0) {//if (previous>p) {
#ifdef DEBUG				
			cout << "break1: " << threshold << endl;
#endif
			break;
		}

	}


	//decrease p is always < previous
	//--> p-previous < 0 
	int catch1 = -1, catch2 = -1;
	while (threshold < 100) {
		previous = p;
		previous_diff = diff;

		rng->end = threshold;
		BinaryImage* cpy = img->doThreshold(rng);


		p = cpy->getPixularity();
		//cout << threshold << ": " << p << endl;

		diff = p - previous;
		//cout << "diff: " << diff << endl;

		delete cpy;//ADD 2006-08-23

		if (catch1 == -1 && previous_diff < diff * 1.02) {//if (previous<p) {//TODO try diff*1.02
			catch1 = threshold;
#ifdef DEBUG
			cout << "break2: " << catch1 << endl;
#endif
			//break;
		}
		if (diff > 0) {
			catch2 = threshold;
#ifdef DEBUG
			cout << "break3: " << catch2 << endl;
#endif
			break;
		}

		threshold += 5;
	}


	int ideal_threshold;
	if (catch1 != -1 && catch2 != -1) {
		cout << "catch1-catch2=" << (catch1 - catch2) << endl;
		if (catch2 - catch1 > 30) {
			ideal_threshold = catch1;
		}
		else {
			ideal_threshold = (catch1 + catch2) / 2;
		}
	}
	else if (catch1 != -1) {
		ideal_threshold = catch1;
	}
	else if (catch2 != -1) {
		ideal_threshold = catch2;
	}
	else {
		//TODO: No threshold found
		ideal_threshold = 0;
	}

	if (ideal_threshold == 10) {
		//measurements at 0 and 5 are ineffective because they are edge values. 10 is the first meaningful value. If selected as an end, look deeper or use 0
		ideal_threshold = 0;
	}

#ifdef DEBUG
	cout << "Threshold: " << ideal_threshold << endl;
#endif

	rng->end = ideal_threshold;

	int count = 0;
	threshold += 10;
	while (threshold < 100) {
		previous = p;
		previous_diff = diff;

		tmpRng->end = threshold;
		BinaryImage* cpy = img->doThreshold(tmpRng);


		p = cpy->getPixularity();
		//cout << threshold << ": " << p << endl;

		diff = p - previous;
		//cout << "diff: " << diff << endl;

		delete cpy;//ADD 2006-08-23

		if (previous < p) {
			count++;
			if (count == 2) {
				increases = true;
				break;
			}
		}
		else {
			count = 0;
		}

		threshold += 10;
	}


#ifdef DEBUG
	cout << "IntensityContourCyan: " << rng->start << " - "
		<< rng->end << endl;
#endif

	//use the range to threshold
	BinaryImage* binImg = img->doThreshold(rng);


	//***1.96 - done with the LOCAL img so delete it now
	delete img; //***1.96 - done here to make sure all returns are mem leak free

#ifdef DEBUG   
	binImg->save("002_threshold.pgm");
#endif   


	/* ----------------------------------------------------
	 * Open the image (erode and dialate)
	 * ----------------------------------------------------
	 */



	int iterations = 4; //Default number of iterations
	int ecount = 0, dcount = 1;
	int itcount = 0;


	if (increases) {//if (p<3) {//if pixularity is less than 3, dialate; otherwise, too many distinct objects and we do not want to connect them.
#ifdef DEBUG
		cout << "Dialating first" << endl;
#endif
		for (int i = 0; i < 5; i++)
			binImg->doDialate(4);
	}
	BinaryImage orgImg(binImg);//Copy of orginal

#ifdef DEBUG
	binImg->save("002a_after_dilate.pgm");
#endif	

	for (int i = 0; i < iterations; i++) {
		ecount = binImg->doErode(i % 2);
		dcount = binImg->doDialate(i % 2);
	}

	while (ecount != 0) {//Shrink all other regions with less than 5 neighbor black pixles
		ecount = binImg->doErode(5);
		itcount++;
	}

	/* int iterations = 4; //Default number of iterations
	 int ecount=0, dcount=1;
	 int itcount = 0;
	   int i;

	 for (i = 0; i<iterations; i++) {
		ecount = binImg->doErode(i%2);
	 }
	 while (ecount!=0) {//Shrink all other regions with less than 5 neighbor black pixles
	   ecount = binImg->doErode(5);
	   itcount++;
	 }


	 for (i = 0; i<iterations; i++) {
		dcount = binImg->doDialate(i%2);
	 }*/


	delete rng;
	delete tmpRng;

	/////////////////////////////////////////Same as IntensityContour.//////////////////////////////////////////////////////////
	int r, c;//because it complained about multiple inilizations
#ifdef DEBUG
	binImg->save("002b_morphologicalized.pgm");
#endif
	//AND opended image with orginal
	binImg->doAnd(orgImg);

#ifdef DEBUG
	binImg->save("002c_after_and.pgm");
#endif


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

	if (NULL == largestFeature)
		return;

	binImg = largestFeature->mask;

	//cout << " found one!\n";

	// so now binImg just points to the mask of *largestFeature

	//cout << "Looking for 2nd Fin Candidate: ";

	////INSERT/////
	binImg->doNot();
	// largestFeature is going to be a new Feature, so we must dispose of old one
	Feature* newLargestFeature; //***1.96
	newLargestFeature = binImg->getLargestFeature();
	delete largestFeature; //***1.96
	if (NULL == newLargestFeature) //***1.96 - just in case
		return;
	largestFeature = newLargestFeature; //***1.96
	binImg = largestFeature->mask;//MEMORY LEAK!!!!!!! - hopefully, no more (JHS)
	// agin, binImg now just points to the mask of *largestFeature
	binImg->doNot();
	///////END Insert

	//cout << " found one!\n";

	//Get a one pixel outline by eroding once and XORing with the previous image

	BinaryImage outline(binImg);

	ecount = 0;
#ifdef VCPP6
	for (i = 0; i < 1; i++) // vc++6.0
#else
	for (int i = 0; i < 1; i++) // vc++ 2011
#endif
	{
		ecount = outline.doErode(0);
	}

#ifdef DEBUG
	binImg->save("002_before_outline.pgm");
#endif

	binImg->doXor(outline);

#ifdef DEBUG
	binImg->save("003_outline.pgm");
#endif

	//***1.0LK - a bit of a mess - JHS
	// at this point binImg points to the largestFeature->mask (eroded and XORed).
	// We want to use this binImg to find a new largestFeature, but we cannot
	// delete the current largestFeature until after the call to binImg->getLargestFeature()
	// because binImg will be wiped out by the deletion of the current largestFeature
	// We must delete the current largestFeature at some point OTHEWISE we have a 
	// memory leak.

	//get rid of outlines of inner features (e.g. glare spots) by selecting largest outline

	Feature* oldLargest = largestFeature; //***1.0LK 
	largestFeature = binImg->getLargestFeature();
	delete oldLargest; //***1.0LK 

	if (largestFeature == NULL) {
#ifdef DEBUG
		cout << "largestFeature NULL, aborting. No fin outline determined";
#endif
		//***1.96 - no need to free binImg here since it points to mask in OLD/gone largestFeature
		return;
	}
	// now binImg can be reset to the mask of the NEW largestFeature
	binImg = largestFeature->mask;

#ifdef DEBUG
	binImg->save("004_largest_outline.pgm");
#endif

	///////////////////2008-02-08 rewrite will start here////////////////////////SAH

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
		cout << "\n   No outline intersected the bisector of the secant line formed by user supplied start and end points!\n";
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



#ifdef VCPP6
	i = 0; // vc++6.0
#else
	int i = 0; // vc++2011
#endif
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
	cout << "IntensityContourCyan::getPointsFromCyanImage(GrayImage* img) COMPLETE" << endl;
#endif


	//CHECK FOR MEMORY LEAKS....opps.
	delete largestFeature; //***1.96 - note binImg is just a pointer to the largestFeature->mask

}

