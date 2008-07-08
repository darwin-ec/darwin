//*******************************************************************
//   file: Database.h
//
// author: Ronald J. Nowling (06/01/2007)
//
//*******************************************************************

/*
 * UGLY HACK: To work around some linking errors, I've decided to list
 * database.cpp as an include.  This means that if changes are made to
 * database.cpp, you have to clean and then do a full build. See end of
 * file for include of database.cpp .  I guess that the problems are
 * due to name-mangling.
 */

#ifndef DATABASE_H
#define DATABASE_H
#include "DatabaseFin.h"


#include "Error.h"

#include "Options.h"

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

#include "sqlite3.h"

//******************************************************************
// Global Variables
//******************************************************************
typedef struct { int id; std::string idcode; std::string name; int fkdamagecategoryid; } DBIndividual;
typedef struct { int id; std::string name; } DBDamageCategory; 
typedef struct { int id; int fkindividualid; std::string imagefilename; std::string dateofsighting; std::string rollandframe; std::string locationcode; std::string shortdescription; } DBImage;
typedef struct { int id; int rows; std::string pixmap; int fkimageid; } DBThumbnail;
typedef struct { int id; int tipposition; int beginle; int endle; int notchposition; int endte; int fkindividualid; } DBOutline;
typedef struct { int id; float xcoordinate; float ycoordinate; int fkoutlineid; int orderid; } DBPoint;
typedef struct { std::string key; std::string value; } DBInfo;
typedef struct { int id; int operation; int value1; int value2; int value3; int value4; int orderid; int fkimageid; } DBImageModification;


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
template <class DATABASE_IMAGE_TYPE>
class Database {
public:

	typedef enum {
			loaded = 0,
			fileNotFound,
			errorLoading,
			errorCreating,
			oldDBVersion
	} db_status_t;

	Database(Options *o, bool createEmptyDB); //***054 - since catagories may change

	~Database();
	
	/*
	 * These should NOT be used!  They are merely public as they are called by sqlite3_exec()
	 */
	static int callbackIndividuals(void *, int, char **, char **);
	static int callbackDamageCategories(void *, int, char **, char **);
	static int callbackDBInfo(void *, int , char **, char **);
	static int callbackImageModifications(void *, int , char **, char **);
	static int callbackImages(void *, int , char **, char **);
	static int callbackOutlines(void *, int , char **, char **);
	static int callbackPoints(void *, int , char **, char **);
	static int callbackThumbnails(void *, int , char **, char **);
		
	void createEmptyDatabase(Options *o); //***054

	unsigned long add(DatabaseFin<DATABASE_IMAGE_TYPE>* data); //***1.85 - return type changed
	void update(DatabaseFin<DATABASE_IMAGE_TYPE> *fin);
	DatabaseFin< DATABASE_IMAGE_TYPE >* getFin(int id);
	std::list< DatabaseFin<DATABASE_IMAGE_TYPE>* >* getAllFins(void);
	void Delete(DatabaseFin<DATABASE_IMAGE_TYPE> *Fin); //***002DB

	DatabaseFin<DATABASE_IMAGE_TYPE>* getItemAbsolute(unsigned pos); //***1.3

	DatabaseFin<DATABASE_IMAGE_TYPE>* getItem(unsigned pos);
	DatabaseFin<DATABASE_IMAGE_TYPE>* getItemByName(std::string name);  

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
		
	string getItemEntryFromList(db_sort_t whichList, unsigned pos); //***1.85

	int getItemListPosFromOffset(db_sort_t whichList, string item); //***1.85

	string getFilename(); //***1.85

	bool openStream();
	bool closeStream();

protected:
	bool dbOpen;

	db_status_t mDBStatus; //***1.85

	std::string mFilename;
	std::fstream mDbFile;

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

	//DatabaseFin<DATABASE_IMAGE_TYPE>* getItem(unsigned pos, std::list<std::string>::iterator it);
	DatabaseFin<DATABASE_IMAGE_TYPE>* getItem(unsigned pos, std::vector<std::string> *theList);

private:
	sqlite3 *db;
	char *zErrMsg;
	int rc;

	static char* handleNull(char *);
	int generateUniqueInt();
	static string escapeString(string);
	static string stripEscape(string);
	
	void setSyncMode(int mode);
	void beginTransaction();
	void commitTransaction();

	void selectAllDamageCategories(std::list<DBDamageCategory> *);
	DBDamageCategory selectDamageCategoryByName(std::string name);
	DBDamageCategory selectDamageCategoryByID(int id);
	void selectAllIndividuals(std::list<DBIndividual> *);
	DBIndividual selectIndividualByID(int);
	DBIndividual selectIndividualByName(std::string name);
	void selectIndividualsByFkDamageCategoryID(std::list<DBIndividual> *, int);
	void selectAllDBInfo(std::list<DBInfo> *);
	void selectAllImageModifications(std::list<DBImageModification> *);
	void selectImageModificationsByFkImageID(std::list<DBImageModification> *, int);
	void selectAllImages(std::list<DBImage> *);
	void selectImagesByFkIndividualID(std::list<DBImage> *, int);
	DBImage selectImageByFkIndividualID(int);
	void selectAllOutlines(std::list<DBOutline> *);
	DBOutline selectOutlineByFkIndividualID(int);
	void selectAllPoints(std::list<DBPoint> *);
	void selectPointsByFkOutlineID(std::list<DBPoint> *, int);
	void selectAllThumbnails(std::list<DBThumbnail> *);
	void selectThumbnailsByFkImageID(std::list<DBThumbnail> *, int);
	DBThumbnail selectThumbnailByFkImageID(int fkimageid);

	void insertIndividual(DBIndividual *);
	void insertDamageCategory(DBDamageCategory *);
	void insertPoint(DBPoint *);
	void insertDBInfo(DBInfo *);
	void insertOutline(DBOutline *);
	void insertImage(DBImage *);
	void insertImageModification(DBImageModification *);
	void insertThumbnail(DBThumbnail *);
	void insertPoints(std::list<DBPoint>* );
	void insertImageModifications(std::list<DBImageModification>* );

	void updateOutline(DBOutline *);
	void updateDamageCategory(DBDamageCategory *);
	void updateIndividual(DBIndividual *);
	void updateImage(DBImage *);
	void updateImageModification(DBImageModification *);
	void updateThumbnail(DBThumbnail *);
	void updateDBInfo(DBInfo *);

	void deletePoints(int);
	void deleteOutlineByFkIndividualID(int);
	void deleteOutlineByID(int);
	void deleteIndividual(int);
	void deleteDamageCategory(int);
	void deleteImage(int);
	void deleteImageModification(int);
	void deleteThumbnail(int);
	void deleteThumbnailByFkImageID(int id);

	void opendb(const char *);
	void closedb();
	void loadLists();

	void sortLists();


};

//#include "Database.cpp" -- all code already included below -- JHS

// *****************************************************************************
//
// Ensures that the given character array is valid, otherwise returns "NULL".
//
template <class DATABASE_IMAGE_TYPE>
char* Database<DATABASE_IMAGE_TYPE>::handleNull(char *in) {

	return strcmp(in, "\0") != 0 ? in : "NULL";
}

// *****************************************************************************
//
// Generates an unique int based on the number of seconds pasted since the Unix
// epoch.
//
template <class DATABASE_IMAGE_TYPE>
int Database<DATABASE_IMAGE_TYPE>::generateUniqueInt() {

	return (int) time(NULL);
}

