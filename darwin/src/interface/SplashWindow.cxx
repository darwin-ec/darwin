//*******************************************************************
//   file: SplashWindow.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (2007)
//         -- linked back to main window so Splash stays on top until 
//            database is completely loaded.
//
// Much of this code is from Active Contour code from USF and other 
// sources.
//
//*******************************************************************

#include "../support.h"
#include "SplashWindow.h"
#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#include "../../pixmaps/logo.xpm"

gboolean on_splashWindow_delete_event(
		GtkWidget *widget,
		GdkEvent *event,
		gpointer userData);

gboolean splashWindow_timeout(
		gpointer userData);

using namespace std;

//static const int SPLASH_TIMEOUT_MILLISECS = 2000; moved to header

SplashWindow::SplashWindow()
	: mWindow(createSplashWindow()),
	  mMainWin(NULL) //***1.85
{ }

SplashWindow::~SplashWindow()
{
	gtk_widget_destroy(mWindow);
}

void SplashWindow::show()
{
	gtk_widget_show_now(mWindow);
	syncDisplay();
}

void SplashWindow::startTimeout(int ms)
{
	gtk_timeout_add(ms,
			splashWindow_timeout,
			(void *) this);
}

void SplashWindow::mwDone(MainWindow *mw) //***1.85 - new function
{
	mMainWin = mw;
}

void SplashWindow::updateStatus(const string &msg)
{
	gdk_window_raise(mWindow->window);
	gtk_label_set(GTK_LABEL(mStatusLabel), msg.c_str());
	syncDisplay();
}

void SplashWindow::syncDisplay()
{
	while (gtk_events_pending())
		gtk_main_iteration();
	gdk_flush();
}

GtkWidget* SplashWindow::createSplashWindow()
{
  GtkWidget *splashWindow;
  GtkWidget *splashVBox;
  GtkWidget *splashPixmap;

  splashWindow = gtk_window_new(WINDOW_DIALOG);
  gtk_object_set_data (GTK_OBJECT (splashWindow), "splashWindow", splashWindow);
  gtk_window_set_title (GTK_WINDOW (splashWindow), _("DARWIN starting..."));
  gtk_window_set_position (GTK_WINDOW (splashWindow), GTK_WIN_POS_CENTER);
  gtk_window_set_modal(GTK_WINDOW (splashWindow), TRUE);
  gtk_window_set_policy(GTK_WINDOW (splashWindow), FALSE, FALSE, FALSE);
  gtk_window_set_wmclass(GTK_WINDOW (splashWindow), "darwin_splash", "DARWIN");

  splashVBox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(splashVBox);
  gtk_container_add(GTK_CONTAINER (splashWindow), splashVBox);

  splashPixmap = create_pixmap_from_data(splashWindow, logo_xpm);
  gtk_widget_show (splashPixmap);
  gtk_box_pack_start (GTK_BOX (splashVBox), splashPixmap, TRUE, TRUE, 0);

  mStatusLabel = gtk_label_new(_("Starting up..."));
  gtk_widget_show(mStatusLabel);
  gtk_box_pack_start(GTK_BOX(splashVBox), mStatusLabel, FALSE, FALSE, 0);

  gtk_signal_connect (GTK_OBJECT (splashWindow), "delete_event",
                      GTK_SIGNAL_FUNC (on_splashWindow_delete_event),
                      (void *) this);

  return splashWindow;
}

gboolean on_splashWindow_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	SplashWindow *win = (SplashWindow *) userData;

	if (NULL == win)
		return FALSE;

	delete win;

	return TRUE;
}

gboolean splashWindow_timeout(
	gpointer userData)
{
	SplashWindow *win = (SplashWindow *) userData;
	if (NULL == win->mMainWin)
	{
		win->startTimeout(500);
		//win->syncDisplay();
		return FALSE;
	}

	win->mMainWin->show(); //***1.85 - show main window ONLY after we are ready

	delete (SplashWindow *) userData;

	return FALSE;
}
