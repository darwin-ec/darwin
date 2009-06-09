//*******************************************************************
//   file: AboutDialog.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (4/12/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

/*! @file AboutDialog.cxx */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#include "../support.h"
#include "AboutDialog.h"
#include "../../pixmaps/ok.xpm"
#include "../../pixmaps/logo_small.xpm"

#include <cstdio>


//! The number of existing AboutDialog objects.
//
static int gNumAboutDialogReferences = 0; 


//*******************************************************************
//
// getNumAboutDialogReferences()
//
//    Returns the number of currently existing AboutDialog objects.  
//
//    Used by the MainWindow object to prevent opening more than one 
//    AboutDialog at a time.
//
int getNumAboutDialogReferences()
{
	return gNumAboutDialogReferences;
}


//*******************************************************************
//
// AboutDialog::AboutDialog()
//
//    Creates the AboutDialog widget and increments reference counter.
//
AboutDialog::AboutDialog(GtkWidget *parentWindow)
: 
	mParentWindow(parentWindow) //***1.3 - so dialog will can be set transient_for MainWin
{
	mDialog = createAboutDialog(); //***1.3 - must follow intitialization of mParentWindow
	gNumAboutDialogReferences++;
}


//*******************************************************************
//
// AboutDialog::~AboutDialog()
//
//    Destroys the existing AboutDialog widgets and decrements the reference 
//    counter.
//
AboutDialog::~AboutDialog()
{
	if (NULL != mDialog)
		gtk_widget_destroy(mDialog);
	
	gNumAboutDialogReferences--;
}


//*******************************************************************
//
// void AboutDialog::show()
//
//    Shows the AboutDialog widget.
//
void AboutDialog::show()
{
	gtk_widget_show(mDialog);
}