// *****************************************************************************
//
// Escapes strings
//
template <class DATABASE_IMAGE_TYPE>
string Database<DATABASE_IMAGE_TYPE>::escapeString(string str) {
	string buffer = str;

	int pos = 0;

	while( (pos = buffer.find("'", pos)) != string::npos && pos <= buffer.size() ) {
		buffer.insert(pos, "'");
		pos += 2;
	}

	return buffer;
}
// *****************************************************************************
//
// Strip escapes
//
template <class DATABASE_IMAGE_TYPE>
string Database<DATABASE_IMAGE_TYPE>::stripEscape(string str) {
	string buffer = str;

	int pos = 0;
	
	while( (pos = buffer.find("''", pos) ) != string::npos) {
		buffer.erase(++pos, 1);
	}
	
	return buffer;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the Individuals
// table.  It populates a DBIndividual and then adds it to the 
// list<DBIndividual> passed as the first argument.
//
template <class DATABASE_IMAGE_TYPE>
int Database<DATABASE_IMAGE_TYPE>::callbackIndividuals(void *individuals, int argc, char **argv, char **azColName) {
	int i;
	DBIndividual temp;

	for(i = 0; i < argc; i++) {
			
		if(! strcmp(azColName[i], "ID") )
			temp.id = atoi( handleNull(argv[i]) );
		else if(! strcmp(azColName[i], "IDCode") )
			temp.idcode = stripEscape( handleNull(argv[i]) );
		else if(! strcmp(azColName[i], "Name") )
			temp.name = stripEscape( handleNull(argv[i]) );
		else if(! strcmp(azColName[i], "fkDamageCategoryID") )
			temp.fkdamagecategoryid = atoi( handleNull(argv[i]) );
	
	}
	
	((std::list<DBIndividual> *)individuals)->push_front(temp);

	return 0;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the
// DamageCategories table.  It populates a DBDamageCategory and then 
// adds it to the list<DBDamageCategory> passed as the first argument.
//
template <class DATABASE_IMAGE_TYPE>
int Database<DATABASE_IMAGE_TYPE>::callbackDamageCategories(void *damagecategories, int argc, char **argv, char **azColName) {
	int i;
	DBDamageCategory temp;

	for(i = 0; i < argc; i++) {
			
		if(! strcmp(azColName[i], "ID") )
			temp.id = atoi( handleNull(argv[i]) );
		else if(! strcmp(azColName[i], "Name") )
			temp.name = stripEscape( handleNull(argv[i]) );

	}
	
	((std::list<DBDamageCategory> *)damagecategories)->push_front(temp);


	return 0;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the
// DBInfo table.  It populates a DBInfo and then adds it to the 
// list<DBInfo> passed as the first argument.
//
template <class DATABASE_IMAGE_TYPE>
int Database<DATABASE_IMAGE_TYPE>::callbackDBInfo(void *dbinfo, int argc, char **argv, char **azColName) {
	int i;
	DBInfo temp;

	for(i = 0; i < argc; i++) {

		if(! strcmp(azColName[i], "Key"))
			temp.key = stripEscape( handleNull(argv[i]) );
		else if(! strcmp(azColName[i], "Value"))
			temp.value = stripEscape( handleNull(argv[i]) );

	}

	((std::list<DBInfo> *)dbinfo)->push_front(temp);

	return 0;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the
// ImageModifications table.  It populates a DBImageModification and   
// then adds it to the list<DBImageModification> passed as the first
// argument.
//
template <class DATABASE_IMAGE_TYPE>
int Database<DATABASE_IMAGE_TYPE>::callbackImageModifications(void *imagemods, int argc, char **argv, char **azColName) {
	int i;
	DBImageModification temp;

	for(i = 0; i < argc; i++) {

		if(! strcmp(azColName[i], "ID"))
			temp.id = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Operation"))
			temp.operation = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Value1"))
			temp.value1 = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Value2"))
			temp.value2 = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Value3"))
			temp.value3 = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Value4"))
			temp.value4 = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "OrderID"))
			temp.orderid = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "fkImageID"))
			temp.fkimageid = atoi(handleNull(argv[i]));

	}

	((std::list<DBImageModification> *) imagemods)->push_front(temp);

	return 0;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the Images
// table.  It populates a DBImage and then adds it to the
// list<DBImage> passed as the first argument.
//
template <class DATABASE_IMAGE_TYPE>
int Database<DATABASE_IMAGE_TYPE>::callbackImages(void *images, int argc, char **argv, char **azColName) {
	int i;
	DBImage temp;

	for(i = 0; i < argc; i++) {

		if(! strcmp(azColName[i], "ID"))
			temp.id = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "ImageFilename"))
			temp.imagefilename = stripEscape( handleNull(argv[i]) );

		else if(! strcmp(azColName[i], "DateOfSighting"))
			temp.dateofsighting = stripEscape( handleNull(argv[i]) );

		else if(! strcmp(azColName[i], "RollAndFrame"))
			temp.rollandframe = stripEscape( handleNull(argv[i]) );

		else if(! strcmp(azColName[i], "LocationCode"))
			temp.locationcode = stripEscape( handleNull(argv[i]) );

		else if(! strcmp(azColName[i], "ShortDescription"))
			temp.shortdescription = stripEscape( handleNull(argv[i]) );

		else if(! strcmp(azColName[i], "fkIndividualID"))
			temp.fkindividualid = atoi(handleNull(argv[i]));

	}

	((std::list<DBImage> *) images)->push_front(temp);

	return 0;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the Outlines
// table.  It populates a DBOutline and then adds it to the
// list<DBOutline> passed as the first argument.
//
template <class DATABASE_IMAGE_TYPE>
int Database<DATABASE_IMAGE_TYPE>::callbackOutlines(void *outlines, int argc, char **argv, char **azColName) {
	int i;
	DBOutline temp;

	for(i = 0; i < argc; i++) {

		if(! strcmp(azColName[i], "ID"))
			temp.id = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "TipPosition"))
			temp.tipposition = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "BeginLE"))
			temp.beginle = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "EndLE"))
			temp.endle = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "NotchPosition"))
			temp.notchposition = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "EndTE"))
			temp.endte = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "fkIndividualID"))
			temp.fkindividualid = atoi(handleNull(argv[i]));

	}

	((std::list<DBOutline> *) outlines)->push_front(temp);

	return 0;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the Points
// table.  It populates a DBPoint and then adds it to the
// list<DBPoint> passed as the first argument.
//
template <class DATABASE_IMAGE_TYPE>
int Database<DATABASE_IMAGE_TYPE>::callbackPoints(void *points, int argc, char **argv, char **azColName) {
	int i;
	DBPoint temp;

	for(i = 0; i < argc; i++) {

		if(! strcmp(azColName[i], "ID"))
			temp.id = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "XCoordinate"))
			temp.xcoordinate = atof(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "YCoordinate"))
			temp.ycoordinate = atof(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "fkOutlineID"))
			temp.fkoutlineid = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "OrderID"))
			temp.orderid = atoi(handleNull(argv[i]));

	}

	((std::list<DBPoint> *) points)->push_back(temp);

	return 0;
}


