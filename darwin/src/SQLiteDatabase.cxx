
#include "SQLiteDatabase.h"

//#include "SQLiteDatabase.cpp" -- all code already included below -- JHS

// *****************************************************************************
//
// Ensures that the given character array is valid, otherwise returns "NULL".
//
char* SQLiteDatabase::handleNull(char *in) {

	return strcmp(in, "\0") != 0 ? in : "NULL";
}


// *****************************************************************************
//
// Escapes strings
//
string SQLiteDatabase::escapeString(string str) {
	string buffer = str;

	int pos = 0;

	while( (pos = buffer.find("'", pos)) != string::npos && pos <= buffer.size() ) {
		buffer.insert(pos, "'");
		pos += 2;
	}

	return buffer;
}

// *****************************************************************************
//
// Strip escapes
//
string SQLiteDatabase::stripEscape(string str) {
	string buffer = str;

	int pos = 0;
	
	while( (pos = buffer.find("''", pos) ) != string::npos) {
		buffer.erase(++pos, 1);
	}
	
	return buffer;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the Individuals
// table.  It populates a DBIndividual and then adds it to the 
// list<DBIndividual> passed as the first argument.
//
int SQLiteDatabase::callbackIndividuals(void *individuals, int argc, char **argv, char **azColName) {
	
	int i;
	DBIndividual temp;

	for(i = 0; i < argc; i++) {
			
		if(! strcmp(azColName[i], "ID") )
			temp.id = atoi( handleNull(argv[i]) );
		else if(! strcmp(azColName[i], "IDCode") )
			temp.idcode = stripEscape( handleNull(argv[i]) );
		else if(! strcmp(azColName[i], "Name") )
			temp.name = stripEscape( handleNull(argv[i]) );
		else if(! strcmp(azColName[i], "fkDamageCategoryID") )
			temp.fkdamagecategoryid = atoi( handleNull(argv[i]) );
	
	}
	
	((std::list<DBIndividual> *)individuals)->push_front(temp);

	return 0;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the
// DamageCategories table.  It populates a DBDamageCategory and then 
// adds it to the list<DBDamageCategory> passed as the first argument.
//
int SQLiteDatabase::callbackDamageCategories(void *damagecategories, int argc, char **argv, char **azColName) {
	
	
	int i;
	DBDamageCategory temp;

	for(i = 0; i < argc; i++) {
			
		if(! strcmp(azColName[i], "ID") )
			temp.id = atoi( handleNull(argv[i]) );
		else if(! strcmp(azColName[i], "Name") )
			temp.name = stripEscape( handleNull(argv[i]) );
		else if(! strcmp(azColName[i], "OrderID"))
			temp.orderid = atoi(handleNull(argv[i]));

	}
	
	((std::list<DBDamageCategory> *)damagecategories)->push_back(temp);


	return 0;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the
// DBInfo table.  It populates a DBInfo and then adds it to the 
// list<DBInfo> passed as the first argument.
//
int SQLiteDatabase::callbackDBInfo(void *dbinfo, int argc, char **argv, char **azColName) {

		int i;
	DBInfo temp;

	for(i = 0; i < argc; i++) {

		if(! strcmp(azColName[i], "Key"))
			temp.key = stripEscape( handleNull(argv[i]) );
		else if(! strcmp(azColName[i], "Value"))
			temp.value = stripEscape( handleNull(argv[i]) );
		

	}

	((std::list<DBInfo> *)dbinfo)->push_front(temp);

	return 0;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the
// ImageModifications table.  It populates a DBImageModification and   
// then adds it to the list<DBImageModification> passed as the first
// argument.
//
int SQLiteDatabase::callbackImageModifications(void *imagemods, int argc, char **argv, char **azColName) {

	int i;
	DBImageModification temp;

	for(i = 0; i < argc; i++) {

		if(! strcmp(azColName[i], "ID"))
			temp.id = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Operation"))
			temp.operation = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Value1"))
			temp.value1 = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Value2"))
			temp.value2 = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Value3"))
			temp.value3 = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Value4"))
			temp.value4 = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "OrderID"))
			temp.orderid = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "fkImageID"))
			temp.fkimageid = atoi(handleNull(argv[i]));

	}

	((std::list<DBImageModification> *) imagemods)->push_front(temp);

	return 0;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the Images
// table.  It populates a DBImage and then adds it to the
// list<DBImage> passed as the first argument.
//
int SQLiteDatabase::callbackImages(void *images, int argc, char **argv, char **azColName) {

	int i;
	DBImage temp;

	for(i = 0; i < argc; i++) {

		if(! strcmp(azColName[i], "ID"))
			temp.id = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "ImageFilename"))
			temp.imagefilename = stripEscape( handleNull(argv[i]) );

		else if(! strcmp(azColName[i], "DateOfSighting"))
			temp.dateofsighting = stripEscape( handleNull(argv[i]) );

		else if(! strcmp(azColName[i], "RollAndFrame"))
			temp.rollandframe = stripEscape( handleNull(argv[i]) );

		else if(! strcmp(azColName[i], "LocationCode"))
			temp.locationcode = stripEscape( handleNull(argv[i]) );

		else if(! strcmp(azColName[i], "ShortDescription"))
			temp.shortdescription = stripEscape( handleNull(argv[i]) );

		else if(! strcmp(azColName[i], "fkIndividualID"))
			temp.fkindividualid = atoi(handleNull(argv[i]));

	}

	((std::list<DBImage> *) images)->push_front(temp);

	return 0;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the Outlines
// table.  It populates a DBOutline and then adds it to the
// list<DBOutline> passed as the first argument.
//
int SQLiteDatabase::callbackOutlines(void *outlines, int argc, char **argv, char **azColName) {

	int i;
	DBOutline temp;

	for(i = 0; i < argc; i++) {

		if(! strcmp(azColName[i], "ID"))
			temp.id = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "TipPosition"))
			temp.tipposition = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "BeginLE"))
			temp.beginle = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "EndLE"))
			temp.endle = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "NotchPosition"))
			temp.notchposition = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "EndTE"))
			temp.endte = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "fkIndividualID"))
			temp.fkindividualid = atoi(handleNull(argv[i]));

	}

	((std::list<DBOutline> *) outlines)->push_front(temp);

	return 0;
}

// *****************************************************************************
//
// This function is called for every row returned in a query on the Points
// table.  It populates a DBPoint and then adds it to the
// list<DBPoint> passed as the first argument.
//
int SQLiteDatabase::callbackPoints(void *points, int argc, char **argv, char **azColName) {
	
	int i;
	DBPoint temp;

	for(i = 0; i < argc; i++) {

		if(! strcmp(azColName[i], "ID"))
			temp.id = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "XCoordinate"))
			temp.xcoordinate = atof(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "YCoordinate"))
			temp.ycoordinate = atof(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "fkOutlineID"))
			temp.fkoutlineid = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "OrderID"))
			temp.orderid = atoi(handleNull(argv[i]));

	}

	((std::list<DBPoint> *) points)->push_back(temp);

	return 0;
}


