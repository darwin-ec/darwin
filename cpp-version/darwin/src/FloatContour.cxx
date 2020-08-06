//*******************************************************************
//   file: FloatContour.cxx
//
// author: 
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

//***006FC added function comment blocks
#include <cmath>
#include "Error.h"
#include "FloatContour.h"
#include "feature.h" //***005FC
#include "utility.h"
#include <iostream>  // temporary - JHS

//***005FC new define below
#define ROUND(X) (int)(X+0.5)

using namespace std;

//********************************************************************
// FloatContour()
//
FloatContour::FloatContour() :  //***006FC revised
	mPointVector()
{
}

//********************************************************************
// FloatContour()
//
FloatContour::FloatContour(const FloatContour& fc) :  //***006FC new
	mPointVector(fc.mPointVector)
{
}

//********************************************************************
// ~FloatContour()
//
FloatContour::~FloatContour()
{
  // just to be nice
  mPointVector.clear();
}

//********************************************************************
// operator=()
//
FloatContour& FloatContour::operator=(FloatContour& fc) { //***006FC new

	if (this == &fc)
		return *this;

	// empty this mPointVector
	mPointVector.erase(mPointVector.begin(),mPointVector.end());

	// copy items over from fc.mPointVector
	vector<point_t>::iterator it;
	it = fc.mPointVector.begin();
	while (it != fc.mPointVector.end())
		mPointVector.push_back(*it);

	return *this;
}


//********************************************************************
//
// bool Contour::findPositionOfClosestPoint(float x, float y, int &position) const
//
//    Sets position to the index of the contour point "closest to" (x,y)
//    and returns true.  If there is no contour, the function returns false.
//
bool FloatContour::findPositionOfClosestPoint(float x, float y, int &position) const
{
	if (length() == 0)
		return false;

	double lowestDistance = distance(x, y, mPointVector[0].x, mPointVector[0].y);
	position = 0;

	for (int i=1; i<length(); i++) {

		double curDistance = distance(x, y, mPointVector[i].x, mPointVector[i].y);

		if (curDistance < lowestDistance) {
			lowestDistance = curDistance;
			position = i;
		}
	}
	return true;
}

//********************************************************************
// length()
//
int FloatContour::length() const {

  return mPointVector.size();

}

//***005FC next 5 functions are new
//********************************************************************
// popFront()
// -- Pops *numPops elements from the front of the vector
//
void FloatContour::popFront(unsigned numPops) {

  unsigned i;

  vector<point_t>::iterator it = mPointVector.begin(), it2 = mPointVector.begin(); //***006FC
  for(i = 0; i<numPops;  i++)
		it2++;
  mPointVector.erase(it,it2);
}

/***008OL old version removed
** new version IF EVER CREATED will use distance from first to 
** middle point without trying to find tip first
** currently, the normalizing is done at the Contour stage
/////////////////////////////////////////////////////////////////////
// normalizeContour()
// -- Rescales the distance between LE & Tip to 2000
//
void FloatContour::normalizeContour() {

  Chain *temp = new Chain(this,3.0);
  int tipPos = findTip(temp);
  int beginLE = findLECutoff(temp,tipPos);
  point_t tipPosPoint = temp->getSavedPoint(tipPos),
    beginLEPoint = temp->getSavedPoint(beginLE);
  delete temp;

  // Get X,Y Position of tip and LE
  float tipx = tipPosPoint.x , tipy = tipPosPoint.y;
  float LEx = beginLEPoint.x , LEy  = beginLEPoint.y;

  // Calculate distance and rescaling factor
  float distance, factor;
  distance = sqrt((tipx - LEx) * (tipx - LEx)
    + ((tipy - LEy) * (tipy - LEy)));
  factor = (float) 2000.0/distance;
  int length = mPointVector.size(), i;
  for( i = 0; i< length ; i++) {
    mPointVector[i].x *= factor;
    mPointVector[i].y *= factor;
  }
}
*/


//********************************************************************
// FindIndexFromChainPoint()
// -- Given an <X,Y> Location from the chain, returns the closest
//    index into the Contour
//
int FloatContour::FindIndexFromChainPoint(float desx, float desy) {

  float x = 0.0 , y = 0.0;
  double diff = 0.0 , ldiff = 0.0 ;
  int len = length();
  int i = 0;
  int lowest=0;
  vector<point_t>::iterator it = mPointVector.begin(); //***006FC
  for( i = 0; i < len ; i++, it++ ) {
    x = it->x;
    y = it->y;
    diff = sqrt((( desx - x ) * (desx - x))
      + (( desy - y ) * (desy - y)));
    if( i == 0 )
      ldiff = diff;
    else if( diff < ldiff) {
      ldiff = diff;
      lowest = i;
    }
  }

  return lowest;
}

//********************************************************************
// ContourToFloatContour()
// -- Convert a Contour into *this* FloatContour
//
void FloatContour::ContourToFloatContour(Contour *c){
  int length = c->length(), i ;
  float x, y;

  //***008OL remove any existing mPointVector
  if (this->length() > 0)
    mPointVector.clear();

  Contour_node_t *cur= c->getHead();

  for( i = 0; i < length ; i++ ) {
    x = (float)cur->data.x;
    y = (float)cur->data.y;
    addPoint( x , y );
    cur = cur->next; // BAD use of public node structure in Contour
  }
}

