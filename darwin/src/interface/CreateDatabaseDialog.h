//*******************************************************************
//   file: CreateDatabaseDialog.h
//
// author: J H Stewman (5/1/2007)
//
// used to create new databases and survey area folders
//
//*******************************************************************

#ifndef CREATE_DATABASE_DIALOG_H
#define CREATE_DATABASE_DIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "../Options.h"
#include "MainWindow.h" // ***1.85

int getNumCreateDatabaseDialogReferences();

class CreateDatabaseDialog
{
	public:

		enum {
			createNewSurveyArea = 0,
			createNewDatabaseOnly
		};

		CreateDatabaseDialog(MainWindow *mainWin, Options *o, string archiveFilename);
		
		~CreateDatabaseDialog();

		void show();

		// GTK+ callback functions

		friend gboolean on_createDbDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend gboolean on_createDbButtonCNX_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_createDbButtonOK_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_createDbSurveyAreaCList_select_row(
				GtkCList *clist,
				gint row,
				gint column,
				GdkEvent *event,
				gpointer userData);

		// following two callbacks probably NOT needed

		friend void on_createDbDatabaseNameEntry_changed(
				GtkEntry *etnry,
				gpointer userData);

		friend void on_createDbSurveyAreaNameEntry_changed(
				GtkEntry *entry,
				gpointer userData);

		friend void on_radioNewSurveyAreaAndDb_clicked(
				GtkRadioButton *entry,
				gpointer userData);

		friend void on_radioNewDbOnly_clicked(
				GtkRadioButton *entry,
				gpointer userData);


	private:

		// class pointer to the CreateDatabaseDialog and parent widgets
		GtkWidget 
			*mDialog,
			*mParentWindow,
			*mSurveyAreaList,
			*mDatabaseList,
			*mNewSurveyAreaName,
			*mNewDatabaseName,
			*mUsingScheme;

		MainWindow *mMainWin;

		GtkTextBuffer *mBuffer; // for entering new category names

		Options *mOptions;

		int 
			mSelectedArea, // position in CList
			mNumDatabases, // number of currently defined databases in selected survey area
			mCreateMode;   // what to do

		std::string mArchiveName;

		GtkWidget* createCreateDatabaseDialog();
};

#endif

