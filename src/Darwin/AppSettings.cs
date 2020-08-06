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
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin
{
    // TODO: Move the image processing stuff out of this
    // Put this in a settings file or something
    public static class AppSettings
    {
        //public static string MLModelFilename = "bear_coordinates_regular_noscale_flip.tflite";
        public static string MLModelFilename = "resnetv2_noscale_flip_3.tflite";

        public static string DocumentationUrl
        {
            get
            {
                return Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Documentation\\usersguide.htm");
            }
        }
        public const int MaxZoom = 1600;
        public const int MinZoom = 6;

        public const string DarwinModsFilenameAppend = "_wDarwinMods";
        public const string DarwinModsFilenameAppendPng = "_wDarwinMods.png";

        public static readonly float[][] GrayscaleConversionMatrix = new float[][] {
                new float[] { 0.299f, 0.299f, 0.299f, 0, 0 },
                new float[] { 0.587f, 0.587f, 0.587f, 0, 0 },
                new float[] { 0.114f, 0.114f, 0.114f, 0, 0 },
                new float[] { 0,      0,      0,      1, 0 },
                new float[] { 0,      0,      0,      0, 1 }
            };

        public static readonly int[] ErosionMatrix = new int[] {
          0,  1,  1,  2,    1,  2,  2,  3,    1,  2,  2,  3,    2,  3,  3,  4,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          1,  2,  2,  3,    2,  3,  3,  4,    2,  3,  3,  4,    3,  4,  4,  5,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,

          1,  2,  2,  3,    2,  3,  3,  4,    2,  3,  3,  4,    3,  4,  4,  5,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          2,  3,  3,  4,    3,  4,  4,  5,    3,  4,  4,  5,    4,  5,  5,  6,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,

          1,  2,  2,  3,    2,  3,  3,  4,    2,  3,  3,  4,    3,  4,  4,  5,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          2,  3,  3,  4,    3,  4,  4,  5,    3,  4,  4,  5,    4,  5,  5,  6,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,

          2,  3,  3,  4,    3,  4,  4,  5,    3,  4,  4,  5,    4,  5,  5,  6,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          3,  4,  4,  5,    4,  5,  5,  6,    4,  5,  5,  6,    5,  6,  6,  7,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,

          1,  2,  2,  3,    2,  3,  3,  4,    2,  3,  3,  4,    3,  4,  4,  5,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          2,  3,  3,  4,    3,  4,  4,  5,    3,  4,  4,  5,    4,  5,  5,  6,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,

          2,  3,  3,  4,    3,  4,  4,  5,    3,  4,  4,  5,    4,  5,  5,  6,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          3,  4,  4,  5,    4,  5,  5,  6,    4,  5,  5,  6,    5,  6,  6,  7,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,

          2,  3,  3,  4,    3,  4,  4,  5,    3,  4,  4,  5,    4,  5,  5,  6,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          3,  4,  4,  5,    4,  5,  5,  6,    4,  5,  5,  6,    5,  6,  6,  7,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,

          3,  4,  4,  5,    4,  5,  5,  6,    4,  5,  5,  6,    5,  6,  6,  7,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          4,  5,  5,  6,    5,  6,  6,  7,    5,  6,  6,  7,    6,  7,  7,  8,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0
        };

        public static readonly int[] DilationMatrix = new int[] {
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          8,  7,  7,  6,    7,  6,  6,  5,    7,  6,  6,  5,    6,  5,  5,  4,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          7,  6,  6,  5,    6,  5,  5,  4,    6,  5,  5,  4,    5,  4,  4,  3,

          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          7,  6,  6,  5,    6,  5,  5,  4,    6,  5,  5,  4,    5,  4,  4,  3,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          6,  5,  5,  4,    5,  4,  4,  3,    5,  4,  4,  3,    4,  3,  3,  2,

          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          7,  6,  6,  5,    6,  5,  5,  4,    6,  5,  5,  4,    5,  4,  4,  3,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          6,  5,  5,  4,    5,  4,  4,  3,    5,  4,  4,  3,    4,  3,  3,  2,

          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          6,  5,  5,  4,    5,  4,  4,  3,    5,  4,  4,  3,    4,  3,  3,  2,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          5,  4,  4,  3,    4,  3,  3,  2,    4,  3,  3,  2,    3,  2,  2,  1,

          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          7,  6,  6,  5,    6,  5,  5,  4,    6,  5,  5,  4,    5,  4,  4,  3,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          6,  5,  5,  4,    5,  4,  4,  3,    5,  4,  4,  3,    4,  3,  3,  2,

          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          6,  5,  5,  4,    5,  4,  4,  3,    5,  4,  4,  3,    4,  3,  3,  2,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          5,  4,  4,  3,    4,  3,  3,  2,    4,  3,  3,  2,    3,  2,  2,  1,

          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          6,  5,  5,  4,    5,  4,  4,  3,    5,  4,  4,  3,    4,  3,  3,  2,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          5,  4,  4,  3,    4,  3,  3,  2,    4,  3,  3,  2,    3,  2,  2,  1,

          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          5,  4,  4,  3,    4,  3,  3,  2,    4,  3,  3,  2,    3,  2,  2,  1,
          0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,    0,  0,  0,  0,
          4,  3,  3,  2,    3,  2,  2,  1,    3,  2,  2,  1,    2,  1,  1,  0
        };
    }
}
