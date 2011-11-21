//*******************************************************************
//   file: SaveFileChooserDialog.cxx
//
// author: John H Stewman
//
//   mods: 7/25/2009 -- support for saving images (full-size)
//
//   date: 7/16/2008
//
// This is used to save DatabaseFins , export catalogs, and 
// choose locations for other exports (finz and data).
// This replaces the SaveFileSelectionDialog class - JHS
//
// In versions 2.02 and later it also supports saving of full-size, 
// full-resolution versions of DARWIN modified images that are otherwise
// saved as thumbnails inside the catalog folder and are rebuilt
// as full-size images on-the-fly whenever DARWIN needs to access
// them.  JHS 7/25/2009
//*******************************************************************

#include <gdk/gdkkeysyms.h>
#include "../support.h"
#include "SaveFileChooserDialog.h"
//#include "ErrorDialog.h"
#include "../CatalogSupport.h"
#include "../Utility.h"

#ifdef WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "/"
#endif

using namespace std;

static int gNumReferences = 0;
//static string gLastDirectory = "";   // disk & path last in use

static int mNumberModes = 7; //how many modes are above in the enum? ***2.22 - updated to 7
static string gLastDirectory[] = {"","","","","","","",""};   // disk & path last in use  //SAH 8 strings for 8 mSaveModes
static string gLastFileName[] = {"","","","","","","",""};    // simple name of last file "touched" -- if any
static string gLastFolderName[] = {"","","","","","","",""};  // simple name of folder last "touched" -- if any

int getNumSaveFileChooserDialogReferences()
{
	return gNumReferences;
}

SaveFileChooserDialog::SaveFileChooserDialog(
	Database *db,
	DatabaseFin<ColorImage> *dbFin,
	MainWindow *mainWin,
	TraceWindow *traceWin,
	Options *o,
	GtkWidget *parent,
	int saveMode,
	vector<DatabaseFin<ColorImage>* > *fins
)
	:	mDatabase(db),
		mFin(dbFin),
		mMainWin(mainWin),
		mTraceWin(traceWin), //***1.2 - so we can delete TraceWindow Object when done
		mOptions(o),
		mParent(parent),      //***1.2 - the parent GTK Window Widget
		mSaveMode(saveMode),
		mFins(fins)
{
	mDialog = createSaveFileChooser(); //***1.1 - this must follow intilization of mFin

	gNumReferences++;
}

SaveFileChooserDialog::~SaveFileChooserDialog()
{
	gtk_widget_destroy(mDialog);

	//delete mFin; // just a pointer to the TraceWindow::mFin, do NOT delete here
	
	gNumReferences--;
}

/**Clear last directory with old survey area
This funtion should be called whenever the current survey area changes.
It is intended to avoid problems of the user being placed in the wrong 
"default" folder because of the gLastDirectory, gLastFileName, and gLastFolderName variables

  @param string mPreviousSurveyArea A string of the previous survey area

  Assumes gLastDirectory is not NULL. (Should never be)
**/
void SaveFileChooserDialog::clearLast(string mPreviousSurveyArea) {
	for (int i=0; i<mNumberModes; i++) {
		if (""!=gLastDirectory[i] && gLastDirectory[i].find(mPreviousSurveyArea)!=string::npos) {
			gLastDirectory[i]="";
			gLastFileName[i]="";
			gLastFolderName[i]="";
		}
	}
	return;
}

