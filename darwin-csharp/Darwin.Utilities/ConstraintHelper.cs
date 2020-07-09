using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Utilities
{
    public static class ConstraintHelper
    {
        public static void ConstrainInt(ref int val, int min, int max)
        {
            if (val < min)
                val = min;
            if (val > max)
                val = max;
        }
    }
}
