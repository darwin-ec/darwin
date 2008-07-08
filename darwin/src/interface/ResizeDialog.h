//*******************************************************************
//   file: ResizeDialog.h
//
// author: Adam Russell
//
//   mods:
//
// Heavily based on Henry's code
//
//*******************************************************************

#ifndef RESIZEDIALOG_H
#define RESIZEDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "../image_processing/ColorImage.h"
#include "TraceWindow.h"

int getNumResizeDialogReferences();

class ResizeDialog
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.
		ResizeDialog(
				TraceWindow *t,
				ColorImage **i);
		
		// Destructor
		// 	Destroys the dialog if it's open and frees
		// 	resources.
		~ResizeDialog();

		// show
		// 	Simply draws the dialog on the screen.
		void show();

		void updateResizeOptions(
			int *newHeight,
			int *newWidth,
			bool aspectChecked);

		// GTK+ callback functions

		friend gboolean on_resize_delete_event(
			GtkWidget *widget,
			GdkEvent *event,
			gpointer userData);
		
		friend void on_resize_button_ok_clicked(
			GtkButton *button,
			gpointer userData);

		friend void on_resize_button_cancel_clicked(
			GtkButton *button,
			gpointer userData);

		friend void on_resize_button_reset_clicked(
			GtkButton *button,
			gpointer userData);

		friend void on_resize_checkbutton_aspect_toggled(
			GtkCheckButton *button,
			gpointer userData);

		friend void on_resize_options_scale_menu_done(
			GtkMenuShell *menushell,
			gpointer userData);

		friend void on_resize_spinbutton_height_changed(
			GtkEditable *adjustment,
			gpointer userData);

		friend void on_resize_spinbutton_width_changed(
			GtkEditable *adjustment,
			gpointer userData);

		friend void on_resize_option_pixel_activated(
			GtkMenuItem *menuitem,
			gpointer userData);

		friend void on_resize_option_percent_activated(
			GtkMenuItem *menuitem,
			gpointer userData);

	private:
		GtkWidget
			*mDialog,
			*mCheckButton,
			*mOptionMenu,
			*mHeightSpinButton,
			*mWidthSpinButton;

		GtkObject
			*mHeightAdjustment,
			*mWidthAdjustment;

		bool
			mPixelChecked,
			mHeightChanged,
			mPixelLast;

		int
			mSpecialHeight,
			mSpecialWidth;

		ColorImage **mImage;
		TraceWindow *mTraceWin;

		GtkWidget* createResizeDialog(int height, int width);
};

#endif
