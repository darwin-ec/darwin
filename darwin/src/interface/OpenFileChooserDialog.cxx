//*******************************************************************
//   file: OpenFileChooserDialog.cxx
//
// author: J H Stewman 7/13/2006)
//         -- opens Images, Fin Traces, and Databases
//
//*******************************************************************

#include "../support.h"
#include "OpenFileChooserDialog.h"

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include <io.h>     //***1.982 - _findFirst()
#include <direct.h> //***1.982 - _mkdir()

#include "../image_processing/transform.h"
#include "MainWindow.h"
#include "ErrorDialog.h"
#include "TraceWindow.h"

using namespace std;

static const int IMAGE_WIDTH = 300;
static const int IMAGE_HEIGHT = 300;

static int gNumReferences = 0;

static bool gShowingPreviews = true; //***051

#ifdef WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "/"
#endif

static string gLastDirectory = "";   // disk & path last in use
static string gLastFileName = "";    // simple name of last file "touched" -- if any
static string gLastFolderName = "";  // simple name of folder last "touched" -- if any

static gchar  *gLastTreePathStr = NULL; // GtkTree path (ex: "10:3:5") for same -- must free with g_free()
static GtkTreePath *gLastTreePath;


//*******************************************************************
//
int getNumOpenFileChooserDialogReferences()
{
	return gNumReferences;
}


//*******************************************************************
//
OpenFileChooserDialog::OpenFileChooserDialog(
		Database<ColorImage> *db,
		MainWindow *m,
		Options *o,
		int openMode
)
	: mOpenMode(openMode),
	  mDatabase(db),
	  mImage(NULL),
	  mMainWin(m),
	  mOptions(o),
	  mBlackImage(true),
	  mShowPreview(gShowingPreviews), //***051
	  mNumTextChars(0),
	  mPreview(NULL) //***1.95
{
	mDialog = createFileChooserDialog();  // call to this must follow setting of mOpenMode

	gNumReferences++;
}


//*******************************************************************
//
OpenFileChooserDialog::~OpenFileChooserDialog()
{
	if (NULL != mPreview) //***1.95
		gtk_widget_destroy(mPreview);

	if (NULL != mDialog)
		gtk_widget_destroy(mDialog);

	delete mImage;

	gNumReferences--;
}


//*******************************************************************
//
void OpenFileChooserDialog::refreshImage()
{
	on_fileChooserDrawingArea_expose_event(NULL, NULL, (void *) this);
}


