//*******************************************************************
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
//*******************************************************************

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

#ifndef DATABASEFIN_HH
#define DATABASEFIN_HH

#include "image_processing/ColorImage.h"
#include "image_processing/GrayImage.h"
#include "FloatContour.h" //***008OL
#include "Outline.h"      //***008OL - JHS addition
#include "Chain.h"        //***008OL 
#include "image_processing/conversions.h"
#include "Options.h" //***1.85
#include "utility.h"
#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include <fstream>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//***054 - slash direction Windows/Linux
#ifdef WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "/"
#endif

const int DATABASEFIN_THUMB_HEIGHT = 25, DATABASEFIN_THUMB_WIDTH = 25;

extern Options *gOptions;

//*******************************************************************
//
// CLASS ItemInfo
//
//*******************************************************************

class ItemInfo 
{
	public:
		ItemInfo(
			const std::string &idCode,
			const std::string &name,
			const std::string &dateOfSighting,
			const std::string &rollAndFrame,
			const std::string &locationCode,
			const std::string &damageCategory,
			const std::string &shortDescription
		) :
			mIDCode(idCode),
			mName(name),
			mDateOfSighting(dateOfSighting),
			mRollAndFrame(rollAndFrame),
			mLocationCode(locationCode),
			mDamageCategory(damageCategory),
			mShortDescription(shortDescription)
		{
			if ("" == mIDCode)
				mIDCode = "NONE";
			if ("" == mName)
				mName = "NONE";
			if ("" == mDateOfSighting)
				mDateOfSighting = "NONE";
			if ("" == mRollAndFrame)
				mRollAndFrame = "NONE";
			if ("" == mLocationCode)
				mLocationCode = "NONE";
			if ("" == mDamageCategory)
				mDamageCategory = "NONE";
			if ("" == mShortDescription)
				mShortDescription = "NONE";
		}

		~ItemInfo ()
		{ }

#ifdef DEBUG
		void print() const
		{
			std::cout << "ID Code: " << mIDCode << std::endl
				<< "Name: " << mName << std::endl
				<< "Date of Sighting: " << mDateOfSighting << std::endl
				<< "Roll and Frame: " << mRollAndFrame << std::endl
				<< "Location Code: " << mLocationCode << std::endl
				<< "Damage Category: " << mDamageCategory << std::endl
				<< "Short Description: " << mShortDescription << std::endl;
		}
#endif

		std::string mIDCode;
		std::string mName;
		std::string mDateOfSighting;
		std::string mRollAndFrame;
		std::string mLocationCode;
		std::string mDamageCategory;
		std::string mShortDescription;

		bool mIsAlternate; //***1.95
};

//*******************************************************************
//
// CLASS DatabaseFin
//
//*******************************************************************

template <class FIN_IMAGE_TYPE>
class DatabaseFin
{
	public:

		//**************************************************************************
		//
		// no sense in having this unless we set member values
		//
		DatabaseFin(){}

		//**************************************************************************
		//
		// called from numerous places in ...
		//   TraceWindow.cxx, ModifyDatabaseWindow.cxx, and 
		//   NoMatchWindow.cxx
		//
		DatabaseFin(
			const std::string filename, //***001DB
			Outline *outline, //***008OL
			const std::string &idcode,
			const std::string &name,
			const std::string &dateOfSighting,
			const std::string &rollAndFrame,
			const std::string &locationCode,
			const std::string &damageCategory,
			const std::string &shortDescription
		) :
			mImageFilename(filename), //***001DB
			mFinOutline(new Outline(outline)), //***006DF,008OL
			mIDCode(idcode),
			mName(name),
			mDateOfSighting(dateOfSighting),
			mRollAndFrame(rollAndFrame),
			mLocationCode(locationCode),
			mDamageCategory(damageCategory),
			mShortDescription(shortDescription),
			mThumbnailPixmap(NULL),
			mThumbnailRows(0),
			mLeft(true), //***1.4
			mFlipped(false), //***1.4
			mXmin(0.0), //***1.4
			mXmax(0.0), //***1.4
			mYmin(0.0), //***1.4
			mYmax(0.0), //***1.4
			mScale(1.0), //***1.4
			mModifiedFinImage(NULL), //***1.5
			mFinFilename(""), //***1.6
			mIsAlternate(false) //***1.95

