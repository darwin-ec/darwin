//*******************************************************************
//   file: OldDatabase.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (9/27/2005)
//         -- change to allow relative storage of image files
//            (now, stored filename has path, if any, related only to
//             the catalog category within which it falls)
//         -- current implementation: ALL images are copies of originals
//            and are stored in the same place as "darwin.db"
//         -- 9/29/2005
//            OldDatabase version 3 - stores catalog category labels in OldDatabase
//
//         -- total rewrite as non template class inheriting from
//            Database ABSTRACT class (7/8/2008)
//
//*******************************************************************

#include "OldDatabase.h"

//*******************************************************************
//

OldDatabase::OldDatabase(Options *o, bool createEmptyDB) 
	:
	Database(o,createEmptyDB) //***1.99 - call parent constructor
	// next two assigns are performed in parent constructor
	//mFilename(o->mDatabaseFileName /*filename*/),
	//mCurrentSort(DB_SORT_NAME)
{
		
	//***1.85 - create an empty OldDatabase ONLY when requested, then let process continue
	//          as previously
	if (createEmptyDB)
	{
		if (mFilename == "NONE")
		{
			printf("\nNO File Name specified for new OldDatabase!\n");
			//***1.85 - return indication of error and let this be handled in calling context
			mDBStatus = errorCreating;
			return;
		}

		// do NOT allow creation of file that already exists
		fstream testFile(mFilename.c_str(), ios::in | ios::binary);
		if (! testFile.fail())
		{
			printf("\nAttempt to create and EXISTING OldDatabase file!\n");
			//***1.85 - return indication of error and let this be handled in calling context
			mDBStatus = errorCreating;
			return;
		}
		// make sure file itself can be created
		mDbFile.open(mFilename.c_str(), ios::out | ios::binary);
		if (!mDbFile)
		{
			printf("\nError creating OldDatabase file!\n");
			//***1.85 - return indication of error and let this be handled in calling context
			mDBStatus = errorCreating;
			return;
		}
		// create the structure of a new empty OldDatabase
		createEmptyDatabase(o); // assumes file is already open
		// clean up before continuing
		mDbFile.close();
		mDbFile.clear();
	}

	//***1.85 - make sure all existing lists are clear (empty)
	mNameList.clear();
	mIDList.clear();
	mDateList.clear();
	mRollList.clear();
	mLocationList.clear();
	mDamageList.clear();
	mDescriptionList.clear();
	mAbsoluteOffset.clear();;

	// NOW, go about the process of actually loading the OldDatabase file

	cout << "Loading OldDatabase ...\n  \"" << mFilename << "\"" <<endl;

	if (mFilename == "NONE")
	{
		printf("\nNO File Name specified for existing OldDatabase!\n");
		//***1.85 - return indication of error and let this be handled in calling context
		mDBStatus = fileNotFound;
		return;
	}

   // new command for all platforms - JHS (5/20/2005)
   fstream testFile(mFilename.c_str(), ios::in | ios::binary);
   if (!testFile)
	{
		//***1.85 - do NOT automatically create new file now
		// create it, since it apparently doesn't exist
		//testFile.clear();
		//testFile.open(mFilename.c_str(), ios::out | ios::binary);
		//testFile.close();

		//***1.85 - return indication of error and let this be handled in calling context
		printf("\nError locating or opening OldDatabase file!\n");
		mDBStatus = fileNotFound;
		return;
	}

	mDbFile.open(mFilename.c_str(), ios::in | ios::out | ios::binary);
	
	// see if we still have problems
	if (!mDbFile)
	{
		printf("\nError locating or opening OldDatabase file!\n");
		//***1.85 - return indication of error and let this be handled in calling context
		mDBStatus = fileNotFound;
		return;
	}

	mDbFile.seekg(0, ios::beg);
	
	unsigned long footerPos;

	if (!(mDbFile.read((char*)&footerPos, sizeof(unsigned long)))) 
	{
//#ifdef DEBUG
//		std::cout << "OldDatabase file is completely empty." << std::endl;
//#endif
		//***054 - create header for empty OldDatabase file
		//       - this contains footer position, version number and catalog labels
		//***1.85 - defer this to calling context
		//mDbFile.clear();
		//createEmptyOldDatabase(o);

		//***1.85 - return indication of error and let this be handled in calling context
		printf("\nOldDatabase file is completely empty.\n");
		mDBStatus = errorLoading;
		return;
	} 
	else 
	{
		mDataSize = 0; //***055DB
		mHeaderSize = sizeof(unsigned long); //***055DB
		//cout << footerPos << endl;

		//***001DB - next 8 lines check for DB version
		unsigned int version;
       	mDbFile.read((char*)&version, sizeof(int));
		mHeaderSize += sizeof(int); //***055DB
		//cout << version << ":" << CURRENT_DBVERSION << endl;

		bool fileOK = false;
		float pgmVersion;
		int dbVersion;

		if(version == CURRENT_DBVERSION) // old test when only an int was expected
		{
			fileOK = true;
			pgmVersion = 1.8f;    // reasonable assumption for any pre version 1.85 creation
			dbVersion = version; // obviously
		}
		else
		{
			//***1.85 - new magic number inclued "DB", program version and db version
			//cout << "MagicNum: " << hex << version << dec << endl;

			// IMPORTANT NOTE: take the code 0x4442073D
			// As an integer the code is interpretted as above with 4442 being the
			// ASCII digits "DB" in hex and 073D begin 1853 in decimal (or 1.85 & 3) for
			// the version numbers ...
			// HOWEVER, if version is treated as the address of a char array
			// the byte order acts as if reversed and the ASCII codes are ...
			// (version[0] == 3D, version[0] == 07, version[0] == 42, version[0] == 44)
			// be careful about how you look at specific positions.
			
			if ((version & 0xFFFF0000) == 0x44420000) // the magic part is "DB"
			{
				version -= 0x44420000; // take out the "DB"
				dbVersion = version % 10; // least significant digit is OldDatabase version
				version /= 10;
				pgmVersion = version / 100.0; // version is 4 digits in this firm (vv.vv)
				fileOK = true;
			}
			else
			{
				printf("\nOld/Incompatible OldDatabase Version.\n");
				//exit(1);  // or convert old DB format to new?

				//***1.85 - let this be handled in calling context
				mDBStatus = oldDBVersion;
				return;
			}
		} 

		if (fileOK) 
		{
			//cout << "Loading OldDatabase ...\n  \"" << mFilename << "\"" <<endl;
			cout << "  (Created by DARWIN version " << pgmVersion 
				<< " with OldDatabase format #" << dbVersion << ")" << endl;

			// load Catalog Category Names specified in OldDatabase
			mDbFile.read((char*)&o->mCatCategoryNamesMax, sizeof(int));
			mHeaderSize += sizeof(int);
			o->mCatCategoryName.resize(o->mCatCategoryNamesMax); //***1.85 - it is a vector now
			for (int id = 0; id < o->mCatCategoryNamesMax; id++)
			{
				getline(mDbFile,o->mCatCategoryName[id]);
				mHeaderSize += o->mCatCategoryName[id].length() + 1; //***055DB,***1.4 (+1 for '\n')
			}

			mFooterPos = footerPos;

			mDataSize = footerPos - mHeaderSize; //***054

			mDbFile.seekg(mFooterPos, ios::beg);

			unsigned long numEntries;
		
			mDbFile.read((char*)&numEntries, sizeof(unsigned long));
#ifdef DEBUG
			std::cout << "Reading OldDatabase (footer position: "
				<< mFooterPos << " number of entries: " << numEntries
				<< ")" << std::endl;
#endif

			unsigned i; // since Visual C++ does not scope local loop counters correctly
	
			for (i = 0; i < numEntries; i++) 
			{
				char line[255];
				mDbFile.getline(line, 255);
				std::string temp = line;
				mNameList.push_back(temp);
			}

			for (i = 0; i < numEntries; i++) 
			{
				char line[255];
				mDbFile.getline(line, 255);
				std::string temp = line;
				mIDList.push_back(temp);
			}
	
			for (i = 0; i < numEntries; i++) 
			{
				char line[255];
				mDbFile.getline(line, 255);
				std::string temp = line;
				mDateList.push_back(temp);
			}

			for (i = 0; i < numEntries; i++) 
			{
				char line[255];
				mDbFile.getline(line, 255);
				std::string temp = line;
				mRollList.push_back(temp);
			}

			for (i = 0; i < numEntries; i++) 
			{
				char line[255];
				mDbFile.getline(line, 255);
				std::string temp = line;
				mLocationList.push_back(temp);
			}

			for (i = 0; i < numEntries; i++) 
			{
				char line[255];
				mDbFile.getline(line, 255);
				std::string temp = line;
				mDamageList.push_back(temp);
			}

			for (i = 0; i < numEntries; i++) 
			{
				char line[255];
				mDbFile.getline(line, 255);
				std::string temp = line;
				mDescriptionList.push_back(temp);
			}

			//***1.3 - list of absolute file offset locations including those
			// of holes (places where fins were deleted)
			bool done = false;
			while (! done)
			{
				char line[255];
				mDbFile.getline(line, 255);
				if (mDbFile.fail())
				{
					mDbFile.clear();
					done = true;
				}
				else
					mAbsoluteOffset.push_back(atoi(line));
			}
			
			//***1.85 - there should probably be more error checks above, but we assume
			// a successful load of the OldDatabase at this point
			mDBStatus = loaded;

		} //***001DB
		else
		{
			//***1.85 - don't think we can reach this, ever
			cout << "Some error occured during reading of the OldDatabase" << endl;
			mDBStatus = errorLoading;
		}
	}
}

