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
using System.Drawing.Imaging;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin.Extensions
{
    // TODO: Move the magic numbers somewhere out of here
    public static class ColorExtensions
    {
        public const byte WhiteByte = 255;
        public static Color ToGrayscaleColor(this Color color)
        { 
            var level = Convert.ToByte(Math.Round(color.R * 0.299 + color.G * 0.587 + color.B * 0.114));
            return Color.FromArgb(level, level, level);
        }

        public static ColorPalette GetGrayscaleColorPalette(Bitmap bmp)
        {
            // There is no regular 8bpp grayscale in .NET, only 16 bit or 8 bit indexed.
            // So we're creating an 8bpp indexed and making the palette 0 -> 255 grayscale
            ColorPalette pal = bmp.Palette;
            for (int i = 0; i <= 255; i++)
            {
                // create greyscale color table
                pal.Entries[i] = Color.FromArgb(i, i, i);
            }
            return pal;
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
