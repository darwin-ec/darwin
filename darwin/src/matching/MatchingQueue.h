//*******************************************************************
//   file: MathcingQueue.h
//
// author: Adam Russell
//
//   mods: J H Stewman (9/29/2005)
//         -- code to allow use of Options to specify catalog category
//            for matching
//
//*******************************************************************

#ifndef MATCHINGQUEUE_H
#define MATCHINGQUEUE_H

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include <list>
#include "Match.h"
#include "../CatalogSupport.h"
#include "../DatabaseFin.h"
#include "../Database.h"


typedef struct {
	std::string
		fileName,
		date,
		location;
} queueItem_t;

class MatchingQueue
{
	public:
		MatchingQueue(Database *d, Options *o);
		~MatchingQueue();

		void add(std::string fileName);
		void remove(int itemNum);

		// Matches stuff
		float match();

		std::list<queueItem_t> getQueue();

		bool queueIsEmpty() const;

		void save(std::string fileName);
		void load(std::string fileName);

		//DatabaseFin<ColorImage>* getItemNum(int itemNum);
		std::string getItemNum(int itemNum);

		void setupMatching(); //***1.1

		void finalizeMatch(); //***1.1

		void summarizeMatching(std::ostream& out = std::cout); //***1.1

		Match *getNextUnknownToMatch(); //***1.1
		Match *getCurrentUnknownToMatch(); //***1.1

		MatchResults *getMatchResults(); //***1.1

		float matchProgress(); //***1.1

	private:
		std::list<std::string> mFileNames;
		Database *mFinDatabase;

		Options *mOptions; //***054

		int mCurrentFinID; //***1.1 - current fin file (unknown fin) being matched

		//***1.1 - variables to compute matching stats

		int
			mNumNoID,        // Number of fins with no ID
			mSum,            // Total of all ranks, used to compute average rank at the end
			mNumTopTen,      // Number of fins with rank <= 10
			mNumID,          // Number of fins with an ID, used to determine quality of the match
			mNumValidTimes,  // Number of valid time taken entries
			mNumInvalidTimes,
			mWorstRank, 
			mBestRank;

		float mTotalTime;
		
		bool mFirstRun;

		//***1.1 - members to replace local vars in old match() function

		DatabaseFin<ColorImage> *mUnknownFin;

		Match *mMatcher;

		MatchResults *mResults;

};

#endif
