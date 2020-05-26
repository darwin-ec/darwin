using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Extensions
{
    public static class DarwinPointExtensions
    {
        public static System.Drawing.Point ToDrawingPoint(this Darwin.Point point)
        {
            return new System.Drawing.Point(point.X, point.Y);
        }

        public static Darwin.Point ToDarwinPoint(this System.Drawing.Point point)
        {
            return new Darwin.Point(point.X, point.Y);
        }
    }
}
