//                                            *
//   file: MatchResults.h
//
// author: Adam Russell
//
//   mods: J H Stewman (2006 & 2007)
//
//                                            *

#ifndef MATCHRESULTS_H
#define MATCHRESULTS_H

#include "../image_processing/conversions.h"
#include "../Database.h"
#pragma warning(disable:4786) //  1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include <list>
#include "../FloatContour.h" //  005CM

// original sizes - should be 128x128 and 64x64 when revised later
// const int MATCHRESULTS_THUMB_HEIGHT = 60, MATCHRESULTS_THUMB_WIDTH = 60;

// new size - same as thumnails stored in database
const int MATCHRESULTS_THUMB_HEIGHT = 25, MATCHRESULTS_THUMB_WIDTH = 25;

// This class is a little silly, I admit... for some reason I feel like
// doing it this way right now, though.
class Result {

	public:
	
		Result(
			FloatContour *unknown, //  005CM
			FloatContour *db, //  005CM
			std::string filename,	//  001DB
			char **thumbnailPixmap, //  1.0
			int thumbnailRows, //  1.0
			int position,
			std::string error,
			std::string idcode,
			std::string name,
			std::string damage,
			std::string date,
			std::string location
		) :	
			mFilename(filename), //  001DB
			mPosition(position),
			mError(error),
			mIdCode(idcode),
			mName(name),
			mDamage(damage),
			mLocation(location),
			mRank(""), //  1.5
			mUnkShiftedLEBegin(0), //  1.1 - following indices set to defaults by constructor
			mUnkShiftedTip(0),
			mUnkShiftedTEEnd(0),
			mDBShiftedLEBegin(0), 
			mDBShiftedTip(0), 
			mDBShiftedTEEnd(0)

