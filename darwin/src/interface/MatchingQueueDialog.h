//*******************************************************************
//   file: MatchingQueueDialog.h
//
// author: Adam Russell
//
//   mods: J H Stewman (2006)
//         -- progress bar added
//         -- support for saving and reviewing of results
//
//*******************************************************************

#ifndef MATCHINGQUEUEDIALOG_H
#define MATCHINGQUEUEDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "../matching/MatchingQueue.h"
#include "../Database.h"
#include "../image_processing/GrayImage.h"
#include "../image_processing/ColorImage.h"
#include "../interface/MainWindow.h"

#include "../Options.h"   //***054

typedef enum {SAVE_QUEUE, LOAD_QUEUE, ADD_FILENAME, DELETE_FILENAME, VIEW_RESULTS} mqActionSelection_t;

int getNumMatchingQueueDialogReferences();

class MatchingQueueDialog
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but doesn't show
		// 	the dialog.  show() must later be called to do that.
		MatchingQueueDialog(MainWindow *mainWin, Database<ColorImage> *db, Options *o);
	
		// Destructor
		// 	Destroys the dialog if it's open and frees resources.
		~MatchingQueueDialog();

		// show
		// 	Simply draws the dialog on the screen.
		void show();

		friend gboolean matchingQueueIdleFunction(
			gpointer userData);

		// GTK+ callback functions

		friend gboolean on_matchingQueueDialog_delete_event(
			GtkWidget *widget,
			GdkEvent *event,
			gpointer userData
			);

		friend void on_MQ_mCList_select_row(
			GtkCList *clist,
			gint row,
			gint column,
			GdkEvent *event,
			gpointer userData
			);

		friend gboolean on_matchingQueueDrawingArea_expose_event(
			GtkWidget *widget,
			GdkEventExpose *event,
			gpointer userData
			);

		friend void on_matchingQueueButtonAdd_clicked(
			GtkButton *button,
			gpointer userData
			);

		friend void on_matchingQueueButtonRemove_clicked(
			GtkButton *button,
			gpointer userData
			);
		
		friend void on_matchingQueueButtonRunMatch_clicked(
			GtkButton *button,
			gpointer userData
			);

		friend void on_matchingQueueButtonViewResults_clicked(
			GtkButton *button,
			gpointer userData
			);

		friend void on_matchingQueueButtonSaveList_clicked(
			GtkButton *button,
			gpointer userData
			);

		friend void on_matchingQueueButtonLoadList_clicked(
			GtkButton *button,
			gpointer userData
			);

		friend void on_matchingQueueButtonCancel_clicked(
			GtkButton *button,
			gpointer userData
			);

		friend gboolean on_matchingQueueFileSelectionDialog_delete_event(
			GtkWidget *widget,
			GdkEvent *event,
			gpointer userData
			);

		friend void on_mqFileSelectionButtonOK_clicked(
			GtkButton *button,
			gpointer userData
			);

		friend void on_mqFileSelectionButtonCancel_clicked(
			GtkButton *button,
			gpointer userData
			);

		friend void on_mqFileSelectionListCell_changed(
			GtkWidget *widget,
			gpointer userData
			);

		friend void on_mqFileSelectionEntry_changed(
			GtkWidget *widget,
			gpointer userData
			);

		//***1.4 - new functions for FileChooser

		friend void on_mqFileChooserButtonOK_clicked(
			MatchingQueueDialog *dialog
			);

		friend void on_mqFileChooserButtonCancel_clicked(
			MatchingQueueDialog *dialog
			);

		friend void on_mqFileChooserFileSelections_changed(
			GtkWidget *widget,
			gpointer userData
			);

		friend void on_mqFileChooserDirectory_changed(
			GtkWidget *widget,
			gpointer userData
			);

		friend void mqFileChooser_run_and_respond(
			MatchingQueueDialog *dlg
			);


	private:
		GtkWidget
			*mDialog,
			*mCList,
			*mDrawingArea,
			*mFileSelectionDialog,
			*mFileChooserDialog, //***1.4
			*mProgressBar1, // for tracking percent of queue processed
			*mProgressBar2; // for tracking percent of database compared to current unknown

		MatchingQueue *mMatchingQueue;

		Database<ColorImage> *mFinDatabase;

		MainWindow *mMainWin;			//***1.1

		Options *mOptions; //***054

		ColorImage *mImage;

		int mLastRowSelected;

		//***1.1 - additions of bools to control idle function
		bool
			mMatchCancelled,
			mMatchRunning,
			mMatchPaused;

		mqActionSelection_t mActionSelected;

		GtkWidget* createMatchingQueueDialog();
		GtkWidget* createMatchingQueueFileSelectionDialog();
		GtkWidget* createMatchingQueueFileChooserDialog(); //***1.4
		void updateQueueList();
};

#endif
