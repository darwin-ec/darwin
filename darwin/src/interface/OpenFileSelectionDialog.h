//*******************************************************************
//   file: OpenFileSelectionDialog.h
//
// author: Adam Russell
//
//   mods: J H Stewman
//         -- fixed previews
//         -- added support for return to previously selected file
//
//*******************************************************************

#ifndef OPENFILESELECTIONDIALOG_H
#define OPENFILESELECTIONDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"
#include <gtk/gtktext.h>

#include "../image_processing/ColorImage.h"
#include "../Database.h"
#include "MainWindow.h"
#include "../Options.h"

// getNumOpenFileSelectionDialogReferences
// 	Returns the number of dialogs open.
int getNumOpenFileSelectionDialogReferences();

class OpenFileSelectionDialog
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.
		// 	
		// 	Does a pointer copy of db and o
		OpenFileSelectionDialog(
				Database<ColorImage> *db,
				MainWindow *m,
				Options *o);
		
		// Destructor
		// 	Destroys the dialog if it's open and frees
		// 	resources.
		~OpenFileSelectionDialog();

		// show
		// 	Simply draws the dialog on the screen.
		void show();

		// GTK+ callback functions
		friend gboolean on_fileSelectionDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_filePreviewCheckButton_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

		friend gboolean on_fileDrawingArea_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend void on_fileSelectionEntry_changed(
				GtkWidget *widget,
				gpointer userData);

		// new functions (2)

		friend void on_directoryList_changed(
				GtkWidget *widget,
				gpointer userData);

		friend void on_fileListCell_changed(
				GtkWidget *widget,
				gpointer userData);

		friend void on_fileButtonOK_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_fileButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);
	private:
		GtkWidget
			*mDialog,
			*mDrawingArea,
			*mText;

		Database<ColorImage> *mDatabase;

		ColorImage *mImage;

		MainWindow *mMainWin;

		Options *mOptions;

		bool mBlackImage;
		bool mShowPreview;

		int mNumTextChars;
		
		GtkWidget* createFileSelectionDialog();

		void refreshImage();
};

#endif
