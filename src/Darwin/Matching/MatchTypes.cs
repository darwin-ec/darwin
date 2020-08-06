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
