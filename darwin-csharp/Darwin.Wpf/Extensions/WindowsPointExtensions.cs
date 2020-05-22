using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Wpf.Extensions
{
    public static class WindowsPointExtensions
    {
        public static Darwin.Point ToDarwinPoint(this System.Windows.Point point)
        {
            return new Darwin.Point((int)Math.Round(point.X), (int)Math.Round(point.Y));
        }
    }
}
