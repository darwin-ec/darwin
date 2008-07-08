//*******************************************************************
//   file: Contour.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (4/12/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include "Contour.h"
#include "FloatContour.h" //***1.4
#include "utility.h"
#include "constants.h"

#include <iostream> // temporary - JHS

using namespace std;


#define MAX_TURN_ANGLE 150

//*******************************************************************
//
// Contour::Contour()
//
//    Default CONSTRUCTOR - creates EMPTY contour.
//
Contour::Contour()
	: mNumPoints(0),
	  mHead(NULL),
	  mTail(NULL),
	  mCurrent(NULL),
	  mPrevAccessNum(0)
{ }


//*******************************************************************
//
// Contour::Contour(Contour *c)
//
//    COPY CONSTRUCTOR -- Makes complete copy of "pointed to" contour.
//
Contour::Contour(Contour* contour)
	: mNumPoints(0),
	  mHead(NULL),
	  mTail(NULL),
	  mCurrent(NULL),
	  mPrevAccessNum(0)
{
	if (NULL == contour)
		return;

	unsigned numPoints = contour->length();

	for (unsigned i = 0; i < numPoints; i++)
		addPoint((*contour)[i].x, (*contour)[i].y);
}


//*******************************************************************
//
// Contour::Contour(Contour &c)
//
//    COPY CONSTRUCTOR -- Makes complete copy of "referenced" contour.
//
//    Question: Shouldn't parameter be const Contour &c ???
//
Contour::Contour(Contour &c)
	: mNumPoints(0),
	  mHead(NULL),
	  mTail(NULL),
	  mCurrent(NULL),
	  mPrevAccessNum(0)
{
	unsigned numPoints = c.length();

	for (unsigned i = 0; i < numPoints; i++)
		addPoint(c[i].x, c[i].y);
}

//*******************************************************************
//
// Contour::Contour(FloatContour *fc)
//
//    Makes complete copy of "pointed to" floating point contour.
//
Contour::Contour(FloatContour* fc)
	: mNumPoints(0),
	  mHead(NULL),
	  mTail(NULL),
	  mCurrent(NULL),
	  mPrevAccessNum(0)
{
	if (NULL == fc)
		return;

	unsigned numPoints = fc->length();

	for (unsigned i = 0; i < numPoints; i++)
		addPoint(round((*fc)[i].x), round((*fc)[i].y));
}

//*******************************************************************
//
// Contour::~Contour()
//
//    DESTRUCTOR - frees all dynamically allocated nodes in contour list.
//
Contour::~Contour()
{
	Contour_node_t *temp = mHead;

	while (temp != NULL) {
		mHead = temp->next;
		delete temp;
		temp = mHead;
	}
}

//*******************************************************************
//
// void Contour::clear()
//
//    Clears Contour of all points.
//
void Contour::clear()
{
	Contour_node_t *temp = mHead;

	while (temp != NULL) {
		mHead = temp->next;
		delete temp;
		temp = mHead;
	}

	mNumPoints = 0;
	mHead = NULL;
	mTail = NULL;
	mCurrent = NULL;
	mPrevAccessNum = 0;
}


//*******************************************************************
//
// Contour& Contour::operator=(Contour &c)
//
//    Overloaded ASSIGNMENT operator
//
Contour& Contour::operator=(Contour &c)
{
	if (this == &c)
		return *this;

	mHead = NULL;
	mTail = NULL;
	mNumPoints = 0;
	mCurrent = NULL;
	mPrevAccessNum = 0;

	unsigned numPoints = c.length();

	for (unsigned i = 0; i < numPoints; i++)
		addPoint(c[i].x, c[i].y);

	return *this;
}


//*******************************************************************
//
// void Contour::addPoint(int x, int y)
//
//    Appends point (x,y) at END of contour list
//
void Contour::addPoint(int x, int y)
{
	if (NULL == mHead) {
		mHead = new Contour_node_t;
		mHead->data.x = x;
		mHead->data.y = y;
		mHead->prev = NULL;
		mHead->next = NULL;
		mTail = mHead;
		mCurrent = mHead;
	} else {
		Contour_node_t *newNode = new Contour_node_t;

		newNode->data.x = x;
		newNode->data.y = y;
		newNode->next = NULL;
		newNode->prev = mTail;
		mTail->next = newNode;
		mTail = newNode;
	}

	mNumPoints++;
}