		{
			//***1.5 - need some way to CATCH error thrown when image file
			//         does not exist or is unsupported type  --
			//         program now crashes when database image is misplaced or misnamed

			mFinImage=new FIN_IMAGE_TYPE(mImageFilename); //***001DB
			FIN_IMAGE_TYPE *thumb = resizeWithBorderNN(
					mFinImage,
					DATABASEFIN_THUMB_HEIGHT,
					DATABASEFIN_THUMB_WIDTH);
			convToPixmapString(thumb, mThumbnailPixmap, mThumbnailRows);
			delete thumb;
		}

					
		//***1.99 - new constructor used by SQLlite database code
		//**************************************************************************
		//
		// Added. Called in Database::getFin().
		//
		DatabaseFin(
			const std::string filename, //***001DB
			Outline *outline, //***008OL
			const std::string &idcode,
			const std::string &name,
			const std::string &dateOfSighting,
			const std::string &rollAndFrame,
			const std::string &locationCode,
			const std::string &damageCategory,
			const std::string &shortDescription,
			int datapos,
			char **pixmap,
			int rows
		) :
			mImageFilename(filename), //***001DB
			mFinOutline(new Outline(outline)), //***006DF,008OL
			mIDCode(idcode),
			mName(name),
			mDateOfSighting(dateOfSighting),
			mRollAndFrame(rollAndFrame),
			mLocationCode(locationCode),
			mDamageCategory(damageCategory),
			mShortDescription(shortDescription),
			mThumbnailPixmap(pixmap),
			mThumbnailRows(rows),
			mDataPos(datapos),
			mLeft(true), //***1.4
			mFlipped(false), //***1.4
			mXmin(0.0), //***1.4
			mXmax(0.0), //***1.4
			mYmin(0.0), //***1.4
			mYmax(0.0), //***1.4
			mScale(1.0), //***1.4
			mModifiedFinImage(NULL), //***1.5
			mFinFilename(""), //***1.6
			mFinImage(NULL)
		{
			// let's see what happens... -- rjn
			/*
			mFinImage=new FIN_IMAGE_TYPE(mImageFilename); //***001DB
			FIN_IMAGE_TYPE *thumb = resizeWithBorderNN(
					mFinImage,
					DATABASEFIN_THUMB_HEIGHT,
					DATABASEFIN_THUMB_WIDTH);
			convToPixmapString(thumb, mThumbnailPixmap, mThumbnailRows);
			delete thumb;
			*/
		}


		/*
		//**************************************************************************
		//
		// don't think this is ever called
		//
		DatabaseFin(
			std::string filename,        //***001DB
			Outline *outline,            //***008OL
			const ItemInfo *iteminfo
		) :
			mImageFilename(filename),          //***001DB
			mFinOutline(new Outline(outline)), //***008OL
			mIDCode(iteminfo->mIDCode),
			mName(iteminfo->mName),
			mDateOfSighting(iteminfo->mDateOfSighting),
			mRollAndFrame(iteminfo->mRollAndFrame),
			mLocationCode(iteminfo->mLocationCode),
			mDamageCategory(iteminfo->mDamageCategory),
			mShortDescription(iteminfo->mShortDescription),
			mThumbnailPixmap(NULL),
			mThumbnailRows(0)
		{
			
			mFinImage=new FIN_IMAGE_TYPE(filename); //***001DB
			FIN_IMAGE_TYPE *thumb = resizeWithBorderNN(
					mFinImage,
					DATABASEFIN_THUMB_HEIGHT,
					DATABASEFIN_THUMB_WIDTH);
			convToPixmapString(thumb, mThumbnailPixmap, mThumbnailRows);
			delete thumb;
		}
		*/
      
