//*******************************************************************
//   file: CreateDatabaseDialog.cxx
//
// author: J H Stewman (5/1/2007)
//
// used to create new survey areas and databases
//
//*******************************************************************

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#include "../support.h"
#include "CreateDatabaseDialog.h"
//#include "ErrorDialog.h"
#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#include "../../pixmaps/ok.xpm"
#include "../../pixmaps/cancel.xpm"
#include "../../pixmaps/logo_small.xpm"
#include "SaveFileChooserDialog.h"

#include <cstdio>

using namespace std;

// number of currently open CreateDatabaseDialog widgets
static int gNumReferences = 0; 

gboolean on_createDbDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

gboolean on_createDbButtonCNX_clicked(
		GtkButton *button,
		gpointer userData);

void on_createDbButtonOK_clicked(
		GtkButton *button,
		gpointer userData);

void on_createDbSurveyAreaCList_select_row(
		GtkCList *clist,
		gint row,
		gint column,
		GdkEvent *event,
		gpointer userData);

// following two callbacks probably NOT needed

void on_createDbDatabaseNameEntry_changed(
		GtkEntry *etnry,
		gpointer userData);

void on_createDbSurveyAreaNameEntry_changed(
		GtkEntry *entry,
		gpointer userData);

void on_radioNewSurveyAreaAndDb_clicked(
		GtkRadioButton *entry,
		gpointer userData);

void on_radioNewDbOnly_clicked(
		GtkRadioButton *entry,
		gpointer userData);
		
//*******************************************************************
//
// getNumCreateDatabaseDialogReferences()
//
//    Returns number of currently open CreateDatabaseDialog widgets.  Used by 
//    MainWindow to prevent opening more than one at a time.
//
int getNumCreateDatabaseDialogReferences()
{
	return gNumReferences;
}


//*******************************************************************
//
// CreateDatabaseDialog::CreateDatabaseDialog()
//
//    Creates the CreateDatabaseDialog widget and increments reference counter.
//
CreateDatabaseDialog::CreateDatabaseDialog(MainWindow *mainWin, Options *o, string archiveFilename)
: 
	mParentWindow(mainWin->getWindow()), // so we can set transient_for MainWin
	mMainWin(mainWin),
	mSurveyAreaList(NULL),
	mDatabaseList(NULL),
	mOptions(o),
	mArchiveName(archiveFilename) // name of archive being IMPORTED, if any
{
	if ("" == mArchiveName)
		mCreateMode = CreateDatabaseDialog::createNewDatabaseOnly;
	else
		mCreateMode = CreateDatabaseDialog::createNewSurveyArea;

	mDialog = createCreateDatabaseDialog(); // must follow setting of parent window above
	gNumReferences++;
}


//*******************************************************************
//
// CreateDatabaseDialog::~CreateDatabaseDialog()
//
//    Destroys the existing CreateDatabaseDialog widget and decrements reference 
//    counter.
//
CreateDatabaseDialog::~CreateDatabaseDialog()
{
	if (NULL != mDialog)
		gtk_widget_destroy(mDialog);
	
	gNumReferences--;
}


//*******************************************************************
//
// void CreateDatabaseDialog::show()
//
//    Shows the CreateDatabaseDialog widget.
//
void CreateDatabaseDialog::show()
{
	gtk_widget_show(mDialog);
}


