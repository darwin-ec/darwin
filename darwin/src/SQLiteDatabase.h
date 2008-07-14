//*******************************************************************
//   file: SQLiteDatabase.h
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

#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H
#include "Database.h"
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
typedef struct { int id; int orderid; std::string name; } DBDamageCategory; 
typedef struct { int id; int fkindividualid; std::string imagefilename; std::string dateofsighting; std::string rollandframe; std::string locationcode; std::string shortdescription; } DBImage;
typedef struct { int id; int rows; std::string pixmap; int fkimageid; } DBThumbnail;
typedef struct { int id; int tipposition; int beginle; int endle; int notchposition; int endte; int fkindividualid; } DBOutline;
typedef struct { int id; float xcoordinate; float ycoordinate; int fkoutlineid; int orderid; } DBPoint;
typedef struct { std::string key; std::string value; } DBInfo;
typedef struct { int id; int operation; int value1; int value2; int value3; int value4; int orderid; int fkimageid; } DBImageModification;


//******************************************************************
// Function Definitions
//******************************************************************

class SQLiteDatabase : public Database {
public:

	SQLiteDatabase(Options *o, bool createEmptyDB); //***054 - since catagories may change

	~SQLiteDatabase();
	
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
		
	virtual void createEmptyDatabase(Options *o); //***054

	virtual unsigned long add(DatabaseFin<ColorImage>* data); //***1.85 - return type changed
	void update(DatabaseFin<ColorImage> *fin);
	DatabaseFin<ColorImage>* getFin(int id);
	std::list< DatabaseFin<ColorImage>* >* getAllFins(void);
	virtual void Delete(DatabaseFin<ColorImage> *Fin); //***002DB

	virtual DatabaseFin<ColorImage>* getItemAbsolute(unsigned pos); //***1.3

	virtual DatabaseFin<ColorImage>* getItem(unsigned pos);
	// virtual DatabaseFin<ColorImage>* getItemByName(std::string name);  

	virtual bool openStream();
	virtual bool closeStream();

	static bool isType(std::string filePath);

protected:
	virtual DatabaseFin<ColorImage>* getItem(unsigned pos, std::vector<std::string> *theList);

private:
	sqlite3 *db;
	char *zErrMsg;
	int rc;

	static char* handleNull(char *);
	int generateUniqueInt();
	static string escapeString(string);
	static string stripEscape(string);

	int lastInsertedRowID();
	void setSyncMode(int mode);
	void beginTransaction();
	void commitTransaction();

	void selectAllDamageCategories(std::list<DBDamageCategory> *);
	DBDamageCategory selectDamageCategoryByName(std::string name);
	DBDamageCategory selectDamageCategoryByID(int id);
	void selectAllIndividuals(std::list<DBIndividual> *);
	DBIndividual selectIndividualByID(int);
	int selectIndividualByName(std::string name);
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

	int insertIndividual(DBIndividual *);
	int insertDamageCategory(DBDamageCategory *);
	int insertPoint(DBPoint *);
	void insertDBInfo(DBInfo *);
	int insertOutline(DBOutline *);
	int insertImage(DBImage *);
	int insertImageModification(DBImageModification *);
	int insertThumbnail(DBThumbnail *);
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
	void addFinToLists(int, string, string, string, string,
							   string, string, string);
	void deleteFinFromLists(int);
	void addFinToLists(DatabaseFin<ColorImage>*);
	void sortLists();
	string nullToNone(string);
	void SQLiteDatabase::deleteEntry(std::vector<string>*, int);
	int SQLiteDatabase::listEntryToID(string);


};

#endif // SQLITEDATABASE_H