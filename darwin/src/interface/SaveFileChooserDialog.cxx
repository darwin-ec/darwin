//*******************************************************************
//   file: SaveFileChooserDialog.cxx
//
// author: John H Stewman
//
//   mods: 
//
//   date: 7/16/2008
//
// This is used to save DatabaseFins , export catalogs, and 
// choose locations for other exports (finz and data).
// This replaces the SaveFileSelectionDialog class - JHS
//
//*******************************************************************

#include <gdk/gdkkeysyms.h>
#include "../support.h"
#include "SaveFileChooserDialog.h"
#include "ErrorDialog.h"
#include "../CatalogSupport.h"

#ifdef WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "/"
#endif

using namespace std;

static int gNumReferences = 0;
//static string gLastDirectory = "";   // disk & path last in use

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
	int saveMode
)
	:	mDatabase(db),
		mFin(dbFin),
		mMainWin(mainWin),
		mTraceWin(traceWin), //***1.2 - so we can delete TraceWindow Object when done
		mOptions(o),
		mParent(parent),      //***1.2 - the parent GTK Window Widget
		mSaveMode(saveMode)
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
	case saveFin:
		saveFCDialog = gtk_file_chooser_dialog_new (
				_("Enter filename for the Traced Fin File(*.fin)"),
				GTK_WINDOW(mParent),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
		// could do this if we had version 2.8 or later of GTK+
		// gtk_file_chooser_set_do_overwrite_confirmation(
		//    GTK_FILE_CHOOSER(saveFCDialog),TRUE);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Fin Files (*.fin)");
		gtk_file_filter_add_pattern(filter, "*.fin");
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
		directory = gOptions->mDarwinHome + PATH_SLASH + "backups";
		gtk_file_chooser_set_current_folder (
				GTK_FILE_CHOOSER (saveFCDialog), 
				directory.c_str());
		gLastDirectory[mSaveMode] = directory;
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

		saveFin(dlg->mFin, fileName);

		if (NULL != dlg->mTraceWin)
			dlg->mTraceWin->setSavedFinFilename(fileName);

		//***1.3 - do not delete trace window here
		//   allow us to return so we can proceed with match after saving fin, if desired
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
void SaveFileChooserDialog::run_and_respond()
{
	gint response = gtk_dialog_run (GTK_DIALOG (mDialog));

	switch (response)
	{
	case GTK_RESPONSE_CANCEL :
		on_saveFileChooserButtonCancel_clicked(NULL,this);
		break;
	case GTK_RESPONSE_ACCEPT :
		on_saveFileChooserButtonOK_clicked(NULL,this);
		break;
	default :
		// no other action required
		break;
	}
}


