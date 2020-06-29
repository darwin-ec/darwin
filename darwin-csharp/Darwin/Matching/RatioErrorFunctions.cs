using Darwin.Database;
using Darwin.Features;
using Darwin.Utilities;
using MathNet.Numerics;
using MathNet.Numerics.LinearAlgebra;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;

namespace Darwin.Matching
{
    public class LandmarkComparison
    {
        public List<PointF> Coordinates { get; set; }
        public Vector<Complex> Landmarks { get; set; }
    }

    public static class RatioErrorFunctions
    {
        /// <summary>
        /// J. Shi et al. / Computer Vision and Image Understanding 102 (2006) 117–133
        /// Section 3
        /// </summary>
        /// <param name="landmarkFeatures"></param>
        /// <param name="allDatabaseIndividuals"></param>
        public static void ComputeLandmarkCovarianceMatrix(List<FeaturePointType> landmarkFeatures, List<DatabaseFin> allDatabaseIndividuals)
        {
            // First, let's find the X/Y coordinates of every feature of
            // every individual, translate/scale them, and create Complex vectors with them

            var comparisonValues = new List<LandmarkComparison>();
            var totalLandmarks = CreateVector.Dense<Complex>(landmarkFeatures.Count);

            foreach (var individual in allDatabaseIndividuals)
            {
                List<PointF> coordinates = new List<PointF>();
                foreach (var featurePoint in landmarkFeatures)
                {
                    coordinates.Add(individual.FinOutline.GetFeaturePointCoords(featurePoint));
                }

                // Now, we need to translate them by moving the geometric center of the 
                // features to 0,0
                float averageX = coordinates.Where(p => !p.IsEmpty).Average(p => p.X);
                float averageY = coordinates.Where(p => !p.IsEmpty).Average(p => p.Y);

                // And we're also going to need to scale it
                float xRange = coordinates.Where(p => !p.IsEmpty).Max(p => p.X) - coordinates.Where(p => !p.IsEmpty).Min(p => p.X);
                float yRange = coordinates.Where(p => !p.IsEmpty).Max(p => p.Y) - coordinates.Where(p => !p.IsEmpty).Min(p => p.Y);

                float scale = (float)Math.Sqrt(Math.Pow(xRange, 2) + Math.Pow(yRange, 2));

                var scaledTranslatedCoordinates = coordinates.Select(c => new PointF
                {
                    X = (c.X - averageX) / scale,
                    Y = (c.Y - averageY) / scale
                }).ToList();

                var landmarkVector = CreateVector.Dense<Complex>(scaledTranslatedCoordinates.Select(c => new Complex(c.X, c.Y)).ToArray());

                totalLandmarks += landmarkVector;

                comparisonValues.Add(new LandmarkComparison
                {
                    Coordinates = scaledTranslatedCoordinates,
                    Landmarks = landmarkVector
                });
            }

            Vector<Complex> averageLandmarks = totalLandmarks / comparisonValues.Count;

            var galleryMatrix = CreateMatrix.Dense<Complex>(landmarkFeatures.Count, comparisonValues.Count);

            int j = 0;
            foreach (var individual in comparisonValues)
            {
                Vector<Complex> translatedLandmarks = individual.Landmarks - averageLandmarks;
                for (var i = 0; i < landmarkFeatures.Count; i++)
                {
                    galleryMatrix[i, j] = translatedLandmarks[i];
                }

                j += 1;
            }

            var covarianceMatrix = galleryMatrix * galleryMatrix.Transpose();
        }

        /// <summary>
        /// J. Shi et al. / Computer Vision and Image Understanding 102 (2006) 117–133
        /// Section 4.2
        /// </summary>
        /// <param name="benchmarkFeatures">Needs to have just two values</param>
        /// <param name="landmarkFeatures"></param>
        /// <param name="allDatabaseIndividuals"></param>
        public static void ComputeRatioCovarianceMatrix(List<FeaturePointType> benchmarkFeatures, List<FeaturePointType> landmarkFeatures, List<DatabaseFin> allDatabaseIndividuals)
        {
            var coordinates = new Dictionary<FeaturePointType, PointF>();

            var ratioPermutations = EnumerableHelper.GetPermutations(landmarkFeatures, 2).ToList();

            List<Vector<double>> ratioList = new List<Vector<double>>();
            var totalRatios = CreateVector.Dense<double>(ratioPermutations.Count - 1);

            foreach (var individual in allDatabaseIndividuals)
            {
                foreach (var featurePoint in landmarkFeatures)
                {
                    coordinates[featurePoint] = individual.FinOutline.GetFeaturePointCoords(featurePoint);
                }

                var benchmarkDistance = MathHelper.GetDistance(
                    coordinates[benchmarkFeatures[0]].X,
                    coordinates[benchmarkFeatures[0]].Y,
                    coordinates[benchmarkFeatures[1]].X,
                    coordinates[benchmarkFeatures[1]].Y);

                Vector<double> ratios = CreateVector.Dense<double>(ratioPermutations.Count - 1);

                int i = 0;
                foreach (var permutation in ratioPermutations)
                {
                    var permutationList = permutation.ToList();

                    if ((permutationList[0] == benchmarkFeatures[0] && permutationList[1] == benchmarkFeatures[1]) ||
                        (permutationList[0] == benchmarkFeatures[1] && permutationList[1] == benchmarkFeatures[0]))
                    {
                        // This is our benchmark
                        continue;
                    }

                    var currentDistance = MathHelper.GetDistance(
                        coordinates[permutationList[0]].X,
                        coordinates[permutationList[0]].Y,
                        coordinates[permutationList[1]].X,
                        coordinates[permutationList[1]].Y);

                    ratios[i] = currentDistance / benchmarkDistance;

                    i += 1;
                }
                totalRatios += ratios;
                ratioList.Add(ratios);
            }

            Vector<double> averageRatios = totalRatios / (ratioPermutations.Count - 1);

            var galleryMatrix = CreateMatrix.Dense<Complex>(ratioPermutations.Count - 1, allDatabaseIndividuals.Count);

            int j = 0;
            foreach (var ratioVector in ratioList)
            {
                Vector<double> translatedRatios = ratioVector - averageRatios;

                for (var i = 0; i < translatedRatios.Count; i++)
                {
                    galleryMatrix[i, j] = translatedRatios[i];
                }

                j += 1;
            }

            var covarianceMatrix = galleryMatrix * galleryMatrix.Transpose();

            var evd = covarianceMatrix.Evd(Symmetricity.Unknown);
            var eigenValues = evd.EigenValues;
            var eigenVectors = evd.EigenVectors;
        }
    }
}
