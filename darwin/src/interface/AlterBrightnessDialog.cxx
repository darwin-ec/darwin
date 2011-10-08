//*******************************************************************
//   file: AlterBrightnessDialog.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (4/12/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************


#include "../support.h"
#include "AlterBrightnessDialog.h"
#include "../Error.h"
#include "../image_processing/transform.h"
#include "../image_processing/ImageMod.h" //***1.8
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "../../pixmaps/ok.xpm"
#include "../../pixmaps/cancel.xpm"

using namespace std;


// number of currently open AlterBrightnessDialog widgets
static int gNumReferences = 0;


//*******************************************************************
//
// int getNumAlterBrightnessDialogReferences()
//
//    Returns number of currently open AlterBrigtnessDialog widgets.  
//    Used by TraceWindow to prevent opening more than one 
//    AlterBrigtnessDialog at a time.
//
int getNumAlterBrightnessDialogReferences()
{
	return gNumReferences;
}


//*******************************************************************
//
// AlterBrightnessDialog::AlterBrightnessDialog()
//
//    Creates the AlterBrightnessDialog widget and a copy of the
//    original image *i.  Sets links to original image and TraceWindow.
//    Also, increments reference counter.
//
AlterBrightnessDialog::AlterBrightnessDialog(
		TraceWindow *t,
		ColorImage **i
)
	: mDialog(createAlterBrightnessDialog()),
	  mTraceWin(t),
	  mOriginalImage(new ColorImage(*i)),
	  mImage(i)
{
	if (NULL == t)
		throw EmptyArgumentError("AlterBrightnessDialog ctor");

	gNumReferences++;
}


//*******************************************************************
//
// AlterBrightnessDialog::~AlterBrightnessDialog()
//
//    Destroys the existing AlterBrightnessDialog widget and the copy
//    of the original image.  Also, decrements reference counter.
//
AlterBrightnessDialog::~AlterBrightnessDialog()
{
	delete mOriginalImage;
	if (NULL != mDialog)
      gtk_widget_destroy(mDialog);
	gNumReferences--;
}


//*******************************************************************
//
// void AlterBrightnessDialog::show()
//
//    Actually shows the AlterBrightnessDialog widget
//
void AlterBrightnessDialog::show()
{
	gtk_widget_show(mDialog);
}


//*******************************************************************
//
// void AlterBrightnessDialog::updateBrightness(int increment)
//
//    Private class function to change the image brightness by
//    specified increment.  It calls the alterBrightness() function
//    in the transform.h library to perform the image transformation.
//    An increment of 0 restores image to original.
//    Finally, the TraceWindow is forced to redisplay the image.
//
void AlterBrightnessDialog::updateBrightness(int increment)
{
	delete *mImage;

	if (0 == increment)
		*mImage = new ColorImage(mOriginalImage);
	else
		*mImage = alterBrightness(mOriginalImage, increment);

	mTraceWin->zoomUpdate(false);
}


