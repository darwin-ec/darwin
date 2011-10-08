//*******************************************************************
//   file: AboutDialog.h
//
// author: Adam Russell
//
//   mods: J H Stewman (4/12/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

/*! @file AboutDialog.h */

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"


// int getNumAboutDialogReferences()
//
//! A global function that returns the number of existing AboutDialog objects.
//!
//!    Used by the MainWindow object to prevent opening more than one 
//!    AboutDialog at a time.
//
int getNumAboutDialogReferences();


//! Class for displaying DARWIN program/project "About" information.
//
//!    Only a single object of this class should exist at a time.  An AboutDialog 
//!    object is created in response to user selection of the MainWin "Help/About"
//!    menu item.  The resulting dialog displays information about the DARWIN
//!    Dolphin Photoidentification software -- current version, team members,
//!    web site, sources of support, etc -- until the user deletes the window
//!    or "clicks" the OK button.
//
class AboutDialog
{
	public:

		//  AboutDialog()
		//
		//! The class constructor.
		//!
		//!    Sets the value of class attributes, allocates and sets 
		//!    up all of the Gtk+ widgets, and increments the object instance counter.  
		//!    The show() member function must be called later to actually display 
		//!    this dialog's GUI.
		//!
		//! @param parentWindow is a link to the parent window,
		//!    so the dialog may be set transient_for the MainWin window
		//!
		//! @pre @em parentWindow != NULL
		//!
		//! @post AboutDialog::mParentWindow points to the MainWin window 
		//!    (@em *parentWindow), AboutDialog::mDialog points to the main 
		//!    GtkDialog created for this instance of the AboutDialog class, and
		//!    the global object instance counter has been incremented.
		//
		AboutDialog(GtkWidget *parentWindow);
		

		//  ~AboutDialog()
		//
		//! The class destructor.
		//!
		//!    Destroys the dialog.  Specifically, it frees all Gtk+ widgets 
		//!    referenced through the AboutDialog::mDialog member. Finally, 
		//!    the object instance counter is decremented.
		//
		~AboutDialog();


		//  show()
		//
		//! A normal member function that displays the dialog's GUI.
		//!
		//!    Simply draws the GTK+ dialog widgets on the screen.
		//!
		//! @pre The createAboutDialog() function must have been called previously.
		//
		void show();


		///////////////////// GTK+ callback functions //////////////////////


		//  on_aboutDialog_delete_event()
		//
		//! A callback function for window "delete" events.
		//!
		//!    A friend of the AboutDialog class that handles user deletion of the 
		//!    Gtk+ dialog window and deletes this instance of the class.
		//!
		//! @return TRUE (Gtk+ requires callback to return value but does not specify which.) 
		//
		friend gboolean on_aboutDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);


		//  on_aboutButtonOK_clicked()
		//
		//! A callback function for OK button "clicked" events.
		//!
		//!    A friend of the AboutDialog class that handles user "click" of 
		//!    the OK button. This initiates destruction of the Gtk+ dialog window 
		//!    and deletes this instance of the class.
		//
		friend void on_aboutButtonOK_clicked(
				GtkButton *button,
				gpointer userData);


	private:

		// Class pointers to the AboutDialog and parent widgets.
		//
		GtkWidget 
			*mDialog,       //!< Points to the AboutDialog window
			*mParentWindow; //!< Points to the parent window (the MainWin window) //***1.3


		//  createAboutDialog()
		//
		//! A member function to build the Gtk+ GUI for the AboutDialog.
		//!
		//!    Does the actual work of creating the AboutDialog widgets, registering
		//!    the callbacks, etc.
		//!
		//! @return A pointer to the GtkWidget that is the main GtkDialog for 
		//!    this instance of the AboutDialog class.
		//!
		//! @pre AboutDialog::mParentWindow must be set and must not be NULL
		//
		GtkWidget* createAboutDialog();
};

#endif
