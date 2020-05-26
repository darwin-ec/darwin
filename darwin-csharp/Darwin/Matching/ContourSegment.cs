using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Matching
{
    public class ContourSegment
    {
		public double Parameter { get; set; }
		public char ContourType { get; set; } // 'd' or 'u' for database or unknown
		public int StartIndex { get; set; }
		public bool Reversed { get; set; } // true means end index is startIndex - 1 rather than startIndex + 1
		public HashSet<int> IntersectingSegs { get; set; }
	}
}
