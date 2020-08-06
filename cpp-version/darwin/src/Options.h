//*******************************************************************
//   file: Options.h
//
// author: Adam Russell
//
//   mods: J H Stewman (9/29/2005)
//         -- add support for catalog categories
//         J H Stewman (4/30/2007)
//         -- add support for multiple surveyAreas and catalogs (databases)
//
//*******************************************************************

#ifndef OPTIONS_H
#define OPTIONS_H

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include <vector>

typedef enum {TEXT, PICTURES, BOTH} toolbarDisplayType;

class Options
{
	public:
		Options()
			:
			mToolbarDisplay(BOTH),
			mMaxIterations(50),
			mContinuityWeight(9.0f),
			mLinearityWeight(3.0f),
			mEdgeWeight(2.0f),
			mHighThreshold(0.85f),
			mLowThreshold(0.15f),
			mGaussianStdDev(1.5f),
			mNumberOfExistingDataPaths(0),   //***2.22
			mNumberOfExistingSurveyAreas(0), //***2.22
			mNumberOfExistingDatabases(0),   //***2.22
			//mDatabaseFileName("darwin.db"),
			mDatabaseFileName(""), //***2.22 - default is now NO default database
			mTempDirectory(""),
#ifdef WIN32
			//mCurrentSurveyArea("surveyAreas\\default"),
#else
			//mCurrentSurveyArea("surveyAreas/default"),
#endif
			mCurrentSurveyArea(""), //***2.22 - default is now NO default survey area
			mCurrentDataPath(""), //***2.22 - NO default data path - figure out from $HOME or $HOMEPATH
			mNumberOfDefinedCatalogSchemes(0), //***1.4 - none is default
			mHideIDs(true) //***1.65
		{
			mCurrentColor[0] = 0.0;
			mCurrentColor[1] = 1.0;
			mCurrentColor[2] = 0.0;
			mCurrentColor[3] = 0.0;

			//***1.9 - default data field names

			mDataFieldSchemeName = "Eckerd Data Fields";
			mDataFieldName.push_back("ID Code");
			mDataFieldName.push_back("Name");
			mDataFieldName.push_back("Date of Sighting");
			mDataFieldName.push_back("Roll/Frame or Lat/Long"); //***2.22 - added Lat/Long
			mDataFieldName.push_back("Location Code");
			mDataFieldName.push_back("Damage Category");
			mDataFieldName.push_back("Description");
			mDataFieldNamesMax = mDataFieldName.size();
		}


		~Options()
		{
			mDefinedCatalogSchemeName.clear();
			for (int i = 0; i <mNumberOfDefinedCatalogSchemes; i++)
				mDefinedCatalogCategoryName[i].clear();
			mDefinedCatalogCategoryName.clear();
		}

		double mCurrentColor[4];
		
		toolbarDisplayType mToolbarDisplay;

		// snake stuff

		int mMaxIterations;

		float
			mContinuityWeight,
			mLinearityWeight,
			mEdgeWeight,

		// canny edge detector

			mHighThreshold,
			mLowThreshold,
			mGaussianStdDev;

		std::string mDatabaseFileName; // the CURRENT database name

		//***054 NEW catalog category support

		//***1.9 - new members for saving data field names, eventually we will support
		//         user specification of field names

		int mDataFieldNamesMax; //***1.9

		std::vector<std::string> mDataFieldName; //***1.9 - names of current data fileds

		std::string mDataFieldSchemeName; //***1.9

		//***1.1 - support for multiple databases, fixed home folder, multiple queues, etc

		// Beginning with version1.1 ...
		// ALL database, match queue, and traced fin files are specified RELATIVE to mDarwinHome
		// and all paths that are part of filenames inside the database, queues, etc are 
		// RELATIVE to mDarwinHome.  This allows movement of a database folder to a new location
		// without loss of database integrity.  Match queues, and traced fin files can be
		// moved in a similar fashion.

		std::string mDarwinHome;                        // value of %DARWINHOME% set by runDarwin.bat
		std::string mTempDirectory; //Temporary directory

		int mNumberOfExistingDataPaths;             //***2.22
		std::string mCurrentDataPath;         //***2.22 - current home for data OUTSIDE DARWINHOME
		std::vector<std::string> mExistingDataPaths; //***2.22 - new homes for data OUTSIDE DARWINHOME

		// For all versions 1.85 and later ...
		// the following is now the root path for all relative filenames in the catalog
		// tracedFins, matchQueues and matchQResults folders .. these folders now exist
		// in EACH surveyArea folder created within the DARWINHOME folder
		std::string mCurrentSurveyArea;     //***1.85 - root for all catalog, tracedFins, etc

		int mNumberOfExistingSurveyAreas;                 //***1.85 number of existing survey areas
		std::vector<std::string> mExistingSurveyAreaName; //***1.85 vector of existing survey area names
		
		int mNumberOfExistingDatabases;                 // number of existing databases
		std::vector<std::string> mExistingDatabaseName; // vector of existing database names

		int mNumberOfDefinedCatalogSchemes;                   // default is 1 (Eckerd Scheme)
		std::vector<std::string> mDefinedCatalogSchemeName;   // default is "Eckerd College Scheme"
		std::vector<int> mDefinedCatalogCategoryNamesMax;             
		std::vector<std::vector <std::string> > 
			mDefinedCatalogCategoryName;                      // Eckerd Scheme has 14 category names

		int mCurrentDefaultCatalogScheme;   // must be in range [0,mNumberOfExistingDatabases)

		bool mHideIDs; //***1.65 - hide or show dolphin IDs throughout software
		               // used for blind testing at EC

		std::string
			mCurrentFontName; //***1.85 - font for all lists and txt fields

		std::string
			SevenZ; //***2.0 - for path update and call to 7z.exe (for stand alone viewer)
};

#endif
