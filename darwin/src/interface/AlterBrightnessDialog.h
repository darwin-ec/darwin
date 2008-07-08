//*******************************************************************
//   file: AlterBrightnessDialog.h
//
// author: Adam Russell
//
//   mods: J H Stewman (4/12/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#ifndef ALTERBRIGHTNESSDIALOG_H
#define ALTERBRIGHTNESSDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"
#include "TraceWindow.h"
#include "../image_processing/ColorImage.h"

//*******************************************************************
//
// getNumAlterBrightnessDialogReferences
//
// 	  Returns the number of AlterBrightnessDialogs currently open.
//
int getNumAlterBrightnessDialogReferences();


class AlterBrightnessDialog
{
	public:
		// Constructor
		// 	  Sets up all the gtk widgets and such, but doesn't show the 
		//    dialog.  show() must later be called to do that.
		AlterBrightnessDialog(
				TraceWindow *t,
				ColorImage **i);
		
		// Destructor
		// 	  Destroys the dialog if it's open and frees resources.
		~AlterBrightnessDialog();

		// show
		// 	  Simply draws the dialog on the screen.
		void show();

		// GTK callback functions
		
		friend gboolean on_alter_brightness_delete_event(
			GtkWidget *widget,
			GdkEvent *event,
			gpointer userData);

		friend void on_alter_brightness_button_ok_clicked(
			GtkButton *button,
			gpointer userData);

		friend void on_alter_brightness_button_cancel_clicked(
			GtkButton *button,
			gpointer userData);

		friend void on_alter_brightness_slider_moved(
			GtkButton *button,
			gpointer userData);

		friend void on_alter_brightness_button_reset_clicked(
			GtkButton *button,
			gpointer userData);

	private:

		// class pointer to AlterBrightnessDialog widget
		GtkWidget *mDialog;

		// mAdjustment->value is amount by which brightness is changed
		GtkAdjustment *mAdjustment;

		// class pointer to TraceWindow widget from which 
		// the AlterBrightnessDialog is launched
		TraceWindow *mTraceWin;

		// class pointers to the original and modified images
		ColorImage *mOriginalImage, **mImage;

		// createAlterBrightnessDialog
		//    does the actual work of creating the AlterBrightnessDialog widget
		GtkWidget* createAlterBrightnessDialog();

		// updateBrightness
		//    changes brightnes of mImage by increment amount
		void updateBrightness(int increment);
};

#endif
