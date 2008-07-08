//*******************************************************************
//   file: Contour.h
//
// author: Adam Russell
//
//   mods: J H Stewman (4/12/2005)
//         -- reformatting of code and addition of comment blocks
//
// The Contour class is meant to represent dolphin fin outlines in Darwin.
//
// ***008OL - Notes on new use of Contour CLASS - 3/10/2005
//
// The Contour class is used ONLY in the initial identification
// of the fin outline.  This occurs in the TraceWindow.  The
// Contour points are initialized by the user trace of the fin,
// and are then repositioned (snapped to) the fin outline by
// the active contour (snake) code.  The user may reposition
// Contour points.  The Contour is then processed to remove knots,
// normalize its size, and evenly space the points.  Key features
// are located using the resulting Contour.  A Chain is created
// in this process.
//
// There is NO Contour kept for further use in the system after
// the TraceWindow processes are comlete.
// 
// It is possible that the finalized Contour could be made a part
// of the Outline CLASS which would allow it to passed thru the 
// system for use IF NEEDED.  That is NOT done at this time.
//
// JHS - 3/10/2005
//*******************************************************************

#ifndef CONTOUR_H
#define CONTOUR_H

#include "Error.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef DEBUG
#include <iostream>
#endif


//! A contour point type for use ONLY in Contours.
//
typedef struct {
	int 
		x,                //!< horizontal coordinate within image
		y;                //!< vertical coordinate within image
} Contour_point_t;


//! A node type used in building a Contour linked list.
//
typedef struct _Contour_node_struct{
	Contour_point_t 
		data;             //!< this fin contour point's position within image
	_Contour_node_struct 
		*prev,            //!< pointer to previous point along fin contour
		*next;            //!< pointer to next point along fin contour
} Contour_node_t;


class FloatContour; //***1.4 - externally defined

//*******************************************************************
//
//! A class using a sequence of points to represent fin countours within an image.
//!
//!    ***008OL - Notes on new use of the Contour CLASS - 3/10/2005
//!
//!    This version of a fin contour uses integer coordinates for all coutour
//!    points. The Contour class is used ONLY in the initial identification
//!    of the fin outline.  This occurs in the TraceWindow.  The
//!    Contour points are initialized by the user trace of the fin,
//!    and are then repositioned (snapped to) the fin outline by
//!    the active contour (snake) code.  The user may reposition
//!    Contour points.  The Contour is then processed to remove knots,
//!    normalize its size, and evenly space the points.  Key features
//!    are located using the resulting Contour.  A Chain is created
//!    in this process.
//!
//!    There is NO Contour kept for further use in the system after
//!    the TraceWindow processes are complete.
//! 
//!    It is possible that the finalized Contour could be made a part
//!    of the Outline CLASS which would allow it to passed thru the 
//!    system for use IF NEEDED.  That is NOT done at this time.
//
class Contour 
{
	public:

		//  Contour()
		//
		//! The class constructor.
		//!
		//!    Creates an empty Contour.
		//
		Contour();


		//  Contour(Contour* contour)
		//
		//! A class copy constructor.
		//!
		//!    Creates a Contour that is a COPY of the Contour pointed to by contour.
		//!
		//! @param contour points to an existing Contour object.
		//
		Contour(Contour* contour);


		//  Contour(Contour &c)
		//
		//! A class copy constructor.
		//!
		//!    Creates a Contour that is a COPY of the Contour c.
		//!
		//! @param c is an existing Contour object.
		//
		Contour(Contour &c);


		//  Contour(FloatContour *fc)
		//
		//! A class copy constructor (of sorts).
		//!
		//!    This creates a sequence of points with integer coordinates from
		//!    the provided sequence of points with floating point coordinates.
		//!
		//! @param fc points to an existing FloatContour object.
		//
		Contour(FloatContour *fc); //***1.4 - creates int copy of float contour


		//  ~Contour()
		//
		//! The class destructor.
		//!
		//!    This function frees all memory involved in the list of
		//!    contour points.
		//
		~Contour();

		//  void clear()
		//
		//! Clears Contour of all points.
		//!
		//!    This function frees all memory involved in the list of
		//!    contour points and resets members to indicate EMPTY state.
		//
		void Contour::clear();


		//! Pops the specified number of points from the front of this Contour.
		//!
		//! @param numPops indicates the number of points to be popped from the
		//!    front of the point list.
		//!
		//! @pre 0 < numPops <= this->length(), but the enforcement takes place within
		//!    the RemovePoint() function called from here.
		//
		void popFront(unsigned numPops); //005CT


		//! Pops the specified number of points from the end of this Contour.
		//!
		//! @param numPops indicates the number of points to be popped from the
		//!    end of the point list.
		//!
		//! @pre 0 < numPops <= this->length(), but the enforcement takes place within
		//!    the RemovePoint() function called from here.
		//
		void popTail(unsigned numPops); //***1.96a


		//  Contour& operator=(Contour &c)
		//
		//! The Assignment Operator for Contour objects.
		//!
		//!    This returns a reference to a Contour object that is a COPY of the
		//!    Contour c.
		//!
		//! @param c is an existing Contour object.
		//
		Contour& operator=(Contour &c);
		

		//! A class for throwing an error when a Contour point index is out of bounds.
		//
		class BoundsViolation : public Error {
			public:
				BoundsViolation() : Error("Attempt to access element outside Contour bounds.") { }
		};


		///////////// Functions to add a single point to Contour ///////////////


