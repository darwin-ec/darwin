//*******************************************************************
//   file: Match.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (8/2/2005)
//         -- reformatting of code and addition of comment blocks
//         -- incorporation of Outline code (spring 2005)
//         -- new matching and mapping methods
//
//*******************************************************************

using Darwin.Database;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace Darwin.Matching
{
    public class Match
    {
        private const int IGNORE_MID_POSIT = 0;
        protected delegate double ErrorBetweenOutlinesDelegate(
                                    FloatContour c1, // mapped unknown fin 
                                    int begin1,
                                    int mid1,
                                    int end1,
                                    FloatContour c2, // envenly spaced database fin
                                    int begin2,
                                    int mid2,
                                    int end2);

        protected ErrorBetweenOutlinesDelegate errorBetweenOutlines;

        DatabaseFin mUnknownFin;
        DarwinDatabase mDatabase;
        protected int mCurrentFin;
        MatchResults mMatchResults;

        //MatchingDialog* mMatchingDialog; // 043MA

        //Options* mOptions; // 054

        protected int
            mUnknownTipPosition,
            mUnknownNotchPosition,
            mUnknownBeginLE,
            mUnknownEndLE,
            mUnknownEndTE;

        protected PointF
            mUnknownTipPositionPoint,
            mUnknownNotchPositionPoint,
            mUnknownBeginLEPoint,
            mUnknownEndLEPoint,
            mUnknownEndTEPoint;

        //*******************************************************************
        //
        // Match::Match(...)
        //
        //    CONSTRUCTOR
        //
        public Match(
                DatabaseFin unknownFin,
                DarwinDatabase db)
        {
            if (unknownFin == null)
                throw new ArgumentNullException(nameof(unknownFin));

            if (db == null)
                throw new ArgumentNullException(nameof(db));

            mUnknownFin = new DatabaseFin(unknownFin);
            mDatabase = db;

            mCurrentFin = 0;

            mMatchResults = new MatchResults(unknownFin.IDCode);

            errorBetweenOutlines = MeanSquaredErrorBetweenOutlineSegments;
            // errorBetweenOutlines(meanSquaredErrorBetweenOutlineSegments) //***1.85 -- vc++6.0
            // errorBetweenOutlines(&Match::meanSquaredErrorBetweenOutlineSegments) //***1.85 -- vc++2011

            mUnknownTipPosition = mUnknownFin.FinOutline.GetFeaturePoint(FeaturePointType.Tip); //***008OL
            mUnknownNotchPosition = mUnknownFin.FinOutline.GetFeaturePoint(FeaturePointType.Notch); //***008OL
            mUnknownBeginLE = mUnknownFin.FinOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeBegin); //***008OL
            mUnknownEndLE = mUnknownFin.FinOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeEnd); //***008OL
            mUnknownEndTE = mUnknownFin.FinOutline.GetFeaturePoint(FeaturePointType.PointOfInflection); //***008OL

            mUnknownTipPositionPoint = mUnknownFin.FinOutline.GetFeaturePointCoords(FeaturePointType.Tip); //***008OL
            mUnknownBeginLEPoint = mUnknownFin.FinOutline.GetFeaturePointCoords(FeaturePointType.LeadingEdgeBegin); //***008OL
            mUnknownEndLEPoint = mUnknownFin.FinOutline.GetFeaturePointCoords(FeaturePointType.LeadingEdgeEnd); //***008OL
            mUnknownNotchPositionPoint = mUnknownFin.FinOutline.GetFeaturePointCoords(FeaturePointType.Notch); //***008OL
            mUnknownEndTEPoint = mUnknownFin.FinOutline.GetFeaturePointCoords(FeaturePointType.PointOfInflection); //***008OL

            // just a pointer to the dialog for display purposes, this will be set
            // to point to the actual dialog IF and WHEN display is desired
            //mMatchingDialog = NULL;
        }


        //*******************************************************************
        //
        // void Match::displayOutlinesHere(MatchingDialog *mDialog)
        //
        //    parameter is a pointer to the dialog where outlines are to be
        //    displayed during matching (NULL for no display)
        //
        //void Match::setDisplay(MatchingDialog* mDialog)
        //{
        //	mMatchingDialog = mDialog;
        //}

        //*******************************************************************
        //
        // float Match::matchSingleFin(int registrationMethod, int regSegmentsUsed, 
        //                             bool categoryToMatch[], bool useFullFinError,
        //                             bool useAbsoluteOffsets)
        //
        //    Attempts to match unknown fin to a single database fin.  The
        //    current fin is mDatabase->getItem(mCurrentFin). The match_method 
        //    parameter determines which of several outline mapping and
        //    error between match techniques is used.
        //
        //    now modified to match ONLY those database fins with category designation
        //    indicated in categoryToMatch array
        //
        //    ***055ER
        //    the new useFullFinError parameter indicates whether or not to use the
        //    entire outlines in the final calculation of the error measure.  When false,
        //    only the portion of the outlines between the adjusted leadingeEdgeBegin and
        //    trailingEdgeEnd points is used. When true, the fin mapping is done using 
        //    various subsets of the outlines (and errors derived from same), but the
        //    final error (measure of mismatch) uses all outline points from each fin.
        //    This parameter does not affect the original and the trim by 1/10th percent
        //    matching techniques, which always use the entire outline in error calculations
        //
        //    ***1.3
        //    New parameter useAbsoluteOffsets, wen true, causes function to step through
        //    the database absolute offset list so that fins are matched in the actual
        //    order that they are stored in the database.  This is used ONLY when MATCH
        //    QUEUES are being processed, and it keeps the fin numbers correct even if the
        //    database is later modified.
        //
        public float MatchSingleFin(RegistrationMethodType registrationMethod, int regSegmentsUsed,
                                    List<SelectableDBDamageCategory> categoryToMatch, bool useFullFinError,
                                    bool useAbsoluteOffsets)
        {
            DatabaseFin thisDBFin;

            if (useAbsoluteOffsets)
            {
                // there may be holes in the absolute offset list so we loop until
                // a non NULL fin is returned or until we reach the end of the list
                do
                {
                    if (mCurrentFin >= (int)mDatabase.sizeAbsolute())
                        return 100.0f;

                    thisDBFin = mDatabase.getItemAbsolute(mCurrentFin);

                    if (null == thisDBFin)
                        mCurrentFin++;
                }
                while (null == thisDBFin);
            }
            else
            {
                if (mCurrentFin >= (int)mDatabase.size())
                    return 100.0f;

                thisDBFin = mDatabase.GetItem(mCurrentFin);
            }


            bool tryMatch = categoryToMatch.Exists(c => c.Name.ToLower() == thisDBFin.DamageCategory.ToLower());

            if (tryMatch)
            {
                float timeTaken;
                MseInfo result = new MseInfo();
                bool gotResult = true; // assume this unless error in switch below

                //***043 JHS - select version of error finding function
                switch (registrationMethod)
                {
                    case RegistrationMethodType.Original3Point:
                        // use beginning of leading edge, tip and largest trailing notch
                        // to map unknown outline to database outline, and then use
                        // version of meanSqError... that trims leading and trailing
                        // edge points to equalize number of points on eaach contour,
                        // and finally compute error between "corresponding" pairs of
                        // mapped points
                        result = FindErrorBetweenFins_Original3Point(thisDBFin, out timeTaken);
                        break;
                    case RegistrationMethodType.TrimFixedPercent:
                        // use a series of calls to meanSqError..., each with different amounts
                        // of the leading edge of each fin "ignored" in order to find the BEST
                        // choice of "leading edge beginning point" correspondence.  This prevents
                        // "bulging" of outlines due to long or short placement of the
                        // beginning of the trace.  Also, the version of meanSqError... used
                        // is one that walks the unknown fin outline point by point, and 
                        // computes the "closest point" on the database outline by finding
                        // the intersection of a perpendicular from the unknown outline point.
                        // This helps minimize errors due to nonuniform point spacing created
                        // during the mapping process.
                        result = FindErrorBetweenFins(thisDBFin, out timeTaken);
                        break;
                    /*removed - JHS
								case TRIM_OPTIMAL :
									// use an optimization process (essentially Newton-Raphson) to
									// shorten the leading edges of each fin to produce a correspondence
									// that yeilds the BEST match.  A fin Outline walking approach
									// is used to conpute the meanSqError....
									result = Match::findErrorBetweenFinsJHS(
												thisDBFin, timeTaken, regSegmentsUsed,
												useFullFinError);
									break;
					*/
                    case RegistrationMethodType.TrimOptimalTotal:
                        // use an optimization process (essentially Newton-Raphson) to
                        // shorten the leading AND trailing edges of each fin to produce a correspondence
                        // that yeilds the BEST match.  A fin Outline walking approach
                        // is used to compute the meanSqError....
                        //errorBetweenOutlines = meanSquaredErrorBetweenOutlineSegments; //***1.85 -- vc++6.0
                        errorBetweenOutlines = MeanSquaredErrorBetweenOutlineSegments; //***1.85 -- vc++2011
                                                                                       // NOTE: error function MUST be set prior to following call
                        result = FindErrorBetweenFinsOptimal(
                                    thisDBFin, out timeTaken, /*regSegmentsUsed, */
                                    false, false,
                                    useFullFinError);
                        break;
                    case RegistrationMethodType.TrimOptimalTip:
                        //errorBetweenOutlines = meanSquaredErrorBetweenOutlineSegments; //***1.85 -- vc++6.0
                        errorBetweenOutlines = MeanSquaredErrorBetweenOutlineSegments; //***1.85 -- vc++2011
                                                                                       // NOTE: error function MUST be set prior to following call
                        result = FindErrorBetweenFinsOptimal(
                                    thisDBFin, out timeTaken, /*regSegmentsUsed, */
                                    true, false,
                                    useFullFinError);
                        break;
                    case RegistrationMethodType.TrimOptimalArea: //***1.85 - new area based metric option
                                                                   //errorBetweenOutlines = areaBasedErrorBetweenOutlineSegments; //***1.85 -- vc++6.0
                        errorBetweenOutlines = AreaBasedErrorBetweenOutlineSegments; //***1.85 -- vc++2011
                                                                                     // NOTE: error function MUST be set prior to following call
                        result = FindErrorBetweenFinsOptimal(
                                    thisDBFin, out timeTaken, /*regSegmentsUsed, */
                                    true, false,
                                    useFullFinError);
                        break;
                    case RegistrationMethodType.TrimOptimalInOut:
                    case RegistrationMethodType.TrimOptimalInOutTip:
                    default:
                        gotResult = false;
                        break;
                }

                if (gotResult)
                {
                    double errorBetweenFins = result.Error; //***005CM

                    // Now, store the result
                    string errorTemp = string.Format("{0:0.00}", errorBetweenFins);

                    Result r = new Result(
                        result.C1, //***005CM
                        result.C2, //***005CM
                        thisDBFin.ImageFilename,  //***001DB
                        thisDBFin.ThumbnailPixmap, //***1.0
                        thisDBFin.ThumbnailRows,    //***1.0
                        mCurrentFin,
                        errorTemp,
                        thisDBFin.IDCode,
                        thisDBFin.Name,
                        thisDBFin.DamageCategory,
                        thisDBFin.DateOfSighting,
                        thisDBFin.LocationCode);

                    //***1.1 - set indices of beginning, tip and end points used in mapping
                    r.SetMappingControlPoints(
                        result.B1, result.T1, result.E1,  // beginning, tip & end of unknown fin
                        result.B2, result.T2, result.E2); // beginning, tip & end of database fin

                    mMatchResults.AddResult(r);

                    //delete thisDBFin; ///***1.6 - old location, did not delete fins not tried
                }
            }

            mCurrentFin++;

            if (useAbsoluteOffsets)
            {
                if (mCurrentFin == (int)mDatabase.sizeAbsolute())
                    return 1.0f;

                return (float)mCurrentFin / mDatabase.sizeAbsolute();
            }

            if (mCurrentFin == (int)mDatabase.size())
                return 1.0f;

            return (float)mCurrentFin / mDatabase.size();
        }


        //*******************************************************************
        //
        // MatchResults* Match::getMatchResults()
        //
        //    Simply return the pointer to the matching results (NOT A COPY).
        //
        //	MatchResults* Match::getMatchResults()
        //{
        //		return mMatchResults;
        //	}


        //*******************************************************************
        // 
        // mseInfo Match::findErrorBetweenFins_Original3Point(...)
        //
        //    ORIGINAL findErrorBetweenFins
        //    Invoked using ORIGINAL_3_POINT mathing_method
        //
        private MseInfo FindErrorBetweenFins_Original3Point(
                DatabaseFin dbFin,
                out float timeTaken)
        {
            //TODO
            timeTaken = 0f;
            if (null == dbFin)
                throw new ArgumentNullException(nameof(dbFin));

            int dbTipPosition, dbBeginLE, dbNotchPosition;

            dbTipPosition = dbFin.FinOutline.GetFeaturePoint(FeaturePointType.Tip); //***008OL
            dbBeginLE = dbFin.FinOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeBegin); //***008OL
            dbNotchPosition = dbFin.FinOutline.GetFeaturePoint(FeaturePointType.Notch); //***008OL

            PointF
                dbTipPositionPoint,
                dbBeginLEPoint,
                dbNotchPositionPoint;
            //temp; //***005CM for unsure code section below

            dbTipPositionPoint = dbFin.FinOutline.GetFeaturePointCoords(FeaturePointType.Tip); //***008OL
            dbBeginLEPoint = dbFin.FinOutline.GetFeaturePointCoords(FeaturePointType.LeadingEdgeBegin); //***008OL
            dbNotchPositionPoint = dbFin.FinOutline.GetFeaturePointCoords(FeaturePointType.Notch); //***008OL

            var mappedContour = mUnknownFin.FinOutline.ChainPoints.MapContour(
                    mUnknownTipPositionPoint,
                    mUnknownBeginLEPoint,
                    mUnknownNotchPositionPoint,
                    dbTipPositionPoint,
                    dbBeginLEPoint,
                    dbNotchPositionPoint);

            /* JHS 4/28/2004
             *
             * 1 - the FloatContours (database and unknown) are already normalized
             *     and evenly spaced
             * 2 - the tip position has not moved in either FloatContour.
             *     It is in the same location as it was in the original contours
             *     since we are NOT evenly re-spacing either after the mapping.  
             */

            FloatContour floatDBContour = new FloatContour(dbFin.FinOutline.ChainPoints);

            //***1.5 - since contour may get trimmed by error function, create temp copies here
            FloatContour
                c1 = new FloatContour(mappedContour),
                c2 = new FloatContour(floatDBContour);

            MseInfo results = MeanSquaredErrorBetweenOutlines_Original( //***005CM
                                                                        //mappedContour,
                    c1, //***1.5
                    mUnknownFin.FinOutline.GetFeaturePoint(FeaturePointType.Tip),
                    //floatDBContour, 
                    c2,
                    dbTipPosition);

            //***1.5 - set key feature point locations
            results.C1 = mappedContour;
            results.B1 = mUnknownBeginLE;
            results.T1 = mUnknownTipPosition;
            results.E1 = mUnknownEndTE;
            results.C2 = floatDBContour;
            results.B2 = dbBeginLE;
            results.T2 = dbTipPosition;
            results.E2 = dbFin.FinOutline.GetFeaturePoint(FeaturePointType.PointOfInflection);

            // both evenly spaced contours are returned as part of results
            return results; //***005C
        }


        //*******************************************************************
        //
        // mseInfo Match::findErrorBetweenFins(...)
        //
        //    Finds error between two fin outlines.  Uses a series possible
        //    "shortenings" of the leading edge on each fin to find the "best"
        //    correspondence between leading edge beginning points. method is
        //    described in code below, but it essentially tests one fin's entire
        //    outline against the other with none 1/20, 2/20, 3/20, 4/20, 
        //    5/20 or 6/20 of the leading edge trimmed away. This trimming is 
        //    performed symmetrically and the best correspondence (least error)
        //    of the 13 trials is returned.
        //
        public MseInfo FindErrorBetweenFins(
                DatabaseFin dbFin,
                out float timeTaken)
        {
            //TODO
            timeTaken = 0f;
            if (null == dbFin)
                throw new ArgumentNullException(nameof(dbFin));

            //Chain *dbChain = new Chain(dbFin->mFinContour, 3.0); removed ***008OL

            int dbTipPosition, dbBeginLE, dbNotchPosition;

            dbTipPosition = dbFin.FinOutline.GetFeaturePoint(FeaturePointType.Tip); //***008OL
            dbBeginLE = dbFin.FinOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeBegin); //***008OL
            dbNotchPosition = dbFin.FinOutline.GetFeaturePoint(FeaturePointType.Notch); //***008OL

            int dbEndTE = dbFin.FinOutline.GetFeaturePoint(FeaturePointType.PointOfInflection); //***055ER

            PointF
                dbTipPositionPoint,
                dbBeginLEPoint,
                dbNotchPositionPoint;
            //temp; //***005CM for unsure code section below

            dbTipPositionPoint = dbFin.FinOutline.GetFeaturePointCoords(FeaturePointType.Tip); //***008OL
            dbBeginLEPoint = dbFin.FinOutline.GetFeaturePointCoords(FeaturePointType.LeadingEdgeBegin); //***008OL
            dbNotchPositionPoint = dbFin.FinOutline.GetFeaturePointCoords(FeaturePointType.Notch); //***008OL

            FloatContour floatDBContour = new FloatContour(dbFin.FinOutline.ChainPoints); //***006CM, 008OL

            //***008OL new strategy 
            // follows strategy of stepping along database fin and finding "closest"
            // point on unknown contour.  It computes the error 7 times and returns the
            // smallest mean squared error.  The 7 contour walks use different start and 
            // stop points.
            //
            // walk 0 : entire unknown outline
            // walk 1 : skips first 1/20 of unknown fin leading edge
            // walk 2 : skips first 1/20 of database fin leading edge
            // walk 3 : skips first 2/20 of unknown fin leading edge
            // walk 4 : skips first 2/20 of database fin leading edge
            // walk 5 : skips first 3/20 of unknown fin leading edge
            // walk 6 : skips first 3/20 of database fin leading edge
            // walk 7 : skips first 4/20 of unknown fin leading edge
            // walk 8 : skips first 4/20 of database fin leading edge
            // walk 9 : skips first 5/20 of unknown fin leading edge
            // walk 10: skips first 5/20 of database fin leading edge
            // walk 11: skips first 6/20 of unknown fin leading edge
            // walk 12: skips first 6/20 of database fin leading edge
            //
            // reasoning : since the error is highly dependent on the ill defined 
            // beginning of the leading edge, we try several points

            FloatContour preMapUnknown = new FloatContour(mUnknownFin.FinOutline.ChainPoints);

            double newError; //***1.0LK

            // 1/20th distance up each leading edge is basic adjustment
            int oneFractionUnk = (mUnknownTipPosition - mUnknownBeginLE) / 20;
            int oneFractionDB = (dbTipPosition - dbBeginLE) / 20;
            int startLeadUnk = 0, startLeadDB = 0;
            PointF startLeadUnkPt, startLeadDBPt;

            // set up default error
            MseInfo results = new MseInfo();
            results.C2 = floatDBContour; //***1.0LK - this doesn't change so set it here
            results.Error = 50000.0;
            //***1.5 - initialize key feature point locations
            results.B1 = mUnknownBeginLE;
            results.T1 = mUnknownTipPosition;
            results.E1 = mUnknownEndTE;
            results.B2 = dbBeginLE;
            results.T2 = dbTipPosition;
            results.E2 = dbEndTE;

            Trace.WriteLine("matching unk " + mUnknownFin.IDCode + " to DB " + dbFin.IDCode);

            //***055ER - found and fixed adjustment of Leading Edge Begin 
            // now all points prior to mUnknownBeginLE and dbBeginLE are ignored

            for (int matchNum = 0; matchNum < 13; matchNum++)
            {
                switch (matchNum)
                {
                    case 0: // match entire unknown fin to entire database fin
                        startLeadUnk = mUnknownBeginLE;
                        startLeadDB = dbBeginLE;
                        break;
                    case 1: // match without first 1/20 unknown leading edge
                        startLeadUnk = mUnknownBeginLE + oneFractionUnk;
                        startLeadDB = dbBeginLE;
                        break;
                    case 2: // match without first 1/20 database leading edge
                        startLeadUnk = mUnknownBeginLE;
                        startLeadDB = dbBeginLE + oneFractionDB;
                        break;
                    case 3: // match without first 2/20 unknown leading edge
                        startLeadUnk = mUnknownBeginLE + 2 * oneFractionUnk;
                        startLeadDB = dbBeginLE;
                        break;
                    case 4: // match without first 2/20 database leading edge
                        startLeadUnk = mUnknownBeginLE;
                        startLeadDB = dbBeginLE + 2 * oneFractionDB;
                        break;
                    case 5: // match without first 3/20 unknown leading edge
                        startLeadUnk = mUnknownBeginLE + 3 * oneFractionUnk;
                        startLeadDB = dbBeginLE;
                        break;
                    case 6: // match without first 3/20 database leading edge
                        startLeadUnk = mUnknownBeginLE;
                        startLeadDB = dbBeginLE + 3 * oneFractionDB;
                        break;
                    case 7: // match without first 4/20 unknown leading edge
                        startLeadUnk = mUnknownBeginLE + 4 * oneFractionUnk;
                        startLeadDB = dbBeginLE;
                        break;
                    case 8: // match without first 4/20 database leading edge
                        startLeadUnk = mUnknownBeginLE;
                        startLeadDB = dbBeginLE + 4 * oneFractionDB;
                        break;
                    case 9: // match without first 5/20 unknown leading edge
                        startLeadUnk = mUnknownBeginLE + 5 * oneFractionUnk;
                        startLeadDB = dbBeginLE;
                        break;
                    case 10: // match without first 5/20 database leading edge
                        startLeadUnk = mUnknownBeginLE;
                        startLeadDB = dbBeginLE + 5 * oneFractionDB;
                        break;
                    case 11: // match without first 6/20 unknown leading edge
                        startLeadUnk = mUnknownBeginLE + 6 * oneFractionUnk;
                        startLeadDB = dbBeginLE;
                        break;
                    case 12: // match without first 6/20 database leading edge
                        startLeadUnk = mUnknownBeginLE;
                        startLeadDB = dbBeginLE + 6 * oneFractionDB;
                        break;
                        //default:
                        /* nothing here */
                        //;
                }

                // beginning of leading edge point to use for this match 
                startLeadUnkPt = preMapUnknown[startLeadUnk];
                startLeadDBPt = floatDBContour[startLeadDB];

                var mappedContour = preMapUnknown.MapContour(
                        mUnknownTipPositionPoint,
                        startLeadUnkPt,
                        mUnknownNotchPositionPoint,
                        dbTipPositionPoint,
                        startLeadDBPt,
                        dbNotchPositionPoint);

                newError = MeanSquaredErrorBetweenOutlineSegments(
                            mappedContour,
                            startLeadUnk,
                            IGNORE_MID_POSIT, //***1.85
                            mUnknownEndTE,
                            floatDBContour,
                            startLeadDB,
                            IGNORE_MID_POSIT, //***1.85
                            dbEndTE);

                //***055ER - new approach walks both outlines and averages error
                //           also, points past End of Trailing Edge point are ignored
                //mseInfo newResults;

                /*
                            // old way using contour walking and perpendicular intersector
                            newResults = meanSquaredErrorBetweenOutlines(
                                        mappedContour,startLeadUnk,mUnknownEndTE,
                                        floatDBContour,startLeadDB,dbEndTE);	
                */
                /**/
                // new way using contour walk and ratio of arc lengths
                //newResults.c1 = mappedContour;
                //newResults.c2 = floatDBContour;
                //newResults.error = meanSquaredErrorBetweenOutlineSegments/*Medial*/(
                //			mappedContour,startLeadUnk,mUnknownEndTE,
                //			floatDBContour,startLeadDB,dbEndTE);	
                /*
                            // try finding a better end point pairing, rather than letting one end dangle

                            int unkEndPos,dbEndPos; // positions of "closest" end point 
                            // find point on database fin trailing edge that is "closest" to end of unknown fin
                            floatDBContour->findPositionOfClosestPoint(
                                (*mappedContour)[mUnknownEndTE].x,
                                (*mappedContour)[mUnknownEndTE].y,
                                dbEndPos);
                            // correct for wildly sweeping fits where closest point is on leading edge
                            if (dbEndPos <= startLeadDB)
                                dbEndPos = dbEndTE;
                            // find point on unknown traling edge that is "closest" to end of database fin
                            mappedContour->findPositionOfClosestPoint(
                                (*floatDBContour)[dbEndTE].x,
                                (*floatDBContour)[dbEndTE].y,
                                unkEndPos);
                            // correct for wildly sweeping fits where closest point is on leading edge
                            if (dbEndPos <= startLeadDB)
                                unkEndPos = mUnknownEndTE;

                            { // use a local scope here to avoid name conflicts with temp vars
                                double 
                                    // dist from database original end to closer unknown end
                                    dx1 = (*floatDBContour)[dbEndTE].x - (*mappedContour)[unkEndPos].x,
                                    dy1 = (*floatDBContour)[dbEndTE].y - (*mappedContour)[unkEndPos].y,
                                    dist1 = (dx1*dx1 + dy1*dy1),
                                    // dist from unknown original end to closer database end
                                    dx2 = (*floatDBContour)[dbEndPos].x - (*mappedContour)[mUnknownEndTE].x,
                                    dy2 = (*floatDBContour)[dbEndPos].y - (*mappedContour)[mUnknownEndTE].y,
                                    dist2 = (dx2*dx2 + dy2*dy2);

                                //if (dist1 < dist2)
                                    newResults.error = meanSquaredErrorBetweenOutlineSegments(
                                        mappedContour,startLeadUnk,unkEndPos,
                                        floatDBContour,startLeadDB,dbEndTE);	
                                //else
                                    double other = meanSquaredErrorBetweenOutlineSegments(
                                        mappedContour,startLeadUnk,mUnknownEndTE,
                                        floatDBContour,startLeadDB,dbEndPos);	
                                    if (other < newResults.error)
                                        newResults.error = other;
                            }
                */

                /**/
                /*
                            newResults.error = 0.5 * (
                                        newResults.error +
                                        meanSquaredErrorBetweenOutlineSegments(
                                            floatDBContour,startLeadDB,dbEndTE,
                                            mappedContour,startLeadUnk,mUnknownEndTE));
                */

                // TODO
                //if (mMatchingDialog != NULL)
                //{
                //	// show the display of the outline registration in the dialog
                //	mMatchingDialog->showOutlines(mappedContour, floatDBContour);
                //}

                //***1.0LK - this if-else revised to fix memory leaks - JHS
                if (0 == matchNum)
                {
                    // initialize database contour and error for result
                    results.C1 = mappedContour;
                    results.Error = newError;
                    //***1.5 - set shifted feature point locations
                    results.B1 = startLeadUnk;
                    results.B2 = startLeadDB;
                }
                else if (newError < results.Error)
                {
                    //***1.0LK - delete existing mapped contour and replace with new
                    // mapped contour and error
                    results.C1 = mappedContour;
                    results.Error = newError;
                    //***1.5 - set shifted feature point locations
                    results.B1 = startLeadUnk;
                    results.B2 = startLeadDB;
                }
                else
                {
                    //***1.0LK - delete the most recently mapped contour - we don't need it
                    //delete mappedContour;
                }
            }

            //delete preMapUnknown; //***1.0LK

            // both evenly spaced contours are returned as part of results
            return results; //***005C

            //***008OL this should now be a symmetric error regardles of which
            // leading edge is shorter

        }


        //*******************************************************************
        //
        // double Match::meanSquaredErrorBetweenChains(...)
        //
        //    finds mean squared error between two chain codes.  
        //    NO LONGER USED.
        //
        private double MeanSquaredErrorBetweenChains(
    Chain chain1,
    int index1,
    Chain chain2,
    int index2)
        {
            if (chain1 == null)
                throw new ArgumentNullException(nameof(chain1));

            if (chain2 == null)
                throw new ArgumentNullException(nameof(chain2));

            if (index1 < 0 || index1 >= chain1.Length)
                throw new ArgumentOutOfRangeException(nameof(index1));

            if (index2 < 0 || index2 >= chain2.Length)
                throw new ArgumentOutOfRangeException(nameof(index2));

            int
                len1 = chain1.Length,
                len2 = chain2.Length,

                trailLen1 = len1 - index1,
                trailLen2 = len2 - index2,
                trailLen,

                leadLen;

            if (trailLen1 < trailLen2)
                trailLen = trailLen1;
            else
                trailLen = trailLen2;

            if (index1 < index2)
                leadLen = index1;
            else
                leadLen = index2;

            int comparisonLength = leadLen + trailLen - 2;

            if (comparisonLength <= 0)
                return 5000.0;

            double sum = 0.0, temp;

            int i, j, k;

            for (i = index1 - leadLen + 2, j = index2 - leadLen + 2, k = 0; k < comparisonLength; i++, j++, k++)
            {
                temp = chain1.RelativeData[i] - chain2.RelativeData[j];
                sum += temp * temp;
            }

            // Note that we've already tested for division by zero.
            return (sum / (double)comparisonLength);

        }


        //*******************************************************************
        //
        // mseInfo Match::meanSquaredErrorBetweenOutlines_Original(...)
        //
        //    Original version of mean squared error function.  It trims leading
        //    and trainlg edge points to form outlines of equal numbers of points
        //    on either side of TIP.  It then computes the mean sqared error between
        //    corresponding (those with same index) points on each outline.
        //
        //    ***008OL created a new version of this function that "walks" the 
        //    database fin outline and computes the "closest" point on the unknown
        //    fin outline for error computation.  See below.
        //
        private MseInfo MeanSquaredErrorBetweenOutlines_Original( //***005CM
                FloatContour c1,
                int tip1,
                FloatContour c2, //***0005CM
                int tip2)
        {
            // Results structure
            MseInfo results = new MseInfo(); //***005Cm

            //***005CM new code begins here
            int leadLen, trailLen;
            int diff;

            // Now we are about to identify the fin with fewer points on the LE
            // and crop the longer one by the difference.

            // Find the shorter LE
            if (tip1 < tip2)
            {
                diff = tip2 - tip1;
                c2.PopFront(diff);
                leadLen = tip1;
            }
            else if (tip2 < tip1)
            {
                diff = tip1 - tip2;
                c1.PopFront(diff);
                leadLen = tip2;
            }
            else // Just in case they're equal
                leadLen = tip1;
            //***005CM end of new code

            // leading edges are of equal length now
            int
                len1 = c1.Length,
                len2 = c2.Length,
                trailLen1 = len1 - leadLen, //***005CM
                trailLen2 = len2 - leadLen; //***005CM

            //***005CM re-normalizing here is not needed -- better matches without it

            if (trailLen1 < trailLen2)
                trailLen = trailLen1;
            else
                trailLen = trailLen2;

            int comparisonLength = leadLen + trailLen - 2;

            if (comparisonLength <= 1)
            {
                results.Error = 5000.0; //***005CM set default value
                results.C1 = c1; //***005CM set default value
                results.C2 = c2; //***005CM set default value
                return results;  //***005CM
            }

            double sum = 0.0;

            int i, j, k;

            for (i = 2, j = 2, k = 0; k < comparisonLength; i++, j++, k++)
            { //***005CM
                sum += (c1[i].X - c2[j].X) * (c1[i].X - c2[j].X) +
                       (c1[i].Y - c2[j].Y) * (c1[i].Y - c2[j].Y);
            }

            // Assign Contours ..
            results.C1 = c1; //***005CM
            results.C2 = c2; //***005CM

            // Note that we've already tested for division by zero.
            results.Error = (sum / (double)comparisonLength);

            return results; //***005CM
        }


        //*******************************************************************
        //
        // mseInfo Match::meanSquaredErrorBetweenOutlines(...)
        //
        //    ***008OL NEW version of function.  It "walks" the database fin.
        //    At each point it computes an intersection of a perpendicular 
        //    to the database fin outline at that point and the unknown fin
        //    outline to dtermine the "closest" corresponding unknown fin
        //    point. The mean sqred error is computed based on the distances
        //    between ALL such pairs of points.
        //
        private MseInfo MeanSquaredErrorBetweenOutlines( //***005CM
                FloatContour c1, // mapped unknown fin 
                int start1,
                int end1,
                FloatContour c2, // envenly spaced database fin //***0005CM
                int start2,
                int end2)
        {
            // Results structure
            MseInfo results = new MseInfo(); //***005Cm

            // this process walks the database contour and at each point finds the
            // closest point along the edge sequence defining the unknown outline
            // (these may or may not be float contour points on the unknown fin)
            // the error measure computed is thus a measure of the distance between
            // outlines not the distance between similarly indexed contour points

            // we walk the database fin because the unknown has been mapped and 
            // therefore the spacing of sample points is no longer uniform on the
            // unknown.  The database fin outline should be evenly spaced at approx.
            // 3.0 unit intervals

            // both contours are examined in their respective index ranges
            // [start1,end1] and [start2,end2]

            double
                x = c1[start1].X,
                y = c1[start1].Y;

            double dx1, dy1; // components of tangent to database outline at point	
            double dx2, dy2; // conponents of current segment on unknown fin
            double beta;    // parameter on unknown fin outline segment where it
                            // intersects the perpendicular through the database outline

            double sqErrorPt2Pt; // current point to point error squared

            double sum = 0.0;
            int ptsFound = 0;
            int c1skips = 0, c2skips = 0; // count jumps forward on non-point-pair event
            //bool ptPairFound = false;

            bool prevPtInSegment = false; // previous point has been found in THIS segment of unknown
            bool prevPtExists = false;     // some previous point has been found (use 1st in unknown initially)
            double betaPrev = 0.0;        // beta of previous point
            int repeatPtUsed = 0;         // number of times previous point is used in error calculation

            int numErrBelow3 = 0;         // count pairs with error dist below 3.0 (sqErr < 9.0)
                                          // i is index on database 
                                          // j is index on unknown
            int i = start2 + 1, j = start1 + 1;

            // not done if at least 3 points remain on each contour
            //bool morePts = ((i+1 < c2->length()) && (j+1 < c1->length())); // old method
            bool morePts = ((i + 1 <= end2) && (j + 1 <= end1));

            // set default values
            results.Error = 50000.0; //***005CM set default value
            results.C1 = c1; //***005CM set default value
            results.C2 = c2; //***005CM set default value

            // compute the mean squared error
            while (morePts)
            {
                // for debugging
                float x1 = c1[j - 1].X;  // coords of unknown point j-1
                float y1 = c1[j - 1].Y;
                float x2 = c2[i].X;    // coords of database point i
                float y2 = c2[i].Y;

                // tangent to database fin outline at point i
                dx2 = c2[i + 1].X - c2[i - 1].X;
                dy2 = c2[i + 1].Y - c2[i - 1].Y;
                // slope of unknown fin outline segment between points j-1 and j 
                dx1 = c1[j].X - c1[j - 1].X;
                dy1 = c1[j].Y - c1[j - 1].Y;

                // parameter value of intersection point between perpendicular to
                // database fin at point i and the unknonwn outline segment between
                // points j-1 and j
                beta = betaPrev;
                if ((dx1 * dx2 + dy1 * dy2) != 0.0)
                    beta = -(dx2 * (c1[j - 1].X - c2[i].X) + dy2 * (c1[j - 1].Y - c2[i].Y))
                            / (dx1 * dx2 + dy1 * dy2);

                if ((0.0 <= beta) && (beta <= 1.0))
                {
                    // do not backtrack prior to last point found in segment
                    if (prevPtInSegment && (beta < betaPrev))
                        beta = betaPrev;

                    // good intersection
                    x = c1[j - 1].X + beta * dx1;
                    y = c1[j - 1].Y + beta * dy1;

                    sqErrorPt2Pt = ((x - c2[i].X) * (x - c2[i].X) +
                                    (y - c2[i].Y) * (y - c2[i].Y));

                    if (sqErrorPt2Pt < 9.0)
                        numErrBelow3++;

                    //***041 new strategy -- progressive error increase for pt pairs far apart
                    //if (sqErrorPt2Pt > 36.0)
                    //	sqErrorPt2Pt *= 2.0; // double the error for pt pairs more than 6.0 units apart

                    sum += sqErrorPt2Pt;

                    ptsFound++; // count the point

                    i++; // advance to next point on database contour

                    betaPrev = beta; // remember this point

                    prevPtExists = true;
                    prevPtInSegment = true;
                }
                else if (beta < 0.0)
                {
                    // not a good intersection within the current unknown segment
                    // intersection is in a previous segment
                    if (prevPtExists)
                    {
                        // use previous point in THIS segment if it exists
                        // note: x & y are still the last point found and used
                        sqErrorPt2Pt = ((x - c2[i].X) * (x - c2[i].X) +
                                        (y - c2[i].Y) * (y - c2[i].Y));

                        if (sqErrorPt2Pt < 9.0)
                            numErrBelow3++;

                        sum += sqErrorPt2Pt;

                        ptsFound++; // count the point
                        repeatPtUsed++;

                        i++; // advance to next point on database contour
                    }
                    else
                    {
                        // intersection is before segment and no previous point has ever
                        // been found, so advance position along database fin
                        i++;

                        c2skips++;
                    }
                }
                else // beta > 1.0
                {
                    // intersection is past current segement so advance to next
                    // segment along unknown fin
                    j++;

                    c1skips++;
                    prevPtInSegment = false;
                }

                // see if we are done (ie, we have run out of points or segments
                //morePts = ((i+1 < c2->length()) && (j+1 < c1->length())); // old method
                morePts = ((i + 1 <= end2) && (j + 1 <= end1));


            }

            // if no points found, the error stays the default
            if (ptsFound > 0)
                results.Error = (sum / (double)ptsFound);


            string traceOut = string.Format("pairs={0}  S2E1(unk)={1} s2E2(db)={2} repeats={3} below3={4} err={5}f",
                ptsFound, end1 + 1 - start1, end2 + 1 - start2,
                   repeatPtUsed, numErrBelow3, results.Error);
            Trace.WriteLine(traceOut);

            return results; //***005CM
        }


        //***************************begin****************************************
        //
        // mseInfo Match::findErrorBetweenFinsOptimal(
        //
        //    ***0051 - new function to register fins by selectively shortening
        //    leading and trailing edges based on best of 4 options at each step
        //    in process.
        //
        private MseInfo FindErrorBetweenFinsOptimal(
                DatabaseFin dbFin,
                out float timeTaken,
                //int regSegmentsUsed,
                bool moveTip, //***1.1
                bool moveEndsInAndOut, //***1.1
                bool useFullFinError) //***055ER
        {
            //TODO
            timeTaken = 0f;
            if (dbFin == null)
                throw new ArgumentNullException(nameof(dbFin));


            //***1.85 - new function pointer to generalize algorithm
            // errorBetweenOutlines = meanSquaredErrorBetweenOutlineSegments;
            // this is actually set prior to calling this function

            int
                dbTipPosition,
                dbBeginLE,
                dbNotchPosition,
                dbEndTE;

            dbTipPosition = dbFin.FinOutline.GetFeaturePoint(FeaturePointType.Tip);
            dbBeginLE = dbFin.FinOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeBegin);
            dbNotchPosition = dbFin.FinOutline.GetFeaturePoint(FeaturePointType.Notch);
            dbEndTE = dbFin.FinOutline.GetFeaturePoint(FeaturePointType.PointOfInflection);

            PointF
                dbTipPositionPoint,
                dbBeginLEPoint,
                dbNotchPositionPoint,
                dbEndTEPoint;

            dbTipPositionPoint = dbFin.FinOutline.GetFeaturePointCoords(FeaturePointType.Tip);
            dbBeginLEPoint = dbFin.FinOutline.GetFeaturePointCoords(FeaturePointType.LeadingEdgeBegin);
            dbNotchPositionPoint = dbFin.FinOutline.GetFeaturePointCoords(FeaturePointType.Notch);
            dbEndTEPoint = dbFin.FinOutline.GetFeaturePointCoords(FeaturePointType.PointOfInflection);

            FloatContour floatDBContour = new FloatContour(dbFin.FinOutline.ChainPoints);

            FloatContour preMapUnknown = new FloatContour(mUnknownFin.FinOutline.ChainPoints);

            FloatContour
                mappedContour = null,
                shortenedDBMappedContour = null,