GtkWidget* SaveFileChooserDialog::createSaveFileChooser (void)
{
/*	GtkWidget *saveFileSelection;
	GtkWidget *saveButtonOK;
	GtkWidget *saveButtonCancel;
	GtkAccelGroup *accel_group;

	accel_group = gtk_accel_group_new ();

	saveFileChooser = gtk_file_chooser_new (_("Save Traced Fin As ... (*.fin)"));

	gtk_window_set_modal(GTK_WINDOW(saveFileSelection),TRUE); //***1.2

	gtk_window_set_transient_for(
			GTK_WINDOW(saveFileSelection),
			GTK_WINDOW(this->mParent));
*/
	GtkWidget *saveFCDialog;
	GtkFileFilter *filter;

	string directory; //***1.95 - set as appropriate in each case below

	switch (mSaveMode) 
	{
	case saveFullSizeModImages: //***2.02 - new mode
		saveFCDialog = gtk_file_chooser_dialog_new (
				_("Enter directory for the Full-Size Image Files (*.png)"),
				GTK_WINDOW(mParent),
				GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
		// could do this if we had version 2.8 or later of GTK+
		// gtk_file_chooser_set_do_overwrite_confirmation(
		//    GTK_FILE_CHOOSER(saveFCDialog),TRUE);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Image Files (*.png)");
		gtk_file_filter_add_pattern(filter, "*.png");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(saveFCDialog),filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "All Files (*.*)");
		gtk_file_filter_add_pattern(filter, "*.*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(saveFCDialog),filter);

		// do NOT allow multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (saveFCDialog), 
				FALSE);
		
		if (gLastDirectory[mSaveMode] == "")
		{
			// everything is relative to the current survey area
			gLastDirectory[mSaveMode] = gOptions->mCurrentSurveyArea;
			gLastDirectory[mSaveMode] += PATH_SLASH;
			gLastDirectory[mSaveMode] += "tracedFins";
		}
		gtk_file_chooser_set_current_folder (
				GTK_FILE_CHOOSER (saveFCDialog), 
				gLastDirectory[mSaveMode].c_str());
		gLastFileName[mSaveMode] = "";
		break;
	case saveMultipleFinz:
		saveFCDialog = gtk_file_chooser_dialog_new (
				_("Enter directory for the Traced Fin Files (*.finz)"),
				GTK_WINDOW(mParent),
				GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
		// could do this if we had version 2.8 or later of GTK+
		// gtk_file_chooser_set_do_overwrite_confirmation(
		//    GTK_FILE_CHOOSER(saveFCDialog),TRUE);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Finz Files (*.finz)");
		gtk_file_filter_add_pattern(filter, "*.finz");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(saveFCDialog),filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "All Files (*.*)");
		gtk_file_filter_add_pattern(filter, "*.*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(saveFCDialog),filter);

		// do NOT allow multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (saveFCDialog), 
				FALSE);
		
		if (gLastDirectory[mSaveMode] == "")
		{
			//***1.85 - everything is now relative to the current survey area
			gLastDirectory[mSaveMode] = gOptions->mCurrentSurveyArea;
			gLastDirectory[mSaveMode] += PATH_SLASH;
			gLastDirectory[mSaveMode] += "tracedFins";
		}
		gtk_file_chooser_set_current_folder (
				GTK_FILE_CHOOSER (saveFCDialog), 
				gLastDirectory[mSaveMode].c_str());
		gLastFileName[mSaveMode] = "";
		break;
	case saveFin:
		saveFCDialog = gtk_file_chooser_dialog_new (
				_("Enter filename for the Traced Fin File(*.finz)"),
				GTK_WINDOW(mParent),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
		// could do this if we had version 2.8 or later of GTK+
		// gtk_file_chooser_set_do_overwrite_confirmation(
		//    GTK_FILE_CHOOSER(saveFCDialog),TRUE);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Finz Files (*.finz)");
		gtk_file_filter_add_pattern(filter, "*.finz");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(saveFCDialog),filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "All Files (*.*)");
		gtk_file_filter_add_pattern(filter, "*.*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(saveFCDialog),filter);

		// do NOT allow multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (saveFCDialog), 
				FALSE);
		
		if (gLastDirectory[mSaveMode] == "")
		{
			//***1.85 - everything is now relative to the current survey area
			gLastDirectory[mSaveMode] = gOptions->mCurrentSurveyArea;
			gLastDirectory[mSaveMode] += PATH_SLASH;
			gLastDirectory[mSaveMode] += "tracedFins";
		}
		gtk_file_chooser_set_current_folder (
				GTK_FILE_CHOOSER (saveFCDialog), 
				gLastDirectory[mSaveMode].c_str());
		gLastFileName[mSaveMode] = "";
		break;
	case saveFinz:
		break;
	case exportDatabase:
		saveFCDialog = gtk_file_chooser_dialog_new (
				_("Enter filename for the EXPORT Database Archive(*.zip)"),
				GTK_WINDOW(mParent),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Backup Archives (*.zip)");
		gtk_file_filter_add_pattern(filter, "*.zip");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(saveFCDialog),filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "All Files (*.*)");
		gtk_file_filter_add_pattern(filter, "*.*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(saveFCDialog),filter);

		// do NOT allow multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (saveFCDialog), 
				FALSE);
		
		// return focus to "catalog" folder wih NO selected database file
		//directory = gOptions->mDarwinHome + PATH_SLASH + "backups";
		directory = gOptions->mCurrentDataPath + PATH_SLASH + "backups";
		gtk_file_chooser_set_current_folder (
				GTK_FILE_CHOOSER (saveFCDialog), 
				directory.c_str());
		gLastDirectory[mSaveMode] = directory;
		gLastFileName[mSaveMode] = "";
		break;
	//***2.22 - choose a data path for creating a new Survey Area
	case chooseDataPath:
		saveFCDialog = gtk_file_chooser_dialog_new (
				_("Choose or Create a Darwin Data Folder (darwinPhotoIdData)"),
				GTK_WINDOW(mParent),
				GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
		// could do this if we had version 2.8 or later of GTK+
		// gtk_file_chooser_set_do_overwrite_confirmation(
		//    GTK_FILE_CHOOSER(saveFCDialog),TRUE);
		
		{
			GtkWidget *msg = gtk_label_new ("Choose or Create a Darwin Data Folder (darwinPhotoIdData) above.");
			gtk_widget_show (msg);
			gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER(saveFCDialog), msg);
		}

		//filter = gtk_file_filter_new();
		//gtk_file_filter_set_name(filter, "Darwin Data Folders (DarwinPhotoIdData*)");
		//gtk_file_filter_add_pattern(filter, "DarwinPhotoIdData*");
		//gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(saveFCDialog),filter);

		// do NOT allow multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (saveFCDialog), 
				FALSE);
		
		if (gLastDirectory[mSaveMode] == "")
		{
			//***1.85 - everything is now relative to the current data path
			gLastDirectory[mSaveMode] = gOptions->mCurrentDataPath;
		}
		gtk_file_chooser_set_current_folder (
				GTK_FILE_CHOOSER (saveFCDialog), 
				gLastDirectory[mSaveMode].c_str());
		gLastFileName[mSaveMode] = "";
		break;
	}

	g_signal_connect(G_OBJECT(saveFCDialog),"current-folder-changed",
			G_CALLBACK(on_saveFileChooserDirectory_changed), (void *) this); 

	g_signal_connect(G_OBJECT(saveFCDialog),"selection-changed",
			G_CALLBACK(on_saveFileChooserFileSelections_changed), 
			(void *) this);

	return saveFCDialog;
}

