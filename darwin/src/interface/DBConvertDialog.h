//*******************************************************************
//   file: DBConvertDialog.h
//
// author: by J H Stewman (7/10/2008)
//
// This dialog enables conversion of an old Version 3 database to 
// an SQLite database.  Options include full backup, copy of old
// database as "*.olddb" and / or retreat without change.
//
//*******************************************************************


#include <gtk/gtk.h>
#include "GtkCompat.h"
#include "../Support.h"

#include "../CatalogSupport.h" // includes all Database types
#include "../Options.h"

#ifndef DBCONVERTDIALOG_H
#define DBCONVERTDIALOG_H

int getNumDBConvertDialogReferences();

class MainWindow; // forward declaration to prevent circular loads

class DBConvertDialog
{
public :

	typedef enum {
		dbBackup = 0,   // do not change value
		imgBackup = 1,  // do not change value
		toSQLite,
		toMySQL,
	} operationType;

	DBConvertDialog(
		MainWindow *mainWin,
		std::string oldDBFilename,
		Database **dbPtr);

	~DBConvertDialog();

	GtkWidget* createDBConvertDialog();

	void run_and_respond();

	bool convert2SQLite(int degreeOfBackup);
	bool convert2MySQL(int degreeOfBackup);

	friend void on_mActionButton_toggled(
				GtkButton *button,
				gpointer userData);

	friend void on_radioConvert2SQLite_toggled(
				GtkButton *button,
				gpointer userData);

	friend void on_radioConvert2MySQL_toggled(
				GtkButton *button,
				gpointer userData);

	friend void on_dbConvert_CNX_button_clicked(
				GtkButton *button,
				gpointer userData);

	friend void on_dbConvert_OK_button_clicked(
				GtkButton *button,
				gpointer userData);

 
private :

	MainWindow
		*mMainWin;           // the MainWindow (our parent)

	std::string
		mDBFilename;         // filename of database we are going to convert

	Database
		**mDBptr;               // used to return pointer to database converted/loaded

	operationType
		mOpType;             // what kind of conversion (toSQlite, toMySQL, to)

	GtkWidget
		*mWindow;            // this object's WINDOW
		
	std::vector<GtkWidget *>
		mActionButton;       // preliminary (backup) action
	
	std::vector<bool> 
		mActionToUse;        // true / false - perform this action

};

#endif // DBCONVERTDIALOG_H

