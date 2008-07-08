//*******************************************************************
//   file: CatalogSchemeDialog.h
//
// author: J H Stewman (7/11/2006)
//
//*******************************************************************

#ifndef CATALOG_SCHEME_DIALOG_H
#define CATALOG_SCHEME_DIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "../Options.h"

int getNumCatalogSchemeDialogReferences();

//**1.95 - new class for modifying of removing scheme or single category name

class CatEntryDialog
{
	public:

		enum {
			removeScheme = 0,
			renameScheme,
			removeCategory,
			insertCategory,
			renameCategory
		};

		CatEntryDialog(GtkWidget *parentWindow, int openMode, std::string Msg = "");
		
		~CatEntryDialog();

		std::string run();

	private:

		GtkWidget
			*mDialog,
			*mEntry;

		std::string 
			mEntryText;

		GtkWidget* createCatEntryDialog(GtkWidget *parentWindow, int openMode, std::string Msg);

};

class CatalogSchemeDialog
{
	public:

		enum {
			defineNewScheme = 0,
			viewExistingSchemes,
			setDefaultScheme
		};

		CatalogSchemeDialog(GtkWidget *parentWindow, Options *o, int openMode);
		
		~CatalogSchemeDialog();

		void show();

		// GTK+ callback functions

		friend gboolean on_catalogSchemeDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend gboolean on_catalogSchemeButtonCancel_clicked( //***1.95
				GtkWidget *button,
				gpointer userData);

		friend void on_catalogSchemeButtonAllowEdit_clicked( //***1.95
				GtkWidget *button,
				gpointer userData);

		friend void on_categoryRename_activate( //***1.95
				GtkMenuItem *item,
				gpointer userData);

		friend void on_categoryInsert_activate( //***1.95
				GtkMenuItem *item,
				gpointer userData);

		friend void on_categoryRemove_activate( //***1.95
				GtkMenuItem *item,
				gpointer userData);

		friend void on_schemeRename_activate( //***1.95
				GtkMenuItem *item,
				gpointer userData);

		friend void on_schemeRemove_activate( //***1.95
				GtkMenuItem *item,
				gpointer userData);

		friend void on_catalogSchemeButtonOK_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_catSchemeCList_select_row(
				GtkCList *clist,
				gint row,
				gint column,
				GdkEvent *event,
				gpointer userData);

		friend void on_catSchemeCategoryList_select_row( //***1.95
				GtkCList *clist,
				gint row,
				gint column,
				GdkEvent *event,
				gpointer userData);

		friend void on_catSchemeComboBox_select_row(
				GtkComboBox *cbox,
				gpointer userData);

	private:

		// class pointer to the CatalogSchemeDialog and parent widgets
		GtkWidget 
			*mDialog,
			*mParentWindow,
			*mSchemeList,
			*mCategoryList,
			*mNewSchemeName,
			*mNewCategoryNames,
			*mEditMessage; //***1.95 - editable or not ...

		int mSelectedRow; //***1.95 - for selected row in any list

		GtkTextBuffer *mBuffer; // for entering new category names

		Options *mOptions;

		int mOpenMode; // defineNewScheme, viewExistingSchemes, or setDefaultScheme

		bool mEditable; //***1.95 - indicates editability of schemes

		//***1.95 - local copies of scheme info for edit support
		int mNumberOfLocalSchemes;
		std::vector<std::string> mLocalSchemeName;
		std::vector<int> mLocalCategoryNamesMax;             
		std::vector<std::vector <std::string> > 
			mLocalCategoryName;
		int mLocalSelectedScheme;

		//int mCurrentDefaultCatalogScheme;   // must be in range [0,mNumberOfExisting...)
		int mLocalDefaultCatalogScheme;   //***1.95 - must be in range [0,mNumberOfLocalSchemes)


		CatEntryDialog *mEntryDialog; //***1.95 - for modifying Schemes

		GtkWidget* createCatalogSchemeDialog();
};

#endif