shortenedUnkMappedContour = null,
shiftedUnkTipMappedContour = null; //***1.1

            MseInfo results = new MseInfo();

            //--------------- new strategy
            // 0 - set jumpSize to 16% of leading edge length
            // 1 - compute leading edge error using entire remaining leading edge of each fin,
            //     and trailing edge error using entire remaining trailing edge of each fin
            // 2 - shorten each leading edge by 1% & compare to entire other fin;
            //     shorten each trailing edge by 1% & compare to entire other fin
            // 3 - choose best of five actions based on minimum error
            //     -- if full:full is minimum error, then done
            //     -- if shortened leading edge is best then reduce DB or UNK leading edge by JumpSize
            //     -- if shortened trailing edge is best then reduce DB or UNK leading edge by JumpSize
            // 4 - compute new error for "sortened" leading edge vs other
            // 5 - if result of (4) smaller than (1) then repeat beginning at (2)
            //     else halve jumpSize and repeat beginning at (4)
            // 

            // 1% and jumpSize for each leading edge
            int onePercentUnk = (int)((mUnknownTipPosition - mUnknownBeginLE) / 100.0);
            int onePercentDB = (int)((dbTipPosition - dbBeginLE) / 100.0);
            int testIncUnk = onePercentUnk * 8; //***1.5 - for decreasing size test increment
            int testIncDB = onePercentDB * 8;   //***1.5 - for decreasing size test increment
            int jumpSizeUnk = 16 * onePercentUnk;
            int jumpSizeDB = 16 * onePercentDB;
            //int tipMoveThreshold = 4 * onePercentUnk; //***1.1
            int tipMoveThreshold = 16 * onePercentUnk; //***1.8 - let tip move freely

            int movedTipShift = 0; //***1.1 - amount to shift unknown Tip on given iteration

            int
                startLeadUnk,
                startLeadDB,
                endTrailUnk,
                endTrailDB,
                movedTipUnk; //***1.1

            // actual end points for registration, depends on regSegmentsUsed
            /*int 
                endDB,
                endUnk;
            */

            PointF
                startLeadUnkPt,
                startLeadDBPt,
                endTrailUnkPt,
                endTrailDBPt;
            //movedTipUnkPt; //***1.1

            double
                error,                     // best so far
                shortenedDBLeadError,      // for test at 1% shorter DB Leading Edge
                shortenedUnkLeadError,     // for test at 1% shorter Unknown Leading Edge
                shortenedDBTrailError,     // for test at 1% shorter DB Trailing Edge
                shortenedUnkTrailError,    // for test at 1% shorter Unknown Trailing Edge
                movedTipUnkError = 0;          // for test at 1% Tip shift on Unknown

            int jumpingOn = 0;   // 1=DBLead,2=UnkLead,3=DBTrail,4=UnkTrail,5=UnkTip

            Trace.WriteLine("matching unk " + mUnknownFin.IDCode + " to DB " + dbFin.IDCode);

            // set initial starting points on each leading edge
            startLeadUnk = mUnknownBeginLE;
            startLeadDB = dbBeginLE;

            // map unknown to database fin and compute leading edge error 
            //startLeadUnkPt = (*preMapUnknown)[startLeadUnk];
            //startLeadDBPt = (*floatDBContour)[startLeadDB];

            // fix the range of segments used in error calculation by setting the END point 
            endTrailUnk = mUnknownEndTE;
            endTrailDB = dbEndTE;

            // set initial position of unknown TIP
            movedTipUnk = mUnknownFin.FinOutline.GetFeaturePoint(FeaturePointType.Tip); //***1.1

            //***055OP
            // find inital lengths (# of points) for each outline
            // the process cannot shrink either outline by more than 50%
            int
                dbLen = endTrailDB - startLeadDB + 1,
                unkLen = endTrailUnk - startLeadUnk + 1;

            // create initial mapping, using ENTIRE fin contour

            mappedContour = preMapUnknown.MapContour(
                    mUnknownTipPositionPoint,
                    preMapUnknown[startLeadUnk],
                    preMapUnknown[endTrailUnk],
                    dbTipPositionPoint,
                    floatDBContour[startLeadDB],
                    floatDBContour[endTrailDB]);

            error = errorBetweenOutlines(
                    mappedContour,
                    startLeadUnk,
                    movedTipUnk, //***1.85
                    endTrailUnk,
                    floatDBContour,
                    startLeadDB,
                    dbTipPosition, //***1.85
                    endTrailDB);

            // for dumping info on shifting indices - leave out of release
            //cout << "Matching " << dbFin->getID() << " e: " << error << endl;

            //***1.5
            // if fins already closely aligned, then halve the test & jump increments
            if (error < 250.0)
            {
                if (testIncUnk > onePercentUnk)
                {
                    testIncUnk /= 2;
                    jumpSizeUnk /= 2;
                }
                if (testIncDB > onePercentDB)
                {
                    testIncDB /= 2;
                    jumpSizeDB /= 2;
                }
            }

            // TODO
            //if (mMatchingDialog != NULL)
            //{
            // show the display of the outline registration in the dialog
            //	mMatchingDialog->showOutlines(mappedContour, floatDBContour);

            // this hook is for screen capturing first alignments - JHS
            //char dummy;
            //cin >> dummy;

            //}

            // now try shortenning one or the other of the leading or trailing edges to 
            // find a more optimal mapping (one with smaller error)

            bool foundBest = false;
            bool skipThisJump = false; //***1.1

            while (!foundBest)
            {
                // shorten DATABASE leading edge by 1% and test error

                if (null != shortenedDBMappedContour)
                {
                    shortenedDBMappedContour = null;
                }

                shortenedDBMappedContour = preMapUnknown.MapContour(
                                            //mUnknownTipPositionPoint,    //***1.1
                                            preMapUnknown[movedTipUnk], //***1.1 - only changes if (moveTip == true)
                                            preMapUnknown[startLeadUnk],
                                            preMapUnknown[endTrailUnk],
                                            dbTipPositionPoint,
                                            floatDBContour[startLeadDB +/*onePercentDB*/testIncDB], //***1.5
                                            floatDBContour[endTrailDB]);

                shortenedDBLeadError = errorBetweenOutlines(
                        shortenedDBMappedContour,
                        startLeadUnk,
                        movedTipUnk, //***1.85
                        endTrailUnk,
                        floatDBContour,
                        startLeadDB +/*onePercentDB*/testIncDB, //***1.5
                        dbTipPosition, //***1.85
                        endTrailDB);

                // TODO
                //if (mMatchingDialog != NULL)
                //{
                //	// show the display of the outline registration in the dialog
                //	mMatchingDialog->showOutlines(shortenedDBMappedContour, floatDBContour);
                //}

                // shorten UNKNOWN leading edge by 1% and test error

                if (null != shortenedUnkMappedContour)
                {
                    shortenedUnkMappedContour = null;
                }

                shortenedUnkMappedContour = preMapUnknown.MapContour(
                        //mUnknownTipPositionPoint,    //***1.1
                        preMapUnknown[movedTipUnk], //***1.1 - only changes if (moveTip == true)
                        preMapUnknown[startLeadUnk +/*onePercentUnk*/testIncUnk], //***1.5
                        preMapUnknown[endTrailUnk],
                        dbTipPositionPoint,
                        floatDBContour[startLeadDB],
                        floatDBContour[endTrailDB]);

                shortenedUnkLeadError = errorBetweenOutlines(
                        shortenedUnkMappedContour,
                        startLeadUnk +/*onePercentUnk*/testIncUnk, //***1.5
                        movedTipUnk, //***1.85
                        endTrailUnk,
                        floatDBContour,
                        startLeadDB,
                        dbTipPosition, //***1.85
                        endTrailDB);

                // TODO
                //if (mMatchingDialog != NULL)
                //{
                //	// show the display of the outline registration in the dialog
                //	mMatchingDialog->showOutlines(shortenedUnkMappedContour, floatDBContour);
                //}

                // shorten DATABASE trailing edge by 1% and test error

                if (null != shortenedDBMappedContour)
                {
                    shortenedDBMappedContour = null;
                }

                shortenedDBMappedContour = preMapUnknown.MapContour(
                        //mUnknownTipPositionPoint,    //***1.1
                        preMapUnknown[movedTipUnk], //***1.1 - only changes if (moveTip == true)
                        preMapUnknown[startLeadUnk],
                        preMapUnknown[endTrailUnk],
                        dbTipPositionPoint,
                        floatDBContour[startLeadDB],
                        floatDBContour[endTrailDB -/*onePercentDB*/testIncDB]); //***1.5

                shortenedDBTrailError = errorBetweenOutlines(
                        shortenedDBMappedContour,
                        startLeadUnk,
                        movedTipUnk, //***1.85
                        endTrailUnk,
                        floatDBContour,
                        startLeadDB,
                        dbTipPosition, //***1.85
                        endTrailDB -/*onePercentDB*/testIncDB); //***1.5

                // TODO
                //if (mMatchingDialog != NULL)
                //{
                //	// show the display of the outline registration in the dialog
                //	mMatchingDialog->showOutlines(shortenedDBMappedContour, floatDBContour);
                //}

                // shorten UNKNOWN trailing edge by 1% and test error

                if (null != shortenedUnkMappedContour)
                {
                    //delete shortenedUnkMappedContour;
                    shortenedUnkMappedContour = null;
                }

                shortenedUnkMappedContour = preMapUnknown.MapContour(
                        //mUnknownTipPositionPoint,    //***1.1
                        preMapUnknown[movedTipUnk], //***1.1 - only changes if (moveTip == true)
                        preMapUnknown[startLeadUnk],
                        preMapUnknown[endTrailUnk -/*onePercentUnk*/testIncUnk], //***1.5
                        dbTipPositionPoint,
                        floatDBContour[startLeadDB],
                        floatDBContour[endTrailDB]);

                shortenedUnkTrailError = errorBetweenOutlines(
                        shortenedUnkMappedContour,
                        startLeadUnk,
                        movedTipUnk, //***1.85
                        endTrailUnk -/*onePercentUnk*/testIncUnk, //***1.5
                        floatDBContour,
                        startLeadDB,
                        dbTipPosition, //***1.85
                        endTrailDB);

                // TODO
                //if (mMatchingDialog != NULL)
                //{
                //	// show the display of the outline registration in the dialog
                //	mMatchingDialog->showOutlines(shortenedUnkMappedContour, floatDBContour);
                //}

                // shift UNKNOWN tip by 1% and compute error (test both directions)

                if (moveTip && (jumpSizeUnk <= tipMoveThreshold))
                {
                    double shift2LeadError, shift2TrailError;

                    // shift Tip toward LEBegin

                    if (null != shiftedUnkTipMappedContour)
                    {
                        shiftedUnkTipMappedContour = null;
                    }

                    shiftedUnkTipMappedContour = preMapUnknown.MapContour(
                            preMapUnknown[movedTipUnk -/*onePercentUnk*/testIncUnk], //***1.5
                            preMapUnknown[startLeadUnk],
                            preMapUnknown[endTrailUnk],
                            dbTipPositionPoint,
                            floatDBContour[startLeadDB],
                            floatDBContour[endTrailDB]);

                    shift2LeadError = errorBetweenOutlines(
                            shiftedUnkTipMappedContour,
                            startLeadUnk,
                            movedTipUnk - testIncUnk, //***1.85
                            endTrailUnk,
                            floatDBContour,
                            startLeadDB,
                            dbTipPosition, //***1.85
                            endTrailDB);

                    // shift Tip toward TEEnd

                    if (null != shiftedUnkTipMappedContour)
                    {
                        shiftedUnkTipMappedContour = null;
                    }

                    shiftedUnkTipMappedContour = preMapUnknown.MapContour(
                            preMapUnknown[movedTipUnk +/*onePercentUnk*/testIncUnk], //***1.5
                            preMapUnknown[startLeadUnk],
                            preMapUnknown[endTrailUnk],
                            dbTipPositionPoint,
                            floatDBContour[startLeadDB],
                            floatDBContour[endTrailDB]);

                    shift2TrailError = errorBetweenOutlines(
                            shiftedUnkTipMappedContour,
                            startLeadUnk,
                            movedTipUnk + testIncUnk, //***1.85
                            endTrailUnk,
                            floatDBContour,
                            startLeadDB,
                            dbTipPosition, //***1.85
                            endTrailDB);

                    // keep best shift to compare to end shifts

                    if (shift2LeadError < shift2TrailError)
                    {
                        movedTipUnkError = shift2LeadError;
                        movedTipShift = -jumpSizeUnk;
                    }
                    else
                    {
                        movedTipUnkError = shift2TrailError;
                        movedTipShift = jumpSizeUnk;
                    }
                }

                // positions of point we are jumping to as new leading edge start points
                int
                    jumpStartLeadDB = 0,
                    jumpStartLeadUnk = 0,
                    jumpEndTrailDB = 0,
                    jumpEndTrailUnk = 0,
                    jumpShiftTipUnk = 0;

                skipThisJump = false; // will be true only if skipping jump code to consider moving tip

                // decide whether to jump farther in one direction or to quit
                if (!moveTip)
                {
                    if ((error <= shortenedDBLeadError) && (error <= shortenedUnkLeadError) &&
                        (error <= shortenedDBTrailError) && (error <= shortenedUnkTrailError))
                    {
                        //***1.5 - either we scale down the test and jump sizes OR we are done
                        // scale down size of unknown test intervals
                        if (testIncUnk > onePercentUnk)
                        {
                            testIncUnk /= 2;
                            jumpSizeUnk /= 2;
                            skipThisJump = true;
                        }
                        // scale down size of database test intervals
                        if (testIncDB > onePercentDB)
                        {
                            testIncDB /= 2;
                            jumpSizeDB /= 2;
                            skipThisJump = true;
                        }
                        // we are done if no interval decreases were needed
                        if (!skipThisJump)
                            foundBest = true;
                        //***1.5 - end new section

                        /* old code
                        // startLeadDB, startLeadUnk, endTrailDB & endTrailUnk are already correctly set
                        foundBest = true;
                        */
                    }
                    else
                    {
                        if ((shortenedDBLeadError <= shortenedUnkLeadError) &&
                            (shortenedDBLeadError <= shortenedDBTrailError) &&
                            (shortenedDBLeadError <= shortenedUnkTrailError))
                        {
                            // shorten DB fin leading edge by jump size
                            jumpStartLeadDB = startLeadDB + jumpSizeDB;
                            jumpStartLeadUnk = startLeadUnk;
                            jumpEndTrailDB = endTrailDB;
                            jumpEndTrailUnk = endTrailUnk;
                            jumpShiftTipUnk = movedTipUnk;

                            jumpingOn = 1;
                        }
                        else if ((shortenedUnkLeadError <= shortenedDBTrailError) &&
                                 (shortenedUnkLeadError <= shortenedUnkTrailError))
                        {
                            // shorten unknown fin leading edge by jump size
                            jumpStartLeadDB = startLeadDB;
                            jumpStartLeadUnk = startLeadUnk + jumpSizeUnk;
                            jumpEndTrailDB = endTrailDB;
                            jumpEndTrailUnk = endTrailUnk;
                            jumpShiftTipUnk = movedTipUnk;

                            jumpingOn = 2;
                        }
                        else if (shortenedDBTrailError <= shortenedUnkTrailError)
                        {
                            // shorten DB fin trailing edge by jump size
                            jumpStartLeadDB = startLeadDB;
                            jumpStartLeadUnk = startLeadUnk;
                            jumpEndTrailDB = endTrailDB - jumpSizeDB;
                            jumpEndTrailUnk = endTrailUnk;
                            jumpShiftTipUnk = movedTipUnk;

                            jumpingOn = 3;
                        }
                        else
                        {
                            // shorten unknown fin trailing edge by jump size
                            jumpStartLeadDB = startLeadDB;
                            jumpStartLeadUnk = startLeadUnk;
                            jumpEndTrailDB = endTrailDB;
                            jumpEndTrailUnk = endTrailUnk - jumpSizeUnk;
                            jumpShiftTipUnk = movedTipUnk;

                            jumpingOn = 4;
                        }
                    }

                }
                else // moveTip == true
                {
                    if ((error <= shortenedDBLeadError) && (error <= shortenedUnkLeadError) &&
                        (error <= shortenedDBTrailError) && (error <= shortenedUnkTrailError) &&
                        ((error <= movedTipUnkError) || (jumpSizeUnk > tipMoveThreshold)))
                    {
                        //***1.5 - either we scale down the test and jump sizes OR we are done
                        // scale down size of unknown test intervals
                        if (testIncUnk > onePercentUnk)
                        {
                            testIncUnk /= 2;
                            jumpSizeUnk /= 2;
                            skipThisJump = true;
                        }
                        // scale down size of database test intervals
                        if (testIncDB > onePercentDB)
                        {
                            testIncDB /= 2;
                            jumpSizeDB /= 2;
                            skipThisJump = true;
                        }
                        // we are done if no interval decreases were needed
                        if (!skipThisJump)
                            foundBest = true;
                        //***1.5 - end new section

                        /* old code
                        if (jumpSizeUnk <= tipMoveThreshold)
                        {
                            // both test intervals are at or below one percent AND
                            // startLeadDB, startLeadUnk, endTrailDB & endTrailUnk & movedTipUnk
                            // are already correctly set
                            foundBest = true;
                        }
                        else
                        {
                            // we have not considered moving tip yet, so decrease jumpSizeUnk
                            // and force process through another iteration
                            jumpSizeUnk = tipMoveThreshold;

                            // no jump improves match this time
                            skipThisJump = true; 
                        }
                        */
                    }
                    else
                    {
                        if ((shortenedDBLeadError <= shortenedUnkLeadError) &&
                            (shortenedDBLeadError <= shortenedDBTrailError) &&
                            (shortenedDBLeadError <= shortenedUnkTrailError) &&
                            ((shortenedDBLeadError <= movedTipUnkError) ||
                             (jumpSizeUnk > tipMoveThreshold)))
                        {
                            // shorten DB fin leading edge by jump size
                            jumpStartLeadDB = startLeadDB + jumpSizeDB;
                            jumpStartLeadUnk = startLeadUnk;
                            jumpEndTrailDB = endTrailDB;
                            jumpEndTrailUnk = endTrailUnk;
                            jumpShiftTipUnk = movedTipUnk;

                            jumpingOn = 1;
                        }
                        else if ((shortenedUnkLeadError <= shortenedDBTrailError) &&
                                 (shortenedUnkLeadError <= shortenedUnkTrailError) &&
                                 ((shortenedUnkLeadError <= movedTipUnkError) ||
                                  (jumpSizeUnk > tipMoveThreshold)))
                        {
                            // shorten unknown fin leading edge by jump size
                            jumpStartLeadDB = startLeadDB;
                            jumpStartLeadUnk = startLeadUnk + jumpSizeUnk;
                            jumpEndTrailDB = endTrailDB;
                            jumpEndTrailUnk = endTrailUnk;
                            jumpShiftTipUnk = movedTipUnk;

                            jumpingOn = 2;
                        }
                        else if ((shortenedDBTrailError <= shortenedUnkTrailError) &&
                                 ((shortenedDBTrailError <= movedTipUnkError) ||
                                  (jumpSizeUnk > tipMoveThreshold)))
                        {
                            // shorten DB fin trailing edge by jump size
                            jumpStartLeadDB = startLeadDB;
                            jumpStartLeadUnk = startLeadUnk;
                            jumpEndTrailDB = endTrailDB - jumpSizeDB;
                            jumpEndTrailUnk = endTrailUnk;
                            jumpShiftTipUnk = movedTipUnk;

                            jumpingOn = 3;
                        }
                        else if ((shortenedUnkTrailError <= movedTipUnkError) ||
                                  (jumpSizeUnk > tipMoveThreshold))
                        {
                            // shorten unknown fin trailing edge by jump size
                            jumpStartLeadDB = startLeadDB;
                            jumpStartLeadUnk = startLeadUnk;
                            jumpEndTrailDB = endTrailDB;
                            jumpEndTrailUnk = endTrailUnk - jumpSizeUnk;
                            jumpShiftTipUnk = movedTipUnk;

                            jumpingOn = 4;
                        }
                        else // ((movedTipUnkError < shortenedUnkTrailError) &&
                             //  (jumpSizeUnk <= tipMoveThreshold))
                        {
                            // move unknown fin tip by jump size
                            jumpStartLeadDB = startLeadDB;
                            jumpStartLeadUnk = startLeadUnk;
                            jumpEndTrailDB = endTrailDB;
                            jumpEndTrailUnk = endTrailUnk;

                            jumpShiftTipUnk = movedTipUnk + movedTipShift;

                            jumpingOn = 5;
                        }
                    }
                }

                if ((!foundBest) && (!skipThisJump))
                {
                    // make a big jump in direction of indicated improvement

                    FloatContour jumpMappedContour = null;
                    double jumpError;
                    bool goodJump = false;

                    do
                    {
                        // we only end up here more than once when the jump has been too far
                        // in which case there is a contour to be deleted

                        if (null != jumpMappedContour)
                        {
                            jumpMappedContour = null;
                        }

                        jumpMappedContour = preMapUnknown.MapContour(
                                                    //mUnknownTipPositionPoint,
                                                    preMapUnknown[jumpShiftTipUnk], //***1.1
                                                    preMapUnknown[jumpStartLeadUnk],
                                                    preMapUnknown[jumpEndTrailUnk],
                                                    dbTipPositionPoint,
                                                    floatDBContour[jumpStartLeadDB],
                                                    floatDBContour[jumpEndTrailDB]);

                        jumpError = errorBetweenOutlines(
                                   jumpMappedContour,
                                jumpStartLeadUnk,
                                jumpShiftTipUnk, //***1.85
                                jumpEndTrailUnk,
                                floatDBContour,
                                jumpStartLeadDB,
                                dbTipPosition, //***1.85
                                jumpEndTrailDB);

                        //TODO
                        //if (mMatchingDialog != NULL)
                        //{
                        //	// show the display of the outline registration in the dialog
                        //	mMatchingDialog->showOutlines(jumpMappedContour, floatDBContour);
                        //}

                        if (jumpError < error)
                        {
                            // good jump, so set up for next test and jump
                            startLeadUnk = jumpStartLeadUnk;
                            startLeadDB = jumpStartLeadDB;
                            endTrailDB = jumpEndTrailDB;
                            endTrailUnk = jumpEndTrailUnk;
                            movedTipUnk = jumpShiftTipUnk; //***1.1

                            error = jumpError;
                            goodJump = true;
                            /*
                            // output jumps of indices - leave out of release version
                            cout << "jump U:b " << startLeadUnk
                                << " t " << movedTipUnk
                                << " e " << endTrailUnk
                                << " D:b " << startLeadDB
                                << " e " << endTrailDB << endl;
                            */
                            //***1.5 - catch it here for outline image capture
                            /*
                            char dummy;
                            cin >> dummy;
                            */
                        }
                        else
                            switch (jumpingOn)
                            {
                                case 1: //jumped on database leading edge
                                        // jumped too far in right direction, so back up on database fin
                                    jumpSizeDB = jumpSizeDB / 2;
                                    if ((testIncDB > onePercentDB) ||
                                        ((testIncDB == jumpSizeDB) && (testIncDB > 1)))
                                        testIncDB = testIncDB / 2; //***1.5
                                    jumpStartLeadDB = startLeadDB + jumpSizeDB;
                                    break;
                                case 2: // jumped on unknown leading edge
                                        // jumped too far in right direction, so back up on unknown
                                    jumpSizeUnk = jumpSizeUnk / 2;
                                    if ((testIncUnk > onePercentUnk) ||
                                        ((testIncUnk == jumpSizeUnk) && (testIncUnk > 1)))
                                        testIncUnk = testIncUnk / 2; //***1.5
                                    jumpStartLeadUnk = startLeadUnk + jumpSizeUnk;
                                    break;
                                case 3: //jumped on database trailing edge
                                        // jumped too far in right direction, so back up on database fin
                                    jumpSizeDB = jumpSizeDB / 2;
                                    if ((testIncDB > onePercentDB) ||
                                        ((testIncDB == jumpSizeDB) && (testIncDB > 1)))
                                        testIncDB = testIncDB / 2; //***1.5
                                    jumpEndTrailDB = endTrailDB - jumpSizeDB;
                                    break;
                                case 4: //jumped on unknown trailing edge
                                        // jumped too far in right direction, so back up on unknown
                                    jumpSizeUnk = jumpSizeUnk / 2;
                                    if ((testIncUnk > onePercentUnk) ||
                                        ((testIncUnk == jumpSizeUnk) && (testIncUnk > 1)))
                                        testIncUnk = testIncUnk / 2; //***1.5
                                    jumpEndTrailUnk = endTrailUnk - jumpSizeUnk;
                                    break;
                                case 5: //jumped on unknown tip shift
                                        // jumped too far in right direction, so back up on database fin
                                    jumpSizeUnk = jumpSizeUnk / 2;
                                    if ((testIncUnk > onePercentUnk) ||
                                        ((testIncUnk == jumpSizeUnk) && (testIncUnk > 1)))
                                        testIncUnk = testIncUnk / 2; //***1.5
                                                                     // do the following with movedTipShift,
                                                                     // since it is signed to indicate shift direction
                                    movedTipShift = movedTipShift / 2;
                                    jumpShiftTipUnk = movedTipUnk + movedTipShift;
                                    break;
                                default:
                                    // should never end up here
                                    break;
                            }

                    }
                    while (!goodJump && (jumpSizeDB > 0) && (jumpSizeUnk > 0));

                    // free the jumpMappedCotour now that we are done
                    if (null != jumpMappedContour)
                    {
                        jumpMappedContour = null;
                    }

                    if ((jumpSizeDB == 0) || (jumpSizeUnk == 0))
                    {
                        // we've run out of space to adjust so we've found the best
                        foundBest = true;
                    }

                    //***055OP - new test to limit amount of shrinkage to 50% of original length
                    //***1.5 - restrict this to 65% now
                    if ((0.65 > (endTrailDB - startLeadDB + 1) / (double)dbLen) ||
                        (0.65 > (endTrailUnk - startLeadUnk + 1) / (double)unkLen))
                    {
                        // we've run out of space to adjust so we've found the best
                        foundBest = true;
                    }

                }

            }

            // beginning of leading edge point to use for FINAL MATCH
            // using TOTAL outlines, not just leading edges
            startLeadUnkPt = preMapUnknown[startLeadUnk];
            startLeadDBPt = floatDBContour[startLeadDB];

            // this will be the notch unless 2nd phase executed
            endTrailUnkPt = preMapUnknown[endTrailUnk];
            endTrailDBPt = floatDBContour[endTrailDB];

            mappedContour = preMapUnknown.MapContour(
                    //mUnknownTipPositionPoint,
                    preMapUnknown[movedTipUnk], //***1.1
                    startLeadUnkPt,
                    endTrailUnkPt, // changed
                    dbTipPositionPoint,
                    startLeadDBPt,
                    endTrailDBPt); // changed

            // set coutour pointers
            results.C1 = mappedContour;
            results.C2 = floatDBContour;

            //***1.1 - set shifted feature point locations
            results.B1 = startLeadUnk;
            results.T1 = movedTipUnk;
            results.E1 = endTrailUnk;
            results.B2 = startLeadDB;
            results.T2 = dbTipPosition;
            results.E2 = endTrailDB;

            //***055ER - allow use of entire fin outlines in last determination
            // of error measure.  This should produce error more in line with visual
            // registration of fins
            //***1.5 - new meaning associated with this parameter
            // useFullFinError == true means to use entire outline between "adjusted and mapped"
            // "start" and "end" points
            // useFullFinError == false means use only the trailing edge (tip to end)
            if (useFullFinError)
            {
                //cout << "Full Fin Error computed!\n";
                results.Error = errorBetweenOutlines(
                        mappedContour,
                        //mUnknownBeginLE, // old limits
                        //mUnknownEndTE,   // old limits
                        startLeadUnk,
                        movedTipUnk, //***1.85
                        endTrailUnk,
                        floatDBContour,
                        //dbBeginLE,       // old limits
                        //dbEndTE);        // old limits
                        startLeadDB,
                        dbTipPosition, //***1.85
                        endTrailDB);
            }
            else
            {
                //cout << "Trailing Edge Only Error computed!\n";
                results.Error = errorBetweenOutlines(
                        mappedContour,
                        movedTipUnk,    //***1.5 - testing trailing edge only for final rankings
                        movedTipUnk, //***1.85
                        endTrailUnk,
                        floatDBContour,
                        dbTipPosition,  //***1.5 - testing trailing edge only for final rankings
                        dbTipPosition, //***1.85
                        endTrailDB);
            }

            // TODO
            //if (mMatchingDialog != NULL)
            //{
            //	// show the display of the outline registration in the dialog
            //	mMatchingDialog->showOutlines(mappedContour, floatDBContour);
            //	Trace.WriteLine("DONE");
            //}

            /*
                    // for now this is removed - JHS
                    // experiment more in next version

                    //***1.85 - output area based error for grins

                    double areaError = areaBasedErrorBetweenOutlineSegments_NEW( // leading edge
                                mappedContour,
                                startLeadUnk,
                                movedTipUnk,
                                endTrailUnk,
                                floatDBContour,
                                startLeadDB,
                                dbTipPosition,
                                endTrailDB);
                    g_print("\nAreaErr = %8.1f\n", areaError);
                    results.error = areaError; //***1.75 - look see

            */

            // both float contours are returned as part of results
            // the DB fin coutour is evenly spaced and the unknown coutour
            // is mapped after having been evenly spaced

            return results;
        }
        //*************************** end ****************************************


        //*******************************************************************
        //
        // double Match::meanSquaredErrorBetweenOutlineSegments(...)
        //
        //    Just computes the error between defined outline segments.
        //    Returns the error as a double, since contours will be
        //    remapped prior to final calculation of meanSquaredErrorBetweenFins.
        //
        private double MeanSquaredErrorBetweenOutlineSegmentsNew(
                FloatContour c1, // mapped unknown fin 
              int start1,
                int tip1,
                FloatContour c2, // envenly spaced database fin //***0005CM
              int start2,
                int tip2)
        {

            double error;

            // this process walks the database contour and at each point finds the
            // closest point along the edge sequence defining the unknown outline
            // (these may or may not be float contour points on the unknown fin)
            // the error measure computed is thus a measure of the distance between
            // outlines not the distance between similarly indexed contour points

            // we walk the database fin because the unknown has been mapped and 
            // therefore the spacing of sample points is no longer uniform on the
            // unknown.  The database fin outline should be evenly spaced at approx.
            // 3.0 unit intervals

            double
                x = c1[start1].X,
                y = c1[start1].Y;

            double dx1, dy1; // components of tangent to database outline at point
            double dx2, dy2; // conponents of current segment on unknown fin
            double beta;    // parameter on unknown fin outline segment where it
                            // intersects the perpendicular through the database outline

            double sqErrorPt2Pt; // current point to point error squared

            double sum = 0.0;
            int ptsFound = 0;
            int c1skips = 0, c2skips = 0; // count jumps forward on non-point-pair event

            bool prevPtInSegment = false; // previous point has been found in THIS segment of unknown
            bool prevPtExists = false;     // some previous point has been found (1st point in c1, initially)
            double betaPrev = 0.0;        // beta of previous point
            int repeatPtUsed = 0;         // number of times previous point is used in error calculation

            int numErrBelow3 = 0;         // count pairs with error dist below 3.0 (sqErr < 9.0)
                                          // i is index on database 
                                          // j is index on unknown
            int i = start2 + 1, j = start1 + 1;

            // not done if at least 3 points remain on each contour's leading edge
            bool morePts = ((i + 1 <= tip2) && (j + 1 <= tip1));

            // set default values
            error = 50000.0; //***005CM set default value

            // compute the mean squared error
            while (morePts)
            {
                // for debugging
                float x1 = c1[j - 1].X;  // coords of unknown point j-1
                float y1 = c1[j - 1].Y;
                float x2 = c2[i].X;    // coords of database point i
                float y2 = c2[i].Y;

                // tangent to database fin outline at point i
                dx2 = c2[i + 1].X - c2[i - 1].X;
                dy2 = c2[i + 1].Y - c2[i - 1].Y;
                // slope of unknown fin outline segment between points j-1 and j 
                dx1 = c1[j].X - c1[j - 1].X;
                dy1 = c1[j].Y - c1[j - 1].Y;

                // parameter value of intersection point between perpendicular to
                // database fin at point i and the unknonwn outline segment between
                // points j-1 and j
                beta = betaPrev;
                /***055ER - correcting so better mim dist from point to curve --
                    now using the constraint that the line from database point i to closest point
                    on unknown segment j-1 to j must be perpendicular to the unknown segment
                      if ((dx1 * dx2 + dy1 * dy2) != 0.0)
                         beta = - (dx2 * (c1[j-1].x - c2[i].x) + dy2 * (c1[j-1].y - c2[i].y))
                                / (dx1 * dx2 + dy1 * dy2) ;
                */
                if ((dx1 * dx1 + dy1 * dy1) != 0.0)
                    beta = -(dx1 * (c1[j - 1].X - c2[i].X) + dy1 * (c1[j - 1].Y - c2[i].Y))
                           / (dx1 * dx1 + dy1 * dy1);

                if ((0.0 <= beta) && (beta <= 1.0))
                {
                    // do not backtrack prior to last point found in segment
                    if (prevPtInSegment && (beta < betaPrev))
                        beta = betaPrev;

                    // good intersection
                    x = c1[j - 1].X + beta * dx1;
                    y = c1[j - 1].Y + beta * dy1;

                    //***055ER
                    // TODO
                    //if (mMatchingDialog != NULL)
                    //{
                    //	// show the display of the outline registration in the dialog
                    //	mMatchingDialog->showErrorPt2Pt(c1, c2, c2[i].x, c2[i].y, x, y);
                    //}

                    sqErrorPt2Pt = ((x - c2[i].X) * (x - c2[i].X) +
                                    (y - c2[i].Y) * (y - c2[i].Y));

                    if (sqErrorPt2Pt < 9.0)
                        numErrBelow3++;

                    //***041 new strategy -- progressive error increase for pt pairs far apart
                    //if (sqErrorPt2Pt > 36.0)
                    //   sqErrorPt2Pt *= 2.0; // double the error for pt pairs more than 3.0 units apart
                    /*
                    if (sqErrorPt2Pt > 36.0)
                       sqErrorPt2Pt *= 2.0; // double it again if pt pairs more than 6.0 units apart
                    */

                    sum += sqErrorPt2Pt;

                    ptsFound++; // count the point

                    i++; // advance to next point on database contour

                    betaPrev = beta; // remember this point

                    prevPtExists = true;
                    prevPtInSegment = true;
                }
                else if (beta < 0.0)
                {
                    // not a good intersection within the current unknown segment
                    // intersection is in a previous segment
                    if (prevPtExists)
                    {
                        //***055ER
                        //TODO
                        //if (mMatchingDialog != NULL)
                        //{
                        //	// show the display of the outline registration in the dialog
                        //	mMatchingDialog->showErrorPt2Pt(c1, c2, c2[i].x, c2[i].y, x, y);
                        //}
                        // use previous point in THIS segment if it exists
                        // note: x & y are still the last point found and used
                        sqErrorPt2Pt = ((x - c2[i].X) * (x - c2[i].X) +
                                        (y - c2[i].Y) * (y - c2[i].Y));

                        if (sqErrorPt2Pt < 9.0)
                            numErrBelow3++;

                        sum += sqErrorPt2Pt;

                        ptsFound++; // count the point
                        repeatPtUsed++;

                        i++; // advance to next point on database contour
                    }
                    else
                    {
                        // intersection is before segment and no previous point has ever
                        // been found, so advance position along database fin
                        i++;

                        c2skips++;
                    }
                }
                else // beta > 1.0
                {
                    // intersection is past current segement so advance to next
                    // segment along unknown fin
                    j++;

                    c1skips++;
                    prevPtInSegment = false;
                }

                // see if we are done (ie, we have run out of points or segments
                morePts = ((i + 1 <= tip2) && (j + 1 <= tip1));

            }

            // if no points found, the error stays the default
            if (ptsFound > 0)
                error = (sum / (double)ptsFound);


            var traceOut = string.Format("pairs={0}  S2E1(unk)={1} s2E2(db)={2} repeats={3} below3={4} err={5}",
                ptsFound, tip1 - start1, tip2 - start2,
                  repeatPtUsed, numErrBelow3, error);
            Trace.WriteLine(traceOut);

            return error;
        }

        /*
        //*******************************************************************
        //
        // double Match::meanSquaredErrorBetweenOutlineSegmentsNew(...)
        //
        //    REVISED: 11/14/05
        //    Computes the error between defined outline segments.
        //    Returns the error as a double.  It is assumed that the unknown
        //    has been mapped to the database fin outline prior to the call.
        //
        double Match::meanSquaredErrorBetweenOutlineSegments( 
                FloatContour *c1, // mapped unknown fin 
                int begin1,
                int end1,
                FloatContour *c2, // envenly spaced database fin //***0005CM
                int begin2,
                int end2)
        {

            double error;

            // this process walks the database contour and at each point finds the
            // closest point along the edge sequence defining the unknown outline
            // (these may or may not be float contour points on the unknown fin)
            // the error measure computed is thus a measure of the distance between
            // outlines not the distance between similarly indexed contour points

            // we walk the database fin because the unknown has been mapped and 
            // therefore the spacing of sample points is no longer uniform on the
            // unknown.  The database fin outline should be evenly spaced at approx.
            // 3.0 unit intervals

            // px,py -- coordinates of previous (currently best) point on unknown
            // qx,qy -- coordinates of new test point (may be closer than x,y)
            double 
                px = c1[begin1].x, 
                py = c1[begin1].y,
                qx,
                qy;

            double dx,dy;   // conponents of current segment on unknown fin
            double beta;    // parameter on unknown fin outline segment where line
                            // from database point intersects perpendicular to the segment

            double sqErrorPt2P, sqErrorPt2Q; // point to point squared errors

            double sum = 0.0;
            int ptsFound = 0;
            bool searching = false;

            bool prevPtInSegment = true;  // previous point has been found in THIS segment of unknown
            double betaPrev = 0.0;         // beta of previous point

            // i is index on database 
            // j is index on unknown
            int i = begin2, j = begin1+1;

            // not done if at least 3 points remain on each contour's leading edge
            bool morePts = ((i+1 <= end2) && (j+2 <= end1));

            // set default values
            error = 50000.0; //***005CM set default value

            // compute the mean squared error
            while (morePts)
            {
                // find first unknown point likely to be "closest" to database point i

                i++; // advance to next database contour point

                // slope of unknown fin outline segment between points j-1 and j 
                dx = c1[j].x - c1[j-1].x;
                dy = c1[j].y - c1[j-1].y;

                // parameter value of intersection point between line from database
                // point i and perpendicular unknown fin segment between points j-1 and j
                beta = betaPrev;
                if ((dx * dx + dy * dy) != 0.0)
                    beta = - (dx * (c1[j-1].x - c2[i].x) + dy * (c1[j-1].y - c2[i].y))
                           / (dx * dx + dy * dy) ;

                if ((0.0 <= beta) && (beta <= 1.0))
                {
                    // do not backtrack prior to last point found in segment
                    if (prevPtInSegment && (beta < betaPrev))
                        beta = betaPrev;

                    // good intersection -- likely to be "closest"
                    px = c1[j-1].x + beta * dx;
                    py = c1[j-1].y + beta * dy;
                    searching = true;

                    betaPrev = beta; // remember this point
                    prevPtInSegment = true;
                }
                else if (beta < 0.0)
                {
                    // not a good intersection within the current unknown segment
                    // intersection is in a previous segment at px,py so begin search for better
                    searching = true;
                    prevPtInSegment = false;
                }
                else // beta > 1.0
                {
                    // intersection is past current segement so set px,py as endpoint (beta = 1.0)
                    // and begin search for better point qx,qy
                    px = c1[j].x;
                    py = c1[j].y;
                    searching = true;
                    prevPtInSegment = false;
                }

                // compute squared error for current "closest" point

                sqErrorPt2P = ((px - c2[i].x) * (px - c2[i].x) + 
                              (py - c2[i].y) * (py - c2[i].y));

                // now search for a better "closest" point

                while ((i <= end2) && (j+2 <= end1) && searching)
                {
                    j++; // advance to next unknown contour segment

                    // slope of unknown fin outline segment between points j-1 and j 
                    dx = c1[j].x - c1[j-1].x;
                    dy = c1[j].y - c1[j-1].y;

                    // parameter value of intersection point between line from database
                    // point i and perpendicular unknown fin segment between points j-1 and j
                    beta = betaPrev;
                    if ((dx * dx + dy * dy) != 0.0)
                        beta = - (dx * (c1[j-1].x - c2[i].x) + dy * (c1[j-1].y - c2[i].y))
                               / (dx * dx + dy * dy) ;

                    if ((0.0 <= beta) && (beta <= 1.0))
                    {
                        // do not backtrack prior to last point found in segment
                        if (prevPtInSegment && (beta < betaPrev))
                            beta = betaPrev;

                        // good intersection
                        qx = c1[j-1].x + beta * dx;
                        qy = c1[j-1].y + beta * dy;

                        sqErrorPt2Q = ((qx - c2[i].x) * (qx - c2[i].x) + 
                                      (qy - c2[i].y) * (qy - c2[i].y));

                        if (sqErrorPt2Q >= sqErrorPt2P)
                        {
                            // moving farther away, so use px,py as "closest" and stop search
                            sum += sqErrorPt2P;
                            ptsFound++;
                            prevPtInSegment = true;
                            searching = false;
                            /////////////////////// do we need to back up to previous segment ???????
                            //j--;
                        }
                        else
                        {
                            // qx,qy is a better "closest point, so save it and keep searching
                            px = qx;
                            py = qy;
                            sqErrorPt2P = sqErrorPt2Q;
                            betaPrev = beta;
                            prevPtInSegment = true;
                            searching = true;
                        }
                    }
                    else if (beta < 0.0)
                    {
                        // use px,py as "closest" point, since we have backtracked
                            sum += sqErrorPt2P;
                            ptsFound++;
                            prevPtInSegment = false;
                            searching = false;
                            j--; // backup one segment, since we went too far searching
                    }
                    else // beta > 1.0
                    {
                        // intersection is past current segement so set px,py as endpoint (beta = 1.0)
                        // and CONTINUE search for better point qx,qy

                        // should there be a distance test here??????
                        px = c1[j].x;
                        py = c1[j].y;
                        sqErrorPt2P = ((px - c2[i].x) * (px - c2[i].x) + 
                                      (py - c2[i].y) * (py - c2[i].y));
                        prevPtInSegment = false;
                        searching = true;
                    }
                }

                //***055ER
                if (mMatchingDialog != NULL)
                {
                    // show the display of the outline registration in the dialog
                    mMatchingDialog->showErrorPt2Pt(c1,c2,c2[i].x,c2[i].y,px,py);
                }

                // see if we are done (ie, we have run out of points or segments
                morePts = ((i+1 <= end2) && (j+2 <= end1));

            }

            // if no points found, the error stays the default
            if (ptsFound > 0)
                error = (sum / (double) ptsFound);

            printf("pair matched new way\n");

            return error;
        }
        */

        //*******************************************************************
        //
        // double Match::meanSquaredErrorBetweenOutlineSegmentsNew(...)
        //
        //    REVISED: 11/14/05
        //    Computes the error between defined outline segments.
        //    Returns the error as a double.  It is assumed that the unknown
        //    has been mapped to the database fin outline prior to the call.
        //
        //    This uses a new approach.
        //    Compute arc length from start to tip and from tip to end.
        //    Use arc length ratios between database and unknown to step along
        //    database points and compute corresponding unknown points.
        //
        private double MeanSquaredErrorBetweenOutlineSegmentsMedial(
                FloatContour c1, // mapped unknown fin 
                int begin1,
                int end1,
                FloatContour c2, // envenly spaced database fin //***0005CM
                int begin2,
                int end2)
        {

            double
                error = 50000.0,
                dbArcLength, unkArcLength;

            // saved segment lengths, each is length of edge entering indexed point
            List<double> segLen1 = new List<double>();
            List<double> segLen2 = new List<double>();

            int k;

            // find length of unknown fin outline
            unkArcLength = 0.0;
            segLen1.Add(0.0); // no segment entering first point
            for (k = 1; k < c1.Length; k++)
            {
                double dx = c1[k].X - c1[k - 1].X;
                double dy = c1[k].Y - c1[k - 1].Y;
                segLen1.Add(Math.Sqrt(dx * dx + dy * dy));
                if ((begin1 < k) && (k <= end1))
                    unkArcLength += segLen1[k];
            }

            // find length of database fin outline
            dbArcLength = 0.0;
            segLen2.Add(0.0); // no segment entering first point
            for (k = 1; k < c2.Length; k++)
            {
                double dx = c2[k].X - c2[k - 1].X;
                double dy = c2[k].Y - c2[k - 1].Y;
                segLen2.Add(Math.Sqrt(dx * dx + dy * dy));
                if ((begin2 < k) && (k <= end2))
                    dbArcLength += segLen2[k];
            }

            double ratio = unkArcLength / dbArcLength;

            double sum = 0.0;
            int ptsFound = 0;

            // this process walks the database contour and at each point computes
            // the length of the step, scales it by the ratio and finds a point
            // at this scaled distance farther along on the unknown

            // we walk the database fin because the unknown has been mapped and 
            // therefore the spacing of sample points is no longer uniform on the
            // unknown.  The database fin outline should be evenly spaced at approx.
            // 3.0 unit intervals

            // i is index on database 
            // j is index on unknown
            int i = begin2 + 1, j = begin1 + 1;

            double segLenUsed = 0.0;

            while (i < end2)
            {
                // find dist to next point on database fin, and scaled distance to
                // corresponding point on unknown
                double howFar = ratio * segLen2[i] + segLenUsed;

                while (segLen1[j] < howFar)
                {
                    howFar -= segLen1[j];
                    j++;
                }
                // remember for next iteration
                segLenUsed = howFar;

                // point is on segment j of unknown, so find it

                double s = (howFar / segLen1[j]);
                double dx = c1[j].X - c1[j - 1].X;
                double dy = c1[j].Y - c1[j - 1].Y;
                double x = c1[j - 1].X + s * dx;
                double y = c1[j - 1].Y + s * dy;

                sum += ((x - c2[i].X) * (x - c2[i].X) +
                       (y - c2[i].Y) * (y - c2[i].Y));
                ptsFound++;

                //***055ER
                //TODO
                //if (mMatchingDialog != NULL)
                //{
                //	// show the display of the outline registration in the dialog
                //	mMatchingDialog->showErrorPt2Pt(c1, c2, c2[i].x, c2[i].y, x, y);
                //}

                i++;
            }

            // if no points found, the error stays the default
            if (ptsFound > 0)
                error = (sum / (double)ptsFound);

            string traceOutput = String.Format("pair matched : {0} points {1} error", ptsFound, error);
            Trace.WriteLine(traceOutput);

            return error;
        }

        //************************************ end of new stuff *******************************

        // Really new stuff -- method to use perpendiculars to medial axis (of sorts)
        // to find  better "shortest" distances between pairs of contour points

        //*******************************************************************
        //
        // double Match::meanSquaredErrorBetweenOutlineSegmentsMedial(...)
        //
        //    REVISED: 11/14/05
        //    Computes the error between defined outline segments.
        //    Returns the error as a double.  It is assumed that the unknown
        //    has been mapped to the database fin outline prior to the call.
        //
        //    This uses a new approach.
        //    Compute arc length from start to tip and from tip to end.
        //    Use arc length ratios between database and unknown to step along
        //    database points and compute corresponding unknown points.
        //
        private double MeanSquaredErrorBetweenOutlineSegments(
                FloatContour c1, // mapped unknown fin 
                int begin1,
                int mid1, //***1.85 not used here but makes prototype same as area based approach
                int end1,
                FloatContour c2, // envenly spaced database fin //***0005CM
                int begin2,
                int mid2, //***1.85 not used here but makes prototype same as area based approach
                int end2)
        {

            double error = 50000.0;
            double[] dbArcLength = new double[2];
            double[] unkArcLength = new double[2]; //***1.982a - lead and trail done separately

            // saved segment lengths, each is length of edge entering indexed point
            List<double> segLen1 = new List<double>();
            List<double> segLen2 = new List<double>();

            int k;

            // find length of unknown fin outline
            unkArcLength[0] = 0.0;
            unkArcLength[1] = 0.0; //***1.982a
            segLen1.Add(0.0); // no segment entering first point
            for (k = 1; k < c1.Length; k++)
            {
                double dx = c1[k].X - c1[k - 1].X;
                double dy = c1[k].Y - c1[k - 1].Y;
                segLen1.Add(Math.Sqrt(dx * dx + dy * dy));
                if ((begin1 < k) && (k <= mid1))    // leading edge
                    unkArcLength[0] += segLen1[k];
                else if ((mid1 < k) && (k <= end1)) //***1.982a - trailing edge
                    unkArcLength[1] += segLen1[k];
            }

            // find length of database fin outline
            dbArcLength[0] = 0.0;
            dbArcLength[1] = 0.0; //***1.982a
            segLen2.Add(0.0); // no segment entering first point
            for (k = 1; k < c2.Length; k++)
            {
                double dx = c2[k].X - c2[k - 1].X;
                double dy = c2[k].Y - c2[k - 1].Y;
                segLen2.Add(Math.Sqrt(dx * dx + dy * dy));
                if ((begin2 < k) && (k <= mid2))    // leading edge
                    dbArcLength[0] += segLen2[k];
                else if ((mid2 < k) && (k <= end2)) //***1.982a - trailing edge
                    dbArcLength[1] += segLen2[k];
            }

            double sum = 0.0;
            int i, j, ptsFound = 0;

            //***1.982a - do the determination of correspoiding point pairs and error
            // calculation in two parts (leading edge and trailing edge) so that the
            // two mid points (TIPS) are known to be the same
            for (int part = 0; part < 2; part++)
            {

                double ratio = unkArcLength[part] / dbArcLength[part];

                // this process walks the database contour and at each point computes
                // the length of the step, scales it by the ratio and finds a point
                // at this scaled distance farther along on the unknown

                // we walk the database fin because the unknown has been mapped and 
                // therefore the spacing of sample points is no longer uniform on the
                // unknown.  The database fin outline should be evenly spaced at approx.
                // 3.0 unit intervals

                FloatContour midPt = new FloatContour();

                int start1, limit1, start2, limit2; //***1.982a

                if (part == 0) // leading edge
                {
                    start1 = begin1; limit1 = mid1;
                    start2 = begin2; limit2 = mid2;
                }
                else // part == 1 trailing edge
                {
                    start1 = mid1; limit1 = end1;
                    start2 = mid2; limit2 = end2;
                }

                // i is index on database 
                // j is index on unknown
                //int i = begin2+1, j = begin1+1;

                i = start2 + 1; //***1.982a
                j = start1 + 1; //***1.982a

                double segLenUsed = 0.0;

                //while (i < end2)
                while (i < limit2) //***1.982a
                {
                    // find dist to next point on database fin, and scaled distance to
                    // corresponding point on unknown
                    double howFar = ratio * segLen2[i] + segLenUsed;

                    while (segLen1[j] < howFar)
                    {
                        howFar -= segLen1[j];
                        j++;
                    }
                    // remember for next iteration
                    segLenUsed = howFar;

                    // point is on segment j of unknown, so find it

                    double s = (howFar / segLen1[j]);
                    double dx = c1[j].X - c1[j - 1].X;
                    double dy = c1[j].Y - c1[j - 1].Y;
                    double x = c1[j - 1].X + s * dx;
                    double y = c1[j - 1].Y + s * dy;

                    // save midpoint (part of medial axis) for use later

                    midPt.AddPoint((float)(0.5 * (c2[i].X + x)), (float)(0.5 * (c2[i].Y + y)));

                    i++;
                }

                // now traverse medial axis and find length of perpendicular through
                // medial axis point with endpoints on the two fin outlines

                //i = begin2 + 1; // index on database fin
                //j = begin1 + 1; // index on unknown fin

                //***1.982a
                i = start2 + 1; // index on database fin
                j = start1 + 1; // index on unknown fin

                /////////////////////////// changes //////////////////////////
                //***1.75 - keep track of j & i values from previous point pair calculation
                int iPrev = i;
                int jPrev = j;
                ////////////////////////// end changes ////////////////////////////

                bool done = false;
                int backI = 0, backJ = 0; //***1.75 - how much we are backing up

                for (k = 1; (k + 1 < midPt.Length) && (!done); k++)
                {
                    double
                        unkX = 0, unkY = 0,
                        dbX = 0, dbY = 0;

                    double
                        mdx = midPt[k + 1].X - midPt[k - 1].X,
                        mdy = midPt[k + 1].Y - midPt[k - 1].Y;
                    bool
                        foundUnk = false,
                        foundDB = false;

                    backI = 0; //***1.75
                    backJ = 0; //***1.75

                    while ((!foundUnk) && (!done))
                    {
                        double
                            dot1 = (c1[j - 1].X - midPt[k].X) * mdx
                                 + (c1[j - 1].Y - midPt[k].Y) * mdy,
                            dot2 = (c1[j].X - midPt[k].X) * mdx
                                 + (c1[j].Y - midPt[k].Y) * mdy;
                        if (((dot1 <= 0.0) && (0.0 <= dot2)) || ((dot2 <= 0.0) && (0.0 <= dot1)))
                        {
                            // this segment contains a point of intersection with the perpendicular from
                            // the medial axis at point k
                            // slope of unknown fin outline segment between points j-1 and j 
                            double
                                dx1 = c1[j].X - c1[j - 1].X,
                                dy1 = c1[j].Y - c1[j - 1].Y;

                            double beta = 0.0;

                            if ((mdx * dx1 + mdy * dy1) != 0.0)
                                beta = -(mdx * (c1[j - 1].X - midPt[k].X) + mdy * (c1[j - 1].Y - midPt[k].Y))
                                       / (mdx * dx1 + mdy * dy1);

                            if ((0.0 <= beta) && (beta <= 1.0))
                            {
                                // found the point on this unknown segment
                                unkX = beta * dx1 + c1[j - 1].X;
                                unkY = beta * dy1 + c1[j - 1].Y;
                                foundUnk = true;
                            }
                            else
                                Trace.WriteLine("Error in medial axis code UNK");
                        }
                        else if ((dot1 < 0.0) && (dot2 < 0.0))
                        {
                            // move forward along unknown
                            j++;
                            //if (j > end1)
                            if (j > limit1) //***1.982a
                                done = true;
                        }
                        else if ((dot1 > 0.0) && (dot2 > 0.0))
                        {
                            // back up on unknown
                            if (backJ < 50) //***1.75 - new constraint
                            {
                                j--;
                                backJ++;
                            }
                            //if ((j < 1) || (backJ >= 50)) // new constraint
                            if ((j < start1 + 1) || (backJ >= 50)) //***1.982a - new constraint
                                done = true;
                        }
                        else
                            Trace.WriteLine("Error in medial axis code UNK2");
                    }

                    while ((!foundDB) && (!done))
                    {
                        double
                            dot1 = (c2[i - 1].X - midPt[k].X) * mdx
                                 + (c2[i - 1].Y - midPt[k].Y) * mdy,
                            dot2 = (c2[i].X - midPt[k].X) * mdx
                                 + (c2[i].Y - midPt[k].Y) * mdy;
                        if (((dot1 <= 0.0) && (0.0 <= dot2)) || ((dot2 <= 0.0) && (0.0 <= dot1)))
                        {
                            // this segment contains a point of intersection with the perpendicular from
                            // the medial axis at point k
                            // slope of database fin outline segment between points i-1 and i 
                            double
                                dx2 = c2[i].X - c2[i - 1].X,
                                dy2 = c2[i].Y - c2[i - 1].Y;

                            double beta = 0.0;

                            if ((mdx * dx2 + mdy * dy2) != 0.0)
                                beta = -(mdx * (c2[i - 1].X - midPt[k].X) + mdy * (c2[i - 1].Y - midPt[k].Y))
                                       / (mdx * dx2 + mdy * dy2);

                            if ((0.0 <= beta) && (beta <= 1.0))
                            {
                                // found the point on this database segment
                                dbX = beta * dx2 + c2[i - 1].X;
                                dbY = beta * dy2 + c2[i - 1].Y;
                                foundDB = true;
                            }
                            else
                                Trace.WriteLine("Error in medial axis code DB");
                        }
                        else if ((dot1 < 0.0) && (dot2 < 0.0))
                        {
                            // move forward along database
                            i++;
                            //if (i > end2)
                            if (i > limit2) //***1.982a
                                done = true;
                        }
                        else if ((dot1 > 0.0) && (dot2 > 0.0))
                        {
                            // back up on database
                            if (backI < 50) //***1.75 - new constraint
                            {
                                i--;
                                backI++;
                            }
                            //if ((i < 1) || (backI >= 50)) // new constraint
                            if ((i < start2 + 1) || (backI >= 50)) //***1.982a - new constraint
                                done = true;
                        }
                        else
                            Trace.WriteLine("Error in medial axis code DB2");
                    }

                    /////////////////////////// changes ////////////////////
                    //***1.75 
                    // these two lines are the old code
                    //sum += ((dbX - unkX) * (dbX - unkX) + (dbY - unkY) * (dbY - unkY));   
                    //ptsFound++;
                    if (foundDB && foundUnk) // new constraint test
                    {
                        sum += ((dbX - unkX) * (dbX - unkX) + (dbY - unkY) * (dbY - unkY));

                        ptsFound++;

                        //***1.75 - keep track of where we start next search
                        iPrev = i;
                        jPrev = j;
                    }
                    else
                    {
                        // could not find corresponding point on one of the outlines
                        // so skip this medial axis point and move on to next one
                        // but reset the unknown and database contour indices back to 
                        // what they were BEFORE the failure
                        i = iPrev;
                        j = jPrev;
                        //g_print("_");
                        done = false; // force search to continue
                    }

                    ////////////////////////// end changes /////////////////

                    //***055ER
                    //if (mMatchingDialog != NULL)
                    //TODO
                    //if ((mMatchingDialog != NULL) && (k % 8 == 0))
                    //{
                    //	// show the display of the outline registration in the dialog
                    //	mMatchingDialog->showErrorPt2Pt(c1, c2, dbX, dbY, unkX, unkY);
                    //}
                }

            } //***1.982a - end of for (part) loop

            // if no points found, the error stays the default
            if (ptsFound > 0)
                error = (sum / (double)ptsFound);

            string traceOutput = string.Format("pair matched (medial): {0} points {1} error", ptsFound, error);
            Trace.WriteLine(traceOutput);

            return error;
        }


        //*********************************
        private double TriangleArea(PointF p, PointF q, PointF r)
        {
            return (0.5 * (p.X * q.Y + q.X * r.Y + r.X * p.Y
                           - p.Y * q.X - q.Y * r.X - r.Y * p.X));
        }

        //******************************************************************
        //
        //***1.75 - new area between curves approach 
        //
        //
        private double AreaBasedErrorBetweenOutlineSegments_OLD(
                FloatContour c1, // mapped unknown fin 
                int begin1,
                int end1,
                FloatContour c2, // envenly spaced database fin //***0005CM
                int begin2,
                int end2)
        {
            double
                //error = 50000.0,
                dbArcLength, unkArcLength;

            // saved segment lengths, each is length of edge entering indexed point
            List<double> segLen1 = new List<double>(); ;
            List<double> segLen2 = new List<double>();

            int k;

            // find length of unknown fin outline
            unkArcLength = 0.0;
            segLen1.Add(0.0); // no segment entering first point
            for (k = 1; k < c1.Length; k++)
            {
                double dx = c1[k].X - c1[k - 1].X;
                double dy = c1[k].Y - c1[k - 1].Y;
                segLen1.Add(Math.Sqrt(dx * dx + dy * dy));
                if ((begin1 < k) && (k <= end1))
                    unkArcLength += segLen1[k];
            }

            // find length of database fin outline
            dbArcLength = 0.0;
            segLen2.Add(0.0); // no segment entering first point
            for (k = 1; k < c2.Length; k++)
            {
                double dx = c2[k].X - c2[k - 1].X;
                double dy = c2[k].Y - c2[k - 1].Y;
                segLen2.Add(Math.Sqrt(dx * dx + dy * dy));
                if ((begin2 < k) && (k <= end2))
                    dbArcLength += segLen2[k];
            }

            // i is index into database fin *c2
            // j is index into unknown fin *c1
            // pivotDB indicates which contour is the point of the next triangle 
            //     (true == DB, false == UNK)
            // initial area zero
            // inital i value is begin2
            // initial j value is begin1
            // initial prevDot is unkRay(i,i+1) X dbRay(j,j+1)
            // initial pivotDB value is true
            // i++
            // while not out of points (i < end2 and j < end1)
            //    if pivotDB
            //       dot = dbRay(i,i+1) X ray(db_i, unk_j+1)
            //       if sign_diff(dot,prevDot)
            //          area += fabs(triangleArea(i,j,j+1))
            //          pivotDB = false
            //          j++
            //          prevDot = dot
            //       else
            //          dot = unkRay(j+1,j) X ray(unk_j,db_i+1)
            //          if sign_diff(dot,prevDot)
            //             area += fabs(triangleArea(i,j,i+1))
            //             i++
            //          else
            //             pt = intersection(i,i+1,j,j+1)
            //             area += fabs(triangleArea(i,j,pt)) + fabs(triangleArea(i+1,j+1,pt))
            //             i++
            //             j++
            //    else --- pivot is unknown
            //       dot = unkRay(j,j+1) X ray(unk_j, db_i+1)
            //       if sign_diff(dot,prevDot)
            //          area += fabs(triangleArea(j,i,i+1))
            //          pivotDB = true
            //          i++
            //          prevDot = dot
            //       else
            //          dot = dbRay(i+1,i) X ray(db_i+1,unk_j+1)
            //          if sign_diff(dot,prevDot)
            //             area += fabs(triangleArea(j,i,j+1))
            //             j++
            //          else
            //             pt = intersection(j,j+1,i,i+1)
            //             area += fabs(triangleArea(j,i,pt)) + fabs(triangleArea(j+1,i+1,pt))
            //             j++
            //             i++
            int i = begin2;
            int j = begin1;
            double area = 0.0;
            bool pivotDB = true;
            bool prevWasXPt = false; // indicates wheter to use previous intersection point
            PointF prevXPt = new PointF();      // and this was the previous intesection point
            double
                dx1 = c2[i + 1].X - c2[i].X, // dbRay
                dy1 = c2[i + 1].Y - c2[i].Y,
                dx2 = c1[j + 1].X - c1[j].X, // unkRay
                dy2 = c1[j + 1].Y - c1[j].Y;
            double
                dot,
                prevDot = dx2 * dy1 - dx1 * dy2; // unkRay X dbRay

            i++;
            while ((i < end2) && (j < end1))
            {
                if (pivotDB)
                {
                    dx1 = c2[i + 1].X - c2[i].X; // dbRay
                    dy1 = c2[i + 1].Y - c2[i].Y;
                    dx2 = c1[j + 1].X - c2[i].X; // joiningRay
                    dy2 = c1[j + 1].Y - c2[i].Y;
                    dot = (dx1 * dy2 - dx2 * dy1);   // dbRay X joiningRay
                    if ((dot <= 0.0) && (prevDot >= 0.0) || (dot >= 0.0) && (prevDot <= 0.0))
                    {
                        // all is well, use next point on opposite contour as triangle base
                        double A;
                        if (prevWasXPt)
                            A = Math.Abs(TriangleArea(c2[i], prevXPt, c1[j + 1])); // pivot, last, next
                        else
                            A = Math.Abs(TriangleArea(c2[i], c1[j], c1[j + 1])); // pivot, last, next
                        area += A;
                        pivotDB = false;
                        prevWasXPt = false;
                        j++;
                        prevDot = dot;

                        Trace.Write("[" + A + "]-");
                    }
                    else
                    {
                        dx1 = c1[j + 1].X - c1[j].X; // unkRay
                        dy1 = c1[j + 1].Y - c1[j].Y;
                        dx2 = c2[i + 1].X - c1[j].X; // ray(db_j to unk_i+1)
                        dy2 = c2[i + 1].Y - c1[j].Y;
                        dot = (dx1 * dy2 - dx2 * dy1);   // unkRay X ray
                        if ((dot < 0.0) && (prevDot <= 0.0) || (dot > 0.0) && (prevDot >= 0.0))
                        {
                            // lines cross but segments do not intersect, so use next segment on
                            // THIS contour as base
                            double A;
                            if (prevWasXPt)
                                A = Math.Abs(TriangleArea(c2[i], prevXPt, c2[i + 1])); // pivot, last, next
                            else
                                A = Math.Abs(TriangleArea(c2[i], c1[j], c2[i + 1])); // pivot, last, next
                            area += A;
                            prevWasXPt = false;
                            i++;
                            Trace.Write("[" + A + "]v");
                        }
                        else // two segments intersect
                        {
                            // find pt of intersection
                            PointF pt = new PointF();
                            // deltas for unknown segment
                            dx1 = c1[j + 1].X - c1[j].X;
                            dy1 = c1[j + 1].Y - c1[j].Y;
                            // deltas for database segment
                            dx2 = c2[i + 1].X - c2[i].X;
                            dy2 = c2[i + 1].Y - c2[i].Y;
                            // beta is parameter for database segment
                            double beta =
                                ((c2[i].Y - c1[j].Y) * dx1 - (c2[i].X - c1[j].X) * dy1) /
                                (dx2 * dy1 - dy2 * dx1);
                            // find point along database segment
                            pt.X = (float)(beta * dx2 + c2[i].X);
                            pt.Y = (float)(beta * dy2 + c2[i].Y);
                            // find area of two triangles sharing intersection point apex
                            double A;
                            if (prevWasXPt)
                                A = Math.Abs(TriangleArea(c2[i], prevXPt, pt)); // pivot, last, next
                            else
                                A = Math.Abs(TriangleArea(c2[i], c1[j], pt));
                            //double A2 = fabs(triangleArea(c2[i+1],c1[j+1],pt));
                            area += A;
                            prevXPt = pt;
                            prevWasXPt = true;
                            i++;
                            //j++;
                            //pivotDB = false;
                            if (dot != 0.0)
                                prevDot = -prevDot;

                            Trace.Write("[" + A + "]x");
                        }
                    }
                }
                else // pivot is on Unknown contour
                {
                    dx1 = c1[j + 1].X - c1[j].X; // unkRay
                    dy1 = c1[j + 1].Y - c1[j].Y;
                    dx2 = c2[i + 1].X - c1[j].X; // joining Ray
                    dy2 = c2[i + 1].Y - c1[j].Y;
                    dot = (dx1 * dy2 - dx2 * dy1);   // unkRay X joiningRay
                    if ((dot <= 0.0) && (prevDot >= 0.0) || (dot >= 0.0) && (prevDot <= 0.0))
                    {
                        // all is well, use next point on opposite contour as triangle base
                        double A;
                        if (prevWasXPt)
                            A = Math.Abs(TriangleArea(c1[j], prevXPt, c2[i + 1])); // pivot, last, next
                        else
                            A = Math.Abs(TriangleArea(c1[j], c2[i], c2[i + 1])); // pivot, last, next
                        area += A;
                        pivotDB = true;
                        prevWasXPt = false;
                        i++;
                        prevDot = dot;

                        Trace.Write("[" + A + "]+");
                    }
                    else
                    {
                        dx1 = c2[i + 1].X - c2[i].X; // unkRay
                        dy1 = c2[i + 1].Y - c2[i].Y;
                        dx2 = c1[j + 1].X - c2[i].X; // testRay (unk_i to db_j+1)
                        dy2 = c1[j + 1].Y - c2[i].Y;
                        dot = (dx1 * dy2 - dx2 * dy1);   // unkRay X testRay
                        if ((dot < 0.0) && (prevDot <= 0.0) || (dot > 0.0) && (prevDot >= 0.0))
                        {
                            // lines cross but segments do not intersect, so use next segment on
                            // THIS contour as base
                            double A;
                            if (prevWasXPt)
                                A = Math.Abs(TriangleArea(c1[j], prevXPt, c1[j + 1])); // pivot, last, next
                            else
                                A = Math.Abs(TriangleArea(c1[j], c2[i], c1[j + 1])); // pivot, last, next
                            area += A;
                            prevWasXPt = false;
                            j++;
                            Trace.Write("[" + A + "]^");
                        }
                        else // two segments intersect
                        {
                            // find pt of intersection
                            PointF pt = new PointF();
                            // deltas for unknown segment
                            dx1 = c1[j + 1].X - c1[j].X;
                            dy1 = c1[j + 1].Y - c1[j].Y;
                            // deltas for database segment
                            dx2 = c2[i + 1].X - c2[i].X;
                            dy2 = c2[i + 1].Y - c2[i].Y;
                            // beta is parameter for database segment
                            double beta =
                                ((c2[i].Y - c1[j].Y) * dx1 - (c2[i].X - c1[j].X) * dy1) /
                                (dx2 * dy1 - dy2 * dx1);
                            // find point along database segment
                            pt.X = (float)(beta * dx2 + c2[i].X);
                            pt.Y = (float)(beta * dy2 + c2[i].Y);
                            // find area of two triangles sharing intersection point apex
                            double A;
                            if (prevWasXPt)
                                A = Math.Abs(TriangleArea(c1[j], prevXPt, pt)); // pivot, last, next
                            else
                                A = Math.Abs(TriangleArea(c1[j], c2[i], pt)); // pivot, last, next
                                                                              //double A2 = fabs(triangleArea(c2[i+1],c1[j+1],pt));
                            area += A;
                            prevXPt = pt;
                            prevWasXPt = true;
                            //i++;
                            j++;
                            //pivotDB = true;
                            if (dot != 0.0)
                                prevDot = -prevDot;
                            Trace.Write("[" + A + "]x");
                        }
                    }
                }
            }
            // total area at end should be normalized : divided by arclength of one or both contours

            return (2.0 * area / (unkArcLength + dbArcLength));

            //return area;
        }

        private double AreaBasedErrorBetweenOutlineSegments(
                FloatContour c1, // mapped unknown fin 
                int begin1,
                int mid1,
                int end1,
                FloatContour c2, // envenly spaced database fin //***0005CM
                int begin2,
                int mid2,
                int end2)
        {
            double retVal = AreaMatch.AreaBasedErrorBetweenOutlineSegments_NEW(
                c1, // mapped unknown fin 
                begin1,
                mid1,
                end1,
                c2, // envenly spaced database fin //***0005CM
                begin2,
                mid2,
                end2);
            //cout << "AreaError: " << retVal << endl;

            return retVal;
        }
    }
}
