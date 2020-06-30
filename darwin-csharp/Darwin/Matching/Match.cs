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
using Darwin.Features;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace Darwin.Matching
{
    public class Match
    {
        public DatabaseFin UnknownFin { get; set; }
        DarwinDatabase Database { get; set; }
        protected int CurrentFinIndex { get; set; }

        private UpdateDisplayOutlinesDelegate _updateOutlines;
        public MatchResults MatchResults { get; set; }

        public List<MatchFactor> MatchFactors { get; set; }

        public Match(DatabaseFin unknownFin,
            DarwinDatabase db,
            UpdateDisplayOutlinesDelegate updateOutlines)
        {
            if (unknownFin == null)
                throw new ArgumentNullException(nameof(unknownFin));

            if (db == null)
                throw new ArgumentNullException(nameof(db));

            UnknownFin = new DatabaseFin(unknownFin);
            Database = db;
            _updateOutlines = updateOutlines;

            CurrentFinIndex = 0;

            MatchResults = new MatchResults(unknownFin.IDCode, unknownFin?.FinFilename, db?.Filename);
        }

        //*******************************************************************
        //
        // Match::Match(...)
        //
        //    CONSTRUCTOR
        //
        public Match(DatabaseFin unknownFin,
            DarwinDatabase db,
            UpdateDisplayOutlinesDelegate updateOutlines,
            RegistrationMethodType registrationMethod,
            bool useFullFinError)
            : this(unknownFin, db, updateOutlines)
        { 
            SetMatchOptions(registrationMethod, useFullFinError);
        }

        public Match(DatabaseFin unknownFin,
            DarwinDatabase db,
            UpdateDisplayOutlinesDelegate updateOutlines,
            List<MatchFactor> matchFactors)
            : this(unknownFin, db, updateOutlines)
        {
            MatchFactors = matchFactors;
        }

        public void SetMatchOptions(
            RegistrationMethodType registrationMethod,
            bool useFullFinError)
        {
            MatchFactors = new List<MatchFactor>();

            // TODO: Move this elsewhere, change for Fins/Bears
            var controlPoints = new List<FeaturePointType> { FeaturePointType.LeadingEdgeBegin, FeaturePointType.Tip, FeaturePointType.PointOfInflection };

            switch (registrationMethod)
            {
                case RegistrationMethodType.Original3Point:
                    // use beginning of leading edge, tip and largest trailing notch
                    // to map unknown outline to database outline, and then use
                    // version of meanSqError... that trims leading and trailing
                    // edge points to equalize number of points on each contour,
                    // and finally compute error between "corresponding" pairs of
                    // mapped points
                    MatchFactors.Add(MatchFactor.CreateOutlineFactor(
                        1.0f,
                        controlPoints,
                        OutlineErrorFunctions.MeanSquaredErrorBetweenOutlineSegments,
                        OutlineErrorFunctions.FindErrorBetweenFins_Original3Point,
                        _updateOutlines));
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
                    MatchFactors.Add(MatchFactor.CreateOutlineFactor(
                        1.0f,
                        controlPoints,
                        OutlineErrorFunctions.MeanSquaredErrorBetweenOutlineSegments,
                        OutlineErrorFunctions.FindErrorBetweenFins,
                        _updateOutlines));
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
                    MatchFactors.Add(MatchFactor.CreateOutlineFactor(
                        1.0f,
                        controlPoints,
                        OutlineErrorFunctions.MeanSquaredErrorBetweenOutlineSegments,
                        OutlineErrorFunctions.FindErrorBetweenFinsOptimal,
                        _updateOutlines,
                        new FinFlagsMatchOptions
                        {
                            MoveTip = false,
                            MoveEndsInAndOut = false,
                            UseFullFinError = useFullFinError
                        }));

                    break;
                case RegistrationMethodType.TrimOptimalTip:
                    MatchFactors.Add(MatchFactor.CreateOutlineFactor(
                        1.0f,
                        controlPoints,
                        OutlineErrorFunctions.MeanSquaredErrorBetweenOutlineSegments,
                        OutlineErrorFunctions.FindErrorBetweenFinsOptimal,
                        _updateOutlines,
                        new FinFlagsMatchOptions
                        {
                            MoveTip = true,
                            MoveEndsInAndOut = false,
                            UseFullFinError = useFullFinError
                        }));
                    break;
                case RegistrationMethodType.TrimOptimalArea: //***1.85 - new area based metric option
                    MatchFactors.Add(MatchFactor.CreateOutlineFactor(
                        1.0f,
                        controlPoints,
                        OutlineErrorFunctions.AreaBasedErrorBetweenOutlineSegments,
                        OutlineErrorFunctions.FindErrorBetweenFinsOptimal,
                        _updateOutlines,
                        new FinFlagsMatchOptions
                        {
                            MoveTip = true,
                            MoveEndsInAndOut = false,
                            UseFullFinError = useFullFinError
                        }));
                    break;
                case RegistrationMethodType.TrimOptimalInOut:
                case RegistrationMethodType.TrimOptimalInOutTip:
                default:
                    throw new NotImplementedException();
            }
        }

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
        public float MatchSingleFin(List<Category> categoriesToMatch)
        {
            if (MatchFactors == null || MatchFactors.Count < 1)
                throw new Exception("Match factors haven't been set yet!");

            // If we have any factors missing updateOutlines, but we know what
            // the delegate should be, fill them in
            if (_updateOutlines != null && MatchFactors.Any(mf => mf.MatchFactorType == MatchFactorType.Outline && mf.UpdateOutlines == null))
            {
                foreach (var matchFactor in MatchFactors.Where(mf => mf.MatchFactorType == MatchFactorType.Outline && mf.UpdateOutlines == null))
                {
                    matchFactor.UpdateOutlines = _updateOutlines;
                }
            }

            DatabaseFin thisDBFin;

            do
            {
                if (CurrentFinIndex >= Database.AllFins.Count)
                    return 100.0f;

                thisDBFin = Database.AllFins[CurrentFinIndex];

                if (null == thisDBFin)
                    CurrentFinIndex++;
            }
            while (null == thisDBFin);

            bool tryMatch = categoriesToMatch.Exists(c => c.Name.ToLower() == thisDBFin.DamageCategory.ToLower());

            if (tryMatch)
            {
                // TODO: This isn't done quite right yet
                MatchError result = new MatchError();
                double errorBetweenFins = 0.0;

                foreach (var factor in MatchFactors)
                {
                    result = factor.FindErrorBetweenIndividuals(UnknownFin, thisDBFin);

                    errorBetweenFins += factor.Weight * result.Error;
                }

                // Now, store the result
                Result r = new Result(
                    result.Contour1, //***005CM
                    result.Contour2, //***005CM
                    thisDBFin.DatabaseID,
                    thisDBFin.ImageFilename,  //***001DB
                    thisDBFin.ThumbnailFilenameUri,
                    CurrentFinIndex,
                    errorBetweenFins,
                    thisDBFin.IDCode,
                    thisDBFin.Name,
                    thisDBFin.DamageCategory,
                    thisDBFin.DateOfSighting,
                    thisDBFin.LocationCode);

                //***1.1 - set indices of beginning, tip and end points used in mapping
                r.SetMappingControlPoints(
                    result.Contour1ControlPoint1, result.Contour1ControlPoint2, result.Contour1ControlPoint3,  // beginning, tip & end of unknown fin
                    result.Contour2ControlPoint1, result.Contour2ControlPoint2, result.Contour2ControlPoint3); // beginning, tip & end of database fin

                MatchResults.AddResult(r);
            }

            CurrentFinIndex++;

            if (CurrentFinIndex == Database.AllFins.Count)
                return 1.0f;

            return (float)CurrentFinIndex / Database.AllFins.Count;
        }
    }
}