//*******************************************************************
//
// GtkWidget* AboutDialog::createAboutDialog()
//
//    A member function to create the GTK+ Widgets for the AboutDialog.
//
GtkWidget* AboutDialog::createAboutDialog()
{
	GtkWidget *aboutDialog;
	GtkWidget *aboutHBox;
	GtkWidget *aboutVBoxMain;
	GtkWidget *aboutVBox;
	GtkWidget *aboutPixmap;
	GtkWidget *aboutLabelVersion;
	GtkWidget *aboutLabelCredits;
	GtkWidget *aboutActionArea;
	GtkWidget *aboutHButtonBox;
	GtkWidget *aboutButtonOK;
	GtkWidget *tmpBox, *tmpLabel, *tmpIcon;

	aboutDialog = gtk_dialog_new ();
	gtk_object_set_data (GTK_OBJECT(aboutDialog), "aboutDialog", aboutDialog);
	gtk_window_set_title (GTK_WINDOW (aboutDialog), _("About..."));
	GTK_WINDOW (aboutDialog)->type = WINDOW_DIALOG;
	gtk_window_set_position (GTK_WINDOW (aboutDialog), GTK_WIN_POS_CENTER);
	gtk_window_set_policy (GTK_WINDOW (aboutDialog), TRUE, TRUE, TRUE);
	gtk_window_set_wmclass(GTK_WINDOW(aboutDialog), "darwin_about", "DARWIN");

	gtk_window_set_modal(GTK_WINDOW(aboutDialog), TRUE); //***1.3
	gtk_window_set_transient_for(GTK_WINDOW(aboutDialog), GTK_WINDOW(mParentWindow));

	aboutVBoxMain = GTK_DIALOG(aboutDialog)->vbox;
	gtk_object_set_data (GTK_OBJECT (aboutDialog), "aboutVBoxMain", aboutVBoxMain);
	gtk_widget_show (aboutVBoxMain);

	aboutHBox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(aboutHBox);

	gtk_box_pack_start(GTK_BOX(aboutVBoxMain), aboutHBox, TRUE, TRUE, 0);

	aboutVBox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(aboutVBox);

	aboutPixmap = create_pixmap_from_data(aboutDialog, logo_small_xpm);
	gtk_widget_show(aboutPixmap);
	gtk_box_pack_start(GTK_BOX(aboutHBox), aboutPixmap, TRUE, TRUE, 0);
	
	gtk_box_pack_start(GTK_BOX(aboutHBox), aboutVBox, TRUE, TRUE, 0);
  
	char version[200];

#ifdef HAVE_SYS_UTSNAME_H

	utsname un;
	uname(&un);

	sprintf(
		  version,
		  "DARWIN\nBuild Version %s\n"
		  "http://darwin.eckerd.edu/\n\n"
		  "Running on: %s %s [%s]",
		  VERSION,
		  un.sysname,
		  un.release,
		  un.machine);
#else

	sprintf(
		  version,
		  "DARWIN\nBuild Version %s\nhttp://darwin.eckerd.edu/",
		  VERSION);
#endif

	aboutLabelVersion = gtk_label_new(version);
	gtk_widget_show (aboutLabelVersion);
	gtk_box_pack_start (GTK_BOX (aboutVBox), aboutLabelVersion, FALSE, FALSE, 0);

	aboutLabelCredits = gtk_label_new (
		  _("\n Research & Programming by:\n\n"
		    "   John H. Stewman\n\t (Project Founder / Faculty Advisor : 1993-2009)\n"
		    "   Kelly R. Debure\n\t (Faculty Advisor : 1997-2009)\n"
			"\n"
			"   Mark C. Allen (1994-1996)\n"
		    "   Rachel Stanley (1994)\n"
		    "   Daniel J. Wilkin (1995-1999)\n"
		    "   Andrew Cameron (1995)\n"
		    "   Zach Roberts (1998-1999)\n"
		    "   Adam Russell (2000-2002)\n"
		    "   Kristen McCoy (2000-2001)\n"
		    "   Henry Burroughs (2000)\n"
			"   Antonia Vassileva (2001-2002)\n"
		    "   Ramesh Madhusudan (2002)\n"
			"   Scott Hale (2005-2008)\n"
			"   Joshua Gregory (2006)\n"
			"   RJ Nowling (2007-2008)\n"
			"\n Financial Support from:\n\n"
			"   National Science Foundation\n"
			"     Grants: IIS-9980031 & DBI-0445126\n"
			"   National Marine Fisheries Service\n"
			"   Eckerd College\n"));

	gtk_widget_show (aboutLabelCredits);
	gtk_box_pack_start (GTK_BOX (aboutVBox), aboutLabelCredits, FALSE, FALSE, 0);

	aboutActionArea = GTK_DIALOG (aboutDialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (aboutDialog), "aboutActionArea", aboutActionArea);
	gtk_widget_show (aboutActionArea);
	gtk_container_set_border_width (GTK_CONTAINER (aboutActionArea), 10);

	aboutHButtonBox = gtk_hbutton_box_new ();
	gtk_widget_show (aboutHButtonBox);
	gtk_box_pack_start (GTK_BOX (aboutActionArea), aboutHButtonBox, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (aboutHButtonBox), GTK_BUTTONBOX_END);
	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, ok_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new(_("OK"));
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);
  
	aboutButtonOK = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(aboutButtonOK), tmpBox);
	gtk_widget_show (aboutButtonOK);
	gtk_container_add (GTK_CONTAINER (aboutHButtonBox), aboutButtonOK);
	GTK_WIDGET_SET_FLAGS (aboutButtonOK, GTK_CAN_DEFAULT);

	gtk_signal_connect (GTK_OBJECT (aboutDialog), "delete_event",
	                  GTK_SIGNAL_FUNC (on_aboutDialog_delete_event),
	                  (void*)this);
	gtk_signal_connect (GTK_OBJECT (aboutButtonOK), "clicked",
	                  GTK_SIGNAL_FUNC (on_aboutButtonOK_clicked),
	                  (void*)this);

	gtk_widget_grab_default (aboutButtonOK);
	return aboutDialog;
}


//*******************************************************************
//
// gboolean on_aboutDialog_delete_event()
//
//    A callback function to process window "delete" events.
//
gboolean on_aboutDialog_delete_event(
		GtkWidget *widget,
		GdkEvent *event,
		gpointer userData)
{
	if (NULL == userData)
		return FALSE;

	delete (AboutDialog*)userData;
	
	return TRUE;
}


//*******************************************************************
//
// void on_aboutButtonOK_clicked()
//
//    A callback function to process OK button "click" events
//
void on_aboutButtonOK_clicked(
		GtkButton *button,
		gpointer userData)
{
	if (NULL == userData)
		return;

	delete (AboutDialog*)userData;
}
