//*******************************************************************
//   file: OpenFileSelectionDialog.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman 8/8/2005)
//         -- reformatting of code and addition of comment blocks
//         -- code to return focus to previous file opened
//
//*******************************************************************


#include "../support.h"
#include "OpenFileSelectionDialog.h"

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

#include "../image_processing/transform.h"
#include "ErrorDialog.h"
#include "TraceWindow.h"

gboolean on_fileSelectionDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

void on_filePreviewCheckButton_toggled(
		GtkToggleButton *togglebutton,
		gpointer userData);

gboolean on_fileDrawingArea_expose_event(
		GtkWidget *widget,
		GdkEventExpose *event,
		gpointer userData);

void on_fileSelectionEntry_changed(
		GtkWidget *widget,
		gpointer userData);

void on_directoryList_changed(
		GtkWidget *widget,
		gpointer userData);

void on_fileListCell_changed(
		GtkWidget *widget,
		gpointer userData);

void on_fileButtonOK_clicked(
		GtkButton *button,
		gpointer userData);

void on_fileButtonCancel_clicked(
		GtkButton *button,
		gpointer userData);

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
int getNumOpenFileSelectionDialogReferences()
{
	return gNumReferences;
}

//*******************************************************************
//
OpenFileSelectionDialog::OpenFileSelectionDialog(
		Database *db,
		MainWindow *m,
		Options *o
)
	: mDialog(createFileSelectionDialog()),
	  mDatabase(db),
	  mImage(NULL),
	  mMainWin(m),
	  mOptions(o),
	  mBlackImage(true),
	  mShowPreview(gShowingPreviews), //***051
	  mNumTextChars(0)
{
	gNumReferences++;
}

//*******************************************************************
//
OpenFileSelectionDialog::~OpenFileSelectionDialog()
{
	if (NULL != mDialog)
		gtk_widget_destroy(mDialog);

	delete mImage;

	gNumReferences--;
}

//*******************************************************************
//
void OpenFileSelectionDialog::show()
{
	gtk_widget_show(mDialog);

	// force issue of redrawing preview image
	if (gLastDirectory != "")
		on_fileSelectionEntry_changed(NULL, (void *)this);
}

//*******************************************************************
//
void OpenFileSelectionDialog::refreshImage()
{
	on_fileDrawingArea_expose_event(NULL, NULL, (void *) this);
}


//*******************************************************************
//
GtkWidget* OpenFileSelectionDialog::createFileSelectionDialog()
{
	GtkWidget *fileSelectionDialog;
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
	gtk_window_set_position(GTK_WINDOW(fileSelectionDialog), GTK_WIN_POS_CENTER); //***1.8
	gtk_window_set_wmclass(GTK_WINDOW(fileSelectionDialog), "darwin_open", "DARWIN");

	fileHBox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (fileHBox);
	gtk_container_add (GTK_CONTAINER (fileSelectionDialog), fileHBox);

	fileSelection = gtk_file_selection_new(_("Select a File"));

	if (gLastDirectory == "")
	{
		// set path to %DARWINHOME% for first file open
		//gLastDirectory = getenv("DARWINHOME");
		gLastDirectory = gOptions->mDarwinHome; //SAH 2008-07-18
		//gLastDirectory += PATH_SLASH;
	}


	gtk_file_selection_set_filename(
			GTK_FILE_SELECTION(fileSelection),
			((gLastDirectory+PATH_SLASH)+gLastFileName).c_str()); //***1.8

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
}

//*******************************************************************
//
gboolean on_fileSelectionDialog_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	OpenFileSelectionDialog *dlg = (OpenFileSelectionDialog *) userData;
	
	if (NULL == dlg)
		return FALSE;

	dlg->mMainWin->displayStatusMessage("Open canceled.");

	delete dlg;

	return TRUE;
}


//*******************************************************************
//
void on_filePreviewCheckButton_toggled(
	GtkToggleButton *togglebutton,
	gpointer userData)
{
	OpenFileSelectionDialog *dlg = (OpenFileSelectionDialog *) userData;
	
	if (NULL == dlg)
		return;

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
}