//*******************************************************************
//
bool OldDatabase::isType(std::string filePath)
{
	// try to open file
	fstream testFile(filePath.c_str(), ios::in | ios::binary);

	if (!testFile)
	{
		printf("\nError locating or opening OldDatabase file!\n");
		return false;
	}

	testFile.seekg(0, ios::beg);
	
	unsigned long footerPos;
	
	// try to read from the file...
	if (!(testFile.read((char*)&footerPos, sizeof(unsigned long)))) 
	{
		printf("\nOldDatabase file is completely empty.\n");
		return false;
	} 


	//check for DB version
	unsigned int version;
    testFile.read((char*)&version, sizeof(int));

	//***1.85 - new magic number inclued "DB", program version and db version
	// IMPORTANT NOTE: take the code 0x4442073D
	// As an integer the code is interpretted as above with 4442 being the
	// ASCII digits "DB" in hex and 073D begin 1853 in decimal (or 1.85 & 3) for
	// the version numbers ...
	// HOWEVER, if version is treated as the address of a char array
	// the byte order acts as if reversed and the ASCII codes are ...
	// (version[0] == 3D, version[0] == 07, version[0] == 42, version[0] == 44)
	// be careful about how you look at specific positions.
	
	if (version != CURRENT_DBVERSION && (version & 0xFFFF0000) != 0x44420000) // the magic part is "DB"
	{
		printf("\nOld/Incompatible OldDatabase Version.\n");
		return false;
	}

	testFile.close();
	testFile.clear();

	return true;
}

