//*******************************************************************
//   file: ErrorDialog.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include <gdk/gdkkeysyms.h>

#include "ErrorDialog.h"
#include "../support.h"
#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#include "../../pixmaps/ok.xpm"
#include "../../pixmaps/warning.xpm"

#ifdef HAVE_CONFIG_H
   #include <config.h>
#endif

using namespace std;

static int gNumReferences = 0;

//*******************************************************************
//
//
int getNumErrorDialogReferences()
{
	return gNumReferences;
}

//***1.3 - need to add a parentWindow widget so we can make these error messages
// MODAL and TRANSIENT_FOR the window that spawned them - JHS

//*******************************************************************
//
//
ErrorDialog::ErrorDialog()
	: mDialog(createErrorDialog(""))
{
	gNumReferences++;
}

//*******************************************************************
//
//
ErrorDialog::ErrorDialog(string errorMsg)
	: mDialog(createErrorDialog(errorMsg))
{
	gNumReferences++;
}

//*******************************************************************
//
//
ErrorDialog::ErrorDialog(Error e)
	: mDialog(createErrorDialog(e.errorString()))
{
	gNumReferences++;
}

//*******************************************************************
//
//
ErrorDialog::~ErrorDialog()
{
	if (NULL != mDialog)
		gtk_widget_destroy(mDialog);

	gNumReferences--;
}

//*******************************************************************
//
//
void ErrorDialog::show()
{
	gtk_widget_show(mDialog);
}

//*******************************************************************
//
//
GtkWidget* ErrorDialog::createErrorDialog(string errorMsg)
{
  GtkWidget *errorDialog;
  GtkWidget *errorVBox;
  GtkWidget *errorHBox;
  GtkWidget *errorPixmap;
  GtkWidget *errorLabel;
  GtkWidget *errorActionArea;
  GtkWidget *errorHButtonBox;
  guint errorButtonOK_key;
  GtkWidget *errorButtonOK;
  GtkAccelGroup *accel_group;
  GtkWidget *tmpLabel, *tmpBox, *tmpIcon;

  accel_group = gtk_accel_group_new ();

  errorDialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (errorDialog), "errorDialog", errorDialog);
  gtk_window_set_title (GTK_WINDOW (errorDialog), _("Error"));
  GTK_WINDOW (errorDialog)->type = WINDOW_DIALOG;
  gtk_window_set_position (GTK_WINDOW (errorDialog), GTK_WIN_POS_CENTER);
  gtk_window_set_policy (GTK_WINDOW (errorDialog), TRUE, TRUE, TRUE);
  gtk_window_set_wmclass(GTK_WINDOW(errorDialog), "darwin_error", "DARWIN");

  gtk_window_set_keep_above(GTK_WINDOW(errorDialog), TRUE); //***2.0 - keep on top
  gtk_window_set_modal(GTK_WINDOW(errorDialog),TRUE); //***1.85 - so focus is HERE

  errorVBox = GTK_DIALOG (errorDialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (errorDialog), "errorVBox", errorVBox);
  gtk_widget_show (errorVBox);

  errorHBox = gtk_hbox_new (FALSE, 2);
  gtk_widget_ref (errorHBox);
  gtk_object_set_data_full (GTK_OBJECT (errorDialog), "errorHBox", errorHBox,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (errorHBox);
  gtk_box_pack_start (GTK_BOX (errorVBox), errorHBox, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (errorHBox), 10);

  errorPixmap = create_pixmap_from_data(errorDialog, warning_xpm);
  gtk_widget_ref (errorPixmap);
  gtk_object_set_data_full (GTK_OBJECT (errorDialog), "errorPixmap", errorPixmap,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (errorPixmap);
  gtk_box_pack_start (GTK_BOX (errorHBox), errorPixmap, TRUE, TRUE, 0);

  if (errorMsg == "")
	  errorLabel = gtk_label_new(_("Undefined Error."));
  else
	  errorLabel = gtk_label_new(errorMsg.c_str());
  
  gtk_widget_ref (errorLabel);
  gtk_object_set_data_full (GTK_OBJECT (errorDialog), "errorLabel", errorLabel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (errorLabel);
  gtk_box_pack_start (GTK_BOX (errorHBox), errorLabel, FALSE, FALSE, 0);
  gtk_misc_set_padding (GTK_MISC (errorLabel), 35, 35);

  errorActionArea = GTK_DIALOG (errorDialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (errorDialog), "errorActionArea", errorActionArea);
  gtk_widget_show (errorActionArea);
  gtk_container_set_border_width (GTK_CONTAINER (errorActionArea), 10);

  errorHButtonBox = gtk_hbutton_box_new ();
  gtk_widget_ref (errorHButtonBox);
  gtk_object_set_data_full (GTK_OBJECT (errorDialog), "errorHButtonBox", errorHButtonBox,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (errorHButtonBox);
  gtk_box_pack_start (GTK_BOX (errorActionArea), errorHButtonBox, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (errorHButtonBox), GTK_BUTTONBOX_END);

  tmpBox = gtk_hbox_new(FALSE, 0);
  tmpIcon = create_pixmap_from_data(tmpBox, ok_xpm);
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
  gtk_widget_show(tmpIcon);
  tmpLabel = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
  gtk_widget_show(tmpLabel);
  gtk_widget_show(tmpBox);
  
  errorButtonOK = gtk_button_new();
  errorButtonOK_key = gtk_label_parse_uline (GTK_LABEL(tmpLabel),
                                   _("_OK"));
  gtk_widget_add_accelerator (errorButtonOK, "clicked", accel_group,
                              errorButtonOK_key, GDK_MOD1_MASK, (GtkAccelFlags)0);
  gtk_widget_ref (errorButtonOK);
  gtk_object_set_data_full (GTK_OBJECT (errorDialog), "errorButtonOK", errorButtonOK,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_container_add(GTK_CONTAINER(errorButtonOK), tmpBox);
  gtk_widget_show (errorButtonOK);
  gtk_container_add (GTK_CONTAINER (errorHButtonBox), errorButtonOK);
  GTK_WIDGET_SET_FLAGS (errorButtonOK, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (errorButtonOK, "clicked", accel_group,
                              GDK_o, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (errorButtonOK, "clicked", accel_group,
                              GDK_o, (GdkModifierType)0,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (errorButtonOK, "clicked", accel_group,
                              GDK_Escape, (GdkModifierType)0,
                              GTK_ACCEL_VISIBLE);

  gtk_signal_connect (GTK_OBJECT (errorDialog), "delete_event",
                      GTK_SIGNAL_FUNC (on_errorDialog_delete_event),
                      (void*)this);
  gtk_signal_connect(GTK_OBJECT(errorButtonOK), "clicked",
		  GTK_SIGNAL_FUNC(on_errorButtonOK_clicked),
		  (void*)this);

  gtk_window_add_accel_group (GTK_WINDOW (errorDialog), accel_group);

  return errorDialog;
}

//*******************************************************************
//
//
gboolean on_errorDialog_delete_event(
		GtkWidget *widget,
		GdkEvent *event,
		gpointer userData)
{
	if (NULL == userData)
		return FALSE;

	delete (ErrorDialog*)userData;
	
	return TRUE;
}

//*******************************************************************
//
//
void on_errorButtonOK_clicked(
		GtkButton *button,
		gpointer userData)
{
	if (NULL == userData)
		return;

	delete (ErrorDialog*)userData;
}