		//**************************************************************************
		//
		// called ONLY from Match.cxx and MatchResultsWindow.cxx
		//
		DatabaseFin(const DatabaseFin<FIN_IMAGE_TYPE> *fin)
			:
			mImageFilename(fin->mImageFilename),        //***001DB
			mFinImage(NULL),                            //*** major change JHS
			mModifiedFinImage(NULL), //***1.5
			mDataPos(fin->mDataPos),                    //***001DB
			mFinOutline(new Outline(fin->mFinOutline)), //***006DF,008OL
			mIDCode(fin->mIDCode),
			mName(fin->mName),
			mDateOfSighting(fin->mDateOfSighting),
			mRollAndFrame(fin->mRollAndFrame),
			mLocationCode(fin->mLocationCode),
			mDamageCategory(fin->mDamageCategory),
			mShortDescription(fin->mShortDescription),
			mThumbnailPixmap(new char*[fin->mThumbnailRows]),
			mThumbnailRows(fin->mThumbnailRows),
			mLeft(fin->mLeft), //***1.4
			mFlipped(fin->mFlipped), //***1.4
			mXmin(fin->mXmin), //***1.4
			mXmax(fin->mXmax), //***1.4
			mYmin(fin->mYmin), //***1.4
			mYmax(fin->mYmax), //***1.4
			mScale(fin->mScale), //***1.4
			mFinFilename(fin->mFinFilename), //***1.6
			mOriginalImageFilename(fin->mOriginalImageFilename), //***1.8
			mImageMods(fin->mImageMods), //***1.8
			mIsAlternate(fin->mIsAlternate) //***1.95
		{
			//***1.5 - just set pointer to original copy from TraceWindow
			//***1.8 - we actually create a COPY of the modified image here
			if (NULL != fin->mModifiedFinImage)
				mModifiedFinImage = new ColorImage(fin->mModifiedFinImage);
			//***1.8 - and we create a COPY of the original image here
			if (NULL != fin->mFinImage)
				mFinImage = new ColorImage(fin->mFinImage);

			for (int i = 0; i < fin->mThumbnailRows; i++) 
			{
				mThumbnailPixmap[i] = new char[strlen(fin->mThumbnailPixmap[i]) + 1];
				strcpy(mThumbnailPixmap[i], fin->mThumbnailPixmap[i]);
			}
		}

		/*
		//**************************************************************************
		//
		// don't know if called at all
		//
		DatabaseFin(const DatabaseFin<FIN_IMAGE_TYPE> &fin)
			:
			mImageFilename (fin.mImageFilename), //***001DB
			mFinImage(new FIN_IMAGE_TYPE(fin.mImageFilename)), //***001DB
			mFinOutline(new Outline(fin.mFinOutline)), //***006DF,008OL
			mIDCode(fin.mIDCode),
			mName(fin.mName),
			mDateOfSighting(fin.mDateOfSighting),
			mRollAndFrame(fin.mRollAndFrame),
			mLocationCode(fin.mLocationCode),
			mDamageCategory(fin.mDamageCategory),
			mShortDescription(fin.mShortDescription),
			mThumbnailPixmap(new char*[fin.mThumbnailRows]),
			mThumbnailRows(fin.mThumbnailRows)
		{
			for (int i = 0; i < fin.mThumbnailRows; i++) {
				mThumbnailPixmap[i] = new char[strlen(fin.mThumbnailPixmap[i]) + 1];
				strcpy(mThumbnailPixmap[i], fin.mThumbnailPixmap[i]);
			}
		}
		*/