//*******************************************************************
//
GtkWidget* OpenFileChooserDialog::createFileChooserDialog()
{
	GtkWidget *openFCDialog;
	GtkFileFilter *filter;

	string directory; //***1.95 - set as appropriate in each case below

	switch (mOpenMode) 
	{
	case openFinImage: //***1.95 - no longer use GtkFileSelectionDialog
  		openFCDialog = gtk_file_chooser_dialog_new (
				_("Select a Fin Image (JPEG, BMP, PNG, PNM)"),
				GTK_WINDOW(this->mMainWin->getWindow()),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);

		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Image Files (*.jpg *.bmp *.png *.ppm *.pgm)");
		gtk_file_filter_add_pattern(filter, "*.jpg");
		gtk_file_filter_add_pattern(filter, "*.bmp");
		gtk_file_filter_add_pattern(filter, "*.png");
		gtk_file_filter_add_pattern(filter, "*.ppm");
		gtk_file_filter_add_pattern(filter, "*.pgm");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(openFCDialog),filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "All Files (*.*)");
		gtk_file_filter_add_pattern(filter, "*.*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(openFCDialog),filter);

		// do NOT allow multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (openFCDialog), 
				FALSE);
		
		directory = gOptions->mDarwinHome;

		if ("" != gLastDirectory)
		{
			if ("" != gLastFileName)
				// go back to last selected file
				gtk_file_chooser_set_filename (
						GTK_FILE_CHOOSER (openFCDialog), 
						((gLastDirectory+PATH_SLASH)+gLastFileName).c_str());
			else
				// go to last selected folder
				gtk_file_chooser_set_current_folder (				
						GTK_FILE_CHOOSER (openFCDialog), 
						gLastDirectory.c_str());
				// JHS - for some bizzare reason this results in the
				// path tool showing the path to the DEFAULT runtime location
				// of the software (e.g, e:\darwin\darwin-1.95\msvc)
				// whenever the last folder opened was in the path to this
				// default runtime location and we left it without selecting 
				// a file.  It is as if there is an error selecting
				// a default file, but I cannot figure out why.
		}
		else
		{
			// first time, set focus to %DARWINHOME% folder wih NO selected file
			gtk_file_chooser_set_current_folder (
					GTK_FILE_CHOOSER (openFCDialog), 
					directory.c_str());
			gLastDirectory = directory;
			gLastFileName = "";
		}

		//***1.95 - test of image preview widget
		mPreview = gtk_image_new ();
		gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER(openFCDialog), mPreview);
		g_signal_connect (GTK_FILE_CHOOSER(openFCDialog), "update-preview",
		    G_CALLBACK (on_fileChooserPreview_update), mPreview);

		//mPreview = gtk_image_new_from_stock (GTK_STOCK_MISSING_IMAGE,GTK_ICON_SIZE_DIALOG);
		//gtk_file_chooser_set_preview_widget_active (GTK_FILE_CHOOSER(openFCDialog), TRUE);
		break;
	case openFinTrace:
  		openFCDialog = gtk_file_chooser_dialog_new (
				_("Select a Traced Fin (*.fin)"),
				GTK_WINDOW(this->mMainWin->getWindow()),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);

		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Fin Files (*.fin)");
		gtk_file_filter_add_pattern(filter, "*.fin");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(openFCDialog),filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "All Files (*.*)");
		gtk_file_filter_add_pattern(filter, "*.*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(openFCDialog),filter);

		// allow multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (openFCDialog), 
				TRUE);
		
		// currently we force focus back on the "tracedFins" folder

		directory = gOptions->mCurrentSurveyArea + PATH_SLASH + "tracedFins";

		if (directory == gLastDirectory)
		{
			// go back to last selected file
			gtk_file_chooser_set_filename (
					GTK_FILE_CHOOSER (openFCDialog), 
					((gLastDirectory+PATH_SLASH)+gLastFileName).c_str());
		}
		else
		{
			// return focus to "tracedFins" folder wih NO selected file
			gtk_file_chooser_set_current_folder (
					GTK_FILE_CHOOSER (openFCDialog), 
					directory.c_str());
			gLastDirectory = directory;
			gLastFileName = "";
		}
		break;
	case openDatabase : //***1.85
		openFCDialog = gtk_file_chooser_dialog_new (
				_("Select a Database (*.db)"),
				GTK_WINDOW(this->mMainWin->getWindow()),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Database Files (*.db)");
		gtk_file_filter_add_pattern(filter, "*.db");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(openFCDialog),filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "All Files (*.*)");
		gtk_file_filter_add_pattern(filter, "*.*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(openFCDialog),filter);

		directory = gOptions->mCurrentSurveyArea + PATH_SLASH + "catalog";

		// do NOT allow multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (openFCDialog), 
				FALSE);
		
		// return focus to "catalog" folder with NO selected database file
		gtk_file_chooser_set_current_folder (
				GTK_FILE_CHOOSER (openFCDialog), 
				directory.c_str());
		gLastDirectory = directory;
		gLastFileName = "";

		break;
	case importDatabase :  //***1.85
	case restoreDatabase : //***1.85
		openFCDialog = gtk_file_chooser_dialog_new (
				_("Select a Database Backup Archive(*.zip)"),
				GTK_WINDOW(this->mMainWin->getWindow()),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Backup Archives (*.zip)");
		gtk_file_filter_add_pattern(filter, "*.zip");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(openFCDialog),filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "All Files (*.*)");
		gtk_file_filter_add_pattern(filter, "*.*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(openFCDialog),filter);

		// do NOT allow multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (openFCDialog), 
				FALSE);
		
		// return focus to "catalog" folder wih NO selected database file
		directory = gOptions->mDarwinHome + PATH_SLASH + "backups";
		gtk_file_chooser_set_current_folder (
				GTK_FILE_CHOOSER (openFCDialog), 
				directory.c_str());
		gLastDirectory = directory;
		gLastFileName = "";

		break;
	case exportDatabase : //***1.85
		openFCDialog = gtk_file_chooser_dialog_new (
				_("Enter filename for the EXPORT Database Archive(*.zip)"),
				GTK_WINDOW(this->mMainWin->getWindow()),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Backup Archives (*.zip)");
		gtk_file_filter_add_pattern(filter, "*.zip");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(openFCDialog),filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "All Files (*.*)");
		gtk_file_filter_add_pattern(filter, "*.*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(openFCDialog),filter);

		// do NOT allow multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (openFCDialog), 
				FALSE);
		
		// return focus to "catalog" folder wih NO selected database file
		directory = gOptions->mDarwinHome + PATH_SLASH + "backups";
		gtk_file_chooser_set_current_folder (
				GTK_FILE_CHOOSER (openFCDialog), 
				directory.c_str());
		gLastDirectory = directory;
		gLastFileName = "";

		break;
	case exportDataFields : //***1.9
		openFCDialog = gtk_file_chooser_dialog_new (
				_("Enter filename for the <tab> separated data file (*.txt)"),
				GTK_WINDOW(this->mMainWin->getWindow()),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "Data Field Files (*.txt)");
		gtk_file_filter_add_pattern(filter, "*.txt");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(openFCDialog),filter);
		filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, "All Files (*.*)");
		gtk_file_filter_add_pattern(filter, "*.*");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(openFCDialog),filter);

		//directory = gOptions->mDarwinHome + PATH_SLASH + "backups";
		directory = gOptions->mCurrentSurveyArea + PATH_SLASH + "sightings"; //***1.96a

		// do NOT allow multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (openFCDialog), 
				FALSE);
		
		// return focus to "catalog" folder wih NO selected database file
		gtk_file_chooser_set_current_folder (
				GTK_FILE_CHOOSER (openFCDialog), 
				directory.c_str());
		gLastDirectory = directory;
		gLastFileName = "";

		break;
