//                                            *
//   file: DatabaseFin.h
//
// author: Adam Russell
//
//   mods: J H Stewman (9/27/2005)
//         -- code to determine whether particular databasefin
//            is being used for an UNKNOWN or is being created as it
//            is read from the database
//            ASSUMPTION: if image filename contains any slashes it is
//            presumed to be an UNKNOWN
//
//                                            *

//
// 0.3.1-DB: Addendum : New DatabaseFin Data structure
// [Data Position] (4 Bytes)
// [Image Filename] (char[255]) **Delimited by '\n'
// [Number of Contour Points] (4 bytes)
// [Contour Points ...] (Number * (int) bytes)
// [Thumbnail Pixmap] (25*25)
// [Short Description] (char[255]) **Delimited by '\n'
//
// Darwin_0.3.8 - DB version 2: Addendum : New DatabaseFin Data structure
// [Data Position] (4 Bytes) -- or "DFIN" as hex number in saved traced fin files
// [Image Filename] (char[255]) **Delimited by '\n'
// [Number of FloatContour Points] (4 bytes)
// [FloatContour Points ...] (Number*2*sizeof(float) bytes)
// [Feature Point Positions] (5*sizeof(int) bytes)
// [Thumbnail Pixmap] (25*25 bytes)
// [Short Description] (char[255]) **Delimited by '\n'
//
// Darwin_1.4 - DB version 4: Addendum : New DatabaseFin Data structure
// this adds fields for tracking changes to image while tracing fin
// [Data Position] (4 Bytes) -- or "DFIN" as hex number in saved traced fin files
// [Image Filename] (char[255]) **Delimited by '\n'
// [Number of FloatContour Points] (4 bytes)
// [FloatContour Points ...] (Number*2*sizeof(float) bytes)
// [Feature Point Positions] (5*sizeof(int) bytes)
// [Thumbnail Pixmap] (25*25 bytes)
// [Is Left Side] '1' or '0'
// [Is Flipped Image] '1' or '0'
// [Clipping bounds xmin,ymin,xmax,ymax] (4 * sizeof(double))
// [Normalizing Scale] (sizeof(double)
// [Alternate (blind) ID] (5 chars) **Delimited by '\n'
// [Short Description] (char[255]) **Delimited by '\n'
//