		//  void addPoint(int x, int y)
		//
		//! Appends a point to the end of this Contour.
		//!
		//!    This function creates a new contour point, appends it to the end of the
		//!    linked-list and increments the Contour::mNumPoints counter.
		//!
		//! @param x is the horizontal image coordinate of the point to be inserted.
		//! @param y is the vertical image coordinate of the point to be inserted.
		//!
		//! @post The point (x,y) has been appended to this Contour.
		//! @post The attribute Contour::mNumPoints has been incremented.
		//
		void addPoint(int x, int y);
		

		//  void addPoint(int x, int y, unsigned pos)
		//
		//! Inserts point into the point sequence for this Contour at position pos.
		//!
		//!    This function creates a new contour point, inserts it at the specified
		//!    position in the linked-list, increments the Contour::mNumPoints counter, and
		//!    resets both Contour::mPrevAccessNum and Contour::mCurrent to the beginning
		//!    of the point sequence..
		//!
		//! @param x is the horizontal image coordinate of the point to be inserted.
		//! @param y is the vertical image coordinate of the point to be inserted.
		//! @param pos is the index at which the point(x,y) will be inserted 
		//!    within this Contour point sequence.
		//!
		//! @pre 0 <= @em pos <= @em mNumPoints
		//!
		//! @post The point(x,y) has been appended to this Contour.
		//! @post The attribute Contour::mNumPoints has been incremented.
		//! @post The attribute Contour::mPrevAccess has been set to 0.
		//! @post The attribute Contour::mCurrent points to the first point in the sequence.
		//
		void addPoint(int x, int y, unsigned pos);

		
		//  int addPointInOrder(int x, int y)
		//
		//! Appends the point (x,y) at the @em best position in the sequence.
		//!
		//!    This function creates a new contour point and inserts it into the existing
		//!    Contour point sequence in the @em best position.  @em Best
		//!    is defined as the position that places the new point @em between or
		//!    @em closest to a pair of existing Contour points. The point (x,y) is
		//!    then inserted by a call to addPoint(x,y,pos).
		//!
		//! @param x is the horizontal image coordinate of the point to be inserted.
		//! @param y is the vertical image coordinate of the point to be inserted.
		//!
		//! @return The index position of the point insertion within the sequence.
		//!
		//! @post The point(x,y) has been inserted into this Contour.
		//!
		//! @sa findPositionOfClosestPoint()
		//! @sa addPoint(int x, int y, unsigned pos)
		//
		int addPointInOrder(int x, int y);


		//////////// Functions to remove single points from Contour ///////////


		// bool removePoint(int x, int y);
		//
		//! Removes point (x,y) from this Contour.
		//
		bool removePoint(int x, int y);
		
		
		//  bool removePoint(unsigned pos);
		//
		//! Removes the point at the indicated position within this Contour.
		//!
		//!    This function removes the point at position @em pos from the 
		//!    linked-list.
		//!
		//! @param pos is the index of the point to be removed.
		//!
		//! @return true if point removed, false otherwise
		//!
		//! @pre 0 <= @em pos < mNumPoints
		//
		bool removePoint(unsigned pos);


		////// Functions to return Min and Max point coordinate values ////////


		int maxX(); //!< Returns maximum X coordinate value of any Contour point.
		int maxY(); //!< Returns maximum Y coordinate value of any Contour point.
		int minX(); //!< Returns minimum X coordinate value of any Contour point.
		int minY(); //!< Returns minimum Y coordinate value of any Contour point.


		//! Returns the number of points in this Contour.
		unsigned length() const;


		//! Returns distance covered traversing Contour from end to end
		double totalDistanceAlongContour() const;


		//! Returns position of Contour point closest to (x,y)
		bool findPositionOfClosestPoint(int x, int y, int &position) const;


		//! Returns reference to Contour point at position pos 
		Contour_point_t& operator[](unsigned pos);
		

		Contour* createScaledContour(float scaleFactor, int xoffset, int yoffset);


		//! Computes new Contour points with approximately the specified spacing  
		Contour* evenlySpaceContourPoints(double space);


		//! Removes knots (loop-backs and severe zig-zags) from Contour
		void removeKnots(double spacing); //005CT


		//! Returns a pointer to the begining of the Contour
		Contour_node_t *getHead() {return mHead;}  //005CT


		//  bool trimAndReorder(Contour &c, Contour_point_t start, Contour_point_t end)
		//  
		//! Trims excess points off Contour and reorders if necessary.
		//
		//!    This Contour is trimmed in the following manner.  The closest point
		//!    to start is found and the closest point to end is found.  All
		//!    points in this Contour following max(closest2StartPos,closest2EndPos) 
		//!    are removed.  All points ahead of min(closest2StartPos,closest2EndPos) 
		//!    are removed.  Then if closest2EndPos < closest2StartPos, the Contour
		//!    is reversed.
		// 
		bool trimAndReorder(Contour_point_t startPt, Contour_point_t endPt);

#ifdef DEBUG
		//! Outputs contour point coordinates to console
		void print() const;
#endif


	protected:

		unsigned 
			mNumPoints;      //!< The number of points along this Contour.

		Contour_node_t 
			*mHead,          //!< A pointer to the first Contour point.
			*mTail,          //!< A pointer to the last Contour point.
			*mCurrent;       //!< A pointer to the current Contour point.

		unsigned 
			mPrevAccessNum;  //!< The index of the previously accessed Contour point


	private:

		//  bool removePoint(Contour_node_t *p)
		//
		//! Removes "pointed to" point from this Contour.
		//!
		//!    Removes the point from this Contour that is pointed to by p.
		//!
		//! @param p Points to the Contour point that is to be removed from the
		//!    linked-list implementation of the point sequence.
		//!
		//! @pre @em p must point to an actual point node within he point sequence
		//!    for this Contour.
		//
		bool removePoint(Contour_node_t *p); //***008OL
};

#endif