/*
	case LOAD_QUEUE:
		matchingQueueFileChooserDialog = gtk_file_chooser_dialog_new (
				_("Load Matching Queue (*.que)"),
				GTK_WINDOW(this->mDialog),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
		directory += "matchQueues";
		// prevent multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (matchingQueueFileChooserDialog), 
				FALSE);

		// currently we force focus back on the "matchQueues" folder

		if (directory == gLastDirectory)
		{
			// go back to last selected file
			gtk_file_chooser_set_filename (
					GTK_FILE_CHOOSER (matchingQueueFileChooserDialog), 
					((gLastDirectory+PATH_SLASH)+gLastFileName).c_str());
		}
		else
		{
			// return focus to "matchQueues" folder wih NO selected file
			gtk_file_chooser_set_current_folder (
					GTK_FILE_CHOOSER (matchingQueueFileChooserDialog), 
					directory.c_str());
			gLastDirectory = directory;
			gLastFileName = "";
		}
		break;

	case SAVE_QUEUE:
		matchingQueueFileChooserDialog = gtk_file_chooser_dialog_new (
				_("Save Matching Queue As ... (*.que)"),
				GTK_WINDOW(this->mDialog),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
		directory += "matchQueues";
		gtk_file_chooser_set_current_folder (
				GTK_FILE_CHOOSER (matchingQueueFileChooserDialog), 
				directory.c_str());
		gtk_file_chooser_set_current_name (
				GTK_FILE_CHOOSER (matchingQueueFileChooserDialog), 
				"Untitled.que");
		// prevent multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (matchingQueueFileChooserDialog), 
				FALSE);
		break;

	case VIEW_RESULTS:
		matchingQueueFileChooserDialog = gtk_file_chooser_dialog_new (
				_("Select Results file ..."),
				GTK_WINDOW(this->mDialog),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);
		directory += "matchQResults";
		//directory += PATH_SLASH;
		// prevent multiple file selections
		gtk_file_chooser_set_select_multiple (
				GTK_FILE_CHOOSER (matchingQueueFileChooserDialog), 
				FALSE);

		// currently we force focus back on the "matchQResults" folder

		if (directory == gLastDirectory)
		{
			// go back to last selected file
			gtk_file_chooser_set_filename (
					GTK_FILE_CHOOSER (matchingQueueFileChooserDialog), 
					((gLastDirectory+PATH_SLASH)+gLastFileName).c_str());
		}
		else
		{
			// return focus to "matchQResults" folder wih NO selected file
			gtk_file_chooser_set_current_folder (
					GTK_FILE_CHOOSER (matchingQueueFileChooserDialog), 
					directory.c_str());
			gLastDirectory = directory;
			gLastFileName = "";
		}
		break;
*/
	default:
		// there is no default action
		//matchingQueueFileChooserDialog = gtk_file_Chooser_new (_("Select a File"));
		break;

	}

	g_signal_connect(G_OBJECT(openFCDialog),"current-folder-changed",
			G_CALLBACK(on_openFileChooserDirectory_changed), (void *) this); 

	g_signal_connect(G_OBJECT(openFCDialog),"selection-changed",
			G_CALLBACK(on_openFileChooserFileSelections_changed), 
			(void *) this);

	return openFCDialog;




	//------------------------------- old code
/*
	GtkWidget *fileChooserDialog;
	GtkWidget *fileHBox;
	GtkWidget *filePreviewCheckButton;
	GtkWidget *fileVBox;
	GtkWidget *fileFrame;
	GtkWidget *frame;
	GtkWidget *fileScrolledWindow;
	GtkWidget *fileSelection;

	  fileSelectionDialog = gtk_window_new (WINDOW_DIALOG);
	gtk_object_set_data (GTK_OBJECT (fileSelectionDialog), "fileSelectionDialog", fileSelectionDialog);
	gtk_window_set_title (GTK_WINDOW (fileSelectionDialog), _("Select a File"));
	gtk_window_set_policy (GTK_WINDOW (fileSelectionDialog), TRUE, TRUE, FALSE);
	gtk_window_set_wmclass(GTK_WINDOW(fileSelectionDialog), "darwin_open", "DARWIN");

	fileHBox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (fileHBox);
	gtk_container_add (GTK_CONTAINER (fileSelectionDialog), fileHBox);

	fileSelection = gtk_file_selection_new(_("Select a File"));

	if (gLastDirectory == "")
	{
		// set path to %DARWINHOME% for first file open
		gLastDirectory = getenv("DARWINHOME");
		gLastDirectory += PATH_SLASH;
	}


	gtk_file_selection_set_filename(
			GTK_FILE_SELECTION(fileSelection),
			(gLastDirectory+gLastFileName).c_str());

	// the file_list is a GtkTreeView rather than a GtkCList.  This code sets
	// the focus on the correct row and scrolls list down appropriately

	if (gLastTreePathStr)
	{
		gtk_tree_view_scroll_to_cell(
			GTK_TREE_VIEW(GTK_FILE_SELECTION(fileSelection)->file_list),
			gtk_tree_path_new_from_string(gLastTreePathStr), NULL,
			TRUE,
			0.5,0.5);
		gtk_tree_selection_select_path(
			gtk_tree_view_get_selection(GTK_TREE_VIEW(GTK_FILE_SELECTION(fileSelection)->file_list)),
			gtk_tree_path_new_from_string(gLastTreePathStr));
	}

	gtk_widget_reparent(GTK_FILE_SELECTION(fileSelection)->main_vbox, fileHBox);
	gtk_widget_show(GTK_FILE_SELECTION(fileSelection)->main_vbox);

	 gtk_object_set_data(GTK_OBJECT(fileSelectionDialog), "filesel", fileSelection);

	filePreviewCheckButton = gtk_check_button_new_with_label (_("Show Preview"));
	gtk_widget_show(filePreviewCheckButton);
	gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(fileSelection)->main_vbox), filePreviewCheckButton, FALSE, FALSE, 0);
	if (gShowingPreviews)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (filePreviewCheckButton), TRUE); //***051
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (filePreviewCheckButton), FALSE); //***051

	fileVBox = gtk_vbox_new(FALSE, 5);
	gtk_widget_show(fileVBox);

	frame = gtk_frame_new(NULL);
	gtk_widget_show(frame);
	gtk_container_add(GTK_CONTAINER(frame), fileVBox);
  
	gtk_box_pack_start(GTK_BOX (fileHBox), frame, TRUE, TRUE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(fileVBox), 5);

	fileFrame = gtk_frame_new (_("Preview"));
	gtk_widget_show (fileFrame);
	gtk_box_pack_start (GTK_BOX (fileVBox), fileFrame, TRUE, TRUE, 0);

	mDrawingArea = gtk_drawing_area_new ();
	gtk_widget_show (mDrawingArea);
	gtk_container_add (GTK_CONTAINER (fileFrame), mDrawingArea);

	gtk_drawing_area_size(
		  GTK_DRAWING_AREA(mDrawingArea),
		  IMAGE_WIDTH,
		  IMAGE_HEIGHT);

	fileScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (fileScrolledWindow);
	gtk_box_pack_start (GTK_BOX (fileVBox), fileScrolledWindow, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (fileScrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	mText = gtk_text_new(NULL, NULL);
	gtk_widget_show(mText);
	gtk_container_add (GTK_CONTAINER (fileScrolledWindow), mText);

	gtk_signal_connect (GTK_OBJECT (fileSelectionDialog), "delete_event",
                      GTK_SIGNAL_FUNC (on_fileSelectionDialog_delete_event),
                      (void *) this);
	gtk_signal_connect (GTK_OBJECT (filePreviewCheckButton), "toggled",
                      GTK_SIGNAL_FUNC (on_filePreviewCheckButton_toggled),
                      (void *) this);
	gtk_signal_connect (GTK_OBJECT (mDrawingArea), "expose_event",
                      GTK_SIGNAL_FUNC (on_fileDrawingArea_expose_event),
                      (void *) this);

	gtk_signal_connect(
		  GTK_OBJECT(GTK_FILE_SELECTION(fileSelection)->ok_button),
		  "clicked",
		  GTK_SIGNAL_FUNC(on_fileButtonOK_clicked),
		  (void *) this);
	gtk_signal_connect(
		  GTK_OBJECT(GTK_FILE_SELECTION(fileSelection)->cancel_button),
		  "clicked",
		  GTK_SIGNAL_FUNC(on_fileButtonCancel_clicked),
		  (void *) this);
	gtk_signal_connect(
		  GTK_OBJECT(GTK_FILE_SELECTION(fileSelection)->selection_entry),
		  "changed",
		  GTK_SIGNAL_FUNC(on_fileSelectionEntry_changed),
		  (void *) this);

	// callbacks for the file_list and dir_list GtkTreeViews in the FileSelection

	GtkTreeSelection *select;
	select = gtk_tree_view_get_selection(
			GTK_TREE_VIEW(GTK_FILE_SELECTION(fileSelection)->dir_list));
	g_signal_connect(G_OBJECT(select),"changed",
			G_CALLBACK(on_directoryList_changed), (void *) this);
	select = gtk_tree_view_get_selection(
			GTK_TREE_VIEW(GTK_FILE_SELECTION(fileSelection)->file_list));
	g_signal_connect(G_OBJECT(select),"changed",
			G_CALLBACK(on_fileListCell_changed), (void *) this);

	return fileSelectionDialog;
*/
}