//********************************************************************
// FloatContourToContour()
// -- Return a pointer to a Contour converted from a FloatContour
//
Contour* FloatContour::FloatContourToContour(FloatContour *fc) {

  Contour* c=new Contour;
  int length= fc->length(), i, x, y;

  for(i=0; i<length; i++){

    x= ROUND(mPointVector[i].x);
    y= ROUND(mPointVector[i].y);
    c->addPoint(x, y);
  }

  return c;
  }

//***005FC end of new function listings

//********************************************************************
// addPoint()
//
void FloatContour::addPoint(float x, float y) {

  point_t p; //***006FC
  p.x = x;
  p.y = y;

  mPointVector.push_back(p);
}

//********************************************************************
// operator[]()
//
point_t& FloatContour::operator[](int pos) { //***006FC changed return type

  if (pos < 0 || pos >= (int)mPointVector.size())
    throw BoundsError("FloatContour::operator[]");

  return mPointVector[pos];
}

//********************************************************************
// const operator[]()
//
const point_t& FloatContour::operator[](int pos) const { //***006FC changed return type

  if (pos < 0 || pos >= (int)mPointVector.size())
    throw BoundsError("FloatContour::operator[]");

  return mPointVector[pos];
}

//********************************************************************
// minX()
//
float FloatContour::minX() {

	if (FloatContour::length() <= 0)
			return 0.0;

	float minx = mPointVector[0].x;

	for (int i = 1; i < FloatContour::length(); i++) {

			if (mPointVector[i].x < minx)
					minx = mPointVector[i].x;
	}

	return minx;
}

//********************************************************************
// minY()
//
float FloatContour::minY() {

	if (FloatContour::length() <= 0)
			return 0.0;

	float miny = mPointVector[0].y;

	for (int i = 1; i < FloatContour::length(); i++) {

			if (mPointVector[i].y < miny)
					miny = mPointVector[i].y;
	}

	return miny;
}

//********************************************************************
// maxX()
//
float FloatContour::maxX() {

	if (FloatContour::length() <= 0)
			return 0.0;

	float maxx = mPointVector[0].x;

	for (int i = 1; i < FloatContour::length(); i++) {

			if (mPointVector[i].x > maxx)
					maxx = mPointVector[i].x;
	}

	return maxx;
}

//********************************************************************
// maxY()
//
float FloatContour::maxY() {

	if (FloatContour::length() <= 0)
			return 0.0;

	float maxy = mPointVector[0].y;

	for (int i = 1; i < FloatContour::length(); i++) {

			if (mPointVector[i].y > maxy)
					maxy = mPointVector[i].y;
	}

	return maxy;
}

//********************************************************************
// evenlySpaceContourPoints()
//
FloatContour* FloatContour::evenlySpaceContourPoints(int space) {

  if (space <= 0)
    return NULL;

  float x, y, ccx, ccy, tx, ty, vx, vy, vsx, vsy;
  unsigned i;
  double a, b, c, sqrt_b2_4ac, t1, t2, t, tLen;

  ccx = (*this)[0].x;
  ccy = (*this)[0].y;
  i = 0;
  bool done = false;
  FloatContour *newContour = new FloatContour();

  while (!done){          // Contour length >= 2

    vsx = (*this)[i].x;
    vsy = (*this)[i].y;
    x = (*this)[i+1].x;
    y = (*this)[i+1].y;
    vx = x - vsx;
    vy = y - vsy;
    tx = x - ccx;
    ty = y - ccy;
    tLen = sqrt(tx * tx + ty * ty);
		if (tLen > space){    // then evaluate intersection

      tx = vsx - ccx;
      ty = vsy - ccy;
      a = vx * vx + vy * vy;
      b = 2 * (vx * tx + vy * ty);    // 2 * dot(v, t)
      c = tx * tx + ty * ty - space * space;
      sqrt_b2_4ac = sqrt(b * b - 4 * a * c);

			if (sqrt_b2_4ac < 0.0)  // should never happen - JHS
				std::cout << "Neg Rad in FloatContour::evenlySpaceContourPoints()\n";

      t1 = (-b + sqrt_b2_4ac)/(2 * a);
      t2 = (-b - sqrt_b2_4ac)/(2 * a);
      if ((t1 >= 0) && (t1 <= 1))
        t = t1;
      else if ((t2 >= 0) && (t2 <= 1))
        t = t2;
      else
        t = 0;
			// update circle ctr for next go
      ccx = vsx + t * vx;
      ccy = vsy + t * vy;
      newContour->addPoint(ccx, ccy);
    }
    else
			i++;
    if ((int)i >= this->length() - 1)
      done = true;
  }

  return newContour;
}

//***006FC next function moved from header

//********************************************************************
// print()
//
void FloatContour::print(){
	   
	int j = mPointVector.size(),i;
	for (i=0; i< j;i++)
		std::cout << "X:" << mPointVector[i].x
                  << " Y: " <<mPointVector[i].y
                  << std::endl;
}

