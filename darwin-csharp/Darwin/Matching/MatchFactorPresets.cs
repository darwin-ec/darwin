using Darwin.Database;
using Darwin.Features;
using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Matching
{
    public static class MatchFactorPresets
    {
        private static List<List<FeaturePointType>> GetVerticalFeaturePointCombinations()
        {
            var result = new List<List<FeaturePointType>>()
            {
                new List<FeaturePointType>() { FeaturePointType.Tip, FeaturePointType.Notch },
                new List<FeaturePointType>() { FeaturePointType.Nasion, FeaturePointType.PointOfInflection },
                new List<FeaturePointType>() { FeaturePointType.Tip, FeaturePointType.UpperLip },
                new List<FeaturePointType>() { FeaturePointType.Notch, FeaturePointType.UpperLip }
            };

            return result;
        }

        public static List<MatchFactor> CreateBearMatchFactors(DarwinDatabase database)
        {
            var matchFactors = new List<MatchFactor>();

            var controlPoints = new List<FeaturePointType>()
            {
                FeaturePointType.Nasion,
                FeaturePointType.Tip,
                FeaturePointType.PointOfInflection,

                // Our alternate control point to also try a mapping on
                FeaturePointType.BottomLipProtrusion
            };

            matchFactors.Add(MatchFactor.CreateOutlineFactor(
                0.55f,
                controlPoints,
                //OutlineErrorFunctions.MeanSquaredErrorBetweenOutlinesWithControlPoints,
                OutlineErrorFunctions.MeanSquaredErrorBetweenOutlineSegments,
                OutlineErrorFunctions.FindErrorBetweenOutlinesWithControlPointJitter,
                new OutlineMatchOptions
                {
                    MoveTip = true,
                    MoveEndsInAndOut = false,
                    UseFullFinError = true,
                    JumpDistancePercentage = 0.01f,
                    TrimBeginLeadingEdge = true,

                    // Also need to make sure there are 4 control points passed in if this is true
                    TryAlternateControlPoint3 = false
                }));

            matchFactors.Add(MatchFactor.CreateFeatureFactor(
                0.1f,
                FeatureErrorFunctions.ComputeCurvatureError,
                FeatureType.BrowCurvature));

            matchFactors.Add(MatchFactor.CreateFeatureFactor(
                0.05f,
                FeatureErrorFunctions.ComputeMouthDentError,
                FeatureType.HasMouthDent));

            var benchmarkFeatures = new List<FeaturePointType>()
            {
                //FeaturePointType.Nasion,
                //FeaturePointType.Tip
                FeaturePointType.Tip,
                FeaturePointType.Notch
            };

            var landmarkFeatures = new List<FeaturePointType>()
            {
                //FeaturePointType.LeadingEdgeBegin,
                FeaturePointType.Tip,
                FeaturePointType.Nasion,
                FeaturePointType.Notch,
                FeaturePointType.UpperLip,
                FeaturePointType.PointOfInflection
            };

            matchFactors.Add(MatchFactor.CreateFeaturePointFactor(
                0.35f,
                benchmarkFeatures,
                landmarkFeatures,
                5, // Number of desired ratios
                database.AllFins,
                //FeaturePointErrorFunctions.ComputeEigenValueWeightedCosineDistance,
                FeaturePointErrorFunctions.ComputeMahalanobisDistance,
                new FeatureSetMatchOptions
                {
                    UseRemappedOutline = false
                }));

            return matchFactors;
        }

        public static List<MatchFactor> CreateBearMatchFactorsOld(DarwinDatabase database)
        {
            var matchFactors = new List<MatchFactor>();

            var controlPoints = new List<FeaturePointType>()
            {
                FeaturePointType.Nasion,
                FeaturePointType.Tip,
                FeaturePointType.PointOfInflection
            };

            var benchmarkFeatures = new List<FeaturePointType>()
            {
                FeaturePointType.Nasion,
                FeaturePointType.Tip
                //FeaturePointType.Tip,
                //FeaturePointType.Notch
            };

            var landmarkFeatures = new List<FeaturePointType>()
            {
                //FeaturePointType.LeadingEdgeBegin,
                FeaturePointType.Tip,
                FeaturePointType.Nasion,
                FeaturePointType.Notch,
                FeaturePointType.UpperLip,
                FeaturePointType.PointOfInflection
            };

            matchFactors.Add(MatchFactor.CreateOutlineFactor(
                0.6f,
                controlPoints,
                //OutlineErrorFunctions.MeanSquaredErrorBetweenOutlinesWithControlPoints,
                OutlineErrorFunctions.MeanSquaredErrorBetweenOutlineSegments,
                OutlineErrorFunctions.FindErrorBetweenOutlinesWithControlPointJitter,
                new OutlineMatchOptions
                {
                    MoveTip = true,
                    MoveEndsInAndOut = false,
                    UseFullFinError = true
                }));

            matchFactors.Add(MatchFactor.CreateFeaturePointFactor(
                0.4f,
                benchmarkFeatures,
                landmarkFeatures,
                5, // Number of desired ratios
                database.AllFins,
                //FeaturePointErrorFunctions.ComputeEigenValueWeightedCosineDistance,
                FeaturePointErrorFunctions.ComputeMahalanobisDistance,
                new FeatureSetMatchOptions
                {
                    UseRemappedOutline = false
                }));

            return matchFactors;
        }

        public static List<MatchFactor> CreateBearMatchFactorsOld2(DarwinDatabase database)
        {
            var matchFactors = new List<MatchFactor>();

            var controlPoints = new List<FeaturePointType>()
            {
                FeaturePointType.Nasion,
                FeaturePointType.Tip,
                FeaturePointType.PointOfInflection,

                // Our alternate control point to also try a mapping on
                FeaturePointType.BottomLipProtrusion
            };

            matchFactors.Add(MatchFactor.CreateOutlineFactor(
                0.55f,
                controlPoints,
                //OutlineErrorFunctions.MeanSquaredErrorBetweenOutlinesWithControlPoints,
                OutlineErrorFunctions.MeanSquaredErrorBetweenOutlineSegments,
                OutlineErrorFunctions.FindErrorBetweenOutlinesWithControlPointJitter,
                new OutlineMatchOptions
                {
                    MoveTip = true,
                    MoveEndsInAndOut = false,
                    UseFullFinError = true,
                    JumpDistancePercentage = 0.01f,
                    TrimBeginLeadingEdge = true,
                    TryAlternateControlPoint3 = true
                }));

            matchFactors.Add(MatchFactor.CreateFeatureFactor(
                0.1f,
                FeatureErrorFunctions.ComputeCurvatureError,
                FeatureType.BrowCurvature));

            matchFactors.Add(MatchFactor.CreateFeatureFactor(
                0.05f,
                FeatureErrorFunctions.ComputeMouthDentError,
                FeatureType.HasMouthDent));

            var benchmarkFeatures = new List<FeaturePointType>()
            {
                FeaturePointType.Nasion,
                FeaturePointType.Tip
                //FeaturePointType.Tip,
                //FeaturePointType.Notch
            };

            var landmarkFeatures = new List<FeaturePointType>()
            {
                //FeaturePointType.LeadingEdgeBegin,
                FeaturePointType.Tip,
                FeaturePointType.Nasion,
                FeaturePointType.Notch,
                FeaturePointType.UpperLip,
                FeaturePointType.PointOfInflection
            };

            matchFactors.Add(MatchFactor.CreateFeaturePointFactor(
                0.35f,
                benchmarkFeatures,
                landmarkFeatures,
                5, // Number of desired ratios
                database.AllFins,
                //FeaturePointErrorFunctions.ComputeEigenValueWeightedCosineDistance,
                FeaturePointErrorFunctions.ComputeMahalanobisDistance,
                new FeatureSetMatchOptions
                {
                    UseRemappedOutline = false
                }));

            return matchFactors;
        }
    }
}