		//**************************************************************************
		//
		// called ONLY from MatchingQueue.cxx 
		//
		//***1.4 - now also from OpenFileChooserDialog.cxx when loading single fin trace
		//
		DatabaseFin(
				const std::string &fileName
		) :
			mFinImage(NULL),     //***1.1
			mModifiedFinImage(NULL), //***1.5
			mImageFilename (""), //***1.1 - fileName is name of fin trace file NOT image
			mFinOutline(NULL),   //***008OL
			mIDCode(""),
			mName(""),
			mDateOfSighting(""),
			mRollAndFrame(""),
			mLocationCode(""),
			mDamageCategory(""),
			mShortDescription(""),
			mThumbnailPixmap(NULL),
			mThumbnailRows(0),
			mLeft(true), //***1.4
			mFlipped(false), //***1.4
			mXmin(0.0), //***1.4
			mXmax(0.0), //***1.4
			mYmin(0.0), //***1.4
			mYmax(0.0), //***1.4
			mScale(1.0), //***1.4
			mFinFilename(""), //***1.6 - gets set to fileName in load if process succeeds
			mIsAlternate(false) //***1.95
		{
			try {
				this->load(fileName);
			} catch (...) {
				throw;
			}
		}

		//**************************************************************************
		//
		// called ONLY from Database.h getItem(pos) member function
		//
		DatabaseFin(
			std::fstream &inFile,
			std::string fileName //***1.99 needed to allow opening db without gOptions
		) :
			mFinImage(NULL),
			mModifiedFinImage(NULL), //***1.5
			mFinOutline(NULL), //***008OL
			mIDCode(""),
			mName(""),
			mDateOfSighting(""),
			mRollAndFrame(""),
			mLocationCode(""),
			mDamageCategory(""),
			mShortDescription(""),
			mThumbnailPixmap(NULL),
			mThumbnailRows(0),
			mLeft(true), //***1.4
			mFlipped(false), //***1.4
			mXmin(0.0), //***1.4
			mXmax(0.0), //***1.4
			mYmin(0.0), //***1.4
			mYmax(0.0), //***1.4
			mScale(1.0), //***1.4
			mFinFilename(""), //***1.4
			mIsAlternate(false) //***1.95
		{
			try {
				string status = this->load(inFile,fileName,true);
				if(status != "Loaded")
					throw Error(status.c_str());
			} catch (...) {
				throw;
			}
		}

		//**************************************************************************
		//
		~DatabaseFin()
		{
			if(mFinImage != NULL)     //***001DB
				delete mFinImage;

			if(mModifiedFinImage != NULL)     //***001DB
				delete mModifiedFinImage;

			if(mFinOutline != NULL)   //***006DF,008OL
   				delete mFinOutline;   //***008OL

			try {
				freePixmapString(mThumbnailPixmap, mThumbnailRows);
			} catch (...) {
				throw;
			}
		}

		/*
		//**************************************************************************
		//
		//***1.85 - this is never used probably needs to be recoded anyway - JHS

		DatabaseFin<FIN_IMAGE_TYPE>& operator=(const DatabaseFin<FIN_IMAGE_TYPE> &fin)
		{
			if (*this == fin)
				return *this;
			
			mImageFilename = fin.mImagefilename;            //***001DB
			mDataPos = fin.mDataPos;                        //***001DB
			mFinImage = new FIN_IMAGE_TYPE(mImageFilename); //***001DB
			mFinOutline = new Outline(fin.mFinOutline);     //***006DF,008OL
			mIDCode = fin.mIDCode;
			mName = fin.mName;
			mDateOfSighting = fin.mDateOfSighting;
			mRollAndFrame = fin.mRollAndFrame;
			mLocationCode = fin.mLocationCode;
			mDamageCategory = fin.mDamageCategory;
			mShortDescription = fin.mShortDescription;
			mThumbnailPixmap = new char*[fin.mThumbnailRows];
			mThumbnailRows = fin.mThumbnailRows;
			mNormScale = fin.mNormScale; //***1.6
			mFinFilename = fin.mFinFilename; //***1.6

			for (int i = 0; i < fin.mThumbnailRows; i++) 
			{
				mThumbnailPixmap[i] = new char[strlen(fin.mThumbnailPixmap[i]) + 1];
				strcpy(mThumbnailPixmap[i], fin.mThumbnailPixmap[i]);
			}

			return *this;
		}
		*/
		//**************************************************************************
		//
		unsigned int getSizeInBytes() const
		{
			unsigned size =
				sizeof(unsigned long)            // For Data position    //***001DB
				+ mImageFilename.length() + 1    // Size of filename //***001DB
				//+ mModifiedImageFilename.length() + 1    // Size of filename //***1.8
				+ sizeof(unsigned int)           // For # of contour points
				+ mFinOutline->length() * 2 * sizeof(float) // for the float contour point coords
				+ 5 * sizeof(int)                // For feature point indices ***006DF
				//+ 2                              //***1.4 - for left and flipped ('1' or '0') 
				+ mIDCode.length() + 1           // idcode + newline char
				+ mName.length() + 1             // ditto pretty much
				+ mDateOfSighting.length() + 1
				+ mRollAndFrame.length() + 1
				+ mLocationCode.length() + 1
				+ mDamageCategory.length() + 1
				+ mShortDescription.length() + 1
				+ sizeof(int);                   // for the number of thumbnail rows

			if (NULL != mThumbnailPixmap) {
				for (int i = 0; i < mThumbnailRows; i++)
					size += strlen(mThumbnailPixmap[i]) + 1;
			}

			return size;
		}

