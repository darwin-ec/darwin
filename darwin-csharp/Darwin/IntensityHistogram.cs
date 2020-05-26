using Darwin.Extensions;
using System;
using System.Drawing;

namespace Darwin
{
    // TODO: The types are a little weird here.  Using byte length
    // but there are ints in places that probably should be byte.  Or
    // should we change from byte for intensity?
    public class IntensityHistogram
    {
        private int[] _histogram;

        public IntensityHistogram(Bitmap bmp)
        {
            if (bmp == null)
                throw new ArgumentNullException(nameof(bmp));

            if (bmp.Width < 1 || bmp.Height < 1)
                throw new ArgumentOutOfRangeException(nameof(bmp));

            _histogram = new int[byte.MaxValue + 1];

            for (var x = 0; x < bmp.Width; x++)
            {
                for (var y = 0; y < bmp.Height; y++)
                {
                    _histogram[bmp.GetPixel(x, y).GetIntensity()] += 1;
                }
            }
        }

        public int FindMaxPeak()
        {
            int maxIndex = -1;

            for (var i = 0; i < _histogram.Length; i++)
            {
                if (maxIndex == -1 || _histogram[i] > _histogram[maxIndex])
                    maxIndex = i;
            }

            return maxIndex;
        }

        public int FindMinVal()
        {
            int minIndex = -1;

            for (var i = 0; i < _histogram.Length; i++)
            {
                if (minIndex == -1 || _histogram[i] < _histogram[minIndex])
                    minIndex = i;
            }

            return minIndex;
        }

        public int GetSize()
        {
            return _histogram.Length;
        }

        public int GetValue(int position)
        {
            return _histogram[position];
        }

        public void Smooth()
        {
            // TODO: Magic number
            int maskSize = 7;

            int offset = maskSize / 2;

            int numVals = 0;
            int sum = 0;
            for (var i = 0; i < _histogram.Length; i++)
            {
                sum = numVals = 0;

                for (var pos = i - offset; pos < i + offset; pos++)
                {
                    if (pos < 0 || pos > _histogram.Length)
                        continue;

                    sum += _histogram[pos];
                    numVals++;
                }

                _histogram[i] = (byte)Math.Round((float)sum / numVals);
            }
        }

        //
        // Returns the ratio of spread values used / total number of values
        //
        //	Note: ignores black, or histogram[0]
        //
        public float FindRange()
        {
            int
                lowestVal,  // lowest value in the histogram other than black
                highestVal; // highest value in the histogram

            lowestVal = 1;

            while (lowestVal < _histogram.Length)
            {
                if (_histogram[lowestVal] > 0)
                    break;

                lowestVal++;
            }

            highestVal = _histogram.Length - 1;

            while (highestVal > lowestVal)
            {
                if (_histogram[highestVal] > 0)
                    break;

                highestVal++;
            }

            return (float)(highestVal - lowestVal) / _histogram.Length;
        }

        public float FindVariance()
        {
            if (_histogram == null)
                return 0.0f;

            float mean = 0.0f;

            double variance = 0.0;

            for (var i = 0; i < _histogram.Length; i++)
                mean += _histogram[i];

            mean /= _histogram.Length;

            for (var hPos = 0; hPos < _histogram.Length; hPos++)
                variance += (_histogram[hPos] - mean) * (_histogram[hPos] - mean);

            variance /= _histogram.Length - 1;

            return (float)variance;
        }

        /*
         * 101AT
         * 
         * Find the next relative min in the histogram. 
         * 
         * @param start The index into the histogram [ 0-getSize() ] at which to start
         * @return a Range object representing the segment of the histrogram from start
         * 	to the next valley.
         */
        public Range FindNextValley(int start)
        {
            int tip = 0; // The highest point in the peak
            int end = 0; // The end of the range comprising this peak

            int direction;
            int directionState = 0;
            int count = 0;
            int pixelCount = 0;

            for (int i = start; i < _histogram.Length; i++)
            {
                direction = GetDirection(i);
                pixelCount += _histogram[i];

                if (directionState == 0 && direction == 1)
                {
                    //first incrase
                    directionState = 1;
                }
                else if (directionState == 1 && direction == -1)
                {
                    //first pleatuea / peak
                    directionState = -1;
                }
                else if (directionState == -1)
                {
                    if (direction == 0)
                    {//plateau
                        count++;
                    }
                    else if (direction == 1 && i - tip > 10)
                    {//started increasing again, break
                        end = i - count + (int)(count / 2.0);
                        break;
                    }
                    else
                    {//(if direction==-1) //decreasing, reset count
                        count = 0;
                    }

                    /*if (count>30) {
                      //early termination
                      cout << "early termination based on many level regions" << endl;
                      end=i-count+(int)(count/2.0);
                      break;
                      }*/

                }

                end = i;
                if (_histogram[i] > _histogram[tip])
                    tip = i;

                //Cases just to continue
                //Decreasing, but have never increased
                //pleteau without a decreased proceeded by an increase
            }

            return new Range
            {
                Start = start,
                End = end,
                Tip = tip,
                HighestValue = _histogram[tip],
                PixelCount = pixelCount
            };
        }

        /*
         * 101AT
         * 
         * Look at neighbors of index to determine the general direction (slope) of the
         * 	histogram at point index.
         * 
         * @param index The position [ 0 - getSize() ] to analyize in the histogram.
         * 
         * @return -1=decreasing (e.g. negative slope)
         *          0=level		(e.g. slope==0)
         *          1=increasing	(e.g. positive slope)
         */
        public int GetDirection(int index)
        {
            // TODO: Magic number
            int diff = 15;

            int inc = 0, desc = 0, level = 0;
            int i; // loop counter can't be in loop b/c throws redefinition error when used twice.
            int start = index - diff,
              end = index + diff;

            //check bounds
            if (start < 0)
                start = 0;

            if (end > _histogram.Length)
                end = _histogram.Length;

            int current; // removed right & left
            int index_value = _histogram[index];

            for (i = start; i < index; i++)
            {
                current = _histogram[i];

                // If less than left neighbors
                if (index_value < current * 1.02)
                {
                    desc++;
                }
                // Else if greater than left neighbors
                else if (index_value * 1.02 > current)
                {
                    inc++;
                }
                else
                {
                    level++;
                }
            }

            for (i = index + 1; i < end; i++)
            {
                current = _histogram[i];

                if (index_value > current * 1.02)
                {//greater than right neighbors
                    desc++;
                }
                else if (index_value * 1.02 < current)
                {//less than right neighbors
                    inc++;
                }
                else
                {
                    level++;
                }
            }

            if (inc > desc && inc > level)
                return 1; // increasing

            if (desc > inc && desc > level)
                return -1; // decreasing

            return 0; // level
        }
    }
}