//*******************************************************************
//

OldDatabase::~OldDatabase()
{
	mDbFile.close();
}


//*******************************************************************
//

bool OldDatabase::closeStream()
{
	mDbFile.close();
	mDbFile.clear(); // just in case closing closed file causes stream error
	return true;
}


//*******************************************************************
//

bool OldDatabase::openStream()
{
	mDbFile.open(mFilename.c_str(), ios::in | ios::out | ios::binary);
	return (! mDbFile.fail());
}

//*******************************************************************
//

void OldDatabase::createEmptyDatabase(Options *o)
{
	mFooterPos = 0;
	mDataSize = 0;
	mHeaderSize = 0;

	mDbFile.seekp(0, ios::beg);
	mDbFile.write((char*)&mFooterPos, sizeof(unsigned long));
	mHeaderSize += sizeof(unsigned long);

	//***1.85 - now we encode "DB" with the program version (VERSION) creating the
	// OldDatabase and the OldDatabase format version (CURRENT_DBVERSION) t time of creation
	unsigned int magicNum = 0x44420000; // "DB" as two most significant BYTES 66 & 68 in decimal

	//cout << "magicNum:" << hex << magicNum << endl;
	dec(cout);

	float pgmVersion = atof(VERSION);
	//cout << "program version: " << pgmVersion << endl;
	int dbVersion = CURRENT_DBVERSION;
	//cout << "OldDatabase version: " << dbVersion << endl;

	cout << "Creating new OldDatabase ... \n  \"" << o->mDatabaseFileName << "\"" << endl;
	cout << "  (program version " << VERSION << " : OldDatabase version " << CURRENT_DBVERSION << endl;
	// in the low two bytes the number (treated as an unsigned int) is
	// the OldDatabase version number (1 decimal digits) assumed to be in the
	// range 0..9 as a whole number ... and in the four next decimal 
	// digits ia the program version at the time of the OldDatabase creation
	// (assumed to be in the range 00.00 .. 99.99)

	// so format is DBabcde, where "DB" indicates OldDatabase file
	// ab.cd is the program version that created it, and e is the OldDatabase
	// format version used
	// as this goes into effect the code is DB01853 as the hex number 0x4442037D

	magicNum += (unsigned int)(pgmVersion * 1000 + dbVersion);

	//cout << "magicNum:" << hex << magicNum << endl;
	dec(cout);

	//mDbFile.write((char*)&version, sizeof(int));
	mDbFile.write((char*)&magicNum, sizeof(int)); //***1.85
	mHeaderSize += sizeof(int);

	//***1.85 - following now uses the current default catalog scheme 
	// (NOT the same as the scheme of the currently loaded OldDatabase)

	//mDbFile.write((char*)&o->mCatCategoryNamesMax, sizeof(int));
	int schemeId = o->mCurrentDefaultCatalogScheme;
	mDbFile.write((char*)&o->mDefinedCatalogCategoryNamesMax[schemeId], sizeof(int));
	mHeaderSize += sizeof(int);

	//for (int i = 0; i < o->mCatCategoryNamesMax; i++)
	for (int i = 0; i < o->mDefinedCatalogCategoryNamesMax[schemeId]; i++)
	{
		char temp[64];
		//sprintf(temp,"%s\n",o->mCatCategoryName[i].c_str());
		sprintf(temp,"%s\n",o->mDefinedCatalogCategoryName[schemeId][i].c_str());
		mDbFile.write((char*)&temp, strlen(temp));
		//mHeaderSize += strlen(temp) + 1; //***1.4 (+1 for '\n')
		//***1.65 - the +1 is not needed since the \n is already part of temp.
		// I do not have a clue why this did not cause problems until now - JHS
		mHeaderSize += strlen(temp);
	}

	int count = 0;
	mDbFile.write((char*)&count, sizeof(int)); // set number of OldDatabase entries to zero

	mFooterPos += mHeaderSize;

	mDbFile.seekp(0, ios::beg);
	mDbFile.write((char*)&mFooterPos, sizeof(unsigned long));

	mDbFile.flush();
}


//*******************************************************************
//

