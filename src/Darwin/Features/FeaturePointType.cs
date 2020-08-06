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
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin.Features
{
    public enum FeaturePointType
    {
        NoFeature = -1,
        LeadingEdgeBegin = 1,
        LeadingEdgeEnd = 2,
        Tip = 3,
        Notch = 4,
        PointOfInflection = 5,

        Nasion = 6,
        Chin = 7,
        UpperLip = 8,
        Brow = 9,
        BottomLipProtrusion = 10,

        Eye = 100,
        NasalLateralCommissure = 101
    }
}
