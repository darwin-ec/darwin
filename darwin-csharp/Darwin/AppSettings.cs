﻿using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin
{
    // TODO: Move the image processing stuff out of this
    public static class AppSettings
    {
        public const int MaxZoom = 1600;
        public const int MinZoom = 6;
        public static float DrawingPointSize = 4;

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

        public static int SnakeMaximumIterations = 50;
        public static float SnakeEnergyContinuity = 9.0f;
        public static float SnakeEnergyLinearity = 3.0f;
        public static float SnakeEnergyEdge = 3.0f;

        public static float GaussianStdDev = 1.5f;
        public static float CannyLowThreshold = 0.15f;
        public static float CannyHighThreshold = 0.85f;
    }
}
