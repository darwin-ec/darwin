//*******************************************************************
//   file: MatchingQueue.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (9/29/2005)
//         -- code to allow use of Options to specify catalog categories
//            for matching
//
//*******************************************************************

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include "Error.h"
#include <iostream>
#include <fstream>

#include "Match.h"
#include "MatchResults.h"
#include "MatchingQueue.h"

#ifdef WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "/"
#endif

using namespace std;

MatchingQueue::MatchingQueue(Database *d, Options *o)
	:
		mFinDatabase(d),
		mOptions(o),
		mUnknownFin(NULL),
		mMatcher(NULL),
		mResults(NULL)
{ }

MatchingQueue::~MatchingQueue()
{
	mFileNames.clear();

	if (NULL != mUnknownFin)
		delete mUnknownFin;

	if (NULL != mMatcher)
		delete mMatcher;	
	
	//***1.3 - mResults is just a pointer to the MatchResults member of
	// mMatcher so do NOT delete it here
}

void MatchingQueue::add(string fileName)
{
	mFileNames.push_back(fileName);
}

void MatchingQueue::setupMatching()
{
	mNumNoID = 0;		
	mSum = 0;
	mNumTopTen = 0;	
	mNumID = 0;	
	mNumValidTimes = 0;
	mNumInvalidTimes = 0;
	mWorstRank = 0; 
	mBestRank = 0;
	mTotalTime = 0.0;
	mFirstRun = true;

	mCurrentFinID = -1; // start prior to first unknown fin in list
}

Match *MatchingQueue::getNextUnknownToMatch()
{
	if (NULL != mMatcher)
	{
		delete mMatcher;
		mMatcher = NULL;
	}

	if (NULL != mUnknownFin)
	{
		delete mUnknownFin;
		mUnknownFin = NULL;
	}

	mCurrentFinID++;

	if (mCurrentFinID >= (int)mFileNames.size())
		return NULL;

	//mUnknownFin = this->getItemNum(mCurrentFinID); replaced
	string tracedFinFilename = this->getItemNum(mCurrentFinID); //***1.1
	
	if(string::npos == tracedFinFilename.rfind(".finz"))
	{
		if (isTracedFinFile(tracedFinFilename))
			mUnknownFin = new DatabaseFin<ColorImage>(tracedFinFilename); //***1.1
	}
	else
		mUnknownFin = openFinz(tracedFinFilename);

	if (NULL == mUnknownFin)
		return NULL;         //***2.0 - in case .finz or .fin file was corrupt

	mMatcher = new Match(mUnknownFin, mFinDatabase, mOptions);
	mResults = mMatcher->getMatchResults();
	mResults->setFinFilename(tracedFinFilename); //***1.1
	mResults->setDatabaseFilename(mOptions->mDatabaseFileName); //***1.1

	return mMatcher;
}

Match *MatchingQueue::getCurrentUnknownToMatch()
{
	return mMatcher; // will be NULL if mCurrentFinID == -1
}


MatchResults * MatchingQueue::getMatchResults()
{
	return mMatcher->getMatchResults();
}


float MatchingQueue::matchProgress()
{
	return ((float)mCurrentFinID / (int)mFileNames.size());
}

void MatchingQueue::finalizeMatch()
{
	int rank = mResults->findRank();

	if (rank == -1)
		mNumNoID++;
	else {
		if (mFirstRun) {
			mFirstRun = false;
			mWorstRank = mBestRank = rank;
		} else {
			if (rank < mBestRank)
				mBestRank = rank;
			if (rank > mWorstRank)
				mWorstRank = rank;
		}
				
		if (rank <= 10)
			mNumTopTen++;
				
		mSum += rank;
		mNumID++;
	}

	float t = mResults->getTimeTaken();

	if (t == -1.0)
		mNumInvalidTimes++;
	else {
		mNumValidTimes++;
		mTotalTime += t;
	}

	if (NULL != mUnknownFin)
	{
		delete mUnknownFin;
		mUnknownFin = NULL;
	}

	if (NULL != mMatcher)
	{
		delete mMatcher;
		mMatcher = NULL;
	}
}