//*******************************************************************
//
// void Contour::addPoint(int x, int y, unsigned pos)
//
//    Adds the point (x,y) as the specified contour point.  That is,
//    the point is inserted in position pos in the list.
//
//    PRE:  0 <= pos<= mNumPoints
//
void Contour::addPoint(int x, int y, unsigned pos)
{
        if (mNumPoints <= 0 || mNumPoints < pos)
                throw BoundsViolation();

        Contour_node_t *temp = mHead;

        for (unsigned i = 0; i < pos; i++)
                temp = temp->next;

        Contour_node_t *newNode = new Contour_node_t;

        newNode->data.x = x;
       	newNode->data.y = y;

	if (temp == mHead) {
		newNode->prev = NULL;
		newNode->next = mHead;
		mHead->prev = newNode;
		mHead = newNode;
	} else if (temp == NULL) {
		newNode->prev = mTail;
		newNode->next = NULL;
		mTail->next = newNode;
		mTail = newNode;
	} else {
      		newNode->next = temp;
        	newNode->prev = temp->prev;
		temp->prev->next = newNode;
		temp->prev = newNode;
	}

        mCurrent = mHead;
        mPrevAccessNum = 0;
        mNumPoints++;
}


//*******************************************************************
//
// int Contour::addPointInOrder(int x, int y)
//
//    Adds the point (x,y) to the contour by finding the correct location
//    between existing contour points whcin minimizes loop-backs or zig-zags.
//
int Contour::addPointInOrder(int x, int y)
{
	if (1 >= mNumPoints)
		this->addPoint(x, y);

	int position;

	if (!findPositionOfClosestPoint(x, y, position))
		return -1;

        // vector: pos->x,y
        double newVecX = x - (*this)[position].x;
        double newVecY = y - (*this)[position].y;
        double magNew = sqrt(newVecX*newVecX + newVecY*newVecY);

	if (0 == position) {
		double nextVecX = (*this)[1].x - (*this)[0].x;
		double nextVecY = (*this)[1].y - (*this)[0].y;
		double nextAngle = acos((nextVecX*newVecX + nextVecY*newVecY)/
		                        (sqrt(nextVecX*nextVecX + nextVecY*nextVecY)*magNew));

		if ((nextAngle < PIOVER2) && (nextAngle > -PIOVER2))
			this->addPoint(x, y, 1);
		else
			this->addPoint(x, y, 0);

	} else if (((int)mNumPoints - 1) == position) {

		double nextVecX = (*this)[position-1].x - (*this)[position].x;
		double nextVecY = (*this)[position-1].y - (*this)[position].y;
		double nextAngle = acos((nextVecX*newVecX + nextVecY*newVecY)/
		                        (sqrt(nextVecX*nextVecX + nextVecY*nextVecY)*magNew));

		//if (abs((int)rtod(nextAngle)) < 90) add at position
                //else add at position + 1
		if (!((nextAngle < PIOVER2) && (nextAngle > -PIOVER2)))
			position++;
                this->addPoint(x, y, position);
	} else {
 		// decide if ray from new point to pos is more like pos+1->pos or new->pos+1
		// to determine if point should be inserted before or after pos.
                // compare bearing of pos->x,y with bearing of pos->pos+1
	        double nextVecX = (*this)[position + 1].x - (*this)[position].x;
	        double nextVecY = (*this)[position + 1].y - (*this)[position].y;
                double nextAngle = acos((nextVecX*newVecX + nextVecY*newVecY)/
                                        (sqrt(nextVecX*nextVecX + nextVecY*nextVecY)*magNew));
                // compare bearing of x,y->pos with bearing of x,y->pos+1
	        double prevVecX = (*this)[position + 1].x - x;
	        double prevVecY = (*this)[position + 1].y - y;
                double prevAngle = acos((prevVecX*(-newVecX) + prevVecY*(-newVecY))/
                                        (sqrt(prevVecX*prevVecX + prevVecY*prevVecY)*magNew));

		//if (nextAngle > prevAngle) add at position
                //else add at position + 1
		if (nextAngle <= prevAngle)
                   position++;
		this->addPoint(x, y, position);
	}
	return position;
}


