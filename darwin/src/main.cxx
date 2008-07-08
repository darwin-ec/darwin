//*******************************************************************
//   file: IntensityContour.cxx
//
//*******************************************************************

///////////////////////////////////////////////////////////////////////////
//                                                                       //
//	DARWIN Software                                                      //
//                                                                       //
// Digital Analysis to Recognize Whale Images on a Network               //
// Digital Archive of Recognized Whale Images on a Network               //
//                                                                       //
// Software designed and coded as a collaborative effort ...             //
//                                                                       //
// ... by  The DARWIN Research Group                                     //
//	Kelly R. Debure, PhD (GTK+, X Window System version)                 //
//	Kristen McCoy (GTK+ Version)                                         //
//	Henry Burroughs (GTK+ Version)                                       //
//	Zach Roberts (X Window System version)                               //
//	Daniel J. Wilkin (X Window System version)                           //
//	John H. Stewman, PhD (all versions)                                  //
//	Mark C. Allen (Microsoft Win 3.1 version, Expert System version)     //
//	Rachel Stanley (Expert System version)                               //
//	Andrew Cameron (protype database)                                    //
//	Adam Russell (stuff)                                                 //
//  Scott Hale (autotrace)
//                                                                       //
// ... at  Eckerd College                                                //
//	St.Petersburg, FL                                                    //
//                                                                       //
// xdarwin (DARWIN for the X Window System)                              //
//                                                                       //
// ... begun on July 27, 1994 (JHS)                                      //
// ... and updated on July 28, 1994 (JHS)                                //
// ... and updated starting June 12, 1995 (DW)                           //
// ... then maintained by Daniel J. Wilkin and Zach Roberts              //
// ... major rewrite using GTK+ 2000-2002 (Adam Russell)                 //
// ... currently maintained by JHS & KWD                                 //
//                                                                       //
// Supported by grants from the National Science Foundation              //
//                                                                       //
// Species Domain - Common Bottlenose Dolphin - Tursiops truncatus       //
//                                                                       //
// Copyright (C) 2001                                                     //
///////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

#include "ConfigFile.h"
#include "support.h"
#include "interface/MainWindow.h"
#include "DatabaseSupport.h"
#include "Options.h"
#include "interface/SplashWindow.h"
#include "waveletUtil.h"

// trying to find memory leaks - next 3 lines
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#ifdef WIN32
#include <crtdbg.h>
#endif
#include <iostream>

using namespace std;

static ConfigFile *gCfg = NULL;
Options *gOptions = NULL; //***1.85 - this is no longer static and is referenced GLOBALLY

//#define TIMING_ENABLED -- set this in the MSVC:Project:Preprocessor:Defines
#ifdef TIMING_ENABLED
ofstream timingOutFile; //***1.95
#endif

void readConfig();
void saveConfig();

