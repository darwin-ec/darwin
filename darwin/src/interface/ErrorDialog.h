//*******************************************************************
//   file: ErrorDialog.h
//
// author: Adam Russell
//
//   mods: 
//
//*******************************************************************

#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include "../Error.h"

int getNumErrorDialogReferences();

class ErrorDialog
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.
		ErrorDialog();
		ErrorDialog(std::string errorMsg);
		ErrorDialog(Error e);

		~ErrorDialog();

		void show();

		// GTK+ callback functions
	
		friend gboolean on_errorDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_errorButtonOK_clicked(
				GtkButton *button,
				gpointer userData);

	private:

		GtkWidget *mDialog;

		GtkWidget* createErrorDialog(std::string errorMsg);
};

// And, a shortcut function
inline void showError(std::string errorMsg)
{
	ErrorDialog *dlg = new ErrorDialog(errorMsg);
	dlg->show();
}

#endif
