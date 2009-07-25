//*******************************************************************
//   file: MainWindow.h
//
// author: Adam Russell
//
//   mods: 
//
//*******************************************************************

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "../Database.h"
#include "../Options.h"
#include "DataExportDialog.h"

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

class OpenFileChooserDialog; //***1.85 - forward declaration

class MainWindow
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the window.  show() must later be
		// 	called to do that.
		//
		// 	Note: db must not be deleted until this
		// 	MainWindow object is deleted.
		MainWindow(Database *db, Options *o);
		
		// Destructor
		// 	Destroys the window if it's open and frees
		// 	resources.
		~MainWindow();

		// show
		// 	Simply draws the window on the screen.
		void show();

		void clearText();		//***002DB

		void refreshDatabaseDisplay();

		void refreshDatabaseDisplayNew(bool sizeChanged); //***1.85

		void selectFromCList(int newCurEntry);      //***004CL

		void selectFromReorderedCList(std::string filename);   //***004CL

		void selectFromReorderedCListNew(std::string selectedIdPlusOffset);   //***1.85

		void refreshOptions(bool initialShow, bool forceReload); //***1.96a - added params

		int getSelectedRow(); //***1.96a

		void displayStatusMessage(const std::string &msg);

		GtkWidget *getWindow(); //***1.3

		void setDatabasePtr(Database *db); //***1.85 - used when opening new DB

		void setExportFilename(std::string filename); //***1.99

		void resetTitleButtonsAndBackupOnDBLoad(); //***1.85 - called after load or import of DB

		// GTK+ callback functions

		friend gboolean on_mainWindow_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_open_activate(
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_open_fin_trace_activate( //***1.4
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_open_database_activate( //***1.85
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_new_database_activate( //***1.85
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_restore_database_activate( //***1.85
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_backup_database_activate( //***1.85
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_import_database_activate( //***1.85
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_export_database_activate( //***1.85
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_export_finz_activate( //***1.99
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_export_fullSzImgs_activate( //***2.02
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_import_finz_activate( //***1.99
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_matching_queue_activate(
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_exit_activate(
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_options_activate(
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_catalog_new_activate( //***1.4
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_catalog_view_activate( //***1.4
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_catalog_select_activate( //***1.4
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_exportData_select_activate( //***1.9
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_about_activate(
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_docs_activate(
				GtkMenuItem *menuitem,
				gpointer userData);

		friend void on_mainButtonOpen_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_mainButtonOpenTrace_clicked( //***1.5 - new callback
				GtkButton *button,
				gpointer userData);

		friend void on_mainButtonMatchingQueue_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_mainButtonOptions_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_mainButtonExit_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_mainCList_click_column(
				GtkCList *clist,
				gint column,
				gpointer userData);

		friend void on_mainCList_select_row(
				GtkCList *clist,
				gint row,
				gint column,
				GdkEvent *event,
				gpointer userData);

		friend void on_mainButtonPrev_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_mainButtonNext_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_mainButtonModify_clicked(	//***002DB
				GtkButton *button,
				gpointer userData);

		friend void on_mainButtonFindNow_clicked(	//***1.85
				GtkButton *button,
				gpointer userData);

		friend gboolean on_mainEventBoxImage_button_press_event(
				GtkWidget *widget,
				GdkEventButton *event,
				gpointer userData);

		//***1.99 - create a popup containing the Original Image
		friend gboolean on_mainEventBoxOrigImage_button_press_event(
				GtkWidget *widget,
				GdkEventButton *event,
				gpointer userData);

		friend gboolean on_mainDrawingAreaImage_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		//***1.99 - so we can display original image as well as modified
		friend gboolean on_mainDrawingAreaOrigImage_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend gboolean on_mainEventBoxOutline_button_press_event(
				GtkWidget *widget,
				GdkEventButton *event,
				gpointer userData);

		friend gboolean on_mainDrawingAreaOutline_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend gboolean on_mainDrawingAreaOutline_configure_event(
				GtkWidget *widget,
				GdkEventConfigure *event,
				gpointer userData);

		friend gboolean on_mainDrawingAreaImage_configure_event(
				GtkWidget *widget,
				GdkEventConfigure *event,
				gpointer userData);

		//***1.99 - so we can display original image as well as modified
		friend gboolean on_mainDrawingAreaOrigImage_configure_event(
				GtkWidget *widget,
				GdkEventConfigure *event,
				gpointer userData);

		//***1.85 -  so the database can be changed from the CreateDatabaseDialog callback
		friend void on_createDbButtonOK_clicked(
				GtkButton *button,
				gpointer userData);

		//***1.95 - so main clist can show ALL images or ONLY primary images
		friend void on_mainButtonShowAllImages_toggled(
				GtkButton *button,
				gpointer userData);

		//***1.85 -  so the database can be changed from the OpenFileChooserDialog callback
		friend void on_fileChooserButtonOK_clicked(
				OpenFileChooserDialog *dlg);

		//***1.85 -  so the database can be changed from the ModifyDatabase callback
		friend void on_m_questionButtonYes_clicked(
				GtkButton *button,
				gpointer userData);

		//***1.9 -  so the database can be changed from the ModifyDatabase callback
		friend void on_modifyButtonSave_clicked(
				GtkButton * button, 
				gpointer userData);

		//***1.9 - for access to mExportToFilename
		friend bool DataExportDialog::saveData();

	private:
		GtkWidget
			*mWindow,
		
			// Text entries to display info about the
			// currently selected fin
			*mEntryID,
			*mEntryName,
			*mEntryDate,
			*mEntryRoll,
			*mEntryLocation,
			*mEntryDamage,
			*mEntryDescription,

			// drawing areas
			*mDrawingAreaImage,
			*mDrawingAreaOrigImage, //***1.99
			*mDrawingAreaOutline,

			*mButtonNext,
			*mButtonPrev,
			*mButtonModify,

			*mCList,
			*mScrollable, //***1.7CL
			
			//***1.85 - new links to buttons and menu items that are now sensitive
			//          to whether or not the database file successfully loaded
			*mOpenImageMenuItem,
			*mOpenFinMenuItem,
			*mQueueMenuItem,
			*mBackupMenuItem,
			*mExportSubMenuItem,
			*mExportDBMenuItem,
			*mExportFinzMenuItem,
			*mExportFullSzImgsMenuItem, //***2.02 - for generating full size modified images
			*mImportDBMenuItem,
			*mImportFinzMenuItem,
			*mOpenImageButton,
			*mOpenFinButton,
			*mQueueButton,

			*mSearchID, //***1.85 - new entry field for find in clist by dolphin ID

			*mToolBar,
			
			*mStatusBar,
			
			*mFrameMod,  //***1.99
			*mFrameOrig; //***1.99

		db_sort_t 
			mOldSort, //***1.85 - previous sorting of CList
			mNewSort; //***1.85 - current/new sorting of CList

		int mDBCurEntry;
		unsigned long mDBCurEntryOffset; //***1.96a

		bool mShowAlternates; //***1.95

		std::vector<int> mRow2Id; //***1.95 - since not all images may be displayed
		std::vector<int> mId2Row; //***1.95 - since not all images may be displayed

		guint mContextID;

		DatabaseFin<ColorImage> *mSelectedFin;

		Database *mDatabase;

		ColorImage 
			*mImageFullsize,     //***2.01
			*mOrigImageFullsize, //***2.01
			*mImage,
			*mOrigImage; //***1.99
		int
			mCurImageHeight, mCurImageWidth,
			mCurContourHeight, mCurContourWidth;

		GdkGC *mGC;

		GdkCursor *mCursor;

		Options *mOptions;

		std::string
			mImportFromFilename, //***1.85
			mExportToFilename;   //***1.85

		// Utility functions
		GtkWidget* createMainWindow(toolbarDisplayType toolbarDisplay);

		void refreshImage();
		void refreshOrigImage();
		void refreshOutline();

		void updateCursor();
		void updateGC();
		void updateGCColor();
};

#endif
