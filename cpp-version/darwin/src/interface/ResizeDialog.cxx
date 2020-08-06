//*******************************************************************
//   file: ResizeDialog.cxx
//
// author: Adam Russell?
//
//   mods: J H Stewman from Willem van Schaik's original code
//
// This code is heavily based on code by Henry Burroughs.
//
//*******************************************************************

#include <gdk/gdkkeysyms.h>
#include "../support.h"
#include "ResizeDialog.h"
#include "../image_processing/transform.h"
#include "../Error.h"
#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#include "../../pixmaps/cancel.xpm"
#include "../../pixmaps/ok.xpm"

gboolean on_resize_delete_event(
			GtkWidget *widget,
			GdkEvent *event,
			gpointer userData);
		
void on_resize_button_ok_clicked(
	GtkButton *button,
	gpointer userData);

void on_resize_button_cancel_clicked(
	GtkButton *button,
	gpointer userData);

void on_resize_button_reset_clicked(
	GtkButton *button,
	gpointer userData);

void on_resize_checkbutton_aspect_toggled(
	GtkCheckButton *button,
	gpointer userData);

void on_resize_options_scale_menu_done(
	GtkMenuShell *menushell,
	gpointer userData);

void on_resize_spinbutton_height_changed(
	GtkEditable *adjustment,
	gpointer userData);

void on_resize_spinbutton_width_changed(
	GtkEditable *adjustment,
	gpointer userData);

void on_resize_option_pixel_activated(
	GtkMenuItem *menuitem,
	gpointer userData);

void on_resize_option_percent_activated(
	GtkMenuItem *menuitem,
	gpointer userData);

using namespace std;

static int gNumReferences = 0;

int getNumResizeDialogReferences()
{
	return gNumReferences;
}

ResizeDialog::ResizeDialog(
		TraceWindow *t,
		ColorImage **i
)
	: mDialog(createResizeDialog((*i)->getNumRows(), (*i)->getNumCols())),
	  mCheckButton(NULL),
	  mOptionMenu(NULL),
	  mHeightSpinButton(NULL),
	  mWidthSpinButton(NULL),
	  mHeightAdjustment(NULL),
	  mWidthAdjustment(NULL),
	  mPixelChecked(true),
	  mHeightChanged(true),
	  mPixelLast(true),
	  mSpecialHeight(-1),
	  mSpecialWidth(-1),
	  mImage(i),
	  mTraceWin(t)
{
	if (NULL == t || NULL == i)
		throw EmptyArgumentError("ResizeDialog ctor");

	gNumReferences++;
}

ResizeDialog::~ResizeDialog()
{
	gtk_widget_destroy(mDialog);
	gNumReferences--;
}

void ResizeDialog::show()
{
	gtk_widget_show(mDialog);
}

void ResizeDialog::updateResizeOptions(
		int *newHeight,
		int *newWidth,
		bool aspectChecked)
{
	int origWidth = (*mImage)->getNumCols();
	int origHeight = (*mImage)->getNumRows();

	int width = *newWidth;
	int height = *newHeight;

	//pixels to percent:
	if (mPixelLast == true && mPixelChecked == false) {
		height = (int) round((float) height / origHeight * 100.0);
		width = (int) round((float) width / origWidth * 100.0);
		if (aspectChecked == true) {
			if (mHeightChanged == true)
				width = (int) round(origWidth * (float) height / origHeight);

			else
				height = (int) round(origHeight * (float) width / origWidth);
		}

		*newWidth = width;
		*newHeight = height;

		mPixelLast = false;
	}
	
	//percent to pixels:
	if (mPixelLast == false && mPixelChecked == true) {
		height = (int) round((float) height / 100.0 * origHeight);
		width = (int) round((float) width / 100.0 * origWidth);

		if (aspectChecked == true) {
			if (mHeightChanged == true)
				width = (int) round(origWidth * (float) height / origHeight);

			else
				height = (int) round(origHeight * (float) width / origWidth);
		}
			
		*newWidth = width;
		*newHeight = height;	

		mPixelLast = true;
	} else {
		if (aspectChecked == true) {
			if (mHeightChanged == true)
				width = (int) round(origWidth * (float) height / origHeight);

			else
				height = (int) round(origHeight * (float) width / origWidth);
		}

		*newWidth = width;
		*newHeight = height;
	}
}