		//**************************************************************************
		//
		std::string getName() const
		{
			return mName;
		}

		//**************************************************************************
		//
		std::string getID() const  		//***002DB
		{
			return mIDCode;
		}
		
		//**************************************************************************
		//
		std::string getDate() const		//***002DB
		{
			return mDateOfSighting;
		}

		//**************************************************************************
		//
		std::string getRoll() const		//***002DB
		{
			return mRollAndFrame;
		}

		//**************************************************************************
		//
		std::string getLocation() const		//***002DB
		{
			return mLocationCode;
		}
		
		//**************************************************************************
		//
		std::string getDamage() const		//***002DB
		{
			return mDamageCategory;
		}

		//**************************************************************************
		//
		std::string getShortDescription() const	//***002DB
		{
			return mShortDescription;
		}

		//**************************************************************************
		//
		std::string getDescription()
		{
			return (mIDCode + "\n"
				+ mName + "\n" 
				+ mDateOfSighting + "\n"
				+ mRollAndFrame + "\n" 
				+ mLocationCode + "\n"
				+ mDamageCategory + "\n"
				+ mShortDescription + "\n");
		}


		//**************************************************************************
		//
		//***008OL strictly for debugging - JHS
		//
		friend void saveFinPlain(DatabaseFin<FIN_IMAGE_TYPE> *fin, std::string fname)
		{
			std::fstream outFile(fname.c_str(), ios::out);

			if (!outFile)
				throw Error("writing to file " + fname);
      
			outFile << fin->mDataPos << ' ';                 //***001DB
			
			outFile << fin->mImageFilename.c_str() << '\n'; //***001DB                                                    //***001DB
	
			unsigned int numPoints = fin->mFinOutline->length(); //***008OL
			outFile << numPoints <<'\n'; //***008OL

			FloatContour *fc = fin->mFinOutline->getFloatContour();

			// save the normalized, evenly spaced float contour
			for (unsigned i = 0; i < numPoints; i++) {
				outFile << (*fc)[i].x << ' ' << (*fc)[i].y << '\n'; //***008OL
			}

			int TipPos, BeginLE, EndLE, EndTE, NotchPos;
        
			TipPos = fin->mFinOutline->getFeaturePoint(TIP);
			outFile << TipPos << ' ';
        
			BeginLE = fin->mFinOutline->getFeaturePoint(LE_BEGIN);
			outFile << BeginLE << ' ';
        
			EndLE = fin->mFinOutline->getFeaturePoint(LE_END);
			outFile << EndLE << ' ';
        
			NotchPos = fin->mFinOutline->getFeaturePoint(NOTCH);
			outFile << NotchPos << ' ';
        
			EndTE = fin->mFinOutline->getFeaturePoint(POINT_OF_INFLECTION); // ????????????????
			outFile << EndTE <<'\n';
  
			fin->writePixmap(outFile);
			
			std::string descTemp = fin->getDescription();
	
			outFile << '\n' << descTemp.c_str() << '\n';

			outFile.close();
		}

