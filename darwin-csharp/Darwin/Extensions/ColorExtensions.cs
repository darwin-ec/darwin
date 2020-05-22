using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin.Extensions
{
    // TODO: Move the magic numbers somewhere out of here
    public static class ColorExtensions
    {
        public static Color ToGrayscaleColor(this Color color)
        { 
            var level = Convert.ToByte(Math.Round(color.R * 0.299 + color.G * 0.587 + color.B * 0.114));
            return Color.FromArgb(level, level, level);
        }

        public static Color ToCyanIntensity(this Color color)
        {
            byte c = Convert.ToByte(255 - color.R);
            byte m = Convert.ToByte(255 - color.G);
            byte y = Convert.ToByte(255 - color.B);

            byte k = Math.Min(c, Math.Min(m, y));

            if (k == 255)
            {
                c = 0;
                m = 0;
                y = 0;
            }
            else
            {
                c = Convert.ToByte(c - k);
                m = Convert.ToByte(m - k);
                y = Convert.ToByte(y - k);
            }

            return Color.FromArgb(c, c, c);
        }

        public static byte GetIntensity(this Color color)
        {
            // Small performance hit, but we want to make sure there aren't any rounding
            // issues here if R == G == B
            if (color.R == color.G && color.G == color.B)
                return color.R;

            return Convert.ToByte(Math.Round(color.R * 0.299 + color.G * 0.587 + color.B * 0.114));
        }
    }
}
