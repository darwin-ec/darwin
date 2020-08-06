//*******************************************************************
//   file: MatchingDialog.h
//
// author: Adam Russell
//
//   mods: J H Stewman
//         -- major modifications from version 1.0 forward
//
// This window now allows user selection of catalog categories,
// method of match, pause and restart of mathcing, and display
// of progress and contour alignment
//
//*******************************************************************

#ifndef MATCHINGDIALOG_H
#define MATCHINGDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "../Database.h"
#include "../DatabaseFin.h"
#include "../matching/Match.h"
#include "../Options.h"
#include "MainWindow.h"
							     //***004CL ^
int getNumMatchingDialogReferences();

class MatchingDialog
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.
		//
		// 	Only makes pointer copies of dbFin, db, and o
		MatchingDialog(
				DatabaseFin<ColorImage> *dbFin,
				Database *db,
				MainWindow *m,			//***004CL
				Options *o);
		
		// Destructor
		// 	Destroys the dialog if it's open and frees
		// 	resources.
		~MatchingDialog();

		// show
		// 	draws the dialog on the screen. 
		//  Restores state when returning from MatchResultsWindow.
		void show(bool returning);

		void showOutlines(FloatContour *unk, FloatContour *db);
		void showErrorPt2Pt(FloatContour *unk, FloatContour *db,
				float x1, float y1, float x2, float y2);

		GtkWidget *getWindow(); // returns pointer to Gtk Window

		// GTK+ callback functions

		friend gboolean on_mMatchDialogDrawingAreaOutlines_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend gboolean on_matchingDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_matchingButtonStartStop_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_matchingButtonPauseContinue_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_mCategoryButton_toggled(
				GtkButton *button,
				gpointer userData);
		friend void on_categoryCheckButtonAll_clicked(
				GtkButton *button,
				gpointer userData);
		friend void on_categoryCheckButtonClear_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_matchingButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);

		friend	void on_mButtonShowHide_toggled(
				GtkButton *button,
				gpointer userData);

		friend gboolean matchingIdleFunction(
				gpointer userData);

		friend void on_radioOriginal_clicked(
				GtkObject *object,
				gpointer userData);

		friend void on_radioTrimFixed_clicked(
				GtkObject *object,
				gpointer userData);

/**1.85 - removed
		friend void on_radioTrimOptimal_clicked(
				GtkObject *object,
				gpointer userData);
*/
		friend void on_radioTrimOptimalTotal_clicked(
				GtkObject *object,
				gpointer userData);

		friend void on_radioTrimOptimalTip_clicked(
				GtkObject *object,
				gpointer userData);

		friend void on_radioTrimOptimalArea_clicked( //***1.85 - new area metric use in matching
				GtkObject *object,
				gpointer userData);

		friend void on_radioTrimOptimalInOut_clicked(
				GtkObject *object,
				gpointer userData);

		friend void on_radioTrimOptimalInOutTip_clicked(
				GtkObject *object,
				gpointer userData);

		friend void on_radioAllPoints_clicked(
				GtkObject *object,
				gpointer userData);

		friend void on_radioTrailingOnly_clicked( //***1.5 - new callback
				GtkObject *object,
				gpointer userData);

		friend void on_radioLeadToTipOnly_clicked(
				GtkObject *object,
				gpointer userData);

		friend void on_radioLeadToNotchOnly_clicked(
				GtkObject *object,
				gpointer userData);

		friend void on_radioLeadThenTrail_clicked(
				GtkObject *object,
				gpointer userData);

	private:
		GtkWidget
			*mDialog,
			*mProgressBar,
			*mButtonShowHide,
			*mButtonStartStop,
			*mButtonPauseContinue,
			*mDrawingAreaOutlines,
			*mCategoryButton[32]; //***051CC max categories 32

		GdkGC *mGC1, *mGC2;

		Match *mMatch;

		Options *mOptions;

		DatabaseFin<ColorImage> *mFin;
		Database *mDatabase;
		MainWindow *mMainWin;			//***004CL

		bool
			mUseFullFinError, //***055ER - use whole outline vs. trimmed outline in final error measure
			mCategoryToMatch[32]; //***051CC max categories 32

		bool 
			mShowingOutlines,
			mMatchCancelled, // indicates matching has been aborted
			mMatchRunning,   // match started and not stopped or finished
			mMatchPaused;    // matched paused in process
		int 
			mCategoriesSelected, // ***051CC number of categories selected
			mRegistrationMethod, // indicates current matching method
			mRegSegmentsUsed;    // indicates

		GtkWidget* createMatchingDialog();

		void updateGC();
		void updateGCColor(GdkGC *gc);
		void updateGCColor(GdkGC *gc, double color[4]);
};

#endif
