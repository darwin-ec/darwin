//*******************************************************************
//   file: Chain.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (7/22/2005)
//         -- reformatting of code and addition of comment blocks
//         -- changes to incorporate Outline CLASS in project
//
//*******************************************************************

using Darwin.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Darwin
{
    public class Chain
    {
        private const double CloseToZero = 1e-10;
		private int _numPoints;
		// the array of doubles containing
		//    the absolute chain angles
		private double[] _chain;
		// the array of doubles
		//    containing the relative chain angles
		private double[] _relativeChain;

		public int Length
		{
			get
			{
				return _numPoints;
			}
		}

		public double this[int i]
		{
			// Routing through "Points" rather than "_points" so we have the null check
			get
			{
				return Data[i];
			}
			set
			{
				Data[i] = value;
			}
		}

		// the array of doubles containing
		//    the absolute chain angles
		public double[] Data
		{
			get
			{
				return _chain;
			}
			set
			{
				_chain = value;
			}
		}

		// the array of doubles
		//    containing the relative chain angles
		public double[] RelativeData
		{
			get
			{
				return _relativeChain;
			}
			set
			{
				_relativeChain = value;
			}
		}

		//*******************************************************************
		//
		// Chain::Chain(FloatContour *fc)  //***008OL entire function added
		//
		//    Used when FloatContour is read from DB file.
		//    
		//    Open Question: Should updateRelativeChain() be used here?
		//
		//    PRE:  fc is normalized and evenly spaced
		//    POST: Chain indices correspond to fc indices
		// 
		public Chain(FloatContour fc)
		{
			if (fc == null)
				throw new ArgumentNullException(nameof(fc));

			_numPoints = fc.Length;

			List<double> chain = new List<double>();
			List<double> relativeChain = new List<double>();

			// NOTE: the Chain angle at each pooint is ALWAYS the angle of 
			// the INCOMING edge FROM the previous contour point - JHS

			if (_numPoints > 1)
			{
				// initial angle of chain is angle of first point relative to (0,0)
				//chain.push_back(rtod(atan2((*fc)[0].y, (*fc)[0].x))); //***2.1mt
				//***2.01 - now just duplicate the first relative angle
				chain.Add(MathHelper.RadiansToDegrees(Math.Atan2(fc[1].Y - fc[0].Y, fc[1].X - fc[0].X))); //***2.01 

				chain.Add(MathHelper.RadiansToDegrees(Math.Atan2(fc[1].Y - fc[0].Y, fc[1].X - fc[0].X)));

				double lastAngle = chain[1]; //***2.1mt - use this to prevent any quadrant discontinuity 

				// initial angle in relativeChain is angle change across point[0]
				//***1.2 - I think initial angle is meaningless and the
				// initial meaningful angle is actually angle change across point[1]
				relativeChain.Add(0.0); //***1.2 - added this

				// without line above the relative chain length & indexing are off by one
				relativeChain.Add(chain[0] - chain[1]);

				for (int i = 2; i < _numPoints; i++)
				{
					double angle = MathHelper.RadiansToDegrees(Math.Atan2(fc[i].Y - fc[i - 1].Y, fc[i].X - fc[i - 1].X)); //***2.01

					//***2.01 - prevent ANY discontinuity of greater than 180 degrees
					if (angle - lastAngle > 180.0)
						chain.Add(angle - 360.0);
					else if (angle - lastAngle < -180.0)
						chain.Add(angle + 360.0);
					else
						chain.Add(angle);

					lastAngle = chain[i];

					//***2.01 - end

					//////////////////////////// begin old ///////////////////////
					relativeChain.Add(chain[i - 1] - chain[i]);
					//////////////////////// end old ///////////////////////////
				}
			}

			// NOTE: there is NO useful change in angle across either
			// the first point or the last point in fc

			// now copy vectors to double arrays

			_chain = chain.ToArray();
			_relativeChain = relativeChain.ToArray();
		}


		//*******************************************************************
		//
		// Chain::Chain(const Chain &c)
		//
		//    COPY Constructor (makes copy of "referenced" original).
		//
		public Chain(Chain c)
{
	_numPoints = c._numPoints;
	//mRadius = c.mRadius; removed 008OL
	_chain = new double[_numPoints];
	_relativeChain = new double[_numPoints];

			Array.Copy(c._chain, _chain, _numPoints);
			Array.Copy(c._relativeChain, _relativeChain, _numPoints);
	}

//*******************************************************************
//
// Chain& Chain::operator=(const Chain &c)
//
//    Overloaded ASSIGNMENT operator
//
//Chain& Chain::operator=(const Chain &c)
//{
//	// check if this is a self-assignment
//	if (this == &c)
//		return *this;

//	_numPoints = c._numPoints;
//	// mRadius = c.mRadius; removed 008OL
//	_chain = new double[_numPoints];
//	mRelativeChain = new double[_numPoints];
//	//mSavedPoints = new point_t[_numPoints]; removed 008OL
//	//_chainPoints = new FloatContour((const FloatContour &)c._chainPoints);  //**006DF,008OL removed JHS

//	memcpy(_chain, c._chain, _numPoints * sizeof(double));
//	memcpy(mRelativeChain, c.mRelativeChain, _numPoints * sizeof(double));
//	//memcpy(mSavedPoints, c.mSavedPoints, _numPoints * sizeof(point_t)); removed 008OL

//	return *this;
//}


//*******************************************************************
//
// double& Chain::getRelativeAngle(int angleNum)
//
//    Returns PUBLIC reference to specified relative angle.
//
//    PRE:  0 < angleNum < _numPoints
//
public double GetRelativeAngle(int angleNum)
{
			if (angleNum < 0 || angleNum >= _numPoints)
				throw new ArgumentOutOfRangeException(nameof(angleNum));

	return _relativeChain[angleNum];
}

//*******************************************************************
//
// double& Chain::getAngle(int angleNum)
//
//    Returns PUBLIC reference to specified absolute angle.
//
//    PRE:  0 < angleNum < _numPoints
//
public double GetAngle(int angleNum)
{
			if (angleNum < 0 || angleNum >= _numPoints)
				throw new ArgumentOutOfRangeException(nameof(angleNum));

			return _chain[angleNum];
}


//*******************************************************************
//
// double Chain::min() const
//
//    Returns MINIMUM chain angle value.  Performs linear search to
//    determine value.
//
public double Min()
{
	if (_numPoints == 0 || _chain == null)
		return 0.0;

			double minVal = _chain[0];
			for (int i = 1; i < _numPoints; i++)
			{
				if (_chain[i] < minVal)
					minVal = _chain[i];
			}

			return minVal;
		}

//*******************************************************************
//
// double Chain::max() const
//
//    Returns MAXIMUM chain angle value.  Performs linear search to
//    determine value.
//
public double Max()
{
			if (_numPoints == 0 || _chain == null)
				return 0.0;

			double maxVal = _chain[0];
			for (int i = 1; i < _numPoints; i++)
			{
				if (_chain[i] > maxVal)
					maxVal = _chain[i];
			}

			return maxVal;
		}

//*******************************************************************
//
// double Chain::min(int start, int end) const
//
//    Returns MINIMUM chain angle value within specified portion of
//    chain.  Performs linear search within range [start, end) to
//    determine value.
// 
public double Min(int start, int end)
{
  // NOTE: returns minimum value in range [start..end)

	if (_numPoints == 0 || _chain == null)
		return 0.0;

	if (start< 0 || start >= _numPoints || start> end)
		throw new ArgumentOutOfRangeException(nameof(start));

	if (end< 0 || end> _numPoints)
		throw new ArgumentOutOfRangeException(nameof(end));

double minVal = _chain[0];
	for (int i = start; i<end; i++) {
		if (_chain[i] < minVal)
			minVal = _chain[i];
	}

	return minVal;
}

//*******************************************************************
//
// double Chain::max(int start, int end) const
//
//    Returns MAXIMUM chain angle value within specified portion of
//    chain.  Performs linear search within range [start, end) to
//    determine value.
// 
public double Max(int start, int end)
{
			// NOTE: returns maximum value in range [start..end)

			if (_numPoints == 0 || _chain == null)
				return 0.0;

			if (start < 0 || start >= _numPoints || start > end)
				throw new ArgumentOutOfRangeException(nameof(start));

			if (end < 0 || end > _numPoints)
				throw new ArgumentOutOfRangeException(nameof(end));

			double maxVal = _chain[0];
	for (int i = start; i<end; i++) {
		if (_chain[i] > maxVal)
			maxVal = _chain[i];
	}

	return maxVal;
}


//*******************************************************************
//
// double Chain::minRelative() const
//
//    Returns MINIMUM relative chain angle value.  Performs linear search to
//    determine value.
//
public double MinRelative()
{
  //***008OL NOTE: Since ther is NO useful angle at first or
  // last point along contour, this returns the minimum
  // relative angle in range [1.._numPoints-1)

	if (_numPoints == 0 || _relativeChain == null)
		return 0.0;

	//***008OL changed range of search below
  double minVal = _relativeChain[1];
	for (int i = 2; i<_numPoints-1; i++) {
		if (_relativeChain[i] < minVal)
			minVal = _relativeChain[i];
	}

	return minVal;
}


//*******************************************************************
//
// double Chain::maxRelative() const
//
//    Returns MAXIMUM relative chain angle value.  Performs linear search to
//    determine value.
//
public double MaxRelative()
{
			//***008OL NOTE: Since ther is NO useful angle at first or
			// last point along contour, this returns the minimum
			// relative angle in range [1.._numPoints-1)

			if (_numPoints == 0 || _relativeChain == null)
				return 0.0;

	//***008OL changed range of search below
	double maxVal = _relativeChain[1];
	for (int i = 2; i<_numPoints-1; i++) {
		if (_relativeChain[i] >maxVal)
			maxVal = _relativeChain[i];
	}

	return maxVal;
}


//*******************************************************************
//
// void Chain::smooth7()
//
//    SMOOTHES _chain using an interval of 7.  A temporary chain copy
//    is created and is padded on leading end [0,3] with _chain[1] (the first
//    meaningful angle) and on the traling end [_numPoints+3,_numPoints+5] 
//    with _chain[_numPoints-1] (the last meaningful angle.  Each angle in
//    the NEW _chain is the average (mean) of 7 values centered on the
//    same location as the value it replaced.
//
public void Smooth7()
{
	if (_numPoints == 0 || _chain == null || _relativeChain == null)
		return;

	int
	chI,
		sI;

	double[] tempChain;

	tempChain = new double[_numPoints + 6];

			Array.Copy(_chain, 0, tempChain, 3, _numPoints);

			tempChain[0] = _chain[1]; //005CH
	tempChain[1] = _chain[1]; //005CH
	tempChain[2] = _chain[1]; //005CH
	tempChain[3] = _chain[1]; //005CH
	tempChain[_numPoints + 3] = _chain[_numPoints - 1];
	tempChain[_numPoints + 4] = _chain[_numPoints - 1];
	tempChain[_numPoints + 5] = _chain[_numPoints - 1];

	chI = 3; //005CH

	double[] smoothChain = new double[_numPoints];

	while (chI < _numPoints + 3)
	{ //005CH
		sI = chI - 3;
		smoothChain[sI] = (tempChain[chI - 3]
					 + tempChain[chI - 2]
					 + tempChain[chI - 1]
					 + tempChain[chI]
					 + tempChain[chI + 1]
					 + tempChain[chI + 2]
					 + tempChain[chI + 3])
					 / 7.0;
		chI++;
	}

	_chain = smoothChain;

	UpdateRelativeChain();
}


//*******************************************************************
//
// void Chain::smooth5()
//
//    SMOOTHES _chain using an interval of 5.  A temporary chain copy
//    is created and is padded on leading end [0,2] with _chain[1] (the first
//    meaningful angle) and on the traling end [_numPoints+2,_numPoints+3] 
//    with _chain[_numPoints-1] (the last meaningful angle.  Each angle in
//    the NEW _chain is the average (mean) of 5 values centered on the
//    same location as the value it replaced.
//
public void Smooth5()
{
			if (_numPoints == 0 || _chain == null || _relativeChain == null)
				return;

	int
	chI,
		sI;

	double[] tempChain = new double[_numPoints + 4];

			Array.Copy(_chain, 0, tempChain, 2, _numPoints);

	tempChain[0] = _chain[1]; //005CH
	tempChain[1] = _chain[1]; //005CH
	tempChain[2] = _chain[1]; //005CH
	tempChain[_numPoints + 2] = _chain[_numPoints - 1];
	tempChain[_numPoints + 3] = _chain[_numPoints - 1];
	chI = 2;

	double[] smoothChain = new double[_numPoints];

	while (chI < _numPoints + 2)
	{
		sI = chI - 2;
		smoothChain[sI] =
			 (tempChain[chI - 2]
			+ tempChain[chI - 1]
			+ tempChain[chI]
			+ tempChain[chI + 1]
			+ tempChain[chI + 2])
			/ 5.0;
		chI++;
	}

	_chain = smoothChain;

	UpdateRelativeChain();
}


//*******************************************************************
//
// void Chain::smooth3()
//
//    SMOOTHES _chain using an interval of 3.  A temporary chain copy
//    is created and is padded on leading end [0,1] with _chain[1] (the first
//    meaningful angle) and on the traling end [_numPoints+1] 
//    with _chain[_numPoints-1] (the last meaningful angle.  Each angle in
//    the NEW _chain is the average (mean) of 3 values centered on the
//    same location as the value it replaced.
//
public void Smooth3()
{
			if (_numPoints == 0 || _chain == null || _relativeChain == null)
				return;

	int chI,
	 sI;

	double[] tempChain;

	tempChain = new double[_numPoints + 2];

			Array.Copy(_chain, 0, tempChain, 1, _numPoints);

			tempChain[0] = _chain[1]; //005CH
	tempChain[1] = _chain[1]; //005CH
	tempChain[_numPoints + 1] = _chain[_numPoints - 1];
	chI = 1;

	double[] smoothChain = new double[_numPoints];

	while (chI < _numPoints + 1)
	{
		sI = chI - 1;
		smoothChain[sI] = tempChain[chI - 1]
			+ tempChain[chI] + tempChain[chI + 1];
		smoothChain[sI] /= 3.0;
		chI++;
	}

	_chain = smoothChain;

	UpdateRelativeChain();
}


//*******************************************************************
//
// void Chain::smooth15()
//
//    SMOOTHES _chain using an interval of 15.  A temporary chain copy
//    is created and is padded on leading end [0,7] with _chain[1] (the first
//    meaningful angle) and on the traling end [_numPoints+7,_numPoints+13] 
//    with _chain[_numPoints-1] (the last meaningful angle.  Each angle in
//    the NEW _chain is the average (mean) of 15 values centered on the
//    same location as the value it replaced.
//
public void Smooth15()
{
			if (_numPoints == 0 || _chain == null || _relativeChain == null)
				return;

	int
	chI,
		sI;

	double[] tempChain = new double[_numPoints + 14];

			Array.Copy(_chain, 0, tempChain, 7, _numPoints);
	int i;
	for (i = 0; i < 8; i++)     //005CH
		tempChain[i] = _chain[1]; //005CH

	for (i = _numPoints + 7; i < _numPoints + 14; i++)
		tempChain[i] = _chain[_numPoints - 1];

	chI = 7;

	double[] smoothChain = new double[_numPoints];

	while (chI < _numPoints + 7)
	{ //005CH
		sI = chI - 7;
		smoothChain[sI] = (tempChain[chI - 7]
					 + tempChain[chI - 6]
					 + tempChain[chI - 5]
					 + tempChain[chI - 4]
					 + tempChain[chI - 3]
					 + tempChain[chI - 2]
					 + tempChain[chI - 1]
					 + tempChain[chI]
					 + tempChain[chI + 1]
					 + tempChain[chI + 2]
					 + tempChain[chI + 3]
					 + tempChain[chI + 4]
					 + tempChain[chI + 5]
					 + tempChain[chI + 6]
					 + tempChain[chI + 7])
			/ 15.0;
		chI++;
	}
	
	_chain = smoothChain;
	UpdateRelativeChain();
}


//*******************************************************************
//
// void Chain::print() const
//
//    Prints a list of chain values in the CONSOLE.
//
public void Print()
{
	for (int i = 0; i< _numPoints; i++)
		Trace.WriteLine(_chain[i]);
}

//*******************************************************************
//
// void Chain::updateRelativeChain()
//
//    Creates (or changes) the mRelativeChain angles to be consistent with
//    the absolute angles in _chain.
//
//    PRE:  _chain and mRelativeChain arrays exists and the memory allocated 
//          for both is the same  
//
private void UpdateRelativeChain()
{
			if (_numPoints == 0 || _chain == null || _relativeChain == null)
				return;

	_relativeChain[0] = _chain[0];

	double temp;

	for (int i = 1; i < _numPoints; i++)
	{

		/*
		////////////////// begin new ///////////////////////
		//***1.1ER - new update for relative chain
		// this new code more efficiently converts delta angles outside the
		// range -180 .. +180 back into that range
		//
		//double change = _chain[i-1] - _chain[i]; // this is consistent with constructor
		// it seems that the subtraction below is correct rather than the one 
		// above -- BUT this is reversed from what is done in the
		// constructor -- shouldn't they be consistent?? -- JHS
		double change = _chain[i] - _chain[i-1]; // BUT this seems to work correctly
		if (change > 180)
			change = change - 360;
		else if (change < -180)
			change = 360 - change;
		mRelativeChain[i] = change;
		///////////////// end new ////////////////////////
		*/
		//////////////////// begin old ////////////////////
		//***1.1ER
		// this is the original code - it needs to be replaced with what is above
		// in the next version - all part of fixing the problem that pulls
		// snake out of deep upturning notches and then finds the lip of the
		// notch as the hole
		temp = MathHelper.DegreesToRadians(_chain[i] - _chain[i - 1]);

		//***008OL NOTE: the following loop corrects angles of greater than
		// 360 degrees, BUT how is it possible for the relative angle to
		// ever be more than 360 degrees?  Would this not automatically
		// indicate an error in computation somewhere?

		while (Math.Abs(temp / Math.PI) > 1)
			temp -= (Math.Abs(temp) / temp) * Math.PI;

		temp = MathHelper.RadiansToDegrees(temp);

		if (temp < CloseToZero && temp > -CloseToZero)
			_relativeChain[i] = 0.0;
		else
			_relativeChain[i] = temp;
		/////////////////////////// end old ///////////////
	}
}
    }
}