using System.Collections.Generic;
using System.Drawing;

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


        //                                                **
        //
        // called from numerous places in ...
        //   TraceWindow.cxx, ModifyDatabaseWindow.cxx, and 
        //   NoMatchWindow.cxx
        //
        public DatabaseFin(
            string filename, //  001DB
            Outline outline, //  008OL
            string idcode,
            string name,
            string dateOfSighting,
            string rollAndFrame,
            string locationCode,
            string damageCategory,
            string shortDescription
        )
        {
            mImageFilename = filename; //  001DB
            mFinOutline = new Outline(outline); //  006DF,008OL
            mIDCode = idcode;
            mName = name;
            mDateOfSighting = dateOfSighting;
            mRollAndFrame = rollAndFrame;
            mLocationCode = locationCode;
            mDamageCategory = damageCategory;
            mShortDescription = shortDescription;
            mThumbnailPixmap = null;
            mThumbnailRows = 0;
            mLeft = true; //  1.4
            mFlipped = false; //  1.4
            mXmin = 0.0; //  1.4
            mXmax = 0.0; //  1.4
            mYmin = 0.0; //  1.4
            mYmax = 0.0; //  1.4
            mScale = 1.0; //  1.4
            mModifiedFinImage = null; //  1.5
            mFinFilename = string.Empty; //  1.6
            mIsAlternate = false; //  1.95

            //  1.5 - need some way to CATCH error thrown when image file
            //         does not exist or is unsupported type  --
            //         program now crashes when database image is misplaced or misnamed

            mFinImage = new Bitmap(mImageFilename); //  001DB

            // TODO
            //FIN_IMAGE_TYPE* thumb = resizeWithBorderNN(
            //		mFinImage,
            //		DATABASEFIN_THUMB_HEIGHT,
            //		DATABASEFIN_THUMB_WIDTH);

            // TODO
            //convToPixmapString(thumb, mThumbnailPixmap, mThumbnailRows);
        }

        //  1.99 - new constructor used by SQLlite database code
        //                                                **
        //
        // Added. Called in Database::getFin().
        //
        public DatabaseFin(
            string filename, //  001DB
            Outline outline, //  008OL
			string idcode,
			string name,
			string dateOfSighting,
			string rollAndFrame,
			string locationCode,
			string damageCategory,
			string shortDescription,
			long datapos,

            char[,] pixmap,

            int rows
		)
        {
            mImageFilename = filename; //  001DB
			mFinOutline =new Outline(outline); //  006DF,008OL
            mIDCode = idcode;
            mName = name;
            mDateOfSighting = dateOfSighting;
            mRollAndFrame = rollAndFrame;
            mLocationCode = locationCode;
            mDamageCategory = damageCategory;
            mShortDescription = shortDescription;
            mThumbnailPixmap = pixmap;
            mThumbnailRows = rows;
            mDataPos = datapos;
            mLeft = true; //  1.4
            mFlipped = false; //  1.4
            mXmin = 0.0; //  1.4
            mXmax = 0.0; //  1.4
            mYmin= 0.0; //  1.4
            mYmax = 0.0; //  1.4
            mScale = 1.0; //  1.4
            mModifiedFinImage = null; //  1.5
            mFinFilename = string.Empty; //  1.6
            mFinImage = null;
            mIsAlternate = false; //  1.99
                                // let's see what happens... -- rjn
            /*
            mFinImage=new FIN_IMAGE_TYPE(mImageFilename); //  001DB
            FIN_IMAGE_TYPE *thumb = resizeWithBorderNN(
                    mFinImage,
                    DATABASEFIN_THUMB_HEIGHT,
                    DATABASEFIN_THUMB_WIDTH);
            convToPixmapString(thumb, mThumbnailPixmap, mThumbnailRows);
            delete thumb;
            */
        }

        //                                                **
        //
        // called ONLY from Match.cxx and MatchResultsWindow.cxx
        //
        public DatabaseFin(DatabaseFin fin)
        {
            mImageFilename = fin.mImageFilename;        //  001DB
			mFinImage = null;                          //   major change JHS
            mModifiedFinImage = null; //  1.5
            mDataPos = fin.mDataPos;                    //  001DB
            mFinOutline = new Outline(fin.mFinOutline); //  006DF,008OL
            mIDCode = fin.mIDCode;
            mName = fin.mName;
            mDateOfSighting = fin.mDateOfSighting;
            mRollAndFrame = fin.mRollAndFrame;
            mLocationCode = fin.mLocationCode;
            mDamageCategory = fin.mDamageCategory;
            mShortDescription = fin.mShortDescription;
            // TODO
            mThumbnailPixmap = null; //new char*[fin.mThumbnailRows];
            mThumbnailRows = fin.mThumbnailRows;
            mLeft = fin.mLeft; //  1.4
            mFlipped = fin.mFlipped; //  1.4
            mXmin = fin.mXmin; //  1.4
            mXmax = fin.mXmax; //  1.4
            mYmin = fin.mYmin; //  1.4
            mYmax = fin.mYmax; //  1.4
            mScale = fin.mScale; //  1.4
            mFinFilename = fin.mFinFilename; //  1.6
            mOriginalImageFilename = fin.mOriginalImageFilename; //  1.8

            mImageMods = fin.mImageMods; //  1.8

            mIsAlternate = fin.mIsAlternate; //  1.95

            //  1.5 - just set pointer to original copy from TraceWindow
            //  1.8 - we actually create a COPY of the modified image here
            if (null != fin.mModifiedFinImage)
                mModifiedFinImage = new Bitmap(fin.mModifiedFinImage);

            //  1.8 - and we create a COPY of the original image here
            if (null != fin.mFinImage)
                mFinImage = new Bitmap(fin.mFinImage);

            // TODO
            //for (int i = 0; i < fin.mThumbnailRows; i++)
            //{
            //    mThumbnailPixmap[i] = new char[strlen(fin->mThumbnailPixmap[i]) + 1];
            //    strcpy(mThumbnailPixmap[i], fin->mThumbnailPixmap[i]);
            //}
        }
    }
}