//*******************************************************************
//
gboolean on_fileChooserDialog_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	OpenFileChooserDialog *dlg = (OpenFileChooserDialog *) userData;
	
	if (NULL == dlg)
		return FALSE;

	dlg->mMainWin->displayStatusMessage("Open canceled.");

	delete dlg;

	return TRUE;
}


//*******************************************************************
//
// this function is NOT used in versions prior to 1.95
//
void on_fileChooserPreviewCheckButton_toggled(
	GtkToggleButton *togglebutton,
	gpointer userData)
{
	OpenFileChooserDialog *dlg = (OpenFileChooserDialog *) userData;
	
	if (NULL == dlg)
		return;
/*
    // this section will be used ONLY if we convert the OpenImage
	// File SELECTION Dialog to a File CHOOSER Dialog -- JHS

	if (togglebutton->active)
	{
		dlg->mShowPreview = true;
		gShowingPreviews = true; //***051

		// nothing more to do, if no file needs displaying
		if ("" == gLastFileName)
			return;

		try
		{
			ColorImage *newImage = new ColorImage(gLastDirectory+gLastFileName);

			delete dlg->mImage;
			dlg->mImage = resizeWithBorder(newImage, IMAGE_HEIGHT, IMAGE_WIDTH);
			dlg->mBlackImage = false;
			dlg->refreshImage();

			gtk_text_freeze(GTK_TEXT(dlg->mText));
			gtk_text_backward_delete(GTK_TEXT(dlg->mText), dlg->mNumTextChars);
			char info[500];
			sprintf(
					info,
					_("Dimensions: %d x %d"),
					newImage->getNumCols(),
					newImage->getNumRows());

			delete newImage;
		
			gtk_text_insert(GTK_TEXT (dlg->mText), NULL, NULL, NULL,
					info, strlen(info));
			dlg->mNumTextChars = strlen(info);
			gtk_text_thaw(GTK_TEXT(dlg->mText));

		} catch (Error &e) {

			if (!dlg->mBlackImage) {
				delete dlg->mImage;
				dlg->mImage = new ColorImage(IMAGE_HEIGHT, IMAGE_WIDTH);
				dlg->mBlackImage = true;
				dlg->refreshImage();
			}
			if (dlg->mNumTextChars > 0) {
				gtk_text_freeze(GTK_TEXT(dlg->mText));
				gtk_text_backward_delete(GTK_TEXT(dlg->mText), dlg->mNumTextChars);
				dlg->mNumTextChars = 0;
				gtk_text_thaw(GTK_TEXT(dlg->mText));
			}
		}
	}
	else
	{
		dlg->mShowPreview = false;
		gShowingPreviews = false; //***051
		
		// show "BLACK" image in preview window
		if (dlg->mBlackImage)
			return;
		delete dlg->mImage;
		dlg->mImage = new ColorImage(IMAGE_HEIGHT, IMAGE_WIDTH);
		dlg->mBlackImage = true;
		dlg->refreshImage();
		// erase any previously displayed image dimensions
		if (dlg->mNumTextChars > 0) {
			gtk_text_freeze(GTK_TEXT(dlg->mText));
			gtk_text_backward_delete(GTK_TEXT(dlg->mText), dlg->mNumTextChars);
			dlg->mNumTextChars = 0;
			gtk_text_thaw(GTK_TEXT(dlg->mText));
		}
	}
*/
}


