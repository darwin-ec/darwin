//*******************************************************************
//   file: ImageViewDialog.h
//
// author: Adam Russell
//
//   mods: 
//
//*******************************************************************

#ifndef IMAGEVIEWDIALOG_H
#define IMAGEVIEWDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include "../image_processing/ColorImage.h"

int getNumImageViewDialogReferences();

class ImageViewDialog
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.
		ImageViewDialog(const std::string &name, ColorImage *image);
		
		// Destructor
		// 	Destroys the dialog if it's open and frees
		// 	resources.
		~ImageViewDialog();

		// show
		// 	Simply draws the dialog on the screen.
		void show();

		// GTK+ callback functions

		friend gboolean on_viewDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_viewButtonZoomIn_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_viewButtonZoomOut_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_viewButtonFlipHorizontally_clicked( //***1.75 - new
				GtkButton * button,
				gpointer userData);

		friend gboolean on_viewEventBox_button_press_event(
				GtkWidget *widget,
				GdkEventButton *event,
				gpointer userData);

		friend gboolean on_viewDrawingArea_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend void on_viewButtonClose_clicked(
				GtkButton *button,
				gpointer userData);

	private:
		GtkWidget
			*mDialog,
			*mDrawingArea,
			*mLabelZoom,
			*mScrolledWindow,
			*mButtonFlipHorizontally, //***1.75
			*mButtonZoomIn,
			*mButtonZoomOut;

		ColorImage 
			*mNonZoomedImage, 
			*mImage,
			*mFlippedImage; //***1.75 - to speed display

		bool 
			mIsFlipped,      //***1.75 - is currently flipped
			mFlippedImageOK; //***1.75 - false when scale changes

		GdkCursor *mCursor;

		int mZoomRatio;

		//***1.75 - location of window center in image 
		//          (as percent of image dimension ... 0.0 to 1.0)
		double
			mImageCenterX,  
			mImageCenterY;

		GtkWidget* createViewDialog(const std::string &title);

		void updateCursor();

		void zoomIn();
		void zoomOut();
		void zoomUpdate(bool zoomChanged);

		void refreshImage();
};

#endif
