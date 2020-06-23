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

        public static byte GetIntensity(byte r, byte g, byte b)
        {
            if (r == g && g == b)
                return r;

            return Convert.ToByte(Math.Round(r * 0.299 + r * 0.587 + b * 0.114));
        }

        public static Color SetIntensity(this Color color, byte intensity)
        {
            byte outputR, outputG, outputB;

            float inphase = 0.596f * color.R - 0.275f * color.G - 0.321f * color.B;
            float quadrature = 0.212f * color.R - 0.523f * color.G + 0.311f * color.B;

            int redTemp = (int)Math.Round(intensity + 0.956 * inphase + 0.621 * quadrature);
            int greenTemp = (int)Math.Round(intensity - 0.272 * inphase - 0.647 * quadrature);
            int blueTemp = (int)Math.Round(intensity - 1.108 * inphase + 1.705 * quadrature);

            if (redTemp > 255)
                outputR = 255;

            else if (redTemp < 0) // Just in case!
                outputR = 0;

            else
                outputR = (byte)redTemp;

            if (greenTemp > 255)
                outputG = 255;

            else if (greenTemp < 0)
                outputG = 0;

            else
                outputG = (byte)greenTemp;

            if (blueTemp > 255)
                outputB = 255;

            else if (blueTemp < 0)
                outputB = 0;

            else
                outputB = (byte)blueTemp;

            return Color.FromArgb(outputR, outputG, outputB);
        }
    }
}
