//*******************************************************************
//   file: MappedContoursDialog.h
//
// author: J H Stewman (1/24/2005)
//
//   mods: 
//
//*******************************************************************

#ifndef MAPPEDCONTOURSDIALOG_H
#define MAPPEDCONTOURSDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"
#include <gtk/gtktext.h>

#include "../Outline.h" //***008OL
#include "../Chain.h"

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

// getNumContourInfoDialogReferences
// 	  Returns the number of ContourInfoDialogs open.
int getNumMappedContourDialogReferences();

class MappedContoursDialog
{
	public:
		// Constructor
		// 	  Uses existing FloatContours - does NOT make copies.
		MappedContoursDialog(
				GtkWidget *parent, //***2.22
				std::string ident1,
				FloatContour *c1,
				int b1, int t1, int e1,
				std::string ident2,
				FloatContour *c2,
				int b2, int t2, int e2);

		// Destructor
		// 	  Destroys the dialog if it's open and frees resources.
		~MappedContoursDialog();

		// show
		// 	  Draws the dialog on the screen.
		void show();

		// various callback functions

		friend gboolean on_mappedContoursDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend gboolean on_mappedDrawingArea_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend gboolean on_mappedChainDrawingArea_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend void on_mappedButtonOK_clicked(
				GtkButton *button,
				gpointer userData);

		//***1.75 - two new members for finding and displaying point pairs used in error calc

		void findPointPairsUsedInMSECalulation(
				FloatContour *c1, // mapped unknown fin 
				int begin1,
				int end1,
				FloatContour *c2, // envenly spaced database fin
				int begin2,
				int end2);

		void showPointPairsUsedInMSECalulation(); // uses mUnkErrPts & mSelErrPts

	private:
		std::string 
			mNameUnk, 
			mNameDB;
		
		GtkWidget
			*mParent, //***2.22
			*mDialog,
			*mDrawingAreaContour,
			*mDrawingAreaChain,
//			*mTextBox,
			*mTextView, //***2.22 - replaces mTextBox (GtkText)
			*mSpinButton;

		GtkTextBuffer
			*mTextBuffer; //***2.22

		//Outline *mFinOutline; //***008OL
		FloatContour 
			*mUnkContour,
			*mDBContour;

		int 
			mDBBegin, mDBTip, mDBEnd,
			mUnkBegin, mUnkTip, mUnkEnd;

		//***1.75 - new float contours for corresponding point pairs used in error
		//          calculation --- so we can display same
		FloatContour 
			mMidPts,
			mUnkErrPts, 
			mSelErrPts;
		bool mShowErrPts;

		GdkGC 
			*mUnknownContourGC, 
			*mDatabaseContourGC, 
			*mChainGC, 
			*mHighlightGC;

		double mTraceColor[4];

		GtkWidget* createDialog();

		void updateGC();
		void updateGCColor(GdkGC *gc);
		void updateGCColor(GdkGC *gc, double color[4]);

		void updateInfo();
};

#endif
