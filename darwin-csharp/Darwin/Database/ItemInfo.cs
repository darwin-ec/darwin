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
