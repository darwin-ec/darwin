//*******************************************************************
//   file: MatchResultsWindow.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman
//         -- comment blocks added
//         -- major changes to user interface
//
//*******************************************************************

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif
#include <gdk/gdkkeysyms.h>
#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

#include "MatchResultsWindow.h"
#include "NoMatchWindow.h"
#include "MappedContoursDialog.h"
//#include "ErrorDialog.h"
#include "ImageViewDialog.h"
#include "../mapContour.h"
#include "../support.h"
#include "../image_processing/transform.h"
#include "SaveFileSelectionDialog.h" //***1.4

#pragma warning (disable : 4305 4309)
#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#include "../../pixmaps/cancel.xpm"
#include "../../pixmaps/fin.xpm"
#include "../../pixmaps/magnify_cursor.xbm"
#include "../../pixmaps/next.xpm"
#include "../../pixmaps/previous.xpm"
#include "../../pixmaps/view_icons.xpm"
#include "../../pixmaps/view_list.xpm"
#include "../feature.h"  //***005CM

static const int FIN_IMAGE_WIDTH = 400; // was 320
static const int FIN_IMAGE_HEIGHT = 300; // was 240
static const int POINT_SIZE = 1;
static const int TABLE_COLS = 4; // was 3 prior to version 1.0

gboolean on_matchResultsWindow_delete_event(
						GtkWidget *widget,
						GdkEvent *event,
						gpointer userData);

void on_mrRadioButtonIcons_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

void on_mrButtonAltID_toggled( //***1.6 - to hide or show real ID's
				GtkToggleButton *togglebutton,
				gpointer userData);

void on_mrButtonShowInfo_toggled( //***1.6 - to hide or show info
				GtkToggleButton *togglebutton,
				gpointer userData);
	
void on_mrRadioButtonList_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

void on_mMRCList_click_column(
				GtkCList *clist,
				gint column,
				gpointer userData);

void on_mMRCList_select_row(
				GtkCList *clist,
				gint row,
				gint column,
				GdkEvent *event,
				gpointer userData);

void on_mMRButtonPrev_clicked(
				GtkButton *button,
				gpointer userData);

void on_mMRButtonNext_clicked(
				GtkButton *button,
				gpointer userData);

void on_mMRButtonSlideShow_clicked( //***1.85 - new
				GtkButton *button,
				gpointer userData);

void on_mrButtonFinsMatch_clicked(
				GtkButton *button,
				gpointer userData);

void on_mrButtonNoMatch_clicked(
				GtkButton *button,
				gpointer userData);

void on_mMRButtonSelectedMod_clicked( //***1.2 - new
				GtkButton *button,
				gpointer userData);

void on_mMRButtonUnknownMod_clicked( //***1.2 - new
				GtkButton *button,
				gpointer userData);

void on_mMRButtonUnknownMorph_clicked( //***1.2 - new
				GtkButton *button,
				gpointer userData);

void on_mrButtonReturnToMatchingDialog_clicked(
				GtkButton *button,
				gpointer userData);

void on_mrButtonSaveResults_clicked( //***1.4 - new
				GtkButton *button,
				gpointer userData);

void on_mrButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);

gboolean on_eventBoxSelected_button_press_event(
				GtkWidget *widget,
				GdkEventButton *event,
				gpointer userData);

gboolean on_mDrawingAreaSelected_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

gboolean on_eventBoxUnknown_button_press_event(
				GtkWidget *widget,
				GdkEventButton *event,
				gpointer userData);

gboolean on_mDrawingAreaUnknown_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

gboolean on_eventBoxOutlines_button_press_event(
				GtkWidget *widget,
				GdkEventButton *event,
				gpointer userData);

gboolean on_mDrawingAreaOutlines_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

void on_finRadioButton_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

gboolean matchResultsSlideShowTimer(
				gpointer userData);


using namespace std;

//*******************************************************************
//
//
//
MatchResultsWindow::MatchResultsWindow(
		const DatabaseFin<ColorImage> *unknownFin,
		const MatchResults *results,
		Database *database,
		MainWindow *m,	                          //*** 004CL
		MatchingDialog *matchingDialog,
		MatchingQueueDialog *matchingQueueDialog, //***1.3
		string resultsFilename, //***1.6
		Options *o
)
	:
		mUnknownFin(new DatabaseFin<ColorImage>(unknownFin)),
		mSelectedFin(NULL),
		mMRView(MR_VIEW_LIST),
		mUnknownImage(NULL),
		mSelectedImage(NULL),
		mSelectedImageOriginal(NULL),    //***1.8
		mSelectedImageModOriginal(NULL), //***1.8
		mSelectedImageMod(NULL),         //***1.8
		mSelectedImageShown(NULL),       //***1.8
		mRegContour(NULL),
		mWindow(NULL),
		mMRCList(NULL),
		mResults(NULL),
		mCurEntry(0),
		mDatabase(database),
		mMainWin(m),				       //*** 004CL
		mMatchingDialog(matchingDialog),   //***043MA
		mReturningToMatchingDialog(false), //***043MA
		mMatchingQueueDialog(matchingQueueDialog), //***1.3
		mOptions(o),
		mGC1(NULL),
		mGC2(NULL),
		mCursor(NULL),
		mSelectedIsModified(true),          //***1.2
		mUnknownIsModified(true),           //***1.2
		mUnknownIsMorphed(false),           //***1.2
		mSaveMessage(resultsFilename),      //***1.6
		mLocalHideIDs(o->mHideIDs),         //***1.65
		mPrevSelImgHeight(FIN_IMAGE_HEIGHT),//***1.8
		mPrevSelImgWidth(FIN_IMAGE_WIDTH),  //***1.8
		mPrevUnkImgHeight(FIN_IMAGE_HEIGHT),//***1.8
		mPrevUnkImgWidth(FIN_IMAGE_WIDTH),   //***1.8
		mSlideShowOn(false) //***1.85

{
	if (NULL == unknownFin || NULL == results || NULL == database)
		throw Error("Empty argument in MatchResultsWindow constructor.");

	try {

		// create full size and resized copies of original unknown image
		mUnknownImageOriginal = new ColorImage(unknownFin->mFinImage);
		mUnknownImage = resizeWithBorder(unknownFin->mFinImage, FIN_IMAGE_HEIGHT, FIN_IMAGE_WIDTH);

		//***1.5 - try to load modified temp image for unknown

		// create full size and resized copies of user modified unknown image
		if (NULL != unknownFin->mModifiedFinImage)
		{
			mUnknownImageModOriginal = new ColorImage(unknownFin->mModifiedFinImage); //***1.5
			mUnknownImageMod = resizeWithBorder(unknownFin->mModifiedFinImage, FIN_IMAGE_HEIGHT, FIN_IMAGE_WIDTH); //***1.5
		}
		else
		{
			mUnknownImageModOriginal = new ColorImage(mUnknownImageOriginal);
			mUnknownImageMod = new ColorImage(mUnknownImage); // already resizedWithBorder
		}
		mUnknownImageShown = mUnknownImageMod; //***1.5 - show modified first

		mUnknownContour = NULL; //***1.0LK
		mWindow = createMatchResultsWindow();
		mResults = new MatchResults(*results);
		mResults->setRankings(); //***1.5 - sets rankings for all display during match results viewing
		MatchResultsWindow::updateList();

	} catch (...) {
		throw;
	}
}

//*******************************************************************
//
//
//
MatchResultsWindow::~MatchResultsWindow()
{
	if (mSlideShowOn)
		gtk_timeout_remove(mTimerID); // stop the timer function FIRST

	delete mUnknownFin;
	delete mSelectedFin;

	gtk_widget_destroy(GTK_WIDGET(mWindow));

	delete mUnknownImage;
	delete mUnknownImageOriginal;
	if (NULL != mUnknownImageModOriginal)
		delete mUnknownImageModOriginal; //***1.5
	if (NULL != mUnknownImageMod)
		delete mUnknownImageMod; //***1.5
	delete mSelectedImage;
	delete mSelectedImageOriginal; //***1.8
	if (NULL != mSelectedImageModOriginal)
		delete mSelectedImageModOriginal; //***1.8
	if (NULL != mSelectedImageMod)
		delete mSelectedImageMod; //***1.8
	delete mResults;

	// delete the MatchingDialog now, IF we are not returning to it
	if ((! mReturningToMatchingDialog) && (NULL != mMatchingDialog))
		delete mMatchingDialog; //***043MA

	// Just to be nice...
	mRadioButtonVector.clear();

	if (NULL != mGC1)
		gdk_gc_unref(mGC1);

	if (NULL != mGC2)
		gdk_gc_unref(mGC2);

	if (NULL != mCursor)
		gdk_cursor_destroy(mCursor);
}

//*******************************************************************
//
//
//
void MatchResultsWindow::show()
{
	gtk_widget_show(GTK_WIDGET(mWindow));

	updateGC();
	updateCursor();

	if (mResults->size() > 0)
		gtk_clist_select_row(GTK_CLIST(mMRCList), 0, 0);
	else {
		gtk_widget_set_sensitive(mMRButtonNext, FALSE);
		gtk_widget_set_sensitive(mMRButtonPrev, FALSE);
	}
}

//*******************************************************************
//
//
//
void MatchResultsWindow::updateList()
{
	if (NULL == mResults)
		return;

	unsigned numEntries = mResults->size();

	// Some variables for the pixmap display
	GdkPixmap *pixmap = NULL;
	GdkBitmap *mask = NULL;

	gtk_clist_freeze(GTK_CLIST(mMRCList));
	gtk_clist_clear(GTK_CLIST(mMRCList));

	//***1.85 - set font as currently selected 

	gtk_widget_modify_font(
			mMRCList,
			(pango_font_description_from_string(mOptions->mCurrentFontName.c_str())));

	for (unsigned i = 0; i < numEntries; i++) {
		Result *r = mResults->getResultNum(i);

		if (NULL == r->mThumbnailPixmap)
			create_gdk_pixmap_from_data(mMRCList, &pixmap, &mask, fin_xpm);
		else
			create_gdk_pixmap_from_data(
					mMRCList,
					&pixmap,
					&mask,
					r->mThumbnailPixmap);

		gchar *idCode, *name, *damage, *date, *location,*error;

		//***1.65 - hide IDs if needed
		if (mLocalHideIDs)
		{
			idCode = new gchar[5];
			strcpy(idCode, "****");
		}
		else if ("NONE" == r->getIdCode())
			idCode = NULL;
		else {
			idCode = new gchar[r->getIdCode().length() + 1];
			strcpy(idCode, r->getIdCode().c_str());
		}
			
		if ("NONE" == r->getName())
			name = NULL;
		else {
			name = new gchar[r->getName().length() + 1];
			strcpy(name, r->getName().c_str());
		}

		//***005CM
		/* temporarily replace the damage category with the match error
		*/
		if ("NONE" == r->getDamage())
			damage = NULL;
		else {
			damage = new gchar[r->getDamage().length() + 1];
			strcpy(damage, r->getDamage().c_str());
		}

		// put the match error measure in the damage list for display
		// until we are sure match values are reasonable
		// put ranking as well - format 1: 235
		// 6 below is room for 4 digit rank plus colon and space
		error = new gchar[r->getError().length()+6+1];
		//***1.5 - only show rank when it is correct - i.e., when last sort was based on error
		/*if (mResults->LastSortedByError())
			sprintf(error, "%4d: ",i+1);
		else
			sprintf(error, "  xx: ");
		*/
		sprintf(error, "%s:", r->getRank().c_str()); //***1.5 - rank is now part of each result
		strcat(error, r->getError().c_str());
		//***005CM end of temporary change

		if ("NONE" == r->getDate())
			date = NULL;
		else {
			date = new gchar[r->getDate().length() + 1];
			strcpy(date, r->getDate().c_str());
		}

		if ("NONE" == r->getLocation())
			location = NULL;
		else {
			location = new gchar[r->getLocation().length() + 1];
			strcpy(location, r->getLocation().c_str());
		}

		gchar *itemInfo[7] = {
			NULL,
			idCode,
			name,
			date,
			location,
			damage,
			error
		};

		gtk_clist_append(GTK_CLIST(mMRCList), itemInfo);

		if (NULL != pixmap)
			gtk_clist_set_pixmap(
					GTK_CLIST(mMRCList),
					i,
					0,
					pixmap,
					mask);

		delete[] idCode;
		delete[] name;
		delete[] damage;
		delete[] date;
		delete[] location;
		delete[] error;

		if (NULL != pixmap)
			gdk_pixmap_unref(pixmap);

		if (NULL != mask)
			gdk_bitmap_unref(mask);
	}

	gtk_clist_thaw(GTK_CLIST(mMRCList));
}