//*******************************************************************
//
gboolean on_fileDrawingArea_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData)
{
	OpenFileSelectionDialog *dlg = (OpenFileSelectionDialog *) userData;
	
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
// void on_fileSelectionEntry_changed(...)
//
//    This is called whenever the contents of the selection_entry changes.
//    This happens when the directory is changed and the selection_entry
//    gets "blanked," and it happens when the highlighted file in the
//    file list changes.
//
void on_fileSelectionEntry_changed(
	GtkWidget *widget,
	gpointer userData)
{
	OpenFileSelectionDialog *dlg = (OpenFileSelectionDialog *) userData;
	
	//g_print("IN on_fileSelectionEntry_changed()\n");

	if (NULL == dlg)
		return;

	GtkWidget *fileSelect = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(dlg->mDialog), "filesel"));

	string fileName = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fileSelect));
	//g_print("fileName is %s\n",fileName.c_str());

	GtkTreeSelection *dirTreeSelect,*fileTreeSelect;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gboolean dirSelected, fileSelected;
	gchar *fname = NULL, *dname = NULL;
	string fNameStr, dNameStr;

	// see if there is a selected folder name
	dirTreeSelect = gtk_tree_view_get_selection(
			GTK_TREE_VIEW(GTK_FILE_SELECTION(fileSelect)->dir_list));
	if (dirSelected = gtk_tree_selection_get_selected(dirTreeSelect,&model,&iter))
	{
		//g_print("There IS a selected FOLDER : ");
		gtk_tree_model_get(model,&iter,0,&dname,-1);
		//g_print("name = \"%s\"\n",dname);
		dNameStr = dname;
	}

	// see if there is a selected file name
	fileTreeSelect = gtk_tree_view_get_selection(
			GTK_TREE_VIEW(GTK_FILE_SELECTION(fileSelect)->file_list));
	if (fileSelected = gtk_tree_selection_get_selected(fileTreeSelect,&model,&iter))
	{
		//g_print("There IS a selected FILE : ");
		gtk_tree_model_get(model,&iter,0,&fname,-1);
		//g_print("name = \"%s\"\n",fname);
		fNameStr = fname;
	}

	if ((FALSE == (bool)dirSelected) && (FALSE == (bool)fileSelected))
	{
		// no selected names in either tree, so we have just jumped
		// to a new directory path, the filename is the path
		gLastDirectory = fileName;
		//gLastDirectory += PATH_SLASH;
		gLastFileName = "";
		//g_print("gLastDirectory set to \"%s\"\n",gLastDirectory.c_str());
	}
	else if ((FALSE == (bool)dirSelected) && (TRUE == (bool)fileSelected))
	{
		// This is one of possibly two calls to this function in response 
		// to a change in the highlighted file. The selected file will NOT 
		// be part of the fileName on the first call, but will on the second.
		int posit = fileName.rfind(fNameStr);

		if (string::npos != posit)
		{
			// take action on FIRST call, setting directory and filename
			// as appropriate

			if (gLastDirectory == "")
				gLastDirectory = fileName;
			gLastFileName = fNameStr;
			//g_print("gLastFileName set to \"%s\"\n",gLastFileName.c_str());
			
			if	(gLastTreePathStr != NULL) {
				g_free(gLastTreePathStr);
				gLastTreePath = NULL;
			}

			// find GtkTree path & save as formatted string.  This identifies
			// the row to make active when we reopen the FileSelector the
			// next time
			gLastTreePath = gtk_tree_model_get_path(model,&iter);
			gLastTreePathStr = gtk_tree_path_to_string(gLastTreePath);
		
		//***1.6 - moved close brace past image preview, to prevent showing "Not Found" Image
		//         fileName contains ONLY the path

			// since a file IS selected, draw the preview image, if appropriate

			if (!dlg->mShowPreview) {
				if (dlg->mBlackImage)
					return;
				delete dlg->mImage;
				dlg->mImage = new ColorImage(IMAGE_HEIGHT, IMAGE_WIDTH);
				dlg->mBlackImage = true;
				dlg->refreshImage();
				return;
			}

			try
			{
				ColorImage *newImage = new ColorImage(fileName);

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

		} //***1.6 - new position of close brace for if (string::npos != posit)
	}

	// free temp strings
	if (fname) g_free(fname);
	if (dname) g_free(dname);
}


