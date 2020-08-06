//*******************************************************************
//   file: MatchResults.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (1/24/2008)
//         -- comment blocks added
//
//*******************************************************************

#include <ctype.h>
#include "../interface/ErrorDialog.h" //***1.9
#include "MatchResults.h"
#include "../DatabaseFin.h"
#include "../Error.h"
#include "../utility.h"
#include "../mapContour.h" //***1.1
#include "../CatalogSupport.h"


#include <iomanip>

using namespace std;

//*******************************************************************
//***1.5
//
bool MatchResults::LastSortedByError()
{
	return (mLastSortBy == MR_ERROR);
}

//*******************************************************************
//***1.5
//
//  This should only be called once -- after all results are added to list, and before results 
//  are displayed.  This is probably best done from the MatchResults constructor.
//
void MatchResults::setRankings()
{
	if (mLastSortBy != MR_ERROR)
		return;
			
	list<Result>::iterator it = mResults.begin();
	//for (int i=0; i<mResults.size(); i++, it++)
	for (int i = 0; it != mResults.end(); i++, it++) //***2.22 - better to limit with iterator
	{
		//***2.22 - I have no idea why this worked before.  alocate 5 chars
		// write 5 chars "%4d " AND an additional '\0' and then go back and
		// put a '\0' in position 4.  This overwrites the char array by one position
		//char rank[5];
		//sprintf(rank, "%4d ",i+1);
		//rank[4] = '\0';
		
		char rank[6]; //***2.22 - this fixes it
		sprintf(rank, "%4d ",i+1);

		it->setRank(rank);
	}
}

//*******************************************************************
//
//
void MatchResults::addResult(const Result& r)
{
	try {
		mResults.push_back(r);
		//sort(); //***1.5 - DO NOT sort here, only sort once AFTER list is built
	} catch (...) {
		throw;
	}
}

//*******************************************************************
//
//
void MatchResults::sort(mr_sort_t sortBy, int &active)
{
	try {
		//***1.0 - keep track of where active result ends up and 
		// reset value of active so redisplay of lists and icons
		// has correct active result after sort
		int newActive = -1;

		mLastSortBy = sortBy;

		list<Result> sortedResults;

		//***1.0 - need list of indices since mResults list is erased as we go
		list<int> actIndex;
		for (int i=0; i<mResults.size(); i++)
			actIndex.push_back(i);

		while (mResults.size() > 0) {
			list<Result>::iterator it = mResults.begin();
			list<Result>::iterator saveIt = it;

			list<int>::iterator actIt = actIndex.begin(); //***1.0
			list<int>::iterator actItLow = actIt;         //***1.0

			string lowest;
			float lowestNum;
			switch (sortBy) {
				case MR_ERROR:
					lowestNum = atof(it->getError().c_str());
					break;
				case MR_NAME:
					lowest = it->getName();
					break;
				case MR_IDCODE:
					lowest = it->getIdCode();
					break;
				case MR_DAMAGE:
					lowest = it->getDamage();
					break;
				case MR_DATE:
					lowest = it->getDate();
					break;
				case MR_LOCATION:
					lowest = it->getLocation();
					break;
			}
	
			++it;

			++actIt; //***1.0

			while (it != mResults.end()) {
				string compare;
				float compareNum;
				switch (sortBy) {
					case MR_ERROR:
						compareNum = atof(it->getError().c_str());
						break;
					case MR_NAME:
						compare = it->getName();
						break;
					case MR_IDCODE:
						compare = it->getIdCode();
						break;
					case MR_DAMAGE:
						compare = it->getDamage();
						break;
					case MR_DATE:
						compare = it->getDate();
						break;
					case MR_LOCATION:
						compare = it->getLocation();
						break;
				}

				if (sortBy == MR_ERROR && compareNum < lowestNum) {
					lowestNum = compareNum;
					saveIt = it;
					actItLow = actIt; //***1.0
				} else if (compare < lowest) {
					lowest = compare;
					saveIt = it;
					actItLow = actIt; //***1.0
				}

				++it;

				++actIt; //***1.0
			}

			sortedResults.push_back(*saveIt);
			mResults.erase(saveIt);

			//***1.0 - save new position of old active Result
			if (((*actItLow) == active) && (newActive == -1))
				newActive = sortedResults.size() - 1;
			actIndex.erase(actItLow);
		}

		mResults = sortedResults;

		active = newActive; //***1.0
	} catch (...) {
		throw;
	}
}

