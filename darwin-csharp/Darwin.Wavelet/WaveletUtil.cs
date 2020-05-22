using Darwin.Utilities;
using System;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net.Http.Headers;
using System.Text;

namespace Darwin.Wavelet
{
    public static class WaveletUtil
    {
        public static WL_Filter MZLowPassFilter = new WL_Filter
        {
            Info = new WL_Struct { Name = "mz", Type = "sbfilter", PList = null },
            Length = 4,
            Offset = 2,
            Coefs = new double[] { 0.125, 0.375, 0.375, 0.125 }
        };

        public static WL_Filter MZHighPassFilter = new WL_Filter
        {
            Info = new WL_Struct { Name = "mz", Type = "sbfilter", PList = null },
            Length = 2,
            Offset = 0,
            Coefs = new double[] { 2.0, 2.0 }
        };

        public static double[] NormalizationCoefficients = new double[] { 1.50, 1.12, 1.03, 1.01, 1.0 };

        public static double NormalizationCoeff(int level)
        {
            if (level <= 0)
                return 0.0;

            if (level < 5)
                return NormalizationCoefficients[level - 1];

            return 1.0;
        }

        public static void ModulusMaxima(double[] src, ref double[] modmax, int length)
        {
            if (length < 2)
                throw new ArgumentOutOfRangeException(nameof(length));

            for (int k = 0; k < length; k++)
                modmax[k] = 0.0;

            bool increase = false;

            if (src[1] >= src[0])
                increase = true;

            // this is sloppy, i know
            // ... i can't think today
            for (int m = 1; m < length; m++)
            {
                if (increase)
                {
                    for (; m < length; m++)
                    {
                        if (src[m] < src[m - 1])
                        {
                            modmax[m - 1] = src[m - 1];
                            increase = false;
                            break;
                        }
                    }
                }
                else
                {
                    for (; m < length; m++)
                    {
                        if (src[m] > src[m - 1])
                        {
                            modmax[m - 1] = src[m - 1];
                            increase = true;
                            break;
                        }
                    }
                }
            }
        }

        public static void WaveGenCoeffFiles(
            string name,
            double[] chain,
            int levels)
        {
            if (chain == null)
                throw new ArgumentNullException(nameof(chain));

            if (levels < 0)
                throw new ArgumentOutOfRangeException(nameof(levels));

            int numPoints = chain.Length;


            // First, make a copy without the first value in the chain,
            // since the first value skews the rest of the chain and is
            // unnecessary for our purposes here
            double[] src = new double[numPoints - 1];
            Array.Copy(chain, 1, src, 0, numPoints - 1);


            // Now set up the variables needed to perform a wavelet
            // transform on the chain
            double[,] continuousResult = new double[levels - 1, MathHelper.NextPowerOfTwo(numPoints - 1)];

            // Now perform the transformation
            WIRWavelet.WL_FrwtVector(src,
                      ref continuousResult,
                      numPoints - 1,
                      levels,
                      WaveletUtil.MZLowPassFilter, WaveletUtil.MZHighPassFilter);

            var filename = string.Format("transforms/{0]-chain", name);
            using (var file = new StreamWriter(filename))
            {
                for (var c = 0; c < numPoints - 1; c++)
                    file.WriteLine(src[c]);
            }

            for (int i = 1; i <= levels; i++)
            {
                var fname = string.Format("transforms/{0}-level{1}", name, i);
                using (var file = new StreamWriter(fname))
                {
                    for (int j = 0; j < numPoints - 1; j++)
                        file.WriteLine(continuousResult[i, j] * NormalizationCoeff(i));
                }
            }
        }

        public static void InitFilters()
        {
            MZLowPassFilter.Coefs = new double[4];

            MZLowPassFilter.Coefs[0] = 0.125;
            MZLowPassFilter.Coefs[1] = 0.375;
            MZLowPassFilter.Coefs[2] = 0.375;
            MZLowPassFilter.Coefs[3] = 0.125;

            MZHighPassFilter.Coefs = new double[2];

            MZHighPassFilter.Coefs[0] = -2.0;
            MZHighPassFilter.Coefs[1] = 2.0;
        }

        /* WL_isDyadic
		 *
		 * Tests to see if X is a power of 2.  Returns TRUE or FALSE.
		 */
        public static int WL_isDyadic(int x)
        {
            if (WL_pow2(WL_log2(x)) != x) return 0;
            else return 1;
        }

