//*******************************************************************
//   file: IntensityContour.cxx
//
//*******************************************************************

///////////////////////////////////////////////////////////////////////////
//                                                                       //
//	DARWIN Software                                                      //
//                                                                       //
// Digital Analysis and Recognition of Whale Image on a Network          //
// Digital Analysis to Recognize Whale Images on a Network               //
// Digital Archive of Recognized Whale Images on a Network               //
//                                                                       //
// Software designed and coded as a collaborative effort ...             //
//                                                                       //
// ... by  The DARWIN Research Group                                     //
//                                                                       //
//	Kelly R. Debure, PhD (GTK+, X Window System version)                 //
//	John H. Stewman, PhD (all versions)                                  //
//	Kristen McCoy (GTK+ Version)                                         //
//	Henry Burroughs (GTK+ Version)                                       //
//	Zach Roberts (X Window System version)                               //
//	Daniel J. Wilkin (X Window System version)                           //
//	Mark C. Allen (Microsoft Win 3.1 version, Expert System version)     //
//	Rachel Stanley (Expert System version)                               //
//	Andrew Cameron (protype database)                                    //
//	Adam Russell (major Object Oriented rewrite & Gtk GUI)               //
//  Scott Hale (autotrace, finz file support, misc. gui/HCI ...)         //
//  R J Nowling (SQLite database, finz file support)                     // 
//                                                                       //
// ... at  Eckerd College                                                //
//	       St.Petersburg, FL                                             //
//                                                                       //
// xdarwin (DARWIN for the X Window System)                              //
//                                                                       //
// ... begun on July 27, 1994 (JHS)                                      //
// ... and updated on July 28, 1994 (JHS)                                //
// ... and updated starting June 12, 1995 (DW)                           //
// ... then maintained by Daniel J. Wilkin and Zach Roberts              //
// ... major rewrite using GTK+ 2000-2002 (Adam Russell)                 //
// ... currently maintained by JHS & KRD                                 //
//                                                                       //
// Supported by grants from the National Science Foundation              //
//                                                                       //
// Species Domain - Common Bottlenose Dolphin - Tursiops truncatus       //
//                                                                       //
// Copyright (C) 2001                                                     //
///////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
//#include <vld.h> //***2.0 - to enable Visual Leak Detector
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h> //***2.22 for threads

#include <gtk/gtk.h>
#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

#include "CatalogSupport.h" //SAH
#include "Interface/TraceWindow.h" //SAH
#include "ConfigFile.h"
#include "support.h"
#include "interface/MainWindow.h"
#include "CatalogSupport.h"
#include "Options.h"
#include "interface/SplashWindow.h"
#include "waveletUtil.h"

// trying to find memory leaks - next 3 lines
//***2.01 - removed from Release version
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#ifdef WIN32
//#include <crtdbg.h>
//#endif