gboolean on_saveFileChooser_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	SaveFileChooserDialog *dlg = (SaveFileChooserDialog *) userData;

	if (NULL == dlg)
		return FALSE;

	delete dlg;
	
	return TRUE;
}

void on_saveFileChooserButtonOK_clicked(
	GtkButton *button,
	gpointer userData)
{
	SaveFileChooserDialog *dlg = (SaveFileChooserDialog *) userData;

	if (NULL == dlg)
		return;

	switch (dlg->mSaveMode) 
	{
	case  SaveFileChooserDialog::exportDatabase :

		// set filename in mainWindow and let it handle rest of export

		dlg->mMainWin->setExportFilename(
			gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog)));

		break;
	case SaveFileChooserDialog::saveFin :
		{
		gchar *temp;

		temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));
		string fileName = temp;
		g_free(temp);

		if (fileName[fileName.length() - 1] == '/' || fileName[fileName.length() - 1] == '\\') {
			showError("Oops.. It looks like you selected a directory.\nPlease try again, and select a fin file.");
			delete dlg;
			return;
		}

		//saveFin(dlg->mFin, fileName);//SAH 2008-07-17
		if (saveFinz(dlg->mFin,fileName))
		{
			//***2.0JHS - reset SAVED fin filenames ONLY if saved 
			dlg->mFin->mFinFilename = fileName; //***2.0JHS
			if (NULL != dlg->mTraceWin)
				dlg->mTraceWin->setSavedFinFilename(fileName);
		}

		//***1.3 - do not delete trace window here
		//   allow us to return so we can proceed with match after saving fin, if desired
		}
		break;
	case SaveFileChooserDialog::saveMultipleFinz:
		{
			gchar *temp;

			temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));
			string fileName = temp;
			g_free(temp);
			
			/*
			if (fileName[fileName.length() - 1] != '/' && fileName[fileName.length() - 1] != '\\') {
				showError("Oops.. It looks like you selected a file.\nPlease try again, and select a directory.");
				delete dlg;
				return;
			}
			*/

			if(dlg->mFins == NULL)
				return;

			vector<DatabaseFin<ColorImage>*>::iterator it;
			for (it = dlg->mFins->begin(); it != dlg->mFins->end(); it++)
			{
				int duplicate = 0;
				string filename(fileName);
				filename += PATH_SLASH + (*it)->mIDCode + ".finz";
				filename = generateUniqueName(filename);
				saveFinz(*it, filename);
			}
		}
		break;
	case SaveFileChooserDialog::saveFullSizeModImages: //***2.02
		{
			gchar *temp;

			temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));
			string fileName = temp;
			g_free(temp);
			
			if(dlg->mFins == NULL)
				return;

			vector<DatabaseFin<ColorImage>*>::iterator it;
			for (it = dlg->mFins->begin(); it != dlg->mFins->end(); it++)
			{
				// get filename for modified image (including path)
				string imageFilename = (*it)->mImageFilename;

				cout << "Saving  " << imageFilename << endl;
					
				ColorImage * img = new ColorImage(imageFilename);

				string saveFilename = fileName + PATH_SLASH;                             // save path
				// strip old path and extension, prepend new path and append "_fullSize.png" 
				imageFilename = imageFilename.substr(imageFilename.rfind(PATH_SLASH)+1); // short name
				imageFilename = imageFilename.substr(0,imageFilename.rfind("."));        // root
				
				saveFilename = saveFilename + imageFilename + "_fullSize.png";  // save path and new name
	
				// save full-sized version in selected folder
				img->save(saveFilename);
				
				delete img;
			}
		}
		break;
	case SaveFileChooserDialog::chooseDataPath: //***2.22 - for multiple data path support
		{
			gchar *temp;

			temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));
			string fileName = temp;
			g_free(temp);
			
			if (fileName != "")
				gOptions->mCurrentDataPath = fileName;
		}
		break;
	default:
		break;
	}


	delete dlg;
}