void MatchingQueue::summarizeMatching(ostream& out)
{
	out << endl << "Matching completed." << endl
		 << "\tAverage time per match: " << mTotalTime / mNumValidTimes << " over ";
	       
	if (mNumValidTimes == 0)
		out << "no valid times.";
	else if (mNumValidTimes == 1)
		out << "1 valid time.";
	else
		out << mNumValidTimes << " valid times.";

	out << endl;

	if (mNumInvalidTimes > 0) {
		out << "Warning: ";

		if (mNumInvalidTimes == 1)
			out << "1 set of results didn't have a valid time entered.";
		else
			out << mNumInvalidTimes << " sets of results didn't have valid times entered.";
		out << endl;
	}
		
	out << endl;

	//***1.2 - list files and theie individual rankings
	for (int idx=0; idx < mNumID; idx++)
	{
		char numStr[16];
		ifstream inFile;

		//string path = getenv("DARWINHOME");
		//***1.85 - everything is now relative to the current survey area
		string path = gOptions->mCurrentSurveyArea;
		path += PATH_SLASH;
		path += "matchQResults";
		path += PATH_SLASH;
		string resultFilename = "results-unknown-";
		sprintf(numStr,"%d",idx);
		resultFilename += numStr;

		inFile.open((path+resultFilename).c_str());
		if (! inFile.fail())
		{
			out << resultFilename << ":";
			string line;
			getline(inFile,line); // fin ID
			getline(inFile,line); // fin File
			out << line.substr(line.find_last_of(PATH_SLASH)+1) << ":";
			getline(inFile,line); // database File
			getline(inFile,line); // ranking
			out << line << endl;
			inFile.close();
		}
		inFile.clear();
	}

	if (mNumID > 0) {
		out << endl;
			
		if (mNumID == 1)
			out << "Out of 1 fin with an ID" << endl;
		else
			out << "Out of " << mNumID << " fins with IDs" << endl;
				
		cout << "\tAverage rank: " << (float)mSum / mNumID << endl;

		if (mNumTopTen == 0)
			out << "\tNo fins ranked in the top ten.";

		else if (mNumTopTen == 1)
			out << "\t1 fin (" << (float) 1 / mNumID * 100.0
			     << "%) ranked in the top ten.";

		else
			out << "\t" << mNumTopTen << " fins ("
			     << (float) mNumTopTen / mNumID * 100.0
			     << "%) ranked in the top ten.";
					
			out << endl << endl;
			
			if (!mFirstRun) {
				out << "\tBest rank: " << mBestRank << endl
				     << "\tWorst rank: " << mWorstRank << endl
				     << endl;
			}
	}

	if (mNumNoID == 0)
		out << "All fins had an ID provided." << endl;
	else if (mNumNoID == 1)
		out << "1 fin with no ID provided." << endl;
	else	
		out << mNumNoID << " fins with no ID provided." << endl;
}

list<queueItem_t> MatchingQueue::getQueue()
{
	list<queueItem_t> qList;

	list<string>::iterator it = mFileNames.begin();

	while (it != mFileNames.end()) {
		queueItem_t qItem;

                string::size_type idx;
                
                idx = it->rfind("/");

                if (idx == string::npos) {
                        idx = it->rfind("\\");
                        
                         if (idx == string::npos)
                                 qItem.fileName = *it;
                         else
                                 qItem.fileName = it->substr(idx + 1);
                } else
                        qItem.fileName = it->substr(idx + 1);
        
		qList.push_back(qItem);

		++it;
	}
	
	return qList;
}


bool MatchingQueue::queueIsEmpty() const
{
	return (mFileNames.size() == 0);
}


void MatchingQueue::remove(int itemNum)
{
	if (itemNum < 0 || itemNum > (int)mFileNames.size() - 1)
		throw Error("Bounds error in MatchingQueue::remove()");

	list<string>::iterator it = mFileNames.begin();

	for (int i = 0; i < itemNum; i++)
		++it;

	mFileNames.erase(it);
}

void MatchingQueue::save(string fileName)
{
	ofstream outFile(fileName.c_str());

	if (!outFile)
		throw Error("Problem writing to file: " + fileName);

	list<string>::iterator it = mFileNames.begin();

	while (it != mFileNames.end()) {

		//***1.1 - we assume ALL traced fin files are in the DARWINHOME/tracedFins
		// folder, so we strip the DARWINHOME part of the path out here.  It will
		// be added back in the load function.  This makes sure that movement of
		// the DARWIN folder does not break the queues.

		//string home = getenv("DARWINHOME");
		//***1.85 - everything is now relative to the current survey area

		//***2.0 - it is possible that fins may be in another survey area of the
		// same darwin installation.  So we now save the NAME of the survey area
		// down to the fin filename (ex: "Tampa Bay\tracedFins\aloa.fin") whereas
		// we used to save only this (ex: "tracedFins\aloa.fin")
		//
		// So, the strategy is to strip less here, and then inside the load()
		// function we will assume that the Current Survey Area is the prefix
		// if none is specified. Otherwise, we will look in the specified
		// Survey Area for the fin.  If neither is the correct path then
		// the fin has been saved where we cannot recover it from the loaded
		// match queue. -- JHS

		string area = gOptions->mCurrentSurveyArea; // current survey area
		string filename = (*it).c_str();            // entire path\filename
		string shortFilename;

		// are we in the CURRENT survey area?

		int pos = filename.find(area);
		if (string::npos != pos)
		{
			// then the fin has been added from inside the current survey area
			filename = filename.substr(area.length()); // strip preamble
			pos = area.rfind(PATH_SLASH);    // just want the areaName
			area = area.substr(pos+1);       // got it
			shortFilename = area + filename; // keep areaName\...
			outFile << "<area> " << shortFilename;
		}
		else
		{
			string darwinHome = gOptions->mDarwinHome;
			pos = filename.find(darwinHome);
			if (string::npos != pos)
			{
				// the fin is in this DARWIN folder hierarchy
				filename = filename.substr(darwinHome.length()); // strip preamble
				pos = darwinHome.rfind(PATH_SLASH);    // just want DARWIN folder name
				darwinHome = darwinHome.substr(pos+1); // got it  
				shortFilename = darwinHome + filename; // keep darwinHome\ ...
				outFile << "<home> " << shortFilename;
			}
			else
			{
				// fin is somewhere other than this DARWIN installation
				outFile << "<full> " << filename; // keep full path
			}
		}

		++it;

		if (it != mFileNames.end())
			outFile << endl;
	}
	outFile.close();
}