GtkWidget* ResizeDialog::createResizeDialog(int height, int width)
{
    GtkWidget *dialog_resize;
    GtkWidget *resize_frame;
    GtkWidget *vbox1;
    GtkWidget *resize_hbox_height_row;
    GtkWidget *resize_label_height;
    GtkObject *resize_spinbutton_height_adj;
    GtkWidget *resize_spinbutton_height;
    GtkWidget *resize_label_scale;
    GtkWidget *resize_options_scale;
    GtkWidget *resize_options_scale_menu;
    GtkWidget *glade_menuitem;
    GtkWidget *resize_hbox_width_row;
    GtkWidget *resize_label_width;
    GtkObject *resize_spinbutton_width_adj;
    GtkWidget *resize_spinbutton_width;
    guint resize_checkbutton_aspect_key;
    GtkWidget *resize_checkbutton_aspect;
    GtkWidget *buttonbox;
    guint resize_ok_button_key;
    GtkWidget *resize_ok_button;
    guint resize_reset_button_key;
    GtkWidget *resize_reset_button;
    guint resize_cancel_button_key;
    GtkWidget *resize_cancel_button;
    GtkAccelGroup *accel_group;
    GtkTooltips *tooltips;
    GtkWidget *tmpBox, *tmpLabel, *tmpIcon;

    tooltips = gtk_tooltips_new();

    accel_group = gtk_accel_group_new();

    dialog_resize = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_object_set_data(GTK_OBJECT(dialog_resize), "dialog_resize",
			dialog_resize);
    gtk_window_set_title(GTK_WINDOW(dialog_resize), _("Resize Image"));
    gtk_window_set_policy(GTK_WINDOW(dialog_resize), FALSE, FALSE, FALSE);
    gtk_window_set_wmclass(GTK_WINDOW(dialog_resize), "darwin_resize", "DARWIN");

    resize_frame = gtk_frame_new(_("Resize Image"));
    gtk_widget_show(resize_frame);
    gtk_container_add(GTK_CONTAINER(dialog_resize), resize_frame);
    gtk_container_set_border_width(GTK_CONTAINER(resize_frame), 1);

    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox1);
    gtk_container_add(GTK_CONTAINER(resize_frame), vbox1);

    resize_hbox_height_row = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(resize_hbox_height_row);
    gtk_box_pack_start(GTK_BOX(vbox1), resize_hbox_height_row,
		       TRUE, TRUE, 0);

    resize_label_height = gtk_label_new(_("Height:"));
    gtk_widget_show(resize_label_height);
    gtk_box_pack_start(GTK_BOX(resize_hbox_height_row), resize_label_height,
		       FALSE, FALSE, 10);

    resize_spinbutton_height_adj = gtk_adjustment_new(height, 1, 2000,
						      1, 10, 10);
    resize_spinbutton_height = gtk_spin_button_new(GTK_ADJUSTMENT
						   (resize_spinbutton_height_adj),
						   1, 0);
    gtk_widget_show(resize_spinbutton_height);
    gtk_box_pack_start(GTK_BOX(resize_hbox_height_row), resize_spinbutton_height,
		       FALSE, FALSE, 0);
    gtk_widget_set_usize(resize_spinbutton_height, 100, -2);
    gtk_tooltips_set_tip(tooltips, resize_spinbutton_height,
			 _("Adjust the height of the image"), NULL);

    resize_label_scale = gtk_label_new(_("Scale:"));
    gtk_widget_show(resize_label_scale);
    gtk_box_pack_start(GTK_BOX(resize_hbox_height_row), resize_label_scale,
		       FALSE, FALSE, 18);

    resize_options_scale = gtk_option_menu_new();
    gtk_widget_show(resize_options_scale);
    gtk_box_pack_start(GTK_BOX(resize_hbox_height_row), resize_options_scale,
		       FALSE, FALSE, 0);
    gtk_tooltips_set_tip(tooltips, resize_options_scale,
			 _("Image scaling options"), NULL);
    resize_options_scale_menu = gtk_menu_new();
    glade_menuitem = gtk_menu_item_new_with_label(_("Pixels"));
    gtk_widget_show(glade_menuitem);
    gtk_menu_append(GTK_MENU(resize_options_scale_menu), glade_menuitem);
    //put signal connecting the "pixels":
    gtk_signal_connect(GTK_OBJECT(glade_menuitem),"activate",
		       GTK_SIGNAL_FUNC(on_resize_option_pixel_activated),
		       (void *)this);
    glade_menuitem = gtk_menu_item_new_with_label(_("%"));
    gtk_widget_show(glade_menuitem);
    gtk_menu_append(GTK_MENU(resize_options_scale_menu), glade_menuitem);
    //put signal connecting "%" here:
    gtk_signal_connect(GTK_OBJECT(glade_menuitem),"activate",
		       GTK_SIGNAL_FUNC(on_resize_option_percent_activated),
		       (void *)this);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(resize_options_scale),
			     resize_options_scale_menu);

    resize_hbox_width_row = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(resize_hbox_width_row);
    gtk_box_pack_start(GTK_BOX(vbox1), resize_hbox_width_row, TRUE, TRUE, 0);

    resize_label_width = gtk_label_new(_("Width:"));
    gtk_widget_show(resize_label_width);
    gtk_box_pack_start(GTK_BOX(resize_hbox_width_row), resize_label_width,
		       FALSE, FALSE, 12);

    resize_spinbutton_width_adj = gtk_adjustment_new(width, 1, 2000, 1, 10, 10);
    resize_spinbutton_width = gtk_spin_button_new(GTK_ADJUSTMENT
						  (resize_spinbutton_width_adj),
						  1, 0);
    gtk_widget_show(resize_spinbutton_width);
    gtk_box_pack_start(GTK_BOX(resize_hbox_width_row), resize_spinbutton_width,
		       FALSE, FALSE, 0);
    gtk_widget_set_usize(resize_spinbutton_width, 100, -2);
    gtk_tooltips_set_tip(tooltips, resize_spinbutton_width,
			 _("Adjust the width of the image"), NULL);

    resize_checkbutton_aspect = gtk_check_button_new_with_label("");
    resize_checkbutton_aspect_key = gtk_label_parse_uline(GTK_LABEL
							  (GTK_BIN
							  (resize_checkbutton_aspect)->child),
							  _("_Keep aspect ratio"));
    gtk_widget_add_accelerator(resize_checkbutton_aspect, "clicked", accel_group,
			       resize_checkbutton_aspect_key, GDK_MOD1_MASK, (GtkAccelFlags)0);
    gtk_widget_show(resize_checkbutton_aspect);
    gtk_box_pack_start(GTK_BOX(resize_hbox_width_row), resize_checkbutton_aspect,
		       FALSE, FALSE, 16);
    gtk_widget_set_usize(resize_checkbutton_aspect, 111, -2);
    gtk_widget_add_accelerator(resize_checkbutton_aspect, "clicked", accel_group,
			       GDK_k, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(resize_checkbutton_aspect), TRUE);

    buttonbox = gtk_hbutton_box_new();
    gtk_widget_show(buttonbox);
    gtk_box_pack_start(GTK_BOX(vbox1), buttonbox, TRUE, TRUE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonbox), GTK_BUTTONBOX_END);

  tmpBox = gtk_hbox_new(FALSE, 0);
  tmpIcon = create_pixmap_from_data(tmpBox, ok_xpm);
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
  gtk_widget_show(tmpIcon);
  tmpLabel = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
  gtk_widget_show(tmpLabel);
  gtk_widget_show(tmpBox);

    resize_ok_button = gtk_button_new();
    resize_ok_button_key = gtk_label_parse_uline(GTK_LABEL(tmpLabel),
						 _("_OK"));
    gtk_widget_add_accelerator(resize_ok_button, "clicked", accel_group,
			       resize_ok_button_key, GDK_MOD1_MASK, (GtkAccelFlags)0);
    gtk_container_add(GTK_CONTAINER(resize_ok_button), tmpBox);
    gtk_widget_show(resize_ok_button);
    gtk_container_add(GTK_CONTAINER(buttonbox), resize_ok_button);
    GTK_WIDGET_SET_FLAGS(resize_ok_button, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, resize_ok_button,
			 _("Accept image changes"), NULL);
    gtk_widget_add_accelerator(resize_ok_button, "clicked", accel_group,
			       GDK_o, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

    resize_reset_button = gtk_button_new_with_label("");
    resize_reset_button_key = gtk_label_parse_uline(GTK_LABEL(GTK_BIN
						    (resize_reset_button)->child),
						    _("_Reset"));
    gtk_widget_add_accelerator(resize_reset_button, "clicked", accel_group,
			       resize_reset_button_key, GDK_MOD1_MASK, (GtkAccelFlags)0);
    gtk_widget_show(resize_reset_button);
    gtk_container_add(GTK_CONTAINER(buttonbox), resize_reset_button);
    GTK_WIDGET_SET_FLAGS(resize_reset_button, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, resize_reset_button,
			 _("Resets image dimensions"), NULL);
    gtk_widget_add_accelerator(resize_reset_button, "clicked", accel_group,
			       GDK_r, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

 tmpBox = gtk_hbox_new(FALSE, 0);
  tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
  gtk_widget_show(tmpIcon);
  tmpLabel = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
  gtk_widget_show(tmpLabel);
  gtk_widget_show(tmpBox);

    resize_cancel_button = gtk_button_new();
    resize_cancel_button_key = 
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
			      _("_Cancel"));
    gtk_widget_add_accelerator(resize_cancel_button, "clicked", accel_group,
			       resize_cancel_button_key, GDK_MOD1_MASK, (GtkAccelFlags)0);
    gtk_container_add(GTK_CONTAINER(resize_cancel_button), tmpBox);
    gtk_widget_show(resize_cancel_button);
    gtk_container_add(GTK_CONTAINER(buttonbox), resize_cancel_button);
    GTK_WIDGET_SET_FLAGS(resize_cancel_button, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, resize_cancel_button,
			 _("Cancel resizing image"), NULL);
    gtk_widget_add_accelerator(resize_cancel_button, "clicked", accel_group,
			       GDK_Escape, (GdkModifierType)0, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(resize_cancel_button, "clicked", accel_group,
			       GDK_c, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

    gtk_widget_grab_default(resize_cancel_button);
    gtk_object_set_data(GTK_OBJECT(dialog_resize), "tooltips", tooltips);

    gtk_window_add_accel_group(GTK_WINDOW(dialog_resize), accel_group);

    mHeightAdjustment = resize_spinbutton_height_adj;
    mWidthAdjustment = resize_spinbutton_width_adj;
    mCheckButton = resize_checkbutton_aspect;
    mOptionMenu = resize_options_scale_menu;
    mHeightSpinButton = resize_spinbutton_height;
    mWidthSpinButton = resize_spinbutton_width;

    gtk_signal_connect(GTK_OBJECT(dialog_resize),"delete_event",
		       GTK_SIGNAL_FUNC(on_resize_delete_event),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(resize_ok_button),"clicked",
		       GTK_SIGNAL_FUNC(on_resize_button_ok_clicked),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(resize_cancel_button),"clicked",
		       GTK_SIGNAL_FUNC(on_resize_button_cancel_clicked),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(resize_reset_button),"clicked",
		       GTK_SIGNAL_FUNC(on_resize_button_reset_clicked),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(resize_checkbutton_aspect),"toggled",
		       GTK_SIGNAL_FUNC(on_resize_checkbutton_aspect_toggled),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(resize_options_scale_menu),"selection-done",
		       GTK_SIGNAL_FUNC(on_resize_options_scale_menu_done),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(resize_spinbutton_width),"changed",
		       GTK_SIGNAL_FUNC(on_resize_spinbutton_width_changed),
		       (void *) this);	   
    gtk_signal_connect(GTK_OBJECT(resize_spinbutton_height),"changed",
		       GTK_SIGNAL_FUNC(on_resize_spinbutton_height_changed),
		       (void *) this);
    
    return dialog_resize;
}

gboolean on_resize_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	ResizeDialog *dlg = (ResizeDialog *) userData;

	if (NULL == dlg)
		return FALSE;

	delete dlg;

	return TRUE;
}	

void on_resize_button_ok_clicked(
	GtkButton *button,
	gpointer userData)
{
	ResizeDialog *dlg = (ResizeDialog *) userData;

	if (NULL == dlg)
		return;

	ColorImage *temp = *(dlg->mImage);

	if (!dlg->mPixelChecked)
		*(dlg->mImage) = resize(
				temp,
				(int) round((float)dlg->mSpecialHeight / 100.0 * (*(dlg->mImage))->getNumRows()),
			        (int) round((float)dlg->mSpecialWidth / 100.0 * (*(dlg->mImage))->getNumCols())
			    );

	else
		*(dlg->mImage) = resize(temp, dlg->mSpecialHeight, dlg->mSpecialWidth);

	delete temp;
	dlg->mTraceWin->zoomUpdate(true, 0, 0);

	delete dlg;
}

void on_resize_button_cancel_clicked(
	GtkButton *button,
	gpointer userData)
{
	ResizeDialog *dlg = (ResizeDialog *) userData;

	if (NULL == dlg)
		return;

	delete dlg;
}

void on_resize_button_reset_clicked(
	GtkButton *button,
	gpointer userData)
{
	ResizeDialog *dlg = (ResizeDialog *) userData;

	if (NULL == dlg)
		return;

	int newHeight, newWidth;

	if (dlg->mPixelChecked) {
		newHeight = (*(dlg->mImage))->getNumRows();
		newWidth = (*(dlg->mImage))->getNumCols();
	} else {
		newHeight = 100;
		newWidth = 100;
	}

	dlg->mSpecialHeight = newHeight;
	dlg->mSpecialWidth = newWidth;

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->mHeightSpinButton), newHeight);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->mWidthSpinButton), newWidth);
}