//*******************************************************************
//
// GtkWidget* CreateDatabaseDialog::createCreateDatabaseDialog()
//
//    function to create the GTK Widget for the AboutDialog.
//
GtkWidget* CreateDatabaseDialog::createCreateDatabaseDialog()
{
	GtkWidget *createDbDialog;
	GtkWidget *createDbHBox;
	GtkWidget *createDbVBoxMain;
	GtkWidget *createDbVBox;
	GtkWidget *createDbPixmap;
	GtkWidget *createDbSurveyAreaListLabel;
	GtkWidget *createDbSurveyAreaList;
	GtkWidget *createDbDatabaseListLabel;
	GtkWidget *createDbDatabaseList;

	//GtkWidget *createDbNewSurveyAreaLabel;
	//GtkWidget *createDbNewSurveyAreaName;

	GtkWidget *createDbNewDatabaseLabel;
	//GtkWidget *createDbNewDatabaseName;

	GtkWidget *createDbActionArea;
	GtkWidget *createDbHButtonBox;
	GtkWidget *createDbButtonOK;
	GtkWidget *tmpBox, *tmpLabel, *tmpIcon;

	//int catIDnum;

	createDbDialog = gtk_dialog_new ();
	gtk_object_set_data (GTK_OBJECT(createDbDialog), "createDbDialog", createDbDialog);
	gtk_window_set_title (GTK_WINDOW (createDbDialog), _("Survey Areas and Databases ..."));
	GTK_WINDOW (createDbDialog)->type = WINDOW_DIALOG;
	gtk_window_set_position (GTK_WINDOW (createDbDialog), GTK_WIN_POS_CENTER);
	gtk_window_set_policy (GTK_WINDOW (createDbDialog), TRUE, TRUE, TRUE);
	gtk_window_set_wmclass(GTK_WINDOW(createDbDialog), "darwin_createDb", "DARWIN");

	gtk_window_set_modal(GTK_WINDOW(createDbDialog), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(createDbDialog), GTK_WINDOW(mParentWindow));

	createDbVBoxMain = GTK_DIALOG(createDbDialog)->vbox;
	gtk_object_set_data (GTK_OBJECT (createDbDialog), "createDbVBoxMain", createDbVBoxMain);
	gtk_widget_show (createDbVBoxMain);

	if (mArchiveName != "")
	{
		// we are IMPORTING, so identify the archive
		GtkWidget *archiveLabel = gtk_label_new("IMPORTING from archive ...");
		gtk_widget_show(archiveLabel);
		gtk_box_pack_start(GTK_BOX(createDbVBoxMain), archiveLabel, TRUE, TRUE, 0);

		GtkWidget *entry = gtk_entry_new();
		gtk_entry_set_text(GTK_ENTRY(entry), mArchiveName.c_str());
		gtk_widget_show(entry);
		gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
		gtk_box_pack_start(GTK_BOX(createDbVBoxMain), entry, TRUE, TRUE, 0);
	}

	createDbHBox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(createDbHBox);
	gtk_box_pack_start(GTK_BOX(createDbVBoxMain), createDbHBox, TRUE, TRUE, 0);

	createDbPixmap = create_pixmap_from_data(createDbDialog, logo_small_xpm);
	gtk_widget_show(createDbPixmap);
	gtk_box_pack_start(GTK_BOX(createDbHBox), createDbPixmap, TRUE, TRUE, 0);

	createDbVBox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(createDbVBox);
	gtk_box_pack_start(GTK_BOX(createDbHBox), createDbVBox, TRUE, TRUE, 10);

		
	// first we set up the left side of the window

	createDbSurveyAreaListLabel = gtk_label_new("Existing Survey Areas");
	gtk_widget_show (createDbSurveyAreaListLabel);
	gtk_box_pack_start (GTK_BOX (createDbVBox), createDbSurveyAreaListLabel, FALSE, FALSE, 10);

	// list the Existing Survey Area Names

	createDbSurveyAreaList = gtk_clist_new(1);
	gtk_widget_show(createDbSurveyAreaList);
	gtk_box_pack_start(GTK_BOX(createDbVBox), createDbSurveyAreaList, FALSE, FALSE, 0);

	mSurveyAreaList = createDbSurveyAreaList;

	gtk_clist_freeze(GTK_CLIST (createDbSurveyAreaList));

	for (int areaId = 0; areaId < mOptions->mNumberOfExistingSurveyAreas; areaId++)
	{
		//gchar *entry[1] = {_(mOptions->mExistingSurveyAreaName[areaId].c_str())}; //***2.22 - gtk4.5 complains
		gchar *entry[1] = {const_cast<char*>(_(mOptions->mExistingSurveyAreaName[areaId].c_str()))};

		gtk_clist_append(
			GTK_CLIST(createDbSurveyAreaList),
			entry);
	}

	gtk_clist_thaw(GTK_CLIST (createDbSurveyAreaList));

	// List the defined category names for selected scheme

	createDbDatabaseListLabel = gtk_label_new("Existing Database Names");
	gtk_widget_show (createDbDatabaseListLabel);
	gtk_box_pack_start (GTK_BOX (createDbVBox), createDbDatabaseListLabel, FALSE, FALSE, 10);

	createDbDatabaseList = gtk_clist_new(1);
	gtk_widget_show(createDbDatabaseList);
	gtk_box_pack_start(GTK_BOX(createDbVBox), createDbDatabaseList, FALSE, FALSE, 0);

	mDatabaseList = createDbDatabaseList;

	// now for the right side of the window

	createDbVBox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(createDbVBox);
	gtk_box_pack_start(GTK_BOX(createDbHBox), createDbVBox, TRUE, TRUE, 10);

	// two radio buttons to determine what mode we are in

	GtkWidget *actionLabel = gtk_label_new("Select what to create");
	gtk_widget_show (actionLabel);
	gtk_box_pack_start (GTK_BOX (createDbVBox), actionLabel, FALSE, FALSE, 10);

	GtkWidget *line = gtk_hseparator_new();
	gtk_widget_show(line);
	gtk_box_pack_start(GTK_BOX(createDbVBox), line, FALSE, FALSE, 0);

	GtkWidget *radioButtonBox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(radioButtonBox);
	gtk_box_pack_start(GTK_BOX(createDbVBox), radioButtonBox, FALSE, FALSE, 0);

	// button creating new database only (within selected survey area)
	GtkWidget *radioButton1 = gtk_radio_button_new_with_label(NULL,"New Database within\nSelected Survey Area");
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton1, FALSE,TRUE, 5);
	gtk_widget_show(radioButton1);
	GSList *radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton1));

	if ("" == mArchiveName)
		//  default is just "new db" when creating a NEW EMPTY database
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (radioButton1), TRUE); 
	else
		//  this is NOT even an option when IMPORTING a database
		gtk_widget_set_sensitive(GTK_WIDGET(radioButton1), FALSE);

	// button creating new survey area AND new database within it
	GtkWidget *radioButton2 = gtk_radio_button_new_with_label(radio_group,_("New Survey Area and\nNew Database"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton2, FALSE,TRUE, 5);
	gtk_widget_show(radioButton2);		
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton2));

	if ("" != mArchiveName)
		//  default is "new db and survey area" when IMPORTING a database
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (radioButton2), TRUE); 

	// connect callbacks AFTER setting initial valuse and sensitivity
	// this prevents callback execution BEFORE entry fields are defined below
	gtk_signal_connect (GTK_OBJECT(radioButton1),"toggled",
	                    GTK_SIGNAL_FUNC (on_radioNewDbOnly_clicked),
	                    (void *) this);

	gtk_signal_connect (GTK_OBJECT(radioButton2),"toggled",
	                    GTK_SIGNAL_FUNC (on_radioNewSurveyAreaAndDb_clicked),
	                    (void *) this);

	// scrolling list of Catalog Schemes
	GtkWidget *schemeLabel = gtk_label_new(_("Using Catalog Scheme"));
    gtk_widget_show(schemeLabel);
    gtk_box_pack_start(GTK_BOX(createDbVBox), schemeLabel, FALSE, FALSE, 10);

	mUsingScheme = gtk_combo_box_new_text();
	gtk_widget_show(mUsingScheme);
    gtk_box_pack_start(GTK_BOX(createDbVBox), mUsingScheme, FALSE, FALSE, 0);

	if ("" == mArchiveName) // not IMPORTING
	{
		int activeNum = 0;

		for (int schemeId = 0; schemeId < mOptions->mNumberOfDefinedCatalogSchemes; schemeId++)
		{
			gtk_combo_box_append_text(
					GTK_COMBO_BOX(mUsingScheme),
					_(mOptions->mDefinedCatalogSchemeName[schemeId].c_str()));
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(mUsingScheme), 
			mOptions->mCurrentDefaultCatalogScheme);
	}
	else // we are IMPORTING
	{
		gtk_combo_box_append_text(
				GTK_COMBO_BOX(mUsingScheme),
				_("<set by archive>"));

		gtk_combo_box_set_active(GTK_COMBO_BOX(mUsingScheme), 0);

		gtk_widget_set_sensitive(mUsingScheme, FALSE);
	}

	// give user a place to select / enter new survey area name
	createDbSurveyAreaListLabel = gtk_label_new("New Survey Area Name");
	gtk_widget_show (createDbSurveyAreaListLabel);
	gtk_box_pack_start (GTK_BOX (createDbVBox), createDbSurveyAreaListLabel, FALSE, FALSE, 10);

	mNewSurveyAreaName = gtk_entry_new();
	gtk_widget_show(mNewSurveyAreaName);
	gtk_box_pack_start(GTK_BOX(createDbVBox), mNewSurveyAreaName, FALSE, FALSE, 0);

	gtk_entry_set_text(GTK_ENTRY (mNewSurveyAreaName), "");

	// give user a place to enter a new database name
	createDbNewDatabaseLabel = gtk_label_new("New Database Name");
	gtk_widget_show (createDbNewDatabaseLabel);
	gtk_box_pack_start (GTK_BOX (createDbVBox), createDbNewDatabaseLabel, FALSE, FALSE, 10);

	mNewDatabaseName = gtk_entry_new();
	gtk_widget_show(mNewDatabaseName);
	gtk_box_pack_start(GTK_BOX(createDbVBox), mNewDatabaseName, FALSE, FALSE, 0);

	if ("" == mArchiveName)
		gtk_entry_set_text(GTK_ENTRY (mNewDatabaseName), "");
	else
	{
		gtk_entry_set_text(GTK_ENTRY (mNewDatabaseName), "<set by archive>");
		gtk_entry_set_editable(GTK_ENTRY (mNewDatabaseName), FALSE);
	}

	// now for space for user button(s)

	createDbActionArea = GTK_DIALOG (createDbDialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (createDbDialog), "createDbActionArea", createDbActionArea);
	gtk_widget_show (createDbActionArea);
	gtk_container_set_border_width (GTK_CONTAINER (createDbActionArea), 10);

	createDbHButtonBox = gtk_hbutton_box_new ();
	gtk_widget_show (createDbHButtonBox);
	gtk_box_pack_start (GTK_BOX (createDbActionArea), createDbHButtonBox, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (createDbHButtonBox), GTK_BUTTONBOX_END);

	// cancel button

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new(_("Cancel"));
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);
  
	GtkWidget *createDbButtonCNX = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(createDbButtonCNX), tmpBox);
	gtk_widget_show (createDbButtonCNX);
	gtk_container_add (GTK_CONTAINER (createDbHButtonBox), createDbButtonCNX);
	//GTK_WIDGET_SET_FLAGS (createDbButtonCNX, GTK_CAN_DEFAULT);

	// ok buton

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, ok_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new(_("OK"));
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);
  
	createDbButtonOK = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(createDbButtonOK), tmpBox);
	gtk_widget_show (createDbButtonOK);
	gtk_container_add (GTK_CONTAINER (createDbHButtonBox), createDbButtonOK);
	GTK_WIDGET_SET_FLAGS (createDbButtonOK, GTK_CAN_DEFAULT);

	gtk_signal_connect (GTK_OBJECT (createDbDialog), "delete_event",
						GTK_SIGNAL_FUNC (on_createDbDialog_delete_event),
						(void*)this);
	gtk_signal_connect (GTK_OBJECT (createDbButtonOK), "clicked",
						GTK_SIGNAL_FUNC (on_createDbButtonOK_clicked),
						(void*)this);
	gtk_signal_connect (GTK_OBJECT (createDbButtonCNX), "clicked",
						GTK_SIGNAL_FUNC (on_createDbButtonCNX_clicked),
						(void*)this);

	gtk_signal_connect (GTK_OBJECT (mSurveyAreaList), "select_row",
						GTK_SIGNAL_FUNC (on_createDbSurveyAreaCList_select_row),
						(void *) this);

	// we show only those databases within the currently selected survey area

	gtk_clist_select_row(GTK_CLIST(mSurveyAreaList), 0, 0);
	mSelectedArea = 0;

	gtk_widget_grab_default (createDbButtonOK);
	return createDbDialog;
}

