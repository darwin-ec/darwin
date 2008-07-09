// DatabaseSupport.cxx

#include "DatabaseSupport.h"

/*
	Tries to instantiate a database object using the filename
	in the Options object.

  */
Database * openDatabase(Options *o, bool create)
{
	Database* db;
	fstream testFile(o->mDatabaseFileName.c_str(), ios::in | ios::binary);

	if(!create && !testFile)
	{
			DummyDatabase* dummy = new DummyDatabase(o, false);
			dummy->setStatus(Database::fileNotFound);
			db = dummy;
	} else if(create || SQLiteDatabase::isType(o->mDatabaseFileName))
		db = new SQLiteDatabase(o, create);
	else if(OldDatabase::isType(o->mDatabaseFileName))
		db = new OldDatabase(o, false);

	if(!testFile)
	{
		testFile.close();
		testFile.clear();
	}

	return db;
}

db_opentype_t databaseOpenType(std::string filePath)
{
	db_opentype_t type;

	fstream testFile(filePath.c_str(), ios::in | ios::binary);

	if(!testFile)
			type = cannotOpen;

	else if(SQLiteDatabase::isType(filePath))
		type = canOpen;

	else if(OldDatabase::isType(filePath))
		type = convert;

	if(!testFile)
	{
		testFile.close();
		testFile.clear();
	}

	return type;
}

/*
	Used for converting between databases
*/
void copyDatabaseContents(Database* from, Database *to)
{
	const unsigned size = from->size();

	for(unsigned i = 0; i < size; i++)
	{
		DatabaseFin<ColorImage>* fin = from->getItem(i);
		cout << i << endl;
		if(fin != NULL)
			to->add(fin);
		else
			cout << "null" << endl;
	}
}