// *****************************************************************************
//
// This function is called for every row returned in a query on the Thumbnails
// table.  It populates a DBThumbnail and then adds it to the
// list<DBThumbnail> passed as the first argument.
//
int SQLiteDatabase::callbackThumbnails(void *thumbnails, int argc, char **argv, char **azColName) {

	int i;
	DBThumbnail temp;

	for(i = 0; i < argc; i++) {

		if(! strcmp(azColName[i], "ID"))
			temp.id = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Rows"))
			temp.rows = atoi(handleNull(argv[i]));

		else if(! strcmp(azColName[i], "Pixmap"))
			temp.pixmap = stripEscape(string( handleNull(argv[i]) ));

		else if(! strcmp(azColName[i], "fkImageID"))
			temp.fkimageid = atoi(handleNull(argv[i]));

	}

	((std::list<DBThumbnail> *) thumbnails)->push_front(temp);

	return 0;
}


// *****************************************************************************
//
// Returns the ID of the last successfully inserted row.  If the table contains
// an INTEGER PRIMARY KEY column, that value is returned.  Otherwise, it returns
// the value of SQLite's hidden column, ROWID.
//
int SQLiteDatabase::lastInsertedRowID() {
	
	return (int) sqlite3_last_insert_rowid(db);
}

// *****************************************************************************
//
// Set synchronous mode.  0 = OFF, 1 = NORMAL, 2 = FULL.  Default is FULL.
//
void SQLiteDatabase::setSyncMode(int mode) {
	
	stringstream sql;

	sql << "PRAGMA synchronous = " << mode << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}


// *****************************************************************************
//
// Begin transaction.
//
void SQLiteDatabase::beginTransaction() {
	
	stringstream sql;

	sql << "BEGIN TRANSACTION;";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}


// *****************************************************************************
//
// Commit transaction.
//
void SQLiteDatabase::commitTransaction() {
	
	stringstream sql;

	sql << "COMMIT TRANSACTION;";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}


// *****************************************************************************
//
// This returns all the DamageCategory rows as a list of DBDamageCategory 
// structs.
//
void SQLiteDatabase::selectAllDamageCategories(std::list<DBDamageCategory> *damagecategories) {
	
	stringstream sql;

	sql << "SELECT * FROM DamageCategories ORDER BY OrderID;";

	rc = sqlite3_exec(db, sql.str().c_str() , callbackDamageCategories, damagecategories, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, "SELECT * FROM DamageCategories;");
		sqlite3_free(zErrMsg);
	}
}


// *****************************************************************************
//
// Returns DBDamageCategory of damage category with Name in 
// DamageCategories table.
//
DBDamageCategory SQLiteDatabase::selectDamageCategoryByName(std::string name) {

	DBDamageCategory dc;

	std::list<DBDamageCategory> damagecategories = std::list<DBDamageCategory>();
	stringstream sql;

	sql << "SELECT * FROM DamageCategories WHERE Name = '" << name << "';";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackDamageCategories, &damagecategories, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}

	if(! damagecategories.empty())
		dc = damagecategories.front();
	else {
		dc.name = "NONE";
		dc.id = -1;
		dc.orderid = -1;
	}

	return dc;
}


// *****************************************************************************
//
// Returns DBDamageCategory of damage category with id in 
// DamageCategories table.
//
DBDamageCategory SQLiteDatabase::selectDamageCategoryByID(int id) {
	
	DBDamageCategory dc;

	std::list<DBDamageCategory> damagecategories = std::list<DBDamageCategory>();
	stringstream sql;

	sql << "SELECT * FROM DamageCategories WHERE ID = '" << id << "';";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackDamageCategories, &damagecategories, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}

	if(! damagecategories.empty())
		dc = damagecategories.front();
	else {
		dc.name = "NONE";
		dc.id = -1;
		dc.orderid = -1;
	}


	return dc;
}


