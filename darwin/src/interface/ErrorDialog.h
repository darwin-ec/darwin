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

/*
//***2.22 - totally unnecessary class now - especially since dialogs come up
//          BEHIND parent windows on the Mac

int getNumErrorDialogReferences();

class ErrorDialog
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.
		ErrorDialog();
		//ErrorDialog(std::string errorMsg);
		ErrorDialog(GtkWidget *parent, std::string errorMsg); //***2.22
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

		GtkWidget *mParent;

		GtkWidget* createErrorDialog(std::string errorMsg);
};
*/

//***2.22 - use this for all error andmessage dialogs now
// And, a shortcut function
inline void showError(std::string errorMsg, GtkWidget *parent=NULL)
{
	//ErrorDialog *dlg = new ErrorDialog(errorMsg);
	//dlg->show();
	//***2.22 - replacing own ErrorDialog with GtkMessageDialogs
	GtkWidget *errd = gtk_message_dialog_new (GTK_WINDOW(parent),
								GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_ERROR,
								GTK_BUTTONS_CLOSE,
								errorMsg.c_str());
	gtk_dialog_run (GTK_DIALOG (errd));
	gtk_widget_destroy (errd);
}

#endif