//*******************************************************************
//
// GtkWidget* AlterBrightnessDialog::createAlterBrightnessDialog()
//
//    Friend function to create GTK Widget for AlterBrightnessDialog
//
GtkWidget* AlterBrightnessDialog::createAlterBrightnessDialog()
{
    GtkWidget *dialog_alter_brightness;
    GtkWidget *dialog_vbox1;
    GtkWidget *vbox1;
    GtkWidget *label1;
    GtkWidget *hscale1;
    GtkWidget *dialog_action_area1;
    GtkWidget *hbuttonbox1;
    GtkWidget *button1;
    GtkWidget *button2;
    GtkWidget *button3;
    GtkWidget *sliderhbox;
    GtkWidget *tmpLabel, *tmpBox, *tmpIcon;

    dialog_alter_brightness = gtk_dialog_new();
    gtk_object_set_data(GTK_OBJECT(dialog_alter_brightness),
			"dialog_alter_brightness",
			dialog_alter_brightness);
    gtk_widget_set_usize(dialog_alter_brightness, 400, 150);
    gtk_window_set_title(GTK_WINDOW(dialog_alter_brightness),
			 _("Alter Brightness"));
    GTK_WINDOW(dialog_alter_brightness)->type = WINDOW_DIALOG;
    gtk_window_set_policy(GTK_WINDOW(dialog_alter_brightness), TRUE, TRUE,
			  FALSE);
    gtk_window_set_wmclass(GTK_WINDOW(dialog_alter_brightness), "darwin_alter", "DARWIN");

    dialog_vbox1 = GTK_DIALOG(dialog_alter_brightness)->vbox;
    gtk_object_set_data(GTK_OBJECT(dialog_alter_brightness),
			"dialog_vbox1", dialog_vbox1);
    gtk_widget_show(dialog_vbox1);

    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox1);
    gtk_box_pack_start(GTK_BOX(dialog_vbox1), vbox1, TRUE, TRUE, 0);

    label1 =
	gtk_label_new(_
		      ("Move the slider bar to change the brightness.\n"));
    gtk_widget_show(label1);
    gtk_box_pack_start(GTK_BOX(vbox1), label1, FALSE, FALSE, 10);
    gtk_widget_set_usize(label1, 265, 30);

    sliderhbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), sliderhbox, TRUE, FALSE, 0);
    gtk_widget_show(sliderhbox);

    label1 = gtk_label_new("Darker");
    gtk_widget_show(label1);
    gtk_box_pack_start(GTK_BOX(sliderhbox), label1, FALSE, FALSE, 10);

    hscale1 =
	gtk_hscale_new(GTK_ADJUSTMENT
		       (gtk_adjustment_new(0, -255, 255, 5, 5, 0)));
    gtk_scale_set_digits(GTK_SCALE(hscale1), 0);
    gtk_scale_set_draw_value(GTK_SCALE(hscale1), FALSE);
    gtk_range_set_update_policy(GTK_RANGE(hscale1),
				GTK_UPDATE_DISCONTINUOUS);
    mAdjustment = gtk_range_get_adjustment(GTK_RANGE(hscale1));
    gtk_widget_show(hscale1);
    gtk_box_pack_start(GTK_BOX(sliderhbox), hscale1, TRUE, TRUE, 0);

    label1 = gtk_label_new("Brighter");
    gtk_widget_show(label1);
    gtk_box_pack_start(GTK_BOX(sliderhbox), label1, FALSE, FALSE, 10);

    dialog_action_area1 = GTK_DIALOG(dialog_alter_brightness)->action_area;
    gtk_object_set_data(GTK_OBJECT(dialog_alter_brightness),
			"dialog_action_area1", dialog_action_area1);
    gtk_widget_show(dialog_action_area1);
    gtk_container_set_border_width(GTK_CONTAINER(dialog_action_area1), 10);

    hbuttonbox1 = gtk_hbutton_box_new();
    gtk_widget_show(hbuttonbox1);
    gtk_box_pack_start(GTK_BOX(dialog_action_area1), hbuttonbox1, TRUE,
		       TRUE, 0);

  tmpBox = gtk_hbox_new(FALSE, 0);
  tmpIcon = create_pixmap_from_data(tmpBox, ok_xpm);
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
  gtk_widget_show(tmpIcon);
  tmpLabel = gtk_label_new("OK");
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
  gtk_widget_show(tmpLabel);
  gtk_widget_show(tmpBox);

    button1 = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(button1), tmpBox);
    gtk_widget_show(button1);
    gtk_container_add(GTK_CONTAINER(hbuttonbox1), button1);

    button3 = gtk_button_new_with_label("Reset Image");
    gtk_widget_show(button3);
    gtk_container_add(GTK_CONTAINER(hbuttonbox1), button3);

  tmpBox = gtk_hbox_new(FALSE, 0);
  tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
  gtk_widget_show(tmpIcon);
  tmpLabel = gtk_label_new("Cancel");
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
  gtk_widget_show(tmpLabel);
  gtk_widget_show(tmpBox);

    button2 = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(button2), tmpBox);
    gtk_widget_show(button2);
    gtk_container_add(GTK_CONTAINER(hbuttonbox1), button2);

    //The signals:

    gtk_signal_connect(GTK_OBJECT(mAdjustment), "value_changed",
		       GTK_SIGNAL_FUNC(on_alter_brightness_slider_moved),
		       (void *) this);

    gtk_signal_connect(GTK_OBJECT(dialog_alter_brightness), "delete_event",
		       GTK_SIGNAL_FUNC(on_alter_brightness_delete_event),
		       (void *) this);

    gtk_signal_connect(GTK_OBJECT(button1), "clicked",
		       GTK_SIGNAL_FUNC
		       (on_alter_brightness_button_ok_clicked),
		       (void *) this);

    gtk_signal_connect(GTK_OBJECT(button2), "clicked",
		       GTK_SIGNAL_FUNC
		       (on_alter_brightness_button_cancel_clicked), (void *) this);

    gtk_signal_connect(GTK_OBJECT(button3), "clicked",
		       GTK_SIGNAL_FUNC
		       (on_alter_brightness_button_reset_clicked),
		       (void *) this);
    return dialog_alter_brightness;
}