//*******************************************************************
//
// gboolean on_createDbDialog_delete_event()
//
//    function to process "delete" events.
//
gboolean on_createDbDialog_delete_event(
		GtkWidget *widget,
		GdkEvent *event,
		gpointer userData)
{
	if (NULL == userData)
		return FALSE;

	delete (CreateDatabaseDialog*)userData;
	
	return TRUE;
}

//*******************************************************************
//
// gboolean on_createDbButtonCNX_clicked()
//
//    function to process "delete" events.
//
gboolean on_createDbButtonCNX_clicked(
		GtkButton *button,
		gpointer userData)
{
	if (NULL == userData)
		return FALSE;

	delete (CreateDatabaseDialog*)userData;
	
	return TRUE;
}


//*******************************************************************
//
// void on_createDbButtonOK_clicked()
//
//    function to process "OK button" events
//
void on_createDbButtonOK_clicked(
		GtkButton *button,
		gpointer userData)
{
	if (NULL == userData)
		return;

	CreateDatabaseDialog *dlg = (CreateDatabaseDialog *) userData;

	string 
		databaseName,
		surveyAreaName;

		databaseName = (char *) gtk_entry_get_text(GTK_ENTRY (dlg->mNewDatabaseName));
		surveyAreaName = (char *) gtk_entry_get_text(GTK_ENTRY (dlg->mNewSurveyAreaName));

	int 
		id,
		savedDefaultScheme,
		selectedScheme;
	bool 
		dataPathSelected(false), //***2.22
		duplicate;
	string 
		fullSurveyAreaName,
		fullDatabaseName,
		commandRoot,
		commandArgs,
		command;

	SaveFileChooserDialog *fsChooserDlg; //***2.22

	switch (dlg->mCreateMode)
	{
	case CreateDatabaseDialog::createNewSurveyArea :

		// a survey area name must be specified

		//***2.22 - here we have to determine whether to use 
		//          1) the existing data path or
		//          2) a new data path
								// this backup file came from another DARWIN installation

		fsChooserDlg = new SaveFileChooserDialog(NULL,
												NULL,
												NULL,
												NULL,
												dlg->mOptions,
												dlg->mDialog,
												SaveFileChooserDialog::chooseDataPath,
												NULL);
		dataPathSelected = fsChooserDlg->run_and_respond();

		if (! dataPathSelected)
		{
			showError("You must select or create a \"darwinPhotoIdData\" folder\nwithin which to create the new Survey Area and Catalog.", NULL);
			return; // do NOT create survey area or catalog
		}

		//***2.22 - looking for existing data path or CREATING a new one

		{
			string path = gOptions->mCurrentDataPath;
			if (path.rfind("darwinPhotoIdData") == string::npos) // force "darwinPhotoIdData" as root folder
			{
				path += PATH_SLASH;
				path += "darwinPhotoIdData";
			}
			gOptions->mCurrentDataPath = path; 
			dlg->mOptions->mCurrentDataPath = path; // both should be the same, but just in case

			// at this point the data path will end in "darwinPhotoIdData"
			
			if (dataPathExists(gOptions->mCurrentDataPath,false))
				cout << "Found data path: ";
			else
				cout << "Missing data path: ";
			cout << gOptions->mCurrentDataPath << endl;
			if (dataPathExists(path,true))
				cout << "Found data path: ";
			else
				cout << "Missing data path: ";
			cout << path << endl;
		}

		//***2.22 - end of new code to set data path


		if (surveyAreaName == "")
		{
			//***2.22 - added dlg->mDialog
			//ErrorDialog *err = new ErrorDialog(dlg->mDialog,"Survey Area must have a Name.");
			//err->show();
			//***2.22 - replacing own ErrorDialog with GtkMessageDialogs
			GtkWidget *errd = gtk_message_dialog_new (GTK_WINDOW(dlg->mDialog),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "Survey Area must have a Name.");
			gtk_dialog_run (GTK_DIALOG (errd));
			gtk_widget_destroy (errd);
			return; // must have a scheme name and at least one category
		}

		// survey area cannot already exist

		duplicate = false;
		for (id = 0; ((id < dlg->mOptions->mNumberOfExistingSurveyAreas) && (! duplicate)); id++)
		{
			gchar *namePtr;
			gtk_clist_get_text(GTK_CLIST(dlg->mSurveyAreaList),id,0,&namePtr);
			string currentName = namePtr;
			duplicate = (surveyAreaName == currentName);
		}

		if (duplicate)
		{
			//***2.22 - added mDialog
			//ErrorDialog *err = new ErrorDialog(dlg->mDialog,"Survey Area Name must differ from an existing one.");
			//err->show();
			//***2.22 - replacing own ErrorDialog with GtkMessageDialogs
			GtkWidget *errd = gtk_message_dialog_new (GTK_WINDOW(dlg->mDialog),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "Survey Area Name must differ from an existing one.");
			gtk_dialog_run (GTK_DIALOG (errd));
			gtk_widget_destroy (errd);
			return; 
		}

		if (dlg->mArchiveName == "") // we are NOT Importing from archive
		{
			// must have a database name

			if (databaseName == "")
			{
				//***2.22 - added mDialog
				//ErrorDialog *err = new ErrorDialog(dlg->mDialog,"Database must have a Name.");
				//err->show();
				//***2.22 - replacing own ErrorDialog with GtkMessageDialogs
				GtkWidget *errd = gtk_message_dialog_new (GTK_WINDOW(dlg->mDialog),
									GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_ERROR,
									GTK_BUTTONS_CLOSE,
									"Database must have a Name.");
				gtk_dialog_run (GTK_DIALOG (errd));
				gtk_widget_destroy (errd);
				return; 
			}

			// database name must end in ".db"

			if (databaseName.find(".db") == string::npos)
			{
				//force an ending of .db -- SAH
				databaseName+=".db";
				//ErrorDialog *err = new ErrorDialog("Database Name must have \".db\" extension.");
				//err->show();
				//return; 
			}

		}

		// set full paths to new Survey Area

		//***2.22 - this has to change to use the new data paths
		//fullSurveyAreaName = dlg->mOptions->mDarwinHome;
		fullSurveyAreaName = gOptions->mCurrentDataPath;
		fullSurveyAreaName += PATH_SLASH;
		fullSurveyAreaName += "surveyAreas";
		fullSurveyAreaName += PATH_SLASH;
		fullSurveyAreaName += surveyAreaName;

		cout << "Creating folders ..." << endl;

		// delete existing database

		if (dlg->mMainWin->mDatabase != NULL)
			delete dlg->mMainWin->mDatabase;

		//Clear default folders in SaveFileChooser (if they contain old survey area)
		SaveFileChooserDialog::clearLast(dlg->mOptions->mCurrentSurveyArea);

		// all is well, now we can create the new survey area and database

		if (dlg->mArchiveName == "")
		{
			// FORCE building of the new survey area folder structure here

			//rebuildFolders(dlg->mOptions->mDarwinHome, surveyAreaName, true);
			//***2.22 - now relative the the current data path, not DARWINHOME
			rebuildFolders(dlg->mOptions->mCurrentDataPath, surveyAreaName, true);

			// set full path to new database being created

			fullDatabaseName = fullSurveyAreaName;
			fullDatabaseName += PATH_SLASH;
			fullDatabaseName += "catalog";
			fullDatabaseName += PATH_SLASH;
			fullDatabaseName += databaseName;

			// save GLOBAL currentDefaultScheme and set as selected here (for this DB creation ONLY)

			savedDefaultScheme = dlg->mOptions->mCurrentDefaultCatalogScheme;
			selectedScheme = gtk_combo_box_get_active(GTK_COMBO_BOX(dlg->mUsingScheme));
			dlg->mOptions->mCurrentDefaultCatalogScheme = selectedScheme;

			// create new one

			dlg->mOptions->mDatabaseFileName = fullDatabaseName;
			dlg->mMainWin->mDatabase = openDatabase(dlg->mOptions, true); //***1.99

			// restore GLOBAL currentDefaultScheme

			dlg->mOptions->mCurrentDefaultCatalogScheme = savedDefaultScheme;

			// update rest of options to reflect new database, and current survey area

			dlg->mOptions->mCurrentSurveyArea = fullSurveyAreaName;
			dlg->mOptions->mNumberOfExistingDatabases ++;
			dlg->mOptions->mExistingDatabaseName.push_back((surveyAreaName + PATH_SLASH) + databaseName);
			dlg->mOptions->mNumberOfExistingSurveyAreas ++;
			dlg->mOptions->mExistingSurveyAreaName.push_back(surveyAreaName);

		}
		else
		{
			// we are IMPORTING from archive
			cout << "\nImporting database from ARCHIVE location ...\n\n  " << dlg->mArchiveName << endl;
			cout << "\n\nPlease wait." << endl;

			// grab database name from archive file in this case -- IGNORE the 
			// fullDatabaseName used above in non import case

			//***1.982 - put archive path/filename in QUOTES
			command = "7z x -aoa "; // extract and overwrite existing file
			command += quoted(dlg->mArchiveName) + " filesToArchive.txt";
			
			system(command.c_str()); // extract the file list file

			string shortDatabaseName;

			ifstream infile;

			infile.open("filesToArchive.txt");
			getline(infile,shortDatabaseName); // first is location of original backup
			getline(infile,shortDatabaseName); // this is the database file
			infile.close();

			// strip the old path info held in the archive
			shortDatabaseName = shortDatabaseName.substr(shortDatabaseName.rfind(PATH_SLASH) + 1);
			// and strip the trailing QUOTE (")
			shortDatabaseName = shortDatabaseName.substr(0,shortDatabaseName.length()-1);

			system("del filesToArchive.txt"); // remove temporary file list file

			string importPath = fullSurveyAreaName + PATH_SLASH + "catalog" + PATH_SLASH;

			// note: the import fuction itself will call the rebuildFolders() function
			// to build the folder structure, so no need to call for folder rebuild here

			importCatalogFrom(dlg->mArchiveName,
					importPath,
					//dlg->mOptions->mDarwinHome,
					dlg->mOptions->mCurrentDataPath, //***2.22
					surveyAreaName);

			// now, recreate the main window database from the newly imported database file

			dlg->mOptions->mDatabaseFileName = importPath + shortDatabaseName;
			//dlg->mMainWin->mDatabase = new Database(dlg->mOptions, false); //***1.99
			dlg->mMainWin->mDatabase = openDatabase(dlg->mOptions, false); //***1.99

			// update rest of options to reflect new database, and current survey area

			dlg->mOptions->mCurrentSurveyArea = fullSurveyAreaName;
			dlg->mOptions->mNumberOfExistingDatabases ++;
			dlg->mOptions->mExistingDatabaseName.push_back((surveyAreaName + PATH_SLASH) + shortDatabaseName);
			dlg->mOptions->mNumberOfExistingSurveyAreas ++;
			dlg->mOptions->mExistingSurveyAreaName.push_back(surveyAreaName);
		}

		// make sure main window looks good - and force reload of fins into CList
		dlg->mMainWin->resetTitleButtonsAndBackupOnDBLoad();
		dlg->mMainWin->show();

		break;

	case CreateDatabaseDialog::createNewDatabaseOnly :

		// CANNOT Import into an existing survey area -- MUST create new one

		if (dlg->mArchiveName != "")
		{
			//***2.22 - added mDialog
			//ErrorDialog *err = new ErrorDialog(dlg->mDialog,"IMPORT into existing survey areas is NOT allowed.");
			//err->show();
			//***2.22 - replacing own ErrorDialog with GtkMessageDialogs
			GtkWidget *errd = gtk_message_dialog_new (GTK_WINDOW(dlg->mDialog),
								GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_ERROR,
								GTK_BUTTONS_CLOSE,
								"IMPORT into existing survey areas is NOT allowed.");
			gtk_dialog_run (GTK_DIALOG (errd));
			gtk_widget_destroy (errd);
			return; 
		}

		// must have and area name 

		if (surveyAreaName == "") 
		{
			//***2.22 - added mDialog
			//ErrorDialog *err = new ErrorDialog(dlg->mDialog,"Survey Area must have a Name.");
			//err->show();
			//***2.22 - replacing own ErrorDialog with GtkMessageDialogs
			GtkWidget *errd = gtk_message_dialog_new (GTK_WINDOW(dlg->mDialog),
								GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_ERROR,
								GTK_BUTTONS_CLOSE,
								"Survey Area must have a Name.");
			gtk_dialog_run (GTK_DIALOG (errd));
			gtk_widget_destroy (errd);
			return; 
		}
		
		// must have a database name

		if (databaseName == "")
		{
			//***2.22 - added mDialog
			//ErrorDialog *err = new ErrorDialog(dlg->mDialog,"Database must have a Name.");
			//err->show();
			//***2.22 - replacing own ErrorDialog with GtkMessageDialogs
			GtkWidget *errd = gtk_message_dialog_new (GTK_WINDOW(dlg->mDialog),
								GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_ERROR,
								GTK_BUTTONS_CLOSE,
								"Database must have a Name..");
			gtk_dialog_run (GTK_DIALOG (errd));
			gtk_widget_destroy (errd);
			return; 
		}

		// database name must end in ".db"

		if (databaseName.find(".db") == string::npos)
		{
			//force extension -- SAH (some of this really out to be extracted to a function. We have code around this block in two locations.
			databaseName += ".db";
			//ErrorDialog *err = new ErrorDialog("Database Name must have \".db\" extension.");
			//err->show();
			//return; 
		}

		// database cannot already exist

		duplicate = false;
		for (id = 0; ((id < dlg->mNumDatabases) && (! duplicate)); id++)
		{
			gchar *namePtr;
			gtk_clist_get_text(GTK_CLIST(dlg->mDatabaseList),id,0,&namePtr);
			string currentName = namePtr;
			duplicate = (databaseName == currentName);
		}

		if (duplicate)
		{
			//***2.22 - added mDialog
			//ErrorDialog *err = new ErrorDialog(dlg->mDialog,"Database Name must differ from an existing one.");
			//err->show();
			//***2.22 - replacing own ErrorDialog with GtkMessageDialogs
			GtkWidget *errd = gtk_message_dialog_new (GTK_WINDOW(dlg->mDialog),
								GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_ERROR,
								GTK_BUTTONS_CLOSE,
								"Database Name must differ from an existing one.");
			gtk_dialog_run (GTK_DIALOG (errd));
			gtk_widget_destroy (errd);
			return; 
		}

		// all is well, now we can create the new database

		// 2.22 - again, here we need to use the current data path, NOT DARWINHOME
		//fullSurveyAreaName = dlg->mOptions->mDarwinHome;
		fullSurveyAreaName = dlg->mOptions->mCurrentDataPath;
		fullSurveyAreaName += PATH_SLASH;
		fullSurveyAreaName += "surveyAreas";
		fullSurveyAreaName += PATH_SLASH;
		fullSurveyAreaName += surveyAreaName;

		fullDatabaseName = fullSurveyAreaName;
		fullDatabaseName += PATH_SLASH;
		fullDatabaseName += "catalog";
		fullDatabaseName += PATH_SLASH;
		fullDatabaseName += databaseName;

		cout << "Creating : " << fullDatabaseName << endl;

		// delete existing database

		if (dlg->mMainWin->mDatabase != NULL)
			delete dlg->mMainWin->mDatabase;

		//Clean up "default" save directories
		SaveFileChooserDialog::clearLast(dlg->mOptions->mCurrentSurveyArea);

		// save GLOBAL currentDefaultScheme and set as selected here (for this DB creation ONLY)

		savedDefaultScheme = dlg->mOptions->mCurrentDefaultCatalogScheme;
		selectedScheme = gtk_combo_box_get_active(GTK_COMBO_BOX(dlg->mUsingScheme));
		dlg->mOptions->mCurrentDefaultCatalogScheme = selectedScheme;

		// create new one

		dlg->mOptions->mDatabaseFileName = fullDatabaseName;
		//dlg->mMainWin->mDatabase = new Database(dlg->mOptions, true); //***1.99
		dlg->mMainWin->mDatabase = openDatabase(dlg->mOptions, true); //***1.99

		// restore GLOBAL currentDefaultScheme

		dlg->mOptions->mCurrentDefaultCatalogScheme = savedDefaultScheme;

		// update rest of options to reflect new database, and current survey area

		dlg->mOptions->mCurrentSurveyArea = fullSurveyAreaName;
		dlg->mOptions->mNumberOfExistingDatabases ++;
		dlg->mOptions->mExistingDatabaseName.push_back((surveyAreaName + PATH_SLASH) + databaseName);

		// make sure main window looks good - and force reload of fins in CList
		dlg->mMainWin->resetTitleButtonsAndBackupOnDBLoad();
		dlg->mMainWin->show();

		break;
	}

	delete dlg;
}