//*******************************************************************
//
void readConfig()
{

#ifdef WIN32

	//***054 - program is now started on Windows Platform by darwin54.bat, 
	//         which sets environment variable DARWINHOME prior to call

	string fileName("");

	char *homedir = getenv("DARWINHOME");

	gOptions->mDarwinHome = homedir; // ***1.1
	
	fileName += homedir;
	fileName += "\\system\\darwin.cfg";

	gCfg = new ConfigFile(fileName.c_str());

	//***1.85 - if no config found,  attempt to load backup copy,
	// else immediately save a backup copy just in case problems arise later 

	if (gCfg->size() > 0)
		gCfg->save((fileName+".bak").c_str());
	else
		gCfg->open((fileName+".bak").c_str());
	
	if ((!gCfg->getItem("DatabaseFileName", gOptions->mDatabaseFileName)) ||
		(gOptions->mDatabaseFileName == "NONE"))
	{
		fileName = "";
		fileName += homedir;
		fileName += "\\surveyAreas\\default"; //***1.85

		gOptions->mCurrentSurveyArea = fileName; //***1.85

		//fileName += "\\catalog\\darwin.db"; //***1.85

		//gOptions->mDatabaseFileName = fileName.c_str();

		//***1.85 - default is now NO DATABASE
		gOptions->mDatabaseFileName = "NONE";
	}
	else
	{
		//***1.85 - break out the currentSurveyArea root name
		int pos = gOptions->mDatabaseFileName.rfind("\\catalog");
		gOptions->mCurrentSurveyArea = gOptions->mDatabaseFileName.substr(0,pos);
	}

#else

	// on UNIX/Linux platform the same DARWINHOME environment variable is used
	// its value must be set in .bashrc, .cshrc, or in a runtime shell script
	
	char 
		*fileName, 
		*homedir;

	homedir = getenv("DARWINHOME");
	//***1.3 - in case Linux can't find DARWINHOME
	if (NULL == homedir)
	{
		cout << "$DARWINHOME is not defined in runtime environment!\n";
		exit(1);
	}

	gOptions->mDarwinHome = homedir; // ***1.1

	fileName = new char[strlen(homedir) + strlen("/.darwin") + 1];
	sprintf(fileName, "%s/.darwin", homedir);

	gCfg = new ConfigFile(fileName);

	delete[] fileName;

	if (!gCfg->getItem("DatabaseFileName", gOptions->mDatabaseFileName)) 
	{
		//***1.85 - build and save current survey area root path
		fileName = new char[strlen(homedir) + strlen("/surveyAreas/default") + 1];
		sprintf(fileName, "%s/surveyAreas/default", homedir);

		gOptions->mCurrentSurveyArea = fileName; //***1.85

		fileName = new char[strlen(homedir) + strlen("/catalog/darwin.db") + 1];
		sprintf(fileName, "%s/catalog/darwin.db", homedir);

		gOptions->mDatabaseFileName = fileName;
		delete[] fileName;
	}
	else
	{
		//***1.85 - break out the currentSurveyArea root path
		int pos = gOptions->mDatabaseFileName.rfind("/catalog");
		gOptions->mCurrentSurveyArea = gOptions->mDatabaseFileName.substr(0,pos);
	}


#endif

	if (!gCfg->getItem("CurrentColor[0]", gOptions->mCurrentColor[0]) ||
	    !gCfg->getItem("CurrentColor[1]", gOptions->mCurrentColor[1]) ||
	    !gCfg->getItem("CurrentColor[2]", gOptions->mCurrentColor[2]) ||
	    !gCfg->getItem("CurrentColor[3]", gOptions->mCurrentColor[3])) 
	{

		for (int i = 0; i < 4; i++)
			gOptions->mCurrentColor[i] = 0.0;

		gOptions->mCurrentColor[1] = 1.0;
	}

	string toolbarType;
	gCfg->getItem("ToolbarDisplayType", toolbarType);

	if (toolbarType.empty())
		gOptions->mToolbarDisplay = BOTH;
	else {
		if (toolbarType == "BOTH")
			gOptions->mToolbarDisplay = BOTH;
		else if (toolbarType == "PICTURES")
			gOptions->mToolbarDisplay = PICTURES;
		else if (toolbarType == "TEXT")
			gOptions->mToolbarDisplay = TEXT;

		else gOptions->mToolbarDisplay = BOTH;
	}

	if (!gCfg->getItem("GaussianStandardDeviation", gOptions->mGaussianStdDev))
		gOptions->mGaussianStdDev = 1.5;

	if (!gCfg->getItem("CannyLowThreshold", gOptions->mLowThreshold))
		gOptions->mLowThreshold = 0.15f;

	if (!gCfg->getItem("CannyHighThreshold", gOptions->mHighThreshold))
		gOptions->mHighThreshold = 0.85f;

	if (!gCfg->getItem("SnakeMaximumIterations", gOptions->mMaxIterations))
		gOptions->mMaxIterations = 50;

	if (!gCfg->getItem("SnakeEnergyContinuity", gOptions->mContinuityWeight))
		gOptions->mContinuityWeight = 9.0;

	if (!gCfg->getItem("SnakeEnergyLinearity", gOptions->mLinearityWeight))
		gOptions->mLinearityWeight = 3.0;

	if (!gCfg->getItem("SnakeEnergyEdge", gOptions->mEdgeWeight))
		gOptions->mEdgeWeight = 3.0;

	//***054 - catalog category support added 
	//         (default: Eckerd College categories always loaded at start)
	//***1.4 - support for multiple catalog schemes added

	if (!gCfg->getItem("NumberOfDefinedCatalogSchemes", gOptions->mNumberOfDefinedCatalogSchemes))
	{
		// no catalog schemes defined in config file so use Eckerd College catalog as default

		gOptions->mNumberOfDefinedCatalogSchemes = 1;
		gOptions->mDefinedCatalogSchemeName.resize(1);
		gOptions->mDefinedCatalogSchemeName[0] = "Eckerd College";
		gOptions->mDefinedCatalogCategoryNamesMax.resize(1);
		gOptions->mDefinedCatalogCategoryNamesMax[0] = 14;
		gOptions->mDefinedCatalogCategoryName.resize(1);
		gOptions->mDefinedCatalogCategoryName[0].resize(14);
		gOptions->mDefinedCatalogCategoryName[0][0] = "NONE";  // shown as "Unspecified" in database and pull-down lists
		gOptions->mDefinedCatalogCategoryName[0][1] = "Upper";
		gOptions->mDefinedCatalogCategoryName[0][2] = "Middle";
		gOptions->mDefinedCatalogCategoryName[0][3] = "Lower";
		gOptions->mDefinedCatalogCategoryName[0][4] = "Upper-Middle";
		gOptions->mDefinedCatalogCategoryName[0][5] = "Upper-Lower";
		gOptions->mDefinedCatalogCategoryName[0][6] = "Middle-Lower";
		gOptions->mDefinedCatalogCategoryName[0][7] = "Leading Edge";
		gOptions->mDefinedCatalogCategoryName[0][8] = "Entire";
		gOptions->mDefinedCatalogCategoryName[0][9] = "Tip-Nick";
		gOptions->mDefinedCatalogCategoryName[0][10] = "Missing Tip";
		gOptions->mDefinedCatalogCategoryName[0][11] = "Extended Tip";
		gOptions->mDefinedCatalogCategoryName[0][12] = "Peduncle";
		gOptions->mDefinedCatalogCategoryName[0][13] = "Pergatory";
	}
	else
	{
		gOptions->mDefinedCatalogSchemeName.resize(gOptions->mNumberOfDefinedCatalogSchemes);
		gOptions->mDefinedCatalogCategoryNamesMax.resize(gOptions->mNumberOfDefinedCatalogSchemes);
		gOptions->mDefinedCatalogCategoryName.resize(gOptions->mNumberOfDefinedCatalogSchemes);

		for (int schemeNum = 0; schemeNum < gOptions->mNumberOfDefinedCatalogSchemes; schemeNum++)
		{
			char temp[64];

			// read catalog scheme name

			sprintf(temp,"DefinedCatalogSchemeName[%d]",schemeNum);
			string SchemeName = temp;
			if (!gCfg->getItem(SchemeName, gOptions->mDefinedCatalogSchemeName[schemeNum]))
				gOptions->mDefinedCatalogSchemeName[schemeNum] = "error";

			// read numer of catalog category names in scheme

			sprintf(temp,"DefinedCatalogCategoryNamesMax[%d]",schemeNum);
			string CatNamesMax = temp;
			if (!gCfg->getItem(CatNamesMax, gOptions->mDefinedCatalogCategoryNamesMax[schemeNum]))
				gOptions->mDefinedCatalogCategoryNamesMax[schemeNum] = 0;

			// read all category names for scheme

			gOptions->mDefinedCatalogCategoryName[schemeNum].resize(
				           gOptions->mDefinedCatalogCategoryNamesMax[schemeNum]);

			for (int catNum = 0; catNum < gOptions->mDefinedCatalogCategoryNamesMax[schemeNum]; catNum++)
			{
				sprintf(temp,"DefinedCatalogCategoryName[%d][%d]",schemeNum,catNum);
				string DefCatName = temp;
				if (!gCfg->getItem(DefCatName, 
					               gOptions->mDefinedCatalogCategoryName[schemeNum][catNum]))
					gOptions->mDefinedCatalogCategoryName[schemeNum][catNum] = "error";
			}
		}
	}

	if (!gCfg->getItem("CurrentDefaultCatalogScheme", gOptions->mCurrentDefaultCatalogScheme))
	{
		gOptions->mCurrentDefaultCatalogScheme = 0;

		// no catalog defined in config file so use Eckerd College catalog

		gOptions->mCatCategoryNamesMax = 14;
		gOptions->mCatCategoryName.resize(14); //***1.85 - container is now a vector not an array
		gOptions->mCatCategoryName[0] = "NONE";  // shown as "Unspecified" in database and pull-down lists
		gOptions->mCatCategoryName[1] = "Upper";
		gOptions->mCatCategoryName[2] = "Middle";
		gOptions->mCatCategoryName[3] = "Lower";
		gOptions->mCatCategoryName[4] = "Upper-Middle";
		gOptions->mCatCategoryName[5] = "Upper-Lower";
		gOptions->mCatCategoryName[6] = "Middle-Lower";
		gOptions->mCatCategoryName[7] = "Leading Edge";
		gOptions->mCatCategoryName[8] = "Entire";
		gOptions->mCatCategoryName[9] = "Tip-Nick";
		gOptions->mCatCategoryName[10] = "Missing Tip";
		gOptions->mCatCategoryName[11] = "Extended Tip";
		gOptions->mCatCategoryName[12] = "Peduncle";
		gOptions->mCatCategoryName[13] = "Pergatory";
	}
	else
	{
		// read category names from config file

		int cur = gOptions->mCurrentDefaultCatalogScheme;

		gOptions->mCatSchemeName = gOptions->mDefinedCatalogSchemeName[cur];

		gOptions->mCatCategoryNamesMax = gOptions->mDefinedCatalogCategoryNamesMax[cur]; //***1.6DB

		//***1.85 - make room in the categor name vector
		int n = gOptions->mCatCategoryNamesMax;
		gOptions->mCatCategoryName.resize(n);

		for (int i = 0; i < n; i++) //***1.85
			gOptions->mCatCategoryName[i] = gOptions->mDefinedCatalogCategoryName[cur][i];
	}

	//***1.65 - load global setting for Show/Hide/Alt IDs in all windows
	// done to support blind testing of software by EC Dolphin Research Group

	if (!gCfg->getItem("HideFinIDsinAllWindows",gOptions->mHideIDs)) //***1.65
		gOptions->mHideIDs = false; // ShowIDs by default, this is the normal use setting

	//***1.85 - add support for multiple survey areas and databases
	if (!gCfg->getItem("NumberOfExistingSurveyAreas",gOptions->mNumberOfExistingSurveyAreas))
	{
		// there is no survey area defined, so assume the "default" survey area and database
		// NOTE: survey areas and databases are all saved in conguration "relative to"
		// the DARWINHOME/surveyAreas folder

		gOptions->mNumberOfExistingSurveyAreas = 1;
		gOptions->mExistingSurveyAreaName.push_back("default");
	}
	else 
	{
		// survey areas are defined, so extract them
		char temp[64]; // buffer for building querry string
		int n = gOptions->mNumberOfExistingSurveyAreas;
		gOptions->mExistingSurveyAreaName.resize(n);
		for (int i = 0; i < n; i++)
		{
			sprintf(temp,"ExistingSurveyAreaName[%d]",i);
			string ExistAreaName = temp;
			if (!gCfg->getItem(ExistAreaName, 
					           gOptions->mExistingSurveyAreaName[i]))
				gOptions->mExistingSurveyAreaName[i] = "error";
		}
	}

	if (!gCfg->getItem("NumberOfExistingDatabases",gOptions->mNumberOfExistingDatabases))
	{
		// but no database is ... this is probably a MORE serious error than handling 
		// like this would reflect

		//****1.85 - default is now NO DATABASE

		//gOptions->mNumberOfExistingDatabases = 1;
		gOptions->mNumberOfExistingDatabases = 0;
#ifdef WIN32
		//gOptions->mExistingDatabaseName.push_back("default\\darwin.db");
		//gOptions->mExistingDatabaseName.push_back("");
#else
		//gOptions->mExistingDatabaseName.push_back("default/darwin.db");
		//gOptions->mExistingDatabaseName.push_back("");
#endif
	}
	else
	{
		// databases are defined, so extract them
		char temp[64]; // buffer for building querry string
		int n = gOptions->mNumberOfExistingDatabases;
		gOptions->mExistingDatabaseName.resize(n);
		for (int i = 0; i < n; i++)
		{
			sprintf(temp,"ExistingDatabaseName[%d]",i);
			string ExistDbName = temp;
			if (!gCfg->getItem(ExistDbName, 
				               gOptions->mExistingDatabaseName[i]))
			gOptions->mExistingDatabaseName[i] = "error";
		}
	}

	//***1.85 - we support selection of font now
	if (!gCfg->getItem("SelectedFontForLists",gOptions->mCurrentFontName))
	{
		gOptions->mCurrentFontName = "Sans 10";
	}
}

