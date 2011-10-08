//
//   file: ExportFinzDialog.h
//
// author: J H Stewman
//
//   date: 7/22/2008
//

#ifndef EXPORTFINZDIALOG_H
#define EXPORTFINZDIALOG_H

#include <set>
#include <vector>
#include "../CatalogSupport.h"

class ExportFinzDialog {
	public:
		//***2.02 - new enum so we can use dialog in two different modes
		enum {
			exportFinz=0,
			exportFullSizeModImages 
		};
		ExportFinzDialog(Database *db, GtkWidget *parent,
				GtkWidget *clist,  db_sort_t currentSort,
				std::vector<int> row2id, std::vector<int> id2row,
				int dialogMode); //***2.02 -new parameter
		~ExportFinzDialog();

		void show();
		void refreshDatabaseDisplayNew(bool sizeChanged);
		void selectFromCList(int newCurEntry);
		void selectFromReorderedCList(std::string filename);
		void selectFromReorderedCListNew(std::string selectedIdPlusOffset);
		void displayStatusMessage(const std::string &msg);

		friend void on_finzButtonFindNow_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_finzCList_click_column(
				GtkCList *clist,
				gint column,
				gpointer userData);

		friend void on_finzCList_select_row(
				GtkCList *clist,
				gint row,
				gint column,
				GdkEvent *event,
				gpointer userData);

		friend void on_finzCList_select_first_row(
				GtkCList *clist,
				gpointer userData);

		friend gboolean on_finzCList_button_release_event(
				GtkCList *clist,
				GdkEvent *event,
				gpointer userData);

		friend void on_finzCList_select_last_row(
				GtkCList *clist,
				gpointer userData);

		friend void on_finzCList_unselect_row(
				GtkCList *clist,
				gint row,
				gint column,
				GdkEvent *event,
				gpointer userData);

		friend void on_finzDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_finzDialogButtonSaveFinz_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_finzDialogButtonSaveImages_clicked( //***2.02
				GtkButton *button,
				gpointer userData);

		friend void on_finzDialogButtonSaveAsCatalog_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_finzDialogButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);

	private:

		GtkWidget
			*mDialog,
			*mParentWindow,
			*mCList,
			*mScrollable,
			*mSearchID, // entry field for find in clist by dolphin ID
			*mButtonSave,
			*mButtonSaveAsCatalog,
			*mButtonCancel,
			*mStatusBar;

		db_sort_t 
			mOldSort, // previous sorting of CList
			mNewSort; // current/new sorting of CList

		//std::set<int>
		std::set<long>  //***2.22 - now 64 bit arch on Mac
			mDBCurEntry; // supports multiple selection

		bool 
			mShowAlternates;

		int
			mDialogMode; //***2.02 - we are EITHER exporting Finz OR saving full-size images

		std::vector<int> 
			mRow2Id,
			mId2Row;

		guint mContextID;

		DatabaseFin<ColorImage> 
			*mSelectedFin;

		Database 
			*mDatabase;

		ColorImage 
			*mImage,
			*mOrigImage;
		int
			mCurImageHeight, 
			mCurImageWidth,
			mCurContourHeight, 
			mCurContourWidth;

		GdkGC 
			*mGC;

		GdkCursor 
			*mCursor;

		Options 
			*mOptions;

		std::string
			mImportFromFilename,
			mExportToFilename;

		GtkWidget* createDialog();
		void copyCList(GtkCList *clist);
};

#endif