        /* WL_log2
		 *
		 * Computes the largest base 2 logarithm less than or equal to LOG2(X).
		 */
        public static int WL_log2(int x)
        {
            int n = 0;
            int prod = 1;

            while (prod <= x)
            {
                n++;
                prod *= 2;
            }
            return (n - 1);
        }

        /* WL_pow2
		 *
		 * Computes 2 raised to the Xth power.
		 */
        public static int WL_pow2(int x)
        {
            int n = 1;

            if (x < 0)
            {
                Trace.WriteLine("\nWL_pow2: negative argument! Returning 0\n");
                return (0);
            }

            while (x-- > 0)
                n *= 2;

            return n;
        }

        public static double[] Extract1DArray(double[,] source, int firstIndex, int length)
        {
            double[] dest = new double[length];
            Array.Copy(source, firstIndex * length, dest, 0, length);
            return dest;
        }

        public static void Patch1DArray(double[] source, ref double[,] dest, int firstIndex, int length)
        {
            Array.Copy(source, 0, dest, firstIndex * length, length);
        }

        public static double[] Extract1DArray(double[,,] source, int firstIndex, int secondIndex, int length)
        {
            double[] dest = new double[length];

            // TODO: This should really be an Array.Copy
            for (int i = 0; i < length; i++)
                dest[i] = source[firstIndex, secondIndex, i];

            return dest;
        }

        public static void Patch1DArray(double[] source, ref double[,,] dest, int firstIndex, int secondIndex, int length)
        {
            // TODO: This should really be an Array.Copy
            for (int i = 0; i < length; i++)
                dest[firstIndex, secondIndex, i] = source[i];
        }

        public static double[,] WL_Extract2D(double[,] data, int sR, int sC, int numR, int numC)
        {
            int i, j;
            double[,] res = new double[numR, numC];

            for (i = 0; i < numR; i++)
                for (j = 0; j < numC; j++)
                    res[i, j] = data[i + sR, j + sC];

            return res;
        }

        public static void WL_Put2D(ref double[,] data, double[,] putD, int sR, int sC, int numR, int numC)
        {
            int i, j;

            for (i = 0; i < numR; i++)
                for (j = 0; j < numC; j++)
                    data[i + sR, j + sC] = putD[i, j];
        }

        /* WL_ReadAsciiDataFile
         *
         * Reads in the contents of an ascii data file.  The data values may be
         * preceeded by any number of comment lines that begin with the
         * character '#'.
         *
         * Data files represent either vectors or matrices.  If there is more than
         * one value on the first line of data, then the number of items on that
         * first line is taken to be the number of columns of data in a matrix.
         * If the total number of data items read is not evenly divisible by
         * the number of columns detected, the file is assumed to be a vector.
         */
         // TODO: Maybe this should actually create the objects?
         // TODO: The port on this hasn't been verified -- changed it a bit
        public static int WL_ReadAsciiDataFile(string filename, out int rows,
                     out int cols, out double[] data)
        {
            rows = cols = 0;
            data = null;
            List<double> dataTemp = new List<double>();
            try
            {
                using (var filein = new StreamReader(filename))
                {
                    string line = string.Empty;

                    while ((line = filein.ReadLine()) != null)
                    {
                        if (string.IsNullOrWhiteSpace(line))
                            continue;

                        line = line.Trim();

                        // Comments
                        if (line.StartsWith("#"))
                            continue;

                        
                        var splitVals = line.Split(' ');

                        if (splitVals.Length > 0)
                        {
                            int currentCols = 0;

                            foreach (var v in splitVals)
                            {
                                double d;
                                if (!string.IsNullOrEmpty(v) && double.TryParse(v, out d))
                                {
                                    dataTemp.Add(d);
                                    currentCols += 1;
                                }
                            }

                            if (currentCols > cols)
                                cols = currentCols;

                            rows += 1;
                        }
                    }
                }

                if (rows == 0 || cols == 0)
                    return 1;

                if (rows % cols != 0)
                {
                    cols = 1;
                }
                else
                {
                    rows = rows / cols;
                }

                data = dataTemp.ToArray();
                return 0;
            }
            catch
            {
                return 1;
            }
        }
    }
}