//*******************************************************************
//
// bool Contour::removePoint(int x, int y)
//
//    Searches the contour for the point with the specified coordinates
//    (x,y) and removes it from the list.
//
bool Contour::removePoint(int x, int y)
{
	Contour_node_t *temp = mHead;

	for (unsigned i = 0; i < mNumPoints; i++) {
		if (temp->data.x == x && temp->data.y == y) {
			if (NULL != temp->prev)
				temp->prev->next = temp->next;
			if (NULL != temp->next)
				temp->next->prev = temp->prev;

			if (temp == mTail)
				mTail = temp->prev;
			if (temp == mHead)
				mHead = temp->next;

			delete temp;

			//mCurrent = mHead; ***1.4TW - moved below 
			mPrevAccessNum = 0;
			mNumPoints--;

			//***1.4TW
			if (mNumPoints == 0)
				mHead = NULL;
			mCurrent = mHead;

			return true;
		}
		else
			temp = temp->next;
	}

	return false;
}


//*******************************************************************
//
// void Contour::popFront(unsigned numPops)
//
//    The specified number of points is removed from the front of
//    the contour.
//
void Contour::popFront(unsigned numPops)
{
	unsigned i;

	for(i = 0; i<numPops;  i++)
		removePoint((unsigned)0); //***008OL added cast
}

//*******************************************************************
//
// void Contour::popTail(unsigned numPops)
//
//    The specified number of points is removed from the end of
//    the contour.
//
void Contour::popTail(unsigned numPops) //***1.96a - new
{
	unsigned i;

	for(i = 0; i<numPops;  i++)
		removePoint(mTail); 
}


//*******************************************************************
//
// bool Contour::removePoint(unsigned pos)
//
//    Removes the contour point at the specified index (pos).  The contour
//    list is traversed linearly to locate the specified point.
//
//    PRE:  0 <= pos < mNumPoints
//
bool Contour::removePoint(unsigned pos)
{
	Contour_node_t *temp = mHead;

	if (mNumPoints <= pos) //***008OL corrected < to <=
		throw BoundsViolation();

	for (unsigned i = 0; i < pos; i++)
		temp = temp->next;

	if (NULL != temp->prev)
		temp->prev->next = temp->next;
	if (NULL != temp->next)
		temp->next->prev = temp->prev;

	if (temp == mTail)
		mTail = temp->prev;
	if (temp == mHead)
		mHead = temp->next;

	delete temp;

	mCurrent = mHead;
	mPrevAccessNum = 0;
	mNumPoints--;

	return true;
}


//*******************************************************************
//
// bool Contour::removePoint(Contour_node_t *temp)
//
//    ***008OL new private function to remove point thru "temp" pointer
//    thus avoiding repeated traversal of list when removePoint(pos)
//    wis called from removeKnots()
//
bool Contour::removePoint(Contour_node_t *temp)
{
	if (NULL != temp->prev)
		temp->prev->next = temp->next;
	if (NULL != temp->next)
		temp->next->prev = temp->prev;

	if (temp == mTail)
		mTail = temp->prev;
	if (temp == mHead)
		mHead = temp->next;
	
	delete temp;

	mCurrent = mHead;
	mPrevAccessNum = 0;
	mNumPoints--;

	return true;
}


//*******************************************************************
//
// int Contour::maxX()
//
//    Traverses the entire contour and returns the MAXIMUM X value found. 
//
int Contour::maxX()
{
	if (NULL == mHead)
		return 0;

	int max = mHead->data.x;
	Contour_node_t *temp = mHead->next;

	while (temp != NULL) {
		if (temp->data.x > max)
			max = temp->data.x;

		temp = temp->next;
	}

	return max;
}


