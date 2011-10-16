//*******************************************************************
//   file: DeleteOutlineDialog.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (7/25/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include "DeleteOutlineDialog.h"

#include "../support.h"
#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#include "../../pixmaps/ok.xpm"
#include "../../pixmaps/cancel.xpm"

#ifdef HAVE_CONFIG_H
#include <config.h> 
#endif

using namespace std;


// number of currently existing DeleteOutlineDialog widgets
static int gNumReferences = 0;


//*******************************************************************
//
// int getNumDeleteOutlineDialogReferences()
//
//    Returns number of currently existing DeleteOutlineDialog widgets
//
int getNumDeleteOutlineDialogReferences()
{
	return gNumReferences;
}


//*******************************************************************
//
// DeleteOutlineDialog::DeleteOutlineDialog (TraceWindow *t, Contour **c)
//
//    CONSTRUCTOR
//
DeleteOutlineDialog::DeleteOutlineDialog (TraceWindow *t, Contour **c)
	:mDialog(createDeleteOutlineDialog()),
	mTraceWin (t),
	mContour(c)
{

	if (NULL == t)
		throw EmptyArgumentError("DeleteOutlineDialog ctor");
	gNumReferences++;
}


//*******************************************************************
//
// DeleteOutlineDialog::~DeleteOutlineDialog()
//
//    DESTRUCTOR
//
DeleteOutlineDialog::~DeleteOutlineDialog()
{
	if (NULL!= mDialog)
		gtk_widget_destroy(mDialog);

        gNumReferences--;
}


//*******************************************************************
//
// void DeleteOutlineDialog::show()
//
//    Member function to actually "show" the DeleteOutlineDialog.
//
void DeleteOutlineDialog::show()
{
	gtk_widget_show(mDialog);
}


//*******************************************************************
//
// void DeleteOutlineDialog::DeleteContour()
//
//    Simply calls TraceWindow::traceReset() to free dynamic memory
//    used in current Contour prior to retracing of fin.
//
void DeleteOutlineDialog::DeleteContour()
{
	mTraceWin->traceReset(); //***006FC
}


