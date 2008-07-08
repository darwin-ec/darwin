//*******************************************************************
//   file: MatchResultsWindow.h
//
// author: Adam Russell
//
//   mods: J H Stewman (2006 & 2007)
//
// BIG NOTE: Don't carry more than one of these objects around at a
// time! 
//
//*******************************************************************

#ifndef MATCHRESULTSWINDOW_H
#define MATCHRESULTSWINDOW_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include <vector>

#include "../image_processing/ColorImage.h"
#include "../Database.h"
#include "../FloatContour.h"
#include "../matching/MatchResults.h"
#include "../Options.h"
#include "MainWindow.h"
#include "../interface/MatchingDialog.h"
                          //*** 004CL ^
#include "../interface/MatchingQueueDialog.h" //***1.3

typedef enum {MR_VIEW_ICONS, MR_VIEW_LIST} mrViewType;

class MatchResultsWindow
{
	public:
		// Constructor
		// 	Sets up all the GTK widgets and stuff, but
		// 	doesn't show them automatically.  show() must
		// 	later be called to do that.
		//
		// 	Note: The Database object passed in must NOT go
		// 	away or everything here will fall apart.  This
		// 	might be a little sloppy, but the database
		// 	should exist for the life of the program. (Could
		// 	create a copy... but this could create bad
		// 	problems down the road)
		//
		MatchResultsWindow(
				const DatabaseFin<ColorImage> *unknownFin,
				const MatchResults *results,
				Database *database,
				MainWindow *mainwin,	        //*** 004CL
				MatchingDialog *matchingDialog, //***043MA
				//GtkWidget *parentWindow,        //***1.3
				MatchingQueueDialog *matchingQueueDialog, //***1.3
				string resultsFilename, //***1.6
				Options *o);
		
		// Destructor
		//
		~MatchResultsWindow();

		// show
		// 	Simply draws the window on the screen.
		//
		void show();

		// GTK+ callback functions
		friend gboolean on_matchResultsWindow_delete_event(
						GtkWidget *widget,
						GdkEvent *event,
						gpointer userData);

		friend void on_mrRadioButtonIcons_toggled(
						GtkToggleButton *togglebutton,
						gpointer userData);

		friend void on_mrButtonAltID_toggled( //***1.6 - to hide or show real ID's
						GtkToggleButton *togglebutton,
						gpointer userData);

		friend void on_mrButtonShowInfo_toggled( //***1.6 - to hide or show info
						GtkToggleButton *togglebutton,
						gpointer userData);
			
		friend void on_mrRadioButtonList_toggled(
						GtkToggleButton *togglebutton,
						gpointer userData);

		friend void on_mMRCList_click_column(
						GtkCList *clist,
						gint column,
						gpointer userData);

		friend void on_mMRCList_select_row(
						GtkCList *clist,
						gint row,
						gint column,
						GdkEvent *event,
						gpointer userData);

		friend void on_mMRButtonPrev_clicked(
						GtkButton *button,
						gpointer userData);

		friend void on_mMRButtonNext_clicked(
						GtkButton *button,
						gpointer userData);

		friend void on_mMRButtonSlideShow_clicked( //***1.85 - new
						GtkButton *button,
						gpointer userData);

		friend void on_mrButtonFinsMatch_clicked(
						GtkButton *button,
						gpointer userData);

		friend void on_mrButtonNoMatch_clicked(
						GtkButton *button,
						gpointer userData);

		friend void on_mMRButtonSelectedMod_clicked( //***1.2 - new
						GtkButton *button,
						gpointer userData);

		friend void on_mMRButtonUnknownMod_clicked( //***1.2 - new
						GtkButton *button,
						gpointer userData);

		friend void on_mMRButtonUnknownMorph_clicked( //***1.2 - new
						GtkButton *button,
						gpointer userData);

		friend void on_mrButtonReturnToMatchingDialog_clicked(
						GtkButton *button,
						gpointer userData);

		friend void on_mrButtonSaveResults_clicked( //***1.4 - new
						GtkButton *button,
						gpointer userData);

		friend void on_mrButtonCancel_clicked(
						GtkButton *button,
						gpointer userData);

		friend gboolean on_eventBoxSelected_button_press_event(
						GtkWidget *widget,
						GdkEventButton *event,
						gpointer userData);

		friend gboolean on_mDrawingAreaSelected_expose_event(
						GtkWidget *widget,
						GdkEventExpose *event,
						gpointer userData);

