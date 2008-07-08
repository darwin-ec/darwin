// DatabaseSupport.h

#ifndef DATABASE_SUPPORT_H
#define DATABASE_SUPPORT_H

#include "Database.h"
#include "SQLiteDatabase.h"
#include "OldDatabase.h"

Database * openDatabase(Options *o, bool create);

#endif