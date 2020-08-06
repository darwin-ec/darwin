//*******************************************************************
//   file: DeleteOutlineDialog.h
//
// author: Adam Russell
//
//   mods: J H Stewman (7/25/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#ifndef DELETEOUTLINEDIALOG_H
#define DELETEOUTLINEDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "TraceWindow.h"

int getNumDeleteOutlineDialogReferences();


class DeleteOutlineDialog
{
	public:
		// Constructor
		//    Sets up all the gtk widgets but doesn't show the dialog.  
		//    show() must later be called to do that.
		DeleteOutlineDialog (TraceWindow *t, Contour **c);
	
		// Destructor
		~DeleteOutlineDialog();
		
		void show();
		void DeleteContour();

		// GTK callback functions

		friend gboolean on_delete_outline_delete_event(
						GtkWidget *widget,
						GdkEvent *event,
						gpointer userData);
		
		friend void on_delete_outline_ok_button_clicked(
						GtkButton *button,
						gpointer userData);

		friend void on_delete_outline_cancel_button_clicked(
						GtkButton *button,
						gpointer userData);

	private:
		GtkWidget *mDialog;
		TraceWindow *mTraceWin;
		Contour **mContour;
		GtkWidget* createDeleteOutlineDialog();
};
#endif
