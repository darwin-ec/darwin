using Darwin.Database;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Matching
{
    public enum RegistrationMethodType
    {
        Original3Point = 10,
        TrimFixedPercent = 20,
        TrimOptimal = 30,
        TrimOptimalTotal = 40,
        TrimOptimalTip = 41,
        TrimOptimalInOut = 42,
        TrimOptimalInOutTip = 43,
        TrimOptimalArea = 45,
        LeadingEdgeAngleMethod = 50,
        SigShift = 60
    }

    public enum RangeOfPointsType
    {
        AllPoints = 100,
        LeadToTipOnly = 200,
        LeadToNotchOnly = 300,
        LeadThenTrail = 400,
        TrailingEdgeOnly = 1
    }
}