		//**************************************************************************
		//
		//***008OL new version of save(fileName) 
		// This is ONLY called when saving a single traced fin in a stand alone file
		// This strips any path info from the image filename, assuming that the file
		// being created and the image file copy referenced inside it are in the same
		// folder. This shell then opens the file and calls save(outFile)
		//
		void save(const std::string &fileName)
		{
			std::fstream outFile(fileName.c_str(), ios::out | ios::binary);

			if (!outFile)
				throw Error("writing to file " + fileName);
      
			//***1.1 - assume that the filename for the image file contains path
			// information which must be stripped BEFORE saving fin in file and
			// BEFORE calculating the record size

			string shortFilename = this->mImageFilename;
			int pos = shortFilename.find_last_of(PATH_SLASH);
			if (pos >= 0)
			{
				shortFilename = shortFilename.substr(pos+1);
			}
			this->mImageFilename = shortFilename;
			//***1.1 end

			//***1.99 - assignment of magic number moved here from calling contexts
			this->mDataPos = 0x4E494644; // DO NOT CHANGE MAGIC # - it is "DFIN" in hex

			this->save(outFile);

			outFile.close();
		}

		//**************************************************************************
		//
		//***008OL - This is the current version of save()
		// note that this saves slightly different information
		// than void save(std::string fileName)..
		// [it doesn't put the string DBFIN at the top]
		//
		void save(std::fstream &outFile)
		{
			try {
				outFile.write((char*)&mDataPos,sizeof(unsigned long));                 //***001DB
			
				outFile.write((char*)mImageFilename.c_str(), mImageFilename.length()); //***001DB
				//outFile.write((char*)mModifiedImageFilename.c_str(), mModifiedImageFilename.length()); //***1.8
				outFile.put('\n');                                                     //***001DB
	
				unsigned int numPoints = mFinOutline->length(); //***008OL
  				outFile.write((char*)&numPoints, sizeof(int)); //***008OL

				FloatContour *fc = mFinOutline->getFloatContour();

				// save the normalized, evenly spaced float contour
				for (unsigned i = 0; i < numPoints; i++) 
				{
					outFile.write((char *)&((*fc)[i].x), sizeof(float)); //***008OL
					outFile.write((char *)&((*fc)[i].y), sizeof(float)); //***008OL
				}

				int TipPos, BeginLE, EndLE, EndTE, NotchPos;
        
				TipPos = mFinOutline->getFeaturePoint(TIP);
				outFile.write((char*)&TipPos, sizeof(int));
        
				BeginLE = mFinOutline->getFeaturePoint(LE_BEGIN);
				outFile.write((char*)&BeginLE, sizeof(int));
        
				EndLE = mFinOutline->getFeaturePoint(LE_END);
				outFile.write((char*)&EndLE, sizeof(int));
        
				NotchPos = mFinOutline->getFeaturePoint(NOTCH);
				outFile.write((char*)&NotchPos, sizeof(int));
        
				EndTE = mFinOutline->getFeaturePoint(POINT_OF_INFLECTION);
				outFile.write((char*)&EndTE, sizeof(int));

				this->writePixmap(outFile);
			
				std::string descTemp = this->getDescription();
	
				outFile.write((char*)descTemp.c_str(), descTemp.length());

				//saveFinPlain(this,"c:\\finPlain.txt"); //***008OL for debugging - JHS

			} catch (...) {
				throw;
			}
		}