//*******************************************************************
//
//***1.7 - this was copied from the MainWindow class
//
void MatchResultsWindow::selectFromCList(int newCurEntry)       //***004CL
{
	//gtk_clist_select_row(GTK_CLIST(mCList), newCurEntry, 0);

	//*** 1.7CL - all that follows

	const int lineHeight = DATABASEFIN_THUMB_HEIGHT + 1;
	static int lastEntry = 0;

	int pageEntries = mScrolledWindow->allocation.height / lineHeight;

	if ((int)mDatabase->size() == 0)
		return;

	// else force scrollable window to scroll down to clist entry

	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(mScrolledWindow));
	int topEntry = (int)(adj->value) / lineHeight;
		
	//g_print("top/last/this = (%d %d %d)\n", topEntry, lastEntry,newCurEntry);

	if ((newCurEntry > topEntry + pageEntries - 2) || (newCurEntry < topEntry))
	{
		// NOTE: the (-2) in the test above prevents highlighting of partially visible
		// item at bottom of page when pressing NEXT button

		if ((lastEntry + 1 == newCurEntry) && 
			(newCurEntry > topEntry) && (newCurEntry == topEntry + pageEntries - 1))
			adj->value += lineHeight; // just scroll down one line
		else
		{
			// reposition list so newCurEntry is at top
			float where = (double)newCurEntry / (int)mResults->size();//mDatabase->size();
			adj->value = where * (adj->upper - adj->lower);
			//if (adj->value > adj->upper - adj->page_size)
			//	adj->value = adj->upper - adj->page_size;
			topEntry = newCurEntry;
		}

		//g_print("scroll value = %5.2f (%d %d)\n",
		//	adj->value,
		//	(newCurEntry-topEntry)*lineHeight,
		//	mScrolledWindow->allocation.height);

		// this should be called but seems to be meaningless and scroll update
		// works without it - JHS
		//gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(mScrolledWindow),adj);

		gtk_adjustment_value_changed(adj);
	}
	lastEntry = newCurEntry;
}

//*******************************************************************
//
//***1.7 - this was copied from the MainWindow class
//
void MatchResultsWindow::selectFromReorderedCList(std::string filename)
{  //***004CL
	if (NULL == mDatabase)
		return;
	try {
		unsigned numEntries = mDatabase->size();
                unsigned i = 0;
                bool found = false;
		while ((i < numEntries) && (!found)) {
			DatabaseFin<ColorImage> *fin = mDatabase->getItem(i);
			if (fin->mImageFilename == filename){
				found = true;
				gtk_clist_select_row(GTK_CLIST(mMRCList), i, 0); //***1.7
				//selectFromCList(i);
			}
			delete fin; //***1.0LK - this fin is only used to find position in clist
			i++;
		}
	} catch (Error e) {
		showError(_("The database seems to be corrupted.\n"
			  "Some (or all) entries may not appear\n"
			  "correctly."));
	}

}

//*******************************************************************
//
//
//
void MatchResultsWindow::updateCursor()
{
	GdkBitmap *bitmap, *mask;
	GdkColor white = {0,0xFFFF,0xFFFF,0xFFFF};
	GdkColor black = {0,0x0000,0x0000,0x0000};

	if (NULL != mCursor)
		gdk_cursor_destroy(mCursor);

        bitmap = gdk_bitmap_create_from_data(NULL,
                      magnify_cursor, magnify_cursor_width,
                      magnify_cursor_height);
        mask = gdk_bitmap_create_from_data(NULL,
                      magnify_mask, magnify_cursor_width,
                      magnify_cursor_height);
        mCursor = gdk_cursor_new_from_pixmap(
                      bitmap, mask, &black, &white,
                      magnify_xhot, magnify_yhot);

	if (NULL == mCursor)
		return;

	// I'm paranoid, what can I say?
	if (NULL != mDrawingAreaSelected)
		gdk_window_set_cursor(mDrawingAreaSelected->window, mCursor);

	if (NULL != mDrawingAreaUnknown)
		gdk_window_set_cursor(mDrawingAreaUnknown->window, mCursor);

	if (NULL != mDrawingAreaOutlines)
		gdk_window_set_cursor(mDrawingAreaOutlines->window, mCursor);
}

//*******************************************************************
//
//
//
void MatchResultsWindow::updateGC()
{
	if (NULL == mGC1)
		mGC1 = gdk_gc_new(mDrawingAreaOutlines->window);

	if (NULL == mGC2)
		mGC2 = gdk_gc_new(mDrawingAreaOutlines->window);

	//***006CM - separate display colors added
	double color1[] = {1.0, 0.0, 0.0, 0.0};
	updateGCColor(mGC1, color1); // graphics context for unknown fin
	double color2[] = {0.0, 0.0, 1.0, 0.0};
	updateGCColor(mGC2, color2); // graphics context for unknown fin
}

//*******************************************************************
//
//
//
void MatchResultsWindow::updateGCColor(GdkGC *gc, double color[4])
{
	if (NULL == gc)
		return;

	GdkColormap *colormap;
	GdkColor gdkColor;

	gdkColor.red = (gushort)(0xFFFFF * color[0]);
	gdkColor.green = (gushort)(0xFFFFF * color[1]);
	gdkColor.blue = (gushort)(0xFFFFF * color[2]);

	colormap = gdk_colormap_get_system();
	gdk_color_alloc(colormap, &gdkColor);

	gdk_gc_set_foreground(gc, &gdkColor);
}

//*******************************************************************
//
//
//
void MatchResultsWindow::updateGCColor(GdkGC *gc)
{
		updateGCColor(gc, mOptions->mCurrentColor);
}

//*******************************************************************
//
//
//
void MatchResultsWindow::refreshSelectedFin()
{
	on_mDrawingAreaSelected_expose_event(
			mDrawingAreaSelected,
			NULL,
			(void *) this);
}

//*******************************************************************
//
//
//
void MatchResultsWindow::refreshUnknownFin()
{
	on_mDrawingAreaUnknown_expose_event(
		mDrawingAreaUnknown,
		NULL,
		(void *) this);
}

//*******************************************************************
//
//
//
void MatchResultsWindow::refreshOutlines()
{
	on_mDrawingAreaOutlines_expose_event(
			mDrawingAreaOutlines,
			NULL,
			(void *) this);
}