//*******************************************************************
//
// void on_createDbCList_select_row()
//
//    function to process "scheme selection" events
//
void on_createDbSurveyAreaCList_select_row(
	GtkCList *clist,
	gint row,
	gint column,
	GdkEvent *event,
	gpointer userData)
{
	CreateDatabaseDialog *createDBWin = (CreateDatabaseDialog *) userData;

	if (NULL == createDBWin)
		return;

	gtk_clist_freeze(GTK_CLIST (createDBWin->mDatabaseList));

	gtk_clist_clear(GTK_CLIST (createDBWin->mDatabaseList));

	Options *o = createDBWin->mOptions;

	gchar *namePtr;
	gtk_clist_get_text(clist,row,column,&namePtr);
	string currentAreaName = namePtr;

	createDBWin->mNumDatabases = 0; // number of db's in selected survey area

	for (int dbId = 0; dbId < o->mNumberOfExistingDatabases; dbId++)
	{
		string areaName = o->mExistingDatabaseName[dbId];

		areaName = areaName.substr(0,areaName.find(PATH_SLASH));

		if (areaName == currentAreaName)
		{
			string dbName = o->mExistingDatabaseName[dbId];
			dbName = dbName.substr(dbName.find(PATH_SLASH)+1);

			//gchar *entry[1] = {_(dbName.c_str())}; //***2.22 - g++4.5 complains
			gchar *entry[1] = {const_cast<char*>(_(dbName.c_str()))};

			gtk_clist_append(
				GTK_CLIST(createDBWin->mDatabaseList),
				entry);

			createDBWin->mNumDatabases ++;
		}
	}

	createDBWin->mSelectedArea = row;

	gtk_clist_thaw(GTK_CLIST (createDBWin->mDatabaseList));

	if (createDBWin->mCreateMode == CreateDatabaseDialog::createNewDatabaseOnly)
	{
		gchar *namePtr;
		gtk_clist_get_text(
			GTK_CLIST(createDBWin->mSurveyAreaList),
			createDBWin->mSelectedArea,0,&namePtr);

		gtk_entry_set_text(GTK_ENTRY(createDBWin->mNewDatabaseName),"");
		gtk_entry_set_text(GTK_ENTRY(createDBWin->mNewSurveyAreaName),namePtr);
		gtk_entry_set_editable(GTK_ENTRY(createDBWin->mNewSurveyAreaName), FALSE); //***1.85

	}
}