		//**************************************************************************
		//
		//***008OL new version of load(filename) - shell opens file
		// and calls load(inFile) below
		// this is only called when reading single fin from stand alone file
		// in the MatchingQueue code or elsewhere.
		//
		void load(const std::string &fileName)
		{
			std::fstream inFile(fileName.c_str(), ios::in | ios::binary);

			if (!inFile)
				throw Error("reading from file " + fileName);

			this->load(inFile, fileName, false);

			mFinFilename = fileName;
		}


		//**************************************************************************
		//
		//***008OL This is the current version of load
		// the fromDBfile paramter indicates that this databaseFin is being
		// loaded from the database (true) or being loaded from a stand alone
		// file (false).  The latter is the case when fins have been saved 
		// for MatchingQueue processing.  In this latter case the image
		// filename and path are absolute and should not be modified.  When
		// loading from the database, the filenames include no path info and
		// must be catenated with the correct catalog path
		//
		string load(std::fstream &inFile, std::string inFilename, bool fromDBfile)
		{
			try {
				char line[255];
                          
				inFile.read((char*)&mDataPos,sizeof(unsigned long));
                          
				inFile.getline(line,255);

				//***1.95 - test for <alt> tag on end of file name.
				// <alt> indicates that this is an ALTERNATE image/fin for a given
				// dolphin.  The tag is stripped and the filed in the fin object is
				// set to indicate the status of the fin/image
				// NOTE: mIsAlternate is set false by default in constructor

				std::string test = line;
				int pos = test.rfind('<'); // position of '<' in possible <alt> tag
				if ((pos != std::string::npos) && (test.substr(pos) == "<alt>"))
				{
					mIsAlternate = true; // tag found
					// now, strip tag off of filename
					line[pos] = '\0';
				}
                         
				//***1.99 - do NOT depend on GLOBAL gOptions anymore
				string path = inFilename;
				path = path.substr(0,path.rfind(PATH_SLASH)); // strip *.db or *.fin
				mImageFilename = path + PATH_SLASH + line;

															  
/*				//***054
				// new code to combine relative path with simple filename
				// EXCEPT in cases where fin is being loaded from stand alone file
				if (fromDBfile)
				{
					//mImageFilename = getenv("DARWINHOME");
					mImageFilename = gOptions->mCurrentSurveyArea; //***1.85
					mImageFilename += PATH_SLASH;
					mImageFilename += "catalog";
					mImageFilename += PATH_SLASH;
					mImageFilename += line;
				}
				else
				{
					//***1.1 - assume the database fin in stand alone file MUST be
					// in the "tracedFin" folder - JHS  
					//mImageFilename = getenv("DARWINHOME");
					mImageFilename = gOptions->mCurrentSurveyArea; //***1.85
					mImageFilename += PATH_SLASH;
					mImageFilename += "tracedFins";
					mImageFilename += PATH_SLASH;
					mImageFilename += line;
				}
*/                          
				//************************
				// Major change here
				// The constructors will no longer load the image of the fin
				// They will ALL set the mFinImage member to NULL
				// If a subsequent process needs to have access to
				// the image, it will create the image by calling
				// the appropriate image constructor, which will in turn
				// read the image file.
				//
				// this should reduce the disk reads during matching
				// JHS 3/29/2005
				//************************

				unsigned int numPoints;
				inFile.read((char*)&numPoints, sizeof(unsigned int));
                          
				FloatContour *newContour = new FloatContour();
				float xTemp, yTemp;

				for (unsigned i = 0; i < numPoints; i++) {
					inFile.read((char*)&xTemp, sizeof(float));
					inFile.read((char*)&yTemp, sizeof(float));

					newContour->addPoint(xTemp, yTemp);
				}

				if(mFinOutline) //***008OL
					delete mFinOutline; //***008OL
                             
				mFinOutline = new Outline(newContour); //***008OL

				//***1.0LK - newContour is COPIED in Outline so we must delete it here
				delete newContour;

				int TipPos, BeginLE, EndLE, EndTE, NotchPos;
      
				inFile.read((char*)&TipPos, sizeof(int));
				mFinOutline->setFeaturePoint(TIP,TipPos); //***008OL
			
				inFile.read((char*)&BeginLE, sizeof(int));
				mFinOutline->setFeaturePoint(LE_BEGIN,BeginLE); //***008OL
			
				inFile.read((char*)&EndLE, sizeof(int));
				mFinOutline->setFeaturePoint(LE_END,EndLE); //***008OL
			
				inFile.read((char*)&NotchPos, sizeof(int));
				mFinOutline->setFeaturePoint(NOTCH,NotchPos); //***008OL
			
				inFile.read((char*)&EndTE, sizeof(int));
				mFinOutline->setFeaturePoint(POINT_OF_INFLECTION,EndTE); //***008OL

				mFinOutline->setLEAngle(0.0,true); //***008OL
		
				this->readPixmap(inFile);
                             
				inFile.getline(line, 255);
				mIDCode = line;
                             
				inFile.getline(line, 255);
				mName = line;
                             
				inFile.getline(line, 255);
				mDateOfSighting = line;
                             
				inFile.getline(line, 255);
				mRollAndFrame = line;
                             
				inFile.getline(line, 255);
				mLocationCode = line;
                             
				inFile.getline(line, 255);
				mDamageCategory = line;
                             
				inFile.getline(line, 255);
				mShortDescription = line;
			
			} catch (...) {
				throw;
			}

			return "Loaded";
		}
  

