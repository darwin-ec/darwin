//*******************************************************************
//   file: Database.cxx
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

#include "Database.h"


//*******************************************************************
//
// Returns number of fins in lists.
//

unsigned Database::size() const
{
	return mNameList.size();
}

//*******************************************************************
//
// Returns number of fins in AbsoluteOffset list.
//

unsigned Database::sizeAbsolute() const
{
	return mAbsoluteOffset.size();
}

//*******************************************************************
//

bool Database::isEmpty() const
{
	return (mNameList.size() == 0);
}

//*******************************************************************
//

Database::db_status_t Database::status() const
{
	return mDBStatus;
}

//*******************************************************************
//***1.85
//

db_sort_t Database::currentSort()
{
	return mCurrentSort;
}

//*******************************************************************
//

void Database::sort(db_sort_t sortBy)
{
	mCurrentSort = sortBy;
}

//*******************************************************************
//***1.85 - returns position of id in mIDList as currently sorted
//

int Database::getIDListPosit(std::string id)
{
	// NOTE: this list in the database contains strings, but the strings
	// are made up of two parts (The Dolphin ID -- a string -- and
	// an integer that either the data offset in the old database
	// or the unique numerical id of the fin 
	// in the Invidiuals table in the SQLite database.)
	
	std::vector<std::string>::iterator it;

	bool found(false);
	int posit(0);
	it = mIDList.begin();

	while ((!found) && (it != mIDList.end()))
	{
		std::string listID = *it;
		listID = listID.substr(0,listID.rfind(" ")); // strip the offset
		// should we ignore CASE????
		if (id == listID)
			found = true;
		else
		{
			++it;
			posit++;
		}
	}
	if (! found)
		posit = NOT_IN_LIST;

	return posit;
}


//*******************************************************************
//
// Returns item from list at given position
//

string Database::getItemEntryFromList(db_sort_t whichList, unsigned pos) {	

	if (pos > this->size())
	       throw BoundsError();

	std::vector<std::string> *it;

	switch (whichList) {
		case DB_SORT_NAME :
			it = &mNameList;
			break;
		case DB_SORT_ID :
			it = &mIDList;
			break;
		case DB_SORT_DATE :
			it = &mDateList;
			break;
		case DB_SORT_ROLL :
			it = &mRollList;
			break;
		case DB_SORT_LOCATION :
			it = &mLocationList;
			break;
		case DB_SORT_DAMAGE :
			it = &mDamageList;
			break;
		case DB_SORT_DESCRIPTION :
			it = &mDescriptionList;
			break;
		default : // it's not a valid sort type
			return "";
	}

	return (*it)[pos];
}

//*******************************************************************
//
// Returns pos of item in list with given string
//

int Database::getItemListPosFromOffset(db_sort_t whichList, string item) {

	std::vector<std::string>::iterator it, last;

	switch (whichList) {

		case DB_SORT_NAME :
			it = mNameList.begin();
			last = mNameList.end();
			break;
		case DB_SORT_ID :
			it = mIDList.begin();
			last = mIDList.end();
			break;
		case DB_SORT_DATE :
			it = mDateList.begin();
			last = mDateList.end();
			break;
		case DB_SORT_ROLL :
			it = mRollList.begin();
			last = mRollList.end();
			break;
		case DB_SORT_LOCATION :
			it = mLocationList.begin();
			last = mLocationList.end();
			break;
		case DB_SORT_DAMAGE :
			it = mDamageList.begin();
			last = mDamageList.end();
			break;
		case DB_SORT_DESCRIPTION :
			it = mDescriptionList.begin();
			last = mDescriptionList.end();
			break;
		default : // it's not a valid sort type
			return NOT_IN_LIST;
	}

	string itemOffset = item.substr(1 + item.rfind(" "));

	bool found(false);
	int posit(0);

	while ((!found) && (it != last)) {
		std::string offset = *it;
		offset = offset.substr(1 + offset.rfind(" ")); // keep just the offset

		if (itemOffset == offset)
			found = true;
		else {
			++it;
			posit++;
		}
	}

	if (! found)
		posit = NOT_IN_LIST;

	return posit;
}


// *****************************************************************************
//
// Returns db filename
//

string Database::getFilename() {

	return mFilename;
}


// *****************************************************************************
//
// Constructor - plain vanilla (***1.99)
//

Database::Database(Options *o, bool createEmptyDB) {
	
	dbOpen = false;
	mFilename = std::string(o->mDatabaseFileName);
	mCurrentSort = DB_SORT_NAME;
	mDBStatus = errorLoading;
}