//*******************************************************************
//
// int Contour::minX()
//
//    Traverses the entire contour and returns the MINIMUM X value found. 
//
int Contour::minX()
{
	if (NULL == mHead)
		return 0;

	int min = mHead->data.x;
	Contour_node_t *temp = mHead->next;

	while (temp != NULL) {
		if (temp->data.x < min)
			min = temp->data.x;

		temp = temp->next;
	}

	return min;
}



//*******************************************************************
//
// int Contour::maxY()
//
//    Traverses the entire contour and returns the MAXIMUM Y value found. 
//
int Contour::maxY()
{
	if (NULL == mHead)
		return 0;

	int max = mHead->data.y;
	Contour_node_t *temp = mHead->next;

	while (temp != NULL) {
		if (temp->data.y > max)
			max = temp->data.y;

		temp = temp->next;
	}

	return max;
}


//*******************************************************************
//
// int Contour::minY()
//
//    Traverses the entire contour and returns the MINIMUM Y value found.
//
int Contour::minY()
{
	if (NULL == mHead)
		return 0;

	int min = mHead->data.y;
	Contour_node_t *temp = mHead->next;

	while (temp != NULL) {
		if (temp->data.y < min)
			min = temp->data.y;

		temp = temp->next;
	}

	return min;
}



//*******************************************************************
//
// unsigned Contour::length() const
//
//    Returns number of points in the contour list.
//
unsigned Contour::length() const
{
	return mNumPoints;
}


//*******************************************************************
//
// double Contour::totalDistanceAlongContour() const
//
//    Returns total distance within the image that would be covered while
//    walking the contour from end to end.  The entire contour is traversed 
//    each time this function is called.
//
//    PRE:  The contour is NOT empty.
//
double Contour::totalDistanceAlongContour() const
{
	if (NULL == mHead)
		return 0;

	double totalDistance = 0;
	Contour_node_t *temp = mHead->next;

	while (temp != NULL) {
		totalDistance += distance(temp->data.x, temp->data.y,
		                          temp->prev->data.x, temp->prev->data.y);
		temp = temp->next;
	}

        return totalDistance;
}


//*******************************************************************
//
// Contour_point_t& Contour::operator[](unsigned pos)
//
//    Returns contour point at specified index (or position).
//    Function traverses linear (doubly) linked list to find position.
//    Traversal begins at current point (one last accessed).
//
//    PRE:  0 <= pos < mNumPoints
//
Contour_point_t& Contour::operator[](unsigned pos)
{
	if (pos >= mNumPoints)
		throw BoundsViolation();

	if (mPrevAccessNum == pos)
		return mCurrent->data;

	if (pos > mPrevAccessNum) {
		unsigned numNodes = pos - mPrevAccessNum;

		do {
			mCurrent = mCurrent->next;
			numNodes--;
		} while (numNodes > 0);

		mPrevAccessNum = pos;
		return mCurrent->data;
	}

	unsigned numNodes = mPrevAccessNum - pos;

	do {
		mCurrent = mCurrent->prev;
		numNodes--;
	} while (numNodes > 0);

	mPrevAccessNum = pos;
	return mCurrent->data;
}

//*******************************************************************
//
// Contour* createScaledContour(Contour* contour, int scale)
//
//    Returns a pointer to a new Contour which has been scaled to zoomed out image  
//
//    PRE:  It is assumed that (*this) Contour is already normalized prior to 
//    calling this function
//
//    ***new function by KRD 08/16/05 for multiscaled snap-to in TraceWindow
//
Contour* Contour::createScaledContour(float scaleFactor, int xoffset, int yoffset)
{
	int x, y;
	unsigned i;

	if (mNumPoints > 2){
		Contour *newContour = new Contour();

		for (i=0; i<mNumPoints; i++){
		// first point of new Contour is first point of old contour
			x = (int)round(((*this)[i].x) *scaleFactor)+xoffset;
			y = (int)round(((*this)[i].y) *scaleFactor)+yoffset;
			newContour->addPoint(x, y); //005CT
		}
		return newContour;
	}
	else
		return new Contour(this);
}


