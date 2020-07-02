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
		}
	};
}
