using Darwin.Database;
using Darwin.Features;
using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Matching
{
    public static class MatchFactorPresets
    {
        public static List<MatchFactor> CreateBearMatchFactors(DarwinDatabase database)
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
    }
}
