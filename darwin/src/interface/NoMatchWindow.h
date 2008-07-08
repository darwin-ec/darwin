//*******************************************************************
//   file: NoMatchWindow.h
//
// author: Adam Russell
//
//   mods: J H Stewman (4/12/2005)
//         -- code to use otions to specify catalog categories
//
//*******************************************************************


#ifndef NOMATCHWINDOW_H
#define NOMATCHWINDOW_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "../image_processing/ColorImage.h"
#include "../Outline.h" //***008OL
#include "../Database.h"
#include "MainWindow.h" //***004CL
                       
#include "../Options.h" //***054

int getNumNoMatchWindowReferences();

class NoMatchWindow
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.

		NoMatchWindow(                
			      DatabaseFin<ColorImage> *Fin,
			      Database *db,
			      MainWindow *m,			//***004CL
				  Options *o);              //***054
				

		// Destructor
		// 	Destroys the dialog if it's open and frees
		// 	resources.
		~NoMatchWindow();

		// show
		// 	Simply draws the dialog on the screen.
		void show();
		
		//void nomatchSave(const std::string &fileName);

		void zoomUpdate(bool setSize, int x = 0, int y = 0);
		
		// GTK+ callback functions

		friend gboolean on_nomatchWindow_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend gboolean on_nomatchDrawingArea_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend void on_primaryAlternateCheckButton_toggled( //***1.95
				GtkButton *button,
				gpointer userData);

		friend void on_nomatchButtonAddToDatabase_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_nomatchButtonDeleteFin_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_nomatchButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);

		//friend gboolean on_n_questionDialog_delete_event(
		//		GtkWidget *widget,
		//		GdkEvent *event,
		//		gpointer userData);

		//friend void on_n_questionButtonYes_clicked(
		//		GtkButton *button,
		//		gpointer userData);

		//friend void on_n_questionButtonCancel_clicked(
		//		GtkButton *button,
		//		gpointer userData);

		friend gboolean on_n_mDrawingArea_configure_event(
				GtkWidget *widget,
				GdkEventConfigure *event,
				gpointer userData);

		friend gboolean on_n_mScrolledWindow_configure_event(
				GtkWidget *widget,
				GdkEventConfigure *event,
				gpointer userData);
	private:
		//Contour *mContour; removed 008OL
		Outline *mFinOutline; //***008OL
		ColorImage
			*mNonZoomedImage,
			*mImage;

		Database *mDatabase;
		DatabaseFin<ColorImage> *mFin;
		MainWindow *mMainWin;			//***004CL
		
		Options *mOptions;    //***054
		std::string mImagefilename;	

		bool mSaveAsAlternate; //***1.95

		GtkWidget
			*mWindow,
			*mQuestionDialog,
			*mDrawingArea,
			*mScrolledWindow,
			*mCList,
			*mSaveModeButton, //***1.95

			// Text entries
			*mEntryID,
			*mEntryName,
			*mEntryDate,
			*mEntryRoll,
			*mEntryLocation,
			*mEntryDamage,
			*mEntryDescription;

		float mZoomScale;
		int mZoomRatio;
		int mDBCurEntry;
	
		// offsets for zoomed images
		int mZoomXOffset, mZoomYOffset;
		int mIgnoreConfigureEventCnt;

		// stuff to handle scrolled window configuration events
		// (these variables limit the size that the scrolled
		// window will be set to.)
		int mSWWidthMax, mSWHeightMax;


		//////////////////////////
		// Private utility functions:

		GtkWidget* createNoMatchWindow(const std::string &title);
		//GtkWidget* createQuestionDialog();

		void refreshImage();

		//***1.96a -- new functinos to support image copying to catalog
		std::string copyImageToDestinationAs(std::string srcName, std::string destPath);
		std::string nextAvailableDestinationFilename(std::string destName);

};

#endif