// *****************************************************************************
//
// This returns all the Individuals rows as a list of DBIndividual structs.
//
void SQLiteDatabase::selectAllIndividuals(std::list<DBIndividual> *individuals) {

	rc = sqlite3_exec(db, "SELECT * FROM Individuals;", SQLiteDatabase::callbackIndividuals, individuals, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, "SELECT * FROM Individuals;");
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Returns DBIndividual of individual with ID in Individuals table.
//
DBIndividual SQLiteDatabase::selectIndividualByID(int id) {
	
	DBIndividual individual;
	
	std::list<DBIndividual> individuals = std::list<DBIndividual>();

	stringstream sql;

	sql << "SELECT * FROM Individuals WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), SQLiteDatabase::callbackIndividuals, &individuals, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}

	// cout << "list empty: " << individuals.empty() << endl;

	if(! individuals.empty())
		individual = individuals.front();
	else {
		individual.id = -1;
		individual.name = "NONE";
		individual.idcode = "NONE";
		individual.fkdamagecategoryid = -1;
	}

	return individual;
}

// *****************************************************************************
//
// This returns all the DBInfo rows as a list of DBInfo structs.
//
void SQLiteDatabase::selectAllDBInfo(std::list<DBInfo> *dbinfo) {
	
	std::string sql = "SELECT * FROM DBInfo;";

	rc = sqlite3_exec(db, sql.c_str(), callbackDBInfo, dbinfo, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Populates given list<DBImageModification> with all rows from 
// ImageModifications table.
//
void SQLiteDatabase::selectAllImageModifications(std::list<DBImageModification> *imagemodifications) {

	std::string sql = "SELECT * FROM ImageModifications;";

	rc = sqlite3_exec(db, sql.c_str(), callbackImageModifications, imagemodifications, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Populates given list<DBImageModification> with all rows from 
// ImageModifications table where fkImageID equals the given int.
//
void SQLiteDatabase::selectImageModificationsByFkImageID(std::list<DBImageModification> *imagemodifications, int fkimageid) {
	
	stringstream sql;

	sql << "SELECT * FROM ImageModifications WHERE fkImageID = " << fkimageid << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackImageModifications, imagemodifications, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Populates given list<DBImage> with all rows from Images table.
//
void SQLiteDatabase::selectAllImages(std::list<DBImage> *images) {

	// cout << "select all img" << endl;
	
	std::string sql = "SELECT * FROM Images;";

	rc = sqlite3_exec(db, sql.c_str(), callbackImages, images, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Populates given list<DBImage> with all rows from Images table where
// the fkIndividualID equals the given int.
//
void SQLiteDatabase::selectImagesByFkIndividualID(std::list<DBImage> *images, int fkindividualid) {
	
	// cout << "select imgs by individ id" << endl;

	stringstream sql;

	sql << "SELECT * FROM Images WHERE fkIndividualID = " << fkindividualid << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackImages, images, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}


// *****************************************************************************
//
// Returns DBImage of row with given fkIndividualID
//
DBImage SQLiteDatabase::selectImageByFkIndividualID(int fkindividualid) {

	// cout << "select img by individ id" << endl;

	DBImage img;
	
	std::list<DBImage> images = std::list<DBImage>();

	selectImagesByFkIndividualID(&images, fkindividualid);
	
	if(! images.empty())
		img = images.front();
	else {
		img.id = -1;
		img.fkindividualid = -1;
		img.imagefilename = "NONE";
		img.dateofsighting = "NONE";
		img.rollandframe = "NONE";
		img.locationcode = "NONE";
		img.shortdescription = "NONE";
	}

	return img;
}

// *****************************************************************************
//
// This returns all the Outlines rows as a list of DBOutline structs.
//
void SQLiteDatabase::selectAllOutlines(std::list<DBOutline> *outlines) {

	// cout << "select all outlines" << endl;

	std::string sql = "SELECT * FROM Outlines;";

	rc = sqlite3_exec(db, sql.c_str(), callbackOutlines, outlines, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Returns DBOutline from Outlines table where the fkIndividualID equals
// the given int.
//
DBOutline SQLiteDatabase::selectOutlineByFkIndividualID(int fkindividualid) {

	// cout << "select outline by individ id" << endl;
	
	DBOutline outline;
	std::list<DBOutline> outlines = std::list<DBOutline>();
	
	stringstream sql;

	sql << "SELECT * FROM Outlines WHERE fkIndividualID = " << fkindividualid << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackOutlines, &outlines, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}

	if(! outlines.empty() ) {
		outline = outlines.front();
	} else {
		outline.id = -1;
		outline.tipposition = -1;
		outline.beginle = -1;
		outline.endle = -1;
		outline.endte = -1;
		outline.notchposition = -1;
		outline.fkindividualid = -1;
	}

	return outline;
}

// *****************************************************************************
//
// Populates given list<DBPoint> with all rows from Points table where
// the fkOutlineID equals the given int.
//
void SQLiteDatabase::selectPointsByFkOutlineID(std::list<DBPoint> *points, int fkoutlineid) {

	stringstream sql;

	sql << "SELECT * FROM Points WHERE fkOutlineID = " << fkoutlineid << " ORDER BY OrderID;";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackPoints, points, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}


// *****************************************************************************
//
// This returns all the Thumbnails rows as a list of DBThumbnail structs.
//
void SQLiteDatabase::selectAllThumbnails(std::list<DBThumbnail> *thumbnails) {
	
	std::string sql = "SELECT * FROM Thumbnails;";

	rc = sqlite3_exec(db, sql.c_str(), callbackThumbnails, thumbnails, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// This returns all the Thumbnails rows as a list of DBThumbnail structs.
//
void SQLiteDatabase::selectThumbnailsByFkImageID(std::list<DBThumbnail> *thumbnails, int fkimageid) {

	stringstream sql;

	sql << "SELECT * FROM Thumbnails WHERE fkImageID = " << fkimageid << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), callbackThumbnails, thumbnails, &zErrMsg);

	if( rc!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s %s\n", zErrMsg, sql.str().c_str());
		sqlite3_free(zErrMsg);
	}
}

// *****************************************************************************
//
// Selects a single Thumbnail.
//
DBThumbnail SQLiteDatabase::selectThumbnailByFkImageID(int fkimageid) {
	
	DBThumbnail thumbnail;

	std::list<DBThumbnail> thumbnails = std::list<DBThumbnail>();

	selectThumbnailsByFkImageID(&thumbnails, fkimageid);

	if(! thumbnails.empty() )
		thumbnail = thumbnails.front();
	else {
		thumbnail.id = -1;
		thumbnail.rows = 0;
	}

	return thumbnail;
}

// *****************************************************************************
//
// Inserts Individual into Individuals table.  id needs to be unique.
//
int SQLiteDatabase::insertIndividual(DBIndividual *individual) {
	
	stringstream sql;
	
	sql << "INSERT INTO Individuals (ID, IDCode, Name, fkDamageCategoryID) VALUES ";
	sql << "(NULL, ";
	sql << "'" << escapeString(individual->idcode) << "', ";
	sql << "'" << escapeString(individual->name) << "', ";
	sql << individual->fkdamagecategoryid << "); ";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);

	if(rc == SQLITE_OK) {
		int id = lastInsertedRowID();
		individual->id = id;
		return id;
	} else
		return -1;
}

// *****************************************************************************
//
// Inserts DamageCategory into DamageCategories table.  Ignores id as
// this is autoincremented in the database.
//
int SQLiteDatabase::insertDamageCategory(DBDamageCategory *damagecategory) {

	stringstream sql;

	sql << "INSERT INTO DamageCategories (ID, Name, OrderID) VALUES ";
	sql << "(NULL, ";
	sql << "'" << escapeString(damagecategory->name) << "', ";
	sql << damagecategory->orderid << ");";

	// cout << sql.str() << endl;

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);

	if(rc == SQLITE_OK) {
		int id = lastInsertedRowID();
		damagecategory->id = id;
		return id;
	} else
		return -1;
}

// *****************************************************************************
//
// Inserts DBPoint into Points table
//
int SQLiteDatabase::insertPoint(DBPoint *point) {

	stringstream sql;
	
	// Not including the attributes might improve the time required to parse the SQL statement
	// and over many inserts, improve speed.  But if the table layout is changed, the order
	// of the values will have to change.
	// sql << "INSERT INTO Points (ID, XCoordinate, YCoordinate, fkOutlineID, OrderID) VALUES ";
	
	sql << "INSERT INTO Points VALUES ";
	sql << "(NULL, ";
	sql << point->xcoordinate << ", ";
	sql << point->ycoordinate << ", ";
	sql << point->fkoutlineid << ", ";
	sql << point->orderid << "); ";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);

	if(rc == SQLITE_OK) {
		int id = lastInsertedRowID();
		point->id = id;
		return id;
	} else
		return -1;
}

// *****************************************************************************
//
// Inserts DBInfo into DBInfo table
//
void SQLiteDatabase::insertDBInfo(DBInfo *dbinfo) {

	stringstream sql;

	sql << "INSERT INTO DBInfo (Key, Value) VALUES ";
	sql << "('" << escapeString(dbinfo->key) << "', ";
	sql << "'" << escapeString(dbinfo->value) << "'); ";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Inserts DBOutline into Outlines table
//
int SQLiteDatabase::insertOutline(DBOutline *outline) {

	stringstream sql;

	sql << "INSERT INTO Outlines (ID, TipPosition, BeginLE, EndLE, NotchPosition, EndTE, fkIndividualID) VALUES ";
	sql << "(NULL, ";
	sql << outline->tipposition << ", ";
	sql << outline->beginle << ", ";
	sql << outline->endle << ", ";
	sql << outline->notchposition << ", ";
	sql << outline->endte << ", ";
	sql << outline->fkindividualid << ");";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);

	if(rc == SQLITE_OK) {
		int id = lastInsertedRowID();
		outline->id = id;
		return id;
	} else
		return -1;
}


// *****************************************************************************
//
// Inserts DBImage into Images table
//
int SQLiteDatabase::insertImage(DBImage *image) {

	stringstream sql;

	sql << "INSERT INTO Images (ID, ImageFilename, DateOfSighting, RollAndFrame, LocationCode, ShortDescription, fkIndividualID) VALUES ";
	sql << "(NULL, ";
	sql << "'" << escapeString(image->imagefilename) << "', ";
	sql << "'" << escapeString(image->dateofsighting) << "', ";
	sql << "'" << escapeString(image->rollandframe) << "', ";
	sql << "'" << escapeString(image->locationcode) << "', ";
	sql << "'" << escapeString(image->shortdescription) << "', ";
	sql << image->fkindividualid << ");";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);

	if(rc == SQLITE_OK) {
		int id = lastInsertedRowID();
		image->id = id;
		return id;
	} else
		return -1;
}


// *****************************************************************************
//
// Inserts DBImageModification into ImageModifications table
//
int SQLiteDatabase::insertImageModification(DBImageModification *imagemod) {

	stringstream sql;

	sql << "INSERT INTO ImageModifications (ID, Operation, Value1, Value2, Value3, Value4, OrderID, fkIndividualID) VALUES ";
	sql << "(NULL, ";
	sql << imagemod->id << ", ";
	sql << imagemod->operation << ", ";
	sql << imagemod->value1 << ", ";
	sql << imagemod->value2 << ", ";
	sql << imagemod->value3 << ", ";
	sql << imagemod->value4 << ", ";
	sql << imagemod->orderid << ", ";
	sql << imagemod->fkimageid << ");";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);

	if(rc == SQLITE_OK) {
		int id = lastInsertedRowID();
		imagemod->id = id;
		return id;
	} else
		return -1;
}


// *****************************************************************************
//
// Inserts DBThumbnail into Thumbnails table
//
int SQLiteDatabase::insertThumbnail(DBThumbnail *thumbnail) {

	stringstream sql;

	sql << "INSERT INTO Thumbnails (ID, Rows, Pixmap, fkImageID) VALUES ";
	sql << "(NULL, ";
	sql << thumbnail->rows << ", ";
	sql << "'" << escapeString(thumbnail->pixmap) << "', ";
	sql << thumbnail->fkimageid << ");";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);

	if(rc == SQLITE_OK) {
		int id = lastInsertedRowID();
		thumbnail->id = id;
		return id;
	} else
		return -1;
}


// *****************************************************************************
//
// Inserts list of DBPoint's into Points table
//
void SQLiteDatabase::insertPoints(std::list<DBPoint>* points) {

	while(! points->empty() ) {
		DBPoint point;
		point = points->front();
		points->pop_front();

		insertPoint(&point);
	}
}


// *****************************************************************************
//
// Inserts list of DBImageModification's into ImageModifications table
//
void SQLiteDatabase::insertImageModifications(std::list<DBImageModification>* imagemods) {

	while(! imagemods->empty() ) {
		DBImageModification imagemod;
		imagemod = imagemods->front();
		imagemods->pop_front();

		insertImageModification(&imagemod);
	}
}


// *****************************************************************************
//
// Updates outline in Outlines table  
//
void SQLiteDatabase::updateOutline(DBOutline *outline) {

	stringstream sql;

	sql << "UPDATE Outlines SET ";
	sql << "TipPosition = " << outline->tipposition << ", ";
	sql << "BeginLE = " << outline->beginle << ", ";
	sql << "EndLE = " << outline->endle << ", ";
	sql << "NotchPosition = " << outline->notchposition << ", ";
	sql << "fkIndividualID = " << outline->fkindividualid << " ";
	sql << "WHERE ID = " << outline->id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}


// *****************************************************************************
//
// Updates row in DamageCategories table using given DBDamageCategory struct.
// Uses ID field for identifying row.
//
void SQLiteDatabase::updateDamageCategory(DBDamageCategory *damagecategory) {

	stringstream sql;

	sql << "UPDATE DamageCategories SET ";
	sql << "Name = '" << escapeString(damagecategory->name) << "' ";
	sql << "AND OrderID = " << damagecategory->orderid << " ";
	sql << "WHERE ID = " << damagecategory->id << ";";

	// cout << sql.str() << endl;

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}


// *****************************************************************************
//
// Updates row in Individuals table using given DBIndividual struct.  Uses ID
// field for identifying row.
//
void SQLiteDatabase::updateIndividual(DBIndividual *individual) {

	stringstream sql;

	sql << "UPDATE Individuals SET ";
	sql << "IDCode = '" << escapeString(individual->idcode) << "', ";
	sql << "Name = '" << escapeString(individual->name) << "', ";
	sql << "fkDamageCategoryID = " << individual->fkdamagecategoryid << " ";
	sql << "WHERE ID = " << individual->id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}


// *****************************************************************************
//
// Updates row in Images table using given DBImage struct.  Uses ID
// field for identifying row.
//
void SQLiteDatabase::updateImage(DBImage *image) {

	stringstream sql;

	sql << "UPDATE Images SET ";
	sql << "ImageFilename = '" << escapeString(image->imagefilename) << "', ";
	sql << "DateOfSighting = '" << escapeString(image->dateofsighting) << "', ";
	sql << "RollAndFrame = '" << escapeString(image->rollandframe) << "', ";
	sql << "LocationCode = '" << escapeString(image->locationcode) << "', ";
	sql << "ShortDescription = '" << escapeString(image->shortdescription) << "' ";
	sql << "fkIndividualID = " << image->fkindividualid << " ";
	sql << "WHERE ID = " << image->id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}


// *****************************************************************************
//
// Updates row in ImageModifications table using given DBImageModification
// struct.  Uses ID field for identifying row.
//
void SQLiteDatabase::updateImageModification(DBImageModification *imagemod) {

	stringstream sql;

	sql << "UPDATE ImageModifications SET ";
	sql << "Operation = " << imagemod->operation << ", ";
	sql << "Value1 = " << imagemod->value1 << ", ";
	sql << "Value2 = " << imagemod->value2 << ", ";
	sql << "Value3 = " << imagemod->value3 << ", ";
	sql << "Value4 = " << imagemod->value4 << ", ";
	sql << "OrderID = " << imagemod->orderid << ", ";
	sql << "fkImageID = " << imagemod->fkimageid << " ";
	sql << "WHERE ID = " << imagemod->id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Updates row in Thumbnails table using given DBThumbnail
// struct.  Uses ID field for identifying row.
//
void SQLiteDatabase::updateThumbnail(DBThumbnail *thumbnail) {

	stringstream sql;

	sql << "UPDATE Thumbnails SET ";
	sql << "Rows = " << thumbnail->rows << ", ";
	sql << "Pixmap = '" << escapeString(thumbnail->pixmap) << "', ";
	sql << "fkImageID = " << thumbnail->fkimageid << " ";
	sql << "WHERE ID = " << thumbnail->id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Updates row in DBInfo table using given DBInfo
// struct.  Uses ID field for identifying row.
//
void SQLiteDatabase::updateDBInfo(DBInfo *dbinfo) {

	stringstream sql;

	sql << "UPDATE DBInfo SET ";
	sql << "Value = '" << escapeString(dbinfo->value) << "', ";
	sql << "WHERE Key = '" <<escapeString(dbinfo->key) << "';";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}



// *****************************************************************************
//
// Deletes set of points from Points table using fkOutlineID  
//
void SQLiteDatabase::deletePoints(int fkOutlineID) {

	stringstream sql;

	sql << "DELETE FROM Points ";
	sql << "WHERE fkOutlineID = " << fkOutlineID << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}



// *****************************************************************************
//
// Delete outline from Outlines table using fkIndividualID  
//
void SQLiteDatabase::deleteOutlineByFkIndividualID(int fkIndividualID) {

	stringstream sql;

	sql << "DELETE FROM Outlines ";
	sql << "WHERE fkIndividualID = " << fkIndividualID << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Delete outline from Outlines table using id  
//
void SQLiteDatabase::deleteOutlineByID(int id) {

	stringstream sql;

	sql << "DELETE FROM Outlines ";
	sql << "WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}


// *****************************************************************************
//
// Delete individual from Individuals table using id  
//
void SQLiteDatabase::deleteIndividual(int id) {

	stringstream sql;

	sql << "DELETE FROM Individuals ";
	sql << "WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Delete damagecategory from DamageCategories table using id  
//
void SQLiteDatabase::deleteDamageCategory(int id) {

	stringstream sql;

	sql << "DELETE FROM DamageCategories ";
	sql << "WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Delete image from Images table using id  
//
void SQLiteDatabase::deleteImage(int id) {

	stringstream sql;

	sql << "DELETE FROM Images ";
	sql << "WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}


// *****************************************************************************
//
// Delete imagemod from ImageModifications table using id  
//
void SQLiteDatabase::deleteImageModification(int id) {

	stringstream sql;

	sql << "DELETE FROM ImageModifications ";
	sql << "WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Delete thumbnail from Thumbnails table using id  
//
void SQLiteDatabase::deleteThumbnail(int id) {

	stringstream sql;

	sql << "DELETE FROM Thumbnails ";
	sql << "WHERE ID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

// *****************************************************************************
//
// Delete thumbnail from Thumbnails table using fkImageID  
//
void SQLiteDatabase::deleteThumbnailByFkImageID(int id) {

	stringstream sql;

	sql << "DELETE FROM Thumbnails ";
	sql << "WHERE fkImageID = " << id << ";";

	rc = sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
}

unsigned long SQLiteDatabase::add(DatabaseFin<ColorImage> *fin) {

	// cout << "adding fin" << endl;
	DBIndividual individual;
	DBImage image;
	DBOutline outline;
	DBThumbnail thumbnail;
	DBDamageCategory dmgCat;
	Outline *finOutline;
	std::list<DBPoint> *points = new std::list<DBPoint>();
	FloatContour *fc;
	int i, numPoints, pos;
	std::string pixTemp = "";
	DBPoint point;
	string shortFilename;

	//***054 - assume that the image filename contains path
	// information which must be stripped BEFORE saving fin
	shortFilename = fin->mImageFilename;
	pos = shortFilename.find_last_of(PATH_SLASH);
	if (pos >= 0) {
		shortFilename = shortFilename.substr(pos+1);
	}
	fin->mImageFilename = shortFilename;
	
	beginTransaction();
	dmgCat = selectDamageCategoryByName( fin->getDamage() );
	
	individual.idcode = fin->getID();
	individual.name = fin->getName();
	individual.fkdamagecategoryid = dmgCat.id;
	insertIndividual(&individual);

	finOutline = fin->mFinOutline;
	outline.beginle = finOutline->getFeaturePoint(LE_BEGIN);
	outline.endle = finOutline->getFeaturePoint(LE_END);
	outline.notchposition = finOutline->getFeaturePoint(NOTCH);
	outline.tipposition = finOutline->getFeaturePoint(TIP);
	outline.endte = finOutline->getFeaturePoint(POINT_OF_INFLECTION);
	outline.fkindividualid = individual.id;
	insertOutline(&outline);
	
	numPoints = finOutline->length();
	fc = finOutline->getFloatContour();
	for(i = 0; i < numPoints; i++) {
		point.xcoordinate = (*fc)[i].x;
		point.ycoordinate = (*fc)[i].y;
		point.orderid = i;
		point.fkoutlineid = outline.id;

		points->push_back(point);
	}
	insertPoints(points);

	image.dateofsighting = fin->getDate();
	image.imagefilename = fin->mImageFilename;
	image.locationcode = fin->getLocation();
	image.rollandframe = fin->getRoll();
	image.shortdescription = fin->getShortDescription();
	image.fkindividualid = individual.id;
	insertImage(&image);


	for (i = 0; i < fin->mThumbnailRows; i++) {;
		pixTemp += fin->mThumbnailPixmap[i];
		pixTemp += "\n";
	}
	thumbnail.rows = fin->mThumbnailRows;
	thumbnail.pixmap = pixTemp;
	thumbnail.fkimageid = image.id;
	insertThumbnail(&thumbnail);

	commitTransaction();

	addFinToLists(individual.id, individual.name, individual.idcode, image.dateofsighting,
		image.rollandframe, image.locationcode, dmgCat.name, image.shortdescription);

	sortLists();
	
	delete points;

	return individual.id; // mDataPos field will be used to map to id in db for individuals
}

// *****************************************************************************
//
// Updates DatabaseFin<ColorImage>
//

void SQLiteDatabase::update(DatabaseFin<ColorImage> *fin) {
	DBIndividual individual;
	DBImage image;
	DBOutline outline;
	DBThumbnail thumbnail;
	Outline *finOutline;
	std::list<DBPoint> points = std::list<DBPoint>();
	FloatContour *fc;
	int i, numPoints;
	std::string pixTemp;
	
	individual.id = fin->mDataPos; // mapping Individuals id to mDataPos
	individual.idcode = fin->getID();
	individual.name = fin->getName();
	individual.fkdamagecategoryid = ( selectDamageCategoryByName(fin->getDamage()) ).id;
	updateIndividual(&individual);

	finOutline = fin->mFinOutline;
	// we do this as we don't know what the outline id is
	outline = selectOutlineByFkIndividualID(individual.id);
	outline.beginle = finOutline->getFeaturePoint(LE_BEGIN);
	outline.endle = finOutline->getFeaturePoint(LE_END);
	outline.notchposition = finOutline->getFeaturePoint(NOTCH);
	outline.tipposition = finOutline->getFeaturePoint(TIP);
	outline.endte = finOutline->getFeaturePoint(POINT_OF_INFLECTION);
	outline.fkindividualid = individual.id;
	updateOutline(&outline);	
	
	numPoints = finOutline->length();
	fc = finOutline->getFloatContour();
	for(i = 0; i < numPoints; i++) {
		DBPoint point;

		point.xcoordinate = (*fc)[i].x;
		point.ycoordinate = (*fc)[i].y;
		point.orderid = i;
		point.fkoutlineid = outline.id;

		points.push_back(point);
	}
	deletePoints(outline.id);
	insertPoints(&points);

	// query db as we don't know the image id
	image = selectImageByFkIndividualID(individual.id);
	image.dateofsighting = fin->getDate();
	image.imagefilename = fin->mImageFilename;;
	image.locationcode = fin->getLocation();
	image.rollandframe = fin->getRoll();
	image.shortdescription = fin->getShortDescription();
	image.fkindividualid = individual.id;
	updateImage(&image);
	
	// query db as we don't know the thumbnail id
	thumbnail = selectThumbnailByFkImageID(image.id);
	thumbnail.rows = fin->mThumbnailRows;
	for (i = 0; i < fin->mThumbnailRows; i++) {
		pixTemp += fin->mThumbnailPixmap[i];
		pixTemp += "\n";
	}
	thumbnail.pixmap = pixTemp;

	updateThumbnail(&thumbnail);

	loadLists(); // reload and re-sort lists.
}

// *****************************************************************************
//
// Returns complete DatabaseFin<ColorImage>. mDataPos field will be used to map to id in 
// db for individuals
//

DatabaseFin<ColorImage>* SQLiteDatabase::getFin(int id) {

	DBIndividual individual;
	DBImage image;
	DBOutline outline;
	DBThumbnail thumbnail;
	DBDamageCategory damagecategory;
	Outline *finOutline;
	std::list<DBPoint> *points = new std::list<DBPoint>();
	FloatContour *fc = new FloatContour();
	DatabaseFin<ColorImage> *fin;
	string imageFilename;
	int pos;
	
	beginTransaction();
	individual = selectIndividualByID(id);
	damagecategory = selectDamageCategoryByID(individual.fkdamagecategoryid);
	image = selectImageByFkIndividualID(id);
	outline = selectOutlineByFkIndividualID(id);
	thumbnail = selectThumbnailByFkImageID(image.id);
	selectPointsByFkOutlineID(points, outline.id);
	commitTransaction();

	// Although having both of these blocks of code seems uesless, this ensures that
	// the given path contains only the image filename.  If the given path contains
	// more, then the first code block will strip it down.

	// Strip path info
	imageFilename = image.imagefilename;
	pos = imageFilename.find_last_of(PATH_SLASH);
	if (pos >= 0) {
		imageFilename = imageFilename.substr(pos+1);
	}
	image.imagefilename = imageFilename;

	// Add current path info
	imageFilename = gOptions->mCurrentSurveyArea; //***1.85
	imageFilename += PATH_SLASH;
	imageFilename += "catalog";
	imageFilename += PATH_SLASH;
	imageFilename += image.imagefilename;
	image.imagefilename = imageFilename;
	
	// assumes list is returned as FIFO (queue)... should be due to use of ORDER BY OrderID
	while(! points->empty() ) {
		DBPoint point = points->front();
		points->pop_front();
		fc->addPoint(point.xcoordinate, point.ycoordinate);
	}

	finOutline = new Outline(fc);
	finOutline->setFeaturePoint(LE_BEGIN, outline.beginle);
	finOutline->setFeaturePoint(LE_END, outline.endle);
	finOutline->setFeaturePoint(NOTCH, outline.notchposition);
	finOutline->setFeaturePoint(TIP, outline.tipposition);
	finOutline->setFeaturePoint(POINT_OF_INFLECTION, outline.endte);
	finOutline->setLEAngle(0.0,true);

	
	// Based on thumbnail size in DatabaseFin<ColorImage>
	char **pixmap = new char*[thumbnail.rows];
	std::string pixmapString = thumbnail.pixmap;
	std::string buffer;

	for(int i = 0; i < thumbnail.rows; i++) {
		int j = 0;
		
		j = pixmapString.find('\n');

		if(j != string::npos && j <= pixmapString.size()) {
			buffer = pixmapString.substr(0, j);
			pixmapString = pixmapString.substr(j + 1);
		} else {
			buffer = pixmapString;
		}

		pixmap[i] = new char[buffer.length() + 1];
		strcpy(pixmap[i], buffer.c_str());
	}

	fin = new DatabaseFin<ColorImage>(image.imagefilename, 
		finOutline, 
		individual.idcode, 
		individual.name,
		image.dateofsighting,
		image.rollandframe,
		image.locationcode,
		damagecategory.name,
		image.shortdescription,
		individual.id, // mDataPos field will be used to map to id in db for individuals
		pixmap,
		thumbnail.rows
		);
	
	delete fc; 	//***1.0LK - fc is COPIED in Outline so we must delete it here
	delete points;
	
	return fin;
}

// *****************************************************************************
//
// Returns all fins from database.
//

std::list< DatabaseFin<ColorImage>* >* SQLiteDatabase::getAllFins(void) {

	std::list<DBIndividual> *individuals = new std::list<DBIndividual>();
	std::list< DatabaseFin<ColorImage>* > *fins = new std::list< DatabaseFin<ColorImage>* >();
	DBIndividual individual;
	
	selectAllIndividuals(individuals);

	while(! individuals->empty() ) {
		individual = individuals->front();
		individuals->pop_front();
		fins->push_back( getFin(individual.id) );
	}

	
	delete individuals;

	return fins;
}

// *****************************************************************************
//
// Returns fin from database.  pos refers to position within one of the sort
// lists.
//

DatabaseFin<ColorImage>* SQLiteDatabase::getItem(unsigned pos) {

	std::vector<std::string> *it;

	if (mCurrentSort == DB_SORT_NAME)
		//it = mNameList.begin();
		it = &mNameList;

	else if (mCurrentSort == DB_SORT_ID)
		//it = mIDList.begin();
		it = &mIDList;

	else if (mCurrentSort == DB_SORT_DATE)
		//it = mDateList.begin();
		it = &mDateList;

	else if (mCurrentSort == DB_SORT_ROLL)
		//it = mRollList.begin();
		it = &mRollList;

	else if (mCurrentSort == DB_SORT_LOCATION)
		//it = mLocationList.begin();
		it = &mLocationList;

	else if (mCurrentSort == DB_SORT_DAMAGE)
		//it = mDamageList.begin();
		it = &mDamageList;

	else if (mCurrentSort == DB_SORT_DESCRIPTION)
		//it = mDescriptionList.begin();
		it = &mDescriptionList;

	else  // it's not a valid sort type
		return NULL;

	return getItem(pos, it);
}

// *****************************************************************************
//
// Delete fin from database
//

void SQLiteDatabase::Delete(DatabaseFin<ColorImage> *fin) {

	DBOutline outline;
	DBImage image;
	int id;

	// mDataPos field will be used to map to id in db for individuals
	id = fin->mDataPos;
	
	outline = selectOutlineByFkIndividualID(id);
	image = selectImageByFkIndividualID(id);

	this->deletePoints(outline.id);
	this->deleteOutlineByFkIndividualID(id);
	this->deleteThumbnailByFkImageID(image.id);
	this->deleteImage(image.id);
	this->deleteIndividual(id);
	
	deleteFinFromLists(id);
}

// *****************************************************************************
//
// Sorts the lists
//

void SQLiteDatabase::sortLists() {
	std::sort(mNameList.begin(),mNameList.end());
  	std::sort(mIDList.begin(),mIDList.end());
  	std::sort(mDateList.begin(),mDateList.end());
  	std::sort(mRollList.begin(),mRollList.end());
  	std::sort(mLocationList.begin(),mLocationList.end());
  	std::sort(mDamageList.begin(),mDamageList.end());
  	std::sort(mDescriptionList.begin(),mDescriptionList.end());
}

//*******************************************************************
//
// Looks up row id in AbsoluteOffset list and then uses getFin(int)
// to retrieve that fin from the database.
//

DatabaseFin<ColorImage>* SQLiteDatabase::getItemAbsolute(unsigned pos) {
	
	// cout << "getItemAbsolute()" << endl;

	if (pos > this->mAbsoluteOffset.size())
	       throw BoundsError();

	if (mAbsoluteOffset[pos] == -1)
		return NULL;               // this is a HOLE, a previously deleted fin

	DatabaseFin<ColorImage> *fin = getFin(this->mAbsoluteOffset[pos]);

	return fin;
}

//*******************************************************************
//
// Given a list and position in that list, returns fin from database.
//

DatabaseFin<ColorImage>* SQLiteDatabase::getItem(unsigned pos, std::vector<std::string> *theList) {	
	
	return getFin( listEntryToID((*theList)[pos]) );
}


string SQLiteDatabase::nullToNone(string str) {

	return str != "NULL" ? str : "NONE";
}

void SQLiteDatabase::addFinToLists(DatabaseFin<ColorImage>* fin)
{
	addFinToLists( fin->mDataPos, fin->getName(), fin->getID(), fin->getDate(),
		fin->getRoll(), fin->getLocation(), fin->getDamage(),
		fin->getShortDescription() );

}

int SQLiteDatabase::listEntryToID(string entry)
{
	istrstream inStream(entry.c_str());

	std::string prev, cur;
	
	// we'll assume the last token in the stream is the position in
	// the file
	while (inStream >> cur)
		prev = cur;

	return atoi(prev.c_str());
}

void SQLiteDatabase::deleteEntry(std::vector<string>* lst, int id)
{
	int toDelete = -1;

	for(unsigned i = 0; i < lst->size(); i++)
			if(	listEntryToID(lst->at(i)) == id)
				toDelete = i;
	
	if(toDelete != -1)
		lst->erase(lst->begin() + toDelete);
}

void SQLiteDatabase::deleteFinFromLists(int id)
{
	int toDelete = -1;

	deleteEntry(&mNameList, id);
	deleteEntry(&mIDList, id);
	deleteEntry(&mDateList, id);
	deleteEntry(&mRollList, id);
	deleteEntry(&mLocationList, id);
	deleteEntry(&mDamageList, id);
	deleteEntry(&mDescriptionList, id);

	for(int i = 0; i < mAbsoluteOffset.size(); i++)
		if(mAbsoluteOffset[i] == id)
			toDelete = i;
	
	if(toDelete != -1)
		mAbsoluteOffset.erase(mAbsoluteOffset.begin() + toDelete);
}

//*******************************************************************
//
// Adds a fin to the sort lists. Does not resort the lists.
//
void SQLiteDatabase::addFinToLists(int datapos, string name, string id, string date, string roll,
								   string location, string damage, string description)
{

	stringstream temp;
	
	temp << nullToNone(name) << " " << datapos;
	mNameList.push_back(temp.str());
	temp.str("");

	temp << nullToNone(id) << " " << datapos;
	mIDList.push_back(temp.str());
	temp.str("");

	temp << nullToNone(date) << " " << datapos;
	mDateList.push_back(temp.str());
	temp.str("");

	temp << nullToNone(roll) << " " << datapos;
	mRollList.push_back(temp.str());
	temp.str("");

	temp << nullToNone(location) << " " << datapos;
	mLocationList.push_back(temp.str());
	temp.str("");

	temp << nullToNone(damage) << " " << datapos;
	mDamageList.push_back(temp.str());
	temp.str("");

	temp << nullToNone(description) << " " << datapos;
	mDescriptionList.push_back(temp.str());
	temp.str("");
	
	mAbsoluteOffset.push_back(datapos);
}


//*******************************************************************
//
// Rebuilds the lists from the database and sorts them.
//

void SQLiteDatabase::loadLists() {

	std::list< DatabaseFin<ColorImage>* > *fins;

	mNameList.clear();
	mIDList.clear();
	mDateList.clear();
	mRollList.clear();
	mLocationList.clear();
	mDamageList.clear();
	mDescriptionList.clear();
	mAbsoluteOffset.clear();

	fins = getAllFins();

	while(! fins->empty() ) {
		DatabaseFin<ColorImage>* fin = fins->front();
		fins->pop_front();

		addFinToLists(fin);

		delete fin;
	}

	delete fins;

	sortLists();
}


// *****************************************************************************
//
// Opens the database file
//

void SQLiteDatabase::opendb(const char *filename) {

	rc = sqlite3_open(filename, &db);

	if( rc ) {
		// cout << "Can't open database: " << sqlite3_errmsg(db) << endl;
		closedb();
		dbOpen = false;
		mDBStatus = errorLoading;
	} else {
		dbOpen = true;
	}
}

// *****************************************************************************
//
// Closes the database file
//

void SQLiteDatabase::closedb() {

	if(dbOpen)
		sqlite3_close(db);

	dbOpen = false;
}

// *****************************************************************************
//
// Create empty db
//

void SQLiteDatabase::createEmptyDatabase(Options *o) {
	stringstream sql;
	DBDamageCategory cat;

	// SQL CREATE TABLE statements... might be better off defined in the header as a constant..

	sql << "CREATE TABLE DamageCategories (";
	sql << "ID INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sql << "OrderID INTEGER, ";
	sql << "Name TEXT ";
	sql << ");" << endl;

	sql << "CREATE TABLE Individuals ( ";
	sql << "ID INTEGER PRIMARY KEY, ";
	sql << "IDCode TEXT, ";
	sql << "Name TEXT, ";
	sql << "fkDamageCategoryID INTEGER ";
	sql << ");" << endl;
	
	sql << "CREATE TABLE Images ( ";
	sql << "ID INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sql << "fkIndividualID INTEGER, ";
	sql << "ImageFilename TEXT, ";
	sql << "DateOfSighting TEXT, ";
	sql << "RollAndFrame TEXT, ";
	sql << "LocationCode TEXT, ";
	sql << "ShortDescription TEXT ";
	sql << ");" << endl;

	sql << "CREATE TABLE ImageModifications ( ";
	sql << "ID INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sql << "Operation INTEGER, ";
	sql << "Value1 INTEGER, ";
	sql << "Value2 INTEGER, ";
	sql << "Value3 INTEGER, ";
	sql << "Value4 INTEGER, ";
	sql << "OrderID INTEGER, ";
	sql << "fkImageID INTEGER ";
	sql << ");" << endl;

	sql << "CREATE TABLE Thumbnails ( ";
	sql << "ID INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sql << "fkImageID INTEGER, ";
	sql << "Rows INTEGER, ";
	sql << "Pixmap TEXT ";
	sql << ");" << endl;
	
	sql << "CREATE TABLE Outlines ( ";
	sql << "ID INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sql << "TipPosition INTEGER, ";
	sql << "BeginLE INTEGER, ";
	sql << "EndLE INTEGER, ";
	sql << "NotchPosition INTEGER, ";
	sql << "EndTE INTEGER, ";
	sql << "fkIndividualID INTEGER ";
	sql << ");" << endl;
	
	sql << "CREATE TABLE Points ( ";
	sql << "ID INTEGER PRIMARY KEY AUTOINCREMENT, ";
	sql << "XCoordinate REAL, ";
	sql << "YCoordinate REAL, ";
	sql << "fkOutlineID INTEGER, ";
	sql << "OrderID INTEGER ";
	sql << ");" << endl;

	sql << "CREATE INDEX dmgcat_orderid ON DamageCategories (OrderID);" << endl;

	sql << "CREATE INDEX dmgcat_name ON DamageCategories (Name);" << endl;

	sql << "CREATE INDEX imgmod_img ON  ImageModifications (fkImageID);" << endl;

	sql << "CREATE INDEX img_indiv ON Images (fkIndividualID);" << endl;

	sql << "CREATE INDEX outln_indiv ON Outlines (fkIndividualID);" << endl;

	sql << "CREATE INDEX pts_outln ON Points (fkOutlineID);" << endl;

	sql << "CREATE INDEX pts_order ON Points (OrderID);" << endl;

	sql << "CREATE INDEX pts_outln_order ON Points (fkOutlineID, OrderID);" << endl;

	sql << "CREATE INDEX thmbnl_img ON Thumbnails (fkImageID);" << endl;

	sqlite3_exec(db, sql.str().c_str(), NULL, 0, &zErrMsg);
	
	// TODO: enter code to populate DBInfo
	
	for (int i = 0; i < o->mCatCategoryNamesMax; i++) {
		cat.name = o->mCatCategoryName[i];
		cat.orderid = i;
		insertDamageCategory(&cat);		
	}

}

//*******************************************************************
//
/*
 * Used to figure out if the given filePath refers to a database file
 * supported by this class.
 */
bool SQLiteDatabase::isType(std::string filePath)
{

	const int MAGIC_NUMBER_LENGTH = 15;
	char MAGIC_NUMBER[] = "SQLite format 3\0";

	// try to open file
	fstream testFile(filePath.c_str(), ios::in | ios::binary);

	if (!testFile)
	{
		printf("\nError locating or opening database file!\n");
		return false;
	}

	testFile.seekg(0, ios::beg);
	
	char magicNumber[MAGIC_NUMBER_LENGTH+1];
	magicNumber[15] = '\0';
	
	// try to read from the file...
	if (!(testFile.read(magicNumber, MAGIC_NUMBER_LENGTH))) 
	{
		printf("\ndatabase file is completely empty.\n");
		return false;
	} 

	if (strcmp(magicNumber, MAGIC_NUMBER) != 0)
	{
		printf("\nNon-SQLite database.\n");
		return false;
	}

	testFile.close();
	testFile.clear();

	return true;
}

// *****************************************************************************
//
// Constructor
//

SQLiteDatabase::SQLiteDatabase(Options *o, bool createEmptyDB)
	:
	Database(o, createEmptyDB)
	{
	std::list<DBDamageCategory> *damagecategories = new std::list<DBDamageCategory>();
	DBDamageCategory damagecategory;
	int i = 0;
	
	zErrMsg = 0;
	dbOpen = false;
	mFilename = std::string(o->mDatabaseFileName);
	mCurrentSort = DB_SORT_NAME;
	
	// cout << "SQLiteDatabase: loading db... " << o->mDatabaseFileName << endl;

	if (mFilename == "NONE") {
		// cout << "\nNO File Name specified for existing database!\n";
		mDBStatus = fileNotFound;
		return;
	}

	opendb(mFilename.c_str());
	if(! dbOpen) {
		mDBStatus = errorLoading;
		return;
	}

	this->setSyncMode(0); // set sync mode to OFF.  Significant improvement in write speed.

	if(createEmptyDB)
		createEmptyDatabase(o);
	
	// get damage categories
	this->selectAllDamageCategories(damagecategories);
	o->mCatCategoryNamesMax = damagecategories->size();
	o->mCatCategoryName.resize( o->mCatCategoryNamesMax );
	
	while(! damagecategories->empty()) {
		damagecategory = damagecategories->front();
		damagecategories->pop_front();
		o->mCatCategoryName[i++] = damagecategory.name;
	}
	
	loadLists();
	mDBStatus = loaded;
	delete damagecategories;

}


// *****************************************************************************
//
// Deconstructor
//

SQLiteDatabase::~SQLiteDatabase() {
	
	closedb();

}

//*******************************************************************
//

bool SQLiteDatabase::closeStream()
{
	closedb();
	return true;
}


//*******************************************************************
//

bool SQLiteDatabase::openStream()
{
	opendb(mFilename.c_str());
	return true;
}