//*******************************************************************
//
gboolean on_fileChooserDrawingArea_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData)
{
	OpenFileChooserDialog *dlg = (OpenFileChooserDialog *) userData;
	
	if (NULL == dlg)
		return FALSE;

	if (NULL == dlg->mImage)
		return FALSE;

	gdk_draw_rgb_image(
		dlg->mDrawingArea->window,
		dlg->mDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
		0, 0,
		dlg->mImage->getNumCols(),
		dlg->mImage->getNumRows(),
		GDK_RGB_DITHER_NONE,
		(guchar*)dlg->mImage->getData(),
		dlg->mImage->getNumCols() * dlg->mImage->bytesPerPixel());

	return TRUE;
}

//*******************************************************************
//
gboolean on_fileChooserPreview_update(
	GtkWidget *widget,
	gpointer userData)
{
	OpenFileChooserDialog *dlg = (OpenFileChooserDialog *) userData;
	
	if (NULL == dlg)
		return FALSE;

	GtkWidget *preview;
	char *filename;
	GdkPixbuf *pixbuf = NULL;
	gboolean have_preview;

	preview = GTK_WIDGET (userData);
	filename = gtk_file_chooser_get_preview_filename (GTK_FILE_CHOOSER(widget));

	if (NULL == filename)
	{
		// must be a folder/directory selection - no preview
		gtk_file_chooser_set_preview_widget_active (GTK_FILE_CHOOSER(widget), FALSE);
		return FALSE;
	}
	else
	{
		string ext = filename;
		ext = ext.substr(ext.rfind(".")+1);
		//g_print("%s\n",ext.c_str());
		for (int i = 0; i < ext.length(); i++)
			ext[i] = tolower(ext[i]);
		if (("jpg" != ext) && ("bmp" != ext) && 
			("pgm" != ext) && ("ppm" != ext) && 
			("png" != ext) && ("gif" != ext))
		{
			// not a supported image file type
			gtk_file_chooser_set_preview_widget_active (GTK_FILE_CHOOSER(widget), FALSE);
			return FALSE;
		}
	}
		
	pixbuf = gdk_pixbuf_new_from_file_at_size (filename, 128, 128, NULL);
	have_preview = (pixbuf != NULL);
	g_free (filename);

	gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
	if (pixbuf)
		gdk_pixbuf_unref (pixbuf);

	gtk_file_chooser_set_preview_widget_active (GTK_FILE_CHOOSER(widget), have_preview);
	
	return TRUE;
}