//*******************************************************************
//
//
Result* MatchResults::getResultNum(int resultNum)
{
	if (resultNum >= (int)mResults.size())
		throw Error("Request for item out of bounds in MatchResults::getResultNum");	
	try {
		list<Result>::iterator it = mResults.begin();

		for (int i = 0; i < resultNum; i++)
			++it;

		return &(*it);

	} catch (...) {
		throw;
	}
}

//*******************************************************************
//
//
void MatchResults::setTimeTaken(float timeTaken)
{
	mTimeTaken = timeTaken;
}

//*******************************************************************
//
//
float MatchResults::getTimeTaken()
{
	return mTimeTaken;
}

//*******************************************************************
//
//
int MatchResults::findRank()
{
	for (int i = 0; i < (int)mResults.size(); i++) {
		Result *r = this->getResultNum(i);

		string rID = r->getIdCode();

		if (caseInsensitiveStringCompare(rID, mFinID))
			return i + 1;
	}
	
	return -1;
}

//*******************************************************************
//
//
void MatchResults::save(std::string fileName)
{
	try {
		ofstream outFile(fileName.c_str());

		if (!outFile)
		{
			throw Error("Problem writing to file: " + fileName
					+ "\n In MatchResults::save()");
		}

		if (mFinID != "") {
			outFile << "Results for ID: " << mFinID << endl;
			outFile << "fin FILE: " << this->mTracedFinFile << endl;
			outFile << " db FILE: " << this->mDatabaseFile << endl;

			int rank = this->findRank();

			if (rank == -1)
				outFile << "ID does not match any in the results list." << endl;
			else
				outFile << "The ID is ranked " << rank << endl;
		}

		if (mTimeTaken > 0.0)
			outFile << "Match Time: " << mTimeTaken << endl << endl;

		outFile << " Rank\tError\tID\tDBPosit\tunkBegin\tunkTip\tunkEnd\tdbBegin\tdbTip\tdbEnd\tDamage\n";     
		outFile << "_____________________________________________________________________\n";

		for (int i = 0; i < (int)mResults.size(); i++) {

			Result *r = this->getResultNum(i);
			int 
				uBegin,uTip,uEnd,
				dbBegin,dbTip,dbEnd;

			r->getMappingControlPoints(uBegin,uTip,uEnd,dbBegin,dbTip,dbEnd);

			outFile << "  " << i + 1
				<< "\t" << r->getError()
				<< "\t" << r->getIdCode()
				<< "\t" << r->getPosition()
				<< "\t" << uBegin
				<< "\t" << uTip
				<< "\t" << uEnd
				<< "\t" << dbBegin
				<< "\t" << dbTip
				<< "\t" << dbEnd
				<< "\t" << r->getDamage()
				<< endl;
		}
		
	} catch (...) {
		throw;
	}
}

