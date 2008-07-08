//*******************************************************************
//   file: AreaMatch.cxx
//
// author: J H Stewman (2007 - addition to DARWIN version 1.85)
//
//   mods: 
//
// computes the error between two fin outlines by finding the
// area between the interlaced contours after mapping
//
//*******************************************************************

#include "AreaMatch.h"

using namespace std;


//*******************************************************************
//
//
double polygonArea(const vector<point_t> &p)
{
	double sum = 0.0;

	int n = p.size();

	for (int i = 0; i < n; i++)
	{
		int j = (i + 1) % n;
		sum += (p[i].x * p[j].y - p[j].x * p[i].y);
	}

	double area = fabs(0.5 * sum);
	//cout << "Area: " << area << endl;

	return area;
}

//*******************************************************************
//
//
void intersect(vector<contour_segment> &segments, 
			   FloatContour *c1, // unknown contour
			   FloatContour *c2, // database contour
			   int dbId, int unkId, 
			   double &s1, double &t1, point_t &p1,  // db param, point coords & unk param
			   double &s2, point_t &p2,       // db param, point coords
			   int &n)
{
	double
		dx1, dy1,
		dx2, dy2;
	point_t 
		A,B, // begin and end of database segment
		C,D; // begin and end of unknown segment
	int 
		idA, idB, // indices of A & B
		idC, idD; // indices of C & D

	idA = segments[dbId].startIndex;
	if (! segments[dbId].reversed)
		idB = idA + 1;
	else
		idB = idA - 1;

	idC = segments[unkId].startIndex;
	if (! segments[unkId].reversed)
		idD = idC + 1;
	else
		idD = idC - 1;

	A = (*c2)[idA];
	B = (*c2)[idB];
	C = (*c1)[idC];
	D = (*c1)[idD];

	dx1 = B.x - A.x;
	dy1 = B.y - A.y;
	dx2 = D.x - C.x;
	dy2 = D.y - C.y;

	double 
		beta1, // parameter on database segment
		beta2; // parameter on unknown segment

	double denom = dx1 * dy2 - dy1 * dx2;
	if (denom != 0.0) // thre is a unique intersection between the lines
	{
		beta1 = ((A.y - C.y) * dx2 - (A.x - C.x) * dy2) / denom; // on database
		if (dx2 != 0.0)
			beta2 = ((A.x - C.x) + beta1 * dx1) / dx2; // on unknown
		else
			beta2 = ((A.y - C.y) + beta1 * dy1) / dy2; // on unknown

		if ((0.0 <= beta1) && (beta1 < 1.0) && (0.0 <= beta2) && (beta2 < 1.0))
		{
			// found single point of intersection
			p1.x = A.x + beta1 * dx1;
			p1.y = A.y + beta1 * dy1;
			n = 1;
			if (! segments[dbId].reversed)
				s1 = idA + beta1;
			else
				s1 = idB - beta1;
			if (! segments[unkId].reversed)
				t1 = idC + beta2;
			else
				t1 = idD - beta2;

			// mark interection so we won't repeat intersection test later
			segments[dbId].intersectingSegs.insert(unkId);
		}
		else 
			n = 0; // no intersection within both segments
	}
	else // might have segment overlap
	{
		double det = C.x * (A.y - B.y) - A.x * (C.y - B.y) + B.x * (C.y - A.y);
		if (det == 0.0) // point C is on line containing A * B
		{
			// find out range of parameter overlap on database segment
			if (dx1 != 0.0)
			{
				beta1 = (C.x - A.x) / dx1;
				beta2 = (D.x - A.x) / dx1;
			}
			else if (dy1 != 0.0)
			{
				beta1 = (C.y - A.y) / dy1;
				beta2 = (D.y - A.y) / dy1;
			}
			else
			{
				cout << "ERROR in match calc: zero length segment in contour\n";
				n = 0;
				return;
			}

			if (((beta1 > 1.0) && (beta2 > 1.0)) || ((beta1 < 0.0) && (beta2 < 0.0)))
			{
				// no segment overlap
				n = 0;
				return;
			}

			// entire database segment may be within unknown segment
			// if so, then pull in the boundaries
			if (beta1 > 1.0)
				beta1 = 1.0;
			else if (beta1 < 0.0)
				beta1 = 0.0;

			if (beta2 > 1.0)
				beta2 = 1.0;
			else if (beta2 < 0.0)
				beta2 = 0.0;

			p1.x = A.x + beta1 * dx1;
			p1.y = A.y + beta1 * dy1;
			p2.x = A.x + beta2 * dx1;
			p2.y = A.y + beta2 * dy1;
			n = 2;
			if (! segments[dbId].reversed)
			{
				s1 = idA + beta1;
				s2 = idA + beta2;
			}
			else
			{
				s1 = idB - beta1;
				s2 = idB - beta2;
			}
			
			// mark interection so we won't repeat intersection test later
			segments[dbId].intersectingSegs.insert(unkId);

			// find parameter of first point along unknown, since it marks the end of
			// some polygon at the beginning of the overlap zone
			double u1;
			if (dx2 != 0.0)
				u1 = (p1.x - C.x) / dx1; // on unknown
			else
				u1 = (p1.y - C.y) / dy1; // on unknown
			if (! segments[unkId].reversed)
				t1 = idC + u1;
			else
				t1 = idD - u1;
		}
		else
			n = 0; // no intersection
	}
}