//*******************************************************************
//
void on_fileChooserButtonOK_clicked(OpenFileChooserDialog *dlg)
{	
	string 
		fileName,
		backupPath, 
		backupFilename,
		fileList, 
		command;

	ifstream
		infile;

	string 
		line,
		restorePath;

	if (NULL == dlg)
		return;
	
	//GSList *fileNames;
	gchar *fname;
					
	ifstream inFile;

	//***1.981a - new vars for user verification
	string
		line2,
		restoreDBName,
		restoreSurveyArea,
		restoreHome,
		message;
	GtkWidget
		*dialog,
		*label;
	gint result;
	int pos;

	switch (dlg->mOpenMode) 
	{
	case OpenFileChooserDialog::openFinImage:
		fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));
	
		fileName = fname;

		//g_print("lastDir: %s\n",gLastDirectory.c_str());
		//g_print("selFile: %s\n",fileName.c_str());

		if (fileName != gLastDirectory) 
		{

			try {
				ColorImage *newImage = new ColorImage(fileName);
			
				TraceWindow *win = new TraceWindow(
						dlg->mMainWin,
						fileName,
						newImage,
						dlg->mDatabase,
						dlg->mOptions);
				win->show();
		
				delete newImage;

				dlg->mMainWin->displayStatusMessage(_("Image opened successfully."));

			} catch (Error &e) {
				showError(e.errorString());
			}
		}
		break;
	case OpenFileChooserDialog::openFinTrace :

		fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));

		//g_print("Checking file: ");
		fileName = (char *)fname;
		g_print(fileName.c_str());
		g_print("\n");

		inFile.open(fileName.c_str());
		if (inFile.fail())
		{
			g_print("Could not open selected file!\n");
			inFile.clear();
		}
		else
		{
			char test[sizeof(unsigned long)+1];
			inFile.read((char*)&test, sizeof(unsigned long));
			inFile.close();
			test[5] = '\0';
			if ((strncmp(test,"DFIN",4) != 0) && (strncmp(test,"NIFD",4) != 0))
				g_print("This is not a traced dolphin fin!\n");
			else
			{
				// open the file and create a TraceWindow
				try {

					//ColorImage *newImage = new ColorImage(fileName);
					
					DatabaseFin<ColorImage> *unkFin = new DatabaseFin<ColorImage>(fileName);

					//***1.65 - if we are not displaying IDs, use fin file rootname instead
					if (dlg->mOptions->mHideIDs)
					{
						int 
							rootStart = 1 + fileName.find_last_of(PATH_SLASH),
							dotPos = fileName.rfind(".");
						string rootName = fileName.substr(rootStart, dotPos-rootStart);
						g_print(rootName.c_str());
						unkFin->mIDCode = rootName;
					}


					//***1.8 - two ways to retrieve original and modifed images now
					if (string::npos == unkFin->mImageFilename.rfind("_wDarwinMods.png"))
					{
						// fin file saved by version 1.75 and earlier
						// mImageFilename retrieved from the fin file is the original
						// image filename.  If there is a modified image it is a PPM
						// file with a name based on the fin filename
						string modImgFilename = fileName.substr(0,fileName.rfind(".fin"));
						ifstream fp;
						modImgFilename += "_wDarwinMods.ppm";
						fp.open(modImgFilename.c_str()); // try opending PPM file
						if (!fp.fail())
						{
							// found modified fin in version 1.75 or earlier file format
							fp.close();
							unkFin->mModifiedFinImage = new ColorImage(modImgFilename);
						}
						// original image filename was in fin file
						unkFin->mFinImage = new ColorImage(unkFin->mImageFilename);
					}
					else
					{
						// fin file saved by version 1.8 and later
						// mImageFilename retireved from the fin file is the modified
						// image filename.  The original image filename is stored
						// as a comment in the PNG image file
						ifstream fp;
						fp.open(unkFin->mImageFilename.c_str()); // try opening PNG file
						if (!fp.fail())
						{
							// found modified fin in new file format
							fp.close();
							unkFin->mModifiedFinImage = new ColorImage(unkFin->mImageFilename);
							// name of original image extracted from modified image is without path
							// we ASSUME it is in the tracedFins folder & want to set the
							// original image filename to the path+filename
							//string path =  getenv("DARWINHOME");
							//***1.85 - everything is now relative to the current survey area
							string path = gOptions->mCurrentSurveyArea;
							path += PATH_SLASH;
							path += "tracedFins";
							path += PATH_SLASH;
							unkFin->mOriginalImageFilename = 
								path + unkFin->mModifiedFinImage->mOriginalImageFilename;
							if ("" != unkFin->mOriginalImageFilename)
								unkFin->mFinImage = new ColorImage(unkFin->mOriginalImageFilename);
							// otherwise we leave it NULL for now, possibly make a copy of the
							// modified image here ??????
							unkFin->mImageMods = unkFin->mModifiedFinImage->mImageMods;
						}
					}

					TraceWindow *win = new TraceWindow(
							dlg->mMainWin,
							fileName,
							unkFin,
							dlg->mDatabase,
							dlg->mOptions);
					win->show();
		
					//delete newImage;

					//delete unkFin; ***1.65 - we pass pointer to TraceWindow and let TraceWindow 
					// delete the fin when done with it -- do NOT delete here

					dlg->mMainWin->displayStatusMessage(_("Fin Trace opened successfully."));

				} catch (Error &e) {
					showError(e.errorString());
				}
			}
		}
		g_free(fname);
		break;

	case  OpenFileChooserDialog::restoreDatabase : //***1/85 - new option
		// create backup filename .. should allow user to choose this

		backupFilename = "\"";
		backupFilename += gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));
		backupFilename += "\"";

		cout << "\nRestoring database from BACKUP location ...\n\n  " << backupFilename << endl;
		cout << "\n\nPlease wait." << endl;

		// extract the file list with the path information
		// this is necessary because the 7-zip program does NOT support relative
		// path information in a simple fashion when wanting to archive only
		// selected files within a folder

		command = "7z x -aoa "; // extract and overwrite existing file
		command += backupFilename + " filesToArchive.txt";
			
		system(command.c_str()); // extract the file list file

		infile.open("filesToArchive.txt");
		getline(infile,line); // first is location of original backup
		getline(infile,line); // this is the database file
		infile.close();

		system("del filesToArchive.txt"); // remove temporary file list file

		restorePath = line.substr(0,line.rfind(PATH_SLASH)+1) + "\""; //***1.981a re-append quote

		//***1.982 - break out parts and inform user what is about to happen

		line2 = line.substr(1,line.length()-2); // strip quotes
		pos = line2.rfind(PATH_SLASH);
		restoreDBName = line2.substr(pos+1); // save database name for restore
		line2 = line2.substr(0,pos); // strip database filename
		pos = line2.rfind(PATH_SLASH);
		line2 = line2.substr(0,pos); // strip "catalog"
		pos = line2.rfind(PATH_SLASH);
		restoreSurveyArea = line2.substr(pos+1); // save survey area name for restore
		line2 = line2.substr(0,pos); // strip survey area name
		pos = line2.rfind(PATH_SLASH);
		line2 = line2.substr(0,pos); // strip "surveyAreas"
		restoreHome = line2; // save DARWINHOME for restore

		if (restoreHome != dlg->mOptions->mDarwinHome)
		{
			// this backup file came from another DARWIN installation
			GtkWidget *msgDialog = gtk_message_dialog_new(GTK_WINDOW(dlg->mDialog),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_INFO,
                                  GTK_BUTTONS_OK,
                                  "This Backup is from a different installation of DARWIN.\n"
								  "It cannot be RESTORED here.  Use IMPORT instead.");
			gtk_dialog_run (GTK_DIALOG (msgDialog));
			gtk_widget_destroy (msgDialog);
			delete dlg;
			return;
		}

		message = "\nYou are about to replace the database below with the\n";
		message += "backup copy you selected.  DO NOT PROCEED if you are\n";
		message += "attempting to move this database to a different location.\n\n";
		message += "DARWINHOME:  " + restoreHome + "\n";
		message += "SURVEYAREA:  " + restoreSurveyArea +"\n";
		message += "     DATABASE:  " + restoreDBName + "\n\n";
		message += "Are you sure you want to proceed with restoring\n";
		message += "the database in the location indicated? \n\n";
			
		dialog = gtk_dialog_new_with_buttons ("Caution, restore cannot be undone!",
						GTK_WINDOW(dlg->mDialog),
						(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
						GTK_STOCK_OK,
						GTK_RESPONSE_ACCEPT,
						GTK_STOCK_CANCEL,
						GTK_RESPONSE_REJECT,
						NULL);

		label = gtk_label_new (message.c_str());
		gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
		gtk_widget_show_all(dialog);

		result = gtk_dialog_run (GTK_DIALOG (dialog));

		gtk_widget_destroy (dialog);
		switch (result)
		{
		case GTK_RESPONSE_ACCEPT:
			// do nothing except proceed with restore below
			break;
		default:
			// abort restore operation
			delete dlg;
			return;
		}

		{	// this BLOCK is simply to allow local vars for file / folder search & repair

			// determine whether the file folder structure is intact for this
			// database location ... ensure existance of surveyAreas\<restoreSurveyArea> and
			// its subfolders ...
			//    catalog, tracedFins, matchQueues, matchQResults, sightings
			struct _finddata_t c_file;
			long hFile;
			string path;

			// Find surveyArea\<restoreSurveyArea>
			path = restoreHome + PATH_SLASH + "surveyAreas";
			if( (hFile = _findfirst(path.c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"SurveyAreas\" folder ...\n" );
				_mkdir(path.c_str());
			}
			_findclose( hFile );
			path += PATH_SLASH + restoreSurveyArea;
			if( (hFile = _findfirst(path.c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"%s\" SurveyArea subfolder...\n", 
					restoreSurveyArea.c_str());
				_mkdir(path.c_str());
			}
			_findclose( hFile );
			// find subfolders and, if missing, fix them ...
			path += PATH_SLASH;
			if( (hFile = _findfirst((path+"catalog").c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"catalog\" folder ...\n" );
				_mkdir((path+"catalog").c_str());
			}
			_findclose( hFile );
			if( (hFile = _findfirst((path+"tracedFins").c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"tracedFins\" folder ...\n" );
				_mkdir((path+"tracedFins").c_str());
			}
			_findclose( hFile );
			if( (hFile = _findfirst((path+"matchQueues").c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"matchQueues\" folder ...\n" );
				_mkdir((path+"matchQueues").c_str());
			}
			_findclose( hFile );
			if( (hFile = _findfirst((path+"matchQResults").c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"matchQResults\" folder ...\n" );
				_mkdir((path+"matchQResults").c_str());
			}
			_findclose( hFile );
			if( (hFile = _findfirst((path+"sightings").c_str(), &c_file )) == -1L )
			{
				printf( "Creating missing \"sightings\" folder ...\n" );
				_mkdir((path+"sightings").c_str());
			}
		}
		//***1.982 - end user notification

		// all is OK, so now proceed ...
		
		// extract database file from the archive

		// note: restorePath and backupFilename are already QUOTED

		command = "7z x -aoa -o"; // put in proper folder and force overwriting
		command += restorePath + " ";
		command += backupFilename + " *.db";

		dlg->mMainWin->mDatabase->closeStream(); // close database in order to overwrite it

		cout << command << endl;

		system(command.c_str()); // start extraction process

		// extract images referenced from database file

		//command = "7z x -aos -o\""; // extract into proper folder without overwriting
		//***1.981a - extra quote removed
		command = "7z x -aos -o"; // extract into proper folder without overwriting
		command += restorePath + " -x!filesToArchive.txt "; // but don't extract file list
		command += " -x!*.db "; // and don't extract database again
		command += backupFilename;

		cout << command << endl;

		system(command.c_str()); // start extraction process

		//***1.982 - update current Survey Area indicator in global Options
		gOptions->mCurrentSurveyArea = 
			restoreHome + PATH_SLASH + "surveyAreas" + PATH_SLASH + restoreSurveyArea;

		// now restart this dialog so we can OPEN the new database
		gtk_widget_destroy(dlg->mDialog);
		dlg->mOpenMode = OpenFileChooserDialog::openDatabase;
		dlg->mDialog = dlg->createFileChooserDialog();
		dlg->run_and_respond();

		return; // without deleting the dialog since it will have happened
		        // already in the OpenDatabase version of the dialog above

		break;  // this is not needed

	case OpenFileChooserDialog::openDatabase : //***1/85 - new option

		fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));

		//g_print("Checking file: ");
		fileName = (char *)fname;
		//g_print(fileName.c_str());
		//g_print("\n");

		inFile.open(fileName.c_str());
		if (inFile.fail())
		{
			g_print("Could not open selected database file!\n");
			inFile.clear();
		}
		else
		{
			// see if this is really a database file
			unsigned long dummy;
			inFile.read((char*)&dummy, sizeof(unsigned long));// skip footer offset
			unsigned int test;
			inFile.read((char*)&test, sizeof(int));           // read version #
			inFile.close();
			if (((test & 0xFFFF0000) != 0x44420000) &&  // not "DB" v1.85 & 3 or later
				(test != CURRENT_DBVERSION))            // nor simple CURRENT version number
			{
				g_print("\nUnsupported database version or a non database file!\n");
				g_print("\nFile NOT loaded.\n");
			}
			else
			{
				// set the options current survey area and database filename
				dlg->mOptions->mDatabaseFileName = fileName;
				string cat = "";
				cat += PATH_SLASH;
				cat += "catalog";
				dlg->mOptions->mCurrentSurveyArea = fileName.substr(0,fileName.rfind(cat));

				// make sure the current Options include the survey area and database
				// filename of the new database ... if not, then add them to the list
				// to make sure program knows of them in future

				bool known = false;
				int i;
				string shortAreaName = dlg->mOptions->mCurrentSurveyArea;
				shortAreaName = shortAreaName.substr(shortAreaName.rfind(PATH_SLASH)+1);
				for (i = 0; ((! known) && (i < dlg->mOptions->mNumberOfExistingSurveyAreas)); i++)
					if (shortAreaName == dlg->mOptions->mExistingSurveyAreaName[i])
						known = true;
				if (! known)
				{
					// add survey area name to list
					dlg->mOptions->mExistingSurveyAreaName.push_back(shortAreaName);
					dlg->mOptions->mNumberOfExistingSurveyAreas ++;
				}

				known = false;
				string shortDbName = dlg->mOptions->mDatabaseFileName;
				shortDbName = shortDbName.substr(shortDbName.rfind(PATH_SLASH)+1);
				shortDbName = (shortAreaName + PATH_SLASH) + shortDbName;
				for (i = 0; ((! known) && (i < dlg->mOptions->mNumberOfExistingDatabases)); i++)
					if (shortDbName == dlg->mOptions->mExistingDatabaseName[i])
						known = true;
				if (! known)
				{
					// add survey area name to list
					dlg->mOptions->mExistingDatabaseName.push_back(shortDbName);
					dlg->mOptions->mNumberOfExistingDatabases ++;
				}

				// close the current database
				delete dlg->mDatabase; // this closes the current database file

				// open the new database
				dlg->mDatabase = new Database<ColorImage>(dlg->mOptions, false);

				// make sure main window has correct pointer to it -- THIS IS IMPORTANT
				dlg->mMainWin->mDatabase = dlg->mDatabase;

				// force the main window to reload and display the new database
				dlg->mMainWin->show();
			}
		}
		g_free(fname);
		break;
	
	case  OpenFileChooserDialog::backupDatabase : //***1/85 - new option

		// nothing occurs here, it is all done in the MainWindow code

		break;

	case  OpenFileChooserDialog::exportDataFields : //***1.9 - new option

		// need a way to send filename back to DataExportDialog

		dlg->mMainWin->mExportToFilename = 
			gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));

		break;
	

	case  OpenFileChooserDialog::exportDatabase : //***1/85 - new option

		// set filename in mainWindow and let it handle rest of export

		dlg->mMainWin->mExportToFilename = 
			gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));

		break;
	
	case  OpenFileChooserDialog::importDatabase : //***1/85 - new option

		// set filename in mainWindow and let it handle rest of export

		dlg->mMainWin->mImportFromFilename = 
			gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));

		break;
	}

	delete dlg;
}


