using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Matching
{
	public class MseInfo
	{
		// members

		public double Error { get; set; }

		public FloatContour C1 { get; set; }
		public FloatContour C2 { get; set; }

		public int B1 { get; set; }
		public int T1 { get; set; }
		public int E1 { get; set; }
		public int B2 { get; set; }
		public int T2 { get; set; }
		public int E2 { get; set; }

		// constructor and destructor

		public MseInfo()
		{
			Error = 10000.0;
			C1 = null;
			C2 = null;
			B1 = 0;
			T1 = 0;
			E1 = 0;
			B2 = 0;
			T2 = 0;
			E2 = 0;
		}
	};
}
