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

namespace Darwin.Database
{
	public class ItemInfo
	{
		public string IDCode { get; set; }
		public string Name { get; set; }
		public string DateOfSighting { get; set; }
		public string RollAndFrame { get; set; }
		public string LocationCode { get; set; }
		public string DamageCategory { get; set; }
		public string ShortDescription { get; set; }

		public bool IsAlternate { get; set; } //  1.95

		public ItemInfo(
			string idCode,
			string name,
			string dateOfSighting,
			string rollAndFrame,
			string locationCode,
			string damageCategory,
			string shortDescription
		)
		{
			IDCode = idCode;
			Name = name;
			DateOfSighting = dateOfSighting;
			RollAndFrame = rollAndFrame;
			LocationCode = locationCode;
			DamageCategory = damageCategory;
			ShortDescription = shortDescription;

			//if ("" == mIDCode)
			//	mIDCode = "NONE";
			//if ("" == mName)
			//	mName = "NONE";
			//if ("" == mDateOfSighting)
			//	mDateOfSighting = "NONE";
			//if ("" == mRollAndFrame)
			//	mRollAndFrame = "NONE";
			//if ("" == mLocationCode)
			//	mLocationCode = "NONE";
			//if ("" == mDamageCategory)
			//	mDamageCategory = "NONE";
			//if ("" == mShortDescription)
			//	mShortDescription = "NONE";
		}
	}
}