//*******************************************************************
//
// MAJOR changes to layout in version 1.75
//
GtkWidget* MatchResultsWindow::createMatchResultsWindow()
{
    GtkWidget *matchResultsWindow;
	GtkWidget *mrButtonAltID; //***1.6
	GtkWidget *mrButtonShowInfo; //***1.6

    //GtkWidget *hbuttonbox1;
    guint mMRButtonPrev_key;
    guint mMRButtonNext_key;
    GtkWidget *frameRight;
    GtkWidget *table;
    GtkWidget *vButtonBox;
    GtkWidget *mrButtonFinsMatch;
    GtkWidget *mrButtonNoMatch;
	GtkWidget *mrButtonReturnToMatchingDialog; //***043MA
    guint mrButtonCancel_key;
    GtkWidget *mrButtonCancel;
    GtkWidget *frameSelected;
    GtkWidget *eventBoxSelected;
    GtkWidget *frameUnknown;
    GtkWidget *eventBoxUnknown;
    GtkWidget *frameOutlines;
    GtkWidget *eventBoxOutlines;
    GtkAccelGroup *accel_group;
    GtkTooltips *tooltips;
	GtkWidget *tmpLabel, *tmpBox, *tmpIcon;

    tooltips = gtk_tooltips_new();

    accel_group = gtk_accel_group_new();

    matchResultsWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_object_set_data(GTK_OBJECT(matchResultsWindow),
			"matchResultsWindow", matchResultsWindow);

	string titleString = "Matching Results";
	if ("" != this->mSaveMessage) //***1.9 - results were loaded from file
		titleString += " - " + this->mSaveMessage; // so append filename to title
	gtk_window_set_title(GTK_WINDOW(matchResultsWindow),
				_(titleString.c_str()));

    gtk_window_set_policy(GTK_WINDOW(matchResultsWindow), TRUE, TRUE,
			  FALSE);
    gtk_window_set_wmclass(GTK_WINDOW(matchResultsWindow),
			   "darwin_matchresults", "DARWIN");
	gtk_window_set_position(GTK_WINDOW(matchResultsWindow), GTK_WIN_POS_CENTER); //***1.8
	gtk_window_set_default_size(GTK_WINDOW(matchResultsWindow), 1024, 600); //***1.7
	gtk_window_set_keep_above(GTK_WINDOW(matchResultsWindow),TRUE); //*** 2.2 - keep above window that got us here

	//***1.7 - paned window is no more, and left frame is also gone
	//         the TOOLBAR is gone, LIST VIEW and ICON VIEW are gone

	// create MAIN frame

    frameRight = gtk_frame_new(NULL);
    gtk_widget_show(frameRight);
    gtk_container_add(GTK_CONTAINER(matchResultsWindow), frameRight);
    gtk_container_set_border_width(GTK_CONTAINER(frameRight), 1);
    gtk_frame_set_shadow_type(GTK_FRAME(frameRight), GTK_SHADOW_IN);

	// create TABLE (2x2) to fit in frame

    table = gtk_table_new(2, 2, FALSE);
    gtk_widget_show(table);
    gtk_container_add(GTK_CONTAINER(frameRight), table);

	// create scrollable window for displaying LIST VIEW of database fins

    mScrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_show(mScrolledWindow);
    gtk_scrolled_window_set_policy(
			GTK_SCROLLED_WINDOW(mScrolledWindow),
			//GTK_POLICY_NEVER, 
			GTK_POLICY_AUTOMATIC, //***1.85 - so side to side scrolling allowed
			GTK_POLICY_ALWAYS);
	MatchResultsWindow::createMRCList();
    gtk_container_add(GTK_CONTAINER(mScrolledWindow), mMRCList);

	// attach scrolling list of results (lower-left of table)

    gtk_table_attach(GTK_TABLE(table), mScrolledWindow, 0, 1, 1, 2,
		     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		     (GtkAttachOptions) (GTK_FILL), 5, 2); // was GTK_FILL x 2

	// create a BOX for list management buttons

	GtkWidget *vButtonBoxListOps = gtk_vbutton_box_new(); //***1.75
	gtk_widget_show(vButtonBoxListOps);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(vButtonBoxListOps),
			      GTK_BUTTONBOX_SPREAD);

	//***1.75 - create non-toolbar toggle button to allow hiding info fields 
	//          (date, location, category, rank)
	
	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, view_list_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new(_("Info"));
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	mrButtonShowInfo = gtk_toggle_button_new();
	gtk_container_add(GTK_CONTAINER(mrButtonShowInfo), tmpBox);
    gtk_widget_show(mrButtonShowInfo);
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(mrButtonShowInfo), FALSE);
    gtk_tooltips_set_tip(tooltips, mrButtonShowInfo,
			 _("Show / Hide information fields (date, location, category, rank)"), NULL);
	gtk_container_add(GTK_CONTAINER(vButtonBoxListOps), mrButtonShowInfo);

	//***1.75 - create non-toolbar toggle button to allow replacing real ID's with random ID's 
	//          extracted from root part of fin filename 

    if (! mOptions->mHideIDs) //***1.65
		mrButtonAltID = gtk_toggle_button_new_with_label(_("Hide ID's"));
	else //***1.65 - set up button based on global option
		mrButtonAltID = gtk_toggle_button_new_with_label(_("Show ID's"));

    gtk_button_set_relief (GTK_BUTTON (mrButtonAltID), GTK_RELIEF_NORMAL);
	gtk_widget_show(mrButtonAltID);
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(mrButtonAltID), FALSE);
	gtk_tooltips_set_tip(tooltips, mrButtonAltID,
			_("Click here to Show / Hide Fin ID's."), NULL);
	gtk_container_add(GTK_CONTAINER(vButtonBoxListOps), mrButtonAltID);

    if (! mOptions->mHideIDs) //***1.65
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mrButtonAltID), FALSE); // showing IDs
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mrButtonAltID), TRUE); // hiding IDs

	// create PREVIOUS fin button

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, previous_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

    mMRButtonPrev = gtk_button_new();
    mMRButtonPrev_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
			      _("_Previous"));
    gtk_widget_add_accelerator(mMRButtonPrev, "clicked", accel_group,
			       mMRButtonPrev_key, GDK_MOD1_MASK,
			       (GtkAccelFlags) 0);
	gtk_container_add(GTK_CONTAINER(mMRButtonPrev), tmpBox);
    gtk_widget_show(mMRButtonPrev);
    gtk_container_add(GTK_CONTAINER(vButtonBoxListOps), mMRButtonPrev); //***1.75 - moved to top of list
    GTK_WIDGET_SET_FLAGS(mMRButtonPrev, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, mMRButtonPrev,
			 _("Cycle to the previous match result."), NULL);
    gtk_widget_add_accelerator(mMRButtonPrev, "clicked", accel_group,
			       GDK_P, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

	// create NEXT fin button

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, next_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

    mMRButtonNext = gtk_button_new();
    mMRButtonNext_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
			      _("_Next"));
    gtk_widget_add_accelerator(mMRButtonNext, "clicked", accel_group,
			       mMRButtonNext_key, GDK_MOD1_MASK,
			       (GtkAccelFlags) 0);
	gtk_container_add(GTK_CONTAINER(mMRButtonNext), tmpBox);
    gtk_widget_show(mMRButtonNext);
    gtk_container_add(GTK_CONTAINER(vButtonBoxListOps), mMRButtonNext); //***1.75 - moved to top of list
    GTK_WIDGET_SET_FLAGS(mMRButtonNext, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, mMRButtonNext,
			 _("Cycle to the next match result."), NULL);
    gtk_widget_add_accelerator(mMRButtonNext, "clicked", accel_group,
			       GDK_N, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

	//***1.85 - create SLIDE_SHOW button

    mMRButtonSlideShow = gtk_button_new_with_label(_("_Scroll[off]"));
	gtk_button_set_use_underline(GTK_BUTTON(mMRButtonSlideShow), TRUE); // use _S as mnemonic
    gtk_widget_show(mMRButtonSlideShow);
    gtk_container_add(GTK_CONTAINER(vButtonBoxListOps), mMRButtonSlideShow);
    GTK_WIDGET_SET_FLAGS(mMRButtonSlideShow, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, mMRButtonSlideShow,
			 _("Start or stop an automatically scrolling\nslide show of selected fins"), NULL);

	//create HBOX to hold all buttons and outlines (lower-right of table)

	GtkWidget *dink = gtk_hbox_new(FALSE,5); //***1.75
	gtk_widget_show(dink);                   //***1.75
    gtk_table_attach(GTK_TABLE(table), dink, 1, 2, 1, 2,
		     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),  //***1.7 was GTK_EXPAND
		     (GtkAttachOptions) (GTK_SHRINK), 5, 2);      //***1.7 was GTK_EXPAND

	// attach list management buttons

    gtk_container_add(GTK_CONTAINER(dink), vButtonBoxListOps);

	// create REGISTERED OUTLINES display

	frameOutlines = gtk_frame_new(_("Registered Outlines"));
    gtk_widget_show(frameOutlines);
	gtk_container_add(GTK_CONTAINER(dink), frameOutlines); //***1.75 - move to bottom left of table

    gtk_container_set_border_width(GTK_CONTAINER(frameOutlines), 5);

    eventBoxOutlines = gtk_event_box_new();
    gtk_widget_show(eventBoxOutlines);
    gtk_container_add(GTK_CONTAINER(frameOutlines), eventBoxOutlines);
    gtk_tooltips_set_tip(tooltips, eventBoxOutlines,
			 _
			 ("Left click to see a larger view of fin outlines."),
			 NULL);

    mDrawingAreaOutlines = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(mDrawingAreaOutlines),
					FIN_IMAGE_WIDTH/4, FIN_IMAGE_HEIGHT/4);
    gtk_widget_show(mDrawingAreaOutlines);
    gtk_container_add(GTK_CONTAINER(eventBoxOutlines),
		      mDrawingAreaOutlines);

	// create button box for match results ACTION buttons

    vButtonBox = gtk_vbutton_box_new();
    gtk_widget_show(vButtonBox);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(vButtonBox),
			      GTK_BUTTONBOX_SPREAD);
	gtk_container_add(GTK_CONTAINER(dink), vButtonBox); //***1.75 - move to bottom left of table

	// create FINS MATCH button

    mrButtonFinsMatch = gtk_button_new_with_label(_("Matches Selected Fin"));
    gtk_widget_show(mrButtonFinsMatch);
    gtk_container_add(GTK_CONTAINER(vButtonBox), mrButtonFinsMatch);
    GTK_WIDGET_SET_FLAGS(mrButtonFinsMatch, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, mrButtonFinsMatch,
			 _
			 ("The selected fin matches the unknown fin.  "
			 "Add the unknown fin image to the database as an alternate "
			 "image of the selected fin. Save sighting data, if desired."),
			 NULL);

	// create NO MATCH button

    mrButtonNoMatch = gtk_button_new_with_label(_("No Match - New Fin"));
    gtk_widget_show(mrButtonNoMatch);
    gtk_container_add(GTK_CONTAINER(vButtonBox), mrButtonNoMatch);
    GTK_WIDGET_SET_FLAGS(mrButtonNoMatch, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, mrButtonNoMatch,
			 _
			 ("No fins match the unknown fin.  Add this unknown dolphin "
			 "to the database as a new entry. Save sighting data, if desired."),
			 NULL);

	// new button to RETURN us to the Matching Dialog

    mrButtonReturnToMatchingDialog = gtk_button_new_with_label(_("Return To Matching Dialog"));
	//***1.1 - only show button if MatchingDialog pointer is valid
    if (NULL != mMatchingDialog)
		gtk_widget_show(mrButtonReturnToMatchingDialog);
    gtk_container_add(GTK_CONTAINER(vButtonBox), mrButtonReturnToMatchingDialog);
    GTK_WIDGET_SET_FLAGS(mrButtonReturnToMatchingDialog, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, mrButtonReturnToMatchingDialog,
			 _
			 ("Return to the Matching Dialog, so another match technique can be initiated with same unknown."),
			 NULL);

	//***1.4 - new button to allow SAVING match results

    GtkWidget *mrButtonSaveResults = gtk_button_new_with_label(_("Save Match Results"));
	//***1.1 - only show button if MatchingDialog pointer is valid
	gtk_widget_show(mrButtonSaveResults);
    gtk_container_add(GTK_CONTAINER(vButtonBox), mrButtonSaveResults);
    GTK_WIDGET_SET_FLAGS(mrButtonSaveResults, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, mrButtonSaveResults,
			 _
			 ("Save the match results for future review or match confirmation."),
			 NULL);
	if ("" != mSaveMessage) //***1.91 - results came from file, so inactivate this option
		gtk_widget_set_sensitive(mrButtonSaveResults, FALSE);

	// create DONE button

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

    mrButtonCancel = gtk_button_new();
    mrButtonCancel_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
			      _("_Done")); //***1.6 - was called cancel
    gtk_widget_add_accelerator(mrButtonCancel, "clicked", accel_group,
			       mrButtonCancel_key, GDK_MOD1_MASK,
			       (GtkAccelFlags) 0);
	gtk_container_add(GTK_CONTAINER(mrButtonCancel), tmpBox);
    gtk_widget_show(mrButtonCancel);
    gtk_container_add(GTK_CONTAINER(vButtonBox), mrButtonCancel);
    GTK_WIDGET_SET_FLAGS(mrButtonCancel, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, mrButtonCancel,
			 _
			 ("Close this window and discard matching results."),
			 NULL);
    gtk_widget_add_accelerator(mrButtonCancel, "clicked", accel_group,
			       GDK_Escape, (GdkModifierType)0, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(mrButtonCancel, "clicked", accel_group,
			       GDK_C, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

	// create frame for SELECTED FIN image (upper-left of table)

    frameSelected = gtk_frame_new(_("Selected Fin"));
    gtk_widget_show(frameSelected);
    gtk_table_attach(GTK_TABLE(table), frameSelected, 0, 1, 0, 1,
		     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
    gtk_container_set_border_width(GTK_CONTAINER(frameSelected), 5);

	//***1.2 - new vbox to hold image and modifying buttons inside frame

	GtkWidget *theVBox = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(theVBox);
    gtk_container_add(GTK_CONTAINER(frameSelected), theVBox);

    eventBoxSelected = gtk_event_box_new();
    gtk_widget_show(eventBoxSelected);
	gtk_box_pack_start(GTK_BOX(theVBox), eventBoxSelected, TRUE, TRUE, 2);
    gtk_tooltips_set_tip(tooltips, eventBoxSelected,
			 _
			 ("Left click to see a larger view of this image."),
			 NULL);

    mDrawingAreaSelected = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(mDrawingAreaSelected),
					FIN_IMAGE_WIDTH, FIN_IMAGE_HEIGHT);
    gtk_widget_show(mDrawingAreaSelected);
    gtk_container_add(GTK_CONTAINER(eventBoxSelected), mDrawingAreaSelected);

	//***1.2 - new buttons for morphing or modifying images
	GtkWidget *theButtonBox;
	
	theButtonBox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(theButtonBox), GTK_BUTTONBOX_SPREAD);
    gtk_widget_show(theButtonBox);
	gtk_box_pack_start(GTK_BOX(theVBox), theButtonBox, FALSE, FALSE, 5);

	mMRButtonSelectedMod = gtk_button_new_with_label(_("Show Original Image"));
    gtk_widget_show(mMRButtonSelectedMod);
    gtk_container_add(GTK_CONTAINER(theButtonBox), mMRButtonSelectedMod);
    gtk_tooltips_set_tip(tooltips, mMRButtonSelectedMod,
			 _
			 ("Use this button to change the displayed Selected Fin\n"
			  "between original and user modified images."),
			 NULL);
	//***1.2 - end

	// create FRAME for UNKNOWN FIN image (upper-right of table)

    frameUnknown = gtk_frame_new(_("Unknown Fin"));
    gtk_widget_show(frameUnknown);
    gtk_table_attach(GTK_TABLE(table), frameUnknown, 1, 2, 0, 1,
		     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
    gtk_container_set_border_width(GTK_CONTAINER(frameUnknown), 5);

	//***1.2 - new vbox to hold image and modifying buttons inside frame
	theVBox = gtk_vbox_new(FALSE, 5);
    gtk_widget_show(theVBox);
    gtk_container_add(GTK_CONTAINER(frameUnknown), theVBox);

    eventBoxUnknown = gtk_event_box_new();
    gtk_widget_show(eventBoxUnknown);
	gtk_box_pack_start(GTK_BOX(theVBox), eventBoxUnknown, TRUE, TRUE, 2);
    gtk_tooltips_set_tip(tooltips, eventBoxUnknown,
			 _
			 ("Left click to see a larger view of this image."),
			 NULL);

    mDrawingAreaUnknown = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(mDrawingAreaUnknown),
					FIN_IMAGE_WIDTH, FIN_IMAGE_HEIGHT);
    gtk_widget_show(mDrawingAreaUnknown);
    gtk_container_add(GTK_CONTAINER(eventBoxUnknown), mDrawingAreaUnknown);

	//***1.2 - new buttons for morphing or modifying images
	
	theButtonBox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(theButtonBox), GTK_BUTTONBOX_SPREAD);
    gtk_widget_show(theButtonBox);
	gtk_box_pack_start(GTK_BOX(theVBox), theButtonBox, FALSE, FALSE, 5);

	mMRButtonUnknownMod = gtk_button_new_with_label(_("Show Original Image"));
    gtk_widget_show(mMRButtonUnknownMod);
    gtk_container_add(GTK_CONTAINER(theButtonBox), mMRButtonUnknownMod);
    gtk_tooltips_set_tip(tooltips, mMRButtonUnknownMod,
			 _
			 ("Use this button to change the displayed Unknown Fin\n"
			  "between original and user modified images."),
			 NULL);

	mMRButtonUnknownMorph = gtk_button_new_with_label(_("Match Selected Fin Orientation"));
    gtk_widget_show(mMRButtonUnknownMorph);
    gtk_container_add(GTK_CONTAINER(theButtonBox), mMRButtonUnknownMorph);
    gtk_tooltips_set_tip(tooltips, mMRButtonUnknownMorph,
			 _
			 ("Use this button to change the orientation of the Unknown Fin\n"
			  "between actual image orientation and that calculated to best\n"
			  "match the orientation of the Selected Fin."),
			 NULL);
	//***1.2 - end


	// connect SIGNALS to CALLBACKS

    gtk_signal_connect(GTK_OBJECT(matchResultsWindow), "delete_event",
		       GTK_SIGNAL_FUNC(on_matchResultsWindow_delete_event),
		       (void*)this);

	//***1.75 - buttons removed so signals NOT connected
    //gtk_signal_connect(GTK_OBJECT(mrRadioButtonIcons), "toggled",
	//	       GTK_SIGNAL_FUNC(on_mrRadioButtonIcons_toggled),
	//	       (void*)this);
    //gtk_signal_connect(GTK_OBJECT(mrRadioButtonList), "toggled",
	//	       GTK_SIGNAL_FUNC(on_mrRadioButtonList_toggled),
	//	       (void*)this);


	//***1.6 - new callback to hide / replace ID's
    gtk_signal_connect(GTK_OBJECT(mrButtonAltID), "toggled",
		       GTK_SIGNAL_FUNC(on_mrButtonAltID_toggled),
		       (void*)this);
	//***1.6 - new callback to hide / show various info fields in CList
    gtk_signal_connect(GTK_OBJECT(mrButtonShowInfo), "toggled",
		       GTK_SIGNAL_FUNC(on_mrButtonShowInfo_toggled),
		       (void*)this);
    gtk_signal_connect(GTK_OBJECT(mMRButtonPrev), "clicked",
		       GTK_SIGNAL_FUNC(on_mMRButtonPrev_clicked), (void*)this);
    gtk_signal_connect(GTK_OBJECT(mMRButtonNext), "clicked",
		       GTK_SIGNAL_FUNC(on_mMRButtonNext_clicked), (void*)this);

	//***1.85 - new callback for start/stop of scrolling slide show
    gtk_signal_connect(GTK_OBJECT(mMRButtonSlideShow), "clicked",
		       GTK_SIGNAL_FUNC(on_mMRButtonSlideShow_clicked), (void*)this);


    gtk_signal_connect(GTK_OBJECT(mrButtonFinsMatch), "clicked",
		       GTK_SIGNAL_FUNC(on_mrButtonFinsMatch_clicked),
		       (void*)this);
    gtk_signal_connect(GTK_OBJECT(mrButtonNoMatch), "clicked",
		       GTK_SIGNAL_FUNC(on_mrButtonNoMatch_clicked), (void*)this);

	// new callback o return us to Matching Dialog
    gtk_signal_connect(GTK_OBJECT(mrButtonReturnToMatchingDialog), "clicked",
		       GTK_SIGNAL_FUNC(on_mrButtonReturnToMatchingDialog_clicked), (void*)this);

	//***1.4 - new callback
	gtk_signal_connect(GTK_OBJECT(mrButtonSaveResults), "clicked",
		       GTK_SIGNAL_FUNC(on_mrButtonSaveResults_clicked), (void*)this);

    gtk_signal_connect(GTK_OBJECT(mrButtonCancel), "clicked",
		       GTK_SIGNAL_FUNC(on_mrButtonCancel_clicked), (void*)this);
    gtk_signal_connect(GTK_OBJECT(eventBoxSelected), "button_press_event",
		       GTK_SIGNAL_FUNC
		       (on_eventBoxSelected_button_press_event), (void*)this);
    gtk_signal_connect(GTK_OBJECT(mDrawingAreaSelected), "expose_event",
		       GTK_SIGNAL_FUNC
		       (on_mDrawingAreaSelected_expose_event), (void*)this);
    gtk_signal_connect(GTK_OBJECT(eventBoxUnknown), "button_press_event",
		       GTK_SIGNAL_FUNC
		       (on_eventBoxUnknown_button_press_event), (void*)this);
    gtk_signal_connect(GTK_OBJECT(mDrawingAreaUnknown), "expose_event",
		       GTK_SIGNAL_FUNC
		       (on_mDrawingAreaUnknown_expose_event), (void*)this);
    gtk_signal_connect(GTK_OBJECT(mDrawingAreaOutlines), "expose_event",
		       GTK_SIGNAL_FUNC
		       (on_mDrawingAreaOutlines_expose_event), (void*)this);

	//***1.2 - new event handler to pop-up outlines
    gtk_signal_connect(GTK_OBJECT(eventBoxOutlines), "button_press_event",
		       GTK_SIGNAL_FUNC
		       (on_eventBoxOutlines_button_press_event), (void*)this);

	//***1.2 - new event handlers for selected and unknonwn image manipulation buttons
    gtk_signal_connect(GTK_OBJECT(mMRButtonSelectedMod), "clicked",
		       GTK_SIGNAL_FUNC(on_mMRButtonSelectedMod_clicked), (void*)this);
    gtk_signal_connect(GTK_OBJECT(mMRButtonUnknownMod), "clicked",
		       GTK_SIGNAL_FUNC(on_mMRButtonUnknownMod_clicked), (void*)this);
    gtk_signal_connect(GTK_OBJECT(mMRButtonUnknownMorph), "clicked",
		       GTK_SIGNAL_FUNC(on_mMRButtonUnknownMorph_clicked), (void*)this);

    gtk_widget_grab_default(mMRButtonNext);
    gtk_object_set_data(GTK_OBJECT(matchResultsWindow), "tooltips",
			tooltips);

    gtk_window_add_accel_group(GTK_WINDOW(matchResultsWindow),
			       accel_group);

    return matchResultsWindow;
}