//*******************************************************************
//
void saveConfig()
{
	if (NULL == gCfg)
		return;

	 // empty list so we can rebuild older style config file in newer format

	gCfg->ClearList();

	gCfg->addItem("DARWINHOME", gOptions->mDarwinHome); //***1.85
	gCfg->addItem("CurrentSurveyArea", gOptions->mCurrentSurveyArea); //***1.85
	gCfg->addItem("DatabaseFileName", gOptions->mDatabaseFileName);

	gCfg->addItem("CurrentColor[0]", gOptions->mCurrentColor[0]);
	gCfg->addItem("CurrentColor[1]", gOptions->mCurrentColor[1]);
	gCfg->addItem("CurrentColor[2]", gOptions->mCurrentColor[2]);
	gCfg->addItem("CurrentColor[3]", gOptions->mCurrentColor[3]);

	switch (gOptions->mToolbarDisplay) {
		case BOTH:
			gCfg->addItem("ToolbarDisplayType", "BOTH");
			break;
		case TEXT:
			gCfg->addItem("ToolbarDisplayType", "TEXT");
			break;
		case PICTURES:
			gCfg->addItem("ToolbarDisplayType", "PICTURES");
			break;
		default:
			gCfg->addItem("ToolbarDisplayType", "BOTH");
			break;
	}
	gCfg->addItem("GaussianStandardDeviation", gOptions->mGaussianStdDev);
	gCfg->addItem("CannyLowThreshold", gOptions->mLowThreshold);
	gCfg->addItem("CannyHighThreshold", gOptions->mHighThreshold);

	gCfg->addItem("SnakeMaximumIterations", gOptions->mMaxIterations);

	gCfg->addItem("SnakeEnergyContinuity", gOptions->mContinuityWeight);
	gCfg->addItem("SnakeEnergyLinearity", gOptions->mLinearityWeight);
	gCfg->addItem("SnakeEnergyEdge", gOptions->mEdgeWeight);

	// number of defined catalog schemes

	gCfg->addItem("NumberOfDefinedCatalogSchemes", gOptions->mNumberOfDefinedCatalogSchemes);

	// for each scheme

	int s; // hack for MSVC "local" counters
	for (s = 0; s < gOptions->mNumberOfDefinedCatalogSchemes; s++)
	{
		char temp[64];

		// name of scheme

		sprintf(temp,"DefinedCatalogSchemeName[%d]",s);
		string SchemeName = temp;
		gCfg->addItem(SchemeName, gOptions->mDefinedCatalogSchemeName[s]);

		// number of category names

		sprintf(temp,"DefinedCatalogCategoryNamesMax[%d]",s);
		string CatNamesMax = temp;
		gCfg->addItem(CatNamesMax, gOptions->mDefinedCatalogCategoryNamesMax[s]);

		// list of defined category names

		for (int i = 0; i < gOptions->mDefinedCatalogCategoryNamesMax[s]; i++)
		{
			sprintf(temp,"DefinedCatalogCategoryName[%d][%d]", s, i);
			string DefCatName = temp;
			gCfg->addItem(DefCatName, gOptions->mDefinedCatalogCategoryName[s][i]);
		}
	}

	// id of current default scheme

	gCfg->addItem("CurrentDefaultCatalogScheme", gOptions->mCurrentDefaultCatalogScheme);

	//***1.85 - add the survey area names
		
	// number of Survey Areas
	gCfg->addItem("NumberOfExistingSurveyAreas", gOptions->mNumberOfExistingSurveyAreas);

	// for each the survey area name
	for (s = 0; s < gOptions->mNumberOfExistingSurveyAreas; s++)
	{
		char temp[64];

		// name of scheme

		sprintf(temp,"ExistingSurveyAreaName[%d]",s);
		string AreaName = temp;
		gCfg->addItem(AreaName, gOptions->mExistingSurveyAreaName[s]);
	}

	//***1.85 - add the existing database names
		
	// number of existing databases
	gCfg->addItem("NumberOfExistingDatabases", gOptions->mNumberOfExistingDatabases);
	
	// for each database name
	for (s = 0; s < gOptions->mNumberOfExistingDatabases; s++)
	{
		char temp[64];

		// name of database

		sprintf(temp,"ExistingDatabaseName[%d]",s);
		string DbName = temp;
		gCfg->addItem(DbName, gOptions->mExistingDatabaseName[s]);
	}

	//***1.65 - add value of global Show/Hide Fin ID indicator

	gCfg->addItem("HideFinIDsinAllWindows",gOptions->mHideIDs); //***1.65

	//***1.85 - save selected FONT used in various lists

	gCfg->addItem("SelectedFontForLists", gOptions->mCurrentFontName); //***1.85

	gCfg->save();
}

