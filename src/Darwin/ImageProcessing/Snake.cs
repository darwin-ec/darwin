//*******************************************************************
//   file: snake.cxx
//
// author: Mark Allen
//
//   mods: 
//
// Much of this code is from Active Contour code from USF and other 
// sources.
//
//*******************************************************************

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

using Darwin.Extensions;
using Darwin.Utilities;
using System;
using System.Drawing;

namespace Darwin.ImageProcessing
{
    public static class Snake
    {
        private const float BigFloat = 1.0e20f;

        public static float EnergyCalc(
            DirectBitmap edgeImage,
            float[] energyWeights,
            int xprevpt,
            int yprevpt,
            int xcurrpt,
            int ycurrpt,
            int xnextpt,
            int ynextpt,
            float averageDistance
        )
        {
            if (edgeImage == null)
                return BigFloat;

            float
                energyCont,
                energyLin,
                energyEdgeStrength,
                energyContDiff;

            int numRows = edgeImage.Height;
            int numCols = edgeImage.Width;

            //  First thing to do is to check image boundaries.  To keep
            //  a point from moving off the image, any neighborhood point
            //  out-of-bounds gets its energy set very large.  Since the
            //  whole idea is to minimize a point's energy, the point
            //  will never take the out-of-bounds route.

            if (xcurrpt < 0 || xcurrpt >= numCols || ycurrpt < 0 || ycurrpt >= numRows)
                return BigFloat;    // no moving past image boundaries

            // energyCont => continuity; how evenly spaced the points on the
            // contour are.  The distance between the point and the point  
            // behind it is compared to averageDistance.  The closer, the lower the   
            // energy.

            // Determines distance from current to previous point, then squares it

            // Calculates distance between the point and the point after it, compare to averageDistance
            energyContDiff = averageDistance - (float)Math.Sqrt(
                        (xcurrpt - xprevpt)
                         * (xcurrpt - xprevpt)
                         + (ycurrpt - yprevpt)
                         * (ycurrpt - yprevpt));

            // Determines energy continuity to be a sum of the distance from the current to the
            // previous point with the deviation of the distance from the average point spacing 
            // multiplied by some scaling factor

            energyCont = energyContDiff * energyContDiff;    // squares the Average distance difference

            // energyLin => linearality; how well the point, plus the points on
            // either side, make a straight line.  The straighter, the lower  
            // the energy.
            energyLin = (float)(xprevpt - 2 * xcurrpt + xnextpt)
            * (float)(xprevpt - 2 * xcurrpt + xnextpt)
            + (float)(yprevpt - 2 * ycurrpt + ynextpt)
            * (float)(yprevpt - 2 * ycurrpt + ynextpt);


            energyEdgeStrength = -1 * (float)edgeImage.GetPixel(xcurrpt, ycurrpt).GetIntensity();

            // Now let's add up those weighted energies
            return energyWeights[0] * energyCont
                + energyWeights[1] * energyLin
                + energyWeights[2] * energyEdgeStrength;
        }

        public static bool MoveContour(
            ref Contour contour,   // The contour we'd like to move

            DirectBitmap edgeImage, // Edge strength image used for energy calculations

            int neighborhoodSize,   // Neighborhood window size
                                    // neighborhoodSize x neighborhoodSize

            float[] energyWeights
        )
        {
            if (contour == null)
                throw new ArgumentNullException(nameof(contour));

            if (edgeImage == null)
                throw new ArgumentNullException(nameof(edgeImage));

            if (neighborhoodSize <= 0)
                throw new ArgumentOutOfRangeException(nameof(neighborhoodSize));


            int
                minPosition = 0,    // keeps track of the value of k (the current node)
                                    // corresponding to the minimum energy found so far
                                    // in the search using k over all neighbors of the
                                    // current node
                                    // It's initialized to zero because there's a (very)
                                    // slight chance that it could be used unitialized
                                    // otherwise

                numNeighbors;

            float
                minEnergy,  // keeps track of the minimum energy found so far
                            // in the search using k over all neighbors of the
                            // current node. 

                energy;     // is used in the search over the neighbors of the
                            // current node as a temporary storage of the energy computed

            System.Drawing.Point
                nextNode = new System.Drawing.Point(),   // contains the coordinates, for the neighbor of the
                                          // next node indexed by j. These coordinates are used
                                          // only to compute the energy associated with the
                                          // associated choice of snaxel positions

                currNode = new System.Drawing.Point();   // contains the coordinates, for the neighbor of the
                                          // current node indexed by k. These coordinates are
                                          // used only to compute the energy associated with the
                                          // associated choice of snaxel positions

            numNeighbors = neighborhoodSize * neighborhoodSize;

            int numPoints = contour.NumPoints;

            if (numPoints <= 2)
                return false;

            float[,] energyMtx = new float[numPoints - 1, numNeighbors];
            int[,] posMtx = new int[numPoints - 1, numNeighbors];

            System.Drawing.Point[] neighbor = new System.Drawing.Point[numNeighbors];

            for (int a = 0; a < numNeighbors; a++)
            {
                neighbor[a].X = (a % neighborhoodSize) - (neighborhoodSize - 1) / 2;
                neighbor[a].Y = (a / neighborhoodSize) - (neighborhoodSize - 1) / 2;
            }

            // initialize first column of energy matrix
            // This block of code simply sets the first column of the energy
            // matrix to zero, since there is no energy associated solely
            // with the first snaxel; energy is based on distance, and there
            // is no distance associated with a single snaxel. It is only when
            // we get to the second snaxel, that we have some measure of distance
            for (int l = 0; l < numNeighbors; l++)
            {
                energyMtx[0, l] = 0.0f;
                posMtx[0, l] = 0;
            }

            // Find the average distance between points
            float distSum = 0.0f;

            for (int d = 1; d < numPoints; d++)
                distSum += (float)MathHelper.GetDistance(contour[d].X, contour[d].Y, contour[d - 1].X, contour[d - 1].Y);

            float averageDistance = distSum / numPoints;

            for (int i = 1; i < numPoints - 1; i++)
            {
                for (int j = 0; j < numNeighbors; j++)
                { // for all neighbors of next node
                    minEnergy = BigFloat;

                    nextNode.X = contour[i + 1].X + neighbor[j].X;
                    nextNode.Y = contour[i + 1].Y + neighbor[j].Y;

                    for (int k = 0; k < numNeighbors; k++)
                    { // for all neighors of curr node
                        currNode.X = contour[i].X + neighbor[k].X;
                        currNode.Y = contour[i].Y + neighbor[k].Y;

                        energy = energyMtx[i - 1, k] +
                             EnergyCalc(
                                edgeImage,
                                energyWeights,
                                contour[i - 1].X + neighbor[posMtx[i - 1, k]].X,
                                contour[i - 1].Y + neighbor[posMtx[i - 1, k]].Y,
                                currNode.X, currNode.Y,
                                nextNode.X, nextNode.Y,
                                averageDistance
                            );
                        if (energy < minEnergy)
                        {
                            minEnergy = energy;
                            minPosition = k;
                        }
                    }

                    // Store minimum energy into table matrix
                    energyMtx[i, j] = minEnergy;
                    posMtx[i, j] = minPosition;
                }
            }

            int pos = posMtx[numPoints - 2, 4];

            // search backwards through table to find optimum positions
            for (int k = numPoints - 2; k > 0; k--)
            {
                contour[k] = new Darwin.Point(contour[k].X + neighbor[pos].X, contour[k].Y + neighbor[pos].Y);

                pos = posMtx[k - 1, pos];
            }

            return true;
        }
    }
}
