// DatabaseSupport.h

#ifndef DATABASE_SUPPORT_H
#define DATABASE_SUPPORT_H

#include "Database.h"
#include "SQLiteDatabase.h"
#include "OldDatabase.h"
#include "DummyDatabase.h"

typedef enum {
			cannotOpen = 0,
				canOpen,
				convert
	} db_opentype_t;

Database * openDatabase(Options *o, bool create);
void copyDatabaseContents(Database* from, Database *to);
db_opentype_t databaseOpenType(std::string filePath);

#endif