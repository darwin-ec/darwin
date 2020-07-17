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
using MathNet.Numerics.LinearAlgebra;
using MathNet.Numerics.LinearAlgebra.Complex;
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

        private List<MatchFactorError> _rawErrorTracking;
        private List<MatchFactorError> RawErrorTracking
        {
            get
            {
                if (_rawErrorTracking == null)
                    _rawErrorTracking = new List<MatchFactorError>();

                return _rawErrorTracking;
            }
            set
            {
                _rawErrorTracking = value;
            }
        }

        public Match(
            DatabaseFin unknownFin,
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

        public Match(
            DatabaseFin unknownFin,
            DarwinDatabase db,
            UpdateDisplayOutlinesDelegate updateOutlines,
            RegistrationMethodType registrationMethod,
            bool useFullFinError)
            : this(unknownFin, db, updateOutlines)
        { 
            SetMatchOptions(registrationMethod, useFullFinError);
        }

        public Match(
            DatabaseFin unknownFin,
            DarwinDatabase db,
            UpdateDisplayOutlinesDelegate updateOutlines,
            List<MatchFactor> matchFactors,
            bool createDefaultFactors = false)
            : this(unknownFin, db, updateOutlines)
        {
            if (!createDefaultFactors)
            {
                MatchFactors = matchFactors;
            }
            else if (Database.CatalogScheme != null)
            {
                switch (db.CatalogScheme.FeatureSetType)
                {
                    case FeatureSetType.DorsalFin:
                        SetMatchOptions(RegistrationMethodType.TrimOptimalTip, true);
                        break;

                    case FeatureSetType.Bear:
                        MatchFactors = MatchFactorPresets.CreateBearMatchFactors(db);
                        break;
                }
            }

            CheckUnknownForRequiredFeatures();

            // Force rediscovery (for debugging/testing, can uncomment
            //UnknownFin.FinOutline.RediscoverFeaturePoints(Database.CatalogScheme.FeatureSetType);
        }

        public void SetMatchOptions(
            RegistrationMethodType registrationMethod,
            bool useFullFinError)
        {
            MatchFactors = new List<MatchFactor>();

            // TODO: Move this elsewhere, change for Fins/Bears?
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
                    MatchFactors.Add(MatchFactor.CreateOutlineFactor(
                        1.0f,
                        controlPoints,
                        OutlineErrorFunctions.MeanSquaredErrorBetweenOutlineSegments,
                        OutlineErrorFunctions.FindErrorBetweenFinsOptimal,
                        _updateOutlines,
                        new OutlineMatchOptions
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
                        new OutlineMatchOptions
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
                        new OutlineMatchOptions
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

            CheckUnknownForRequiredFeatures();
        }

        /// <summary>
        /// This verifies whether the match settings can be run against the current database.
        /// E.g., whether all the features are present on all database individuals.
        /// </summary>
        /// <returns>true if it can be run, false if not</returns>
        public bool VerifyMatchSettings()
        {
            if (MatchFactors == null || MatchFactors.Count < 1)
                return false;

            var featurePointTypes = new List<FeaturePointType>();
            var featureTypes = new List<FeatureType>();

            foreach (var factor in MatchFactors)
            {
                if (factor.DependentFeaturePoints != null)
                    featurePointTypes.AddRange(factor.DependentFeaturePoints);

                if (factor.DependentFeatures != null)
                    featureTypes.AddRange(factor.DependentFeatures);
            }

            var distinctFeaturePointList = featurePointTypes.Distinct().ToList();
            var distinctFeatureList = featureTypes.Distinct().ToList();

            // If the current unknown is missing any feature points, rediscover them
            // algorithmically
            if (UnknownFin.FinOutline != null && (!UnknownFin.FinOutline.ContainsAllFeaturePointTypes(distinctFeaturePointList) || !UnknownFin.FinOutline.ContainsAllFeatureTypes(distinctFeatureList)))
                UnknownFin.FinOutline.RediscoverFeaturePoints(Database.CatalogScheme.FeatureSetType);

            return Database.ContainsAllFeaturePointTypes(distinctFeaturePointList) && Database.ContainsAllFeatureTypes(distinctFeatureList);
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
        public float MatchSingleIndividual(List<Category> categoriesToMatch)
        {
            if (MatchFactors == null || MatchFactors.Count < 1)
                throw new Exception("Match factors haven't been set yet!");

            var stopWatch = System.Diagnostics.Stopwatch.StartNew();

            // If we have any factors missing updateOutlines, but we know what
            // the delegate should be, fill them in
            if (_updateOutlines != null && MatchFactors.Any(mf => mf.MatchFactorType == MatchFactorType.Outline && mf.UpdateOutlines == null))
            {
                foreach (var matchFactor in MatchFactors.Where(mf => mf.MatchFactorType == MatchFactorType.Outline && mf.UpdateOutlines == null))
                {
                    matchFactor.UpdateOutlines = _updateOutlines;
                }
            }

            if (CurrentFinIndex >= Database.AllFins.Count)
                return 100.0f;

            DatabaseFin thisDBFin = Database.AllFins[CurrentFinIndex];

            bool tryMatch = categoriesToMatch.Exists(c => c.Name.ToLower() == thisDBFin.DamageCategory.ToLower());

            if (tryMatch)
            {
                // TODO: This isn't done quite right yet
                MatchError matchErrorResult = new MatchError();
                double errorBetweenFins = 0.0;

                List<MatchFactorError> rawError = new List<MatchFactorError>();

                int factorIndex = 0;
                try
                {
                    foreach (var factor in MatchFactors)
                    {
                        var factorResult = factor.FindErrorBetweenIndividuals(UnknownFin, thisDBFin);

                        if (factor.MatchFactorType == MatchFactorType.Outline)
                        {
                            Vector<double> saveRawRatios = null;
                            if (matchErrorResult.RawRatios != null)
                                saveRawRatios = matchErrorResult.RawRatios;

                            Vector<double> saveRHat = null;
                            if (matchErrorResult.RHat != null)
                                saveRHat = matchErrorResult.RHat;

                            Vector<double> saveDBRHat = null;
                            if (matchErrorResult.DBRHat != null)
                                saveDBRHat = matchErrorResult.DBRHat;

                            Vector<double> saveDBRawRatios = null;
                            if (matchErrorResult.DBRawRatios != null)
                                saveDBRawRatios = matchErrorResult.DBRawRatios;

                            matchErrorResult = factorResult;

                            if (factorResult.Contour1 != null)
                                UnknownFin.FinOutline.RemappedChainPoints = factorResult.Contour1;

                            if (saveRawRatios != null)
                                matchErrorResult.RawRatios = saveRawRatios;
                            if (saveRHat != null)
                                matchErrorResult.RHat = saveRHat;
                            if (saveDBRawRatios != null)
                                matchErrorResult.DBRawRatios = saveDBRawRatios;
                            if (saveDBRHat != null)
                                matchErrorResult.DBRHat = saveDBRHat;
                        }
                        else
                        {
                            matchErrorResult.RawRatios = factorResult.RawRatios;
                            matchErrorResult.RHat = factorResult.RHat;
                            matchErrorResult.DBRawRatios = factorResult.DBRawRatios;
                            matchErrorResult.DBRHat = factorResult.DBRHat;
                        }

                        // We're going to rescale this later -- should probably remove this
                        // errorBetweenFins += factor.Weight * result.Error;
                        var matchFactorError = new MatchFactorError
                        {
                            FactorIndex = factorIndex,
                            Error = factorResult.Error,
                            Weight = factor.Weight
                        };
                        rawError.Add(matchFactorError);
                        RawErrorTracking.Add(matchFactorError);

                        factorIndex += 1;
                    }
                }
                catch (Exception ex)
                {
                    Trace.WriteLine(ex);
                }

                // Now, store the result
                Result r = new Result(
                    matchErrorResult.Contour1, //***005CM
                    matchErrorResult.Contour2, //***005CM
                    thisDBFin.ID,
                    thisDBFin.ImageFilename,  //***001DB
                    thisDBFin.ThumbnailFilenameUri,
                    CurrentFinIndex,
                    rawError,
                    errorBetweenFins,
                    thisDBFin.IDCode,
                    thisDBFin.Name,
                    thisDBFin.DamageCategory,
                    thisDBFin.DateOfSighting,
                    thisDBFin.LocationCode);

                if (matchErrorResult.RHat != null)
                    r.RHat = matchErrorResult.RHat;

                if (matchErrorResult.RawRatios != null)
                    r.RawRatios = matchErrorResult.RawRatios;

                if (matchErrorResult.DBRHat != null)
                    r.DBRHat = matchErrorResult.DBRHat;

                if (matchErrorResult.DBRawRatios != null)
                    r.DBRawRatios = matchErrorResult.DBRawRatios;

                //***1.1 - set indices of beginning, tip and end points used in mapping
                r.SetMappingControlPoints(
                    matchErrorResult.Contour1ControlPoint1, matchErrorResult.Contour1ControlPoint2, matchErrorResult.Contour1ControlPoint3,  // beginning, tip & end of unknown fin
                    matchErrorResult.Contour2ControlPoint1, matchErrorResult.Contour2ControlPoint2, matchErrorResult.Contour2ControlPoint3); // beginning, tip & end of database fin
              
                MatchResults.AddResult(r);

                stopWatch.Stop();
                MatchResults.TimeTaken += stopWatch.ElapsedMilliseconds;
            }

            CurrentFinIndex += 1;

            if (CurrentFinIndex >= Database.AllFins.Count)
            {
                if (RawErrorTracking != null && RawErrorTracking.Count > 0)
                {
                    // Now that we're through matching, let's rescale the errors
                    Dictionary<int, float> scaleFactors = new Dictionary<int, float>();
                    
                    foreach (var idx in RawErrorTracking.Select(r => r.FactorIndex).Distinct())
                    {
                        double minError = RawErrorTracking.Where(r => r.FactorIndex == idx).Min(r => r.Error);
                        double maxError = RawErrorTracking.Where(r => r.FactorIndex == idx).Max(r => r.Error);

                        // Force minError to 0 if it's not <= already
                        if (minError > 0)
                            minError = 0;

                        scaleFactors[idx] = (float)(1 / (maxError - minError));
                    }

                    foreach (var result in MatchResults.Results)
                    {
                        if (result.RawError != null && result.RawError.Count > 0)
                        {
                            double scaledError = 0.0;

                            foreach (var raw in result.RawError)
                            {
                                scaledError += scaleFactors[raw.FactorIndex] * raw.Error * raw.Weight;
                            }

                            result.Error = scaledError;
                            result.Confidence = 1.0f - scaledError;

                            // For rounding issues so we prevent seeing "-0" in the UI
                            if (result.Confidence < 0)
                                result.Confidence = 0;
                        }
                    }
                }

                return 1.0f;
            }

            return (float)CurrentFinIndex / Database.AllFins.Count;
        }

        private void CheckUnknownForRequiredFeatures()
        {
            if (UnknownFin == null || MatchFactors == null || MatchFactors.Count < 1)
                return;

            var featurePointTypes = new List<FeaturePointType>();
            var featureTypes = new List<FeatureType>();

            foreach (var factor in MatchFactors)
            {
                if (factor.DependentFeaturePoints != null)
                    featurePointTypes.AddRange(factor.DependentFeaturePoints);

                if (factor.DependentFeatures != null)
                    featureTypes.AddRange(factor.DependentFeatures);
            }

            var distinctFeaturePointList = featurePointTypes.Distinct().ToList();
            var distinctFeatureList = featureTypes.Distinct().ToList();

            // If the current unknown is missing any feature points, rediscover them
            // algorithmically
            if (Database.CatalogScheme != null && UnknownFin.FinOutline != null && (!UnknownFin.FinOutline.ContainsAllFeaturePointTypes(distinctFeaturePointList) || !UnknownFin.FinOutline.ContainsAllFeatureTypes(distinctFeatureList)))
                UnknownFin.FinOutline.RediscoverFeaturePoints(Database.CatalogScheme.FeatureSetType);
        }
    }
}