//**************************************************************
void on_radioNewSurveyAreaAndDb_clicked(
				GtkRadioButton *entry,
				gpointer userData)
{
	CreateDatabaseDialog *dlg = (CreateDatabaseDialog *) userData;

	dlg->mCreateMode = CreateDatabaseDialog::createNewSurveyArea;

	if ("" == dlg->mArchiveName)
		gtk_entry_set_text(GTK_ENTRY(dlg->mNewDatabaseName),"");
	gtk_entry_set_text(GTK_ENTRY(dlg->mNewSurveyAreaName),"");
	gtk_entry_set_editable(GTK_ENTRY(dlg->mNewSurveyAreaName), TRUE); //***1.85
}


//*************************************************************

void on_radioNewDbOnly_clicked(
				GtkRadioButton *entry,
				gpointer userData)
{
	CreateDatabaseDialog *dlg = (CreateDatabaseDialog *) userData;

	dlg->mCreateMode = CreateDatabaseDialog::createNewDatabaseOnly;

	gchar *namePtr;
	gtk_clist_get_text(
		GTK_CLIST(dlg->mSurveyAreaList),
		dlg->mSelectedArea,0,&namePtr);

	gtk_entry_set_text(GTK_ENTRY(dlg->mNewDatabaseName),"");
	gtk_entry_set_text(GTK_ENTRY(dlg->mNewSurveyAreaName),namePtr);
	gtk_entry_set_editable(GTK_ENTRY(dlg->mNewSurveyAreaName), FALSE); //***1.85
}

