//*******************************************************************
//   file: DataExportDialog.h
//
// author: by J H Stewman (5/15/2007)
//
// This dialog enables selective export of data from the database
// in <tab> separated spreadsheet format to file or possibly
// to a printer for printing.
//
//*******************************************************************


#include <gtk/gtk.h>
#include "GtkCompat.h"
#include "../Support.h"

#include "../Database.h"
#include "../Options.h"

#ifndef DATAEXPORTDIALOG_H
#define DATAEXPORTDIALOG_H

int getNumDataExportDialogReferences();

class MainWindow; // forward declaration to prevent circular loads

class DataExportDialog
{
public :

	typedef enum {
		saveToFile,
		sendToPrinter,
	} operationType;

	DataExportDialog(
		Database *db,
		MainWindow *mainWin,
		Options *o,
		operationType opType);

	~DataExportDialog();

	GtkWidget* createDataExportDialog();

	void run_and_respond();

	bool saveData();

	bool printData();

	friend void on_mDataFieldButton_toggled(
				GtkButton *button,
				gpointer userData);

	friend void on_dataFieldCheckButtonAll_clicked(
				GtkButton *button,
				gpointer userData);

	friend void on_dataFieldCheckButtonClear_clicked(
				GtkButton *button,
				gpointer userData);

	friend void on_dataFieldOrigImgButton_toggled(
				GtkButton *button,
				gpointer userData);

	friend void on_dataFieldModImgButton_toggled(
				GtkButton *button,
				gpointer userData);

	friend void on_radioOutToFile_toggled(
				GtkButton *button,
				gpointer userData);

	friend void on_radioOutToPrinter_toggled(
				GtkButton *button,
				gpointer userData);

	friend void on_data_export_CNX_button_clicked(
				GtkButton *button,
				gpointer userData);

	friend void on_data_export_OK_button_clicked(
				GtkButton *button,
				gpointer userData);

 
private :

	MainWindow
		*mMainWin;           // the MainWindow (our parent)

	Database
		*mDatabase;          // link to the CURRENTLY OPEN database

	Options
		*mOptions;           // link to global Options

	operationType
		mOpType;             // what kind of export (to file, to printer, ...)

	GtkWidget
		*mWindow,            // this object's WINDOW
		*mModImgButton,      // button for include/exclude original image name
		*mOrigImgButton;     // button for include/exclude modified image name
		
	std::vector<GtkWidget *>
		mDataFieldButton;    // data field buttons
	
	std::vector<std::string>
		mDataFieldName;      // displayed data field labels (names)

	int 
		mDataFieldsSelected; // # of data fields selected for export

	std::vector<bool> 
		mDataFieldToUse;    // true / false - use this data field in export

	std::string
		mModImgFilename,     // actual name of original image file
		mOrigImgFilename;    // actual name of modified image file
};

#endif // DATAEXPORTDIALOG_H