//*******************************************************************
//
// this version uses a multimap (balanced search tree) and is O(nlgn)
//
void findSegsAndRefPointsForContour (		
		FloatContour *c1, // part of some contour (DB or UNK)
		int begin1,
		int end1,
		char conType, // 'd' or 'u'
		vector<contour_segment> &segments,
		multimap<double, ref_point> &refPts
		)
{
	// this runs down the contour and appends segments and refPts to existing lists
	// called by areaErro function

	double 
		dxL = (*c1)[end1].x - (*c1)[begin1].x, 
		dyL = (*c1)[end1].y - (*c1)[begin1].y,
		dxLsq = dxL * dxL,
		dyLsq = dyL * dyL,
		denom = dxLsq + dyLsq,
		xB = (*c1)[begin1].x,
		yB = (*c1)[begin1].y,
		xE = (*c1)[end1].x,
		yE = (*c1)[end1].y;

	//vector<contour_segment> segments;
	//multimap<double, ref_point> refPts;
	// project each unknown fin contour point onto reference line

	// first point

	contour_segment seg;
	ref_point pt;
	multimap<double, ref_point>::iterator ppIt;
	vector<contour_segment>::iterator it;

	bool seg_reversed, seg_perpendicular;

	// create segment record and append to vector

	seg.contourType = conType; // unknown or database
	seg.parameter = 0.0;   // first point on reference line
	seg.reversed = false;
	seg.startIndex = begin1;
	segments.push_back(seg);

	//cout << "Param: " << seg.parameter << endl;

	// create reference point record with segment index & add to active list indicators
	// and then insert reference point into multipmap in order by reference line parameter

	pt.parameter = 0.0;
	pt.segId = segments.size() - 1;
	pt.segOp = 0;       // indicate "add" op
	ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap

	ref_point rp = (*ppIt).second;

	// all middle points
	for (int i = begin1+1; i < end1; i++)
	{
		double s = (((*c1)[i].x - xB)*dxL + ((*c1)[i].y - yB)*dyL) / denom;
		// this is the end of previous segment
		it = segments.end();
		it--; // back up to the actual last element
		seg_reversed = (s < (*it).parameter);
		seg_perpendicular = (s == (*it).parameter);
		if (seg_reversed)
		{
			// the segment is a reversed segment, so correct info in previous segment
			(*it).reversed = true;
			(*it).parameter = s;
			(*it).startIndex = i;
		}

		// create reference point record with existing segment index & remove from active 
		// list indicators and then insert reference point into multipmap in order by 
		// reference line parameter

		pt.parameter = s;
		pt.segId = segments.size() - 1;
		if (seg_reversed)
		{
			// segment runs in reverse direction so ...
			(*ppIt).second.segOp = 1;  // previous endpoint is where segment is "removed" 
			pt.segOp = 0;       // and this 2nd endpoint is where segment is "added"
			ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
		}
		else if (seg_perpendicular)
			(*ppIt).second.segOp = 2;       // indicate immediate processing (seg perp to ref line)
		else
		{
			pt.segOp = 1;       // indicate "remove" op
			ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
		}

		// it is also the beginning of a new segment
		seg.contourType = conType; // unknown or database
		seg.parameter = s;
		seg.reversed = false;
		seg.startIndex = i;
		segments.push_back(seg);

		//cout << "Param: " << seg.parameter << endl;

		// create reference point record with segment index & add to active list indicators
		// and then insert reference point into multipmap in order by reference line parameter
		pt.parameter = s;
		pt.segId = segments.size() - 1;
		pt.segOp = 0;       // indicate "add" op
		ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap

		rp  = ppIt->second;

	}

	// last point (does NOT begin any new segment -- just ends last one)
	//seg.contourType = 'u'; // unknown
	//seg.parameter = 1.0;   // last point on reference line
	//seg.reversed = false;
	//seg.startIndex = begin1;
	//segments.push_back(seg);

	//cout << "Param: " << 1.0 << endl;

	// create reference point record with existing segment index & remove from active 
	// list indicators and then insert reference point into multipmap in order by 
	// reference line parameter

	it = segments.end();
	it--; // back up to the actual last element
	pt.parameter = 1.0;
	pt.segId = segments.size() - 1;
	if (1.0 < (*it).parameter)
	{
		// segment runs in reverse direction so ...
		(*ppIt).second.segOp = 1;  // previous endpoint is where segment is "removed" 
		pt.segOp = 0;       // and this 2nd endpoint is where segmetn is "added"
		ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
	}
	else if ((1.0 == (*it).parameter))
		(*ppIt).second.segOp = 2;       // indicate immediate processing (seg perp to ref line)
	else
	{
		pt.segOp = 1;       // indicate "remove" op
		ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
	}

	//multimap<double,ref_point>::iterator mit;

	//for (mit = refPts.begin(); mit != refPts.end(); mit++)
		//cout << "ref: " << (*mit).first << " " 
		//     << (*mit).second.segId << " " << (*mit).second.segOp << endl;


}