//*******************************************************************
//
//
//
void MatchResultsWindow::createMRCList()
{
    GtkWidget *labelPicture;
    GtkWidget *labelID;
    GtkWidget *labelName;
    GtkWidget *labelDate;
    GtkWidget *labelLocation;
    GtkWidget *labelDamage;
    GtkWidget *labelError;

    mMRCList = gtk_clist_new(7);
	gtk_clist_set_row_height(GTK_CLIST(mMRCList), MATCHRESULTS_THUMB_HEIGHT);
    gtk_widget_show(mMRCList);
    gtk_clist_column_titles_show(GTK_CLIST(mMRCList));

	for (int i = 0; i < 7; i++)
		gtk_clist_set_column_auto_resize(GTK_CLIST(mMRCList), i, TRUE);

    labelPicture = gtk_label_new(_("Picture"));
    gtk_widget_show(labelPicture);
    gtk_clist_set_column_widget(GTK_CLIST(mMRCList), 0, labelPicture);

    labelID = gtk_label_new(_("ID"));
	gtk_widget_show(labelID);
    gtk_clist_set_column_widget(GTK_CLIST(mMRCList), 1, labelID);

    //if (mOptions->mHideIDs) //***1.65 - changed way of doing this
	//	gtk_clist_set_column_visibility(GTK_CLIST(mMRCList), 1, FALSE);

    labelName = gtk_label_new(_("Name"));
    gtk_widget_show(labelName);
    gtk_clist_set_column_widget(GTK_CLIST(mMRCList), 2, labelName);

    labelDate = gtk_label_new(_("Date"));
    gtk_widget_show(labelDate);
    gtk_clist_set_column_widget(GTK_CLIST(mMRCList), 3, labelDate);

    labelLocation = gtk_label_new(_("Location"));
    gtk_widget_show(labelLocation);
    gtk_clist_set_column_widget(GTK_CLIST(mMRCList), 4, labelLocation);

    labelDamage = gtk_label_new(_("Damage"));
    gtk_widget_show(labelDamage);
    gtk_clist_set_column_widget(GTK_CLIST(mMRCList), 5, labelDamage);

	labelError = gtk_label_new(_("Rank: Error"));
    gtk_widget_show(labelError);
    gtk_clist_set_column_widget(GTK_CLIST(mMRCList), 6, labelError);


    gtk_signal_connect(GTK_OBJECT(mMRCList), "click_column",
	GTK_SIGNAL_FUNC(on_mMRCList_click_column), (void*)this);
	gtk_signal_connect(GTK_OBJECT(mMRCList), "select_row",
	GTK_SIGNAL_FUNC(on_mMRCList_select_row), (void*)this);
}