// *****************************************************************************
//
// This function is called for every row returned in a query on the Thumbnails
// table.  It populates a DBThumbnail and then adds it to the
// list<DBThumbnail> passed as the first argument.
//
template <class DATABASE_IMAGE_TYPE>
int Database<DATABASE_IMAGE_TYPE>::callbackThumbnails(void *thumbnails, int argc, char **argv, char **azColName) {
	int i;
	DBThumbnail temp;

	for(i = 0; i < argc; i++) {

		if(! strcmp(azColName[i], "ID"))
			temp.id = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Rows"))
			temp.rows = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Pixmap"))
			temp.pixmap = stripEscape(string( handleNull(argv[i]) ));

		else if(! strcmp(azColName[i], "fkImageID"))
			temp.fkimageid = atoi(handleNull(argv[i]));

	}

	((std::list<DBThumbnail> *) thumbnails)->push_front(temp);

	return 0;
}

// *****************************************************************************
//
// Set synchronous mode.  0 = OFF, 1 = NORMAL, 2 = FULL.  Default is FULL.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::setSyncMode(int mode) {
	
	stringstream sql;

	sql << "PRAGMA synchronous = " << mode << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Begin transaction.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::beginTransaction() {
	
	stringstream sql;

	sql << "BEGIN TRANSACTION;";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Commit transaction.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::commitTransaction() {
	
	stringstream sql;

	sql << "COMMIT TRANSACTION;";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// This returns all the DamageCategory rows as a list of DBDamageCategory 
// structs.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectAllDamageCategories(std::list<DBDamageCategory> *damagecategories) {

	rc = sqlite3_exec(db, "SELECT * FROM DamageCategories;", callbackDamageCategories, damagecategories, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, "SELECT * FROM DamageCategories;");
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Returns DBDamageCategory of damage category with Name in 
// DamageCategories table.
//
template <class DATABASE_IMAGE_TYPE> 
DBDamageCategory Database<DATABASE_IMAGE_TYPE>::selectDamageCategoryByName(std::string name) {
	
	DBDamageCategory dc;

	std::list<DBDamageCategory> damagecategories = std::list<DBDamageCategory>();
	stringstream sql;

	sql << "SELECT * FROM DamageCategories WHERE Name = '" << name << "';";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackDamageCategories, &damagecategories, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}

	if(! damagecategories.empty())
		dc = damagecategories.front();
	else {
		dc.name = "NONE";
		dc.id = -1;
	}


	return dc;
}

// *****************************************************************************
//
// Returns DBDamageCategory of damage category with id in 
// DamageCategories table.
//
template <class DATABASE_IMAGE_TYPE> 
DBDamageCategory Database<DATABASE_IMAGE_TYPE>::selectDamageCategoryByID(int id) {
	
	DBDamageCategory dc;

	std::list<DBDamageCategory> damagecategories = std::list<DBDamageCategory>();
	stringstream sql;

	sql << "SELECT * FROM DamageCategories WHERE ID = '" << id << "';";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackDamageCategories, &damagecategories, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}

	if(! damagecategories.empty())
		dc = damagecategories.front();
	else {
		dc.name = "NONE";
		dc.id = -1;
	}


	return dc;
}


// *****************************************************************************
//
// This returns all the Individuals rows as a list of DBIndividual structs.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectAllIndividuals(std::list<DBIndividual> *individuals) {

	rc = sqlite3_exec(db, "SELECT * FROM Individuals;", Database<DATABASE_IMAGE_TYPE>::callbackIndividuals, individuals, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, "SELECT * FROM Individuals;");
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Returns DBIndividual of individual with ID in Individuals table.
//
template <class DATABASE_IMAGE_TYPE>
DBIndividual Database<DATABASE_IMAGE_TYPE>::selectIndividualByID(int id) {
	
	std::list<DBIndividual> individuals = std::list<DBIndividual>();

	stringstream sql;

	sql << "SELECT * FROM Individuals WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), Database<DATABASE_IMAGE_TYPE>::callbackIndividuals, &individuals, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}

	return individuals.front();
}

// *****************************************************************************
//
// Returns DBIndividual of individual with given name in Individuals 
// table.
//
template <class DATABASE_IMAGE_TYPE>
DBIndividual Database<DATABASE_IMAGE_TYPE>::selectIndividualByName(std::string name) {
	
	std::list<DBIndividual> individuals = std::list<DBIndividual>();

	stringstream sql;

	sql << "SELECT * FROM Individuals WHERE Name = '" << name << "';";

	rc = sqlite3_exec(db, sql.str().c_str(), Database<DATABASE_IMAGE_TYPE>::callbackIndividuals, &individuals, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}

	return (! individuals.empty() ) ? individuals.front() : NULL;
}