//*******************************************************************
//
// this version uses a vector witha simple insertion sort and is 
// closer to O(n) than O(n^2) because the intersections along the
// reference line are almost in order as found
//
void findSegsAndRefPointsForContour_Vec (		
		FloatContour *c1, // part of some contour (DB or UNK)
		int begin1,
		int end1,
		char conType, // 'd' or 'u'
		vector<contour_segment> &segments,
		vector<ref_point> &refPts
		)
{
	// this runs down the contour and appends segments and refPts to existing lists
	// called by areaErro function

	double 
		dxL = (*c1)[end1].x - (*c1)[begin1].x, 
		dyL = (*c1)[end1].y - (*c1)[begin1].y,
		dxLsq = dxL * dxL,
		dyLsq = dyL * dyL,
		denom = dxLsq + dyLsq,
		xB = (*c1)[begin1].x,
		yB = (*c1)[begin1].y,
		xE = (*c1)[end1].x,
		yE = (*c1)[end1].y;

	//vector<contour_segment> segments;
	//multimap<double, ref_point> refPts;
	// project each unknown fin contour point onto reference line

	// first point

	contour_segment seg;
	ref_point pt;
	vector<ref_point>::iterator ppIt,ppItPrev;
	vector<contour_segment>::iterator it;

	bool seg_reversed, seg_perpendicular;

	// create segment record and append to vector

	seg.contourType = conType; // unknown or database
	seg.parameter = 0.0;   // first point on reference line
	seg.reversed = false;
	seg.startIndex = begin1;
	segments.push_back(seg);

	if (segments.size() == 0)
		cout << "Segment push failed" << endl;

	//cout << "Param: " << seg.parameter << endl;

	// create reference point record with segment index & add to active list indicators
	// and then insert reference point into multipmap in order by reference line parameter

	pt.parameter = 0.0;
	pt.segId = segments.size() - 1;
	pt.segOp = 0;       // indicate "add" op

	// first point -- NO insertion sort done here
	refPts.push_back(pt);     // insert pt into multimap
	ppItPrev = refPts.begin();

	ref_point rp = (*ppItPrev);

	// all middle points
	for (int i = begin1+1; i < end1; i++)
	{
		double s = (((*c1)[i].x - xB)*dxL + ((*c1)[i].y - yB)*dyL) / denom;
		// this is the end of previous segment
		it = segments.end();
		it--; // back up to the actual last element
		seg_reversed = (s < (*it).parameter);
		seg_perpendicular = (s == (*it).parameter);
		if (seg_reversed)
		{
			// the segment is a reversed segment, so correct info in previous segment
			(*it).reversed = true;
			(*it).parameter = s;
			(*it).startIndex = i;
		}

		// create reference point record with existing segment index & remove from active 
		// list indicators and then insert reference point into multipmap in order by 
		// reference line parameter

		pt.parameter = s;
		pt.segId = segments.size() - 1;
		if (seg_reversed)
		{
			// segment runs in reverse direction so ...
			(*ppItPrev).segOp = 1;  // previous endpoint is where segment is "removed" 
			pt.segOp = 0;       // and this 2nd endpoint is where segment is "added"
			//ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
			// a small insertion sort done here
			ppIt = refPts.end();
			ppIt--;
			while ((ppIt != refPts.end()) && ((*ppIt).parameter > pt.parameter))
				ppIt--;
			ppIt++;
			ppItPrev = refPts.insert(ppIt,pt);     // insert pt into multimap
		}
		else if (seg_perpendicular)
			(*ppItPrev).segOp = 2;       // indicate immediate processing (seg perp to ref line)
		else
		{
			pt.segOp = 1;       // indicate "remove" op
			//ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
			// a small insertion sort done here
			ppIt = refPts.end();
			ppIt--;
			while ((ppIt != refPts.end()) && ((*ppIt).parameter > pt.parameter))
				ppIt--;
			ppIt++;
			ppItPrev = refPts.insert(ppIt,pt);     // insert pt into multimap
		}

		// it is also the beginning of a new segment
		seg.contourType = conType; // unknown or database
		seg.parameter = s;
		seg.reversed = false;
		seg.startIndex = i;
		segments.push_back(seg);

		//cout << "Param: " << seg.parameter << endl;

		// create reference point record with segment index & add to active list indicators
		// and then insert reference point into multipmap in order by reference line parameter
		pt.parameter = s;
		pt.segId = segments.size() - 1;
		pt.segOp = 0;       // indicate "add" op
		//ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
		// a small insertion sort done here
		ppIt = refPts.end();
		ppIt--;
		while ((ppIt != refPts.end()) && ((*ppIt).parameter > pt.parameter))
			ppIt--;
		ppIt++;
		ppItPrev = refPts.insert(ppIt,pt);     // insert pt into multimap

		rp  = (*ppItPrev);

	}

	// last point (does NOT begin any new segment -- just ends last one)
	//seg.contourType = 'u'; // unknown
	//seg.parameter = 1.0;   // last point on reference line
	//seg.reversed = false;
	//seg.startIndex = begin1;
	//segments.push_back(seg);

	//cout << "Param: " << 1.0 << endl;

	// create reference point record with existing segment index & remove from active 
	// list indicators and then insert reference point into multipmap in order by 
	// reference line parameter

	it = segments.end();
	it--; // back up to the actual last element
	pt.parameter = 1.0;
	pt.segId = segments.size() - 1;
	if (1.0 < (*it).parameter)
	{
		// segment runs in reverse direction so ...
		(*ppItPrev).segOp = 1;  // previous endpoint is where segment is "removed" 
		pt.segOp = 0;       // and this 2nd endpoint is where segmetn is "added"
		//ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
		// a small insertion sort done here
		ppIt = refPts.end();
		ppIt--;
		while ((ppIt != refPts.end()) && ((*ppIt).parameter > pt.parameter))
			ppIt--;
		ppIt++;
		ppItPrev = refPts.insert(ppIt,pt);     // insert pt into multimap
	}
	else if ((1.0 == (*it).parameter))
		(*ppItPrev).segOp = 2;       // indicate immediate processing (seg perp to ref line)
	else
	{
		pt.segOp = 1;       // indicate "remove" op
		//ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
		// a small insertion sort done here
		ppIt = refPts.end();
		ppIt--;
		while ((ppIt != refPts.end()) && ((*ppIt).parameter > pt.parameter))
			ppIt--;
		ppIt++;
		ppItPrev = refPts.insert(ppIt,pt);     // insert pt into multimap
	}

	//multimap<double,ref_point>::iterator mit;

	//for (mit = refPts.begin(); mit != refPts.end(); mit++)
		//cout << "ref: " << (*mit).first << " " 
		//     << (*mit).second.segId << " " << (*mit).second.segOp << endl;


}