//bool OldDatabase::add(DatabaseFin<ColorImage> *data)
unsigned long OldDatabase::add(DatabaseFin<ColorImage> *data) //***1.85
{
#ifdef DEBUG
	std::cout << "Adding item to OldDatabase." << std::endl;
#endif
	try {
		int version = CURRENT_DBVERSION; //***001DB
		if (!mDbFile)
			return false;
	
		//***054 - assume that the filename for the image file contains path
		// information which must be stripped BEFORE saving fin in file and
		// BEFORE calculating the record size

		string shortFilename = data->mImageFilename;
		int pos = shortFilename.find_last_of(PATH_SLASH);
		if (pos >= 0)
		{
			shortFilename = shortFilename.substr(pos+1);
		}
		data->mImageFilename = shortFilename;

		mFooterPos = mDataSize + mHeaderSize; //***054
		unsigned long dataPos = mFooterPos;
		unsigned thisDataSize = data->getSizeInBytes();
	
		mDataSize += thisDataSize;
		mFooterPos += thisDataSize;
	
		mDbFile.seekp(0, ios::beg);
		mDbFile.write((char*)&mFooterPos, sizeof(unsigned long));

		mDbFile.seekp(dataPos, ios::beg);

		unsigned long numEntries = mNameList.size() + 1;
		data->mDataPos = dataPos;  //***001DB
		data->save(mDbFile);

		mDbFile.write((char*)&numEntries, sizeof(unsigned long));

		// Add this entry to the footer table
		char line[255];
		std::string temp;
	
		// I can't remember how to use strstream stuff right now,
		// so here're some sprintfs...
		if ("" == data->getName())
			sprintf(line, "%s %lu", "NONAME", dataPos);
		else
			sprintf(line, "%s %lu", data->getName().c_str(), dataPos);
	
		temp = line;
		mNameList.push_back(temp);

		if ("" == data->mIDCode)
			sprintf(line, "%s %lu", "NOID", dataPos);
		else
			sprintf(line, "%s %lu", data->mIDCode.c_str(), dataPos);

		temp = line;
		mIDList.push_back(temp);

		if ("" == data->mDateOfSighting)
			sprintf(line, "%s %lu", "NODATE", dataPos);
		else
			sprintf(line, "%s %lu", data->mDateOfSighting.c_str(), dataPos);

		temp = line;
		mDateList.push_back(temp);

		if ("" == data->mRollAndFrame)
			sprintf(line, "%s %lu", "NOROLL", dataPos);
		else
			sprintf(line, "%s %lu", data->mRollAndFrame.c_str(), dataPos);

		temp = line;
		mRollList.push_back(temp);

		if ("" == data->mLocationCode)
			sprintf(line, "%s %lu", "NOLOC", dataPos);
		else
			sprintf(line, "%s %lu", data->mLocationCode.c_str(), dataPos);

		temp = line;
		mLocationList.push_back(temp);

		if ("" == data->mDamageCategory)
			sprintf(line, "%s %lu", "NODAM", dataPos);
		else
			sprintf(line, "%s %lu", data->mDamageCategory.c_str(), dataPos);

		temp = line;
		mDamageList.push_back(temp);

		if ("" == data->mShortDescription)
			sprintf(line, "%s %lu", "NODESC", dataPos);
		else
			sprintf(line, "%s %lu", data->mShortDescription.c_str(), dataPos);

		temp = line;
		mDescriptionList.push_back(temp);

		//***1.3 - update absolute offset list
		mAbsoluteOffset.push_back(dataPos);

		//cout << "pushing " << dataPos << " on absOff list\n";

		writeFooter();		//***001DB replaced inline code 0.3.1-DB
		//return true;
		return dataPos; //***1.85 - return byte offset of added fin
	} catch (...) {
		throw;
	}
}


//*******************************************************************
//

DatabaseFin<ColorImage>* OldDatabase::getItemAbsolute(unsigned pos)
{
#ifdef DEBUG
	std::cout << "Getting item at absolute position " << pos << " from the OldDatabase." << std::endl;
#endif

	if (pos > this->mAbsoluteOffset.size())
	       throw BoundsError();


	if (!mDbFile) {
#ifdef DEBUG
		std::cout << "Problem with mDBFile: returning NULL." << std::endl;
#endif
		return NULL;
	}

	if (mAbsoluteOffset[pos] == -1)
		return NULL;                 // this is a HOLE, a previously deleted fin

	mDbFile.seekg(this->mAbsoluteOffset[pos]);

	DatabaseFin<ColorImage> *fin = new DatabaseFin<ColorImage>(mDbFile);

	return fin;
}

//*******************************************************************
//

