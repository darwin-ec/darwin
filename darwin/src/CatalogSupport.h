// CatalogSupport.h

#ifndef CATALOG_SUPPORT_H
#define CATALOG_SUPPORT_H

#include "interface/ErrorDialog.h"
#include "Database.h"
#include "SQLiteDatabase.h"
#include "OldDatabase.h"
#include "DummyDatabase.h"
#include "interface/MainWindow.h"
#include "interface/DBConvertDialog.h"
#include "utility.h"

typedef enum {
	cannotOpen = 0,
	canOpen,
	convert
} db_opentype_t;

Database* openDatabase(Options *o, bool create);
Database * openDatabase(MainWindow *mainWin, std::string filename);
void copyFins(Database* from, Database *to);
db_opentype_t databaseOpenType(std::string filePath);
Database* convertDatabase(Options* o, std::string sourceFilename);
Database* duplicateDatabase(Options* o, Database* sourceDatabase, std::string targetFilename);

//***2.22 - new function to support multiple data areas OUTSIDE DARWINHOME
//          dataPath should terminate in "darwinPhotoIdData" and may be anywhere
bool dataPathExists (std::string dataPath, bool forceCreate); //***2.22

//***2.22 - new function to see if file exists without opening file
//          as done in utility.h (fileExists(fname)
bool filespecFound(std::string path); //***2.22

//***2.22 - new function to move defualt and sample survey areas data OUTSIDE 
//          DARWINHOME the first time DARWIN runs
void moveAreasAndBackups(bool force); //***2.22

void rebuildFolders(std::string home, std::string area, bool force);
void extractCatalogFiles(std::string backupFilename, std::string toFollder);

bool backupCatalog(Database *db);
bool restoreCatalogFrom(std::string filename,
						std::string restorePath, 
						std::string restoreHome, 
						std::string restoreArea);
bool exportCatalogTo(Database *db, Options *o, std::string filename);
bool importCatalogFrom(std::string backupFilename, 
						std::string restorePath, 
						std::string restoreHome, 
						std::string restoreArea);

bool createArchive (Database *db, std::string filename); // creates zipped catalog
bool continueOverwrite(std::string winLabel, std::string message, std::string fileName);

DatabaseFin<ColorImage>* openFin(std::string filename);
bool saveFin(DatabaseFin<ColorImage>* fin, std::string filename);

DatabaseFin<ColorImage>* openFinz(std::string filename);
bool saveFinz(DatabaseFin<ColorImage>* fin, std::string &filename); //***2.0 - ref param change

std::string saveImages(DatabaseFin<ColorImage>* fin, std::string savefolder, std::string filename);

bool testFileExistsAndPrompt(std::string filename);

void importFin(Database* db, DatabaseFin<ColorImage>* fin);

bool isTracedFinFile(std::string fileName);

#endif