//*******************************************************************
//
//
//
void MatchResultsWindow::createMRIconTable()
{
	mRadioButtonVector.clear();
	mMRIconTableViewPort = gtk_viewport_new(NULL, NULL);
	int numEntries = mResults->size();

	int rows = (int)round((float)numEntries/ TABLE_COLS);
	GtkWidget *iconTable = gtk_table_new(rows, TABLE_COLS, TRUE);
	gtk_widget_show(iconTable);
	gtk_container_add(GTK_CONTAINER(mMRIconTableViewPort), iconTable);

	GSList *buttonGroup = NULL;

	for (unsigned i = 0, r = 0, c = 0; i < numEntries; i++, c++) {

		if (c >= TABLE_COLS) {
				c = 0;
				++r;
		}

		Result *res = mResults->getResultNum(i);
		GtkWidget *rb = createFinRadioButton(res->getIdCode(), res->mThumbnailPixmap, i, buttonGroup);
		buttonGroup = gtk_radio_button_group(GTK_RADIO_BUTTON(rb));
		gtk_widget_show(rb);
		mRadioButtonVector.push_back(rb);

		gtk_table_attach(
						GTK_TABLE(iconTable), rb,
						c, c + 1,
						r, r + 1,
						(GtkAttachOptions)GTK_FILL, (GtkAttachOptions)GTK_FILL,
						2, 2);

		//***1.0 - attempt to keep lists and icons consistent
		if (i == mCurEntry)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb),TRUE);
		else
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb),FALSE);
	}
}

//*******************************************************************
//
//
//
GtkWidget* MatchResultsWindow::createFinRadioButton(
		std::string id,
		char **pixMapString,
		int num,
		GSList *group)
{
		GtkWidget *finRadioButton = gtk_radio_button_new(group);
		gtk_container_set_border_width(GTK_CONTAINER(finRadioButton), 10);
		gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(finRadioButton), FALSE);

		GtkWidget *vBox = gtk_vbox_new(FALSE, 0);
		gtk_widget_show(vBox);

		GtkWidget *pixmap;
		if (NULL == pixMapString)
			pixmap = create_pixmap_from_data(vBox, fin_xpm);
		else
			pixmap = create_pixmap_from_data(vBox,pixMapString);

		gtk_widget_show(pixmap);
		gtk_box_pack_start(GTK_BOX(vBox), pixmap, TRUE, TRUE, 0);

		GtkWidget *labelID = gtk_label_new(id.c_str());
		gtk_misc_set_padding(GTK_MISC(labelID), 0, 2);
		gtk_widget_show(labelID);
		gtk_box_pack_start(GTK_BOX(vBox), labelID, FALSE, FALSE, 0);

		gtk_container_add(GTK_CONTAINER(finRadioButton), vBox);

		gtk_signal_connect(GTK_OBJECT(finRadioButton), "toggled",
						GTK_SIGNAL_FUNC(on_finRadioButton_toggled),
						(void *)this);

		return finRadioButton;
}

//*******************************************************************
//
//
//
gboolean on_matchResultsWindow_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData
	)
{
	delete (MatchResultsWindow*) userData;
	return TRUE;
}

//*******************************************************************
//
//
//
gboolean on_mDrawingAreaUnknown_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData
	)
{
	MatchResultsWindow *resultsWindow = (MatchResultsWindow*)userData;

	//***1.5 - all mUnknownImage occurences below changed to mUnknownImageShown

	// Check to make sure we've got all the stuff we need to redraw
	// the image.  If not, fail silently
	if (NULL == resultsWindow)
		return FALSE;
	if (NULL == resultsWindow->mUnknownImageShown)
		return FALSE;

	//***1.7 - new block to resize appropriate image
	int 
		newHeight = resultsWindow->mDrawingAreaUnknown->allocation.height,
		newWidth = resultsWindow->mDrawingAreaUnknown->allocation.width;

	// only resize the images if necessary
	if ((resultsWindow->mPrevUnkImgHeight != newHeight) || 
		(resultsWindow->mPrevUnkImgWidth != newWidth))
	{
		resultsWindow->mPrevUnkImgHeight = newHeight;
		resultsWindow->mPrevUnkImgWidth = newWidth;

		bool unmodified = resultsWindow->mUnknownImageShown == resultsWindow->mUnknownImage;

		// delete existing UNMODIFIED image
		if(resultsWindow->mUnknownImage)
			delete resultsWindow->mUnknownImage;

		// create newly resized UNMODIFIED image
		resultsWindow->mUnknownImage = resizeWithBorder(
				resultsWindow->mUnknownImageOriginal, 
				newHeight, 
				newWidth);	//***1.7
			
		// delete existing MODIFIED image
		if(resultsWindow->mUnknownImageMod)
			delete resultsWindow->mUnknownImageMod;

		// create newly resized MODIFIED image
		resultsWindow->mUnknownImageMod = resizeWithBorder(
				resultsWindow->mUnknownImageModOriginal, 
				newHeight, 
				newWidth);	//***1.7

		if (unmodified)
			resultsWindow->mUnknownImageShown = resultsWindow->mUnknownImage;
		else
			resultsWindow->mUnknownImageShown = resultsWindow->mUnknownImageMod;
	}
	
	if (NULL == resultsWindow->mUnknownImageShown)
		return FALSE;

	gdk_draw_rgb_image(
			widget->window,
			widget->style->fg_gc[GTK_STATE_NORMAL],
			0, 0,
			resultsWindow->mUnknownImageShown->getNumCols(),
			resultsWindow->mUnknownImageShown->getNumRows(),
			GDK_RGB_DITHER_NONE,
			(guchar*)resultsWindow->mUnknownImageShown->getData(),
			resultsWindow->mUnknownImageShown->getNumCols() * resultsWindow->mUnknownImageShown->bytesPerPixel());

	for (int i=0; i<3; i++)
		gdk_draw_rectangle( //***1.75 - draws border around image in contour color
			widget->window,
			resultsWindow->mGC1,
			FALSE,
			i,i,
			resultsWindow->mDrawingAreaUnknown->allocation.width-(1+2*i),
			resultsWindow->mDrawingAreaUnknown->allocation.height-(1+2*i));

	return TRUE;
}

//*******************************************************************
//
//
//
gboolean on_mDrawingAreaOutlines_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData
	)
{
	MatchResultsWindow *resWin = (MatchResultsWindow *) userData;

	if (NULL == resWin)
		return FALSE;

	// Redraw the background
	gdk_draw_rectangle(
		resWin->mDrawingAreaOutlines->window,
		resWin->mDrawingAreaOutlines->style->bg_gc[GTK_STATE_NORMAL],
		TRUE,
		0,
		0,
		FIN_IMAGE_WIDTH,
		FIN_IMAGE_HEIGHT);

	//***1.85 - if no selected fin, then all fins matched in DB are no longer
	// part of DB and there is no outline to draw ... return quitely
	if (NULL == resWin->mSelectedFin)
		return FALSE;

	FloatContour *c = resWin->mUnknownContour; //***005CM
	FloatContour *fc = resWin->mRegContour;

	float
		xMax = (fc->maxX() > (float)c->maxX()) ? fc->maxX() : (float)c->maxX(),
		yMax = (fc->maxY() > (float)c->maxY()) ? fc->maxY() : (float)c->maxY(),
		xMin = (fc->minX() < (float)c->minX()) ? fc->minX() : (float)c->minX(),
		yMin = (fc->minY() < (float)c->minY()) ? fc->minY() : (float)c->minY();

	float
		xRange = xMax - xMin,
		yRange = yMax - yMin;

	//float
	//	heightRatio = FIN_IMAGE_HEIGHT / yRange,
	//	widthRatio = FIN_IMAGE_WIDTH / xRange;

	//***1.7
	float
		heightRatio = resWin->mDrawingAreaOutlines->allocation.height / yRange,
		widthRatio = resWin->mDrawingAreaOutlines->allocation.width / xRange;


	float ratio;

	int xOffset = 0, yOffset = 0;

	if (heightRatio < widthRatio) {
		ratio = heightRatio;
		//xOffset = (int) round((FIN_IMAGE_WIDTH - ratio * xRange) / 2);
		xOffset = (int) round((resWin->mDrawingAreaOutlines->allocation.width - ratio * xRange) / 2); //***1.7
	} else {
		ratio = widthRatio;
		//yOffset = (int) round((FIN_IMAGE_HEIGHT - ratio * yRange) / 2);
		yOffset = (int) round((resWin->mDrawingAreaOutlines->allocation.height - ratio * yRange) / 2); //***1.7
	}

	unsigned numPoints = c->length();
	unsigned i;

	for (i = 0; i < numPoints; i++) {
		int xCoord = (int) round(((*c)[i].x - xMin) * ratio + xOffset);
		int yCoord = (int) round(((*c)[i].y - yMin) * ratio + yOffset);

		gdk_draw_rectangle(
			widget->window,
			resWin->mGC1,
			TRUE,
			xCoord - POINT_SIZE / 2,
			yCoord - POINT_SIZE / 2,
			POINT_SIZE,
			POINT_SIZE);
	}

	numPoints = fc->length();

	for (i = 0; i < numPoints; i++) {
		int xCoord = (int) round(((*fc)[i].x - xMin) * ratio + xOffset);
		int yCoord = (int) round(((*fc)[i].y - yMin) * ratio + yOffset);

		gdk_draw_rectangle(
			widget->window,
			resWin->mGC2,
			TRUE,
			xCoord - POINT_SIZE / 2,
			yCoord - POINT_SIZE / 2,
			POINT_SIZE,
			POINT_SIZE);
	}

	return TRUE;
}

//*******************************************************************
//
//
//
gboolean on_mDrawingAreaSelected_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData
	)
{
	MatchResultsWindow *resultsWindow = (MatchResultsWindow*)userData;

	// Check to make sure we've got all the stuff we need to redraw
	// the image.  If not, fail silently
	if (NULL == resultsWindow)
		return FALSE;

	//***1.85 - it is possible that results were from a different database
	// than the one that was currently loaded, or that ALL ins in the DB
	// at time of results creation have been deleted from DB now -- this means
	// there may NOT be any fins to select

	if(NULL == resultsWindow->mSelectedFin) //***1.85
	{
		// redraw a blank background
		gdk_draw_rectangle(
			resultsWindow->mDrawingAreaSelected->window,
			resultsWindow->mDrawingAreaSelected->style->bg_gc[GTK_STATE_NORMAL],
			TRUE,
			0,
			0,
			resultsWindow->mDrawingAreaSelected->allocation.width,
			resultsWindow->mDrawingAreaSelected->allocation.height);
		return TRUE;
	}

	//***1.7 - new block to resize appropriate image
	int 
		newHeight = resultsWindow->mDrawingAreaSelected->allocation.height,
		newWidth = resultsWindow->mDrawingAreaSelected->allocation.width;

	// only resize the images if necessary
	if ((resultsWindow->mPrevSelImgHeight != newHeight) || 
		(resultsWindow->mPrevSelImgWidth != newWidth))
	{
		resultsWindow->mPrevSelImgHeight = newHeight;
		resultsWindow->mPrevSelImgWidth = newWidth;

		bool unmodified = resultsWindow->mSelectedImageShown == resultsWindow->mSelectedImage;

		// delete existing UNMODIFIED image
		if(resultsWindow->mSelectedImage)
			delete resultsWindow->mSelectedImage;

		// create newly resized UNMODIFIED image
		resultsWindow->mSelectedImage = resizeWithBorder(
				resultsWindow->mSelectedImageOriginal, 
				newHeight, 
				newWidth);	//***1.7
			
		// delete existing MODIFIED image
		if(resultsWindow->mSelectedImageMod)
			delete resultsWindow->mSelectedImageMod;

		// create newly resized MODIFIED image
		resultsWindow->mSelectedImageMod = resizeWithBorder(
				resultsWindow->mSelectedImageModOriginal, 
				newHeight, 
				newWidth);	//***1.7

		if (unmodified)
			resultsWindow->mSelectedImageShown = resultsWindow->mSelectedImage;
		else
			resultsWindow->mSelectedImageShown = resultsWindow->mSelectedImageMod;
	}

	if (NULL == resultsWindow->mSelectedImageShown) //***1.8
		return FALSE;

	gdk_draw_rgb_image(
			widget->window,
			widget->style->fg_gc[GTK_STATE_NORMAL],
			0, 0,
			resultsWindow->mSelectedImageShown->getNumCols(),
			resultsWindow->mSelectedImageShown->getNumRows(),
			GDK_RGB_DITHER_NONE,
			(guchar*)resultsWindow->mSelectedImageShown->getData(),
			resultsWindow->mSelectedImageShown->getNumCols() * resultsWindow->mSelectedImageShown->bytesPerPixel());

	for (int i=0; i<3; i++)
		gdk_draw_rectangle( //***1.75 - draws border around image in contour color
			widget->window,
			resultsWindow->mGC2,
			FALSE,
			i,i,
			resultsWindow->mDrawingAreaSelected->allocation.width-(1+2*i),
			resultsWindow->mDrawingAreaSelected->allocation.height-(1+2*i));

	return TRUE;
}