void MatchingQueue::load(string fileName)
{
	ifstream inFile(fileName.c_str());

	if (!inFile)
		throw Error("File: " + fileName + " does not exist.");

	mFileNames.clear();

	const int BUFFERSIZE = 1024;

	char c[BUFFERSIZE];

	string entry;

	while (!inFile.eof()) {
		inFile.getline(c, BUFFERSIZE);
		entry = c;

		//***1.1 - we assume ALL traced fin files are in the DARWINHOME/tracedFins
		// folder, so we prepend the DARWINHOME part of the path here. The entries in
		// the queue file contain the path and fin filename relative to DARWINHOME.
		// This makes sure that movement of the DARWIN folder does not break the queues.

		//string filename = getenv("DARWINHOME");
		//***1.85 - everything is now relative to the current survey area
		
		//***2.0 - we now have four possible scenarios with the filenames
		// The first TOKEN on each line is either ...
		// MISSING, so we have an OLD match queue file and must assume
		//    that each line is relative to the CURRENT survey area
		// "<full>", so the entire path was saved, in which case we use it as is
		// "<area>", a partial path beginning within the CURRENT survey area
		//    is assumed
		// "<home>" a partial path beginning somewhere within the current DARWIN
		//    installation is assumed

		string thisHome = gOptions->mDarwinHome;
		thisHome = thisHome.substr(0,thisHome.rfind(PATH_SLASH));

		string thisArea = gOptions->mCurrentSurveyArea;
		thisArea = thisArea.substr(0,thisArea.rfind(PATH_SLASH));

		string filename;

		string token = entry.substr(0,6);
		if (token == "<area>")
		{
			// path relative to SOME survey area ASSUMED inside this DARWIN install

			filename = gOptions->mCurrentSurveyArea;
			filename = filename.substr(0,filename.rfind(PATH_SLASH)+1); // strip area name
			entry = entry.substr(7); // strip token
			filename += entry; // append saved area name + rest
		}
		else if (token == "<home>")
		{
			// path relative to a DARWIN installation, but ASSUME it is is same
			// relative location (ex: "Program Files/darwin" & "Program Files/darwin2.0")
			filename = gOptions->mDarwinHome;
			filename = filename.substr(0,filename.rfind(PATH_SLASH)+1); // strip home name
			entry = entry.substr(7); // strip token
			filename += entry; // append saved DARWIN home name + rest
		}
		else if (token == "<full>")
		{
			// complete path, use as is
			filename = entry.substr(7); // strip token first
		}
		else
		{
			// MISSING - an old path relative to the CURRENT survey area
			filename = gOptions->mCurrentSurveyArea + PATH_SLASH;
			filename += entry; // append rest
		}

		mFileNames.push_back(filename); //***1.1
		entry = "";
	}

	inFile.close(); // just to be nice
}

/*
DatabaseFin<ColorImage>* MatchingQueue::getItemNum(int itemNum)
{
#ifdef DEBUG
	cout << "Getting queue item num: " << itemNum << endl;
#endif
	if (itemNum > (int)mFileNames.size() - 1)
		throw Error("bounds problem in MatchingQueue::getItemNum");

	try {
		list<string>::iterator it = mFileNames.begin();

		for (int i = 0; i < itemNum; i++)
			++it;

		DatabaseFin<ColorImage> *newFin = new DatabaseFin<ColorImage>(*it);
	
		return newFin;
	
	} catch (...) {
		throw;
	}
}
*/

std::string MatchingQueue::getItemNum(int itemNum)
{
#ifdef DEBUG
	cout << "Getting queue item num: " << itemNum << endl;
#endif
	if (itemNum > (int)mFileNames.size() - 1)
		throw Error("bounds problem in MatchingQueue::getItemNum");

	try {
		list<string>::iterator it = mFileNames.begin();

		for (int i = 0; i < itemNum; i++)
			++it;

		return *it;
	
	} catch (...) {
		throw;
	}
}