void on_resize_checkbutton_aspect_toggled(
	GtkCheckButton *button,
	gpointer userData)
{
	ResizeDialog *dlg = (ResizeDialog *) userData;

	if (NULL == dlg)
		return;

	//Time to extract all the data:
	int newHeight = (int) GTK_ADJUSTMENT(dlg->mHeightAdjustment)->value;
	int newWidth = (int) GTK_ADJUSTMENT(dlg->mWidthAdjustment)->value;

	bool aspectChecked;
	
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dlg->mCheckButton)))
		aspectChecked = true;
	else
		aspectChecked = false;

	dlg->updateResizeOptions(&newHeight,&newWidth,aspectChecked);

	dlg->mSpecialHeight = newHeight;
	dlg->mSpecialWidth = newWidth;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->mHeightSpinButton),newHeight);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->mWidthSpinButton),newWidth);
}

void on_resize_options_scale_menu_done(
	GtkMenuShell *menushell,
	gpointer userData)
{
	ResizeDialog *dlg = (ResizeDialog *) userData;

	if (NULL == dlg)
		return;

	int newHeight = (int) GTK_ADJUSTMENT(dlg->mHeightAdjustment)->value;
	int newWidth = (int) GTK_ADJUSTMENT(dlg->mWidthAdjustment)->value;

	bool aspectChecked;
	
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dlg->mCheckButton)))
		aspectChecked = true;
	else
		aspectChecked = false;

	dlg->updateResizeOptions(&newHeight,&newWidth,aspectChecked);

	dlg->mSpecialHeight = newHeight;
	dlg->mSpecialWidth = newWidth;
	
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->mHeightSpinButton),newHeight);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->mWidthSpinButton),newWidth);
}

