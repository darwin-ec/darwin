//*******************************************************************
//   file: ModifyDatabaseWindow.h
//
// author: Adam Russell
//
//   mods: J H Stewman (4/12/2005)
//         -- code to use Options to specify Catalog Categories used
//
//*******************************************************************


#ifndef MODIFYWINDOW_H
#define MODIFYWINDOW_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "../image_processing/ColorImage.h"
#include "../Database.h"
#include "MainWindow.h"

int getNumModifyDatabaseWindowReferences();

class ModifyDatabaseWindow
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.

		ModifyDatabaseWindow(         
				int DBCurEntry,
				MainWindow *m,			//***004CL
				DatabaseFin<ColorImage> *Fin,
				Database *db,
				Options *o);             //***054
				

		// Destructor
		// 	Destroys the dialog if it's open and frees
		// 	resources.
		~ModifyDatabaseWindow();

		// show
		// 	Simply draws the dialog on the screen.
		void show();
		
		void clearText();
		
		void modifySave(const std::string &fileName);

		void zoomUpdate(bool setSize, int x = 0, int y = 0);
		
		// GTK+ callback functions

		friend gboolean on_modifyWindow_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend gboolean on_modifyDrawingArea_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend void on_modifyButtonSave_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_modifyButtonDeleteFin_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_modifyButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);

		friend gboolean on_m_questionDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_m_questionButtonYes_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_m_questionButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);

		friend gboolean on_m_mDrawingArea_configure_event(
				GtkWidget *widget,
				GdkEventConfigure *event,
				gpointer userData);

		friend gboolean on_m_mScrolledWindow_configure_event(
				GtkWidget *widget,
				GdkEventConfigure *event,
				gpointer userData);
		

	private:

		ColorImage
			*mNonZoomedImage,
		        *mImage;

		Database *mDatabase;
		DatabaseFin<ColorImage> *mFin;
                MainWindow *mMainWin;				//***004CL
		
		std::string mImagefilename;	
		
		GtkWidget
			*mWindow,
			*mQuestionDialog,
			*mDrawingArea,
			*mScrolledWindow,

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

		Options *mOptions;

		//////////////////////////
		// Private utility functions:

		GtkWidget* createModifyDatabaseWindow(const std::string &title);
		GtkWidget* createQuestionDialog();

		void refreshImage();

		
};

#endif
