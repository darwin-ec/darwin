// CatalogSupport.h

#ifndef CATALOG_SUPPORT_H
#define CATALOG_SUPPORT_H

#include "Database.h"
#include "SQLiteDatabase.h"
#include "OldDatabase.h"
#include "DummyDatabase.h"

typedef enum {
			cannotOpen = 0,
				canOpen,
				convert
	} db_opentype_t;

Database* openDatabase(Options *o, bool create);
void copyFins(Database* from, Database *to);
db_opentype_t databaseOpenType(string filePath);
Database* convertDatabase(Options* o, string sourceFilename);
Database* duplicateDatabase(Options* o, Database* sourceDatabase, string targetFilename);

#endif