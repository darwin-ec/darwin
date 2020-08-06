//*******************************************************************
//   file: SaveFileSelectionDialog.h
//
// author: Adam Russell
//
//   mods:
//
//*******************************************************************

#ifndef SAVEFILESELECTIONDIALOG_H
#define SAVEFILESELECTIONDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "../DatabaseFin.h"
#include "TraceWindow.h"

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

int getNumSaveFileSelectionDialogReferences();

class SaveFileSelectionDialog
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.
		//
		// 	Note: Only does a pointer copy of dbFin.  Also,
		// 	will delete dbFin when done!
		SaveFileSelectionDialog(
				DatabaseFin<ColorImage> *dbFin, 
				TraceWindow *traceWin,
				GtkWidget *parent);

		// Destructor
		// 	Destroys the dialog if it's open and frees
		// 	resources.
		~SaveFileSelectionDialog();

		// show
		// 	Simply draws the dialog on the screen.
		void show();

		// GTK+ callback functions

		friend gboolean on_saveFileSelection_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_saveButtonOK_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_saveButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);

	private:
		GtkWidget 
			*mDialog,
			*mParent;

		TraceWindow *mTraceWin; // the parent, to be deleted after successful save
		
		DatabaseFin<ColorImage> *mFin;

		GtkWidget *createSaveFileSelection();

		void save(std::string fileName);
};

#endif
