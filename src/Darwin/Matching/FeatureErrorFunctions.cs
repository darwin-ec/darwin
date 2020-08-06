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
using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Matching
{
    public static class FeatureErrorFunctions
    {
        public static MatchError ComputeCurvatureError(
            DatabaseFin unknownFin,
            DatabaseFin databaseFin,
            MatchOptions options)
        {
            if (unknownFin == null)
                throw new ArgumentNullException(nameof(unknownFin));

            if (databaseFin == null)
                throw new ArgumentNullException(nameof(databaseFin));

            var maxError = new MatchError { Error = 5000 };

            if (databaseFin.FinOutline?.FeatureSet?.Features.ContainsKey(Features.FeatureType.BrowCurvature) != true ||
                unknownFin.FinOutline?.FeatureSet?.Features.ContainsKey(Features.FeatureType.BrowCurvature) != true)
            {
                return maxError;
            }

            var unknownCurvature = unknownFin.FinOutline?.FeatureSet?.Features[Features.FeatureType.BrowCurvature].Value;
            var databaseCurvature = databaseFin.FinOutline?.FeatureSet?.Features[Features.FeatureType.BrowCurvature].Value;

            if (unknownCurvature == null || databaseCurvature == null)
                return maxError;

            return new MatchError
            {
                Error = Math.Abs(unknownCurvature.Value - databaseCurvature.Value)
            };
        }

        public static MatchError ComputeMouthDentError(
            DatabaseFin unknownFin,
            DatabaseFin databaseFin,
            MatchOptions options)
        {
            if (unknownFin == null)
                throw new ArgumentNullException(nameof(unknownFin));

            if (databaseFin == null)
                throw new ArgumentNullException(nameof(databaseFin));

            var maxError = new MatchError { Error = 1 };

            if (databaseFin.FinOutline?.FeatureSet?.Features.ContainsKey(Features.FeatureType.HasMouthDent) != true ||
                unknownFin.FinOutline?.FeatureSet?.Features.ContainsKey(Features.FeatureType.BrowCurvature) != true)
            {
                return maxError;
            }

            var unknownHasMouthDent = unknownFin.FinOutline?.FeatureSet?.Features[Features.FeatureType.HasMouthDent].Value;
            var databaseHasMouthDent = databaseFin.FinOutline?.FeatureSet?.Features[Features.FeatureType.HasMouthDent].Value;

            if (unknownHasMouthDent == null || databaseHasMouthDent == null)
                return maxError;

            return new MatchError
            {
                Error = Math.Abs(unknownHasMouthDent.Value - databaseHasMouthDent.Value)
            };
        }
    }
}