//*******************************************************************
//
// GtkWidget* DeleteOutlineDialog::createDeleteOutlineDialog()
//
//    Member function to create and arrange all widgets comprising the
//    DeleteOutlineDialog.
//
GtkWidget* DeleteOutlineDialog::createDeleteOutlineDialog()
{
	GtkWidget *delete_outline_dialog;
	GtkWidget *dialog_Vbox;
	GtkWidget *dialog_Hbox;
	GtkWidget *dialog_action_area;
	GtkWidget *OK_button;
	GtkWidget *Cancel_button;
	GtkWidget *dialog_label;
	GtkWidget *dialog_Hbuttonbox;
	GtkWidget *tmpLabel, *tmpBox, *tmpIcon;

	delete_outline_dialog= gtk_dialog_new();
	//***1.4TW - force answer to question about existing outline BEFORE alowing any more
	// interaction with TraceWindow using PENCIL
	gtk_window_set_modal(GTK_WINDOW(delete_outline_dialog), TRUE); //***1.4TW
	gtk_object_set_data (GTK_OBJECT (delete_outline_dialog),
				"delete_outline_dialog",
				delete_outline_dialog);
	gtk_window_set_title(GTK_WINDOW(delete_outline_dialog), _("Delete Outline"));
	GTK_WINDOW (delete_outline_dialog)->type = WINDOW_DIALOG;
	gtk_window_set_position (GTK_WINDOW (delete_outline_dialog), GTK_WIN_POS_MOUSE);
	gtk_window_set_policy (GTK_WINDOW (delete_outline_dialog), TRUE, TRUE, TRUE);
	gtk_window_set_wmclass(GTK_WINDOW(delete_outline_dialog), "darwin_prompt", "DARWIN");

	dialog_Vbox= GTK_DIALOG (delete_outline_dialog)->vbox;
	gtk_object_set_data (GTK_OBJECT (delete_outline_dialog), "dialog_Vbox", dialog_Vbox);
	gtk_widget_show (dialog_Vbox);

	dialog_Hbox = gtk_hbox_new (FALSE, 2);
	gtk_widget_ref (dialog_Hbox);
  	gtk_object_set_data_full (GTK_OBJECT (delete_outline_dialog), 
			"dialog_Hbox", dialog_Hbox,(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (dialog_Hbox);
	gtk_box_pack_start (GTK_BOX (dialog_Vbox), dialog_Hbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_Hbox), 10);


	dialog_label= gtk_label_new(_("Delete the current outline of the fin?\n"));
	gtk_widget_ref (dialog_label);
  	gtk_object_set_data_full (GTK_OBJECT (delete_outline_dialog), 
			"dialog_label", dialog_label, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (dialog_label);
	gtk_box_pack_start(GTK_BOX(dialog_Hbox), dialog_label, FALSE, FALSE, 0);

	gtk_misc_set_padding (GTK_MISC (dialog_label), 35, 35);

	dialog_action_area = GTK_DIALOG (delete_outline_dialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (delete_outline_dialog), "dialog_action_area", dialog_action_area);
	gtk_widget_show(dialog_action_area);
	gtk_container_set_border_width(GTK_CONTAINER(dialog_action_area), 10);

	dialog_Hbuttonbox = gtk_hbutton_box_new ();
	gtk_widget_ref (dialog_Hbuttonbox);
 	gtk_object_set_data_full (GTK_OBJECT (dialog_action_area), 
			"dialog_Hbuttonbox", dialog_Hbuttonbox,(GtkDestroyNotify) gtk_widget_unref);

	gtk_widget_show (dialog_Hbuttonbox);
	gtk_box_pack_start(GTK_BOX(dialog_action_area), dialog_Hbuttonbox, TRUE,TRUE, 0);

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, ok_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("OK");
 	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
  	gtk_widget_show(tmpBox);

	OK_button =  gtk_button_new();
	gtk_widget_ref (OK_button);
  	gtk_object_set_data_full (GTK_OBJECT (delete_outline_dialog), 
			"OK_button", OK_button, (GtkDestroyNotify) gtk_widget_unref);
	gtk_container_add(GTK_CONTAINER(OK_button), tmpBox);
	gtk_widget_show(OK_button);
	gtk_container_add(GTK_CONTAINER(dialog_Hbuttonbox), OK_button);

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
  	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
  	gtk_widget_show(tmpIcon);
  	tmpLabel = gtk_label_new("Cancel");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
 	gtk_widget_show(tmpLabel);
  	gtk_widget_show(tmpBox);

	Cancel_button= gtk_button_new();
	gtk_widget_ref (Cancel_button);
	gtk_object_set_data_full (GTK_OBJECT (delete_outline_dialog), 
			"Cancel_button", Cancel_button, (GtkDestroyNotify) gtk_widget_unref);
	gtk_container_add(GTK_CONTAINER(Cancel_button), tmpBox);
	gtk_widget_show(Cancel_button);
	gtk_container_add(GTK_CONTAINER(dialog_Hbuttonbox), Cancel_button);

	// widget signals
	gtk_signal_connect(GTK_OBJECT(delete_outline_dialog), "delete_event",
	                   GTK_SIGNAL_FUNC (on_delete_outline_delete_event),
	                   (void *) this);


	gtk_signal_connect(GTK_OBJECT(OK_button), "clicked",
                       GTK_SIGNAL_FUNC (on_delete_outline_ok_button_clicked),
	                   (void *) this);

	gtk_signal_connect(GTK_OBJECT(Cancel_button), "clicked",
                       GTK_SIGNAL_FUNC(on_delete_outline_cancel_button_clicked),
	                   (void *) this);

	return delete_outline_dialog;
}


//*******************************************************************
//
// gboolean on_delete_outline_delete_event (...)
//
//    Friend function to handle "delete" events
//
gboolean on_delete_outline_delete_event (GtkWidget *widget,
                        		GdkEvent *event,
                        		gpointer userData)
{

	DeleteOutlineDialog *dialog= (DeleteOutlineDialog *) userData;

	if (NULL ==dialog)
		return FALSE;
	delete dialog;
	return TRUE;
}


//*******************************************************************
//
// void on_delete_outline_ok_button_clicked(...)
//
//    Friend function to process"OK" events
//
void on_delete_outline_ok_button_clicked(
        GtkButton *button,
        gpointer userData)
{

	DeleteOutlineDialog *dialog= (DeleteOutlineDialog *) userData;
	if (NULL==dialog)
		return;
	dialog->DeleteContour ();
	delete dialog;
}


//*******************************************************************
//
// void on_delete_outline_cancel_button_clicked(...)
//
//    Friend function to process "cancel" event
//
void on_delete_outline_cancel_button_clicked(
        GtkButton *button,
        gpointer userData)
{

	DeleteOutlineDialog *dialog= (DeleteOutlineDialog *) userData;

	if (NULL==dialog)
                return;
	delete dialog;
}