//*******************************************************************
//
//
//
void on_mrButtonFinsMatch_clicked(
	GtkButton *button,
	gpointer userData
	)
{
	MatchResultsWindow *resWin = (MatchResultsWindow *) userData;

	if (NULL == resWin)
		return;

	//***1.6 - for now, just do the same thing as the NO MATCH callback -- allow adding image
	// to database using the same process.  Eventually, we should grab ID, NAME and
	// some other info fields from the selected fin before opening the NoMatchWindow

	DatabaseFin<ColorImage> *unknown = new DatabaseFin<ColorImage>(resWin->mUnknownFin);//**003MR >

	unknown->mIsAlternate = true; //***1.95 - add as alternate 

	/*
	NoMatchWindow *NoMatchWin= new NoMatchWindow(
			unknown,
			resWin->mDatabase,
			resWin->mMainWin,
			resWin->mOptions);

	NoMatchWin->show();
	*/

	string title = "Matches [";
	title += resWin->mSelectedFin->getID() + "] - Add to Database as Additional Fin Image";

	TraceWindow *win = new TraceWindow(
			resWin->mMainWin,
			title,
			unknown,
			resWin->mDatabase,
			resWin->mOptions);
	win->show();

	//***1.6 - do NOT delete here, allow return to dialog after save
	//delete resWin;
}

//*******************************************************************
//
//
//
void on_mrButtonNoMatch_clicked(
	GtkButton *button,
	gpointer userData
	)
{
	MatchResultsWindow *resWin = (MatchResultsWindow *) userData;

	if (NULL == resWin)
		return;

	if (NULL == resWin->mUnknownFin)
		return;

	DatabaseFin<ColorImage> *unknown = new DatabaseFin<ColorImage>(resWin->mUnknownFin);//**003MR >

	/*
	NoMatchWindow *NoMatchWin= new NoMatchWindow(
			unknown,
			resWin->mDatabase,
			resWin->mMainWin,			//***004CL
			resWin->mOptions);          //***054

	NoMatchWin->show();				//**003MR <
	*/

	TraceWindow *win = new TraceWindow(
			resWin->mMainWin,
			"No Match - Add to Database as NEW Fin/Image",
			unknown,
			resWin->mDatabase,
			resWin->mOptions);
	win->show();

	//***1.6 - do NOT delete here, allow return to dialog after save
	//delete resWin;
}

//*******************************************************************
//
//
//
void on_mrButtonReturnToMatchingDialog_clicked(
	GtkButton *button,
	gpointer userData
	)
{
	MatchResultsWindow *resWin = (MatchResultsWindow *) userData;

	if (NULL == resWin)
		return;

	// if MatchingDialog handle is NULL we got here from MathcingQueueDialog
	// so inactivate this return button (do nothing)
	if (NULL == resWin->mMatchingDialog)
		return;

	resWin->mReturningToMatchingDialog = true;

	resWin->mMatchingDialog->show(true); // returning

	delete resWin;

}


//*******************************************************************
//
//
//
void on_mrButtonCancel_clicked(
	GtkButton *button,
	gpointer userData
	)
{
	//***1-3 - this was tried but caused crashing problems if MatchingQueueDialog was
	// deleted before this class -- whole thig seems not to be needed.  It was an attempt
	// to keep the MatchingQueueWindow ABOVE the MainWindow on return.
	// DELETE if behavior contiunes OK.
	
	MatchResultsWindow *resWin = (MatchResultsWindow*) userData;

	if (resWin->mMatchingQueueDialog)
		resWin->mMatchingQueueDialog->show();
	
	delete resWin;
	
	//delete (MatchResultsWindow*) userData; //*** 2.2 - back to code above, for this version
}

//*******************************************************************
//
//
//
void on_mMRCList_select_row(
	GtkCList *clist,
	gint row,
	gint column,
	GdkEvent *event,
	gpointer userData
	)
{
	MatchResultsWindow *resWin = (MatchResultsWindow *) userData;

	if (NULL == resWin)
		return;

	if (row == 0)
		gtk_widget_set_sensitive(resWin->mMRButtonPrev, FALSE);
	else
		gtk_widget_set_sensitive(resWin->mMRButtonPrev, TRUE);

	if (row == resWin->mResults->size() - 1)
		gtk_widget_set_sensitive(resWin->mMRButtonNext, FALSE);
	else
		gtk_widget_set_sensitive(resWin->mMRButtonNext, TRUE);

	resWin->mCurEntry = row;
	Result* r = resWin->mResults->getResultNum(row);

	delete resWin->mSelectedFin;

	//***1.3 - decide whether results are from match queue results file or from
	// normal trace & match, and then decide how to load selected database fin
	//***1.6 - no decision now, since all matching is done same way as in match queue
	//if (NULL == resWin->mMatchingDialog)
		resWin->mSelectedFin = resWin->mDatabase->getItemAbsolute(r->getPosition());
	//else
	//	resWin->mSelectedFin = resWin->mDatabase->getItem(r->getPosition());

	if (NULL == resWin->mSelectedFin)
		return;

	/*
	//***1.8 - the old way
	delete resWin->mSelectedImage;

        ColorImage *temp = new ColorImage(resWin->mSelectedFin->mImageFilename); //***001DB
        resWin->mSelectedImage = resizeWithBorder(
                                        temp,			//***001DB
                                        resWin->mDrawingAreaSelected->allocation.height,//***1.7
                                        resWin->mDrawingAreaSelected->allocation.width);//***1.7

        delete temp;						//***001DB
	*/

	//***1.8 - the new way, supporting original and modified images

	if (NULL != resWin->mSelectedImageModOriginal)
		delete resWin->mSelectedImageModOriginal;

	resWin->mSelectedImageModOriginal = new ColorImage(resWin->mSelectedFin->mImageFilename);

	// set the selected fin attributes, from the attributes embedded in the image
	// if there are any
	resWin->mSelectedFin->mImageMods = resWin->mSelectedImageModOriginal->mImageMods;
	//string origName = (((resWin->mOptions->mDarwinHome + 
	//	PATH_SLASH) + "catalog") + PATH_SLASH) + 
	//	resWin->mSelectedImageModOriginal->mOriginalImageFilename;
	//***1.85 - now use path to CURRENT catalog
	string origName = resWin->mOptions->mDatabaseFileName;
	origName = origName.substr(0,origName.rfind(PATH_SLASH)+1);
	origName += resWin->mSelectedImageModOriginal->mOriginalImageFilename;
	resWin->mSelectedFin->mOriginalImageFilename = origName;

	//***1.982b - fix memory leak
	if (NULL != resWin->mSelectedImageMod)
		delete resWin->mSelectedImageMod;

	resWin->mSelectedImageMod = resizeWithBorder(
                                        resWin->mSelectedImageModOriginal,
                                        resWin->mDrawingAreaSelected->allocation.height,
                                        resWin->mDrawingAreaSelected->allocation.width);

	if (NULL != resWin->mSelectedImageOriginal)
		delete resWin->mSelectedImageOriginal;

	resWin->mSelectedImageOriginal = new ColorImage(resWin->mSelectedFin->mOriginalImageFilename);

	//***1.982b - fix memory leak
	if (NULL != resWin->mSelectedImage)
		delete resWin->mSelectedImage;

	resWin->mSelectedImage = resizeWithBorder(
                                        resWin->mSelectedImageOriginal,
                                        resWin->mDrawingAreaSelected->allocation.height,
                                        resWin->mDrawingAreaSelected->allocation.width);

	// set up new selected fin and show MODIFIED image and proper buttons
	resWin->mSelectedImageShown = resWin->mSelectedImageMod;
	resWin->mSelectedIsModified = true;
	gtk_button_set_label(GTK_BUTTON(resWin->mMRButtonSelectedMod),"Show Original Image");
	gtk_widget_show(resWin->mMRButtonSelectedMod);

	//***1.8 - end

	resWin->refreshSelectedFin();
	resWin->selectFromCList(resWin->mCurEntry);

  //***005CM - get the saved contours from Match results.
	resWin->mRegContour = r->dbContour;
	resWin->mUnknownContour = r->unknownContour;

	resWin->refreshOutlines();
}

//*******************************************************************
//
//
//
void on_mMRCList_click_column(
	GtkCList *clist,
	gint column,
	gpointer userData
	)
{
	MatchResultsWindow *resWin = (MatchResultsWindow *) userData;

	if (NULL == resWin)
		return;

	switch (column) {
		case 1:
			resWin->mResults->sort(MR_IDCODE,resWin->mCurEntry);
			break;
		case 2:
			resWin->mResults->sort(MR_NAME,resWin->mCurEntry);
			break;
		case 3:
			resWin->mResults->sort(MR_DATE,resWin->mCurEntry);
			break;
		case 4:
			resWin->mResults->sort(MR_LOCATION,resWin->mCurEntry);
			break;
		case 5:
			resWin->mResults->sort(MR_DAMAGE,resWin->mCurEntry);
			break;
		case 6:
			resWin->mResults->sort(MR_ERROR,resWin->mCurEntry);
			break;
		default:
			resWin->mResults->sort(MR_ERROR,resWin->mCurEntry);
			break;
	}

	resWin->updateList();

	if (resWin->mResults->size() > 0)
		gtk_clist_select_row(GTK_CLIST(resWin->mMRCList), resWin->mCurEntry, 0);
	else {
		gtk_widget_set_sensitive(resWin->mMRButtonNext, FALSE);
		gtk_widget_set_sensitive(resWin->mMRButtonPrev, FALSE);
	}
}

//*******************************************************************
//
//
//
gboolean on_eventBoxUnknown_button_press_event(
	GtkWidget *widget,
	GdkEventButton *event,
	gpointer userData)
{
	MatchResultsWindow *resWin = (MatchResultsWindow *) userData;

	if (NULL == resWin)
		return FALSE;

	//***1.5 - blow up either the original or the modified image as currently selected

	ImageViewDialog *dlg; //***1.8 - moved here

	if (! resWin->mUnknownIsModified)
	{
		if (NULL == resWin->mUnknownImageOriginal)
			return FALSE;

		//***2.22 - added resWin->mWindow below - so dialog is set transient for this window
		dlg = new ImageViewDialog(resWin->mWindow, _("Unknown Fin"), resWin->mUnknownImageOriginal);
	}
	else
	{
		if (NULL == resWin->mUnknownImageModOriginal)
			return FALSE;

		//***2.22 - added resWin->mWindow below - so dialog is set transient for this window
		dlg = new ImageViewDialog(resWin->mWindow, _("Unknown Fin"), resWin->mUnknownImageModOriginal);
	}

	dlg->show();

	return TRUE;

}

