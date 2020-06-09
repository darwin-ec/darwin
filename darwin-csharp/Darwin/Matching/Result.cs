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

        public string ThumbnailFilenameUri { get; set; }
        public int mThumbnailRows;

        public string ImageFilename { get; set; }    //  001DB - image file for database fin
        public int mPosition { get; set; }            // position (index) of database fin in database file

        public double Error { get; set; }
        public string IDCode { get; set; }
        public string Name { get; set; }
        public string Damage { get; set; }
        public string Date { get; set; }
        public string Location { get; set; }
        public int Rank { get; set; } //  1.5

        //  1.1 - new members to track three point correspondences for final mapping in match

        public int UnkShiftedLEBegin { get; set; }
        public int UnkShiftedTip { get; set; }
        public int UnkShiftedTEEnd { get; set; }
        public int DBShiftedLEBegin { get; set; }
        public int DBShiftedTip { get; set; }
        public int DBShiftedTEEnd { get; set; }

        public Result(
            FloatContour unknown, //  005CM
            FloatContour db, //  005CM
            string filename,   //  001DB
            string thumbnailFilenameUri, //  1.0
            int position,
            double error,
            string idcode,
            string name,
            string damage,
            string date,
            string location
        )
        {
            unknownContour = new FloatContour(unknown); //  1.3 - Mem Leak - make copies now
            dbContour = new FloatContour(db);           //  1.3 - Mem Leak - make copies now

            ImageFilename = filename; //  001DB
            mPosition = position;
            Error = error;
            IDCode = idcode;
            Name = name;
            Damage = damage;
            Date = date;
            Location = location;
            Rank = 0; //  1.5
            UnkShiftedLEBegin = 0; //  1.1 - following indices set to defaults by constructor
            UnkShiftedTip = 0;
            UnkShiftedTEEnd = 0;
            DBShiftedLEBegin = 0;
            DBShiftedTip = 0;
            DBShiftedTEEnd = 0;

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

            ThumbnailFilenameUri = thumbnailFilenameUri;
            //  1.0
            //if (null == thumbnailPixmap)
            //{
            //    ThumbnailFilenameUri = null;
            //    mThumbnailRows = 0;
            //}
            //else
            //{
            //    // in the future we will revise this call and create a smaller version
            //    // rather than copying or creating a pixelized larger version
            //    // 
            //    // makeDoubleSizePixmapString(thumbnailPixmap, mThumbnailPixmap, mThumbnailRows);

            //    // simply copy the thumbnail for now and use 25 x 25 thumnails everywhere

            //    //TODO
            //    //mThumbnailRows = thumbnailRows;
            //    //mThumbnailPixmap = new char*[mThumbnailRows];

            //    //for (int i = 0; i < mThumbnailRows; i++)
            //    //{
            //    //	mThumbnailPixmap[i] = new char[strlen(thumbnailPixmap[i]) + 1];
            //    //	strcpy(mThumbnailPixmap[i], thumbnailPixmap[i]);
            //    //}
            //}
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
            ImageFilename = r.ImageFilename;   //  001DB
            mPosition = r.mPosition;
            Error = r.Error;
            IDCode = r.IDCode;
            Name = r.Name;
            Damage = r.Damage;
            Location = r.Location;
            Rank = r.Rank; //  1.5
            unknownContour = new FloatContour(r.unknownContour); //  1.3 - Mem Leak - make copies now
            dbContour = new FloatContour(r.dbContour);           //  1.3 - Mem Leak - make copies now
            UnkShiftedLEBegin = r.UnkShiftedLEBegin;
            UnkShiftedTip = r.UnkShiftedTip;
            UnkShiftedTEEnd = r.UnkShiftedTEEnd;
            DBShiftedLEBegin = r.DBShiftedLEBegin;
            DBShiftedTip = r.DBShiftedTip;
            DBShiftedTEEnd = r.DBShiftedTEEnd;
            ThumbnailFilenameUri = r.ThumbnailFilenameUri;
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
            UnkShiftedLEBegin = unkLEBegin;
            UnkShiftedTip = unkTip;
            UnkShiftedTEEnd = unkTEEnd;
            DBShiftedLEBegin = dbLEBegin;
            DBShiftedTip = dbTip;
            DBShiftedTEEnd = dbTEEnd;
        }

        //  1.1 - gets six indices for points used in final contour mapping
        public void GetMappingControlPoints(
                out int unkLEBegin, out int unkTip, out int unkTEEnd,
                out int dbLEBegin, out int dbTip, out int dbTEEnd)
        {
            unkLEBegin = UnkShiftedLEBegin;
            unkTip = UnkShiftedTip;
            unkTEEnd = UnkShiftedTEEnd;
            dbLEBegin = DBShiftedLEBegin;
            dbTip = DBShiftedTip;
            dbTEEnd = DBShiftedTEEnd;
        }
    }
}