// *****************************************************************************
//
// Returns DBIndividual of individual with the given FkDamageCategory
// value in Individuals table.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectIndividualsByFkDamageCategoryID(std::list<DBIndividual> *individuals, int fkdamagecategoryid) {

	stringstream sql;

	sql << "SELECT * FROM Individuals WHERE fkDamageCategoryID = " << fkdamagecategoryid << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackIndividuals, &individuals, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// This returns all the DBInfo rows as a list of DBInfo structs.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectAllDBInfo(std::list<DBInfo> *dbinfo) {

	rc = sqlite3_exec(db, "SELECT * FROM DBInfo;", callbackDBInfo, dbinfo, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Populates given list<DBImageModification> with all rows from 
// ImageModifications table.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectAllImageModifications(std::list<DBImageModification> *imagemodifications) {

	rc = sqlite3_exec(db, "SELECT * FROM ImageModifications;", callbackImageModifications, imagemodifications, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Populates given list<DBImageModification> with all rows from 
// ImageModifications table where fkImageID equals the given int.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectImageModificationsByFkImageID(std::list<DBImageModification> *imagemodifications, int fkimageid) {
	
	stringstream sql;

	sql << "SELECT * FROM ImageModifications WHERE fkImageID = " << fkimageid << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackImageModifications, imagemodifications, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Populates given list<DBImage> with all rows from Images table.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectAllImages(std::list<DBImage> *images) {

	rc = sqlite3_exec(db, "SELECT * FROM Images;", callbackImages, images, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Populates given list<DBImage> with all rows from Images table where
// the fkIndividualID equals the given int.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectImagesByFkIndividualID(std::list<DBImage> *images, int fkindividualid) {
	


	stringstream sql;

	sql << "SELECT * FROM Images WHERE fkIndividualID = " << fkindividualid << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackImages, images, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}


// *****************************************************************************
//
// Returns DBImage of row with given fkIndividualID
//
template <class DATABASE_IMAGE_TYPE>
DBImage Database<DATABASE_IMAGE_TYPE>::selectImageByFkIndividualID(int fkindividualid) {
	

	DBImage img;
	
	std::list<DBImage> images = std::list<DBImage>();

	selectImagesByFkIndividualID(&images, fkindividualid);
	
	if(! images.empty())
		img = images.front();
	else {
		img.id = -1;
		img.fkindividualid = -1;
		img.imagefilename = "NONE";
		img.dateofsighting = "NONE";
		img.rollandframe = "NONE";
		img.locationcode = "NONE";
		img.shortdescription = "NONE";
	}

	return img;
}

// *****************************************************************************
//
// This returns all the Outlines rows as a list of DBOutline structs.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectAllOutlines(std::list<DBOutline> *outlines) {

	rc = sqlite3_exec(db, "SELECT * FROM Outlines;", callbackOutlines, outlines, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Returns DBOutline from Outlines table where the fkIndividualID equals
// the given int.
//
template <class DATABASE_IMAGE_TYPE>
DBOutline Database<DATABASE_IMAGE_TYPE>::selectOutlineByFkIndividualID(int fkindividualid) {
	
	DBOutline outline;
	std::list<DBOutline> outlines = std::list<DBOutline>();
	
	stringstream sql;

	sql << "SELECT * FROM Outlines WHERE fkIndividualID = " << fkindividualid << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackOutlines, &outlines, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}

	if(! outlines.empty() ) {
		outline = outlines.front();
	} else {
		outline.id = -1;
		outline.tipposition = -1;
		outline.beginle = -1;
		outline.endle = -1;
		outline.endte = -1;
		outline.notchposition = -1;
		outline.fkindividualid = -1;
	}

	return outline;
}


// *****************************************************************************
//
// This returns all the Points rows as a list of DBPoint structs.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectAllPoints(std::list<DBPoint> *points) {

	rc = sqlite3_exec(db, "SELECT * FROM Points ORDER BY OrderID;", callbackPoints, points, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Populates given list<DBPoint> with all rows from Points table where
// the fkOutlineID equals the given int.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectPointsByFkOutlineID(std::list<DBPoint> *points, int fkoutlineid) {

	stringstream sql;

	sql << "SELECT * FROM Points WHERE fkOutlineID = " << fkoutlineid << " ORDER BY OrderID;";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackPoints, points, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}


// *****************************************************************************
//
// This returns all the Thumbnails rows as a list of DBThumbnail structs.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectAllThumbnails(std::list<DBThumbnail> *thumbnails) {

	rc = sqlite3_exec(db, "SELECT * FROM Thumbnails;", callbackThumbnails, thumbnails, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// This returns all the Thumbnails rows as a list of DBThumbnail structs.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::selectThumbnailsByFkImageID(std::list<DBThumbnail> *thumbnails, int fkimageid) {

	stringstream sql;

	sql << "SELECT * FROM Thumbnails WHERE fkImageID = " << fkimageid << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackThumbnails, thumbnails, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// This returns all the Thumbnail as DBThumbnail struct.
//
template <class DATABASE_IMAGE_TYPE>
DBThumbnail Database<DATABASE_IMAGE_TYPE>::selectThumbnailByFkImageID(int fkimageid) {
	
	DBThumbnail thumbnail;

	std::list<DBThumbnail> thumbnails = std::list<DBThumbnail>();

	selectThumbnailsByFkImageID(&thumbnails, fkimageid);

	if(! thumbnails.empty() )
		thumbnail = thumbnails.front();
	else {
		thumbnail.id = -1;
		thumbnail.rows = 0;
	}

	return thumbnail;
}

// *****************************************************************************
//
// Inserts Individual into Individuals table.  id needs to be unique.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::insertIndividual(DBIndividual *individual) {
	
	stringstream sql;
	
	sql << "INSERT INTO Individuals (ID, IDCode, Name, fkDamageCategoryID) VALUES ";
	sql << "(" << individual->id << ", ";
	sql << "'" << escapeString(individual->idcode) << "', ";
	sql << "'" << escapeString(individual->name) << "', ";
	sql << individual->fkdamagecategoryid << "); ";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Inserts DamageCategory into DamageCategories table.  Ignores id as
// this is autoincremented in the database.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::insertDamageCategory(DBDamageCategory *damagecategory) {

	stringstream sql;

	sql << "INSERT INTO DamageCategories (ID, Name) VALUES ";
	sql << "(NULL, ";
	sql << "'" << escapeString(damagecategory->name) << "');";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Inserts DBPoint into Points table
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::insertPoint(DBPoint *point) {

	stringstream sql;
	
	// Not including the attributes might improve the time required to parse the SQL statement
	// and over many inserts, improve speed.  But if the table layout is changed, the order
	// of the values will have to change.
	// sql << "INSERT INTO Points (ID, XCoordinate, YCoordinate, fkOutlineID, OrderID) VALUES ";
	
	sql << "INSERT INTO Points VALUES ";
	sql << "(NULL, ";
	sql << point->xcoordinate << ", ";
	sql << point->ycoordinate << ", ";
	sql << point->fkoutlineid << ", ";
	sql << point->orderid << "); ";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Inserts DBInfo into DBInfo table
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::insertDBInfo(DBInfo *dbinfo) {

	stringstream sql;

	sql << "INSERT INTO DBInfo (Key, Value) VALUES ";
	sql << "('" << escapeString(dbinfo->key) << "', ";
	sql << "'" << escapeString(dbinfo->value) << "'); ";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Inserts DBOutline into Outlines table
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::insertOutline(DBOutline *outline) {

	stringstream sql;

	sql << "INSERT INTO Outlines (ID, TipPosition, BeginLE, EndLE, NotchPosition, EndTE, fkIndividualID) VALUES ";
	sql << "(NULL, ";
	sql << outline->tipposition << ", ";
	sql << outline->beginle << ", ";
	sql << outline->endle << ", ";
	sql << outline->notchposition << ", ";
	sql << outline->endte << ", ";
	sql << outline->fkindividualid << ");";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Inserts DBImage into Images table
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::insertImage(DBImage *image) {

	stringstream sql;

	sql << "INSERT INTO Images (ID, ImageFilename, DateOfSighting, RollAndFrame, LocationCode, ShortDescription, fkIndividualID) VALUES ";
	sql << "(NULL, ";
	sql << "'" << escapeString(image->imagefilename) << "', ";
	sql << "'" << escapeString(image->dateofsighting) << "', ";
	sql << "'" << escapeString(image->rollandframe) << "', ";
	sql << "'" << escapeString(image->locationcode) << "', ";
	sql << "'" << escapeString(image->shortdescription) << "', ";
	sql << image->fkindividualid << ");";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Inserts DBImageModification into ImageModifications table
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::insertImageModification(DBImageModification *imagemod) {

	stringstream sql;

	sql << "INSERT INTO ImageModifications (ID, Operation, Value1, Value2, Value3, Value4, OrderID, fkIndividualID) VALUES ";
	sql << "(NULL, ";
	sql << imagemod->id << ", ";
	sql << imagemod->operation << ", ";
	sql << imagemod->value1 << ", ";
	sql << imagemod->value2 << ", ";
	sql << imagemod->value3 << ", ";
	sql << imagemod->value4 << ", ";
	sql << imagemod->orderid << ", ";
	sql << imagemod->fkimageid << ");";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Inserts DBThumbnail into Thumbnails table
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::insertThumbnail(DBThumbnail *thumbnail) {

	stringstream sql;

	sql << "INSERT INTO Thumbnails (ID, Rows, Pixmap, fkImageID) VALUES ";
	sql << "(NULL, ";
	sql << thumbnail->rows << ", ";
	sql << "'" << escapeString(thumbnail->pixmap) << "', ";
	sql << thumbnail->fkimageid << ");";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Inserts list of DBPoint's into Points table
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::insertPoints(std::list<DBPoint>* points) {

	beginTransaction();

	while(! points->empty() ) {
		DBPoint point;
		point = points->front();
		points->pop_front();

		insertPoint(&point);
	}

	commitTransaction();
}

// *****************************************************************************
//
// Inserts list of DBImageModification's into ImageModifications table
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::insertImageModifications(std::list<DBImageModification>* imagemods) {

	while(! imagemods->empty() ) {
		DBImageModification imagemod;
		imagemod = imagemods->front();
		imagemods->pop_front();

		insertImageModification(&imagemod);
	}
}

// *****************************************************************************
//
// Updates outline in Outlines table  
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::updateOutline(DBOutline *outline) {

	stringstream sql;

	sql << "UPDATE Outlines SET ";
	sql << "TipPosition = " << outline->tipposition << ", ";
	sql << "BeginLE = " << outline->beginle << ", ";
	sql << "EndLE = " << outline->endle << ", ";
	sql << "NotchPosition = " << outline->notchposition << ", ";
	sql << "fkIndividualID = " << outline->fkindividualid << " ";
	sql << "WHERE ID = " << outline->id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Updates row in DamageCategories table using given DBDamageCategory struct.
// Uses ID field for identifying row.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::updateDamageCategory(DBDamageCategory *damagecategory) {

	stringstream sql;

	sql << "UPDATE DamageCategories SET ";
	sql << "Name = '" << escapeString(damagecategory->name) << "' ";
	sql << "WHERE ID = " << damagecategory->id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Updates row in Individuals table using given DBIndividual struct.  Uses ID
// field for identifying row.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::updateIndividual(DBIndividual *individual) {

	stringstream sql;

	sql << "UPDATE Individuals SET ";
	sql << "IDCode = '" << escapeString(individual->idcode) << "', ";
	sql << "Name = '" << escapeString(individual->name) << "', ";
	sql << "fkDamageCategoryID = " << individual->fkdamagecategoryid << " ";
	sql << "WHERE ID = " << individual->id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Updates row in Images table using given DBImage struct.  Uses ID
// field for identifying row.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::updateImage(DBImage *image) {

	stringstream sql;

	sql << "UPDATE Images SET ";
	sql << "ImageFilename = '" << escapeString(image->imagefilename) << "', ";
	sql << "DateOfSighting = '" << escapeString(image->dateofsighting) << "', ";
	sql << "RollAndFrame = '" << escapeString(image->rollandframe) << "', ";
	sql << "LocationCode = '" << escapeString(image->locationcode) << "', ";
	sql << "ShortDescription = '" << escapeString(image->shortdescription) << "' ";
	sql << "fkIndividualID = " << image->fkindividualid << " ";
	sql << "WHERE ID = " << image->id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Updates row in ImageModifications table using given DBImageModification
// struct.  Uses ID field for identifying row.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::updateImageModification(DBImageModification *imagemod) {

	stringstream sql;

	sql << "UPDATE ImageModifications SET ";
	sql << "Operation = " << imagemod->operation << ", ";
	sql << "Value1 = " << imagemod->value1 << ", ";
	sql << "Value2 = " << imagemod->value2 << ", ";
	sql << "Value3 = " << imagemod->value3 << ", ";
	sql << "Value4 = " << imagemod->value4 << ", ";
	sql << "OrderID = " << imagemod->orderid << ", ";
	sql << "fkImageID = " << imagemod->fkimageid << " ";
	sql << "WHERE ID = " << imagemod->id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Updates row in Thumbnails table using given DBThumbnail
// struct.  Uses ID field for identifying row.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::updateThumbnail(DBThumbnail *thumbnail) {

	stringstream sql;

	sql << "UPDATE Thumbnails SET ";
	sql << "Rows = " << thumbnail->rows << ", ";
	sql << "Pixmap = '" << escapeString(thumbnail->pixmap) << "', ";
	sql << "fkImageID = " << thumbnail->fkimageid << " ";
	sql << "WHERE ID = " << thumbnail->id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Updates row in DBInfo table using given DBInfo
// struct.  Uses ID field for identifying row.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::updateDBInfo(DBInfo *dbinfo) {

	stringstream sql;

	sql << "UPDATE DBInfo SET ";
	sql << "Value = '" << escapeString(dbinfo->value) << "', ";
	sql << "WHERE Key = '" <<escapeString(dbinfo->key) << "';";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}



// *****************************************************************************
//
// Deletes set of points from Points table using fkOutlineID  
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::deletePoints(int fkOutlineID) {

	stringstream sql;

	sql << "DELETE FROM Points ";
	sql << "WHERE fkOutlineID = " << fkOutlineID << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}



// *****************************************************************************
//
// Delete outline from Outlines table using fkIndividualID  
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::deleteOutlineByFkIndividualID(int fkIndividualID) {

	stringstream sql;

	sql << "DELETE FROM Outlines ";
	sql << "WHERE fkIndividualID = " << fkIndividualID << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Delete outline from Outlines table using id  
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::deleteOutlineByID(int id) {

	stringstream sql;

	sql << "DELETE FROM Outlines ";
	sql << "WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}


// *****************************************************************************
//
// Delete individual from Individuals table using id  
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::deleteIndividual(int id) {

	stringstream sql;

	sql << "DELETE FROM Individuals ";
	sql << "WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Delete damagecategory from DamageCategories table using id  
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::deleteDamageCategory(int id) {

	stringstream sql;

	sql << "DELETE FROM DamageCategories ";
	sql << "WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Delete image from Images table using id  
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::deleteImage(int id) {

	stringstream sql;

	sql << "DELETE FROM Images ";
	sql << "WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}


// *****************************************************************************
//
// Delete imagemod from ImageModifications table using id  
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::deleteImageModification(int id) {

	stringstream sql;

	sql << "DELETE FROM ImageModifications ";
	sql << "WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Delete thumbnail from Thumbnails table using id  
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::deleteThumbnail(int id) {

	stringstream sql;

	sql << "DELETE FROM Thumbnails ";
	sql << "WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Delete thumbnail from Thumbnails table using fkImageID  
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::deleteThumbnailByFkImageID(int id) {

	stringstream sql;

	sql << "DELETE FROM Thumbnails ";
	sql << "WHERE fkImageID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}


// *****************************************************************************
//
// Inserts DatabaseFin
//
template <class DATABASE_IMAGE_TYPE>
unsigned long Database<DATABASE_IMAGE_TYPE>::add(DatabaseFin<DATABASE_IMAGE_TYPE> *fin) {

	DBIndividual individual;
	DBImage image;
	DBOutline outline;
	DBThumbnail thumbnail;
	Outline *finOutline;
	std::list<DBPoint> *points = new std::list<DBPoint>();
	FloatContour *fc;
	int i, numPoints, pos;
	std::string pixTemp = "";
	DBPoint point;
	string shortFilename;

	//***054 - assume that the image filename contains path
	// information which must be stripped BEFORE saving fin
	shortFilename = fin->mImageFilename;
	pos = shortFilename.find_last_of(PATH_SLASH);
	if (pos >= 0) {
		shortFilename = shortFilename.substr(pos+1);
	}
	fin->mImageFilename = shortFilename;
	
	individual.id = generateUniqueInt();
	individual.idcode = fin->getID();
	individual.name = fin->getName();
	individual.fkdamagecategoryid = ( selectDamageCategoryByName( fin->getDamage() ) ).id;
	insertIndividual(&individual);

	finOutline = fin->mFinOutline;
	outline.beginle = finOutline->getFeaturePoint(LE_BEGIN);
	outline.endle = finOutline->getFeaturePoint(LE_END);
	outline.notchposition = finOutline->getFeaturePoint(NOTCH);
	outline.tipposition = finOutline->getFeaturePoint(TIP);
	outline.endte = finOutline->getFeaturePoint(POINT_OF_INFLECTION);
	outline.fkindividualid = individual.id;
	insertOutline(&outline);

	// we do this as we don't know what the outline id is
	outline = selectOutlineByFkIndividualID(individual.id);	
	
	numPoints = finOutline->length();
	fc = finOutline->getFloatContour();
	for(i = 0; i < numPoints; i++) {
		point.xcoordinate = (*fc)[i].x;
		point.ycoordinate = (*fc)[i].y;
		point.orderid = i;
		point.fkoutlineid = outline.id;

		points->push_back(point);
	}
	insertPoints(points);

	image.dateofsighting = fin->getDate();
	image.imagefilename = fin->mImageFilename;
	image.locationcode = fin->getLocation();
	image.rollandframe = fin->getRoll();
	image.shortdescription = fin->getShortDescription();
	image.fkindividualid = individual.id;
	insertImage(&image);

	// do this as we don't know the image id
	image = selectImageByFkIndividualID(individual.id);

	for (i = 0; i < fin->mThumbnailRows; i++) {;
		pixTemp += fin->mThumbnailPixmap[i];
		pixTemp += "\n";
	}
	thumbnail.rows = fin->mThumbnailRows;
	thumbnail.pixmap = pixTemp;
	thumbnail.fkimageid = image.id;
	insertThumbnail(&thumbnail);

	loadLists(); // reload and re-sort lists
	
	delete points;

	return individual.id; // mDataPos field will be used to map to id in db for individuals
}

// *****************************************************************************
//
// Updates DatabaseFin
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::update(DatabaseFin<DATABASE_IMAGE_TYPE> *fin) {
	DBIndividual individual;
	DBImage image;
	DBOutline outline;
	DBThumbnail thumbnail;
	Outline *finOutline;
	std::list<DBPoint> points = std::list<DBPoint>();
	FloatContour *fc;
	int i, numPoints;
	std::string pixTemp;
	
	individual.id = fin->mDataPos; // mapping Individuals id to mDataPos
	individual.idcode = fin->getID();
	individual.name = fin->getName();
	individual.fkdamagecategoryid = ( selectDamageCategoryByName(fin->getDamage()) ).id;
	updateIndividual(&individual);

	finOutline = fin->mFinOutline;
	// we do this as we don't know what the outline id is
	outline = selectOutlineByFkIndividualID(individual.id);
	outline.beginle = finOutline->getFeaturePoint(LE_BEGIN);
	outline.endle = finOutline->getFeaturePoint(LE_END);
	outline.notchposition = finOutline->getFeaturePoint(NOTCH);
	outline.tipposition = finOutline->getFeaturePoint(TIP);
	outline.endte = finOutline->getFeaturePoint(POINT_OF_INFLECTION);
	outline.fkindividualid = individual.id;
	updateOutline(&outline);	
	
	numPoints = finOutline->length();
	fc = finOutline->getFloatContour();
	for(i = 0; i < numPoints; i++) {
		DBPoint point;

		point.xcoordinate = (*fc)[i].x;
		point.ycoordinate = (*fc)[i].y;
		point.orderid = i;
		point.fkoutlineid = outline.id;

		points.push_back(point);
	}
	deletePoints(outline.id);
	insertPoints(&points);

	// query db as we don't know the image id
	image = selectImageByFkIndividualID(individual.id);
	image.dateofsighting = fin->getDate();
	image.imagefilename = fin->mImageFilename;;
	image.locationcode = fin->getLocation();
	image.rollandframe = fin->getRoll();
	image.shortdescription = fin->getShortDescription();
	image.fkindividualid = individual.id;
	updateImage(&image);
	
	// query db as we don't know the thumbnail id
	thumbnail = selectThumbnailByFkImageID(image.id);
	thumbnail.rows = fin->mThumbnailRows;
	for (i = 0; i < fin->mThumbnailRows; i++) {
		pixTemp += fin->mThumbnailPixmap[i];
		pixTemp += "\n";
	}
	thumbnail.pixmap = pixTemp;

	updateThumbnail(&thumbnail);

	loadLists(); // reload and re-sort lists.
}

// *****************************************************************************
//
// Returns complete DatabaseFin. mDataPos field will be used to map to id in 
// db for individuals
//
template <class DATABASE_IMAGE_TYPE>
DatabaseFin<DATABASE_IMAGE_TYPE>* Database<DATABASE_IMAGE_TYPE>::getFin(int id) {

	DBIndividual individual;
	DBImage image;
	DBOutline outline;
	DBThumbnail thumbnail;
	DBDamageCategory damagecategory;
	Outline *finOutline;
	std::list<DBPoint> *points = new std::list<DBPoint>();
	FloatContour *fc = new FloatContour();
	DatabaseFin<DATABASE_IMAGE_TYPE> *fin;
	string imageFilename;
	int pos;
	
	individual = selectIndividualByID(id);
	damagecategory = selectDamageCategoryByID(individual.fkdamagecategoryid);
	image = selectImageByFkIndividualID(id);
	outline = selectOutlineByFkIndividualID(id);
	thumbnail = selectThumbnailByFkImageID(image.id);
	selectPointsByFkOutlineID(points, outline.id);

	// Although having both of these blocks of code seems uesless, this ensures that
	// the given path contains only the image filename.  If the given path contains
	// more, then the first code block will strip it down.

	// Strip path info
	imageFilename = image.imagefilename;
	pos = imageFilename.find_last_of(PATH_SLASH);
	if (pos >= 0) {
		imageFilename = imageFilename.substr(pos+1);
	}
	image.imagefilename = imageFilename;

	// Add current path info
	imageFilename = gOptions->mCurrentSurveyArea; //***1.85
	imageFilename += PATH_SLASH;
	imageFilename += "catalog";
	imageFilename += PATH_SLASH;
	imageFilename += image.imagefilename;
	image.imagefilename = imageFilename;
	
	// assumes list is returned as FIFO (queue)... should be due to use of ORDER BY OrderID
	while(! points->empty() ) {
		DBPoint point = points->front();
		points->pop_front();
		fc->addPoint(point.xcoordinate, point.ycoordinate);
	}

	finOutline = new Outline(fc);
	finOutline->setFeaturePoint(LE_BEGIN, outline.beginle);
	finOutline->setFeaturePoint(LE_END, outline.endle);
	finOutline->setFeaturePoint(NOTCH, outline.notchposition);
	finOutline->setFeaturePoint(TIP, outline.tipposition);
	finOutline->setFeaturePoint(POINT_OF_INFLECTION, outline.endte);
	finOutline->setLEAngle(0.0,true);

	
	// Based on thumbnail size in DatabaseFin
	char **pixmap = new char*[thumbnail.rows];
	std::string pixmapString = thumbnail.pixmap;
	std::string buffer;

	for(int i = 0; i < thumbnail.rows; i++) {
		int j = 0;
		
		j = pixmapString.find('\n');

		if(j != string::npos && j <= pixmapString.size()) {
			buffer = pixmapString.substr(0, j);
			pixmapString = pixmapString.substr(j + 1);
		} else {
			buffer = pixmapString;
		}

		pixmap[i] = new char[buffer.length() + 1];
		strcpy(pixmap[i], buffer.c_str());
	}

	fin = new DatabaseFin<DATABASE_IMAGE_TYPE>(image.imagefilename, 
		finOutline, 
		individual.idcode, 
		individual.name,
		image.dateofsighting,
		image.rollandframe,
		image.locationcode,
		damagecategory.name,
		image.shortdescription,
		individual.id, // mDataPos field will be used to map to id in db for individuals
		pixmap,
		thumbnail.rows
		);
	
	delete fc; 	//***1.0LK - fc is COPIED in Outline so we must delete it here
	delete points;
	
	return fin;
}

// *****************************************************************************
//
// Returns all fins from database.
//
template <class DATABASE_IMAGE_TYPE>
std::list< DatabaseFin<DATABASE_IMAGE_TYPE>* >* Database<DATABASE_IMAGE_TYPE>::getAllFins(void) {

	std::list<DBIndividual> *individuals = new std::list<DBIndividual>();
	std::list< DatabaseFin<DATABASE_IMAGE_TYPE>* > *fins = new std::list< DatabaseFin<DATABASE_IMAGE_TYPE>* >();
	DBIndividual individual;
	
	beginTransaction();
	selectAllIndividuals(individuals);

	while(! individuals->empty() ) {
		individual = individuals->front();
		individuals->pop_front();
		fins->push_back( getFin(individual.id) );
	}

	commitTransaction();
	delete individuals;

	return fins;
}

// *****************************************************************************
//
// Returns fin from database.  pos refers to position within one of the sort
// lists.
//
template <class DATABASE_IMAGE_TYPE>
DatabaseFin<DATABASE_IMAGE_TYPE>* Database<DATABASE_IMAGE_TYPE>::getItem(unsigned pos) {

	std::vector<std::string> *it;

	if (mCurrentSort == DB_SORT_NAME)
		//it = mNameList.begin();
		it = &mNameList;

	else if (mCurrentSort == DB_SORT_ID)
		//it = mIDList.begin();
		it = &mIDList;

	else if (mCurrentSort == DB_SORT_DATE)
		//it = mDateList.begin();
		it = &mDateList;

	else if (mCurrentSort == DB_SORT_ROLL)
		//it = mRollList.begin();
		it = &mRollList;

	else if (mCurrentSort == DB_SORT_LOCATION)
		//it = mLocationList.begin();
		it = &mLocationList;

	else if (mCurrentSort == DB_SORT_DAMAGE)
		//it = mDamageList.begin();
		it = &mDamageList;

	else if (mCurrentSort == DB_SORT_DESCRIPTION)
		//it = mDescriptionList.begin();
		it = &mDescriptionList;

	else  // it's not a valid sort type
		return NULL;

	return getItem(pos, it);
}

// *****************************************************************************
//
// Retrieve the fin with given name from the database.
//
template <class DATABASE_IMAGE_TYPE>
DatabaseFin<DATABASE_IMAGE_TYPE>* Database<DATABASE_IMAGE_TYPE>::getItemByName(std::string name) {

	DBIndividual individual;
	DatabaseFin<DATABASE_IMAGE_TYPE> *fin;
	
	individual = selectIndividualByName(name);
	fin = getFin(individual.id);

	return fin;
}

// *****************************************************************************
//
// Delete fin from database
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::Delete(DatabaseFin<DATABASE_IMAGE_TYPE> *fin) {
	
	DBOutline outline;
	DBImage image;
	int id;

	// mDataPos field will be used to map to id in db for individuals
	id = fin->mDataPos;
	
	outline = selectOutlineByFkIndividualID(id);
	image = selectImageByFkIndividualID(id);

	this->deletePoints(outline.id);
	this->deleteOutlineByFkIndividualID(id);
	this->deleteThumbnailByFkImageID(image.id);
	this->deleteImage(image.id);
	this->deleteIndividual(id);

	loadLists(); // reload and re-sort lists
}

// *****************************************************************************
//
// Sorts the lists
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::sortLists() {
	std::sort(mNameList.begin(),mNameList.end());
  	std::sort(mIDList.begin(),mIDList.end());
  	std::sort(mDateList.begin(),mDateList.end());
  	std::sort(mRollList.begin(),mRollList.end());
  	std::sort(mLocationList.begin(),mLocationList.end());
  	std::sort(mDamageList.begin(),mDamageList.end());
  	std::sort(mDescriptionList.begin(),mDescriptionList.end());
}

//*******************************************************************
//
// Returns number of fins in lists.
//
template <class DATABASE_IMAGE_TYPE>
unsigned Database<DATABASE_IMAGE_TYPE>::size() const
{
	return mNameList.size();
}

//*******************************************************************
//
// Returns number of fins in AbsoluteOffset list.
//
template <class DATABASE_IMAGE_TYPE>
unsigned Database<DATABASE_IMAGE_TYPE>::sizeAbsolute() const
{
	return mAbsoluteOffset.size();
}

//*******************************************************************
//
template <class DATABASE_IMAGE_TYPE>
bool Database<DATABASE_IMAGE_TYPE>::isEmpty() const
{
	return (mNameList.size() == 0);
}

//*******************************************************************
//
template <class DATABASE_IMAGE_TYPE>
typename Database<DATABASE_IMAGE_TYPE>::db_status_t Database<DATABASE_IMAGE_TYPE>::status() const
{
	return mDBStatus;
}

//*******************************************************************
//***1.85
//
template <class DATABASE_IMAGE_TYPE>
db_sort_t Database<DATABASE_IMAGE_TYPE>::currentSort()
{
	return mCurrentSort;
}

//*******************************************************************
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::sort(db_sort_t sortBy)
{
	mCurrentSort = sortBy;
}

//*******************************************************************
//***1.85 - returns position of id in mIDList as currently sorted
//
template <class DATABASE_IMAGE_TYPE>
int Database<DATABASE_IMAGE_TYPE>::getIDListPosit(std::string id)
{
	// NOTE: this list in the database contains strings, but the strings
	// are made up of two parts ("the Id" and "id" of the fin 
	// in the Invidiuals table.)
	
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
// Looks up row id in AbsoluteOffset list and then uses getFin(int)
// to retrieve that fin from the database.
//
template <class DATABASE_IMAGE_TYPE>
DatabaseFin<DATABASE_IMAGE_TYPE>* Database<DATABASE_IMAGE_TYPE>::getItemAbsolute(unsigned pos) {

	if (pos > this->mAbsoluteOffset.size())
	       throw BoundsError();

	if (mAbsoluteOffset[pos] == -1)
		return NULL;               // this is a HOLE, a previously deleted fin

	DatabaseFin<DATABASE_IMAGE_TYPE> *fin = getFin(this->mAbsoluteOffset[pos]);

	return fin;
}

//*******************************************************************
//
// Returns item from list at given position
//
template <class DATABASE_IMAGE_TYPE>
string Database<DATABASE_IMAGE_TYPE>::getItemEntryFromList(db_sort_t whichList, unsigned pos) {	

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
template <class DATABASE_IMAGE_TYPE>
int Database<DATABASE_IMAGE_TYPE>::getItemListPosFromOffset(db_sort_t whichList, string item) {

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

//*******************************************************************
//
// Given a list and position in that list, returns fin from database.
//
template <class DATABASE_IMAGE_TYPE>
DatabaseFin<DATABASE_IMAGE_TYPE>* Database<DATABASE_IMAGE_TYPE>::getItem(unsigned pos, std::vector<std::string> *theList) {	

	istrstream inStream((*theList)[pos].c_str());

	std::string prev, cur;
	
	// we'll assume the last token in the stream is the position in
	// the file
	while (inStream >> cur)
		prev = cur;

	return getFin(atoi(prev.c_str()));
}

//*******************************************************************
//
// Rebuilds the lists from the database and sorts them.
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::loadLists() {

	std::list< DatabaseFin<DATABASE_IMAGE_TYPE>* > *fins;

	mNameList.clear();
	mIDList.clear();
	mDateList.clear();
	mRollList.clear();
	mLocationList.clear();
	mDamageList.clear();
	mDescriptionList.clear();
	mAbsoluteOffset.clear();

	fins = getAllFins();

	while(! fins->empty() ) {
		DatabaseFin<DATABASE_IMAGE_TYPE>* fin = fins->front();
		fins->pop_front();

		stringstream temp;
		
		if("NULL" != fin->getName())
			temp << fin->getName() << " " << fin->mDataPos;
		else
			temp << "NONE " << fin->mDataPos;
		mNameList.push_back(temp.str());
		temp.str("");

		if("NULL" != fin->getID())
			temp << fin->getID() << " " << fin->mDataPos;
		else
			temp << "NONE " << fin->mDataPos;
		mIDList.push_back(temp.str());
		temp.str("");

		if("NULL" != fin->getDate())
			temp << fin->getDate() << " " << fin->mDataPos;
		else
			temp << "NONE " << fin->mDataPos;
		mDateList.push_back(temp.str());
		temp.str("");

		if("NULL" != fin->getRoll())
			temp << fin->getRoll() << " " << fin->mDataPos;
		else
			temp << "NONE " << fin->mDataPos;
		mRollList.push_back(temp.str());
		temp.str("");

		if("NULL" != fin->getLocation())
			temp << fin->getLocation() << " " << fin->mDataPos;
		else
			temp << "NONE " << fin->mDataPos;
		mLocationList.push_back(temp.str());
		temp.str("");

		if("NULL" != fin->getDamage())
			temp << fin->getDamage() << " " << fin->mDataPos;
		else
			temp << "NONE " << fin->mDataPos;
		mDamageList.push_back(temp.str());
		temp.str("");

		if("NULL" != fin->getShortDescription())
			temp << fin->getShortDescription() << " " << fin->mDataPos;
		else
			temp << "NONE " << fin->mDataPos;
		mDescriptionList.push_back(temp.str());
		temp.str("");
		
		mAbsoluteOffset.push_back(fin->mDataPos);
	}

	delete fins;

	sortLists();
}


// *****************************************************************************
//
// Opens the database file
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::opendb(const char *filename) {

	rc = sqlite3_open(filename, &db);

	if( rc ) {
		cout << "Can't open database: " << sqlite3_errmsg(db) << endl;
		closedb();
		dbOpen = false;
		mDBStatus = errorLoading;
	} else {
		dbOpen = true;
	}
}

// *****************************************************************************
//
// Closes the database file
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::closedb() {

	if(dbOpen)
		sqlite3_close(db);

	dbOpen = false;
}

// *****************************************************************************
//
// Create empty db
//
template <class DATABASE_IMAGE_TYPE>
void Database<DATABASE_IMAGE_TYPE>::createEmptyDatabase(Options *o) {
	
	int schemeId = o->mCurrentDefaultCatalogScheme;
	stringstream sql;
	DBDamageCategory cat;

	// SQL CREATE TABLE statements... might be better off defined in the header as a constant..

	sql << "CREATE TABLE DamageCategories (";
	sql << "ID INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sql << "Name TEXT ";
	sql << ");" << endl;

	sql << "CREATE TABLE Individuals ( ";
	sql << "ID INTEGER PRIMARY KEY, ";
	sql << "IDCode TEXT, ";
	sql << "Name TEXT, ";
	sql << "fkDamageCategoryID INTEGER ";
	sql << ");" << endl;
	
	sql << "CREATE TABLE Images ( ";
	sql << "ID INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sql << "fkIndividualID INTEGER, ";
	sql << "ImageFilename TEXT, ";
	sql << "DateOfSighting TEXT, ";
	sql << "RollAndFrame TEXT, ";
	sql << "LocationCode TEXT, ";
	sql << "ShortDescription TEXT ";
	sql << ");" << endl;

	sql << "CREATE TABLE ImageModifications ( ";
	sql << "ID INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sql << "Operation INTEGER, ";
	sql << "Value1 INTEGER, ";
	sql << "Value2 INTEGER, ";
	sql << "Value3 INTEGER, ";
	sql << "Value4 INTEGER, ";
	sql << "OrderID INTEGER, ";
	sql << "fkImageID INTEGER ";
	sql << ");" << endl;

	sql << "CREATE TABLE Thumbnails ( ";
	sql << "ID INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sql << "fkImageID INTEGER, ";
	sql << "Rows INTEGER, ";
	sql << "Pixmap TEXT ";
	sql << ");" << endl;
	
	sql << "CREATE TABLE Outlines ( ";
	sql << "ID INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sql << "TipPosition INTEGER, ";
	sql << "BeginLE INTEGER, ";
	sql << "EndLE INTEGER, ";
	sql << "NotchPosition INTEGER, ";
	sql << "EndTE INTEGER, ";
	sql << "fkIndividualID INTEGER ";
	sql << ");" << endl;
	
	sql << "CREATE TABLE Points ( ";
	sql << "ID INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sql << "XCoordinate REAL, ";
	sql << "YCoordinate REAL, ";
	sql << "fkOutlineID INTEGER, ";
	sql << "OrderID INTEGER ";
	sql << ");" << endl;

	sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
	
	// TODO: enter code to populate DBInfo
	
	for (int i = 0; i < o->mDefinedCatalogCategoryNamesMax[schemeId]; i++) {
		cat.name = o->mDefinedCatalogCategoryName[schemeId][i];
		insertDamageCategory(&cat);		
	}

}

// *****************************************************************************
//
// Returns db filename
//
template <class DATABASE_IMAGE_TYPE>
string Database<DATABASE_IMAGE_TYPE>::getFilename() {

	return mFilename;
}

// *****************************************************************************
//
// Constructor
//
template <class DATABASE_IMAGE_TYPE>
Database<DATABASE_IMAGE_TYPE>::Database(Options *o, bool createEmptyDB) {
	
	std::list<DBDamageCategory> *damagecategories = new std::list<DBDamageCategory>();
	DBDamageCategory damagecategory;
	int i = 0;
	
	zErrMsg = 0;
	dbOpen = false;
	mFilename = std::string(o->mDatabaseFileName);
	mCurrentSort = DB_SORT_NAME;
	
	cout << "Database: loading db... " << o->mDatabaseFileName << endl;

	if (mFilename == "NONE") {
		cout << "\nNO File Name specified for existing database!\n";
		mDBStatus = fileNotFound;
		return;
	}

	opendb(mFilename.c_str());
	if(! dbOpen) {
		mDBStatus = errorLoading;
		return;
	}

	this->setSyncMode(0); // set sync mode to OFF.  Significant improvement in write speed.

	if(createEmptyDB)
		createEmptyDatabase(o);
	
	// get damage categories
	this->selectAllDamageCategories(damagecategories);
	o->mCatCategoryNamesMax = damagecategories->size();
	o->mCatCategoryName.resize( o->mCatCategoryNamesMax );
	
	while(! damagecategories->empty()) {
		damagecategory = damagecategories->front();
		damagecategories->pop_front();
		o->mCatCategoryName[i++] = damagecategory.name;
	}
	
	loadLists();
	mDBStatus = loaded;
	delete damagecategories;

}


// *****************************************************************************
//
// Deconstructor
//
template <class DATABASE_IMAGE_TYPE>
Database<DATABASE_IMAGE_TYPE>::~Database() {
	
	closedb();

}

//*******************************************************************
//
template <class DATABASE_IMAGE_TYPE>
bool Database<DATABASE_IMAGE_TYPE>::closeStream()
{
	closedb();
	return true;
}


//*******************************************************************
//
template <class DATABASE_IMAGE_TYPE>
bool Database<DATABASE_IMAGE_TYPE>::openStream()
{
	opendb(mFilename.c_str());
	return true;
}

#endif // DATABASE_H