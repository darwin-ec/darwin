//*******************************************************************
//   file: OpenFileChooserDialog.h
//
// author: J H Stewman (7/13/2006)
//         -- modification of older OpenFileSelectionDialog
//         -- this will be used to open Images, Fin Tracings, and Databases
//         -- Open Fin Tracings will be implemented first
//
//*******************************************************************

#ifndef OPENFILECHOOSERDIALOG_H
#define OPENFILECHOOSERDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"
#include <gtk/gtktext.h>

#include "../image_processing/ColorImage.h"
#include "../Database.h"
#include "MainWindow.h"
#include "../Options.h"

// getNumOpenFileChooserDialogReferences
// 	Returns the number of dialogs open.
int getNumOpenFileChooserDialogReferences();

class OpenFileChooserDialog
{
	public:

		// three modes of operation
		enum {
			openFinImage = 0,
			openFinTrace,
			openDatabase,
			importDatabase, //***1.85
			exportDatabase, //***1.85
			backupDatabase, //***1.85
			restoreDatabase,  //***1.85
			exportDataFields //***1.9
		};

		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.
		// 	
		// 	Does a pointer copy of db and o
		OpenFileChooserDialog(
				Database<ColorImage> *db,
				MainWindow *m,
				Options *o,
				int openMode);
		
		// Destructor
		// 	Destroys the dialog if it's open and frees
		// 	resources.
		~OpenFileChooserDialog();

		void run_and_respond(); // replaces show

		// GTK+ callback functions
		friend gboolean on_fileChooserDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_fileChooserPreviewCheckButton_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

		friend gboolean on_fileChooserDrawingArea_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend gboolean on_fileChooserPreview_update( //***1.95
				GtkWidget *widget,
				gpointer userData);

		friend void on_fileChooserButtonOK_clicked(
				OpenFileChooserDialog *dialog);

		friend void on_fileChooserButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_openFileChooserFileSelections_changed(
				GtkWidget *widget,
				gpointer userData);

		friend void on_openFileChooserDirectory_changed(
				GtkWidget *widget,
				gpointer userData);

		friend void openFileChooser_run_and_respond(MainWindow *win);

	private:
		GtkWidget
			*mDialog,
			*mDrawingArea,
			*mText,
			*mPreview; //***1.95

		Database<ColorImage> *mDatabase;

		ColorImage *mImage;

		MainWindow *mMainWin;

		Options *mOptions;

		int mOpenMode; // openFinImage, openFinTrace or openDatabase

		bool mBlackImage;
		bool mShowPreview;

		int mNumTextChars;
		
		GtkWidget* createFileChooserDialog();

		void refreshImage();
};

#endif
