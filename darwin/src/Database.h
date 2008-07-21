//*******************************************************************
//   file: Database.h
//
// author: J H Stewman (7/8/2008)
//
// This ABSTRACT class is the parent for ALL present and future
// types of database implementations.  Current derived classes
// include ...
//    OldDatabase -- the flat file version
//    SQLiteDatabase -- the first SQL relational database version
//
//*******************************************************************

#ifndef DATABASE_H
#define DATABASE_H
#include "DatabaseFin.h"

#include "Error.h"

#include "Options.h"

#include "CatalogScheme.h" //***1.99

#include <fstream>
#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include <strstream>
#include <vector>
#include <algorithm>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "image_processing/ColorImage.h"
#include <list>
#include <sstream>
#include <ctime>

#include <iostream>

#ifdef WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "/"
#endif

#define NOT_IN_LIST -1

typedef enum {
	DB_SORT_NAME,
	DB_SORT_ID,
	DB_SORT_DATE,
	DB_SORT_ROLL,
	DB_SORT_LOCATION,
	DB_SORT_DAMAGE,
	DB_SORT_DESCRIPTION
} db_sort_t;


//******************************************************************
// Function Definitions
//******************************************************************
class Database {
public:

	typedef enum {
			loaded = 0,
			fileNotFound,
			errorLoading,
			errorCreating,
			oldDBVersion
	} db_status_t;

	Database(Options *o, CatalogScheme cat, bool createEmptyDB);
	Database(); // called only by DummyDatabase()
	virtual ~Database() {};
	
	virtual void createEmptyDatabase(Options *o) = 0;

	virtual unsigned long add(DatabaseFin<ColorImage>* data) = 0; 
	virtual void Delete(DatabaseFin<ColorImage> *Fin) = 0;

	virtual DatabaseFin<ColorImage>* getItemAbsolute(unsigned pos) = 0;

	virtual DatabaseFin<ColorImage>* getItem(unsigned pos) = 0;
	// virtual DatabaseFin<ColorImage>* getItemByName(std::string name) = 0;  

	void sort(db_sort_t sortBy);

	unsigned size() const;
	unsigned sizeAbsolute() const; //***1.3 - size of absolute offset list
	bool isEmpty() const;

	db_sort_t currentSort(); //***1.85

	db_status_t status() const; //***1.85
	int getIDListPosit(std::string id); //***1.85

	class BoundsError : public Error 
	{
		public:
			BoundsError() : Error("Attempt to access element out of database bounds.")
			{ }
	};

	//***1.85 - new functions for processing lists IN MEMORY without file access
		
	std::string getItemEntryFromList(db_sort_t whichList, unsigned pos); //***1.85

	int getItemListPosFromOffset(db_sort_t whichList, std::string item); //***1.85

	std::string getFilename(); //***1.85

	virtual bool openStream() = 0;
	virtual bool closeStream() = 0;

	//***1.99 - new access functions for catalog scheme moved from Options
	std::string catCategoryName(int id);
	std::string catSchemeName();
	int catCategoryNamesMax();
	void appendCategoryName(std::string name);
	void setCatSchemeName(std::string name);
	void clearCatalogScheme();
	CatalogScheme catalogScheme();

protected:
	bool dbOpen;

	db_status_t mDBStatus; //***1.85

	std::string mFilename;

	//***1.99 - the catalog scheme for this database (moved from Options)
	std::vector<std::string> 
		mCatCategoryNames;     // names of catalog categories
	std::string 
		mCatSchemeName;       // name of catalog scheme

	unsigned long mFooterPos;
	unsigned long mDataSize;
	unsigned long mHeaderSize; //***054

	/*
	 * These store strings of the format "value pos" where value holds 
	 * whatever the list name refers to.  "NONE" is used for empty values.
	 * pos originally referred to the offset in the catalogue file. Now,
	 * pos refers to the id field of the Individuals table in the db.
	 */
	std::vector<std::string>
		mNameList,
		mIDList,
		mDateList,
		mRollList,
		mLocationList,
		mDamageList,
		mDescriptionList;

	//***1.3 - absolute file locations of all fins (even deleted holes)
	std::vector<long int>
		mAbsoluteOffset;

	db_sort_t mCurrentSort;

	//DatabaseFin* getItem(unsigned pos, std::list<std::string>::iterator it);
	virtual DatabaseFin<ColorImage>* getItem(unsigned pos, std::vector<std::string> *theList) = 0;

private:
	
	//void loadLists();
	//void sortLists();
};

#endif // DATABASE_H