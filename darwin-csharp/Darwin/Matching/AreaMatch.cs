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

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace Darwin.Matching
{
    public static class AreaMatch
    {
        //*******************************************************************
        //
        //
        public static double PolygonArea(List<PointF> p)
        {
            double sum = 0.0;

            int n = p.Count;

            for (int i = 0; i < n; i++)
            {
                int j = (i + 1) % n;
                sum += (p[i].X * p[j].Y - p[j].X * p[i].Y);
            }

            double area = Math.Abs(0.5 * sum);
            //cout << "Area: " << area << endl;

            return area;
        }

        //*******************************************************************
        //
        //
        public static void Intersect(ref List<ContourSegment> segments,
                       FloatContour c1, // unknown contour
                       FloatContour c2, // database contour
                       int dbId, int unkId,
                       out double s1, out double t1, out PointF p1,  // db param, point coords & unk param
                       out double s2, out PointF p2,       // db param, point coords
                       out int n)
        {
            s1 = t1 = s2 = 0;
            p1 = null;
            p2 = null;
            double
                dx1, dy1,
                dx2, dy2;
            PointF
                A, B, // begin and end of database segment
                C, D; // begin and end of unknown segment
            int
                idA, idB, // indices of A & B
                idC, idD; // indices of C & D

            idA = segments[dbId].StartIndex;
            if (!segments[dbId].Reversed)
                idB = idA + 1;
            else
                idB = idA - 1;

            idC = segments[unkId].StartIndex;
            if (!segments[unkId].Reversed)
                idD = idC + 1;
            else
                idD = idC - 1;

            A = c2[idA];
            B = c2[idB];
            C = c1[idC];
            D = c1[idD];

            dx1 = B.X - A.X;
            dy1 = B.Y - A.Y;
            dx2 = D.X - C.X;
            dy2 = D.Y - C.Y;

            double
                beta1 = 0, // parameter on database segment
                beta2 = 0; // parameter on unknown segment

            double denom = dx1 * dy2 - dy1 * dx2;
            if (denom != 0.0) // thre is a unique intersection between the lines
            {
                beta1 = ((A.X - C.Y) * dx2 - (A.X - C.X) * dy2) / denom; // on database
                if (dx2 != 0.0)
                    beta2 = ((A.X - C.X) + beta1 * dx1) / dx2; // on unknown
                else
                    beta2 = ((A.Y - C.Y) + beta1 * dy1) / dy2; // on unknown

                if ((0.0 <= beta1) && (beta1 < 1.0) && (0.0 <= beta2) && (beta2 < 1.0))
                {
                    // found single point of intersection
                    p1.X = (float)(A.X + beta1 * dx1);
                    p1.Y = (float)(A.Y + beta1 * dy1);
                    n = 1;
                    if (!segments[dbId].Reversed)
                        s1 = idA + beta1;
                    else
                        s1 = idB - beta1;
                    if (!segments[unkId].Reversed)
                        t1 = idC + beta2;
                    else
                        t1 = idD - beta2;

                    // mark interection so we won't repeat intersection test later
                    segments[dbId].IntersectingSegs.Add(unkId);
                }
                else
                    n = 0; // no intersection within both segments
            }
            else // might have segment overlap
            {
                double det = C.X * (A.Y - B.Y) - A.X * (C.Y - B.Y) + B.X * (C.Y - A.Y);
                if (det == 0.0) // point C is on line containing A * B
                {
                    // find out range of parameter overlap on database segment
                    if (dx1 != 0.0)
                    {
                        beta1 = (C.X - A.X) / dx1;
                        beta2 = (D.X - A.X) / dx1;
                    }
                    else if (dy1 != 0.0)
                    {
                        beta1 = (C.Y - A.Y) / dy1;
                        beta2 = (D.Y - A.Y) / dy1;
                    }
                    else
                    {
                        Trace.WriteLine("ERROR in match calc: zero length segment in contour");
                        n = 0;
                    }

                    if (((beta1 > 1.0) && (beta2 > 1.0)) || ((beta1 < 0.0) && (beta2 < 0.0)))
                    {
                        // no segment overlap
                        n = 0;
                        s1 = t1 = s2 = 0;
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

                    p1.X = (float)(A.X + beta1 * dx1);
                    p1.Y = (float)(A.Y + beta1 * dy1);
                    p2.X = (float)(A.X + beta2 * dx1);
                    p2.Y = (float)(A.Y + beta2 * dy1);
                    n = 2;
                    if (!segments[dbId].Reversed)
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
                    segments[dbId].IntersectingSegs.Add(unkId);

                    // find parameter of first point along unknown, since it marks the end of
                    // some polygon at the beginning of the overlap zone
                    double u1;
                    if (dx2 != 0.0)
                        u1 = (p1.X - C.X) / dx1; // on unknown
                    else
                        u1 = (p1.Y - C.Y) / dy1; // on unknown
                    if (!segments[unkId].Reversed)
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
        public static void FindSegsAndRefPointsForContour(
                FloatContour c1, // part of some contour (DB or UNK)
                int begin1,
                int end1,
                char conType, // 'd' or 'u'
                ref List<ContourSegment> segments,
                ref SortedDictionary<double, List<RefPoint>> refPts
                )
        {
            // this runs down the contour and appends segments and refPts to existing lists
            // called by areaErro function

            double
                dxL = c1[end1].X - c1[begin1].X,
                dyL = c1[end1].Y - c1[begin1].Y,
                dxLsq = dxL * dxL,
                dyLsq = dyL * dyL,
                denom = dxLsq + dyLsq,
                xB = c1[begin1].X,
                yB = c1[begin1].Y,
                xE = c1[end1].X,
                yE = c1[end1].Y;

            //vector<contour_segment> segments;
            //multimap<double, ref_point> refPts;
            // project each unknown fin contour point onto reference line

            // first point

            ContourSegment seg = new ContourSegment();
            RefPoint pt = new RefPoint();

            // Used as iterator in the C++ version, reference to the inserted point
            RefPoint ppIt;

            // Counter / iterator for list of List<ContourSegment> 
            int it = 0;

            bool seg_reversed, seg_perpendicular;

            // create segment record and append to vector

            seg.ContourType = conType; // unknown or database
            seg.Parameter = 0.0;   // first point on reference line
            seg.Reversed = false;
            seg.StartIndex = begin1;
            segments.Add(seg);

            //cout << "Param: " << seg.parameter << endl;

            // create reference point record with segment index & add to active list indicators
            // and then insert reference point into multipmap in order by reference line parameter

            pt.Parameter = 0.0;
            pt.SegId = segments.Count - 1;
            pt.SegOp = 0;       // indicate "add" op

            if (refPts[pt.Parameter] == null)
                refPts[pt.Parameter] = new List<RefPoint>();

            refPts[pt.Parameter].Add(pt);
            ppIt = pt;
            //RefPoint rp = pt;

            // all middle points
            for (int i = begin1 + 1; i < end1; i++)
            {
                double s = ((c1[i].X - xB) * dxL + (c1[i].Y - yB) * dyL) / denom;
                // this is the end of previous segment
                it = segments.Count - 1;
                //it--; // back up to the actual last element
                seg_reversed = (s < segments[it].Parameter);
                seg_perpendicular = (s == segments[it].Parameter);
                if (seg_reversed)
                {
                    // the segment is a reversed segment, so correct info in previous segment
                    segments[it].Reversed = true;
                    segments[it].Parameter = s;
                    segments[it].StartIndex = i;
                }

                // create reference point record with existing segment index & remove from active 
                // list indicators and then insert reference point into multipmap in order by 
                // reference line parameter

                pt.Parameter = s;
                pt.SegId = segments.Count - 1;
                if (seg_reversed)
                {
                    // segment runs in reverse direction so ...
                    ppIt.SegOp = 1;  // previous endpoint is where segment is "removed" 
                    pt.SegOp = 0;       // and this 2nd endpoint is where segment is "added"

                    if (refPts[pt.Parameter] == null)
                        refPts[pt.Parameter] = new List<RefPoint>();
                    refPts[pt.Parameter].Add(pt);

                    ppIt = pt; // Keep a reference to the inserted point 
                }
                else if (seg_perpendicular)
                    ppIt.SegOp = 2;       // indicate immediate processing (seg perp to ref line)
                else
                {
                    pt.SegOp = 1;       // indicate "remove" op

                    // insert pt into multimap
                    if (refPts[pt.Parameter] == null)
                        refPts[pt.Parameter] = new List<RefPoint>();
                    refPts[pt.Parameter].Add(pt);

                    ppIt = pt; // Keep a reference to the inserted point 
                }

                // it is also the beginning of a new segment
                seg.ContourType = conType; // unknown or database
                seg.Parameter = s;
                seg.Reversed = false;
                seg.StartIndex = i;
                segments.Add(seg);

                //cout << "Param: " << seg.parameter << endl;

                // create reference point record with segment index & add to active list indicators
                // and then insert reference point into multipmap in order by reference line parameter
                pt.Parameter = s;
                pt.SegId = segments.Count - 1;
                pt.SegOp = 0;       // indicate "add" op

                if (refPts[pt.Parameter] == null)
                    refPts[pt.Parameter] = new List<RefPoint>();
                refPts[pt.Parameter].Add(pt);

                ppIt = pt;  // Keep a reference to the inserted point 

                //rp = pt;
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

            it = segments.Count - 1;
            //it--; // back up to the actual last element
            pt.Parameter = 1.0;
            pt.SegId = segments.Count - 1;
            if (1.0 < segments[it].Parameter)
            {
                // segment runs in reverse direction so ...
                ppIt.SegOp = 1;  // previous endpoint is where segment is "removed" 
                pt.SegOp = 0;       // and this 2nd endpoint is where segmetn is "added"

                if (refPts[pt.Parameter] == null)
                    refPts[pt.Parameter] = new List<RefPoint>();
                refPts[pt.Parameter].Add(pt);
                ppIt = pt;     // Keep a reference to the inserted point
            }
            else if ((1.0 == segments[it].Parameter))
                ppIt.SegOp = 2;       // indicate immediate processing (seg perp to ref line)
            else
            {
                pt.SegOp = 1;       // indicate "remove" op
                if (refPts[pt.Parameter] == null)
                    refPts[pt.Parameter] = new List<RefPoint>();
                refPts[pt.Parameter].Add(pt);
                ppIt = pt;     // Keep a reference to the inserted point
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
        public static void FindSegsAndRefPointsForContour_Vec(
                FloatContour c1, // part of some contour (DB or UNK)
                int begin1,
                int end1,
                char conType, // 'd' or 'u'
                ref List<ContourSegment> segments,
                ref List<RefPoint> refPts
                )
        {
            // this runs down the contour and appends segments and refPts to existing lists
            // called by areaErro function

            double
                dxL = c1[end1].X - c1[begin1].X,
                dyL = c1[end1].Y - c1[begin1].Y,
                dxLsq = dxL * dxL,
                dyLsq = dyL * dyL,
                denom = dxLsq + dyLsq,
                xB = c1[begin1].X,
                yB = c1[begin1].Y,
                xE = c1[end1].X,
                yE = c1[end1].Y;

            //vector<contour_segment> segments;
            //multimap<double, ref_point> refPts;
            // project each unknown fin contour point onto reference line

            // first point

            ContourSegment seg = new ContourSegment();
            RefPoint pt = new RefPoint();

            // vector<ref_point>::iterator in the C++ version
            int ppIt, ppItPrev;
            // vector<contour_segment>::iterator in the C++ version
            int it;

            bool seg_reversed, seg_perpendicular;

            // create segment record and append to vector

            seg.ContourType = conType; // unknown or database
            seg.Parameter = 0.0;   // first point on reference line
            seg.Reversed = false;
            seg.StartIndex = begin1;
            segments.Add(seg);

            if (segments.Count == 0)
                Trace.WriteLine("Segment push failed");

            //cout << "Param: " << seg.parameter << endl;

            // create reference point record with segment index & add to active list indicators
            // and then insert reference point into multipmap in order by reference line parameter

            pt.Parameter = 0.0;
            pt.SegId = segments.Count - 1;
            pt.SegOp = 0;       // indicate "add" op

            // first point -- NO insertion sort done here
            refPts.Add(pt);     // insert pt into multimap

            // refPts.begin()
            ppItPrev = 0;

            RefPoint rp = refPts.First();

            // all middle points
            for (int i = begin1 + 1; i < end1; i++)
            {
                double s = ((c1[i].X - xB) * dxL + (c1[i].Y - yB) * dyL) / denom;
                // this is the end of previous segment
                it = segments.Count - 1;
                //it--; // back up to the actual last element
                seg_reversed = (s < segments[it].Parameter);
                seg_perpendicular = (s == segments[it].Parameter);
                if (seg_reversed)
                {
                    // the segment is a reversed segment, so correct info in previous segment
                    segments[it].Reversed = true;
                    segments[it].Parameter = s;
                    segments[it].StartIndex = i;
                }

                // create reference point record with existing segment index & remove from active 
                // list indicators and then insert reference point into multipmap in order by 
                // reference line parameter

                pt.Parameter = s;
                pt.SegId = segments.Count - 1;
                if (seg_reversed)
                {
                    // segment runs in reverse direction so ...
                    refPts[ppItPrev].SegOp = 1;  // previous endpoint is where segment is "removed" 
                    pt.SegOp = 0;       // and this 2nd endpoint is where segment is "added"
                                        //ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
                                        // a small insertion sort done here
                    ppIt = refPts.Count - 1;

                    while (ppIt >= 0 && refPts[ppIt].Parameter > pt.Parameter)
                        ppIt--;

                    ppIt++;

                    refPts.Insert(ppIt, pt);     // insert pt into multimap
                    ppItPrev = ppIt;
                }
                else if (seg_perpendicular)
                    refPts[ppItPrev].SegOp = 2;       // indicate immediate processing (seg perp to ref line)
                else
                {
                    pt.SegOp = 1;       // indicate "remove" op
                                        //ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
                                        // a small insertion sort done here
                    ppIt = refPts.Count - 1;

                    while (ppIt >= 0 && (refPts[ppIt].Parameter > pt.Parameter))
                        ppIt--;

                    ppIt++;
                    refPts.Insert(ppIt, pt);     // insert pt into multimap
                    ppItPrev = ppIt;
                }

                // it is also the beginning of a new segment
                seg.ContourType = conType; // unknown or database
                seg.Parameter = s;
                seg.Reversed = false;
                seg.StartIndex = i;
                segments.Add(seg);

                //cout << "Param: " << seg.parameter << endl;

                // create reference point record with segment index & add to active list indicators
                // and then insert reference point into multipmap in order by reference line parameter
                pt.Parameter = s;
                pt.SegId = segments.Count - 1;
                pt.SegOp = 0;       // indicate "add" op
                                    //ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
                                    // a small insertion sort done here
                ppIt = refPts.Count - 1;

                while ((ppIt >= 0) && (refPts[ppIt].Parameter > pt.Parameter))
                    ppIt--;
                ppIt++;
                refPts.Insert(ppIt, pt);     // insert pt into multimap
                ppItPrev = ppIt;
                rp = refPts[ppItPrev];
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

            it = segments.Count - 1;
            //it--; // back up to the actual last element
            pt.Parameter = 1.0;
            pt.SegId = segments.Count - 1;
            if (1.0 < segments[it].Parameter)
            {
                // segment runs in reverse direction so ...
                refPts[ppItPrev].SegOp = 1;  // previous endpoint is where segment is "removed" 
                pt.SegOp = 0;       // and this 2nd endpoint is where segmetn is "added"
                                    //ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
                                    // a small insertion sort done here
                ppIt = refPts.Count - 1;

                while (ppIt >= 0 && (refPts[ppIt].Parameter > pt.Parameter))
                    ppIt--;

                ppIt++;

                refPts.Insert(ppIt, pt);// insert pt into multimap
                ppItPrev = ppIt;
            }
            else if (1.0 == segments[it].Parameter)
                refPts[ppItPrev].SegOp = 2;       // indicate immediate processing (seg perp to ref line)
            else
            {
                pt.SegOp = 1;       // indicate "remove" op
                                    //ppIt = refPts.insert(make_pair(pt.parameter,pt));     // insert pt into multimap
                                    // a small insertion sort done here
                ppIt = refPts.Count - 1;

                while (ppIt >= 0 && (refPts[ppIt].Parameter > pt.Parameter))
                    ppIt--;

                ppIt++;

                refPts.Insert(ppIt, pt);     // insert pt into multimap
                ppItPrev = ppIt;
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
        public static double FindAreaBetweenOutlineSegments(
                FloatContour c1, // mapped unknown fin 
                int begin1,
                int end1,
                FloatContour c2, // envenly spaced database fin //***0005CM
                int begin2,
                int end2)
        {

            // NOTE: this function is called twice (once for leading edge error and once for
            // trailing edge error)

            // reference line L is from begin to end (corresponding to LEBegin to TIP on first call
            // and TIP to TEEnd on second call)

            List<ContourSegment> segments = new List<ContourSegment>();

            SortedDictionary<double, List<RefPoint>> refPts = new SortedDictionary<double, List<RefPoint>>();

            FindSegsAndRefPointsForContour(c1, begin1, end1, 'u', ref segments, ref refPts); // unknown contour
            FindSegsAndRefPointsForContour(c2, begin2, end2, 'd', ref segments, ref refPts); // database contour

            //multimap<double, ref_point>::iterator mit;

            //for (mit = refPts.begin(); mit != refPts.end(); mit++)
            //	cout << "ref: " << (*mit).first << " " 
            //	     << segments[(*mit).second.segId].contourType 
            //		 << (*mit).second.segId << " " << (*mit).second.segOp << endl;


            // the segments and refPts are all valid and are ordered by parameter of their projection
            // onto the reference line
            // now, process the refPts in sequence to determine the contour intersections
            // (this includes touches, crrossings, ...)

            List<IntPoint> intersectPoints = new List<IntPoint>();

            SortedSet<int> activeDBsegs = new SortedSet<int>();
            SortedSet<int> activeUNKsegs = new SortedSet<int>();
            SortedSet<int> perpsDB = new SortedSet<int>();
            SortedSet<int> perpsUNK = new SortedSet<int>();

            //set<int>::iterator sit;

            //multimap<double, ref_point>::iterator mit2;

            // push first point common to both contours (this may cause point to be in list twice
            // but that is better than it being missed)

            IntPoint iPtOutside = new IntPoint();
            iPtOutside.ParamDb = begin2;
            iPtOutside.ParamUnk = begin1;
            iPtOutside.Pt = c1[begin1];
            intersectPoints.Add(iPtOutside);

            foreach (var key in refPts.Keys)
            {
                // find all refPts with same projection parameter
                // "add" all segments beginning here ore perpendicular to here
                // "remove" all segments ending here
                foreach (var mit2 in refPts[key])
                {
                    if (mit2.SegOp == 0) // add segment to active list
                        if (segments[mit2.SegId].ContourType == 'd')
                            activeDBsegs.Add(mit2.SegId);
                        else
                            activeUNKsegs.Add(mit2.SegId);
                    else if (mit2.SegOp == 1) // remove segment from active list
                        if (segments[mit2.SegId].ContourType == 'd')
                            activeDBsegs.Add(mit2.SegId);
                        else
                            activeUNKsegs.Add(mit2.SegId);
                    else
                    {
                        //cout << "perpSeg: " << segments[(*mit2).second.segId].contourType 
                        //     << (*mit2).second.segId << endl;
                        if (segments[mit2.SegId].ContourType == 'd')
                        {
                            activeDBsegs.Add(mit2.SegId);
                            perpsDB.Add(mit2.SegId);
                        }
                        else
                        {
                            activeUNKsegs.Add(mit2.SegId);
                            perpsUNK.Add(mit2.SegId);
                        }
                    }
                }

                // now find all intersesctions between active DB segs and active UNK segs

                //set<int>::iterator uit, dit;  // for accessing unknown and database segment IDs

                foreach (var dit in activeDBsegs)
                {
                    foreach (var uit in activeUNKsegs)
                    {
                        double
                            s1, s2, // parameter of intersection along database contour
                            t1; // parameter of intersection along unknown contour
                        PointF
                            p1, p2; // intersection point(s) .. 2 if segmetns overlap
                        int n;      // number of intersection pts

                        // if n is 2 after call to intersect it means the two segments have
                        // an overlap zone coincident to the same line and p1 and p2 are the
                        // ends of that segment

                        //cout << "testing segs db:" << *dit << " unk:" << *uit << endl;

                        if (!segments[dit].IntersectingSegs.Contains(uit))
                        {
                            IntPoint iPt = new IntPoint();
                            Intersect(ref segments, c1, c2, dit, uit, out s1, out t1, out p1, out s2, out p2, out n);
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

                                iPt.ParamDb = s1;
                                iPt.ParamUnk = t1;
                                iPt.Pt = p1;
                                intersectPoints.Add(iPt);
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
                                iPt.ParamDb = s1;
                                iPt.ParamUnk = t1;
                                iPt.Pt = p1;
                                intersectPoints.Add(iPt);

                                //iPt.param = s2;
                                //iPt.pt = p2;
                                //intersectPoints.push_back(iPt);
                            }
                            else if ((intersectPoints.Count == 0) && (n == 0))
                                Trace.WriteLine("ERROR in area: first interest point MISSED");
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


                foreach (var pit in perpsDB)
                    activeDBsegs.Remove(pit);
                perpsDB.Clear();
                foreach (var pit in perpsUNK)
                    activeUNKsegs.Remove(pit);
                perpsUNK.Clear();
            }

            // push last point common to both contours
            //int_point iPt;
            var lastCommon = new IntPoint();
            lastCommon.ParamDb = end2;
            lastCommon.ParamUnk = end1;
            lastCommon.Pt = c1[end1];
            intersectPoints.Add(lastCommon);

            // now calculate the area between the contours
            double sum = 0.0;

            for (int i = 0; i + 1 < intersectPoints.Count; i++) // index into intersectPoints
            {
                // sequence of points for area calc is from intersect point
                // then IN order through database contour points to next intersect point
                // then in reverse order through unknown points to first intersect point

                List<PointF> points = new List<PointF>();
                points.Add(intersectPoints[i].Pt);

                int lo, hi;

                // limits of index alond database contour between intersect points
                if (Math.Floor(intersectPoints[i].ParamDb) == intersectPoints[i].ParamDb)
                    lo = Convert.ToInt32(intersectPoints[i].ParamDb + 1);
                else
                    lo = Convert.ToInt32(Math.Ceiling(intersectPoints[i].ParamDb));

                if (Math.Ceiling(intersectPoints[i + 1].ParamDb) == intersectPoints[i + 1].ParamDb)
                    hi = Convert.ToInt32(intersectPoints[i + 1].ParamDb - 1);
                else
                    hi = Convert.ToInt32(Math.Floor(intersectPoints[i + 1].ParamDb));

                int j;

                for (j = lo; j <= hi; j++)
                    points.Add(c2[j]); // database contour points between intersections

                points.Add(intersectPoints[i + 1].Pt); // push next intersect point

                // limits of index alond unknown contour between intersect points
                if (Math.Ceiling(intersectPoints[i + 1].ParamUnk) == intersectPoints[i + 1].ParamUnk)
                    hi = Convert.ToInt32(intersectPoints[i + 1].ParamUnk - 1);
                else
                    hi = Convert.ToInt32(Math.Floor(intersectPoints[i + 1].ParamUnk));

                if (Math.Floor(intersectPoints[i].ParamUnk) == intersectPoints[i].ParamUnk)
                    lo = Convert.ToInt32(intersectPoints[i].ParamUnk + 1);
                else
                    lo = Convert.ToInt32(Math.Ceiling(intersectPoints[i].ParamUnk));

                for (j = hi; j >= lo; j--)
                    points.Add(c1[j]); // unknown contour points between intersections

                // last point is same as initial point

                //sum += polygonArea(points); // original method - just the sum of areas

                double area = PolygonArea(points);
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
        public static double FindAreaBetweenOutlineSegments_Vec(
                FloatContour c1, // mapped unknown fin 
                int begin1,
                int end1,
                FloatContour c2, // envenly spaced database fin //***0005CM
                int begin2,
                int end2)
        {

            // NOTE: this function is called twice (once for leading edge error and once for
            // trailing edge error)

            // reference line L is from begin to end (corresponding to LEBegin to TIP on first call
            // and TIP to TEEnd on second call)

            List<ContourSegment> segments = new List<ContourSegment>();
            segments.Clear();
            List<RefPoint> refPts = new List<RefPoint>();
            List<RefPoint> refPtsUnk = new List<RefPoint>();
            List<RefPoint> refPtsDb = new List<RefPoint>();


            FindSegsAndRefPointsForContour_Vec(c1, begin1, end1, 'u', ref segments, ref refPtsUnk); // unknown contour
            FindSegsAndRefPointsForContour_Vec(c2, begin2, end2, 'd', ref segments, ref refPtsDb); // database contour

            // now MERGE the two ref points vectors (as in a MergeSort)
            int unkI = 0, dbI = 0, unkN = refPtsUnk.Count, dbN = refPtsDb.Count;
            while ((unkI < unkN) && (dbI < dbN))
                if (refPtsUnk[unkI].Parameter < refPtsDb[dbI].Parameter)
                    refPts.Add(refPtsUnk[unkI++]);
                else
                    refPts.Add(refPtsDb[dbI++]);
            while (unkI < unkN)
                refPts.Add(refPtsUnk[unkI++]);
            while (dbI < dbN)
                refPts.Add(refPtsDb[dbI++]);

            // Originally vector<ref_point> in C++ version
            int mit;

            //for (mit = refPts.begin(); mit != refPts.end(); mit++)
            //	cout << "ref: " << (*mit).first << " " 
            //	     << segments[(*mit).second.segId].contourType 
            //		 << (*mit).second.segId << " " << (*mit).second.segOp << endl;


            // the segments and refPts are all valid and are ordered by parameter of their projection
            // onto the reference line
            // now, process the refPts in sequence to determine the contour intersections
            // (this includes touches, crrossings, ...)

            List<IntPoint> intersectPoints = new List<IntPoint>();

            SortedSet<int> activeDBsegs = new SortedSet<int>();
            SortedSet<int> activeUNKsegs = new SortedSet<int>();
            SortedSet<int> perpsDB = new SortedSet<int>();
            SortedSet<int> perpsUNK = new SortedSet<int>();

            // set<int>::iterator in C++ version
            int sit = 0;

            // vector<ref_point>::iterator in C++ version
            int mit2;

            // push first point common to both contours (this may cause point to be in list twice
            // but that is better than it being missed)

            IntPoint firstCommon = new IntPoint();
            firstCommon.ParamDb = begin2;
            firstCommon.ParamUnk = begin1;
            firstCommon.Pt = c1[begin1];
            intersectPoints.Add(firstCommon);

            for (mit = 0; mit < refPts.Count; mit = mit2)
            {
                // find all refPts with same projection parameter
                // "add" all segments beginning here ore perpendicular to here
                // "remove" all segments ending here
                mit2 = mit;
                while ((mit2 != refPts.Count) && (refPts[mit2].Parameter == refPts[mit].Parameter))
                {
                    if (refPts[mit2].SegOp == 0) // add segment to active list
                        if (segments[refPts[mit2].SegId].ContourType == 'd')
                            activeDBsegs.Add(refPts[mit2].SegId);
                        else
                            activeUNKsegs.Add(refPts[mit2].SegId);
                    else if (refPts[mit2].SegOp == 1) // remove segment from active list
                        if (segments[refPts[mit2].SegId].ContourType == 'd')
                            activeDBsegs.Remove(refPts[mit2].SegId);
                        else
                            activeUNKsegs.Remove(refPts[mit2].SegId);
                    else
                    {
                        //cout << "perpSeg: " << segments[(*mit2).second.segId].contourType 
                        //     << (*mit2).second.segId << endl;
                        if (segments[refPts[mit2].SegId].ContourType == 'd')
                        {
                            activeDBsegs.Add(refPts[mit2].SegId);
                            perpsDB.Add(refPts[mit2].SegId);
                        }
                        else
                        {
                            activeUNKsegs.Add(refPts[mit2].SegId);
                            perpsUNK.Add(refPts[mit2].SegId);
                        }
                    }
                    mit2++;
                }

                // now find all intersesctions between active DB segs and active UNK segs

                foreach (var dit in activeDBsegs)
                {
                    foreach (var uit in activeUNKsegs)
                    {
                        double
                            s1, s2, // parameter of intersection along database contour
                            t1; // parameter of intersection along unknown contour
                        PointF
                            p1, p2; // intersection point(s) .. 2 if segmetns overlap
                        int n;      // number of intersection pts

                        // if n is 2 after call to intersect it means the two segments have
                        // an overlap zone coincident to the same line and p1 and p2 are the
                        // ends of that segment

                        //cout << "testing segs db:" << *dit << " unk:" << *uit << endl;

                        if (!segments[dit].IntersectingSegs.Contains(uit))
                        {
                            IntPoint iPt = new IntPoint();
                            Intersect(ref segments, c1, c2, dit, uit, out s1, out t1, out p1, out s2, out p2, out n);
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

                                iPt.ParamDb = s1;
                                iPt.ParamUnk = t1;
                                iPt.Pt = p1;
                                intersectPoints.Add(iPt);
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
                                iPt.ParamDb = s1;
                                iPt.ParamUnk = t1;
                                iPt.Pt = p1;
                                intersectPoints.Add(iPt);

                                //iPt.param = s2;
                                //iPt.pt = p2;
                                //intersectPoints.push_back(iPt);
                            }
                            else if ((intersectPoints.Count == 0) && (n == 0))
                                Trace.WriteLine("ERROR in area: first interest point MISSED");
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


                foreach (var p in perpsDB)
                    activeDBsegs.Remove(p);

                perpsDB.Clear();

                foreach (var p in perpsUNK)
                    activeUNKsegs.Remove(p);
                perpsUNK.Clear();
            }

            // push last point common to both contours
            //int_point iPt;
            var lastCommon = new IntPoint();
            lastCommon.ParamDb = end2;
            lastCommon.ParamUnk = end1;
            lastCommon.Pt = c1[end1];
            intersectPoints.Add(lastCommon);

            // now calculate the area between the contours
            double sum = 0.0;

            for (int i = 0; i + 1 < intersectPoints.Count; i++) // index into intersectPoints
            {
                // sequence of points for area calc is from intersect point
                // then IN order through database contour points to next intersect point
                // then in reverse order through unknown points to first intersect point

                List<PointF> points = new List<PointF>();
                points.Add(intersectPoints[i].Pt);

                int lo, hi;

                // limits of index alond database contour between intersect points
                if (Math.Floor(intersectPoints[i].ParamDb) == intersectPoints[i].ParamDb)
                    lo = Convert.ToInt32(intersectPoints[i].ParamDb + 1);
                else
                    lo = Convert.ToInt32(Math.Ceiling(intersectPoints[i].ParamDb));

                if (Math.Ceiling(intersectPoints[i + 1].ParamDb) == intersectPoints[i + 1].ParamDb)
                    hi = Convert.ToInt32(intersectPoints[i + 1].ParamDb - 1);
                else
                    hi = Convert.ToInt32(Math.Floor(intersectPoints[i + 1].ParamDb));

                int j;

                for (j = lo; j <= hi; j++)
                    points.Add(c2[j]); // database contour points between intersections

                points.Add(intersectPoints[i + 1].Pt); // push next intersect point

                // limits of index alond unknown contour between intersect points
                if (Math.Ceiling(intersectPoints[i + 1].ParamUnk) == intersectPoints[i + 1].ParamUnk)
                    hi = Convert.ToInt32(intersectPoints[i + 1].ParamUnk - 1);
                else
                    hi = Convert.ToInt32(Math.Floor(intersectPoints[i + 1].ParamUnk));

                if (Math.Floor(intersectPoints[i].ParamUnk) == intersectPoints[i].ParamUnk)
                    lo = Convert.ToInt32(intersectPoints[i].ParamUnk + 1);
                else
                    lo = Convert.ToInt32(Math.Ceiling(intersectPoints[i].ParamUnk));

                for (j = hi; j >= lo; j--)
                    points.Add(c1[j]); // unknown contour points between intersections

                // last point is same as initial point

                // sum += polygonArea(points); // original method - just the sum of areas

                double area = PolygonArea(points);
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
        public static double AreaBasedErrorBetweenOutlineSegments_NEW(
                FloatContour c1, // mapped unknown fin 
                int begin1,
                int mid1,
                int end1,
                FloatContour c2, // envenly spaced database fin //***0005CM
                int begin2,
                int mid2,
                int end2)
        {
            // find length of database fin outline
            double dbArcLength = 0.0;
            for (int k = 1; k < c2.Length; k++)
            {
                double dx = c2[k].X - c2[k - 1].X;
                double dy = c2[k].Y - c2[k - 1].Y;
                if ((begin2 < k) && (k <= end2))
                    dbArcLength += Math.Sqrt(dx * dx + dy * dy);
            }

            double area;

            // NOTE: the Vector-based attempt to speed the process was actually slower,
            // and had bugs I could not isolate ... so eliminate this call for now - JHS (4/30/07)
            // area  = findAreaBetweenOutlineSegments_Vec(c1,begin1,mid1,c2,begin2,mid2);
            // area += findAreaBetweenOutlineSegments_Vec(c1,mid1,end1,c2,mid2,end2);

            area = FindAreaBetweenOutlineSegments(c1, begin1, mid1, c2, begin2, mid2);
            area += FindAreaBetweenOutlineSegments(c1, mid1, end1, c2, mid2, end2);

            double returnVal = area / dbArcLength;
            // cout << "AreaError: " << returnVal << endl;

            return (returnVal);
        }

    }
}
