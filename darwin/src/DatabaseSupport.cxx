// DatabaseSupport.cxx

#include "DatabaseSupport.h"


Database * openDatabase(Options *o, bool create)
{
	SQLiteDatabase *db = new SQLiteDatabase(o,create);

	return db;
}

