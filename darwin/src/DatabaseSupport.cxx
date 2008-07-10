// DatabaseSupport.cxx

#include "DatabaseSupport.h"

/*
 *	Tries to instantiate a database object using the filename
 *	in the Options object.  As of right now, all new databases
 *  will be SQLite databases but old databases can be opened.
 *  
 *  In the future, if MySQL support is added, the filename
 *  could also be a URL and this method would instantiate
 *  the database type appropriately.
 *
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

/*
 * Used to determine if the given file refers to:
 *     1) A file we can't read
 *     2) An unsupported database type that should
 *        be converted to a supported type
 *     3) A supported database
 */
db_opentype_t databaseOpenType(string filePath)
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
 * Used for converting between databases. Relies on the add()
 * method of the target database, and thus, most likely will
 * not create new damage categories.
 * ONLY WORKS FOR DATABASES WITH THE SAME DAMAGE CATEGORIES!!!
*/
void copyFins(Database* from, Database *to)
{
	cout << "copy fins called" << endl;
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

/*
 * Used to convert unsupported types to new SQLite databases.
 * IGNORES THE FILENAMES IN THE OPTIONS! (Decision made
 * as part of a move towards de-coupling the options and
 * the database filename and categories.
 *
 * Returns the new converted database.
 */
Database* convertDatabase(Options* o, string sourceFilename)
{
	/* We're moving the source file and then 
		creating the target with the old name
		*/
	string targetFilename = sourceFilename;

	// "file.db" -> "file.olddb"
	int pos = sourceFilename.rfind(".");
	string newFilenameOfSourceDatabase = sourceFilename.substr(0, pos);
	newFilenameOfSourceDatabase += ".olddb";

	// move "file.db" "file.olddb" -- renames source file
	string mvCmd("move \"");
	mvCmd += sourceFilename;
	mvCmd += "\" \"";
	mvCmd += newFilenameOfSourceDatabase;
	mvCmd += "\"";
	system(mvCmd.c_str());

	/*
	 * open the old database -- needs to be opened first
	 * due to the side effects (category names) set in 
	 * the options.
	 */
	o->mDatabaseFileName = newFilenameOfSourceDatabase;
	Database* sourceDatabase = openDatabase(o, false);
	
	Database* targetDatabase = duplicateDatabase(o, sourceDatabase, targetFilename);
	
	sourceDatabase->closeStream();
	delete sourceDatabase;

	return targetDatabase;
}

/*
 * Used to duplicate the contents of a database to a new database.
 * IGNORES THE FILENAMES IN THE OPTIONS! (Decision made
 * as part of a move towards de-coupling the options and
 * the database filename and categories.
 *
 * Returns the new converted database.
 * 
 * This probably shouldn't be used directly until types other
 * than the SQLiteDatabase are supported.  The idea here is that
 * a type (SQLite vs MySQL) would be determined by the targetFilename
 * and thus this function is very generic.
 */
Database* duplicateDatabase(Options* o, Database* sourceDatabase, string targetFilename)
{
	o->mDatabaseFileName = targetFilename;
	Database* targetDatabase = openDatabase(o, true);

	copyFins(sourceDatabase, targetDatabase);
	
	return targetDatabase;
}