		//**************************************************************************
		//
		void loadOldDB(std::fstream &inFile) 
		{ 
			//***005DB stub for future
		}

		//**************************************************************************
		//
		//          //////////////////// Members //////////////////////////
		//
		//**************************************************************************

		// mFinImage gets created from mImageFilename as needed.

		FIN_IMAGE_TYPE *mFinImage;
		std::string mImageFilename; //***001DB
  
		Outline *mFinOutline; //***008OL

		std::string mIDCode;
		std::string mName;
		std::string mDateOfSighting;
		std::string mRollAndFrame;
		std::string mLocationCode;
		std::string mDamageCategory;
		std::string mShortDescription;
		unsigned long mDataPos;     //***001DB
		char **mThumbnailPixmap;
		int mThumbnailRows;

		//***1.4 - new members for tracking image modifications during tracing
		bool mLeft, mFlipped;              // left side or flipped internally to swim left
		double mXmin, mYmin, mXmax, mYmax; // internal cropping bounds
		double mScale;                     // image to Outline scale change
		FIN_IMAGE_TYPE *mModifiedFinImage;      // modified fin image from TraceWin, ...

		ImageModList mImageMods;    //***1.8 - for list of image modifications
 		std::string mOriginalImageFilename; //***1.8 - filename of original unmodified image

		std::string mFinFilename;   //***1.6 - for name of fin file if fin saved outside DB

		bool mIsAlternate; //***1.95 - allow designation of primary and alternate fins/images

	private:
		
		//**************************************************************************
		//
		void writePixmap(std::fstream &outFile)
		{
			if (NULL == mThumbnailPixmap)
				return;
		
			outFile.write((char *)&mThumbnailRows, sizeof(int));

			std::string pixTemp = "";

			for (int i = 0; i < mThumbnailRows; i++) {
				pixTemp += mThumbnailPixmap[i];
				pixTemp += '\n';
			}
			
			outFile.write(pixTemp.c_str(), pixTemp.length());
		}

		//**************************************************************************
		//
		void readPixmap(std::fstream &inFile)
		{
			inFile.read((char *)&mThumbnailRows, sizeof(int));

			mThumbnailPixmap = new char*[mThumbnailRows];
			
			// How do you like this magic number?
			char line[2048];
			
			for (int i = 0; i < mThumbnailRows; i++) {
				inFile.getline(line, 2048);

				mThumbnailPixmap[i] = new char[strlen(line) + 1];
				strcpy(mThumbnailPixmap[i], line);
			}
		}
};



#endif