//*******************************************************************
//
int main(int argc, char *argv[])
{
#ifdef WIN32
	//trying to find memory leaks - this next line
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR "");
	textdomain (PACKAGE);
#endif

	gtk_set_locale();
	gtk_init(&argc, &argv);
  
	if (argc > 1) 
	{
		if (!strcmp(argv[1], "--help")) 
		{
			cout << "Usage: darwin [option]" << endl
			     << "\t Where viable options include: " << endl
			     //<< "\t --testreg (Tests the registration features by registering each" << endl
			     //<< "\t            outline in the database with every other one." << endl
			     //<< "\t            Dumps output to multiple files in current directory.)" << endl
			     << "\t --version (Prints program version and exits)" << endl;
			return 0;	
		}

		if (!strcmp(argv[1], "--version")) 
		{
			cout << PACKAGE << " " << VERSION << endl;
			return 0;	
		}

	}

	gOptions = new Options();
	readConfig();

	// ugly hack from waveletUtil.h
	initFilters();

#ifdef TIMING_ENABLED
	//***1.95 - open file for timing
	string fname = gOptions->mDarwinHome + PATH_SLASH + "DarwinAutotraceTimes.txt";
	timingOutFile.open(fname.c_str(),ios_base::out | ios_base::app); // append
	//***1.95 - end of new code
#endif

	add_pixmap_directory(PACKAGE_DATA_DIR "/pixmaps");
	add_pixmap_directory(PACKAGE_SOURCE_DIR "/pixmaps");
  
	gdk_rgb_init();
	SplashWindow *splash = new SplashWindow();
	splash->show();

	splash->updateStatus(_("Loading fin database..."));
	//Database *db = new Database(gOptions, false); //***1.99
	Database *db = openDatabase(gOptions, false); //***1.99

	//splash->updateStatus(_("Initializing interface..."));
	splash->startTimeout();

	MainWindow *mainWin = new MainWindow(db, gOptions);
	
	splash->mwDone(mainWin); //***1.85 - notify splash we are done & splash shows main window

	gtk_main(); //***1.85

	// ugly hack from waveletUtil.h
	destroyFilters();

	saveConfig();

#ifdef TIMING_ENABLED
	timingOutFile.close(); //***1.95 - to save timing information
#endif

	// just to be nice...
	delete gCfg;
	delete gOptions;

	return 0;
}