//*******************************************************************
//
void on_fileChooserButtonCancel_clicked(
	GtkButton *button,
	gpointer userData)
{
	OpenFileChooserDialog *dlg = (OpenFileChooserDialog *) userData;
	
	if (NULL == dlg)
		return;

	dlg->mMainWin->displayStatusMessage(_("Open canceled."));

	//***1.95 - capture current folder and filename HERE
	gchar *folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dlg->mDialog));
	gchar *fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));

	gLastDirectory = folder;
	if (NULL == fname)
		gLastFileName = "";
	else
	{
		string 
			name = fname, 
			path = folder;
		if (path.substr(path.rfind(PATH_SLASH)+1) == name.substr(name.rfind(PATH_SLASH)+1))
			// there is no real filename, just a folder at end of path
			gLastFileName = "";
		else
			gLastFileName = name.substr(name.rfind(PATH_SLASH)+1);
		g_free(fname);
	}
	g_free(folder);

	delete dlg;
}


//*******************************************************************
//
// void on_openFileChooserFileSelections_changed(...)
//
void on_openFileChooserFileSelections_changed(
	GtkWidget *widget,
	gpointer userData
	)
{
	//g_print("IN on_openFileChooserFileSelections_changed()\n");

	OpenFileChooserDialog *dlg = (OpenFileChooserDialog *) userData;

	if (NULL == dlg)
		return;
		
	gchar *fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dlg->mDialog));

	if (NULL == fileName)
	{
		// change in folder or something eles that "unselected" all files
		gLastFileName = "";
	}
	else
	{
		// strip path and set global last filename so we can return there next time
		gLastFileName = fileName;
		gLastFileName = gLastFileName.substr(gLastFileName.find_last_of(PATH_SLASH)+1);

		//	g_print("Last Filename : ");
		//	g_print(gLastFileName.c_str());
		//	g_print("\n");

		g_free(fileName);
	}
}


//*******************************************************************
//
// void on_openFileChooserDirectory_changed(...)
//
void on_openFileChooserDirectory_changed(
	GtkWidget *widget,
	gpointer userData)
{
	OpenFileChooserDialog *dlg = (OpenFileChooserDialog *) userData;
	
	//g_print("IN on_openFileChooserDirectory_changed()\n");

	if (NULL == dlg)
		return;

	gchar *folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dlg->mDialog));

	// set global last directory and blank filename so we can return there next time
	gLastDirectory = folder;
	gLastFileName = "";

	//g_print("Last Folder : ");
	//g_print(gLastDirectory.c_str());
	//g_print("\n");

	g_free(folder);
}


//*******************************************************************
//
void OpenFileChooserDialog::run_and_respond()
{
	gint response = gtk_dialog_run (GTK_DIALOG (mDialog));

	switch (response)
	{
	case GTK_RESPONSE_CANCEL :
		on_fileChooserButtonCancel_clicked(NULL,this);
		break;
	case GTK_RESPONSE_ACCEPT :
		on_fileChooserButtonOK_clicked(this);
		break;
	default :
		// no other action required
		break;
	}
}
