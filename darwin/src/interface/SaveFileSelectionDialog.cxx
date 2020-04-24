//*******************************************************************
//   file: SaveFileSelectionDialog.cxx
//
// author: Adam Russell?
//
//   mods: 
//
// This isused to open images load match queues, and view match results.
// It has largely been replaced by a GtkFileCooserDialog in versions
// 1.8 and later - JHS
//
//*******************************************************************

#include <gdk/gdkkeysyms.h>
#include "../support.h"
#include "SaveFileSelectionDialog.h"
#include "ErrorDialog.h"

#include "ImageViewDialog.h" // for test of PNG file I/O

#ifdef WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "/"
#endif

gboolean on_saveFileSelection_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

void on_saveButtonOK_clicked(
		GtkButton *button,
		gpointer userData);

void on_saveButtonCancel_clicked(
		GtkButton *button,
		gpointer userData);

using namespace std;

static int gNumReferences = 0;
static string gLastDirectory = "";   // disk & path last in use

int getNumSaveFileSelectionDialogReferences()
{
	return gNumReferences;
}

SaveFileSelectionDialog::SaveFileSelectionDialog(
	DatabaseFin<ColorImage> *dbFin,
	TraceWindow *traceWin,
	GtkWidget *parent
)
	:	mFin(dbFin),
		mTraceWin(traceWin), //***1.2 - so we can delete TraceWindow Object when done
		mParent(parent)      //***1.2 - the parent GTK Window Widget
{
	mDialog = createSaveFileSelection(); //***1.1 - this must follow intilization of mFin

	gNumReferences++;
}

SaveFileSelectionDialog::~SaveFileSelectionDialog()
{
	gtk_widget_destroy(mDialog);

	//delete mFin; // just a pointer to the TraceWindow::mFin, do NOT delete here
	
	gNumReferences--;
}

void SaveFileSelectionDialog::show()
{
	gtk_widget_show(mDialog);
}

void SaveFileSelectionDialog::save(string fileName)
{
	try {

		mFin->save(fileName);
	
	} catch (Error e) {
		showError(e.errorString());
	}
}