//*******************************************************************
//
// gboolean on_alter_brightness_delete_event(...)
//
//    Friend function to process "delete" events
//  
gboolean on_alter_brightness_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	AlterBrightnessDialog *dlg = (AlterBrightnessDialog *) userData;

	if (NULL == dlg)
		return FALSE;
	
	dlg->updateBrightness(0);

	delete dlg;

	return TRUE;
}


//*******************************************************************
//
// void on_alter_brightness_button_ok_clicked(...)
//
//    Friend function to process "OK button" events
//
void on_alter_brightness_button_ok_clicked(
	GtkButton *button,
	gpointer userData)
{
	AlterBrightnessDialog *dlg = (AlterBrightnessDialog *) userData;

	if (NULL == dlg)
		return;

	dlg->updateBrightness((int) round(dlg->mAdjustment->value));

	//***1.8 - add modification to list
	ImageMod imod(ImageMod::IMG_brighten,
	              (int) round(dlg->mAdjustment->value));
	dlg->mTraceWin->theImageMods().add(imod);

	delete dlg;
}


//*******************************************************************
//
// void on_alter_brightness_button_cancel_clicked(...)
//
//    Friend function to process "CANCEL button" events.  Brightness
//    adjustment is reset to zero.
//
void on_alter_brightness_button_cancel_clicked(
	GtkButton *button,
	gpointer userData)
{
	AlterBrightnessDialog *dlg = (AlterBrightnessDialog *) userData;

	if (NULL == dlg)
		return;

	dlg->updateBrightness(0);

	delete dlg;
}


//*******************************************************************
//
// void on_alter_brightness_slider_moved(...)
//
//    Friend function to process "change of brightness" events. Adjustment
//    amount is set to slider value.
//
void on_alter_brightness_slider_moved(
	GtkButton *button,
	gpointer userData)
{
	AlterBrightnessDialog *dlg = (AlterBrightnessDialog *) userData;

	if (NULL == dlg)
		return;

	dlg->updateBrightness((int) round(dlg->mAdjustment->value));
}


//*******************************************************************
//
// void on_alter_brightness_button_reset_clicked(...)
//
//    Friend function to handle "reset brightness alteration button" events.
//    Adjustment and slider widget are reset to zero.
//
void on_alter_brightness_button_reset_clicked(
	GtkButton *button,
	gpointer userData)
{
	AlterBrightnessDialog *dlg = (AlterBrightnessDialog *) userData;

	if (NULL == dlg)
		return;

	dlg->updateBrightness(0);
	gtk_adjustment_set_value(dlg->mAdjustment, 0);
}