//*******************************************************************
//
// void on_directoryList_changed(...)
//
//    This is called when the selected row on the directory list changes.
//    That means the highlighted selection is different, but the same
//    directory list is being displayed.  The directory path does NOT
//    change here, only when a double-click occurs. 
//
void on_directoryList_changed(
	GtkWidget *widget,
	gpointer userData)
{
	//g_print("IN on_directoryList_changed()\n");

	OpenFileSelectionDialog *dlg = (OpenFileSelectionDialog *) userData;
	
	if (NULL == dlg)
		return;

	GtkWidget *fileSelect = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(dlg->mDialog), "filesel"));

	string fileName = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fileSelect));
	//g_print("fileName is %s\n",fileName.c_str());

	GtkTreeSelection *treeSelect = GTK_TREE_SELECTION(widget);

	GtkTreeIter iter;
	GtkTreeModel *model;

	if (gtk_tree_selection_get_selected(treeSelect,&model,&iter))
	{
		// A change has occurred in the highlighting of a name
		// and some name is now highlighted.  It is NOT the currently
		// active or open folder BUT it will be the new folder opened
		// if a double-click occurs following this call to the function.
		gchar *fname;
		gtk_tree_model_get(model,&iter,0,&fname,-1);
		//g_print("name = \"%s\"\n",fname);

		gLastFolderName = fname;

		g_free(fname);
	}
	else
	{
		// There is NO highlighted or selected folder name
		// so we have made a CHANGE in the currently active directory.

		//g_print("Just changed to NEW directory : ");

		treeSelect = gtk_tree_view_get_selection(
			GTK_TREE_VIEW(GTK_FILE_SELECTION(fileSelect)->file_list));

		if (FALSE == gtk_tree_selection_get_selected(treeSelect,&model,&iter))
		{
			// There is NO highlighted file name in the other tree then
			// this is the only callback being signalled, and the change
			// of directory name must be handled here.
					
			gLastDirectory = fileName;
			//gLastDirectory += PATH_SLASH;
			gLastFileName = "";
			//g_print("gLastDirectory set to \"%s\"\n",gLastDirectory.c_str());
		}
		//else
			//g_print("No action taken here.\n");

		// show "BLACK" image in preview window since NO file is selected
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
}

//*******************************************************************
//
// void on_fileListCell_changed(...)
//
//    This gets called once AFTER the on_fileSelectionEntry_changed()
//    callback.  So, the signal that the GtkFileSelection->selection_entry
//    has "changed" seems to preceed the signal that the 
//    GtkFileSelection->file_viewhas "changed" -- this may NOT be a consistent
//    order.
//
void on_fileListCell_changed(
	GtkWidget *widget,
	gpointer userData)
{
	//g_print("IN on_fileListCell_changed()\n");

	OpenFileSelectionDialog *dlg = (OpenFileSelectionDialog *) userData;

	if (NULL == dlg)
		return;
		
	GtkWidget *fileSelect = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(dlg->mDialog), "filesel"));

	string fileName = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fileSelect));
	//g_print("fileName is %s\n",fileName.c_str());

	GtkTreeSelection *treeSelect = GTK_TREE_SELECTION(widget);

	GtkTreeIter iter;
	GtkTreeModel *model;

	if (gtk_tree_selection_get_selected(treeSelect,&model,&iter))
	{
		// A change has occurred in the highlighting of a file name
		// and some name is now highlighted.  The selected file is the file
		// that will be opened if a double-click occurs following this call 
		// to the function.
		gchar *fname;
		gtk_tree_model_get(model,&iter,0,&fname,-1);
		//g_print("name = \"%s\"\n",fname);

		gLastFileName = fname;

		g_free(fname);
	}
	else
	{
		// There is NO highlighted or selected file name
		// so we have just made a CHANGE in the currently active directory.
		// Nothing to do here.
		
		//g_print("Just changed to NEW directory\n");

	}
}


//*******************************************************************
//
void on_fileButtonOK_clicked(
	GtkButton *button,
	gpointer userData)
{
	OpenFileSelectionDialog *dlg = (OpenFileSelectionDialog *) userData;
	
	if (NULL == dlg)
		return;

	GtkWidget *fileSelect = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(dlg->mDialog), "filesel"));

	string fileName = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fileSelect));

	//***1.8 - clean up crash on attempt to open non file, eliminate this
	//if (fileName[fileName.length() - 1] == '/' || fileName[fileName.length() - 1] == '\\') {
	//if (fileName == gLastDirectory) { //***1.8 - folders do NOT have slash at end
	//	showError(_("Oops.. It looks like you selected a directory.\nPlease try again, and select an image file."));
	//	dlg->mMainWin->displayStatusMessage(_("Error opening selected file."));
	//} else {
	
	if (fileName != gLastDirectory) { //***1.8 - try and open it, else do nothing

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
		delete dlg; //***1.8 - moved from below
	}
}

//*******************************************************************
//
void on_fileButtonCancel_clicked(
	GtkButton *button,
	gpointer userData)
{
	OpenFileSelectionDialog *dlg = (OpenFileSelectionDialog *) userData;
	
	if (NULL == dlg)
		return;

	dlg->mMainWin->displayStatusMessage(_("Open canceled."));
	
	delete dlg;
}
