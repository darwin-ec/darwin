//*******************************************************************
//   file: OptionsDialog.h
//
// author: Adam Russell
//
//   mods: J H Stewman (2007)
//         -- user selectable fonts
//
//*******************************************************************

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <gtk/gtk.h>
#include "GtkCompat.h"


#include "MainWindow.h"
#include "../Options.h"

int getNumOptionsDialogReferences();

class OptionsDialog
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.
		OptionsDialog(MainWindow *m, Options *o);
		
		// Destructor
		// 	Destroys the dialog if it's open and frees
		// 	resources.
		~OptionsDialog();

		// show
		// 	Simply draws the dialog on the screen.
		void show();

		// GTK+ callback functions

		friend gboolean on_optionsDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_colorButtonChoose_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_optionsButtonOK_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_optionsButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);
		
		friend gboolean on_colorSelectionDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_colorSelectOKButton_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_colorSelectCancelButton_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_colorSelectHelpButton_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_fontSelectButton_clicked( //***1.85
				GtkButton *button,
				gpointer userData);

		friend void on_fontSelectDialog_OK_Button_clicked( //***1.85
				GtkButton *button,
				gpointer userData);

		friend void on_fontSelectDialog_CNX_Button_clicked( //***1.85
				GtkButton *button,
				gpointer userData);

	private:
		GtkWidget
			*mDialog,
			*mColorSelectionDialog,

			// radio buttons
			*mToolbarRadioButtonText,
			*mToolbarRadioButtonPictures,
			*mToolbarRadioButtonBoth,

			//***1.65 - dolphin ID related radio buttons - primarily for testing
			*mShowIDsButton,
			*mHideIDsButton,

			// text entries
			*mACIterationsSpinButton,
			*mACContinuitySpinButton,
			*mACLinearitySpinButton,
			*mACEdgeSpinButton,
			
			*mCEHighThresholdSpinButton,
			*mCELowThresholdSpinButton,
			*mCEGaussianStdDevSpinButton,
			
			*mFontName,            //***1.85
			*mFontSample,          //***1.85
			*mFontSelectionDialog; //***1.85

		double mColor[4];

		MainWindow *mMainWin;
		Options *mOptions;

		GtkWidget *createOptionsDialog(Options *o);
		GtkWidget *createColorSelectionDialog();
};

#endif