		{
			unknownContour = new FloatContour(*unknown); //  1.3 - Mem Leak - make copies now
			dbContour = new FloatContour(*db);           //  1.3 - Mem Leak - make copies now

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
			if (NULL == thumbnailPixmap) 
			{
				mThumbnailPixmap = NULL;
				mThumbnailRows = 0;
			} 
			else
			{	
				// in the future we will revise this call and create a smaller version
				// rather than copying or creating a pixelized larger version
				// 
				// makeDoubleSizePixmapString(thumbnailPixmap, mThumbnailPixmap, mThumbnailRows);
				
				// simply copy the thumbnail for now and use 25 x 25 thumnails everywhere
				mThumbnailRows = thumbnailRows;
				mThumbnailPixmap = new char*[mThumbnailRows];

				for (int i = 0; i < mThumbnailRows; i++) {
					mThumbnailPixmap[i] = new char[strlen(thumbnailPixmap[i]) + 1];
					strcpy(mThumbnailPixmap[i], thumbnailPixmap[i]);
				}
			}
		}

/* 1.1 - this form of constructor is never used - JHS
		Result(
			int position,
			std::string error,
			std::string idcode,
			std::string name,
			std::string damage,
			std::string date,
			std::string location	
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

		Result(const Result& r)
		: 
      		mFilename(r.mFilename),   //  001DB
			mPosition(r.mPosition),
			mError(r.mError),
			mIdCode(r.mIdCode),
			mName(r.mName),
			mDamage(r.mDamage),
			mLocation(r.mLocation),
			mRank(r.mRank), //  1.5
			unknownContour(new FloatContour(*r.unknownContour)), //  1.3 - Mem Leak - make copies now
			dbContour(new FloatContour(*r.dbContour)),           //  1.3 - Mem Leak - make copies now
			mUnkShiftedLEBegin(r.mUnkShiftedLEBegin), 
			mUnkShiftedTip(r.mUnkShiftedTip), 
			mUnkShiftedTEEnd(r.mUnkShiftedTEEnd),
			mDBShiftedLEBegin(r.mDBShiftedLEBegin), 
			mDBShiftedTip(r.mDBShiftedTip), 
			mDBShiftedTEEnd(r.mDBShiftedTEEnd)

		{
			if (NULL == r.mThumbnailPixmap) {
				mThumbnailPixmap = NULL;
				mThumbnailRows = 0;
			} else {
				mThumbnailRows = r.mThumbnailRows;
				mThumbnailPixmap = new char*[mThumbnailRows];

				for (int i = 0; i < mThumbnailRows; i++) {
					mThumbnailPixmap[i] = new char[strlen(r.mThumbnailPixmap[i]) + 1];
					strcpy(mThumbnailPixmap[i], r.mThumbnailPixmap[i]);
				}
			}
		}

		~Result()
		{
			if (NULL != mThumbnailPixmap)
				freePixmapString(mThumbnailPixmap, mThumbnailRows);
			//  1.3 - cannot delete the contours here - don't remember why
			// they are (probably) just pointers to contours managed elsewhere
			
			//  1.3 - MemLeak - must delete copies now
			if (NULL != unknownContour)
				delete unknownContour;
			if (NULL != dbContour)
				delete dbContour;
			
		}

		Result& operator=(const Result &r)
		{
			if (this == &r)
				return *this;

			mPosition = r.mPosition;
			mError = r.mError;
			mIdCode = r.mIdCode;
			mName = r.mName;
			mDamage = r.mDamage;
			mLocation = r.mLocation;
			mRank = r.mRank; //  1.5
			unknownContour = r.unknownContour; //  005CM
			dbContour = r.dbContour; //  005CM

			if (NULL == r.mThumbnailPixmap) {
				mThumbnailPixmap = NULL;
				mThumbnailRows = 0;
			} else {
				mThumbnailRows = r.mThumbnailRows;
				mThumbnailPixmap = new char*[mThumbnailRows];

				for (int i = 0; i < mThumbnailRows; i++) {
					mThumbnailPixmap[i] = new char[strlen(r.mThumbnailPixmap[i]) + 1];
					strcpy(mThumbnailPixmap[i], r.mThumbnailPixmap[i]);
				}
			}		

			return *this;
		}
		
		int getPosition() const { return mPosition; }
		std::string getName() const { return mName; }
		std::string getDate() const { return mDate; }
		std::string getError() const { return mError; }
		std::string getIdCode() const { return mIdCode; }
		std::string getDamage() const { return mDamage; }
		std::string getLocation() const { return mLocation; }
		std::string getRank() const { return mRank; } //  1.5

		void setRank (const std::string rank) {mRank = rank;} //  1.5

		//  1.1 - sets six indices for points used in final contour mapping
		void setMappingControlPoints(
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
		void getMappingControlPoints(
				int &unkLEBegin, int &unkTip, int &unkTEEnd,
				int &dbLEBegin, int &dbTip, int &dbTEEnd)
		{
			unkLEBegin = mUnkShiftedLEBegin;
			unkTip = mUnkShiftedTip;
			unkTEEnd = mUnkShiftedTEEnd;
			dbLEBegin = mDBShiftedLEBegin;
			dbTip = mDBShiftedTip;
			dbTEEnd = mDBShiftedTEEnd;
		}
		
		//  005CM - contours used in final match
		FloatContour 
			*unknownContour,
			*dbContour;

		char **mThumbnailPixmap;
		int mThumbnailRows;

	private:
	
		std::string mFilename;    //  001DB - image file for database fin
		int mPosition;            // position (index) of database fin in database file

		std::string
			mError,
			mIdCode,
			mName,
			mDamage,
			mDate,
			mLocation,
			mRank; //  1.5
		
		//  1.1 - new members to track three point correspondences for final mapping in match

		int 
			mUnkShiftedLEBegin, 
			mUnkShiftedTip, 
			mUnkShiftedTEEnd,
			mDBShiftedLEBegin, 
			mDBShiftedTip, 
			mDBShiftedTEEnd;

};	

typedef enum {MR_ERROR, MR_NAME, MR_IDCODE, MR_DAMAGE, MR_DATE, MR_LOCATION} mr_sort_t;

class MatchResults {

	public:

		MatchResults()
		: 
			mLastSortBy(MR_ERROR),
			mTimeTaken(-1.00),
			mFinID(""),
			mTracedFinFile(""),
			mDatabaseFile("")
		{ }
			  
		MatchResults(std::string id)
		: 
			mLastSortBy(MR_ERROR),
			mTimeTaken(-1.00),
			mFinID(id),
			mTracedFinFile(""),
			mDatabaseFile("")
		{ }


		//  008OL -- MatchResultsWindow calls constructor of this type and none existed
		MatchResults(const MatchResults &results)
		: 
			mLastSortBy(results.mLastSortBy),
			mTimeTaken(results.mTimeTaken),
			mFinID(results.mFinID),
			mResults(results.mResults),
			mTracedFinFile(results.mTracedFinFile),
			mDatabaseFile(results.mDatabaseFile)
		{ }

		//  1.0LK - fixing memory leak
		~MatchResults()
		{
			mResults.clear();
		}

		void addResult(const Result& r);
		int size() const { return mResults.size(); }
		
		// sort assumes sorting by last
		void sort() { int dummy=0; sort(mLastSortBy, dummy); }

		void sort(mr_sort_t sortBy, int &active); 

		//  1.5 - indicates whether sorted order is by error measure, so rank numbering is appropriate
		bool LastSortedByError(); //  1.5

		void setRankings(); //  1.5

		// doesn't make a copy to save time... so DON'T DELETE
		// THE RESULT WHEN DONE
		Result* getResultNum(int resultNum);

		void setTimeTaken(float timeTaken);

		// getTimeTaken
		// 	A return of -1.00 indicates that the amount of
		// 	time is undefined.
		float getTimeTaken();

		void save(std::string fileName);

		int findRank();

		//  1.1 - the following functions used in MatchQueue context

		void setFinFilename(std::string fname) //  1.1
		{	mTracedFinFile = fname; }

		void setDatabaseFilename(std::string fname) //  1.1
		{	mDatabaseFile = fname; }

		DatabaseFin<ColorImage> *load(Database *db, std::string fileName);


	private:
	
		mr_sort_t mLastSortBy;
		std::list<Result> mResults;		
		float mTimeTaken;
		std::string mFinID; // this is the unknown fin ID

		// following is used by save() to write info into saved results of 
		// MatchQueue initiated match

		std::string mTracedFinFile; //  1.1 - "" or file containing saved unknown DatabaseFin
		std::string mDatabaseFile; //  1.1 - "" or database file matched by MatchQueue
};

#endif
