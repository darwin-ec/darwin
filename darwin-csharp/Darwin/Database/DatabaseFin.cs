using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;

namespace Darwin.Database
{
    public class DatabaseFin
    {
		public Bitmap mFinImage;
		public string mImageFilename; //  001DB

		public Outline mFinOutline; //  008OL

		public string mIDCode;
		public string mName;
		public string mDateOfSighting;
		public string mRollAndFrame;
		public string mLocationCode;
		public string mDamageCategory;
		public string mShortDescription;
		public long mDataPos;     //  001DB
		public char[,] mThumbnailPixmap;
		public int mThumbnailRows;

		//  1.4 - new members for tracking image modifications during tracing
		public bool mLeft, mFlipped;              // left side or flipped internally to swim left
		public double mXmin, mYmin, mXmax, mYmax; // internal cropping bounds
		public double mScale;                     // image to Outline scale change
		public Bitmap mModifiedFinImage;      // modified fin image from TraceWin, ...

		public List<ImageMod> mImageMods;    //  1.8 - for list of image modifications
		public string mOriginalImageFilename; //  1.8 - filename of original unmodified image

		public string mFinFilename;   //  1.6 - for name of fin file if fin saved outside DB

		public bool mIsAlternate; //  1.95 - allow designation of primary and alternate fins/images
	}
}