DatabaseFin<ColorImage>* OldDatabase::getItem(unsigned pos)
{
#ifdef DEBUG
	std::cout << "Getting item at position " << pos << " from the OldDatabase." << std::endl;
#endif
	if (pos > this->size())
	       throw BoundsError();

	if (!mDbFile) {
#ifdef DEBUG
		std::cout << "Problem with mDBFile: returning NULL." << std::endl;
#endif
		return NULL;
	}

	//std::list<std::string>::iterator it;
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


//*******************************************************************
//
/*
DatabaseFin<ColorImage>* OldDatabase::getItemByName(std::string name)
{
#ifdef DEBUG
	std::cout << "Getting item named " << name << " from the OldDatabase." << std::endl;
#endif
	// std::list<std::string>::iterator it = mNameList.begin();
	std::vector<std::string>::iterator it = mNameList.begin();

	while (it != mNameList.end()) {
		istrstream inStream(it->c_str());	

		std::string thisName;
		inStream >> thisName;
		thisName = lowerCase(thisName);
		std::string tmpName = lowerCase(name);
		
		if (tmpName == thisName)
			return getItem(0, it);
		it++;
	}
	return NULL;
}

  */
/*
  again list & vector -- function not used
//*******************************************************************
//

ItemInfo* OldDatabase::getItemInfo(unsigned pos)
{
#ifdef DEBUG
	cout << "Getting Item Info" << endl;
#endif
	if (pos > this->size())
		throw BoundsError();
	
	if(!mDbFile)
		return NULL;

	std::list<std::string>::iterator it;

	if (mCurrentSort == DB_SORT_NAME)
		it = mNameList.begin();

	else if (mCurrentSort == DB_SORT_ID)
		it = mIDList.begin();

	else if (mCurrentSort == DB_SORT_DATE)
		it = mDateList.begin();

	else if (mCurrentSort == DB_SORT_ROLL)
		it = mRollList.begin();

	else if (mCurrentSort == DB_SORT_LOCATION)
		it = mLocationList.begin();

	else if (mCurrentSort == DB_SORT_DAMAGE)
		it = mDamageList.begin();

	else if (mCurrentSort == DB_SORT_DESCRIPTION)
		it = mDescriptionList.begin();

	else  // it's not a valid sort type
		return NULL;

	for (unsigned i = 0; i < pos; i++)
		it++;

	istrstream inStream(it->c_str());

	std::string filePos;
	inStream >> filePos >> filePos;

	// this stuff is bad... shouldn't know what's inside a dbfin
	mDbFile.seekg(atoi(filePos.c_str()), ios::beg);
        
	//***001DB - removed reading of image data

	unsigned int numPoints;
	mDbFile.read((char*)&numPoints, sizeof(unsigned int));

	mDbFile.seekg(numPoints * sizeof(Contour_point_t), ios::cur);

	int numPixRows;
	mDbFile.read((char*)&numPixRows, sizeof(int));
	char pixLine[2048];
	for (int i = 0; i < numPixRows; i++)
		mDbFile.getline(pixLine, 2048);
	
	char line[255];	
	mDbFile.getline(line, 255);
	std::string idcode = line;

	mDbFile.getline(line, 255);
	std::string name = line;

	mDbFile.getline(line, 255);
	std::string dateOfSighting = line;

	mDbFile.getline(line, 255);
	std::string rollAndFrame = line;

	mDbFile.getline(line, 255);
	std::string locationCode = line;

	mDbFile.getline(line, 255);
	std::string damageCategory = line;

	mDbFile.getline(line, 255);
	std::string shortDescription = line;
	
	ItemInfo *newInfo = new ItemInfo(
			idcode,
			name,
			dateOfSighting,
			rollAndFrame,
			locationCode,
			damageCategory,
			shortDescription
			);

	return newInfo;
}
*/

//*******************************************************************
//
// writeFooter()     //***001DB thru line 639
// RM 8-2002
// All this does is sort the lists, and write the footer
// NOTE : the write pointer in the buffer needs to be set
// before calling this function.
//

void OldDatabase::writeFooter(){
  
  	//mNameList.sort();
  	//mIDList.sort();
  	//mDateList.sort();
  	//mRollList.sort();
  	//mLocationList.sort();
  	//mDamageList.sort();
  	//mDescriptionList.sort();

	//***1.85 - they are vectors now so use the algorithms::sort()
	std::sort(mNameList.begin(),mNameList.end());
  	std::sort(mIDList.begin(),mIDList.end());
  	std::sort(mDateList.begin(),mDateList.end());
  	std::sort(mRollList.begin(),mRollList.end());
  	std::sort(mLocationList.begin(),mLocationList.end());
  	std::sort(mDamageList.begin(),mDamageList.end());
  	std::sort(mDescriptionList.begin(),mDescriptionList.end());
  
  	//std::list<std::string>::iterator it = mNameList.begin();
  	std::vector<std::string>::iterator it = mNameList.begin();
      
  	// Now, write out the footer
  	while (it != mNameList.end()) {
    		mDbFile.write(it->c_str(), it->length() + 1);
    		mDbFile.put('\n');
    
    		it++;
  	}
  
  	it = mIDList.begin();
  
  	while (it != mIDList.end()) {
    		mDbFile.write(it->c_str(), it->length() + 1);
    		mDbFile.put('\n');
    
    		it++;
  	}
  
  	it = mDateList.begin();
  
  	while (it != mDateList.end()) {
    		mDbFile.write(it->c_str(), it->length() + 1);
    		mDbFile.put('\n');
   	 
    		it++;
  	}
  
  	it = mRollList.begin();
  
  	while (it != mRollList.end()) {
    		mDbFile.write(it->c_str(), it->length() + 1);
    		mDbFile.put('\n');
    
    		it++;
  	}
  
  	it = mLocationList.begin();
  
  	while (it != mLocationList.end()) {
    		mDbFile.write(it->c_str(), it->length() + 1);
    		mDbFile.put('\n');
    
    		it++;
  	}
  
  	it = mDamageList.begin();
  
  	while (it != mDamageList.end()) {
    		mDbFile.write(it->c_str(), it->length() + 1);
    		mDbFile.put('\n');
      
    		it++;
  	}
  
  	it = mDescriptionList.begin();
  
  	while (it != mDescriptionList.end()) {
    		mDbFile.write(it->c_str(), it->length() + 1);
    		mDbFile.put('\n');

    		it++;
  	}

	//***1.3 - write new absolute offset list
	for (int i = 0; i < mAbsoluteOffset.size(); i++)
	{
		char line[256];
		sprintf(line,"%ld\n",mAbsoluteOffset[i]);
		string temp = line;
		mDbFile.write(temp.c_str(), temp.length());
	}

  	return;
}


//*******************************************************************
//
// ***002DB through line 936
//  This function deletes all occurances of *Fin, from all sorted lists
// It also updates the data position of the fin in the DB File and list
// if applicable (Only fins that occur after the deleted fin..)
//

void OldDatabase::DeleteFinFromList(DatabaseFin<ColorImage> *Fin)
{
	unsigned long FinPos = Fin->mDataPos;
	unsigned long FinSize= Fin->getSizeInBytes();
  
	unsigned long numEntries = mNameList.size();
		
	unsigned i, delPos;
	unsigned long thisDataPos;

  
  
	std::string filePos;
	std::string temp;
	std::string str; //***005DB
	char line [255];
  
	//std::list<std::string>::iterator it;
	std::vector<std::string>::iterator it;

	istrstream *inStream; 
  
	// remove fin from NAME list

	it  = mNameList.begin();
	for (i = 0; i < numEntries; i++,it++)
	{
		// Create a new string stream and strip the Fin's position off it.
		inStream = new istrstream(it->c_str());
		//***005DB next 9 lines allow for multi-word entries
		temp = "";
		filePos = "";
		while (*inStream >> str) 
		{ 
			if (temp == "")
				temp = filePos;
			else
		  		temp += (" " + filePos);
			filePos = str;
		}
    
		thisDataPos= (atoi(filePos.c_str()));
    
		// If the data positions match up, we want to mark it for deletion
		// Otherwise, only if the fin occurs later in the file, we want to
		// update the data position by subtracting the size of the deleted fin
		// in both the list AND the DB file.

		if(thisDataPos == FinPos)
		{
			delPos = i;
		}
		else if (thisDataPos > FinPos)
		{
			mDbFile.seekp(thisDataPos);
			thisDataPos = thisDataPos - FinSize;
			mDbFile.write((char*)&thisDataPos,sizeof(unsigned long));
			sprintf(line, "%s %lu", temp.c_str(), thisDataPos);
			temp = line;
			*it = temp;
		}
		delete inStream; // Free up the stream..
	}

	// now, actually delete it from the list..
	it = mNameList.begin();
	for (i = 0; i < delPos; i++)
		it++;
	mNameList.erase(it);

	// Remove fin from ID list

	it = mIDList.begin();
	for (i = 0; i < numEntries; i++, it++)
	{
		inStream = new istrstream(it->c_str());
		//***005DB next 9 lines allow for multi-word entries
		temp = "";
		filePos = "";
		while (*inStream >> str) 
		{ 
			if (temp == "")
				temp = filePos;
			else
		  		temp += (" " + filePos);
			filePos = str;
		}
		thisDataPos = (atoi(filePos.c_str()));
  
		if(thisDataPos == FinPos)
		{
			delPos = i;
		}
		else if (thisDataPos > FinPos)
		{
			mDbFile.seekp(thisDataPos);
			thisDataPos = thisDataPos-FinSize;
			mDbFile.write((char*)&thisDataPos,sizeof(unsigned long));
			sprintf(line, "%s %lu", temp.c_str(), thisDataPos);
			temp = line;
			*it = temp;
     
		}
		delete inStream;
	}
  
	it = mIDList.begin();
	for (i = 0; i < delPos; i++)
		it++;
	mIDList.erase(it);
  
	// Remove fin from DATA list

	it= mDateList.begin();
	for (i = 0; i < numEntries; i++,it++)
	{
		inStream = new istrstream(it->c_str());
		//***005DB next 9 lines allow for multi-word entries
		temp = "";
		filePos = "";
		while (*inStream >> str) 
		{ 
			if (temp == "")
				temp = filePos;
			else
		  		temp += (" " + filePos);
			filePos = str;
		}

		thisDataPos = (atoi(filePos.c_str()));
		if (thisDataPos == FinPos)
		{
			delPos = i;
		}
		else if (thisDataPos > FinPos)
		{
			mDbFile.seekp(thisDataPos);
			thisDataPos = thisDataPos-FinSize;
			mDbFile.write((char*)&thisDataPos,sizeof(unsigned long));
			sprintf(line, "%s %lu", temp.c_str(), thisDataPos);
			temp = line;
			*it = temp;
		}
		delete inStream;
	}
	it = mDateList.begin();
	for (i = 0; i <  delPos; i++)
		it++;
	mDateList.erase(it);
 
	// delete fin from ROLL/FRAME list

	it= mRollList.begin();
	for (i = 0; i < numEntries; i++,it++)
	{
		inStream = new istrstream(it->c_str()); 
		//***005DB next 9 lines allow for multi-word entries
		temp = "";
		filePos = "";
		while (*inStream >> str) 
		{ 
			if (temp == "")
				temp = filePos;
			else
		  		temp += (" " + filePos);
			filePos = str;
		}
    
		thisDataPos = (atoi(filePos.c_str()));
		if (thisDataPos == FinPos)
		{
			delPos = i;
		}
		else if (thisDataPos > FinPos)
		{
			mDbFile.seekp(thisDataPos);
			thisDataPos = thisDataPos-FinSize;
			mDbFile.write((char*)&thisDataPos,sizeof(unsigned long));
			sprintf(line, "%s %lu", temp.c_str(), thisDataPos);
			temp = line;
			*it = temp;
		}
		delete inStream;
	}
	it = mRollList.begin();
	for (i = 0; i < delPos; i++)
		it++;
	mRollList.erase(it);
  
	// remove fin from LOCATION list

	it= mLocationList.begin();
	for (i = 0; i < numEntries; i++,it++)
	{
		inStream = new istrstream(it->c_str());
		//***005DB next 9 lines allow for multi-word entries
		temp = "";
		filePos = "";
		while (*inStream >> str) 
		{ 
			if (temp == "")
				temp = filePos;
			else
		  		temp += (" " + filePos);
			filePos = str;
		}
    
		thisDataPos = (atoi(filePos.c_str()));
		if (thisDataPos == FinPos)
		{
			delPos = i;
		}
		else if (thisDataPos > FinPos)
		{
			mDbFile.seekp(thisDataPos);
      
			thisDataPos = thisDataPos-FinSize;
			mDbFile.write((char*)&thisDataPos,sizeof(unsigned long));
     
			sprintf(line, "%s %lu", temp.c_str(), thisDataPos);
			temp = line;
			*it = temp;
		}
		delete inStream;
	}
	it = mLocationList.begin();
	for (i = 0; i < delPos; i++)
		it++;
	mLocationList.erase(it);

	// remove fin from DAMAGE Category list
 
	it = mDamageList.begin();
	for (i = 0; i < numEntries; i++,it++)
	{
		inStream = new istrstream(it->c_str());
		//***005DB next 9 lines allow for multi-word entries
		temp = "";
		filePos = "";
		while (*inStream >> str) 
		{ 
			if (temp == "")
				temp = filePos;
			else
		  		temp += (" " + filePos);
			filePos = str;
		}
   
		thisDataPos = (atoi(filePos.c_str()));
		if (thisDataPos == FinPos)
		{
			delPos = i;
		}
		else if (thisDataPos > FinPos)
		{
			mDbFile.seekp(thisDataPos);
			thisDataPos = thisDataPos-FinSize;
			mDbFile.write((char*)&thisDataPos,sizeof(unsigned long));
			if ("" == temp)
				sprintf(line, "%s %lu", "NONAME", thisDataPos);
			else
				sprintf(line, "%s %lu", temp.c_str(), thisDataPos);
			temp = line;
			*it = temp;
		}
		delete inStream;
	}

	it = mDamageList.begin();
	for (i = 0; i < delPos; i++)
		it++;
	mDamageList.erase(it);

	// remove fin from DESCRIPTION list

	it = mDescriptionList.begin();
	for (i = 0; i < numEntries; i++,it++)
	{ 
		inStream = new istrstream(it->c_str());
		//***005DB next 9 lines allow for multi-word entries
		temp = "";
		filePos = "";
		while (*inStream >> str) 
		{ 
			if (temp == "")
				temp = filePos;
			else
		  		temp += (" " + filePos);
			filePos = str;
		}
     
		thisDataPos = (atoi(filePos.c_str()));
     
		if (thisDataPos == FinPos)
		{
			delPos = i;
		}
		else if (thisDataPos > FinPos)
		{
			mDbFile.seekp(thisDataPos);
       
			thisDataPos=thisDataPos-FinSize;
			mDbFile.write((char*)&thisDataPos,sizeof(unsigned long));
     
			if ("" == temp)
				sprintf(line, "%s %lu", "NONAME", thisDataPos);
			else
				sprintf(line, "%s %lu", temp.c_str(), thisDataPos);
			temp = line;
			*it = temp;
		}
		delete inStream;
	}

	it = mDescriptionList.begin();
	for (i = 0; i < delPos; i++)
		it++;
	mDescriptionList.erase(it);


	//***1.3 - update new absolute offset list

	// mark deleted fin with a HOLE (-1 value) in absolute offset list
	for (i=0; i < mAbsoluteOffset.size(); i++)
		if (mAbsoluteOffset[i] == FinPos)
			mAbsoluteOffset[i] = -1; // mark it as a HOLE (in the list)
		else if (mAbsoluteOffset[i] > (long int)FinPos)
			mAbsoluteOffset[i] = mAbsoluteOffset[i] - (long int)FinSize;

	// NOTE: there is no need to write anything to the file here.  The writeFooter()
	// function will take care of that when called from delete().

}


//*******************************************************************
//
// void OldDatabase::Delete(DatabaseFin<ColorImage> *Fin)
//
//    This function handles all the tasks needed to delete a file from the DB
// and rewrite it. 
//

void OldDatabase::Delete(DatabaseFin<ColorImage> *Fin){
  
	//***054 - revise size calculation to use short filename (without path)
	// the short file name (without path is what is actually stored in the OldDatabase file
	// however, the Fin->mImageFilename member variable actually contains the FULL
	// path and filename. Correct the filename BEFORE calculating the DatabaseFin
	// record size or the deletionwill not be correct.
		
	string shortFilename = Fin->mImageFilename;
	int pos = shortFilename.find_last_of(PATH_SLASH);
	if (pos >= 0)
	{
		shortFilename = shortFilename.substr(pos+1);
	}
	Fin->mImageFilename = shortFilename;

	// First delete Fin from the list ..  
	DeleteFinFromList(Fin);

	unsigned long numEntries = mNameList.size();
  
	if(numEntries == 0)
	{
		//***1.6DB
		//mDbFile.close();
		//mDbFile.open(mFilename.c_str(), ios::in | ios::out | ios::binary | ios::trunc);

		//***1.6DB
		// we cannot just wipe the file clean here anymore. The header and absolute
		// offset lists should be preserved. - JHS
    
		//return;
	}
  
	unsigned long finSize = Fin->getSizeInBytes();
	mDataSize -= finSize;
	mFooterPos -= finSize; // set new footer position after fin deleted
	unsigned long size1 = Fin->mDataPos;
	unsigned long size2 = (mFooterPos - Fin->mDataPos); //***005DB

	// For efficiency, we want to do two black copies, leaving out
	// only the fin. 
	char *temp1 = new char[(size1+1)];
	char *temp2 = new char[(size2+1)];
	mDbFile.seekg(0 , ios::beg);
  
	mDbFile.read(temp1 , size1);

	mDbFile.seekg((Fin->mDataPos + finSize),ios::beg);
	mDbFile.read( temp2 , size2);
	mDbFile.close();

	mDbFile.open(mFilename.c_str(), ios::in | ios::out | ios::binary | ios::trunc);

	mDbFile.seekp(0,ios::beg);
	mDbFile.write( temp1 , size1);
	mDbFile.write( temp2, size2);
	mDbFile.seekp(0,ios::beg);
	mDbFile.write((char*)&mFooterPos,sizeof(unsigned long));
 
	mDbFile.seekp(mFooterPos);
	mDbFile.write((char*)&numEntries, sizeof(unsigned long));
	writeFooter();

	delete [] temp1; //***1.0LK
	delete [] temp2; //***1.0LK
}

/*
//*******************************************************************
//

void OldDatabase::sort(db_sort_t sortBy)
{
	mCurrentSort = sortBy;
}
*/

//*******************************************************************
//

//DatabaseFin<ColorImage>* OldDatabase::getItem(unsigned pos, std::list<std::string>::iterator it)
DatabaseFin<ColorImage>* OldDatabase::getItem(unsigned pos, std::vector<std::string> *theList)
{
	//for (unsigned i = 0; i < pos; i++)
	//	it++;

	istrstream inStream((*theList)[pos].c_str());

	std::string prev, cur;
	
	// we'll assume the last token in the stream is the position in
	// the file
	while (inStream >> cur)
		prev = cur;

	mDbFile.seekg(atoi(prev.c_str()), ios::beg);

	DatabaseFin<ColorImage> *fin = new DatabaseFin<ColorImage>(mDbFile);

	return fin;
}



////////////////////////////////////////////////////////////////////////
////////////////////////***1.85 new***////////////////////////////////

//*******************************************************************
//
/*
string OldDatabase::getItemEntryFromList(db_sort_t whichList, unsigned pos)
{
	if (pos > this->size())
	       throw BoundsError();

	if (!mDbFile) {
		return NULL;
	}

	//using OldDatabase::db_sort_t;

	//std::list<std::string>::iterator it;
	std::vector<std::string> *it;

	switch (whichList)
	{
	case DB_SORT_NAME :
		//it = mNameList.begin();
		it = &mNameList;
		break;
	case DB_SORT_ID :
		///it = mIDList.begin();
		it = &mIDList;
		break;
	case DB_SORT_DATE :
		//it = mDateList.begin();
		it = &mDateList;
		break;
	case DB_SORT_ROLL :
		//it = mRollList.begin();
		it = &mRollList;
		break;
	case DB_SORT_LOCATION :
		//it = mLocationList.begin();
		it = &mLocationList;
		break;
	case DB_SORT_DAMAGE :
		//it = mDamageList.begin();
		it = &mDamageList;
		break;
	case DB_SORT_DESCRIPTION :
		//it = mDescriptionList.begin();
		it = &mDescriptionList;
		break;
	default : // it's not a valid sort type
		return "";
	}

	//for (unsigned i = 0; i < pos; i++)
	//	it++;

	//return *it;
	return (*it)[pos];
}

//*******************************************************************
//

int OldDatabase::getItemListPosFromOffset(db_sort_t whichList, string item)
{

	if (!mDbFile) {
		return NULL;
	}

	//std::list<std::string>::iterator it, last;
	std::vector<std::string>::iterator it, last;

	switch (whichList)
	{
	case DB_SORT_NAME :
		it = mNameList.begin();
		last = mNameList.end();
		break;
	case DB_SORT_ID :
		it = mIDList.begin();
		last = mIDList.end();
		break;
	case DB_SORT_DATE :
		it = mDateList.begin();
		last = mDateList.end();
		break;
	case DB_SORT_ROLL :
		it = mRollList.begin();
		last = mRollList.end();
		break;
	case DB_SORT_LOCATION :
		it = mLocationList.begin();
		last = mLocationList.end();
		break;
	case DB_SORT_DAMAGE :
		it = mDamageList.begin();
		last = mDamageList.end();
		break;
	case DB_SORT_DESCRIPTION :
		it = mDescriptionList.begin();
		last = mDescriptionList.end();
		break;
	default : // it's not a valid sort type
		return NOT_IN_LIST;
	}

	string itemOffset = item.substr(1 + item.rfind(" "));

	bool found(false);
	int posit(0);

	while ((!found) && (it != last))
	{
		std::string offset = *it;
		offset = offset.substr(1 + offset.rfind(" ")); // keep just the offset
		if (itemOffset == offset)
			found = true;
		else
		{
			++it;
			posit++;
		}
	}
	if (! found)
		posit = NOT_IN_LIST;

	return posit;
}
*/