//*******************************************************************
//
// Contour* Contour::evenlySpaceContourPoints(double space)
//
//    Returns a pointer to a new Contour which has evenly spaced points.  
//
//    PRE:  It is assumed that (*this) Contour is already normalized prior to 
//    calling this function
//
//    ***006CM - new version of function by JHS 5/1/2004
//    corrects erroneous handling of contour foldbacks in excess of
//    150 degrees
//
Contour* Contour::evenlySpaceContourPoints(double space)
{
	int x, y, ccx, ccy, tx, ty, vx, vy, vsx, vsy, qx, qy;
	unsigned i, done;                                          //005CT
	double a, b, c, sqrt_b2_4ac, t1, t2, t, tLen;              //005CT
	double curBearing, prevBearing;                            //005CT
	//double changeInAngle;

	// cc= (ccx,ccy) is the center point of the current circle with radius == space
	// vs= (vsx,vsy) is the beginning of the edge segment being intersected with the circle
	// p = (x,y) is the end of the vector being intersected with the circle
	//
	// there are three cases for intersections
	//
	// (1) vs == cc and dist(cc,p) > space
	//     compute Q = point on edge (cc,p) at dist space from cc
	// (2) dist(cc,p) == space
	//     Q is simply p
	// (3) dist(cc,p) < space
	//     set vs = p
	//     set p = next point in original contour
	// (4) vs != cc and dist(cc,p) > space
	//     this means that dist(vs,p) < space (from previous tests)
	//     find Q = point on edge(vs,p) at dist space from cc
	//
	// in all cases the edge (cc,Q) is the next edge in the new contour
	// and Q will be added to the new contour, cc will be moved to Q
	// vs will be moved to Q and and in case (2) p will be moved to the next p
	// ONLY IF the edge bearing is less than 150 degrees change from previous
	// edge bearing
	//
	// if the bearing change is 150 degrees or more, the edge (cc,Q) is skipped
	// and the following occurs
	//
	// case (1) increment index i (moves point p only, leaving vs == cc)
	// case (2) increment index i (moves point p only, leaving vs == cc)
	// case (4) vs is moved back to cc
	//

	if (mNumPoints > 2){
		// point cc (circle center & previous point added to new contour)
		ccx = (*this)[0].x;
		ccy = (*this)[0].y;
		// point vs (start of edge being tested for intersection with circle)
		vsx = (*this)[0].x;
		vsy = (*this)[0].y;
		// point p (end point of edge being tested for circle intersection)
		x = (*this)[1].x;
		y = (*this)[1].y;
		// index of p
		i = 1;
		// will be true when all original contour points have been used
		done = 0;

		Contour *newContour = new Contour();

		// first point of new Contour is first point of old contour
		newContour->addPoint(ccx, ccy); //005CT

		// bearing into first point should be approximately this
		prevBearing = (-3.14159 * 0.25); // -45 degrees

		while (!done){

			// vector from vs to p
			vx = x - vsx;
			vy = y - vsy;
			// vector from cc to p
			tx = x - ccx;
			ty = y - ccy;

			//005CT - new thru line 461
			tLen = sqrt((double)(tx * tx + ty * ty));    // dist(cc,p)

			if (tLen > space){     // then evaluate intersection
				if ((ccx == vsx) && (ccy == vsy))
				{
					// case (1) Q is on edge(cc,p) at dist space from cc
					//***1.3 - added round() function - truncation was causing points to
					//         drift away from long line segments
					qx = (int)round((space / tLen) * x + (1.0 - (space / tLen)) * ccx);
					qy = (int)round((space / tLen) * y + (1.0 - (space / tLen)) * ccy);

					// check that wrapping is not occurring
					curBearing =  atan2((double)(qy - ccy), (double)(qx - ccx));

					/*
					/////////////////////// begin new /////////////////////
					//***1.1ER - correct for limits (+/- pi) of atan2
					// fixing this is required to prevent pulling edge out of 
					// deep downward turned notches, but creates problem
					// with notch finding -- fix in next version
					*/
					double changeInAngle = fabs(rtod(prevBearing - curBearing));
					if (changeInAngle > 180)
						changeInAngle = 360 - changeInAngle;

					if (changeInAngle <= 150)
					/////////////////////// end new /////////////////////
					

					/////////////////////// begin old //////////////////
					//if (fabs(rtod(prevBearing - curBearing)) <= MAX_TURN_ANGLE) //***1.4 - was 150
					////////////////////////end old /////////////////////
					{
						// add Q to new contour and move forward
						newContour->addPoint(qx, qy);
						//cout << "x=" << qx << ":" << "y=" << qy << ":" << distance(qx,qy,ccx,ccy)
						//	   << " from previous point" << endl;
						prevBearing = curBearing;
						ccx = qx;
						ccy = qy;
						vsx = qx;
						vsy = qy;
					}
					else // change in bearing is excessive
					{
						// skip this point Q and move p forward
						// next time we calculate edge(cc,p) intersection with circle
						i++;
						if (i < mNumPoints)
						{
							vsx = ccx;
							vsy = ccy;
							x = (*this)[i].x;
							y = (*this)[i].y;
						}
						else
							done = 1;
					}
				}
				else  // cc != vs
				{
					// case (4) Q is on edge(vs,p) at dist space from cc
					tx = vsx - ccx;
					ty = vsy - ccy;
					a = vx * vx + vy * vy;
					b = 2 * (vx * tx + vy * ty);    // 2 * dot(v, t)
					c = tx * tx + ty * ty - space * space;

					if (b * b - 4 * a * c < 0.0)
						std::cout << "Neg Radical"; // this should NEVER happen
					else
					{
						sqrt_b2_4ac = sqrt(b * b - 4 * a * c);
						t1 = (-b + sqrt_b2_4ac)/(2 * a);
						t2 = (-b - sqrt_b2_4ac)/(2 * a);
						if ((t1 >= 0) && (t1 <= 1))
							t = t1;
						else if ((t2 >= 0) && (t2 <= 1))
							t = t2;
						else
							t = 0; // this should NEVER happen, either

						qx = (int)round(vsx + (t * vx));
						qy = (int)round(vsy + (t * vy));
					}

					// check that wrapping is not occurring
					curBearing =  atan2((double)(qy - ccy), (double)(qx - ccx));

					/*
					//////////////////////// begin new ///////////////////
					//***1.1ER - correct for limits (+/- pi) of atan2
					// fixing this is required to prevent pulling edge out of 
					// deep downward turned notches, but creates problem
					// with notch finding -- fix in next version
					//
					changeInAngle = fabs(rtod(prevBearing - curBearing));
					if (changeInAngle > 180)
						changeInAngle = 360 - changeInAngle;

					if (changeInAngle <= 150)
					//////////////////////// end new ////////////////////
					*/

					//////////////////////// begin old //////////////////
					if (fabs(rtod(prevBearing - curBearing)) <= MAX_TURN_ANGLE) //***1.4 - was 150
					/////////////////////// end old /////////////////////
					{
						// add Q to new countour and move cc forward
						newContour->addPoint(qx, qy);
						//cout << "x=" << qx << ":" << "y=" << qy << ":" << distance(qx,qy,ccx,ccy)
						//	   << " from previous point" << endl;
						prevBearing = curBearing;
						ccx = qx;
						ccy = qy;
						vsx = qx;
						vsy = qy;
					}
					else // change in angle is excessive
					{
						// skip this point Q, move vs back to cc
						// next time we calculate edge(cc,p) intersection with circle
						vsx = ccx;
						vsy = ccy;
					}
				}
			}
			else if (tLen < space)
			{
				// case(3) move vs and p forward
				i++;
				if (i < mNumPoints)
				{
					vsx = x;
					vsy = y;
					x = (*this)[i].x;
					y = (*this)[i].y;
				}
				else
					done = 1;
			}
			else // tLen == space
			{
				// case(2) point Q is simply point p
				qx = x;
				qy = y;

				// check that wrapping is not occurring
				curBearing =  atan2((double)(qy - ccy), (double)(qx - ccx));
					
				/*
				/////////////////// begin new //////////////////////
				//***1.1ER - correct for limits (+/- pi) of atan2
				// fixing this is required to prevent pulling edge out of 
				// deep downward turned notches, but creates problem
				// with notch finding -- fix in next version
				//
				changeInAngle = fabs(rtod(prevBearing - curBearing));
				if (changeInAngle > 180)
					changeInAngle = 360 - changeInAngle;

				if (changeInAngle <= 150)
				/////////////////// end new /////////////////////
				*/

				/////////////////////// begin old /////////////////////
				if (fabs(rtod(prevBearing - curBearing)) <= MAX_TURN_ANGLE) //***1.4 - was 150
				////////////////////// end old ///////////////////////
				{
					// add Q to new countour and move cc forward
					newContour->addPoint(qx, qy);
					//cout << "x=" << qx << ":" << "y=" << qy << ":" << distance(qx,qy,ccx,ccy)
					//	   << " from previous point" << endl;
					prevBearing = curBearing;
					ccx = qx;
					ccy = qy;
					vsx = qx;
					vsy = qy;
				}
				else // change in angle is excessive
				{
					// skip this point Q, move vs back to cc (if needed) and move p forward
					// next time we calculate edge(cc,p) intersection with circle
					vsx = ccx;
					vsy = ccy;
					i++;
					if (i < mNumPoints)
					{
						x = (*this)[i].x;
						y = (*this)[i].y;
					}
					else
						done = 1;
				}
			}

		} // while (!done)

		// debugging code -- JHS
		for (int k=1; k<newContour->mNumPoints; k++)
		{
			double dx = (*newContour)[k-1].x - (*newContour)[k].x;
			double dy = (*newContour)[k-1].y - (*newContour)[k].y;
			double dist = sqrt(dx*dx+dy*dy);
			//if ((dist < space-0.2) || (space+0.2 < dist))
			//	std::cout << '|' << dist << '|';
			//else
			//	std::cout << '.';
		}

		return newContour;
	}
	else
		return new Contour(this);
}