#include <iostream>
#ifdef WIN32
#include <direct.h>
#else
#include <sys/param.h> //***2.22 - for MAXPATHLEN
#endif

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
void readConfig(string homedir)
{

	//***2.22 - major rewrite to clean up for Windows and Mac platforms

	gOptions->mDarwinHome = homedir; // homedir should always be defined in main()

	string fileName; // for configuration filename

#ifdef WIN32

	string tempdir = getenv("temp");
	gOptions->mTempDirectory = tempdir + "\\darwin";
	
	fileName = homedir + "\\system\\darwin.cfg";

	gCfg = new ConfigFile(fileName.c_str());

	// if config found, then save a backup, else load the backup copy
	if (gCfg->size() > 0)
		gCfg->save((fileName+".bak").c_str());
	else
		gCfg->open((fileName+".bak").c_str());
	
	if ((!gCfg->getItem("DatabaseFileName", gOptions->mDatabaseFileName)) ||
		(gOptions->mDatabaseFileName == "NONE"))
	{
		// if no database found then set default survey area and NONE as database
		//gOptions->mCurrentSurveyArea = homedir + "\\surveyAreas\\default";
		gOptions->mCurrentSurveyArea = "NONE"; //***2.22 - set below based on current data path
		gOptions->mDatabaseFileName = "NONE";
	}
	else
	{
		// if database is found, then break out the currentSurveyArea root name
		int pos = gOptions->mDatabaseFileName.rfind("\\catalog");
		gOptions->mCurrentSurveyArea = gOptions->mDatabaseFileName.substr(0,pos);
	}

#else

	//***2.22 - no predefined "temp" dir on Mac
	gOptions->mTempDirectory = getenv("HOME");
	gOptions->mTempDirectory += "/darwintmp";

	fileName = homedir + "/system/darwinrc";
	gCfg = new ConfigFile(fileName);

	// if config found, save a backup, else load backup copy
	if (gCfg->size() > 0)
		gCfg->save((fileName+"~").c_str());
	else
		gCfg->open((fileName+"~").c_str());

	if (!gCfg->getItem("DatabaseFileName", gOptions->mDatabaseFileName)) 
	{
		// if no database found then set default survey area and NONE as database
		//gOptions->mCurrentSurveyArea = homedir + "/surveyAreas/default";
		gOptions->mCurrentSurveyArea = "NONE"; //***2.22 - set below based on current data path
		gOptions->mDatabaseFileName = "NONE";
	}
	else
	{
		// if database is found, then break out the currentSurveyArea root path
		int pos = gOptions->mDatabaseFileName.rfind("/catalog");
		gOptions->mCurrentSurveyArea = gOptions->mDatabaseFileName.substr(0,pos);
	}

#endif

	//***2.22 - new item in configuration
	if (!gCfg->getItem("CurrentDataPath", gOptions->mCurrentDataPath))
	{
		// no action needed, we set this initially in main before calling readConfig()
	}

	//***2.22 - NOW set the current survey area based on the current data path
	if ("NONE" == gOptions->mCurrentSurveyArea)
	{		
		// this happens when NO database found
		gOptions->mCurrentSurveyArea = gOptions->mCurrentDataPath + PATH_SLASH 
					+ "surveyAreas" + PATH_SLASH + "default";
	}

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
		// no default catalog defined in config file so use Eckerd College catalog
		gOptions->mCurrentDefaultCatalogScheme = 0;
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
		//****1.85 - default is now NO DATABASE
		gOptions->mNumberOfExistingDatabases = 0;
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

	gCfg->addItem("DARWINHOME", gOptions->mDarwinHome); // info only - pgm doesn't use
	gCfg->addItem("CurrentDataPath", gOptions->mCurrentDataPath); //***2.22
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
	//g_thread_init(NULL); //***2.22
	//gdk_threads_init(); //***2.22

#ifdef WIN32
	//trying to find memory leaks - this next line
//	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR "");
	textdomain (PACKAGE);
#endif

	gtk_set_locale();

	gtk_init(&argc, &argv);

	// handle any/all command line arguments
	// here are the possible options (DARWIN 2.0)
	// --set-home="..."   this directly sets gOptions->mDarwinHome
	// --help             this invokes the command line help
	// --version          DARWIN returns the current version(s)
	// "*.finz"           DARWIN opens as a fin viewer for this Fin ONLY

	// DARWIN uses the following strategy to find its HOME path for this
	// invocation of the program
	// (1) if a --set-home option is found, then the specified path is used
	// (2) else if argv[0] contains a path, then that path is used
	// (3) else the current working directory is used
	//
	// (4) if a path is found in the darwin.cfg file AND it differs from any set above,
	//     then the user is querried for clarification????
	// (5) if the DARWINHOME and DATABASE paths in the darwin.cfg file differ
	//     then the user is querried for clarification????
	//
	// The environment variable DARWINHOME is no longer used in any way.

	string darwinhome(argv[0]);//first argument is absolute if double-clicked in windows; otherwise, may be relative
	//we will overide this if argv[1]==--set-home="..."
	//cout << "DH: " << darwinhome << endl; //***2.22 - diagnostic
#ifdef WIN32		
	int basePos = darwinhome.rfind("\\system\\bin\\darwin.exe");
	if (basePos!=string::npos) {
		darwinhome=darwinhome.substr(0,basePos);
	} else { //try current directory...may be relative if executed from command line
		char c_cwd[_MAX_PATH];   // _MAX_PATH is only on windows
		getcwd(c_cwd, _MAX_PATH);
		string cwd(c_cwd);
		darwinhome=cwd;
		basePos = darwinhome.rfind("\\system\\bin");
		if (basePos!=string::npos)	{
			darwinhome=darwinhome.substr(0,basePos);
		}
#else
	//***2.22 - on Mac the path is always relative
	// so try current directory
	char c_cwd[MAXPATHLEN]; //***2.22 - for Mac & Linux
	getcwd(c_cwd, MAXPATHLEN);
	string cwd(c_cwd);
	darwinhome=cwd;
	int basePos = darwinhome.rfind("/system/bin");
	if (basePos!=string::npos)	{
		darwinhome=darwinhome.substr(0,basePos);
#endif
	}

	//cout << "DH: " << darwinhome << endl; //***2.22 - diagnostic

	vector<string> options; // so we can handle multiple command line options
	string finz("");       // assume only one of these

	//***2.22 - chaged string argv to argV -- do ont know why this worked before - JHS
	for (int i=1; i<argc; i++) {
		string argV(argv[i]);
		if (argV.find("--")==0) {//we have an option index
			options.push_back(argV);
		} else if (argV.find(".finz")!=string::npos) {//we have a finz file
			finz = argV;
		}
	}

	string option;
	while (!options.empty()) 
	{
		option = options.back();
		options.pop_back();

		if (option.find("--help")!=string::npos) 
		{
			cout << "Usage: darwin [option] [filename.finz]" << endl
			     << "\t Where viable options include: " << endl
			     << "\t --set-home=\"...\" (Set DARWINHOME; else, use path containing .exe)" << endl
			     << "\t --version (Print program version and exit)" << endl
			     << "\t --help (Print this usage message and exit)" << endl
				 << endl
				 << "\t If a filename.finz is given, open Darwin as a FIN viewer only." << endl;
			return 0;	
		}

		if (option.find("--version")!=string::npos) 
		{
			cout << PACKAGE << " " << VERSION << endl;
			return 0;	
		}

		if (option.find("--set-home=")!=string::npos) {			
			darwinhome=option.substr(option.find("=")+1);
			//override darwinhome with user supplied path
		}
	}

	gOptions = new Options();

	//***2.22 - new section to support multiple data areas OUTSIDE DARWINHOME
	//
	// NOTE and WARNING: the HOMEPATH var in Windows does NOT include the drive letter
	// We have to prepend HOMEDRIVE to get the complete path
	//
	gOptions->mNumberOfExistingDataPaths = 1;
#ifdef WIN32
	gOptions->mCurrentDataPath = getenv("HOMEDRIVE");
	if (NULL != getenv("HOMEPATH"))
		gOptions->mCurrentDataPath += getenv("HOMEPATH"); // user's home
#else
	if (NULL != getenv("HOME"))
		gOptions->mCurrentDataPath = getenv("HOME"); // Mac & Linux - user's home
#endif
	
	if ("" != gOptions->mCurrentDataPath)
	{
		string myhome = gOptions->mCurrentDataPath;
		gOptions->mCurrentDataPath += PATH_SLASH;
		gOptions->mCurrentDataPath +=  "Documents"; // user's Documents folder
		if (! filespecFound(gOptions->mCurrentDataPath))
		{
			// this must be a Windows XP platform with a "My Documents" foler
			gOptions->mCurrentDataPath = myhome + PATH_SLASH + "My Documents";
			if (! filespecFound(gOptions->mCurrentDataPath))
				showError("no Documents or My Documents folder found.");
		}
		gOptions->mCurrentDataPath += PATH_SLASH;
		gOptions->mCurrentDataPath += "darwinPhotoIdData"; // new standard folder name
	}
	else
		gOptions->mCurrentDataPath = gOptions->mDarwinHome; // OLD way - application folder

	//showError(darwinhome+"\n"+gOptions->mCurrentDataPath);

	// read configuration AFTER setting default darwinhome and data path

	readConfig(darwinhome);
	//cout << "DARWINHOME=" << gOptions->mDarwinHome << endl;


	//***2.22 - new - MOVE data OUT of Application folders
	// if this is the first time DARWIN has run, then the "firstTime.txt" file
	// exists in the system folder.  If so, then check whether gOptions->mCurrentDataPath
	// exists.  If NOT, then force CREATE gOptions->mCurrentDataPath and MOVE the 
	// surveyAreas and backups folders from the Application folder (gOptions->mDarwinHome)
	// to gOptions->mCurrentDataPath
	string firstTime = darwinhome + PATH_SLASH + "system" + PATH_SLASH + "firstTime.txt";
	if (filespecFound(firstTime))
	{
		if (! filespecFound(gOptions->mCurrentDataPath))
		{
			moveAreasAndBackups(true); // MOVE surveyAreas and backups folders OUT of application
			// change current survey area to "default" in the current data path folder
			gOptions->mCurrentSurveyArea = gOptions->mCurrentDataPath + PATH_SLASH 
					+ "surveyAreas" + PATH_SLASH + "default";
		}
		string command = "";
#ifdef WIN32
		command += "del ";
#else
		command += "rm -f \"";
#endif
		command += firstTime + "\""; // put filenames in quotes in case of spaces in path
		system(command.c_str());    // remove the "system/firstTime.txt" file
	}

	gOptions->mExistingDataPaths.push_back(gOptions->mCurrentDataPath); // push first data path

	//cout << "DataHome: " << gOptions->mCurrentDataPath << endl;
	//showError(gOptions->mDarwinHome+"\n"+gOptions->mCurrentDataPath);

	//***2.22 - end of new multiple data path code (for now)

	//SAH
	//Create Temporary Directory
	string cmd = "";
	cmd = "mkdir \"" + gOptions->mTempDirectory + "\"";
	system(cmd.c_str());

	// ugly hack from waveletUtil.h
	initFilters();

#ifdef TIMING_ENABLED
	//***1.95 - open file for timing
	string fname = gOptions->mDarwinHome + PATH_SLASH + "DarwinAutotraceTimes.txt";
	timingOutFile.open(fname.c_str(),ios_base::out | ios_base::app); // append
	//***1.95 - end of new code
#endif

	//***2.22 - using strings - problems the old way with build on Mac
	string packageDataDir(PACKAGE_DATA_DIR);
	string packageSrcDir(PACKAGE_SOURCE_DIR);
	packageDataDir += "/pixmaps";
	packageSrcDir += "/pixmaps";
	add_pixmap_directory(packageDataDir.c_str()); //***2.22 - was PACKAGE_DATA_DIR "/pixmaps"
	add_pixmap_directory(packageSrcDir.c_str());  //***2.22 - was PACKAGE_SOURCE_DIR "/pixmaps"
  
	gdk_rgb_init();

	//***2.0 - create a compound command to call 7z -- first prepending current
	// darwin system\bin to search PATH and then calling 7z -- this is necessary
	// so 7z can be found when darwin starts as a stand alone FINZ viewer (JHS)
#ifdef WIN32
	gOptions->SevenZ = "set PATH=" + gOptions->mDarwinHome + "\\system\\bin;%PATH% & 7z"; //***2.0
#else
	gOptions->SevenZ = "export PATH=" + gOptions->mDarwinHome + "/system/bin;$PATH & 7z"; //***2.0
#endif
	if (finz.empty()) { //Standard open
		SplashWindow *splash = new SplashWindow();
		splash->show();
		splash->updateStatus(_("Loading fin database..."));
		Database *db = openDatabase(gOptions, false); //***1.99

		splash->startTimeout();

		MainWindow *mainWin = new MainWindow(db, gOptions);
		splash->mwDone(mainWin); //***1.85 - notify splash we are done & splash shows main window

		gdk_threads_enter(); //***2.22
		gtk_main(); //***1.85
		gdk_threads_leave(); //***2.22

		saveConfig();

	} else { 

		DatabaseFin<ColorImage>* dbFin = openFinz(finz);
		if (NULL!=dbFin) {
			TraceWindow *traceWin= new TraceWindow(
					   NULL, //MainWindow
					   finz,
					   dbFin,
					   NULL, //Database
					   gOptions);
			traceWin->show();
			
			//gdk_threads_enter(); //***2.22
			gtk_main();
			//gdk_threads_leave(); //***2.22
			
			// All clean up is handeled in the TraceWindow
			// Do NOT delete traceWin or dbFin here
			
		} else {//bad format finz

			cout << "Invalid finz format." << endl;
			GtkWidget *dialog = gtk_dialog_new_with_buttons (
				"Cannot open file",
				NULL, // do not have parent
				(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
				GTK_STOCK_OK,
				GTK_RESPONSE_ACCEPT,
				NULL);

				gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 140);

				GtkWidget *label = gtk_label_new("Cannot open file. It is not a valid finz format.");
				gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),label);
				gtk_widget_show(label);

				gint result = gtk_dialog_run (GTK_DIALOG (dialog));

				gtk_widget_destroy (dialog);
		}
	}
	
	// ugly hack from waveletUtil.h
	destroyFilters();

	//SAH--Remove Temporary Directory -- just to be nice
#ifdef WIN32
	cmd = "rmdir /s /q \"" + gOptions->mTempDirectory + "\"";
#else
	cmd = "rm -r \"" + gOptions->mTempDirectory + "\""; //***2.22 - for Mac
#endif
	system(cmd.c_str());

#ifdef TIMING_ENABLED
	timingOutFile.close(); //***1.95 - to save timing information
#endif

	// just to be nice...
	delete gCfg;
	delete gOptions;

	return 0;
}