//*******************************************************************
//
//
//
gboolean on_eventBoxSelected_button_press_event(
	GtkWidget *widget,
	GdkEventButton *event,
	gpointer userData)
{
	MatchResultsWindow *resWin = (MatchResultsWindow *) userData;

	if (NULL == resWin)
		return FALSE;

	/* old code
	if (NULL == resWin->mSelectedFin)
		return FALSE;

        delete resWin->mSelectedFin->mFinImage; //***001DB
        resWin->mSelectedFin->mFinImage  = new ColorImage(resWin->mSelectedFin->mImageFilename); //***001DB
	ImageViewDialog *dlg = new ImageViewDialog(resWin->mSelectedFin->mIDCode, resWin->mSelectedFin->mFinImage);
	*/
	
	//***1.8 - blow up either the original or the modified image as currently selected

	ImageViewDialog *dlg; //***1.8 - moved here

	if (! resWin->mSelectedIsModified)
	{
		if (NULL == resWin->mSelectedImageOriginal)
			return FALSE;

		//***2.22 - added resWin->mWindow below - so dialog is set transient for this window
		dlg = new ImageViewDialog(resWin->mWindow, _("Selected Fin"), resWin->mSelectedImageOriginal);
	}
	else
	{
		if (NULL == resWin->mSelectedImageModOriginal)
			return FALSE;

		//***2.22 - added resWin->mWindow below - so dialog is set transient for this window
		dlg = new ImageViewDialog(resWin->mWindow, _("Selected Fin"), resWin->mSelectedImageModOriginal);
	}

	dlg->show();

	return TRUE;
}

//*******************************************************************
//
//
//
//***1.2 - new handler to allow magnification of mapped outlines
gboolean on_eventBoxOutlines_button_press_event(
	GtkWidget *widget,
	GdkEventButton *event,
	gpointer userData)
{
	MatchResultsWindow *resWin = (MatchResultsWindow *) userData;

	if (NULL == resWin)
		return FALSE;

	if (NULL == resWin->mSelectedFin)
		return FALSE;
	
	int uBegin,uTip,uEnd,dbBegin,dbTip,dbEnd;
	resWin->mResults->getResultNum(resWin->mCurEntry)->getMappingControlPoints(
				uBegin,uTip,uEnd,dbBegin,dbTip,dbEnd);

	MappedContoursDialog *dlg = new MappedContoursDialog(
				resWin->mWindow, //***2.22
				resWin->mUnknownFin->getID(),
				resWin->mUnknownContour,
				uBegin,uTip,uEnd,
				resWin->mSelectedFin->getID(),
				resWin->mRegContour,
				dbBegin,dbTip,dbEnd);
	dlg->show();

	return TRUE;
}

//*******************************************************************
//
//
//
void on_mMRButtonNext_clicked(
	GtkButton *button,
	gpointer userData)
{
	MatchResultsWindow *mrWin = (MatchResultsWindow *)userData;

	if (mrWin->mCurEntry < mrWin->mResults->size() - 1)
		gtk_clist_select_row(GTK_CLIST(mrWin->mMRCList), mrWin->mCurEntry + 1, 0);
}

//*******************************************************************
//
//
//
void on_mMRButtonPrev_clicked(
	GtkButton *button,
	gpointer userData)
{
	MatchResultsWindow *mrWin = (MatchResultsWindow *)userData;

	if (mrWin->mCurEntry > 0)
		gtk_clist_select_row(GTK_CLIST(mrWin->mMRCList), mrWin->mCurEntry - 1, 0);
}

//*******************************************************************
//***1.85
//
void on_mMRButtonSlideShow_clicked(
	GtkButton *button,
	gpointer userData)
{
	MatchResultsWindow *mrWin = (MatchResultsWindow *)userData;

	mrWin->mSlideShowOn = (! mrWin->mSlideShowOn); // flip the start / stop status

	// now depending on the NEW status ...

	if (mrWin->mSlideShowOn)
	{
		// redisplay button and START slide show at currently selected fin
		gtk_button_set_label(GTK_BUTTON(mrWin->mMRButtonSlideShow), _("_Scroll[on]"));
		mrWin->mTimerID = gtk_timeout_add(4000, matchResultsSlideShowTimer, userData);
	}
	else
	{
		// redisplay button ... slide show stops at currently selected fin
		gtk_button_set_label(GTK_BUTTON(mrWin->mMRButtonSlideShow), _("_Scroll[off]"));
	}
}

//*******************************************************************
//***1.85
//
//
gboolean matchResultsSlideShowTimer(
	gpointer userData)
{
	MatchResultsWindow *mrWin = (MatchResultsWindow *)userData;

	if (! mrWin->mSlideShowOn)
		return FALSE; // terminate me

	// we are at the end, so stop (and redisplay button)
	if (mrWin->mCurEntry >= mrWin->mResults->size() - 1)
	{
		on_mMRButtonSlideShow_clicked(GTK_BUTTON(mrWin->mMRButtonSlideShow), userData);
		return FALSE;
	}

	// advance to the next selected fin
	gtk_clist_select_row(GTK_CLIST(mrWin->mMRCList), mrWin->mCurEntry + 1, 0);

	return TRUE; // keep me going
}

//*******************************************************************
//***1.6 - new function hides or exposes fin ID's
//
//
void on_mrButtonAltID_toggled(
			GtkToggleButton *togglebutton,
			gpointer userData)
{
	MatchResultsWindow *resWin = (MatchResultsWindow *)userData;

	if (NULL == resWin)
		return;

	if (togglebutton->active)
	{
		gtk_button_set_label(GTK_BUTTON(togglebutton),"Show ID's");
		//gtk_clist_set_column_visibility(GTK_CLIST(resWin->mMRCList), 1, FALSE);
		resWin->mLocalHideIDs = true; //***1.65
	}
	else
	{
		gtk_button_set_label(GTK_BUTTON(togglebutton),"Hide ID's");
		//gtk_clist_set_column_visibility(GTK_CLIST(resWin->mMRCList), 1, TRUE);
		resWin->mLocalHideIDs = false; //***1.65
	}
	resWin->updateList(); //***1.65 - force change to affect list
}

//*******************************************************************
//***1.6 - new function hides or exposes fin information fields
//
//
void on_mrButtonShowInfo_toggled(
			GtkToggleButton *togglebutton,
			gpointer userData)
{
	MatchResultsWindow *resWin = (MatchResultsWindow *)userData;

	if (NULL == resWin)
		return;

	//resWin->mShowAltID = ! resWin->mShowAltID;

	if (togglebutton->active)
	{
		gtk_clist_set_column_visibility(GTK_CLIST(resWin->mMRCList), 2, FALSE); // name
		gtk_clist_set_column_visibility(GTK_CLIST(resWin->mMRCList), 3, FALSE); // date
		gtk_clist_set_column_visibility(GTK_CLIST(resWin->mMRCList), 4, FALSE); // location
		gtk_clist_set_column_visibility(GTK_CLIST(resWin->mMRCList), 5, FALSE); // damage
		gtk_clist_set_column_visibility(GTK_CLIST(resWin->mMRCList), 6, FALSE); // rank:error
	}
	else
	{
		gtk_clist_set_column_visibility(GTK_CLIST(resWin->mMRCList), 2, TRUE); // name
		gtk_clist_set_column_visibility(GTK_CLIST(resWin->mMRCList), 3, TRUE); // date
		gtk_clist_set_column_visibility(GTK_CLIST(resWin->mMRCList), 4, TRUE); // location
		gtk_clist_set_column_visibility(GTK_CLIST(resWin->mMRCList), 5, TRUE); // damage
		gtk_clist_set_column_visibility(GTK_CLIST(resWin->mMRCList), 6, TRUE); // rank:error
	}
}


//*******************************************************************
//
//
//
void on_mrRadioButtonIcons_toggled(
			GtkToggleButton *togglebutton,
			gpointer userData)
{
	if (togglebutton->active) {
		MatchResultsWindow *resWin = (MatchResultsWindow *)userData;

		if (NULL == resWin)
				return;

		resWin->mResults->sort(MR_ERROR,resWin->mCurEntry);

		// I like increasing the reference count and then destroying the
		// widget rather than letting the reference count decrease
		// destroy it.
		gtk_widget_ref(resWin->mMRCList);

		gtk_container_remove(
						GTK_CONTAINER(resWin->mScrolledWindow),
						resWin->mMRCList);

		gtk_widget_destroy(resWin->mMRCList);
		resWin->mMRCList = NULL;

		resWin->createMRIconTable();

		gtk_widget_set_sensitive(resWin->mMRButtonNext, FALSE);
		gtk_widget_set_sensitive(resWin->mMRButtonPrev, FALSE);

		gtk_widget_show(resWin->mMRIconTableViewPort);
		gtk_container_add(
						GTK_CONTAINER(resWin->mScrolledWindow),
						resWin->mMRIconTableViewPort);

		resWin->mMRView = MR_VIEW_ICONS;
	}
}

//*******************************************************************
//
//
//
void on_mrRadioButtonList_toggled(
			GtkToggleButton *togglebutton,
			gpointer userData)
{
	if (togglebutton->active) {
		MatchResultsWindow *resWin = (MatchResultsWindow *)userData;

		if (NULL == resWin)
				return;

		gtk_widget_ref(resWin->mMRIconTableViewPort);

		gtk_container_remove(
						GTK_CONTAINER(resWin->mScrolledWindow),
						resWin->mMRIconTableViewPort);

		gtk_widget_destroy(resWin->mMRIconTableViewPort);
		resWin->mMRIconTableViewPort = NULL;

		resWin->createMRCList();
		gtk_widget_show(resWin->mMRCList);
		gtk_container_add(
						GTK_CONTAINER(resWin->mScrolledWindow),
						resWin->mMRCList);
		resWin->updateList();

		if (resWin->mResults->size() > 0)
			//gtk_clist_select_row(GTK_CLIST(resWin->mMRCList), 0, 0);
			gtk_clist_select_row(GTK_CLIST(resWin->mMRCList), resWin->mCurEntry, 0);
		else {
			gtk_widget_set_sensitive(resWin->mMRButtonNext, FALSE);
			gtk_widget_set_sensitive(resWin->mMRButtonPrev, FALSE);
		}

		resWin->mMRView = MR_VIEW_LIST;
	}
}

