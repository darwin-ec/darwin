//*******************************************************************
//   file: SaveFileChooserDialog.h
//
// author: J H Stewman
//
//   mods:
//
//*******************************************************************

#ifndef SAVEFILECHOOSERDIALOG_H
#define SAVEFILECHOOSERDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "../DatabaseFin.h"
#include "TraceWindow.h"

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include <vector>

int getNumSaveFileChooserDialogReferences();

class SaveFileChooserDialog
{
	public:

		enum {//!!!IF you edit this, update mNumberModes in the cxx file!!!!
			saveFin = 0,
			saveFinz,
			exportDatabase,
			exportDataFields,
			saveMultipleFinz,
			saveFullSizeModImages, //***2.02
			chooseDataPath //***2.22
		} ;

		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.
		//
		// 	Note: Only does a pointer copy of dbFin.  Also,
		// 	will delete dbFin when done!
		SaveFileChooserDialog(
				Database *db,
				DatabaseFin<ColorImage> *dbFin, 
				MainWindow *mainWin,
				TraceWindow *traceWin,
				Options *o,
				GtkWidget *parent,
				int saveMode,
				std::vector<DatabaseFin<ColorImage>* > *fins = NULL);

		// Destructor
		// 	Destroys the dialog if it's open and frees
		// 	resources.
		~SaveFileChooserDialog();

		bool run_and_respond(); // replaces show

		static void clearLast(std::string mPreviousSurveyArea); //clear gLast variables if they contain previous survey areas

		// GTK+ callback functions

		friend gboolean on_saveFileChooser_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_saveFileChooserButtonOK_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_saveFileChooserButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_saveFileChooserFileSelections_changed(
				GtkWidget *widget,
				gpointer userData);

		friend void on_saveFileChooserDirectory_changed(
				GtkWidget *widget,
				gpointer userData);

	private:
		GtkWidget 
			*mDialog,
			*mParent;


		MainWindow 
			*mMainWin;

		TraceWindow 
			*mTraceWin; // the parent, to be deleted after successful save
		
		DatabaseFin<ColorImage> 
			*mFin;
				
		Database 
			*mDatabase;
				
		Options 
			*mOptions;

		int 
			mSaveMode; // openFinImage, openFinTrace or openDatabase

		std::vector<DatabaseFin<ColorImage>* >
			*mFins;

		GtkWidget *createSaveFileChooser();

};

#endif
