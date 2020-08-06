//*******************************************************************
//   file: ContourInfoDialog.h
//
// author: Adam Russell
//
//   mods: J H Stewman (4/12/2005)
//         -- incorporation of Outline class
//         J H Stewman (7/25/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#ifndef CONTOURINFODIALOG_H
#define CONTOURINFODIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"
#include <gtk/gtktext.h>

#include "../Outline.h" //***008OL
#include "../Chain.h"

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

// getNumContourInfoDialogReferences
// 	  Returns the number of ContourInfoDialogs open.
int getNumContourInfoDialogReferences();

class ContourInfoDialog
{
	public:
		// Constructor
		// 	  Sets up all the gtk widgets but doesn't show the dialog.  
		//    show() must later be called to do that.
		//
		// 	  Simply uses existing Outline - does NOT make copy.
		ContourInfoDialog(
				GtkWidget *parent, //***2.22
				std::string name,
				Outline *oL, //***008OL
				double outlineColor[4]);

		// Destructor
		// 	  Destroys the dialog if it's open and frees resources.
		~ContourInfoDialog();

		// show
		// 	  Simply draws the dialog on the screen.
		void show();

		// various callback functions

		friend gboolean on_infoDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend gboolean on_infoContourDrawingArea_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend gboolean on_infoChainDrawingArea_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend void on_infoButtonOK_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_infoButtonGenCoeffFiles_clicked(
				GtkButton *button,
				gpointer userData);

	private:
		std::string mName;
		
		GtkWidget
			*mParent, //***2.22
			*mDialog,
			*mDrawingAreaContour,
			*mDrawingAreaChain,
//			*mTextBox,
			*mTextView, //***2.22
			*mSpinButton;

		GtkTextBuffer
			*mTextBuffer; //***2.22 - replaces mTextBox (gtkText)

		Outline *mFinOutline; //***008OL

		GdkGC *mContourGC, *mChainGC, *mHighlightGC;

		double mTraceColor[4];

		GtkWidget* createInfoDialog();

		void updateGC();
		void updateGCColor(GdkGC *gc);
		void updateGCColor(GdkGC *gc, double color[4]);

		void updateInfo();
};

#endif