void on_saveFileChooserButtonCancel_clicked(
	GtkButton *button,
	gpointer userData)
{
	SaveFileChooserDialog *dlg = (SaveFileChooserDialog *) userData;

	delete dlg;
}

//*******************************************************************
//
// void on_saveFileChooserFileSelections_changed(...)
//
void on_saveFileChooserFileSelections_changed(
	GtkWidget *widget,
	gpointer userData
	)
{
	//g_print("IN on_openFileChooserFileSelections_changed()\n");

	SaveFileChooserDialog *dlg = (SaveFileChooserDialog *) userData;

	if (NULL == dlg)
		return;
		
	gchar *fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));

	if (NULL == fileName)
	{
		// change in folder or something eles that "unselected" all files
		gLastFileName[dlg->mSaveMode] = "";
	}
	else
	{
		// strip path and set global last filename so we can return there next time
		gLastFileName[dlg->mSaveMode] = fileName;
		gLastFileName[dlg->mSaveMode] = gLastFileName[dlg->mSaveMode].substr(gLastFileName[dlg->mSaveMode].find_last_of(PATH_SLASH)+1);

		//	g_print("Last Filename : ");
		//	g_print(gLastFileName.c_str());
		//	g_print("\n");

		g_free(fileName);
	}
}


//*******************************************************************
//
// void on_saveFileChooserDirectory_changed(...)
//
void on_saveFileChooserDirectory_changed(
	GtkWidget *widget,
	gpointer userData)
{
	SaveFileChooserDialog *dlg = (SaveFileChooserDialog *) userData;
	
	//g_print("IN on_openFileChooserDirectory_changed()\n");

	if (NULL == dlg)
		return;

	gchar *folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dlg->mDialog));

	// set global last directory and blank filename so we can return there next time
	gLastDirectory[dlg->mSaveMode] = folder;
	gLastFileName[dlg->mSaveMode] = "";

	//g_print("Last Folder : ");
	//g_print(gLastDirectory.c_str());
	//g_print("\n");

	g_free(folder);
}

//*******************************************************************
//
bool SaveFileChooserDialog::run_and_respond()
{
	gint response = gtk_dialog_run (GTK_DIALOG (mDialog));

	switch (response)
	{
	case GTK_RESPONSE_CANCEL :
		on_saveFileChooserButtonCancel_clicked(NULL,this);
		break;
	case GTK_RESPONSE_ACCEPT :
		on_saveFileChooserButtonOK_clicked(NULL,this);
		return true;
		break;
	default :
		// no other action required
		break;
	}
	return false;
}