//*******************************************************************
//
//
//
void on_finRadioButton_toggled(
			GtkToggleButton *togglebutton,
			gpointer userData)
{
	MatchResultsWindow *resWin = (MatchResultsWindow *)userData;

	if (NULL == resWin)
			return;

	int i;

	// A little dangerous, maybe... but exciting
	for (i = 0; !GTK_TOGGLE_BUTTON(resWin->mRadioButtonVector[i])->active; i++);

	resWin->mCurEntry = i;
	Result *r = resWin->mResults->getResultNum(i);

	delete resWin->mSelectedFin;

	resWin->mSelectedFin = resWin->mDatabase->getItem(r->getPosition());

	if (NULL == resWin->mSelectedFin)
			return;

	delete resWin->mSelectedImage;

	/*
	//*** this is commented out in version 1.8 - will need rewriting
	//    if ICON view is going to be used again

	ColorImage *temp = new ColorImage(resWin->mSelectedFin->mImageFilename); //***001DB
	resWin->mSelectedImage = resizeWithBorder(
					temp,			//***001DB
					//FIN_IMAGE_HEIGHT,
					//FIN_IMAGE_WIDTH);
					resWin->mDrawingAreaSelected->allocation.height,//***1.7
					resWin->mDrawingAreaSelected->allocation.width);//***1.7

        delete temp;			//***001DB
	resWin->refreshSelectedFin();
	*/
	printf("IMAGES not reloaded in ICON view - code needs changes\n");

  //***005CM replacing the following
  /*
	delete resWin->mRegContour;

	resWin->mRegContour = autoMapContour(
					resWin->mSelectedFin->mFinContour,
					resWin->mUnknownContour);
	*/

  //***005CM with the next 3 lines
  // Now, get the saved contours from Match results.
	resWin->mRegContour = r->dbContour;
	resWin->mUnknownContour = r->unknownContour;

	resWin->refreshOutlines();
}

//*******************************************************************
//
//
//
//***1.2 - new callback
void on_mMRButtonSelectedMod_clicked(
	GtkButton *button,
	gpointer userData)
{
	MatchResultsWindow *mrWin = (MatchResultsWindow *)userData;

	mrWin->mSelectedIsModified = ! mrWin->mSelectedIsModified; // flip value of flag

	//***1.8 - new selection to change image shown
	// IS implemented now - will do so when database version updated
	if (mrWin->mSelectedIsModified)
		mrWin->mSelectedImageShown = mrWin->mSelectedImageMod;
	else
		mrWin->mSelectedImageShown = mrWin->mSelectedImage;
	mrWin->refreshSelectedFin();

	// TOGGLE the button label (Original / Modified) to match new function
	if (mrWin->mSelectedIsModified)
		gtk_button_set_label(GTK_BUTTON(mrWin->mMRButtonSelectedMod),"Show Original Image");
	else
		gtk_button_set_label(GTK_BUTTON(mrWin->mMRButtonSelectedMod),"Show Modified Image");
	gtk_widget_show(mrWin->mMRButtonSelectedMod);

	//showError("The Image Original/Modified view option\nis not available in this\nsoftware version!");
}

//*******************************************************************
//
//
//
//***1.2 - new callback
void on_mMRButtonUnknownMod_clicked(
	GtkButton *button,
	gpointer userData)
{
	MatchResultsWindow *mrWin = (MatchResultsWindow *)userData;

	mrWin->mUnknownIsModified = ! mrWin->mUnknownIsModified; // flip value of flag

	//***1.5 - new selection to change image shown
	if (mrWin->mUnknownIsModified)
		mrWin->mUnknownImageShown = mrWin->mUnknownImageMod;
	else
		mrWin->mUnknownImageShown = mrWin->mUnknownImage;

	mrWin->refreshUnknownFin();

	// TOGGLE the button label (Origainl / Modified) to match new function
	if (mrWin->mUnknownIsModified)
		gtk_button_set_label(GTK_BUTTON(mrWin->mMRButtonUnknownMod),"Show Original Image");
	else
		gtk_button_set_label(GTK_BUTTON(mrWin->mMRButtonUnknownMod),"Show Modified Image");
	gtk_widget_show(mrWin->mMRButtonUnknownMod);

	//showError("The Image Original/Modified view option\nis not available in this\nsoftware version!");
}

//*******************************************************************
//
//
//
//***1.2 - new callback
void on_mMRButtonUnknownMorph_clicked(
	GtkButton *button,
	gpointer userData)
{
	MatchResultsWindow *mrWin = (MatchResultsWindow *)userData;

	mrWin->mUnknownIsMorphed = ! mrWin->mUnknownIsMorphed; // flip value of flag

	// TOGGLE the button label (Origainl / Modified) to match new function
	if (mrWin->mUnknownIsMorphed)
		gtk_button_set_label(GTK_BUTTON(mrWin->mMRButtonUnknownMorph),"Show Original Orientation");
	else
		gtk_button_set_label(GTK_BUTTON(mrWin->mMRButtonUnknownMorph),"Match Selected Fin Orientation");
	gtk_widget_show(mrWin->mMRButtonUnknownMorph);

	//showError("The Image Morphing option\nis not available in this\nsoftware version!");

}
//*******************************************************************
//
//
//
//***1.4 - new callback
void on_mrButtonSaveResults_clicked(
	GtkButton *button,
	gpointer userData)
{
	MatchResultsWindow *mrWin = (MatchResultsWindow *)userData;

	// make sure results did not come from a file, already
	if (mrWin->mSaveMessage != "")
		return;

	mrWin->mSaveMessage = "Saved Match Results as follows ...\n\n";

	// save fin and images, if needed
	string finFileRoot = mrWin->saveFinIfNeeded(); 

	// make sure there was an Outline to save, if save attempted
	if (finFileRoot == "NoOutline")
		return;

	if (finFileRoot == "AlreadySaved")
	{
		finFileRoot = mrWin->mUnknownFin->mFinFilename;
		finFileRoot = finFileRoot.substr(finFileRoot.find_last_of(PATH_SLASH)+1);
		finFileRoot = finFileRoot.substr(0,finFileRoot.rfind('.'));
	}
	
	// added by RJ -- needs to be tested
	finFileRoot = extractBasename(finFileRoot);
	int pos = finFileRoot.rfind('.');
	if(pos != string::npos)
		finFileRoot = finFileRoot.substr(0, pos);

	//***1.9 - break out database name to make part of results filename
	string dbName = mrWin->mOptions->mDatabaseFileName;
	dbName = dbName.substr(dbName.rfind(PATH_SLASH)+1);
	dbName = dbName.substr(0,dbName.rfind(".db"));

	char fName[500];
	//sprintf(fName, "%s%smatchQResults%smatch-for-%s.res", 
			//gOptions->mDarwinHome.c_str(), PATH_SLASH, PATH_SLASH, finFileRoot.c_str());
	//***1.85 - match results folder is now inside current survey area
	//sprintf(fName, "%s%smatchQResults%smatch-for-%s.res", 
			//gOptions->mCurrentSurveyArea.c_str(), PATH_SLASH, PATH_SLASH, finFileRoot.c_str());
	sprintf(fName, "%s%smatchQResults%s%s-DB-match-for-%s.res", 
		mrWin->mOptions->mCurrentSurveyArea.c_str(), PATH_SLASH, PATH_SLASH, 
			dbName.c_str(), finFileRoot.c_str());
	

	mrWin->mResults->save(fName);

	//***1.6 - more message
	
	mrWin->mSaveMessage += "Results File: " + dbName + "-DB-match-for-";
	mrWin->mSaveMessage += (finFileRoot + ".res\n");

	//***1.6 - now display the message
	GtkWidget *dlg = gtk_message_dialog_new(
		GTK_WINDOW(mrWin->mWindow), 
		GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
		"%s", mrWin->mSaveMessage.c_str());
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);

	gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE); //***1.6 - prevent multiple saves
}

//*******************************************************************
//***1.4 - force saving fin file as newly named file
//
// function returns root name of fin and image files saved
//
string MatchResultsWindow::saveFinIfNeeded()
{
	if (NULL == mUnknownFin->mFinOutline)
		return "NoOutline";

	if (mUnknownFin->mFinFilename != "")
		return "AlreadySaved";

	ifstream infile;

	string shortFilename = mUnknownFin->mImageFilename;
	int pos = shortFilename.find_last_of(PATH_SLASH);
	if (pos >= 0)
	{
		shortFilename = shortFilename.substr(pos+1);
	}

	pos = shortFilename.rfind('.');
	string rootName = shortFilename.substr(0,pos);
	string ext = shortFilename.substr(pos);

	//***1.85 - traced fins now go inside current survey area folder
	string path = gOptions->mCurrentSurveyArea;
	path += PATH_SLASH;
	path += "tracedFins";
	path += PATH_SLASH;

	string finFileName = path + rootName + ".finz";

	finFileName = generateUniqueName(finFileName); // prevents overwriting an existing finz

	saveFinz(mUnknownFin, finFileName);

	// this loop prevents overwriting previous fin and associated images
/*
	int i = 1;
	char num[8];
	//printf("Checking: %s ", finFileName.c_str());
	infile.open(finFileName.c_str());
	while (! infile.fail())
	{
		infile.close();
		//printf(" - file exists.\n");
		i++;
		sprintf(num,"[%d]",i);
		finFileName = (path + rootName + num) + ".fin";
		//printf("Checking: %s ", finFileName.c_str());
		infile.open(finFileName.c_str());
	}

	infile.clear();

	// at this point we have a root file name that is NOT in use, so we
	// can name the fin file and the associated image using this root

	if (i > 1)
		rootName = rootName + num;

	int slashPos = mUnknownFin->mImageFilename.find_last_of(PATH_SLASH);
	string destImgShortName = mUnknownFin->mImageFilename.substr(slashPos+1);

	//printf("copying \"%s\" to tracedFins\n",destImgShortName.c_str());

	string copyFilename = path + destImgShortName;

	// copy image over into tracedFins folder

#ifdef WIN32
	string command = "copy \"";
#else
	string command = "cp \"";
#endif
	command += mUnknownFin->mImageFilename;
	command += "\" \"";
	command += copyFilename;
	command += "\"";

#ifdef DEBUG
	printf("copy command: \"%s\"",command.c_str());
#endif

	if (copyFilename != mUnknownFin->mImageFilename) //***1.8 - prevent copy onto self
	{
		printf("copying \"%s\" to tracedFins\n",destImgShortName.c_str()); //***1.8 - moved here
		system(command.c_str());
	} */

	//***1.6 - build message to show user about saving of images, fin, and results
	//mSaveMessage += "Unknown Image: ";
	//mSaveMessage += extractBasename(mUnknownFin->mImageFilename);

	//string copyModFilename = "";

	/*
	//***1.5 - save modified image alongside original
	if (NULL != mUnknownFin->mModifiedFinImage)
	{
		// create filename
		copyModFilename = path + rootName + "_wDarwinMods.png"; //***1.9

		//***1.9 - need modified image filename, original filename & image mods
		//         passed to save_wMods(), AND scale must be set before saving

		// NOTE: At this point the mUnknownFin->mImageMods are valid and set
		// but the mUnknownFin->mModifiedFinImage->mImageMods are NOT ...
		// I am not sure why yet - JHS

		// save image
		mUnknownFin->mModifiedFinImage->save_wMods(
				copyModFilename,
				shortFilename,
				mUnknownFin->mImageMods);
				*/
		
		//***1.6 - more of message
		//mSaveMessage += " and ";
		//mSaveMessage += finFileName + "_wDarwinMods.png"; //***1.9
	//}

	mSaveMessage += "\n"; //***1.6

	string temp = mUnknownFin->mImageFilename; // save original name temporarily

	/* //***1.9 - save the name of the modified file if it exists
	if ("" == copyModFilename)
		mUnknownFin->mImageFilename = copyFilename; // change filename to reflect copy name
	else
		mUnknownFin->mImageFilename = copyModFilename; //***1.9

	mUnknownFin->save(finFileName);
	*/
	
	mResults->setFinFilename(finFileName);

	mUnknownFin->mImageFilename = temp; // restore original image name ?????

	//***1.6 - more message
	mSaveMessage += "Fin Trace: ";
	mSaveMessage += finFileName + "\n";

	// return rootName;
	return finFileName;
}