//*******************************************************************
//
// void Contour::print() const
//
//    Outputs the coordinates of all contour points to the console.
//
#ifdef DEBUG
void Contour::print() const
{
	Contour_node_t *temp = mHead;

	int count = 0;
	while (temp != NULL) {
		cout << count++ << " "<< temp->data.x << " "
		     << temp->data.y << endl;
		temp = temp->next;
	}
}
#endif


//*******************************************************************
//
// bool Contour::findPositionOfClosestPoint(int x, int y, int &position) const
//
//    Sets position to the index of the contour point "closest to" (x,y)
//    and returns true.  If there is no contour, the function returns false.
//
bool Contour::findPositionOfClosestPoint(int x, int y, int &position) const
{
	if (NULL == mHead)
		return false;

	double lowestDistance = distance(x, y, mHead->data.x, mHead->data.y);
	position = 0;

	Contour_node_t *temp = mHead->next;

	unsigned counter = 1;

	while (temp != NULL) {
		double curDistance = distance(x, y, temp->data.x, temp->data.y);

		if (curDistance < lowestDistance) {
			lowestDistance = curDistance;
			position = counter;
		}

		temp = temp->next;
		counter++;
	}
	return true;
}


//*******************************************************************
//
// void Contour::removeKnots(double spacing)
//
//    Removes extra points along any loops (knots) that have formed in the 
//    Contour.
//
//    ***006CM - revised so that the algorithm does NOT go back to
//    beginning of list each time a point is removed -- JHS 4/28/2004
//
void Contour::removeKnots(double spacing)
{
	// first remove all points which are closer than spacing
	double space = spacing - 1;

	Contour_node_t *temp = mHead;

	if (temp == NULL)
		return; // if no points, we're done

	// skip first pt, then need next pt to compute distance
	int count = 1; // index of next point (first one considered for removal)
	// temp always points to the previous point
	while (temp->next != NULL) {
		if (distance(temp->data.x, temp->data.y,
					 temp->next->data.x, temp->next->data.y) < space){
			this->removePoint(temp->next); //***008OL
		}
		else
		{
			// spacing is adequate so move to next point
			temp = temp->next;
			count++;
		}
	}

	temp = mHead;
	if (temp->next == NULL)
		return; // if only one point, we're done

	double curBearing, prevBearing;
	// first angle
	prevBearing = (-3.14159 * 0.25); // -45 degrees

	count = 1; // first point to consider for removal

	// only delete one bad angle point from the trace at a time
	while (temp->next != NULL) {
		curBearing = atan2((double)(temp->next->data.y - temp->data.y),
		                   (double)(temp->next->data.x - temp->data.x));

		// if angle from *temp to *temp->next doubles back,
		// then remove *temp->next

		/*
		///////////////////////////// begin new //////////////////////
		//***1.1ER - fix problem related to downward opening notches
		// this function AND evenlySpaceContourPoints() currently remove
		// all points that would allow contour to trend to left and up past
		// the horizontal - this occurs because of the limits of atan2()
		// which are +/- pi
		// in the next software version this needs to be fixed, but there 
		// is a related problem, once contours are allowed to have angles
		// up and left (absolute angles in range -180 .. -90, that is
		// the NOTCH finding process, finds the upper lip of the notch
		// not the notch itself
		// that relates to the use of the wavelet code and the absolute
		// chains which can then have notch angular changes of, say -350 when
		// the correct angular change is +10, and the notch max/min finding
		// thus selects the HUGE entry angle change rather than the
		// more modest interior change
		//
		*/
		double changeInAngle = fabs(rtod(prevBearing - curBearing));
		// since atan2 returns -pi .. +pi, we must correct if bearing goes from + to -
		if (changeInAngle > 180)
			changeInAngle = 360 - changeInAngle;
		if (changeInAngle > 150)
		/////////////////////////// end new //////////////////////////
		

		/*
		////////////////////// begin old //////////////////////////
		double changeInAngle = rtod(prevBearing - curBearing);
		if (fabs(changeInAngle) > MAX_TURN_ANGLE) //***1.4 - was 150
		////////////////////// end old ////////////////////////////
		*/
			this->removePoint(temp->next); //***008OL
		else
		{
			// angle is ok, so move on
			prevBearing = curBearing;
			temp = temp->next;
			count++;
		}
	}
}

