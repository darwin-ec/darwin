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

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin.Utilities
{
    public static class MathHelper
    {
        public static double RadiansToDegrees(double radians)
        {
            return radians * (180 / Math.PI);
        }

        public static double DegreesToRadians(double degrees)
        {
            return degrees * (Math.PI / 180);
        }

        public static double GetDistance(double x1, double y1, double x2, double y2)
        {
            return Math.Sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
        }

        public static double GetDistance(System.Drawing.Point p1, System.Drawing.Point p2)
        {
            return Math.Sqrt((p2.X - p1.X) * (p2.X - p1.X) + (p2.Y - p1.Y) * (p2.Y - p1.Y));
        }

        public static int NextPowerOfTwo(int x)
        {
            double powerOfTwo = 0.0;

            for (int i = 0; (double)x > powerOfTwo; i++)
                powerOfTwo = Math.Pow(2.0, i);

            return (int)powerOfTwo;
        }

        public static double CosineSimilarity(double x1, double y1, double x2, double y2)
        {
            return LinearAlgebra.DotProduct(x1, y1, x2, y2) / (LinearAlgebra.Magnitude(x1, y1) * LinearAlgebra.Magnitude(x2, y2));
        }

        public static double CosineSimilarity(double x1, double y1, double z1, double x2, double y2, double z2)
        {
            return LinearAlgebra.DotProduct(x1, y1, z1, x2, y2, z2) / (LinearAlgebra.Magnitude(x1, y1, z1) * LinearAlgebra.Magnitude(x2, y2, z2));
        }

        public static double CosineAngularDistance(double x1, double y1, double x2, double y2)
        {
            return Math.Acos(CosineSimilarity(x1, y1, x2, y2)) / Math.PI;
        }

        public static double CosineAngularDistance(double x1, double y1, double z1, double x2, double y2, double z2)
        {
            return Math.Acos(CosineSimilarity(x1, y1, z1, x2, y2, z2)) / Math.PI;
        }
    }
}
