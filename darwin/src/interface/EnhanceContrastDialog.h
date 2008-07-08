//*******************************************************************
//   file: EnhanceContrastDialog.h
//
// author: Adam Russell
//
//   mods: 
//
//*******************************************************************

#ifndef ENHANCECONTRASTDIALOG_H
#define ENHANCECONTRASTDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "TraceWindow.h"
#include "../image_processing/ColorImage.h"

int getNumEnhanceContrastDialogReferences();

class EnhanceContrastDialog
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.
		EnhanceContrastDialog(
				TraceWindow *t,
				ColorImage **i);
		
		// Destructor
		// 	Destroys the dialog if it's open and frees
		// 	resources.
		~EnhanceContrastDialog();

		// show
		// 	Simply draws the dialog on the screen.
		void show();

		// GTK callback functions
		
		friend gboolean on_enhance_contrast_delete_event(
			GtkWidget *widget,
			GdkEvent *event,
			gpointer userData);

		friend void on_enhance_contrast_button_ok_clicked(
			GtkButton *button,
			gpointer userData);

		friend void on_enhance_contrast_button_cancel_clicked(
			GtkButton *button,
			gpointer userData);

		friend void on_enhance_contrast_minSlider_moved(
			GtkButton *button,
			gpointer userData);

		friend void on_enhance_contrast_maxSlider_moved(
			GtkButton *button,
			gpointer userData);

		friend void on_enhance_contrast_button_reset_clicked(
			GtkButton *button,
			gpointer userData);

	private:
		GtkWidget *mDialog;
		GtkAdjustment *mAdjustment1;
		GtkAdjustment *mAdjustment2;

		TraceWindow *mTraceWin;

		ColorImage *mOriginalImage, **mImage;

		GtkWidget* createEnhanceContrastDialog();

		void updateContrast(unsigned minLevel, unsigned maxLevel);
};

#endif