//******************************************************************
//
// 4/23/2007 - JHS
//
// new approach using standard triangle onto which are mapped the
// endpoints and then intersection (and touch) points between database
// and unknown contours.  The idea is to limit or eliminate the searching
// of previous and subsequent segments when determining intersections
//
// create two reference lines
//   L1 - start to tip
//   L2 - tip to end
// traverse the database contour, for each point p(i)
//   compute the point q(i) at which the perpendicular from L1 or L2
//   through q(i) also contains p(i)
// sort the points by parameter value along L1 or L2
//   (note: this can be done quickly using an insertion sort from the
//    trailing end as the points are added -- in almost all cases the new
//    point's parameter will follow the last point -- maybe use an STL list
//    and use rfind() followed by insert or use a priority que
//    with the parameter as the key)
// repeat previous two steps for unknown contour
//
// What is saved for each contour segment is this ...
//   if this is segment's 1st endpt
//     create segment record containing ...
//       type of contour (DB,UNK)
//       parameter along reference line for perpendicular through contour point
//       index of segment endpoint within contour
//     insert segment into list ordered by refence line parameter of 1st endpt
//     keep pointer to segment (prevSeg)
//   else -- this is 2nd endpt of prevSeg (and possibly 1st endpt of next)
//     ammend prevSeg with ...
//       parameter along reference line for perpendicular through contour point
//       index of segment endpoint within contour
//     if parameter of 2nd endpt < parameter of 1st endpt
//       set reversed flag true
//       remove and reinsert segment into list using parameter of 2nd endpt
//     if this is not the last contour point
//       create next new segment record containing ...
//         type of contour (DB,UNK)
//         parameter along reference line for perpendicular through contour point
//         index of segment endpoint within contour
//       insert segment into list ordered by refence line parameter of 1st endpt
//       keep pointer to segment (prevSeg)
//
// now, 
// For each point on L1 (each group of segments beginning at the same parameter)
//   if DB segment begins
//     add it to active DB segment list
//   if DB segment ends
//     remove it from active DB segment list 
//   if UNK segment begins
//     add it to active UNK segment list (with empty set of known intersections)
//   if UNK segment ends
//     remove it from active UNK segment list (with empty set of known intersections)
//   for each active DB segment
//     for each active UNK segment the DB segment is not already known to intersect
//       if single point of intersection exists between segment pair
//         save intersection indication to known intersection sets for both segments
//         create intersection record containing ...
//           (low index + intersection parameter) as pseudoIndices along both contours
//           coordinates of point
//         add intersection record to list ordered by pseudoIndex along DB coutour
//       if one segment contains all or part of the other
//         save intersection indication to known intersection sets for both segments
//         create TWO intersection record containing FOR BOTH ENDPOINTS ...
//           (low index + intersection parameters) as pseudoIndices along both contours
//           coordinates of points
//         add intersection records to list ordered by pseudoIndices along DB coutour
//        
// To process results,
// lasti and i are indices along DB contour
// lastj and j are indices along UNK contour
// k is index along list of found intersections
// lasti=0, lastj=0
// for each intersection int(k) in list (in order of occurrence along DB contour)
//   i = trunc(int(k).dbIndex)
//   if i < int(k).dbIndex <= i+1
//     int(k) is within segment DBseg(i,i+1)
//     polygon is poly(lasti, lasti+1, ..., i, int(k), int(k).unkIndex, ..., lastj)
//     NOTE: order of int(k).unkIndex, ..., lastj may be increasing or decreasing
//     NOTE: if lasti is non integer then last polygon closure is an intersection point
//       and will be at int(k-1) rather than at an actual contour point
//     compute area of polygon
//     add area to sum
//     lasti = int(k).dbIndex
//     lastj = int(k).unkIndex
//     k++ (move to next polygon closure point)
//  else -- ??????
//  
// this is the multimap - O(n lg n) version
//
double findAreaBetweenOutlineSegments( 
		FloatContour *c1, // mapped unknown fin 
		int begin1,
		int end1,
		FloatContour *c2, // envenly spaced database fin //***0005CM
		int begin2,
		int end2)
{

	// NOTE: this function is called twice (once for leading edge error and once for
	// trailing edge error)

	// reference line L is from begin to end (corresponding to LEBegin to TIP on first call
	// and TIP to TEEnd on second call)

	vector<contour_segment> segments;
	segments.clear();
	multimap<double, ref_point> refPts;
	refPts.clear();

	findSegsAndRefPointsForContour (c1,begin1,end1,'u',segments,refPts); // unknown contour
	findSegsAndRefPointsForContour (c2,begin2,end2,'d',segments,refPts); // database contour

	multimap<double,ref_point>::iterator mit;

	//for (mit = refPts.begin(); mit != refPts.end(); mit++)
	//	cout << "ref: " << (*mit).first << " " 
	//	     << segments[(*mit).second.segId].contourType 
	//		 << (*mit).second.segId << " " << (*mit).second.segOp << endl;


	// the segments and refPts are all valid and are ordered by parameter of their projection
	// onto the reference line
	// now, process the refPts in sequence to determine the contour intersections
	// (this includes touches, crrossings, ...)

	vector<int_point> intersectPoints;
	intersectPoints.clear();

	set<int> activeDBsegs, activeUNKsegs, perpsDB, perpsUNK;
	activeDBsegs.clear();
	activeUNKsegs.clear();
	perpsDB.clear();
	perpsUNK.clear();

	set<int>::iterator sit;

	multimap<double,ref_point>::iterator mit2;

	// push first point common to both contours (this may cause point to be in list twice
	// but that is better than it being missed)

	int_point iPt;
	iPt.paramDb = begin2;
	iPt.paramUnk = begin1;
	iPt.pt = (*c1)[begin1];
	intersectPoints.push_back(iPt);

	for (mit = refPts.begin(); mit != refPts.end(); mit = mit2)
	{
		// find all refPts with same projection parameter
		// "add" all segments beginning here ore perpendicular to here
		// "remove" all segments ending here
		mit2 = mit;
		while ((mit2 != refPts.end()) && ((*mit2).first == (*mit).first))
		{
			if ((*mit2).second.segOp == 0) // add segment to active list
				if (segments[(*mit2).second.segId].contourType == 'd')
					activeDBsegs.insert((*mit2).second.segId);
				else
					activeUNKsegs.insert((*mit2).second.segId);
			else if ((*mit2).second.segOp == 1) // remove segment from active list
				if (segments[(*mit2).second.segId].contourType == 'd')
					activeDBsegs.erase((*mit2).second.segId);
				else
					activeUNKsegs.erase((*mit2).second.segId);
			else
			{
				//cout << "perpSeg: " << segments[(*mit2).second.segId].contourType 
				//     << (*mit2).second.segId << endl;
				if (segments[(*mit2).second.segId].contourType == 'd')
				{
					activeDBsegs.insert((*mit2).second.segId);
					perpsDB.insert((*mit2).second.segId);
				}
				else
				{
					activeUNKsegs.insert((*mit2).second.segId);
					perpsUNK.insert((*mit2).second.segId);
				}
			}
			mit2++;
		}

		// now find all intersesctions between active DB segs and active UNK segs

		set<int>::iterator uit, dit;  // for accessing unknown and database segment IDs

		for (dit = activeDBsegs.begin(); dit != activeDBsegs.end(); dit++)
		{
			for (uit = activeUNKsegs.begin(); uit != activeUNKsegs.end(); uit++)
			{
				double 
					s1, s2, // parameter of intersection along database contour
					t1; // parameter of intersection along unknown contour
				point_t 
					p1, p2; // intersection point(s) .. 2 if segmetns overlap
				int n;      // number of intersection pts
				
				// if n is 2 after call to intersect it means the two segments have
				// an overlap zone coincident to the same line and p1 and p2 are the
				// ends of that segment

				//cout << "testing segs db:" << *dit << " unk:" << *uit << endl;

				if (segments[*dit].intersectingSegs.count(*uit) == 0)
				{
					int_point iPt;
					intersect(segments, c1, c2, *dit, *uit, s1, t1, p1, s2, p2, n);
					if (n == 1)
					{
						// single intersection point
						// s1 is parameter on DB segment
						// p1 is point of intersection
						// mark the flag for intersection between segments *dit and *uit
						//cout << "intersect: "
						//	<< "param " << s1 << " pt( "
						//	<< p1.x << ", " << p1.y << " )"
						//	<< " unkparam " << t1 << endl;
						
						iPt.paramDb = s1;
						iPt.paramUnk = t1;
						iPt.pt = p1;
						intersectPoints.push_back(iPt);
					}
					else if (n == 2)
					{
						// segments overlap on same line
						// s1 and s2 are start and stop parameters on DB segment
						// p1 and p2 are the end point of the overlap 
						// mark the flag for intersection between segments *dit and *uit
						//cout << "  overlap: "
						//	<< "param " << s1 << " pt( "
						//	<< p1.x << ", " << p1.y << " ) to "
						//	<< "param " << s2 << " pt( "
						//	<< p2.x << ", " << p2.y << " )"
						//	<< " unkparam " << t1 << endl;

						// since overlapping segments represent polygons with NO area
						// we can skip over these and NOT add them to the list for later
						// use in area calculation
						iPt.paramDb = s1;
						iPt.paramUnk = t1;
						iPt.pt = p1;
						intersectPoints.push_back(iPt);

						//iPt.param = s2;
						//iPt.pt = p2;
						//intersectPoints.push_back(iPt);
					}
					else if ((intersectPoints.size() == 0) && (n == 0))
						cout << "ERROR in area: first interest point MISSED" << endl;
				}
			}
		}

		//cout << "next RefPt:\n";
		//cout << " ActiveDbSegs: ";
		//for (sit = activeDBsegs.begin(); sit != activeDBsegs.end(); sit++)
		//	cout << (*sit) << ' ';
		//cout << endl;

		//cout << "ActiveUnkSegs: ";
		//for (sit = activeUNKsegs.begin(); sit != activeUNKsegs.end(); sit++)
		//	cout << (*sit) << ' ';
		//cout << endl;

		
		// now remove all perps before going on to nex ref point

		set<int>::iterator pit;

		for (pit = perpsDB.begin(); pit != perpsDB.end(); pit++)
			activeDBsegs.erase(*pit);
		perpsDB.clear();

		for (pit = perpsUNK.begin(); pit != perpsUNK.end(); pit++)
			activeUNKsegs.erase(*pit);
		perpsUNK.clear();
	}

	// push last point common to both contours
	//int_point iPt;
	iPt.paramDb = end2;
	iPt.paramUnk = end1;
	iPt.pt = (*c1)[end1];
	intersectPoints.push_back(iPt);

	// now calculate the area between the contours
	double sum = 0.0;
	
	for (int i = 0; i+1 < intersectPoints.size(); i++) // index into intersectPoints
	{
		// sequence of points for area calc is from intersect point
		// then IN order through database contour points to next intersect point
		// then in reverse order through unknown points to first intersect point

		vector<point_t> points;
		points.push_back(intersectPoints[i].pt);

		int lo, hi;

		// limits of index alond database contour between intersect points
		if (floor(intersectPoints[i].paramDb) == intersectPoints[i].paramDb)
			lo = intersectPoints[i].paramDb + 1;
		else
			lo = ceil(intersectPoints[i].paramDb);

		if (ceil(intersectPoints[i+1].paramDb) == intersectPoints[i+1].paramDb)
			hi = intersectPoints[i+1].paramDb - 1;
		else
			hi = floor(intersectPoints[i+1].paramDb);

		int j;

		for (j = lo; j <= hi; j++)
			points.push_back((*c2)[j]); // database contour points between intersections
	
		points.push_back(intersectPoints[i+1].pt); // push next intersect point

		// limits of index alond unknown contour between intersect points
		if (ceil(intersectPoints[i+1].paramUnk) == intersectPoints[i+1].paramUnk)
			hi = intersectPoints[i+1].paramUnk - 1;
		else
			hi = floor(intersectPoints[i+1].paramUnk);

		if (floor(intersectPoints[i].paramUnk) == intersectPoints[i].paramUnk)
			lo = intersectPoints[i].paramUnk + 1;
		else
			lo = ceil(intersectPoints[i].paramUnk);

		for (j = hi; j >= lo; j--)
			points.push_back((*c1)[j]); // unknown contour points between intersections

		// last point is same as initial point

		//sum += polygonArea(points); // original method - just the sum of areas

		double area = polygonArea(points);
		//sum += area * area; // do a squared area -- probably NO ????

		// this weights each area by length of outline along which it occurs
		// and seems to have some merit
		//sum += area * (intersectPoints[i+1].paramDb - intersectPoints[i].paramDb);
		sum += area;
	}

	return sum;
/*
	// find length of database fin outline
	double dbArcLength = 0.0;
	for (int k = 1; k < c2->length(); k++)
	{
		double dx = (*c2)[k].x - (*c2)[k-1].x;
		double dy = (*c2)[k].y - (*c2)[k-1].y;
		if ((begin2 < k) && (k <= end2))
			dbArcLength += sqrt(dx * dx + dy * dy);
	}

	return (sum / dbArcLength);
*/
}

