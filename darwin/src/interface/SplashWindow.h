//*******************************************************************
//   file: SplashWindow.h
//
// author: Adam Russell
//
//   mods:
// 
//*******************************************************************

#ifndef SPLASHWINDOW_H
#define SPLASHWINDOW_H

#include <gtk/gtk.h>
#include "GtkCompat.h"
#include "../interface/MainWindow.h"
#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

#define SPLASH_TIMEOUT_MILLISECS    2000

class SplashWindow
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the window.  show() must later be
		// 	called to do that.
		SplashWindow();
		
		// Destructor
		// 	Destroys the window if it's open and frees
		// 	resources.
		~SplashWindow();

		// show
		// 	Simply draws the window on the screen.
		void show();

		void mwDone(MainWindow *mainWin); //***1.85 - used by main window to notify that it is done

		void startTimeout(int ms = SPLASH_TIMEOUT_MILLISECS); //***1.85 - added parameter
		void updateStatus(const std::string &msg);

		// GTK+ callback functions
		friend gboolean on_splashWindow_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend gboolean splashWindow_timeout(
				gpointer userData);

	private:
		GtkWidget
			*mWindow,
			*mStatusLabel;

		MainWindow *mMainWin; //***1.85

		void syncDisplay();
		GtkWidget *createSplashWindow();
};

#endif
