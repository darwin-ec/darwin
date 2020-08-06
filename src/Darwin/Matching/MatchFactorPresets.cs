// This file is part of DARWIN.
// Copyright (C) 1994 - 2020
//
// DARWIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DARWIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DARWIN.  If not, see<https://www.gnu.org/licenses/>.

using Darwin.Database;
using Darwin.Features;
using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Matching
{
    public static class MatchFactorPresets
    {
        private static List<IEnumerable<FeaturePointType>> GetVerticalFeaturePointCombinations()
        {
            var result = new List<IEnumerable<FeaturePointType>>()
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
                
                // Pass in a specific set of ratios.  If not passed, it'll automatically
                // compute the ratio permutations of all the landmarks we pass in.
                //GetVerticalFeaturePointCombinations(),

                5, // Number of desired ratios
                database.AllFins,
                //FeaturePointErrorFunctions.ComputeEigenValueWeightedCosineDistance,
                FeaturePointErrorFunctions.ComputeMahalanobisDistance,

                // The option below will compare after using the lowest error mapping
                // from the outline error function above, assuming there's an outline error
                // function before this.
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