/*
//*******************************************************************
//***2.22 - new callback to save new SurveyArea using a NEW or OTHER darwinDataPath
void on_finzDialogButtonSaveImages_clicked(
		GtkButton *button,
		gpointer userData)
{
	ExportFinzDialog *dlg = (ExportFinzDialog *) userData;

	if (NULL == dlg)
		return;

	bool saved = false;
	//set<int> selectedFins = selectedRows(GTK_CLIST(dlg->mCList));
	set<long> selectedFins = selectedRows(GTK_CLIST(dlg->mCList)); //***2.22 - now 64 bit arch on Mac

	if (selectedFins.empty()) {//idiot, selected something first for export
		return;
	} else {// one or more fins selected
		//set<int>::iterator it;
		set<long>::iterator it; //***2.22 - now 64 bit arch on Mac
		vector<DatabaseFin<ColorImage>* > fins;
		for (it = selectedFins.begin(); it != selectedFins.end(); it++) {
			int id = dlg->mRow2Id[*it];
			fins.push_back(dlg->mDatabase->getItem(id));
		}

		SaveFileChooserDialog *fsChooserDlg = new SaveFileChooserDialog(dlg->mDatabase,
												NULL,
												NULL,
												NULL,
												dlg->mOptions,
												dlg->mDialog,
												SaveFileChooserDialog::saveFullSizeModImages,
												&fins);
		saved=fsChooserDlg->run_and_respond();

		vector<DatabaseFin<ColorImage>* >::iterator finsit;
		for (finsit = fins.begin(); finsit != fins.end(); finsit++) {
			delete ((DatabaseFin<ColorImage>*) *finsit);
		}
		
	}

	if (saved)
		delete dlg;


}
*/

