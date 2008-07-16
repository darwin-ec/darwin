// DatabaseSupport.cxx

#include "CatalogSupport.h"

#include <set>
#include <cctype>

using namespace std;

// The woCaseLesThan::operator() is used to compare strings without 
// distinguishing case differences.  Specifically, it is used in 
// backupDatabaseTo() to build set of uninque image filenames associated 
// with a particular *.db file being archived.

class woCaseLessThan {
public:
	bool operator() (const string &s1, const string &s2) const
	{ 
		string first(s1), second(s2);
		int i;
		for (i=0; i < first.length(); i++)
			first[i] = tolower(first[i]);
		for (i=0; i < second.length(); i++)
			second[i] = tolower(second[i]);
		return (first < second);
	}
};

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

	if (create || SQLiteDatabase::isType(o->mDatabaseFileName))
		db = new SQLiteDatabase(o, create);
	else if(OldDatabase::isType(o->mDatabaseFileName))
		db = openDatabase(NULL,o->mDatabaseFileName);
	else
	{
		DummyDatabase* dummy = new DummyDatabase(o, false);
		dummy->setStatus(Database::fileNotFound);
		db = dummy;
	} 

	return db;
}

//
// This version is called to open and return (possibly converting)
// an EXISTING database.
//
Database * openDatabase(MainWindow *mainWin, string filename)
{
	Database *db = NULL;
	
	Options o;	
	o.mDatabaseFileName = filename;
	o.mDarwinHome = getenv("DARWINHOME");

	db_opentype_t opentype = databaseOpenType(filename);
	
	switch (opentype)
	{
	case cannotOpen:
			// notify user of problem
			g_print("Could not open selected database file!\n");
		break;
	case convert:
		{
			// convert the OldDatabase
			DBConvertDialog *cdlg = new DBConvertDialog(
					mainWin,
					filename,
					&db); // this is set to return converted/opened SQLDatabase
			cdlg->run_and_respond();
		}
		break;
	case canOpen:
			// open the SQLite database
			o.mDatabaseFileName = filename;
			o.mDarwinHome = getenv("DARWINHOME");
			db = new SQLiteDatabase(&o, false);
		break;
	default:
		// nothing required, shold NEVER end up here
		break;
	}

	if (NULL == db)
	{
		// either open database failed or we ended up in the default case above
		o.mDatabaseFileName = filename;
		o.mDarwinHome = getenv("DARWINHOME");
		db = new DummyDatabase(&o, false);
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
	db_opentype_t type = cannotOpen;

	if(SQLiteDatabase::isType(filePath))
		type = canOpen;

	else if(OldDatabase::isType(filePath))
		type = convert;

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
		if(fin != NULL) {
			to->add(fin);
			cout << "added" << endl;
			delete fin;
		}
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

/*
 * This makes a backup in the "DARWINHOME/backups" folder with
 * a name coprised of the SurveyArea name, catalog Database
 * filename, the current date, and a sequence number, if necessary.
 *
 * It assumes ONLY that the database (db) is open and complete.
 *
 */

bool backupCatalog(Database *db)
{
	cout << "\nCreating BACKUP of Database ...\n  " 
	     << db->getFilename() << endl;

	cout << "\nCollecting list of files comprising database ... \n\n  Please Wait." << endl;

	// build list of image names referenced from within database

	set<string,woCaseLessThan> imageNames;
	set<string,woCaseLessThan>::iterator it, oit;
	DatabaseFin<ColorImage> *fin;
	ImageFile<ColorImage> img;
	string catalogPath = db->getFilename();;
	catalogPath = catalogPath.substr(0,catalogPath.rfind(PATH_SLASH)+1);
	int i;


	int limit = db->sizeAbsolute();

	for (i = 0; i < limit; i++)
	{
		fin = db->getItemAbsolute(i);

		if (NULL == fin)
			continue; // found a hole (previously deleted fin) in the database
		
		// if modified image filename not already in set, then add it

		it = imageNames.find(fin->mImageFilename);
		if (it == imageNames.end())
		{
			imageNames.insert(fin->mImageFilename);

			if (img.loadPNGcommentsOnly(fin->mImageFilename))
			{
				// if original image filename is not in set, then add it

				string origImageName = catalogPath + img.mOriginalImageFilename;
				oit = imageNames.find(origImageName);
				if (oit == imageNames.end())
					imageNames.insert(origImageName);

				// make sure fields are empty for next image file read

				img.mImageMods.clear();
				img.mOriginalImageFilename = "";
			}
		}

		delete fin; // make sure to return storage
	}

	// create backup filename .. should allow user to choose this

	string shortName = db->getFilename();
	shortName = shortName.substr(1+shortName.rfind(PATH_SLASH));
	shortName = shortName.substr(0,shortName.rfind(".db")); // just root name of DB file

	string shortArea = db->getFilename();
	shortArea = shortArea.substr(0,shortArea.rfind(PATH_SLASH));
	shortArea = shortArea.substr(0,shortArea.rfind(PATH_SLASH));
	shortArea = shortArea.substr(1+shortArea.rfind(PATH_SLASH)); // just the area name

	shortName = shortArea + "_" + shortName; // merge two name parts

	// now append the date & time
	shortName = shortName.substr(0,shortName.rfind(".db"));
	time_t ltime;
	time( &ltime );
	tm *today = localtime( &ltime );
	char buffer[128];
	strftime(buffer,128,"_%b_%d_%Y",today); // month name, day & year
	shortName += buffer;
	// and append ".zip"
	shortName += ".zip";
	
	string backupPath, backupFilename, fileList, command;

	backupPath = getenv("DARWINHOME");
	backupPath = backupPath 
		+ PATH_SLASH 
		+ "backups" 
		+ PATH_SLASH;
	backupFilename = backupPath + shortName;

	// find out if archive already exists, and if so, append a suffix to backup
	ifstream testFile(backupFilename.c_str());
	if (! testFile.fail())
	{
		testFile.close();
		backupFilename = backupFilename.substr(0,backupFilename.rfind(".zip")); // strip ".zip"
		char suffix[16];
		int i=2;
		bool done=false;
		while (! done)
		{
			sprintf(suffix,"[%d]",i);
			testFile.open((backupFilename + suffix + ".zip").c_str());
			if (testFile.fail())
				done = true;
			else
			{
				testFile.close();
				i++;
			}
		}
		backupFilename = backupFilename + suffix + ".zip";
	}
	
	// put quotes around name
	backupFilename = "\"" + backupFilename + "\"";

	fileList = "\"";
	fileList += backupPath + "filesToArchive.txt\"";

	cout << "\nBACKUP filename is ...\n\n  " << backupFilename << endl;

	// create the archive file using 7z compression program (Windows)

	command += "7z a -tzip ";
	command += backupFilename + " @" + fileList;
			
	db->closeStream();

	ofstream archiveListFile;
	archiveListFile.open((backupPath + "filesToArchive.txt").c_str());
	if (! archiveListFile.fail())
	{	
		archiveListFile <<  fileList << endl;
		archiveListFile << "\"" << db->getFilename() << "\"" << endl;
		for (it = imageNames.begin(); it != imageNames.end(); ++it)
			archiveListFile << "\"" << (*it) << "\"" << endl;

		archiveListFile.close();

		system(command.c_str()); // start the archive process using 7-zip

		//***1.982 - remove "filesToArchive.txt"
		command = "del " + fileList;
		system(command.c_str());
	}

	if (! db->openStream())
	{
		ErrorDialog *err = new ErrorDialog("Database failed to reopen.");
		err->show();
		return false;
	}

	return true;
}

/*
 * Restores from "DARWINHOME/backups".  Assumes the survey area in the 
 * Options is the target.
 */

bool restoreCatalogFrom(Options *o, std::string filename)
{
	return false;
}
/*
	string cmd("7z.exe x");
	cmd += " '" + filename + "' ";
	cmd += " '" + o->mCurrentSurveyArea;
	cmd += PATH_SLASH;
	cmd += "catalog" + "' ";
	system(cmd.c_str());


	return false;
	backupFilename = "\"";
		backupFilename += gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));
		backupFilename += "\"";

		cout << "\nRestoring database from BACKUP location ...\n\n  " << backupFilename << endl;
		cout << "\n\nPlease wait." << endl;

		// extract the file list with the path information
		// this is necessary because the 7-zip program does NOT support relative
		// path information in a simple fashion when wanting to archive only
		// selected files within a folder

		command = "7z x -aoa "; // extract and overwrite existing file
		command += backupFilename + " filesToArchive.txt";
			
		system(command.c_str()); // extract the file list file

		infile.open("filesToArchive.txt");
		getline(infile,line); // first is location of original backup
		getline(infile,line); // this is the database file
		infile.close();

		system("del filesToArchive.txt"); // remove temporary file list file

		restorePath = line.substr(0,line.rfind(PATH_SLASH)+1) + "\""; //***1.981a re-append quote

		//***1.982 - break out parts and inform user what is about to happen

		line2 = line.substr(1,line.length()-2); // strip quotes
		pos = line2.rfind(PATH_SLASH);
		restoreDBName = line2.substr(pos+1); // save database name for restore
		line2 = line2.substr(0,pos); // strip database filename
		pos = line2.rfind(PATH_SLASH);
		line2 = line2.substr(0,pos); // strip "catalog"
		pos = line2.rfind(PATH_SLASH);
		restoreSurveyArea = line2.substr(pos+1); // save survey area name for restore
		line2 = line2.substr(0,pos); // strip survey area name
		pos = line2.rfind(PATH_SLASH);
		line2 = line2.substr(0,pos); // strip "surveyAreas"
		restoreHome = line2; // save DARWINHOME for restore

		{	// this BLOCK is simply to allow local vars for file / folder search & repair

			// determine whether the file folder structure is intact for this
			// database location ... ensure existance of surveyAreas\<restoreSurveyArea> and
			// its subfolders ...
			//    catalog, tracedFins, matchQueues, matchQResults, sightings
			struct _finddata_t c_file;
			long hFile;
			string path;

			// Find surveyArea\<restoreSurveyArea>
			path = restoreHome + PATH_SLASH + "surveyAreas";
			if( (hFile = _findfirst(path.c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"SurveyAreas\" folder ...\n" );
				_mkdir(path.c_str());
			}
			_findclose( hFile );
			path += PATH_SLASH + restoreSurveyArea;
			if( (hFile = _findfirst(path.c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"%s\" SurveyArea subfolder...\n", 
					restoreSurveyArea.c_str());
				_mkdir(path.c_str());
			}
			_findclose( hFile );
			// find subfolders and, if missing, fix them ...
			path += PATH_SLASH;
			if( (hFile = _findfirst((path+"catalog").c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"catalog\" folder ...\n" );
				_mkdir((path+"catalog").c_str());
			}
			_findclose( hFile );
			if( (hFile = _findfirst((path+"tracedFins").c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"tracedFins\" folder ...\n" );
				_mkdir((path+"tracedFins").c_str());
			}
			_findclose( hFile );
			if( (hFile = _findfirst((path+"matchQueues").c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"matchQueues\" folder ...\n" );
				_mkdir((path+"matchQueues").c_str());
			}
			_findclose( hFile );
			if( (hFile = _findfirst((path+"matchQResults").c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"matchQResults\" folder ...\n" );
				_mkdir((path+"matchQResults").c_str());
			}
			_findclose( hFile );
			if( (hFile = _findfirst((path+"sightings").c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"sightings\" folder ...\n" );
				_mkdir((path+"sightings").c_str());
			}
		}
		//***1.982 - end user notification

		// all is OK, so now proceed ...
		
		// extract database file from the archive

		// note: restorePath and backupFilename are already QUOTED

		command = "7z x -aoa -o"; // put in proper folder and force overwriting
		command += restorePath + " ";
		command += backupFilename + " *.db";

		dlg->mMainWin->mDatabase->closeStream(); // close database in order to overwrite it

		cout << command << endl;

		system(command.c_str()); // start extraction process

		// extract images referenced from database file

		//command = "7z x -aos -o\""; // extract into proper folder without overwriting
		//***1.981a - extra quote removed
		command = "7z x -aos -o"; // extract into proper folder without overwriting
		command += restorePath + " -x!filesToArchive.txt "; // but don't extract file list
		command += " -x!*.db "; // and don't extract database again
		command += backupFilename;

		cout << command << endl;

		system(command.c_str()); // start extraction process

		//***1.982 - update current Survey Area indicator in global Options
		gOptions->mCurrentSurveyArea = 
			restoreHome + PATH_SLASH + "surveyAreas" + PATH_SLASH + restoreSurveyArea;
}
*/

bool exportCatalogTo(Options *o, std::string filename)
{
	return false;
}


bool importCatalogFrom(Options *o, std::string filename)
{
	return false;
}

DatabaseFin<ColorImage>* openFinz(string filename)
{
	string tempdir("");
	tempdir += "%TEMP%";
	tempdir += PATH_SLASH;
	tempdir += "darwin";
	tempdir += PATH_SLASH;
	tempdir += filename;

	// extract finz to temp dir
	string cmd("7z.exe x");
	cmd += " '" + filename + "' ";
	cmd += " '" + tempdir + "' ";
	system(cmd.c_str());

	Options o = Options();
	o.mDatabaseFileName = tempdir + PATH_SLASH + "database.db";

	SQLiteDatabase db = SQLiteDatabase(&o, false);

	DatabaseFin<ColorImage>* fin = db.getItem(0); // first and only fin
	
	// I really hope C++ managed code works how I think it does...

	return fin;
}

void saveFinz(DatabaseFin<ColorImage>* fin, string filename)
{
	string baseFilename = extractBasename(filename);

	string tempdir("");
	tempdir += "%TEMP%";
	tempdir += PATH_SLASH;
	tempdir += "darwin";
	tempdir += PATH_SLASH;
	tempdir += baseFilename;

	// create target dir
	string cmd("mkdir ");
	cmd += " '" + tempdir + "' ";
	system(cmd.c_str());

	// copy modified image over into catalog folder
	string srcFilename = getenv("DARWINHOME");
	srcFilename += PATH_SLASH;
	srcFilename += "catalog";
	srcFilename += PATH_SLASH;
	srcFilename += fin->mImageFilename;

	string targetFilename = tempdir + PATH_SLASH + fin->mImageFilename;

	cmd = "copy '";
	cmd += " '" + srcFilename + "' ";
	cmd += targetFilename + "'";
	system(cmd.c_str());

	// copy original
	srcFilename = getenv("DARWINHOME");
	srcFilename += PATH_SLASH;
	srcFilename += "catalog";
	srcFilename += PATH_SLASH;
	srcFilename += fin->mOriginalImageFilename;

	targetFilename = tempdir + PATH_SLASH + fin->mOriginalImageFilename;

	cmd = "copy '";
	cmd += " '" + srcFilename + "' ";
	cmd += targetFilename + "'";
	system(cmd.c_str());

	Options o = Options();
	o.mCatCategoryNamesMax = 1;
	o.mCatCategoryName.resize( o.mCatCategoryNamesMax );
	o.mCatCategoryName[0] = fin->mDamageCategory;
	o.mDatabaseFileName = tempdir + PATH_SLASH + "database.db";

	SQLiteDatabase db = SQLiteDatabase(&o, true);

	db.add(fin);

	db.closeStream();

	// compress contents
	srcFilename = tempdir + PATH_SLASH + "*.*";
	targetFilename = filename;

	cmd = "7z.exe a";
	cmd += " '" + targetFilename + "' ";
	cmd += " '" + srcFilename + "' ";
	system(cmd.c_str());
}