//*******************************************************************
//
// this is the Vector - O(n) version
//
double findAreaBetweenOutlineSegments_Vec( 
		FloatContour *c1, // mapped unknown fin 
		int begin1,
		int end1,
		FloatContour *c2, // envenly spaced database fin //***0005CM
		int begin2,
		int end2)
{

	// NOTE: this function is called twice (once for leading edge error and once for
	// trailing edge error)

	// reference line L is from begin to end (corresponding to LEBegin to TIP on first call
	// and TIP to TEEnd on second call)

	vector<contour_segment> segments;
	segments.clear();
	vector<ref_point> refPts, refPtsUnk, refPtsDb;
	refPts.clear();
	refPtsUnk.clear();
	refPtsDb.clear();

	findSegsAndRefPointsForContour_Vec (c1,begin1,end1,'u',segments,refPtsUnk); // unknown contour
	findSegsAndRefPointsForContour_Vec (c2,begin2,end2,'d',segments,refPtsDb); // database contour

	// now MERGE the two ref points vectors (as in a MergeSort)
	int unkI=0, dbI=0, unkN = refPtsUnk.size(), dbN = refPtsDb.size();
	while ((unkI < unkN) && (dbI < dbN))
		if (refPtsUnk[unkI].parameter < refPtsDb[dbI].parameter)
			refPts.push_back(refPtsUnk[unkI++]);
		else
			refPts.push_back(refPtsDb[dbI++]);
	while (unkI < unkN)
		refPts.push_back(refPtsUnk[unkI++]);
	while (dbI < dbN)
		refPts.push_back(refPtsDb[dbI++]);
	
	vector<ref_point>::iterator mit;

	//for (mit = refPts.begin(); mit != refPts.end(); mit++)
	//	cout << "ref: " << (*mit).first << " " 
	//	     << segments[(*mit).second.segId].contourType 
	//		 << (*mit).second.segId << " " << (*mit).second.segOp << endl;


	// the segments and refPts are all valid and are ordered by parameter of their projection
	// onto the reference line
	// now, process the refPts in sequence to determine the contour intersections
	// (this includes touches, crrossings, ...)

	vector<int_point> intersectPoints;
	intersectPoints.clear();

	set<int> activeDBsegs, activeUNKsegs, perpsDB, perpsUNK;
	activeDBsegs.clear();
	activeUNKsegs.clear();
	perpsDB.clear();
	perpsUNK.clear();

	set<int>::iterator sit;

	vector<ref_point>::iterator mit2;

	// push first point common to both contours (this may cause point to be in list twice
	// but that is better than it being missed)

	int_point iPt;
	iPt.paramDb = begin2;
	iPt.paramUnk = begin1;
	iPt.pt = (*c1)[begin1];
	intersectPoints.push_back(iPt);

	for (mit = refPts.begin(); mit != refPts.end(); mit = mit2)
	{
		// find all refPts with same projection parameter
		// "add" all segments beginning here ore perpendicular to here
		// "remove" all segments ending here
		mit2 = mit;
		while ((mit2 != refPts.end()) && ((*mit2).parameter == (*mit).parameter))
		{
			if ((*mit2).segOp == 0) // add segment to active list
				if (segments[(*mit2).segId].contourType == 'd')
					activeDBsegs.insert((*mit2).segId);
				else
					activeUNKsegs.insert((*mit2).segId);
			else if ((*mit2).segOp == 1) // remove segment from active list
				if (segments[(*mit2).segId].contourType == 'd')
					activeDBsegs.erase((*mit2).segId);
				else
					activeUNKsegs.erase((*mit2).segId);
			else
			{
				//cout << "perpSeg: " << segments[(*mit2).second.segId].contourType 
				//     << (*mit2).second.segId << endl;
				if (segments[(*mit2).segId].contourType == 'd')
				{
					activeDBsegs.insert((*mit2).segId);
					perpsDB.insert((*mit2).segId);
				}
				else
				{
					activeUNKsegs.insert((*mit2).segId);
					perpsUNK.insert((*mit2).segId);
				}
			}
			mit2++;
		}

		// now find all intersesctions between active DB segs and active UNK segs

		set<int>::iterator uit, dit;  // for accessing unknown and database segment IDs

		for (dit = activeDBsegs.begin(); dit != activeDBsegs.end(); dit++)
		{
			for (uit = activeUNKsegs.begin(); uit != activeUNKsegs.end(); uit++)
			{
				double 
					s1, s2, // parameter of intersection along database contour
					t1; // parameter of intersection along unknown contour
				point_t 
					p1, p2; // intersection point(s) .. 2 if segmetns overlap
				int n;      // number of intersection pts
				
				// if n is 2 after call to intersect it means the two segments have
				// an overlap zone coincident to the same line and p1 and p2 are the
				// ends of that segment

				//cout << "testing segs db:" << *dit << " unk:" << *uit << endl;

				if (segments[*dit].intersectingSegs.count(*uit) == 0)
				{
					int_point iPt;
					intersect(segments, c1, c2, *dit, *uit, s1, t1, p1, s2, p2, n);
					if (n == 1)
					{
						// single intersection point
						// s1 is parameter on DB segment
						// p1 is point of intersection
						// mark the flag for intersection between segments *dit and *uit
						//cout << "intersect: "
						//	<< "param " << s1 << " pt( "
						//	<< p1.x << ", " << p1.y << " )"
						//	<< " unkparam " << t1 << endl;
						
						iPt.paramDb = s1;
						iPt.paramUnk = t1;
						iPt.pt = p1;
						intersectPoints.push_back(iPt);
					}
					else if (n == 2)
					{
						// segments overlap on same line
						// s1 and s2 are start and stop parameters on DB segment
						// p1 and p2 are the end point of the overlap 
						// mark the flag for intersection between segments *dit and *uit
						//cout << "  overlap: "
						//	<< "param " << s1 << " pt( "
						//	<< p1.x << ", " << p1.y << " ) to "
						//	<< "param " << s2 << " pt( "
						//	<< p2.x << ", " << p2.y << " )"
						//	<< " unkparam " << t1 << endl;

						// since overlapping segments represent polygons with NO area
						// we can skip over these and NOT add them to the list for later
						// use in area calculation
						iPt.paramDb = s1;
						iPt.paramUnk = t1;
						iPt.pt = p1;
						intersectPoints.push_back(iPt);

						//iPt.param = s2;
						//iPt.pt = p2;
						//intersectPoints.push_back(iPt);
					}
					else if ((intersectPoints.size() == 0) && (n == 0))
						cout << "ERROR in area: first interest point MISSED" << endl;
				}
			}
		}

		//cout << "next RefPt:\n";
		//cout << " ActiveDbSegs: ";
		//for (sit = activeDBsegs.begin(); sit != activeDBsegs.end(); sit++)
		//	cout << (*sit) << ' ';
		//cout << endl;

		//cout << "ActiveUnkSegs: ";
		//for (sit = activeUNKsegs.begin(); sit != activeUNKsegs.end(); sit++)
		//	cout << (*sit) << ' ';
		//cout << endl;

		
		// now remove all perps before going on to nex ref point

		set<int>::iterator pit;

		for (pit = perpsDB.begin(); pit != perpsDB.end(); pit++)
			activeDBsegs.erase(*pit);
		perpsDB.clear();

		for (pit = perpsUNK.begin(); pit != perpsUNK.end(); pit++)
			activeUNKsegs.erase(*pit);
		perpsUNK.clear();
	}

	// push last point common to both contours
	//int_point iPt;
	iPt.paramDb = end2;
	iPt.paramUnk = end1;
	iPt.pt = (*c1)[end1];
	intersectPoints.push_back(iPt);

	// now calculate the area between the contours
	double sum = 0.0;
	
	for (int i = 0; i+1 < intersectPoints.size(); i++) // index into intersectPoints
	{
		// sequence of points for area calc is from intersect point
		// then IN order through database contour points to next intersect point
		// then in reverse order through unknown points to first intersect point

		vector<point_t> points;
		points.clear();
		points.push_back(intersectPoints[i].pt);

		int lo, hi;

		// limits of index alond database contour between intersect points
		if (floor(intersectPoints[i].paramDb) == intersectPoints[i].paramDb)
			lo = intersectPoints[i].paramDb + 1;
		else
			lo = ceil(intersectPoints[i].paramDb);

		if (ceil(intersectPoints[i+1].paramDb) == intersectPoints[i+1].paramDb)
			hi = intersectPoints[i+1].paramDb - 1;
		else
			hi = floor(intersectPoints[i+1].paramDb);

		int j;

		for (j = lo; j <= hi; j++)
			points.push_back((*c2)[j]); // database contour points between intersections
	
		points.push_back(intersectPoints[i+1].pt); // push next intersect point

		// limits of index alond unknown contour between intersect points
		if (ceil(intersectPoints[i+1].paramUnk) == intersectPoints[i+1].paramUnk)
			hi = intersectPoints[i+1].paramUnk - 1;
		else
			hi = floor(intersectPoints[i+1].paramUnk);

		if (floor(intersectPoints[i].paramUnk) == intersectPoints[i].paramUnk)
			lo = intersectPoints[i].paramUnk + 1;
		else
			lo = ceil(intersectPoints[i].paramUnk);

		for (j = hi; j >= lo; j--)
			points.push_back((*c1)[j]); // unknown contour points between intersections

		// last point is same as initial point

		// sum += polygonArea(points); // original method - just the sum of areas

		double area = polygonArea(points);
		//sum += area * area; // do a squared area -- probably NO ????

		// this weights each area by length of outline along which it occurs
		// and seems to have some merit
		// sum += area * (intersectPoints[i+1].paramDb - intersectPoints[i].paramDb);
		sum += area;
	}

	return sum;
/*
	// find length of database fin outline
	double dbArcLength = 0.0;
	for (int k = 1; k < c2->length(); k++)
	{
		double dx = (*c2)[k].x - (*c2)[k-1].x;
		double dy = (*c2)[k].y - (*c2)[k-1].y;
		if ((begin2 < k) && (k <= end2))
			dbArcLength += sqrt(dx * dx + dy * dy);
	}

	return (sum / dbArcLength);
*/
}

