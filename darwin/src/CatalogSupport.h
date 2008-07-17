// CatalogSupport.h

#ifndef DATABASE_SUPPORT_H
#define DATABASE_SUPPORT_H

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
Database * openDatabase(MainWindow *mainWin, string filename);
void copyFins(Database* from, Database *to);
db_opentype_t databaseOpenType(string filePath);
Database* convertDatabase(Options* o, string sourceFilename);
Database* duplicateDatabase(Options* o, Database* sourceDatabase, string targetFilename);

bool backupCatalog(Database *db);
bool restoreCatalogFrom(Options *o, std::string filename);
bool exportCatalogTo(Database *db, Options *o, std::string filename);
bool importCatalogFrom(Options *o, std::string filename);

<<<<<<< .mine
bool createArchive (Database *db, string filename); // creates zipped catalog

=======
DatabaseFin<ColorImage>* openFinz(string filename);
void saveFinz(DatabaseFin<ColorImage>* fin, string filename);

>>>>>>> .r36
#endif