//*******************************************************************
//
//
DatabaseFin<ColorImage> *MatchResults::load(Database *db, std::string fileName)
{
	try {

		ifstream inFile(fileName.c_str());

		if (!inFile)
			throw Error("Problem reading from file: " + fileName
					+ "\n In MatchResults::load()");

		string line = "";
		int pos;
		
		getline(inFile,line);
		pos = line.find(":") + 2; // position of finID
		mFinID = line.substr(pos);
		//cout << "finID[" << mFinID << "]\n";

		string test = line.substr(0,pos-1);
		if (test != "Results for ID:")
			return NULL;

		getline(inFile,line);
		pos = line.find(":") + 2; // position of fin filename
		mTracedFinFile = line.substr(pos);
		//cout << "finFilename[" << mTracedFinFile << "]\n";

		//***1.85 - do this AFTER making sure correct database is loaded
		//DatabaseFin<ColorImage> *unkFin = new DatabaseFin<ColorImage>(mTracedFinFile);

		getline(inFile,line);
		pos = line.find(":") + 2; // position of database filename
		mDatabaseFile = line.substr(pos);

		//***2.2 - if we can, determine if database path has just changed drive letter
		// or some part of path that is a prefix to the SurveyArea.  If the database
		// is in the same SurveyArea and has the same database name, then we can
		// proceed with the building of the MatchResults
		int p = mDatabaseFile.find("surveyAreas"); 
		string rqdAreaAndDB = mDatabaseFile.substr(p);  // survey area and database name
		string rqdPreamble = mDatabaseFile.substr(0,p);    // strip it to get preamble

		string currentDBFile = db->getFilename();
		p = currentDBFile.find("surveyAreas"); 
		string currentAreaAndDB = currentDBFile.substr(p); // survey area and catalog name
		string currentPreamble = currentDBFile.substr(0,p); // strip it to get preamble

		//***1.85 - if database used in match is not currently loaded, abort quietly  
		if (db->getFilename() != mDatabaseFile)
		{
			//cout << mDatabaseFile << endl;
			//cout << rqdPreamble << "   " << rqdAreaAndDB << endl;
			//cout << currentDBFile << endl;
			//cout << currentPreamble << "   " << currentAreaAndDB << endl;

			if (currentAreaAndDB == rqdAreaAndDB)
			{
				string msg = "The Survey Area and Catalog for the match results appear corrrect,\n";
				msg += "but the path to DARWIN's home folder seems to have changed.\n";
				msg += "Is it OK to open the indicated catalog in the current location\n";
				msg += "as shown below?\n\n";
				msg += (currentPreamble + rqdAreaAndDB);

				cout << msg << endl;
				//ErrorDialog *err = new ErrorDialog(msg);
				//err->show();

				//***2.2 - path possibly has to be fixed to FIN file as well
				if ((currentDBFile != mDatabaseFile) && (currentAreaAndDB == rqdAreaAndDB))
				{
					//add preamble of current DARWINHOME to FINs relative location
					p = mTracedFinFile.find("surveyAreas");
					string currentFinAreaPlus = mTracedFinFile.substr(p);
					mTracedFinFile = currentPreamble + currentFinAreaPlus;
					cout << mTracedFinFile << endl;
				}
			}
			else
			{
				string msg = "The WRONG database is currently loaded for viewing these results ...\n\n";
				msg += "LOADED DB:\n    " + db->getFilename() + "\n\n";
				msg += "REQUIRED DB:\n    " + mDatabaseFile + "\n\n";
				msg += "Please load the required database from the main window\n";
				msg += "and then reload the desired results file.";

				//ErrorDialog *err = new ErrorDialog(msg);
				//err->show();
				//***2.22 - replacing own ErrorDialog with GtkMessageDialogs
				GtkWidget *errd = gtk_message_dialog_new (NULL,
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_ERROR,
										GTK_BUTTONS_CLOSE,
										msg.c_str());
				gtk_dialog_run (GTK_DIALOG (errd));
				gtk_widget_destroy (errd);
				return NULL;
			}
		}
		
		DatabaseFin<ColorImage> *unkFin;
		//***1.85 - NOW create the database fin for the unknown
		if(mTracedFinFile.rfind(".finz") == string::npos)
			unkFin = new DatabaseFin<ColorImage>(mTracedFinFile);
		else
			unkFin = openFinz(mTracedFinFile);


		getline(inFile,line); // skip Ranking
		getline(inFile,line); // skip headers
		getline(inFile,line); // skip line separator

		// get match info on each matched database fin
		while (getline(inFile,line))
		{

			pos = line.find("\t");
			string rank = line.substr(0,pos); 
			line = line.substr(pos+1);

			pos = line.find("\t");
			string error = line.substr(0,pos);
			line = line.substr(pos+1);

			pos = line.find("\t");
			string dbFinID = line.substr(0,pos);
			line = line.substr(pos+1);

			cout << "dbFinID[" << dbFinID << "]"; //*** 2.2 - show for now


			string numStr;
			int 
				dbFinPosition,
				uBegin,uTip,uEnd,
				dbBegin,dbTip,dbEnd;

			pos = line.find("\t");
			numStr = line.substr(0,pos);
			line = line.substr(pos+1);
			dbFinPosition = atoi(numStr.c_str());
			cout << "[" << dbFinPosition << "]" << endl; //*** 2.2 - show for now 

			pos = line.find("\t");
			numStr = line.substr(0,pos);
			uBegin = atoi(numStr.c_str());
			line = line.substr(pos+1);
			//cout << "[" << uBegin << "]";

			pos = line.find("\t");
			numStr = line.substr(0,pos);
			uTip = atoi(numStr.c_str());
			line = line.substr(pos+1);
			//cout << "[" << uTip << "]";

			pos = line.find("\t");
			numStr = line.substr(0,pos);
			uEnd = atoi(numStr.c_str());
			line = line.substr(pos+1);
			//cout << "[" << uEnd << "]";

			pos = line.find("\t");
			numStr = line.substr(0,pos);
			dbBegin = atoi(numStr.c_str());
			line = line.substr(pos+1);
			//cout << "[" << dbBegin << "]";

			pos = line.find("\t");
			numStr = line.substr(0,pos);
			dbTip = atoi(numStr.c_str());
			line = line.substr(pos+1);
			//cout << "[" << dbTip << "]";

			pos = line.find("\t");
			numStr = line.substr(0,pos);
			dbEnd = atoi(numStr.c_str());
			line = line.substr(pos+1);
			//cout << "[" << dbEnd << "]";

			string damage = line;
			//cout << "[" << damage << "]" << endl;

			line = "";
	
			DatabaseFin<ColorImage> *thisDBFin = db->getItemAbsolute(dbFinPosition);

			if (NULL == thisDBFin)
			{
				cout << "Skipping matched fin that has been DELETED from the database!\n";
				continue;
			}

			if (thisDBFin->getID() != dbFinID)
				cout << "Disaster " << thisDBFin->getID() 
				     << " " << dbFinID << "\n";

			FloatContour *mappedUnknownContour = mapContour(
					unkFin->mFinOutline->getFloatContour(),
					(*(unkFin->mFinOutline->getFloatContour()))[uTip],
					(*(unkFin->mFinOutline->getFloatContour()))[uBegin],
					(*(unkFin->mFinOutline->getFloatContour()))[uEnd],
					(*(thisDBFin->mFinOutline->getFloatContour()))[dbTip],
					(*(thisDBFin->mFinOutline->getFloatContour()))[dbBegin],
					(*(thisDBFin->mFinOutline->getFloatContour()))[dbEnd]);

			Result r(
					mappedUnknownContour,                      //***1.3 - Mem Leak - constructor make copy now
					thisDBFin->mFinOutline->getFloatContour(), //***1.3 - Mem Leak - constructor make copy now
					thisDBFin->mImageFilename,
					thisDBFin->mThumbnailPixmap,
					thisDBFin->mThumbnailRows,
					dbFinPosition, // position of fin in database
					error,
					thisDBFin->mIDCode,
					thisDBFin->mName,
					thisDBFin->mDamageCategory,
					thisDBFin->mDateOfSighting,
					thisDBFin->mLocationCode);

			r.setMappingControlPoints(
					uBegin,uTip,uEnd,  // beginning, tip & end of unknown fin
					dbBegin,dbTip,dbEnd); // beginning, tip & end of database fin

			addResult(r);

			delete mappedUnknownContour; //***1.3 - Mem Leak
			delete thisDBFin;
		}

		//***1.5 - moved sort here after list built, rather than calling from inside addResult()
		//sort(); //***1.5 - results in the file are already sorted

		return unkFin;


	} catch (...) {

		return NULL;
		throw;
	}
}