//*******************************************************************
//
//
double areaBasedErrorBetweenOutlineSegments_NEW( 
		FloatContour *c1, // mapped unknown fin 
		int begin1,
		int mid1,
		int end1,
		FloatContour *c2, // envenly spaced database fin //***0005CM
		int begin2,
		int mid2,
		int end2)
{
	// find length of database fin outline
	double dbArcLength = 0.0;
	for (int k = 1; k < c2->length(); k++)
	{
		double dx = (*c2)[k].x - (*c2)[k-1].x;
		double dy = (*c2)[k].y - (*c2)[k-1].y;
		if ((begin2 < k) && (k <= end2))
			dbArcLength += sqrt(dx * dx + dy * dy);
	}

	double area;

	// NOTE: the Vector-based attempt to speed the process was actually slower,
	// and had bugs I could not isolate ... so eliminate this call for now - JHS (4/30/07)
	// area  = findAreaBetweenOutlineSegments_Vec(c1,begin1,mid1,c2,begin2,mid2);
	// area += findAreaBetweenOutlineSegments_Vec(c1,mid1,end1,c2,mid2,end2);

	area  = findAreaBetweenOutlineSegments(c1,begin1,mid1,c2,begin2,mid2);
	area += findAreaBetweenOutlineSegments(c1,mid1,end1,c2,mid2,end2);

	double returnVal = area / dbArcLength;
	// cout << "AreaError: " << returnVal << endl;

	return (returnVal);
}
