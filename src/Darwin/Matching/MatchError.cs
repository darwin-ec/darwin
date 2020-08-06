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

using MathNet.Numerics.LinearAlgebra;
using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Matching
{
	public class MatchFactorError
	{
		public int FactorIndex { get; set; }
		public double Error { get; set; }
		public double Weight { get; set; }
	}

	public class MatchError
	{
		public double Error { get; set; }
		public FloatContour Contour1 { get; set; }
		public FloatContour Contour2 { get; set; }

		public int Contour1ControlPoint1 { get; set; }
		public int Contour1ControlPoint2 { get; set; }
		public int Contour1ControlPoint3 { get; set; }
		public int Contour2ControlPoint1 { get; set; }
		public int Contour2ControlPoint2 { get; set; }
		public int Contour2ControlPoint3 { get; set; }

		public Vector<double> RHat { get; set; }
		public Vector<double> RawRatios { get; set; }

		public Vector<double> DBRHat { get; set; }
		public Vector<double> DBRawRatios { get; set; }

		public MatchError()
		{
			Error = 10000.0;
			Contour1 = null;
			Contour2 = null;
			Contour1ControlPoint1 = 0;
			Contour1ControlPoint2 = 0;
			Contour1ControlPoint3 = 0;
			Contour2ControlPoint1 = 0;
			Contour2ControlPoint2 = 0;
			Contour2ControlPoint3 = 0;

			RHat = null;
			RawRatios = null;

			DBRHat = null;
			DBRawRatios = null;
		}
	};
}