GtkWidget* SaveFileSelectionDialog::createSaveFileSelection (void)
{
	GtkWidget *saveFileSelection;
	GtkWidget *saveButtonOK;
	GtkWidget *saveButtonCancel;
	GtkAccelGroup *accel_group;

	accel_group = gtk_accel_group_new ();

	saveFileSelection = gtk_file_selection_new (_("Save Traced Fin As ... (*.fin)"));

	gtk_window_set_modal(GTK_WINDOW(saveFileSelection),TRUE); //***1.2

	gtk_window_set_transient_for(
			GTK_WINDOW(saveFileSelection),
			GTK_WINDOW(this->mParent));

	//***1.1 - have file viewer open in tracedFins folder
	if (gLastDirectory == "")
	{
		// set path to %DARWINHOME% for first file open
		//gLastDirectory = getenv("DARWINHOME");
		//***1.85 - everything is now relative to the current survey area
		gLastDirectory = gOptions->mCurrentSurveyArea;
		gLastDirectory += PATH_SLASH;
		gLastDirectory += "tracedFins";
		gLastDirectory += PATH_SLASH;
	}

	//***1.1 - go to correct folder location and suggest initial filename based on fin ID
  	gtk_file_selection_set_filename(
			GTK_FILE_SELECTION(saveFileSelection),
			gLastDirectory.c_str());
	gtk_file_selection_complete(
			GTK_FILE_SELECTION (saveFileSelection), 
			(this->mFin->mIDCode + ".fin").c_str());

	//***1.1 - prevent user from changing folder location
	gtk_widget_hide(GTK_FILE_SELECTION (saveFileSelection)->dir_list);
	gtk_widget_hide(GTK_FILE_SELECTION (saveFileSelection)->history_pulldown);
	gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (saveFileSelection));

	// prevent multiple file selections
	gtk_file_selection_set_select_multiple (GTK_FILE_SELECTION (saveFileSelection), FALSE);

  gtk_object_set_data (GTK_OBJECT (saveFileSelection), "saveFileSelection", saveFileSelection);
  gtk_container_set_border_width (GTK_CONTAINER (saveFileSelection), 10);
  GTK_WINDOW (saveFileSelection)->type = WINDOW_DIALOG;
  gtk_window_set_wmclass(GTK_WINDOW(saveFileSelection), "darwin_save", "DARWIN");

  saveButtonOK = GTK_FILE_SELECTION (saveFileSelection)->ok_button;
  gtk_object_set_data (GTK_OBJECT (saveFileSelection), "saveButtonOK", saveButtonOK);
  gtk_widget_show (saveButtonOK);
  GTK_WIDGET_SET_FLAGS (saveButtonOK, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (saveButtonOK, "clicked", accel_group,
                              GDK_O, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  saveButtonCancel = GTK_FILE_SELECTION (saveFileSelection)->cancel_button;
  gtk_object_set_data (GTK_OBJECT (saveFileSelection), "saveButtonCancel", saveButtonCancel);
  gtk_widget_show (saveButtonCancel);
  GTK_WIDGET_SET_FLAGS (saveButtonCancel, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (saveButtonCancel, "clicked", accel_group,
                              GDK_C, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (saveButtonCancel, "clicked", accel_group,
                              GDK_Escape, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  gtk_signal_connect (GTK_OBJECT (saveFileSelection), "delete_event",
                      GTK_SIGNAL_FUNC (on_saveFileSelection_delete_event),
                      (void *) this);
  gtk_signal_connect (GTK_OBJECT (saveButtonOK), "clicked",
                      GTK_SIGNAL_FUNC (on_saveButtonOK_clicked),
                      (void *) this);
  gtk_signal_connect (GTK_OBJECT (saveButtonCancel), "clicked",
                      GTK_SIGNAL_FUNC (on_saveButtonCancel_clicked),
                      (void *) this);

  gtk_widget_grab_default (saveButtonOK);
  gtk_window_add_accel_group (GTK_WINDOW (saveFileSelection), accel_group);

  return saveFileSelection;
}

gboolean on_saveFileSelection_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	SaveFileSelectionDialog *dlg = (SaveFileSelectionDialog *) userData;

	if (NULL == dlg)
		return FALSE;

	delete dlg;
	
	return TRUE;
}

void on_saveButtonOK_clicked(
	GtkButton *button,
	gpointer userData)
{
	SaveFileSelectionDialog *dlg = (SaveFileSelectionDialog *) userData;

	if (NULL == dlg)
		return;

	string fileName = gtk_file_selection_get_filename(GTK_FILE_SELECTION(dlg->mDialog));

	if (fileName[fileName.length() - 1] == '/' || fileName[fileName.length() - 1] == '\\') {
		showError("Oops.. It looks like you selected a directory.\nPlease try again, and select a fin file.");
		delete dlg;
		return;
	}

	//***1.4 - enforce ".fin" extension
	int posit = fileName.rfind(".fin");
	int shouldBe = (fileName.length() - 4);
	if (posit != shouldBe)
		fileName += ".fin";

	//***1.1 - saved fin traces now go in a standard location - DARWINHOME/tracedFins

	// this should be more flexible and allow user to place files wherever desired,
	// but for now they all go here

	// copy unknown image to tracedFins folder and use short filename in database

	string shortFilename = dlg->mFin->mImageFilename;
	int pos = shortFilename.find_last_of(PATH_SLASH);
	if (pos >= 0)
	{
		shortFilename = shortFilename.substr(pos+1);
	}

	//printf("copying \"%s\" to tracedFins\n",shortFilename.c_str());

	//string path = getenv("DARWINHOME");
	//***1.85 - everything is now relative to the current survey area
	string path = gOptions->mCurrentSurveyArea;
	path += PATH_SLASH;
	path += "tracedFins";
	path += PATH_SLASH;
	string copyFilename = path + shortFilename;

	// copy image over into tracedFins folder

#ifdef WIN32
	string command = "copy \"";
#else
	string command = "cp \"";
#endif
	command += dlg->mFin->mImageFilename;
	command += "\" \"";
	command += copyFilename;
	command += "\"";

#ifdef DEBUG
	printf("copy command: \"%s\"",command.c_str());
#endif

	if (copyFilename != dlg->mFin->mImageFilename) //***1.8 - prevent copy onto self
	{
		printf("copying \"%s\" to tracedFins\n",shortFilename.c_str()); //***1.8 - moved here
		system(command.c_str());
				
		// ***1.8 - save path & name of copy of original image file
		dlg->mFin->mOriginalImageFilename = copyFilename; // ***1.8
	}

	//***1.5 - save modified image alongside original
	if (NULL == dlg->mFin->mModifiedFinImage)
		throw Error("Attempt to save Trace without modified image");

	// create filename
	//int pos = copyFilename.find_last_of('.');
	//copyFilename = copyFilename.substr(0,pos);
	//copyFilename += "_wDarwinMods.ppm";
	// base this on FIN filename now
	pos = fileName.find_last_of(PATH_SLASH);
	copyFilename = path + fileName.substr(pos+1);
	pos = copyFilename.rfind(".fin");
	copyFilename = copyFilename.substr(0,pos);
	//copyFilename += "_wDarwinMods.ppm";
	copyFilename += "_wDarwinMods.png"; //***1.8 - new file format
		
	dlg->mFin->mModifiedFinImage->save_wMods( //***1.8 - new save modified image call
		copyFilename,    // the filename of the modified image
		shortFilename,   // the filename of the original image
		dlg->mFin->mImageMods); // the list of image modifications
	
	// set image filename to path & filename of modified image so that name is 
	// saved as part of the DatabaseFin record in the file
	dlg->mFin->mImageFilename = copyFilename; //***1.8 - save this filename now

	/*
	//***1.8 - write a PNG file as a test
	dlg->mFin->mModifiedFinImage->save("testFinImage.png"); 
	//***1.8 - NOW read it back in
	ColorImage *temp = new ColorImage("testFinImage.png");
	ImageViewDialog *seeIt = new ImageViewDialog("PNG file",temp);
	seeIt->show();

	delete temp;
	*/

	// DatabaseFin::save()  shortens the mImageFilename, so this call must
	// precede the setting of the mImagefilename to the path+filename
	// needed in the TraceWindow code if Add to Database is done after a
	// save of the fin trace
	dlg->save(fileName); 

	// set image filename to path & filename of modified image so that name is 
	// saved as part of the DatabaseFin record in the file
	dlg->mFin->mImageFilename = copyFilename; //***1.8 - save this filename now


	dlg->mTraceWin->setSavedFinFilename(fileName);

	//***1.3 - do not delete trace window here
	//   allow us to return so we can proceed with match after saving fin, if desired
	//delete dlg->mTraceWin;

	delete dlg;
}

void on_saveButtonCancel_clicked(
	GtkButton *button,
	gpointer userData)
{
	SaveFileSelectionDialog *dlg = (SaveFileSelectionDialog *) userData;

	delete dlg;
}
