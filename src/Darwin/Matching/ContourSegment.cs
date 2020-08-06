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