		friend gboolean on_eventBoxUnknown_button_press_event(
						GtkWidget *widget,
						GdkEventButton *event,
						gpointer userData);

		friend gboolean on_mDrawingAreaUnknown_expose_event(
						GtkWidget *widget,
						GdkEventExpose *event,
						gpointer userData);

		friend gboolean on_eventBoxOutlines_button_press_event(
						GtkWidget *widget,
						GdkEventButton *event,
						gpointer userData);

		friend gboolean on_mDrawingAreaOutlines_expose_event(
						GtkWidget *widget,
						GdkEventExpose *event,
						gpointer userData);

		friend void on_finRadioButton_toggled(
						GtkToggleButton *togglebutton,
						gpointer userData);

		friend gboolean matchResultsSlideShowTimer(
						gpointer userData);

	private:

		DatabaseFin<ColorImage> *mUnknownFin, *mSelectedFin;

		// Not really necessary at the moment, but it keeps
		// track of the view anyway.
		mrViewType mMRView;

		ColorImage

			*mUnknownImageOriginal,    // full sized original, others are scaled to fit display
			*mUnknownImageModOriginal, //***1.5 - full sized modified image from TraceWindow
			*mUnknownImage,            // scaled to fit display
			*mUnknownImageMod,         //***1.5 - scaled to fit display
			*mUnknownImageShown,       //***1.5 - pointer to either of above

			*mSelectedImageOriginal,    //***1.8 - full sized original, others are scaled to fit display
			*mSelectedImageModOriginal, //***1.8 - full sized modified image from TraceWindow
			*mSelectedImage,
			*mSelectedImageMod,        //***1.5 - for clipped, zoomed, enhanced version
			*mSelectedImageShown;       //***1.8 - pointer to either of above

		FloatContour *mUnknownContour; //***005CM
		FloatContour *mRegContour;
		
		GtkWidget
			*mWindow,
			//*mParentWindow,        //***1.3
			*mScrolledWindow,
			*mMRCList,
			*mMRIconTableViewPort,
			*mDrawingAreaSelected,
			*mDrawingAreaUnknown,
			*mDrawingAreaOutlines,
			
			*mMRButtonNext,
			*mMRButtonPrev,
			*mMRButtonSlideShow,    //***1.85
			
			*mMRButtonSelectedMod,  //***1.2
			*mMRButtonUnknownMod,   //***1.2
			*mMRButtonUnknownMorph; //***1.2

		//***1.8 - globals to track previous image drawable sizes within window
		int 
			mPrevSelImgHeight, 
			mPrevSelImgWidth,
			mPrevUnkImgHeight,
			mPrevUnkImgWidth;

		//***1.2 - new bools to track states of displayed images
		bool
			mSelectedIsModified,
			mUnknownIsModified,
			mUnknownIsMorphed;

		bool
			mSlideShowOn; //***1.85

		int mTimerID; //***1.85 - id of active timer function

		std::vector<GtkWidget*> mRadioButtonVector;
	
		MatchResults *mResults;
		int mCurEntry;

		Database *mDatabase;
		MainWindow *mMainWin;	//*** 004CL

		MatchingDialog *mMatchingDialog; //***043MA so we can return to the Matching Dialog
		bool mReturningToMatchingDialog;

		MatchingQueueDialog *mMatchingQueueDialog; //***1.3 so we can return here if needed

		Options *mOptions;

		GdkGC *mGC1, *mGC2;
		GdkCursor *mCursor;

		string mSaveMessage; //***1.6 - used to display name(s) of files saved when saving RESULTS

		bool mLocalHideIDs; //***1.65 - to allow hide/show of IDs, just affecting this window

		// GTK+ utility functions to set up and manage widgets
		GtkWidget* createMatchResultsWindow();
		void createMRCList();

		void createMRIconTable();
		GtkWidget* createFinRadioButton(
						std::string id,
						char **pixMapString,
						int num,
						GSList *group);

		void selectFromCList(int newCurEntry);               //***1.75CL
		void selectFromReorderedCList(std::string filename); //***1.75CL

		void updateList();

		void refreshSelectedFin();
		void refreshUnknownFin();

		void refreshOutlines();

		void updateCursor();
		void updateGC();
		void updateGCColor(GdkGC *gc);
		void updateGCColor(GdkGC *gc, double color[4]);

		//***1.4 - part of saving MatchResults
		string saveFinIfNeeded();

};

#endif
