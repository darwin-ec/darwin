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
    }
}
