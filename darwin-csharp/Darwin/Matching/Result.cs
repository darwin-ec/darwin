using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Matching
{
    public class Result
    {
        //  005CM - contours used in final match
        public FloatContour unknownContour;
        public FloatContour dbContour;

        public char[,] mThumbnailPixmap;
        public int mThumbnailRows;

        public string mFilename;    //  001DB - image file for database fin
        public int mPosition;            // position (index) of database fin in database file

        public string
            mError,
            mIdCode,
            mName,
            mDamage,
            mDate,
            mLocation,
            mRank; //  1.5

        //  1.1 - new members to track three point correspondences for final mapping in match

        public int
            mUnkShiftedLEBegin,
            mUnkShiftedTip,
            mUnkShiftedTEEnd,
            mDBShiftedLEBegin,
            mDBShiftedTip,
            mDBShiftedTEEnd;

        public Result(
            FloatContour unknown, //  005CM
            FloatContour db, //  005CM
            string filename,   //  001DB
            char[,] thumbnailPixmap, //  1.0
            int thumbnailRows, //  1.0
            int position,
            string error,
            string idcode,
            string name,
            string damage,
            string date,
            string location
        )
        {
            unknownContour = new FloatContour(unknown); //  1.3 - Mem Leak - make copies now
            dbContour = new FloatContour(db);           //  1.3 - Mem Leak - make copies now

            mFilename = filename; //  001DB
            mPosition = position;
            mError = error;
            mIdCode = idcode;
            mName = name;
            mDamage = damage;
            mLocation = location;
            mRank = string.Empty; //  1.5
            mUnkShiftedLEBegin = 0; //  1.1 - following indices set to defaults by constructor
            mUnkShiftedTip = 0;
            mUnkShiftedTEEnd = 0;
            mDBShiftedLEBegin = 0;
            mDBShiftedTip = 0;
            mDBShiftedTEEnd = 0;

            // MAJOR change JHS
            // removed reloading of image and recreation of thumbnail here
            /*
						ColorImage *temp = new ColorImage(filename); //  001DB
									convColorToPixmapString(
									temp,   //  001DB
									MATCHRESULTS_THUMB_HEIGHT,
									MATCHRESULTS_THUMB_WIDTH,
									mThumbnailPixmap,
									mThumbnailRows);
						delete temp;  //  001DB
			*/

            //  1.0
            if (null == thumbnailPixmap)
            {
                mThumbnailPixmap = null;
                mThumbnailRows = 0;
            }
            else
            {
                // in the future we will revise this call and create a smaller version
                // rather than copying or creating a pixelized larger version
                // 
                // makeDoubleSizePixmapString(thumbnailPixmap, mThumbnailPixmap, mThumbnailRows);

                // simply copy the thumbnail for now and use 25 x 25 thumnails everywhere

                //TODO
                //mThumbnailRows = thumbnailRows;
                //mThumbnailPixmap = new char*[mThumbnailRows];

                //for (int i = 0; i < mThumbnailRows; i++)
                //{
                //	mThumbnailPixmap[i] = new char[strlen(thumbnailPixmap[i]) + 1];
                //	strcpy(mThumbnailPixmap[i], thumbnailPixmap[i]);
                //}
            }
        }

        /* 1.1 - this form of constructor is never used - JHS
				Result(
					int position,
					string error,
					string idcode,
					string name,
					string damage,
					string date,
					string location	
				) :
					mThumbnailPixmap(NULL),
					mThumbnailRows(0),
					mPosition(position),
					mError(error),
					mIdCode(idcode),
					mName(name),
					mDamage(damage),
					mLocation(location)
				{ }
		*/

        public Result(Result r)
        {
            mFilename = r.mFilename;   //  001DB
            mPosition = r.mPosition;
            mError = r.mError;
            mIdCode = r.mIdCode;
            mName = r.mName;
            mDamage = r.mDamage;
            mLocation = r.mLocation;
            mRank = r.mRank; //  1.5
            unknownContour = new FloatContour(r.unknownContour); //  1.3 - Mem Leak - make copies now
            dbContour = new FloatContour(r.dbContour);           //  1.3 - Mem Leak - make copies now
            mUnkShiftedLEBegin = r.mUnkShiftedLEBegin;
            mUnkShiftedTip = r.mUnkShiftedTip;
            mUnkShiftedTEEnd = r.mUnkShiftedTEEnd;
            mDBShiftedLEBegin = r.mDBShiftedLEBegin;
            mDBShiftedTip = r.mDBShiftedTip;
            mDBShiftedTEEnd = r.mDBShiftedTEEnd;

            // TODO
            //if (NULL == r.mThumbnailPixmap)
            //{
            //	mThumbnailPixmap = NULL;
            //	mThumbnailRows = 0;
            //}
            //else
            //{
            //	mThumbnailRows = r.mThumbnailRows;
            //	mThumbnailPixmap = new char*[mThumbnailRows];

            //	for (int i = 0; i < mThumbnailRows; i++)
            //	{
            //		mThumbnailPixmap[i] = new char[strlen(r.mThumbnailPixmap[i]) + 1];
            //		strcpy(mThumbnailPixmap[i], r.mThumbnailPixmap[i]);
            //	}
            //}
        }

        //		Result& operator=(const Result &r)
        //		{
        //			if (this == &r)
        //				return *this;

        //		mPosition = r.mPosition;
        //		mError = r.mError;
        //		mIdCode = r.mIdCode;
        //		mName = r.mName;
        //		mDamage = r.mDamage;
        //		mLocation = r.mLocation;
        //		mRank = r.mRank; //  1.5
        //		unknownContour = r.unknownContour; //  005CM
        //		dbContour = r.dbContour; //  005CM

        //			if (NULL == r.mThumbnailPixmap) {
        //				mThumbnailPixmap = NULL;
        //				mThumbnailRows = 0;
        //			} else {
        //				mThumbnailRows = r.mThumbnailRows;
        //				mThumbnailPixmap = new char*[mThumbnailRows];

        //				for (int i = 0; i<mThumbnailRows; i++) {
        //					mThumbnailPixmap[i] = new char[strlen(r.mThumbnailPixmap[i]) + 1];
        //					strcpy(mThumbnailPixmap[i], r.mThumbnailPixmap[i]);
        //}
        //			}		

        //			return * this;
        //		}


        //  1.1 - sets six indices for points used in final contour mapping
        public void SetMappingControlPoints(
                int unkLEBegin, int unkTip, int unkTEEnd,
                int dbLEBegin, int dbTip, int dbTEEnd)
        {
            mUnkShiftedLEBegin = unkLEBegin;
            mUnkShiftedTip = unkTip;
            mUnkShiftedTEEnd = unkTEEnd;
            mDBShiftedLEBegin = dbLEBegin;
            mDBShiftedTip = dbTip;
            mDBShiftedTEEnd = dbTEEnd;
        }

        //  1.1 - gets six indices for points used in final contour mapping
        public void GetMappingControlPoints(
                out int unkLEBegin, out int unkTip, out int unkTEEnd,
                out int dbLEBegin, out int dbTip, out int dbTEEnd)
        {
            unkLEBegin = mUnkShiftedLEBegin;
            unkTip = mUnkShiftedTip;
            unkTEEnd = mUnkShiftedTEEnd;
            dbLEBegin = mDBShiftedLEBegin;
            dbTip = mDBShiftedTip;
            dbTEEnd = mDBShiftedTEEnd;
        }
    }
}