void on_resize_spinbutton_height_changed(
	GtkEditable *adjustment,
	gpointer userData)
{
	ResizeDialog *dlg = (ResizeDialog *) userData;

	if (NULL == dlg)
		return;

	dlg->mHeightChanged = true;

	//Time to extract all the data:
	int newHeight = (int) GTK_ADJUSTMENT(dlg->mHeightAdjustment)->value;
	int newWidth = (int) GTK_ADJUSTMENT(dlg->mWidthAdjustment)->value;
	bool aspectChecked;
	
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dlg->mCheckButton)))
		aspectChecked = true;
	else
		aspectChecked = false;

	if (dlg->mSpecialHeight != newHeight) {	
		dlg->updateResizeOptions(&newHeight,&newWidth,aspectChecked);
		dlg->mSpecialHeight = newHeight;
		dlg->mSpecialWidth = newWidth;

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->mHeightSpinButton),newHeight);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->mWidthSpinButton),newWidth);
	}
}

void on_resize_spinbutton_width_changed(
	GtkEditable *adjustment,
	gpointer userData)
{
	ResizeDialog *dlg = (ResizeDialog *) userData;

	if (NULL == dlg)
		return;

	dlg->mHeightChanged = false;

	//Time to extract all the data:
	int newHeight = (int) GTK_ADJUSTMENT(dlg->mHeightAdjustment)->value;
	int newWidth = (int) GTK_ADJUSTMENT(dlg->mWidthAdjustment)->value;
	
	bool aspectChecked;
	
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dlg->mCheckButton)))
		aspectChecked = true;
	else
		aspectChecked = false;

	if (dlg->mSpecialWidth != newWidth) {	
		dlg->updateResizeOptions(&newHeight,&newWidth,aspectChecked);
		dlg->mSpecialHeight = newHeight;
		dlg->mSpecialWidth = newWidth;

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->mHeightSpinButton),newHeight);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dlg->mWidthSpinButton),newWidth);
	}
}

void on_resize_option_pixel_activated(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	ResizeDialog *dlg = (ResizeDialog *) userData;

	if (NULL == dlg)
		return;

	dlg->mPixelChecked = true;
}

void on_resize_option_percent_activated(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	ResizeDialog *dlg = (ResizeDialog *) userData;

	if (NULL == dlg)
		return;

	dlg->mPixelChecked = false;
}