//  bool trimAndReorder(Contour &c, Contour_point_t start, Contour_point_t end)
//  
// Trims excess points off Contour and reorders if necessary.
//
//    This Contour is trimmed in the following manner.  The closest point
//    to start is found and the closest point to end is found.  All
//    points in this Contour following max(closest2StartPos,closest2EndPos) 
//    are removed.  All points ahead of min(closest2StartPos,closest2EndPos) 
//    are removed.  Then if closest2EndPos < closest2StartPos, the Contour
//    is reversed.  Finally, the startPt and endPts are placed at their
//    respective ends of this Contour.
// 
bool Contour::trimAndReorder(Contour_point_t startPt, Contour_point_t endPt)
{
	int closest2StartPos, closest2EndPos;
	
	if (! this->findPositionOfClosestPoint(startPt.x, startPt.y, closest2StartPos) ||
		! this->findPositionOfClosestPoint(endPt.x, endPt.y, closest2EndPos))
		return false;

	bool reverseIt;
	int firstIndex, secondIndex;
	if (closest2StartPos <= closest2EndPos)
	{
		reverseIt = false;
		firstIndex = closest2StartPos;
		secondIndex = closest2EndPos;
	}
	else
	{
		reverseIt = true;
		firstIndex = closest2EndPos;
		secondIndex = closest2StartPos;
	}

	// trim extra points from Contour

	popTail(length() - secondIndex - 1); // remove all points following secondIndex position
	popFront(firstIndex);                // remove all points preceding firstIndex position

	if (reverseIt)
	{
		// reverse the point order in this Contour

		Contour c(this); // COPY this Contour
		clear();         // then clear out this Contour

		Contour_node_t *ptr = c.getHead(); // get ptr to first point in COPY of Contour
		while (NULL != ptr)
		{
			if (length() == 0)
				addPoint(ptr->data.x, ptr->data.y); // prepend pt to NEW this Contour
			else
				addPoint(ptr->data.x, ptr->data.y, 0); // prepend pt to NEW this Contour
			c.popFront(1);                         // pop first point from COPY
			ptr = c.getHead();                     // get ptr to next point in COPY
		}
	}

	// prepend start Pt and append end Pt to this Contour
	addPoint(startPt.x, startPt.y, 0);
	addPoint(endPt.x, endPt.y);

	return true;
}


