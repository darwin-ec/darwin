//*******************************************************************
//   file: MatchingDialog.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (8/2/2005)
//         -- reformatting of code and addition of comment blocks
//         J H Stewman (8/30/2005)
//         -- additional controls (matching method, catalog category,
//         -- display of matching process)
//
//*******************************************************************

#include <gdk/gdkkeysyms.h>
#include "../support.h"
#include "MatchingDialog.h"
#include "MatchResultsWindow.h"
//#include "ErrorDialog.h"
//#include "../CatalogCategories.h"
#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#include "../../pixmaps/cancel.xpm"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef DEBUG
#include <iostream>
#endif

gboolean on_mMatchDialogDrawingAreaOutlines_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

gboolean on_matchingDialog_delete_event(
		GtkWidget *widget,
		GdkEvent *event,
		gpointer userData);

void on_matchingButtonStartStop_clicked(
		GtkButton *button,
		gpointer userData);

void on_matchingButtonPauseContinue_clicked(
		GtkButton *button,
		gpointer userData);

void on_mCategoryButton_toggled(
		GtkButton *button,
		gpointer userData);
void on_categoryCheckButtonAll_clicked(
		GtkButton *button,
		gpointer userData);
void on_categoryCheckButtonClear_clicked(
		GtkButton *button,
		gpointer userData);

void on_matchingButtonCancel_clicked(
		GtkButton *button,
		gpointer userData);

void on_mButtonShowHide_toggled(
		GtkButton *button,
		gpointer userData);

gboolean matchingIdleFunction(
		gpointer userData);

void on_radioOriginal_clicked(
		GtkObject *object,
		gpointer userData);

void on_radioTrimFixed_clicked(
		GtkObject *object,
		gpointer userData);

void on_radioTrimOptimalTotal_clicked(
		GtkObject *object,
		gpointer userData);

void on_radioTrimOptimalTip_clicked(
		GtkObject *object,
		gpointer userData);

void on_radioTrimOptimalArea_clicked(
		GtkObject *object,
		gpointer userData);

void on_radioTrimOptimalInOut_clicked(
		GtkObject *object,
		gpointer userData);

void on_radioTrimOptimalInOutTip_clicked(
		GtkObject *object,
		gpointer userData);

void on_radioAllPoints_clicked(
		GtkObject *object,
		gpointer userData);

void on_radioTrailingOnly_clicked( //***1.5 - new callback
		GtkObject *object,
		gpointer userData);

void on_radioLeadToTipOnly_clicked(
		GtkObject *object,
		gpointer userData);

void on_radioLeadToNotchOnly_clicked(
		GtkObject *object,
		gpointer userData);

void on_radioLeadThenTrail_clicked(
		GtkObject *object,
		gpointer userData);

using namespace std;

static int gNumReferences = 0;

// new constants for drawing outlines during matching

static const int FIN_IMAGE_WIDTH = 200;
static const int FIN_IMAGE_HEIGHT = 200;
static const int POINT_SIZE = 1;

//***055MD -- There are now two versions of the Matching Dialog
// one used for testing and evaluation of various matching methods
// and one for the end user -- use the define below to select which
// version to use -- if CREATE_USER_MATCHING_DIALOG is NOT defined
// then a more complete TESTING version of the dialog is created
//
#define CREATE_USER_MATCHING_DIALOG

//***055MD -- The ability to display lines between point pairs while
// computing the meanSquaredError can be disabled or enabled by
// defining CREATE_POINT_2_POINT_LINES below
//
//#define CREATE_POINT_2_POINT_LINES

//*******************************************************************
//
//
//
int getNumMatchingDialogReferences()
{
	return gNumReferences;
}


//*******************************************************************
//
//
//
MatchingDialog::MatchingDialog(
		DatabaseFin<ColorImage> *dbFin,
		Database *db,
		MainWindow *m,				//***004CL
		Options *o
)
	: //mDialog(createMatchingDialog()),
	  mMatch(new Match(dbFin, db, o)),
	  mMainWin(m),					//***004CL
	  mOptions(o),
	  mFin(dbFin),
	  mDatabase(db),
	  mMatchCancelled(false),
	  mMatchRunning(false),
	  mMatchPaused(false),
#ifndef CREATE_USER_MATCHING_DIALOG
	  //mRegistrationMethod(ORIGINAL_3_POINT),
	  mRegistrationMethod(TRIM_OPTIMAL_TIP), //***1.5 - set same for both now
#else
	  //mRegistrationMethod(TRIM_FIXED_PERCENT),
	  mRegistrationMethod(TRIM_OPTIMAL_TIP), //***1.5 - set same for both now
#endif
	  mRegSegmentsUsed(ALL_POINTS),
	  mGC1(NULL),
	  mGC2(NULL),
	  mShowingOutlines(false),
	  mUseFullFinError(true), //***055ER, ***1.5 - new default value
	  mCategoriesSelected(0) //***051
{
	if (NULL == dbFin || NULL == db)
		throw EmptyArgumentError("MatchingDialog ctor.");

	mMatch->getMatchResults()->setFinFilename(mFin->mFinFilename); //***1.4
	mMatch->getMatchResults()->setDatabaseFilename(mOptions->mDatabaseFileName); //***1.4


	// NOTE: mDrawingAreaOutlines is set by createMatchingDialog() -- DO NOT
	// set it explicitly in the constructor
	// ALSO: createMatchingDialog() uses mFin and so must be called here.
	// It CANNOT be called in the initialization list above.

	mDialog = createMatchingDialog();

	gNumReferences++;
}


//*******************************************************************
//
//
//
MatchingDialog::~MatchingDialog()
{
	gNumReferences--;
	gtk_widget_destroy(mDialog);
	delete mFin;
	delete mMatch;

	if (NULL != mGC1)
		gdk_gc_unref(mGC1);
}


//*******************************************************************
//
void MatchingDialog::updateGC()
{
	if (NULL == mGC1)
		mGC1 = gdk_gc_new(mDrawingAreaOutlines->window);

	if (NULL == mGC2)
		mGC2 = gdk_gc_new(mDrawingAreaOutlines->window);

	//***006CM - separate display colors added
	double color1[] = {1.0, 0.0, 0.0, 0.0};
	updateGCColor(mGC1, color1); // graphics context for unknown fin (RED)
	double color2[] = {0.0, 0.0, 1.0, 0.0};
	updateGCColor(mGC2, color2); // graphics context for database fin (BLUE)
}


//*******************************************************************
//
void MatchingDialog::updateGCColor(GdkGC *gc, double color[4])
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
void MatchingDialog::updateGCColor(GdkGC *gc)
{
		updateGCColor(gc, mOptions->mCurrentColor);
}


//*******************************************************************
//
// void MatchingDialog::show(bool returning)
//
//    Shows the MatchingDialog.
//
//    PARAMETER returning :
//         false - initial display
//         true - subsequent display (returning from MatchResultsWindow)
//
void MatchingDialog::show(bool returning)
{
	// on all EXCEPT FIRST call - initialize match (done first time by 
	// CONSTRUCTOR), set display option, zero progress bar, and
	// set buttons for RESTART

	if (returning)
	{
		delete mMatch;
		mMatch = new Match(mFin,mDatabase, mOptions);
		mMatch->getMatchResults()->setFinFilename(mFin->mFinFilename); //***2.01
		mMatch->getMatchResults()->setDatabaseFilename(mOptions->mDatabaseFileName); //***2.01
		if (mShowingOutlines)
			mMatch->setDisplay(this);
		else
			mMatch->setDisplay(NULL);
		gtk_progress_set_value(
				GTK_PROGRESS(mProgressBar),
				0.0);
		gtk_button_set_label(GTK_BUTTON(mButtonStartStop),"Start");
		gtk_widget_show(mButtonStartStop);
		gtk_button_set_label(GTK_BUTTON(mButtonPauseContinue),"Pause");
		gtk_widget_show(mButtonPauseContinue);
	}

	// on all calls - show the dialog, initialize state & register idle function

	gtk_widget_show(mDialog);

	updateGC();

	mMatchCancelled = false;
	mMatchRunning = false;
	mMatchPaused = false;

	// NOTE: idle function is added here, each time the window is shown, so that
	// it is started on first showing AND after each return from a MatchResultsWindow.
	// The idle function is stopped (removed) each time a MatchResultsWindow is
	// created.
	gtk_idle_add(matchingIdleFunction, (void *) this);
}


//*******************************************************************
//
// void MatchingDialog::showOutlines(FloatContour *unk, FloatContour *db)
//
//    Display the two contours (outlines) during registratin process
//
void MatchingDialog::showOutlines(FloatContour *unk, FloatContour *db)
{
	if ((NULL == unk) || (NULL == db))
		return;

	// Redraw the background
	gdk_draw_rectangle(
		mDrawingAreaOutlines->window,
		mDrawingAreaOutlines->style->bg_gc[GTK_STATE_NORMAL],
		TRUE,
		0,
		0,
		FIN_IMAGE_WIDTH + 1,
		FIN_IMAGE_HEIGHT + 1);

	float
		xMax = (db->maxX() > (float)unk->maxX()) ? db->maxX() : (float)unk->maxX(),
		yMax = (db->maxY() > (float)unk->maxY()) ? db->maxY() : (float)unk->maxY(),
		xMin = (db->minX() < (float)unk->minX()) ? db->minX() : (float)unk->minX(),
		yMin = (db->minY() < (float)unk->minY()) ? db->minY() : (float)unk->minY();

	float
		xRange = xMax - xMin + 2*POINT_SIZE, //***1.5 - added POINT_SIZE
		yRange = yMax - yMin + 2*POINT_SIZE; //***1.5 - added POINT_SIZE

	float
		heightRatio = FIN_IMAGE_HEIGHT / yRange,
		widthRatio = FIN_IMAGE_WIDTH / xRange;

	float ratio;

	int xOffset = 0, yOffset = 0;

	//xOffset = POINT_SIZE / 2; //***1.5 - keep it on the screen
	//yOffset = POINT_SIZE / 2; //***1.5 - keep it on the screen

	if (heightRatio < widthRatio) {
		ratio = heightRatio;
		xOffset = (int) round((FIN_IMAGE_WIDTH - ratio * xRange) / 2) - 1;
	} else {
		ratio = widthRatio;
		yOffset = (int) round((FIN_IMAGE_HEIGHT - ratio * yRange) / 2) - 1;
	}

	unsigned i; // declared here for MSVC++ compatibility

	// draw unknown fin outline in RED

	unsigned numPoints = unk->length();
	for (i = 0; i < numPoints; i++) {
		int xCoord = (int) round(((*unk)[i].x - xMin) * ratio + xOffset);
		int yCoord = (int) round(((*unk)[i].y - yMin) * ratio + yOffset);

		gdk_draw_rectangle(
			mDrawingAreaOutlines->window,
			mGC1,
			TRUE,
			xCoord - POINT_SIZE / 2,
			yCoord - POINT_SIZE / 2,
			POINT_SIZE,
			POINT_SIZE);
	}

	// draw database fin outline in BLUE

	numPoints = db->length();
	for (i = 0; i < numPoints; i++) {
		int xCoord = (int) round(((*db)[i].x - xMin) * ratio + xOffset);
		int yCoord = (int) round(((*db)[i].y - yMin) * ratio + yOffset);

		gdk_draw_rectangle(
			mDrawingAreaOutlines->window,
			mGC2,
			TRUE,
			xCoord - POINT_SIZE / 2,
			yCoord - POINT_SIZE / 2,
			POINT_SIZE,
			POINT_SIZE);
	}

#ifdef CREATE_POINT_2_POINT_LINES
	g_usleep(100000); // 0.1 second delay
#endif
/*
	// Redraw the background
	gdk_draw_rectangle(
		mDrawingAreaOutlines->window,
		mDrawingAreaOutlines->style->bg_gc[GTK_STATE_NORMAL],
		TRUE,
		0,
		0,
		FIN_IMAGE_WIDTH,
		FIN_IMAGE_HEIGHT);
*/
#ifndef CREATE_POINT_2_POINT_LINES
	g_usleep(100000); // 0.1 second delay
#endif

}

//*******************************************************************
//
// void MatchingDialog::showErrorPt2Pt(...)
//
//    Display line segment between corresponding points on two contours 
//    during error calculation
//
void MatchingDialog::showErrorPt2Pt(FloatContour *unk, FloatContour *db,
									float x1, float y1, float x2, float y2)
{

#ifdef CREATE_POINT_2_POINT_LINES

	// otherwise this function does NOTHING

	if ((NULL == unk) || (NULL == db))
		return;

	// Redraw the background
/*	gdk_draw_rectangle(
		mDrawingAreaOutlines->window,
		mDrawingAreaOutlines->style->bg_gc[GTK_STATE_NORMAL],
		TRUE,
		0,
		0,
		FIN_IMAGE_WIDTH,
		FIN_IMAGE_HEIGHT);
*/
	float
		xMax = (db->maxX() > (float)unk->maxX()) ? db->maxX() : (float)unk->maxX(),
		yMax = (db->maxY() > (float)unk->maxY()) ? db->maxY() : (float)unk->maxY(),
		xMin = (db->minX() < (float)unk->minX()) ? db->minX() : (float)unk->minX(),
		yMin = (db->minY() < (float)unk->minY()) ? db->minY() : (float)unk->minY();

	float
		xRange = xMax - xMin,
		yRange = yMax - yMin;

	float
		heightRatio = FIN_IMAGE_HEIGHT / yRange,
		widthRatio = FIN_IMAGE_WIDTH / xRange;

	float ratio;

	int xOffset = 0, yOffset = 0;

	if (heightRatio < widthRatio) {
		ratio = heightRatio;
		xOffset = (int) round((FIN_IMAGE_WIDTH - ratio * xRange) / 2);
	} else {
		ratio = widthRatio;
		yOffset = (int) round((FIN_IMAGE_HEIGHT - ratio * yRange) / 2);
	}

	unsigned i; // declared here for MSVC++ compatibility

	// draw line from point to point in RED

	int xCoord1 = (int) round((x1 - xMin) * ratio + xOffset);
	int yCoord1 = (int) round((y1 - yMin) * ratio + yOffset);
	int xCoord2 = (int) round((x2 - xMin) * ratio + xOffset);
	int yCoord2 = (int) round((y2 - yMin) * ratio + yOffset);

	gdk_draw_line(
			mDrawingAreaOutlines->window,
			mGC1,
			xCoord1,
			yCoord1,
			xCoord2,
			yCoord2);

	//g_usleep(10000); // 0.01 second delay

#endif
}


/////////////////// CREATE one of two MATCHING DIALOGS ////////////////////


#ifndef CREATE_USER_MATCHING_DIALOG

// use debugging and testing version of dialog

//*******************************************************************
//
// GtkWidget* MatchingDialog::createMatchingDialog()
//
//    NEW VERSION -- primarily for developer use, testing, etc 
//
GtkWidget* MatchingDialog::createMatchingDialog()
{
	GtkWidget *matchingDialog;
	GtkWidget *hpaned, *hpanedTop, *hpanedBottom;
	GtkWidget *frame;
	GtkWidget *paneLeft;
	GtkWidget *paneRight;
	GtkAccelGroup *accel_group;

	GSList *radio_group;
	GtkWidget *radioButton;
	GtkWidget *radioButtonBox;
	GtkWidget *categoryCheckButton;
	GtkWidget *vbox, *hbox,*frameVbox;
	GtkWidget *buttonBox;

	accel_group = gtk_accel_group_new ();

	//------------- old for now --------------
	GtkWidget *matchingVBox;
	GtkWidget *matchingLabel;
	GtkWidget *buttonLabel;
	GtkWidget *dialog_action_area1;
	GtkWidget *matchingHButtonBox;
	GtkWidget *matchingButtonPause;
	guint matchingButtonCancel_key;
	GtkWidget *matchingButtonCancel;

	GtkWidget *tmpIcon, *tmpLabel, *tmpBox;

	//----------------------------------------
	// the dialog has two predefined sections 
	// the top is a vbox
	// the bottom is an action area 
	matchingDialog = gtk_dialog_new ();
	gtk_object_set_data (GTK_OBJECT (matchingDialog), "matchingDialog", matchingDialog);
	gtk_window_set_title (GTK_WINDOW (matchingDialog), _("Matching Fin..."));
	GTK_WINDOW (matchingDialog)->type = WINDOW_DIALOG;
	gtk_window_set_position (GTK_WINDOW (matchingDialog), GTK_WIN_POS_CENTER);
	gtk_window_set_policy (GTK_WINDOW (matchingDialog), TRUE, TRUE, FALSE);
	gtk_window_set_wmclass(GTK_WINDOW(matchingDialog), "darwin_matching", "DARWIN");

	// create horizontally paned container and place it in the vbox of the dialog
	hpaned = gtk_hpaned_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(matchingDialog)->vbox), hpaned, TRUE, TRUE, 0);

	// create stuff for the action area -- will contain buttons to ...
	// 1 - initiate, pause, contine, cancel
	// 2 - return to trace window ???
	dialog_action_area1 = GTK_DIALOG (matchingDialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (matchingDialog), "dialog_action_area1", dialog_action_area1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 6);

	// Create a horizontally packed box for the pause & cancel buttons and 
	// place it below the progress bar
	matchingHButtonBox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (dialog_action_area1), matchingHButtonBox, TRUE, TRUE, 0);

	// this stuff all goes in the LEFT of the vbox section of the dialog
	// 
	// create the left panes (Top & Bottom) -- will contain multiple frames to ...
	// 1 - select method of matching (i.e., method of creating measure of mismatch)
	// 2 - select amount of database to search (1 dolphin,. all, catalog, etc)
	// 3 - show/hide fin alignment during search ???
	// 4 - dump search data to file ???

	// vertically paned container on left
	paneLeft = gtk_vpaned_new();
	gtk_widget_show(paneLeft);
	gtk_paned_add1(GTK_PANED(hpaned), paneLeft);

	// a framed, vertically organized box in the TOP of the paneLeft container.
	// this is where the categories and associated buttons go
	frame = gtk_frame_new(_("Search ONLY Selected Categories:"));
	gtk_widget_show(frame);
	gtk_paned_add1 (GTK_PANED (paneLeft), frame);
	frameVbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(frameVbox);
	gtk_container_add (GTK_CONTAINER (frame), frameVbox);

   	// a horizontally arranged box for the actual Catalog Category checkboxes
	hpanedTop = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hpanedTop);
	gtk_box_pack_start(GTK_BOX(frameVbox), hpanedTop, TRUE, TRUE, 6);

	// a framed, horizontally arranged box in the BOTTOM of the paneLeft container.
	// this is where the selection of Matching Method goes
	frame = gtk_frame_new(_("Select a Search Method:"));
	gtk_widget_show(frame);
	gtk_paned_add2 (GTK_PANED (paneLeft), frame);
	hpanedBottom = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hpanedBottom);
	gtk_container_add (GTK_CONTAINER (frame), hpanedBottom);

	// the right pane -- it contains ...
	// 1 - aligned outlines
	// 2 - progress meter ...
	paneRight = gtk_vbox_new(FALSE,0);
	gtk_paned_add2(GTK_PANED(hpaned), paneRight);

	// fill in the Catalog Category related selections in the TOP pane

	// NOTE: These categories are currently based on the Eckerd College database

	// set up categories and buttons in 5 columns
	int catColumnHeight = 1 + (mDatabase->catCategoryNamesMax() / 5); //***2.01
	for (int catID=0; catID < mDatabase->catCategoryNamesMax(); catID++) //***2.01
	{
		if (catID % catColumnHeight == 0)
		{
			// create a new vbox for the next column of buttons
			vbox = gtk_vbox_new(FALSE, 0);
			gtk_widget_show(vbox);
			gtk_container_add(GTK_CONTAINER(hpanedTop), vbox);
		}
		// create a button for the next category
		if ("NONE" == mDatabase->catCategoryName(catID)) //***2.01
			mCategoryButton[catID] = gtk_check_button_new_with_label(_("Unspecified"));
		else
			mCategoryButton[catID] = gtk_check_button_new_with_label(
				_(mDatabase->catCategoryName(catID).c_str()));
		gtk_container_add(GTK_CONTAINER(vbox), mCategoryButton[catID]);
		gtk_widget_show(mCategoryButton[catID]);
		// set button as active if it matches the category of the unkown
		// selected in the TraceWindow
		if (mDatabase->catCategoryName(catID) == (this)->mFin->mDamageCategory)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mCategoryButton[catID]),TRUE);
			mCategoryToMatch[catID] = TRUE;
			mCategoriesSelected++;
		}
		else
			mCategoryToMatch[catID] = FALSE;
		
		gtk_signal_connect (GTK_OBJECT(mCategoryButton[catID]),"toggled",
			GTK_SIGNAL_FUNC (on_mCategoryButton_toggled),
			(void *) this);
	}

	// drop down and put ALL or NONE buttons below category check boxes
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(frameVbox), hbox, TRUE, TRUE, 6);

	buttonBox = gtk_hbutton_box_new ();
	gtk_widget_show (buttonBox);
	gtk_box_pack_start (GTK_BOX (hbox), buttonBox, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonBox), GTK_BUTTONBOX_SPREAD);

	// ALL or NONE
	categoryCheckButton = gtk_button_new_with_label(_("Select All"));
	gtk_container_add(GTK_CONTAINER(buttonBox), categoryCheckButton);
	gtk_widget_show(categoryCheckButton);

	gtk_signal_connect (GTK_OBJECT(categoryCheckButton),"clicked",
			GTK_SIGNAL_FUNC (on_categoryCheckButtonAll_clicked),
			(void *) this);

	categoryCheckButton = gtk_button_new_with_label(_("Clear All"));
	gtk_container_add(GTK_CONTAINER(buttonBox), categoryCheckButton);
	gtk_widget_show(categoryCheckButton);

	gtk_signal_connect (GTK_OBJECT(categoryCheckButton),"clicked",
			GTK_SIGNAL_FUNC (on_categoryCheckButtonClear_clicked),
			(void *) this);

	// fill in the Method of match related selection in the BOTTOM pane

	radioButtonBox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(radioButtonBox);
	gtk_container_add(GTK_CONTAINER(/*paneLeft*/hpanedBottom), radioButtonBox);

	// button for original method of match / fin alignment using Start, Tip & Notch
	radioButton = gtk_radio_button_new_with_label(NULL,"Original Method");
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                    GTK_SIGNAL_FUNC (on_radioOriginal_clicked),
	                    (void *) this);

	// button for aligning leading edges by "trimming" in 1/20th increments
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Best of 13"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                    GTK_SIGNAL_FUNC (on_radioTrimFixed_clicked),
	                    (void *) this);

	// button for aligning leading edges using optimizing approach
	// from leading edge begin to tip
	/*
	//***1.5 - leave this button out now, this matching method does not work well enough
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Align Optimally - JHS"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
                        GTK_SIGNAL_FUNC (on_radioTrimOptimal_clicked),
                        (void *) this);
	*/

	//***051OM
	// button for aligning leading edges using optimizing approach
	// from leading edge begin to tip
	/*
	//***1.5 - leave this button out now, this matching method has been superseded
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Align Optimally - TOTAL"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
                        GTK_SIGNAL_FUNC (on_radioTrimOptimalTotal_clicked),
                        (void *) this);
	*/

	//***1.1begin

	// new buttons for testing Tip movement and in/out movement of ends

	// button for aligning leading edges using optimizing approach
	// from leading edge begin to trailing edge end
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Iterative (Ends & Tip)"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (radioButton), TRUE); //***1.5

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
                        GTK_SIGNAL_FUNC (on_radioTrimOptimalTip_clicked),
                        (void *) this);

	//***1.85 - new button for aligning using the area between contours as metric
	// button for aligning leading edges using optimizing approach
	// from leading edge begin to to trailing edge end
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Iterative (Ends & Tip) - Area metric"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	// let old method be the active one
	//gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (radioButton), TRUE); //***1.5

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
                        GTK_SIGNAL_FUNC (on_radioTrimOptimalArea_clicked),
                        (void *) this);

	// button for aligning leading edges using optimizing approach
	// from leading edge begin to tip
	/*
	//***1.5 - remove for now, useful when ready to test new methods
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Align Optimally - IN&OUT"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
                        GTK_SIGNAL_FUNC (on_radioTrimOptimalInOut_clicked),
                        (void *) this);
	*/

	// button for aligning leading edges using optimizing approach
	// from leading edge begin to tip
	/*
	//***1.5 - leave this button out now, this matching method will never be evaluated
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Align Optimally - IN&OUT&TIP"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
                        GTK_SIGNAL_FUNC (on_radioTrimOptimalInOutTip_clicked),
                        (void *) this);
	*/

	//***1.1end 


	// second set of radio buttons to choose the range of Outline
	// segments used in the registration process

	radioButtonBox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(radioButtonBox);
	gtk_container_add(GTK_CONTAINER(hpanedBottom), radioButtonBox);

	// put in a label for the radio buttons

	//buttonLabel = gtk_label_new (_("Select Range of Points used to test\nAlign Optimally - JHS\n(These have NO effects on other methods!)"));
	buttonLabel = gtk_label_new (_("Range of Points used to compute\n"
		                           "the error for ranking results.\n"
		                           "-- These ONLY affect the\n"
								   "-- Align Iteratively methods!"));
	gtk_widget_show (buttonLabel);
	gtk_box_pack_start (GTK_BOX (radioButtonBox), buttonLabel, FALSE, FALSE, 6);

	// button for All Points used to compute final error for use in ranking results

	radioButton = gtk_radio_button_new_with_label(NULL,"All Points");
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                   GTK_SIGNAL_FUNC (on_radioAllPoints_clicked),
	                   (void *) this);

	// button for Trailing Edge Only used to compute final error for use in ranking results

	radioButton = gtk_radio_button_new_with_label(radio_group,_("Trailing Edge Only"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                   GTK_SIGNAL_FUNC (on_radioTrailingOnly_clicked),
	                   (void *) this);

	// button for leading edge to NOTCH Only

	/*
	//***1.5 - no longer needed
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Leading Edge to NOTCH Only"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                   GTK_SIGNAL_FUNC (on_radioLeadToNotchOnly_clicked),
	                   (void *) this);
	*/

	// button for two stage Optimization (Lead to TIP then Lead To Trail)
	// causses trimming of leading edge then trimming of trailing edge

	/*
	//***1.5 - no longer needed
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Leading Edge then Trailing Edge"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
                      GTK_SIGNAL_FUNC (on_radioLeadThenTrail_clicked),
                      (void *) this);
	*/

	// create vertically packed box in right pane

	matchingVBox = gtk_vbox_new(FALSE, 0);
	gtk_object_set_data (GTK_OBJECT (matchingDialog), "matchingVBox", matchingVBox);
	gtk_container_add(GTK_CONTAINER(paneRight), matchingVBox);
	gtk_container_set_border_width (GTK_CONTAINER (matchingVBox), 4);

	// framed area for display of outline registration

	frame = gtk_frame_new(_("Registered Outlines"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX (matchingVBox), frame, FALSE, FALSE, 6);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

	GtkWidget *eventBox = gtk_event_box_new();
	gtk_widget_show(eventBox);
	gtk_container_add(GTK_CONTAINER(frame), eventBox);

	mDrawingAreaOutlines = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(mDrawingAreaOutlines),
					FIN_IMAGE_WIDTH, FIN_IMAGE_HEIGHT);
	gtk_widget_show(mDrawingAreaOutlines);
	gtk_container_add(GTK_CONTAINER(eventBox), mDrawingAreaOutlines);

	gtk_signal_connect(GTK_OBJECT(mDrawingAreaOutlines), "expose_event",
		       GTK_SIGNAL_FUNC
		       (on_mMatchDialogDrawingAreaOutlines_expose_event), (void*)this);

	// checkbox to control outline registration display

	mButtonShowHide = gtk_check_button_new_with_label(_("Show Registration of Fin Outlines"));
	gtk_widget_show(mButtonShowHide);
	gtk_box_pack_start(GTK_BOX (matchingVBox), mButtonShowHide, FALSE, FALSE, 6);

	gtk_signal_connect(GTK_OBJECT(mButtonShowHide), "toggled",
		       GTK_SIGNAL_FUNC
		       (on_mButtonShowHide_toggled), (void*)this);

	// Place label "Progress:" above the sliding progress bar

	matchingLabel = gtk_label_new (_("Progress:"));
	gtk_widget_show (matchingLabel);
	gtk_box_pack_start (GTK_BOX (matchingVBox), matchingLabel, FALSE, FALSE, 6);

	// Place the progress bar below the label

	mProgressBar = gtk_progress_bar_new ();
	gtk_widget_show (mProgressBar);
	gtk_box_pack_start (GTK_BOX (matchingVBox), mProgressBar, FALSE, FALSE, 0);
	gtk_progress_set_show_text (GTK_PROGRESS (mProgressBar), TRUE);

	dialog_action_area1 = GTK_DIALOG (matchingDialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (matchingDialog), "dialog_action_area1", dialog_action_area1);
	gtk_widget_show (dialog_action_area1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 6);

	// Create a horizontally packed box for the pause & cancel buttons and 
	// place it below the progress bar

	matchingHButtonBox = gtk_hbutton_box_new ();
	gtk_widget_show (matchingHButtonBox);
	gtk_box_pack_start (GTK_BOX (dialog_action_area1), matchingHButtonBox, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (matchingHButtonBox), GTK_BUTTONBOX_END);

	// Place the "start" button on the left

	mButtonStartStop = gtk_button_new_with_label (_("Start"));
	gtk_widget_show (mButtonStartStop);
	gtk_container_add (GTK_CONTAINER (matchingHButtonBox), mButtonStartStop);
	GTK_WIDGET_SET_FLAGS (mButtonStartStop, GTK_CAN_DEFAULT);


	// Place the "pause/continue" button right of start/stop

	mButtonPauseContinue = gtk_button_new_with_label (_("Pause"));
	gtk_widget_show (mButtonPauseContinue);
	gtk_container_add (GTK_CONTAINER (matchingHButtonBox), mButtonPauseContinue);

	// Place the "Cancel" button on the right

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);
  
	gtk_widget_show_all (matchingVBox);
	gtk_widget_show_all(hpaned);

	matchingButtonCancel = gtk_button_new();
	matchingButtonCancel_key = gtk_label_parse_uline(GTK_LABEL(tmpLabel),
                                   _("_Cancel"));
	gtk_widget_add_accelerator (matchingButtonCancel, "clicked", accel_group,
                              matchingButtonCancel_key, GDK_MOD1_MASK, (GtkAccelFlags)0);
	gtk_container_add(GTK_CONTAINER(matchingButtonCancel), tmpBox);
	gtk_widget_show (matchingButtonCancel);
	gtk_container_add (GTK_CONTAINER (matchingHButtonBox), matchingButtonCancel);
	GTK_WIDGET_SET_FLAGS (matchingButtonCancel, GTK_CAN_DEFAULT);

	// <ctrl-C> and <esc> are equivalent to clicking Cancel button
	gtk_widget_add_accelerator (matchingButtonCancel, "clicked", accel_group,
	                           GDK_C, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (matchingButtonCancel, "clicked", accel_group,
	                           GDK_Escape, (GdkModifierType)0,
	                           GTK_ACCEL_VISIBLE);

	// register callbacks for window delete and button events
	gtk_signal_connect (GTK_OBJECT (matchingDialog), "delete_event",
	                   GTK_SIGNAL_FUNC (on_matchingDialog_delete_event),
	                   (void *) this);
	gtk_signal_connect (GTK_OBJECT (mButtonPauseContinue), "clicked",
	                   GTK_SIGNAL_FUNC (on_matchingButtonPauseContinue_clicked),
	                   (void *) this);
	gtk_signal_connect (GTK_OBJECT (mButtonStartStop), "clicked",
	                   GTK_SIGNAL_FUNC (on_matchingButtonStartStop_clicked),
	                   (void *) this);
	gtk_signal_connect (GTK_OBJECT (matchingButtonCancel), "clicked",
	                   GTK_SIGNAL_FUNC (on_matchingButtonCancel_clicked),
	                   (void *) this);

	gtk_window_add_accel_group (GTK_WINDOW (matchingDialog), accel_group);

	return matchingDialog;
}


#else // Create the END USER version of the Matching Dialog


//*******************************************************************
//
// GtkWidget* MatchingDialog::createMatchingDialog()
//
//    This is the END USER version - and does not allow for 
//    selection of as many matching methods / parameters.
//
GtkWidget* MatchingDialog::createMatchingDialog()
{
	GtkWidget *matchingDialog;
	GtkWidget *hpaned, *hpanedTop, *hpanedBottom;
	GtkWidget *frame;
	GtkWidget *paneLeft;
	GtkWidget *paneRight;
	GtkAccelGroup *accel_group;

	GSList *radio_group;
	GtkWidget *radioButton;
	GtkWidget *radioButtonBox;
	GtkWidget *categoryCheckButton;
	GtkWidget *vbox, *hbox,*frameVbox;
	GtkWidget *buttonBox;

	accel_group = gtk_accel_group_new ();

	//------------- old for now --------------
	GtkWidget *matchingVBox;
	GtkWidget *matchingLabel;
	GtkWidget *buttonLabel;
	GtkWidget *dialog_action_area1;
	GtkWidget *matchingHButtonBox;
	//GtkWidget *matchingButtonPause;
	guint matchingButtonCancel_key;
	GtkWidget *matchingButtonCancel;

	GtkWidget *tmpIcon, *tmpLabel, *tmpBox;

	//----------------------------------------
	// the dialog has two predefined sections 
	// the top is a vbox
	// the bottom is an action area 
	matchingDialog = gtk_dialog_new ();
	gtk_object_set_data (GTK_OBJECT (matchingDialog), "matchingDialog", matchingDialog);
	gtk_window_set_title (GTK_WINDOW (matchingDialog), _("Matching Fin ..."));
	GTK_WINDOW (matchingDialog)->type = WINDOW_DIALOG;
	gtk_window_set_position (GTK_WINDOW (matchingDialog), GTK_WIN_POS_CENTER);
	gtk_window_set_policy (GTK_WINDOW (matchingDialog), TRUE, TRUE, FALSE);
	gtk_window_set_wmclass(GTK_WINDOW(matchingDialog), "darwin_matching", "DARWIN");

	// create horizontally paned container and place it in the vbox of the dialog
	hpaned = gtk_hpaned_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(matchingDialog)->vbox), hpaned, TRUE, TRUE, 0);

	// create stuff for the action area -- will contain buttons to ...
	// 1 - initiate, pause, contine, cancel
	// 2 - return to trace window ???
	dialog_action_area1 = GTK_DIALOG (matchingDialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (matchingDialog), "dialog_action_area1", dialog_action_area1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 6);

	// Create a horizontally packed box for the pause & cancel buttons and 
	// place it below the progress bar
	matchingHButtonBox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX (dialog_action_area1), matchingHButtonBox, TRUE, TRUE, 0);

	// this stuff all goes in the LEFT of the vbox section of the dialog
	// 
	// create the left panes (Top & Bottom) -- will contain multiple frames to ...
	// 1 - select method of matching (i.e., method of creating measure of mismatch)
	// 2 - select amount of database to search (1 dolphin,. all, catalog, etc)
	// 3 - show/hide fin alignment during search ???
	// 4 - dump search data to file ???

	// vertically paned container on left
	paneLeft = gtk_vpaned_new();
	gtk_widget_show(paneLeft);
	gtk_paned_add1(GTK_PANED(hpaned), paneLeft);

	// a framed, vertically organized box in the TOP of the paneLeft container.
	// this is where the categories and associated buttons go
	frame = gtk_frame_new(_("Search ONLY Selected Categories:"));
	gtk_widget_show(frame);
	gtk_paned_add1 (GTK_PANED (paneLeft), frame);
	frameVbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(frameVbox);
	gtk_container_add (GTK_CONTAINER (frame), frameVbox);

   	// a horizontally arranged box for the actual Catalog Category checkboxes
	hpanedTop = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hpanedTop);
	gtk_box_pack_start(GTK_BOX(frameVbox), hpanedTop, TRUE, TRUE, 6);

	// a framed, horizontally arranged box in the BOTTOM of the paneLeft container.
	// this is where the selection of Matching Method goes
	frame = gtk_frame_new(_("Select a Search Method:"));
	gtk_widget_show(frame);
	gtk_paned_add2 (GTK_PANED (paneLeft), frame);
	hpanedBottom = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hpanedBottom);
	gtk_container_add (GTK_CONTAINER (frame), hpanedBottom);

	// the right pane -- it contains ...
	// 1 - aligned outlines
	// 2 - progress meter ...
	paneRight = gtk_vbox_new(FALSE,0);
	gtk_paned_add2(GTK_PANED(hpaned), paneRight);

	// fill in the Catalog Category related selections in the TOP pane

	// NOTE: These categories are currently based on the Eckerd College database

	// set up categories and buttons in 5 columns
	int catColumnHeight = 1 + (mDatabase->catCategoryNamesMax() / 5);
	for (int catID=0; catID < mDatabase->catCategoryNamesMax(); catID++)
	{
		if (catID % catColumnHeight == 0)
		{
			// create a new vbox for the next column of buttons
			vbox = gtk_vbox_new(FALSE, 0);
			gtk_widget_show(vbox);
			gtk_container_add(GTK_CONTAINER(hpanedTop), vbox);
		}
		// create a button for the next category
		if ("NONE" == mDatabase->catCategoryName(catID))
			mCategoryButton[catID] = gtk_check_button_new_with_label(_("Unspecified"));
		else
			mCategoryButton[catID] = gtk_check_button_new_with_label(
				_(mDatabase->catCategoryName(catID).c_str()));
		gtk_container_add(GTK_CONTAINER(vbox), mCategoryButton[catID]);
		gtk_widget_show(mCategoryButton[catID]);
		// set button as active if it matches the category of the unkown
		// selected in the TraceWindow
		if (mDatabase->catCategoryName(catID) == (this)->mFin->mDamageCategory)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mCategoryButton[catID]),TRUE);
			mCategoryToMatch[catID] = TRUE;
			mCategoriesSelected++;
		}
		else
			mCategoryToMatch[catID] = FALSE;
		
		gtk_signal_connect (GTK_OBJECT(mCategoryButton[catID]),"toggled",
			GTK_SIGNAL_FUNC (on_mCategoryButton_toggled),
			(void *) this);
	}

	// drop down and put ALL or NONE buttons below category check boxes
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(frameVbox), hbox, TRUE, TRUE, 6);

	buttonBox = gtk_hbutton_box_new ();
	gtk_widget_show (buttonBox);
	gtk_box_pack_start (GTK_BOX (hbox), buttonBox, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonBox), GTK_BUTTONBOX_SPREAD);

	// ALL or NONE
	categoryCheckButton = gtk_button_new_with_label(_("Select All"));
	gtk_container_add(GTK_CONTAINER(buttonBox), categoryCheckButton);
	gtk_widget_show(categoryCheckButton);

	gtk_signal_connect (GTK_OBJECT(categoryCheckButton),"clicked",
			GTK_SIGNAL_FUNC (on_categoryCheckButtonAll_clicked),
			(void *) this);

	categoryCheckButton = gtk_button_new_with_label(_("Clear All"));
	gtk_container_add(GTK_CONTAINER(buttonBox), categoryCheckButton);
	gtk_widget_show(categoryCheckButton);

	gtk_signal_connect (GTK_OBJECT(categoryCheckButton),"clicked",
			GTK_SIGNAL_FUNC (on_categoryCheckButtonClear_clicked),
			(void *) this);

	// fill in the Method of match related selection in the BOTTOM pane

	radioButtonBox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(radioButtonBox);
	gtk_container_add(GTK_CONTAINER(/*paneLeft*/hpanedBottom), radioButtonBox);

/* no longer used - 11/15/2005

	// button for original method of match / fin alignment using Start, Tip & Notch
	radioButton = gtk_radio_button_new_with_label(NULL,"Original Method");
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                    GTK_SIGNAL_FUNC (on_radioOriginal_clicked),
	                    (void *) this);
*/
	// button for alligning leading edges by "trimming" in 1/20th increments
	radioButton = gtk_radio_button_new_with_label(/*radio_group*/NULL,_("Align Quick & Dirty")); //***055MD
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                    GTK_SIGNAL_FUNC (on_radioTrimFixed_clicked),
	                    (void *) this);

/* no longer used -- 11/15/2005

	// button for aligning leading edges using optimizing approach
	// from leading edge begin to tip
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Align Optimally - JHS"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
                        GTK_SIGNAL_FUNC (on_radioTrimOptimal_clicked),
                        (void *) this);
*/

	//***051OM
	// button for aligning leading edges using optimizing approach
	// from leading edge begin to tip
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Align Iteratively"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (radioButton), TRUE); //***1.5

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
		                //***1.3 - method swapped to optimal with moving TIP
                        //GTK_SIGNAL_FUNC (on_radioTrimOptimalTotal_clicked),
                        GTK_SIGNAL_FUNC (on_radioTrimOptimalTip_clicked), //***1.3
                        (void *) this);

	// second set of radio buttons to choose the range of Outline
	// segments used in the registration process

	radioButtonBox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(radioButtonBox);
	gtk_container_add(GTK_CONTAINER(hpanedBottom), radioButtonBox);


	//***1.6 - new ability to select range of points used in ranking results

	// second set of radio buttons to choose the range of Outline
	// segments used in the registration process

	radioButtonBox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(radioButtonBox);
	gtk_container_add(GTK_CONTAINER(hpanedBottom), radioButtonBox);

	// put in a label for the radio buttons

	//buttonLabel = gtk_label_new (_("Select Range of Points used to test\nAlign Optimally - JHS\n(These have NO effects on other methods!)"));
	buttonLabel = gtk_label_new (_("Range of Points used to compute\n"
		                           "the error for ranking results.\n"
		                           "-- These ONLY affect the\n"
								   "-- Align Iteratively methods!"));
	gtk_widget_show (buttonLabel);
	gtk_box_pack_start (GTK_BOX (radioButtonBox), buttonLabel, FALSE, FALSE, 6);

	// button for All Points used to compute final error for use in ranking results

	radioButton = gtk_radio_button_new_with_label(NULL,"All Points");
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                   GTK_SIGNAL_FUNC (on_radioAllPoints_clicked),
	                   (void *) this);

	// button for Trailing Edge Only used to compute final error for use in ranking results

	radioButton = gtk_radio_button_new_with_label(radio_group,_("Trailing Edge Only"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                   GTK_SIGNAL_FUNC (on_radioTrailingOnly_clicked),
	                   (void *) this);

	//***1.6 - end of new section

/* no longer used - 11/15/2005

	// put in a label for the radio buttons

	buttonLabel = gtk_label_new (_("Select Range of Points used to test\nAlign Optimally - JHS\n(These have NO effects on other methods!)"));
	gtk_widget_show (buttonLabel);
	gtk_box_pack_start (GTK_BOX (radioButtonBox), buttonLabel, FALSE, FALSE, 6);

	// button for All Points Used

	radioButton = gtk_radio_button_new_with_label(NULL,"All Points");
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                   GTK_SIGNAL_FUNC (on_radioAllPoints_clicked),
	                   (void *) this);

	// button for leading edge to TIP Only

	radioButton = gtk_radio_button_new_with_label(radio_group,_("Leading Edge to TIP Only"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                   GTK_SIGNAL_FUNC (on_radioLeadToTipOnly_clicked),
	                   (void *) this);

	// button for leading edge to NOTCH Only

	radioButton = gtk_radio_button_new_with_label(radio_group,_("Leading Edge to NOTCH Only"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                   GTK_SIGNAL_FUNC (on_radioLeadToNotchOnly_clicked),
	                   (void *) this);

	// button for two stage Optimization (Lead to TIP then Lead To Trail)
	// causses trimming of leading edge then trimming of trailing edge

	radioButton = gtk_radio_button_new_with_label(radio_group,_("Leading Edge then Trailing Edge"));
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 0);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
                      GTK_SIGNAL_FUNC (on_radioLeadThenTrail_clicked),
                      (void *) this);
*/

	// create vertically packed box in right pane

	matchingVBox = gtk_vbox_new(FALSE, 0);
	gtk_object_set_data (GTK_OBJECT (matchingDialog), "matchingVBox", matchingVBox);
	gtk_container_add(GTK_CONTAINER(paneRight), matchingVBox);
	gtk_container_set_border_width (GTK_CONTAINER (matchingVBox), 4);

	// framed area for display of outline registration

	frame = gtk_frame_new(_("Registered Outlines"));
	gtk_widget_show(frame);
	gtk_box_pack_start(GTK_BOX (matchingVBox), frame, FALSE, FALSE, 6);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

	GtkWidget *eventBox = gtk_event_box_new();
	gtk_widget_show(eventBox);
	gtk_container_add(GTK_CONTAINER(frame), eventBox);

	mDrawingAreaOutlines = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(mDrawingAreaOutlines),
					FIN_IMAGE_WIDTH, FIN_IMAGE_HEIGHT);
	gtk_widget_show(mDrawingAreaOutlines);
	gtk_container_add(GTK_CONTAINER(eventBox), mDrawingAreaOutlines);

	gtk_signal_connect(GTK_OBJECT(mDrawingAreaOutlines), "expose_event",
		       GTK_SIGNAL_FUNC
		       (on_mMatchDialogDrawingAreaOutlines_expose_event), (void*)this);

	// checkbox to control outline registration display

	mButtonShowHide = gtk_check_button_new_with_label(_("Show Registration of Fin Outlines"));
	gtk_widget_show(mButtonShowHide);
	gtk_box_pack_start(GTK_BOX (matchingVBox), mButtonShowHide, FALSE, FALSE, 6);

	gtk_signal_connect(GTK_OBJECT(mButtonShowHide), "toggled",
		       GTK_SIGNAL_FUNC
		       (on_mButtonShowHide_toggled), (void*)this);

	// Place label "Progress:" above the sliding progress bar

	matchingLabel = gtk_label_new (_("Progress:"));
	gtk_widget_show (matchingLabel);
	gtk_box_pack_start (GTK_BOX (matchingVBox), matchingLabel, FALSE, FALSE, 6);

	// Place the progress bar below the label

	mProgressBar = gtk_progress_bar_new ();
	gtk_widget_show (mProgressBar);
	gtk_box_pack_start (GTK_BOX (matchingVBox), mProgressBar, FALSE, FALSE, 0);
	gtk_progress_set_show_text (GTK_PROGRESS (mProgressBar), TRUE);

	dialog_action_area1 = GTK_DIALOG (matchingDialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (matchingDialog), "dialog_action_area1", dialog_action_area1);
	gtk_widget_show (dialog_action_area1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 6);

	// Create a horizontally packed box for the pause & cancel buttons and 
	// place it below the progress bar

	matchingHButtonBox = gtk_hbutton_box_new ();
	gtk_widget_show (matchingHButtonBox);
	gtk_box_pack_start (GTK_BOX (dialog_action_area1), matchingHButtonBox, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (matchingHButtonBox), GTK_BUTTONBOX_END);

	// Place the "start" button on the left

	mButtonStartStop = gtk_button_new_with_label (_("Start"));
	gtk_widget_show (mButtonStartStop);
	gtk_container_add (GTK_CONTAINER (matchingHButtonBox), mButtonStartStop);
	GTK_WIDGET_SET_FLAGS (mButtonStartStop, GTK_CAN_DEFAULT);


	// Place the "pause/continue" button right of start/stop

	mButtonPauseContinue = gtk_button_new_with_label (_("Pause"));
	gtk_widget_show (mButtonPauseContinue);
	gtk_container_add (GTK_CONTAINER (matchingHButtonBox), mButtonPauseContinue);

	// Place the "Cancel" button on the right

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);
  
	gtk_widget_show_all (matchingVBox);
	gtk_widget_show_all(hpaned);

	matchingButtonCancel = gtk_button_new();
	matchingButtonCancel_key = gtk_label_parse_uline(GTK_LABEL(tmpLabel),
                                   _("_Cancel"));
	gtk_widget_add_accelerator (matchingButtonCancel, "clicked", accel_group,
                              matchingButtonCancel_key, GDK_MOD1_MASK, (GtkAccelFlags)0);
	gtk_container_add(GTK_CONTAINER(matchingButtonCancel), tmpBox);
	gtk_widget_show (matchingButtonCancel);
	gtk_container_add (GTK_CONTAINER (matchingHButtonBox), matchingButtonCancel);
	GTK_WIDGET_SET_FLAGS (matchingButtonCancel, GTK_CAN_DEFAULT);

	// <ctrl-C> and <esc> are equivalent to clicking Cancel button
	gtk_widget_add_accelerator (matchingButtonCancel, "clicked", accel_group,
	                           GDK_C, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (matchingButtonCancel, "clicked", accel_group,
	                           GDK_Escape, (GdkModifierType)0,
	                           GTK_ACCEL_VISIBLE);

	// register callbacks for window delete and button events
	gtk_signal_connect (GTK_OBJECT (matchingDialog), "delete_event",
	                   GTK_SIGNAL_FUNC (on_matchingDialog_delete_event),
	                   (void *) this);
	gtk_signal_connect (GTK_OBJECT (mButtonPauseContinue), "clicked",
	                   GTK_SIGNAL_FUNC (on_matchingButtonPauseContinue_clicked),
	                   (void *) this);
	gtk_signal_connect (GTK_OBJECT (mButtonStartStop), "clicked",
	                   GTK_SIGNAL_FUNC (on_matchingButtonStartStop_clicked),
	                   (void *) this);
	gtk_signal_connect (GTK_OBJECT (matchingButtonCancel), "clicked",
	                   GTK_SIGNAL_FUNC (on_matchingButtonCancel_clicked),
	                   (void *) this);

	gtk_window_add_accel_group (GTK_WINDOW (matchingDialog), accel_group);

	return matchingDialog;
}


#endif // done with creation of Matching Dialog

//*******************************************************************
//
// returns pointer to Gtk Window
//
GtkWidget* MatchingDialog::getWindow()
{
	return mDialog;
}

//*******************************************************************
//
// gboolean on_mMatchDialogDrawingAreaOutlines_expose_event(...)
//
//    NEW NEW NEW
//
gboolean on_mMatchDialogDrawingAreaOutlines_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData
	)
{
	MatchingDialog *matchWin = (MatchingDialog *) userData;

	if (NULL == matchWin)
		return FALSE;

	// not sure what to do here

	return TRUE;
}


//*******************************************************************

void on_mCategoryButton_toggled(
	GtkButton *button,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *) userData;

	if (NULL == dlg)
		return;

	// some ONE category button was toggled, so make sure all flags 
	// are consistent with current check button states

	for (int catID=0; catID < dlg->mDatabase->catCategoryNamesMax(); catID++)
	{
		bool checked = gtk_toggle_button_get_active(
			                    GTK_TOGGLE_BUTTON(dlg->mCategoryButton[catID]));
		
		if (! (checked == dlg->mCategoryToMatch[catID]))
		{
			dlg->mCategoryToMatch[catID] = checked;
			dlg->mCategoriesSelected = (checked) 
				? dlg->mCategoriesSelected + 1 
				: dlg->mCategoriesSelected - 1;
		}

#ifdef DEBUG
		g_print("%d ",dlg->mCategoryToMatch[catID]);
#endif

	}

#ifdef DEBUG
	g_print("%d\n",dlg->mCategoriesSelected);
#endif

}


//*******************************************************************

void on_categoryCheckButtonAll_clicked(
	GtkButton *button,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *) userData;

	if (NULL == dlg)
		return;

	// make sure all flags and check buttons are TRUE (checked)

	for (int catID=0; catID < dlg->mDatabase->catCategoryNamesMax(); catID++)
		if (! dlg->mCategoryToMatch[catID])
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->mCategoryButton[catID]),TRUE);
}


//*******************************************************************

void on_categoryCheckButtonClear_clicked(
	GtkButton *button,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *) userData;

	if (NULL == dlg)
		return;

	// make sure all flags and check buttons are FALSE (unchecked)

	for (int catID=0; catID < dlg->mDatabase->catCategoryNamesMax(); catID++)
		if (dlg->mCategoryToMatch[catID])
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->mCategoryButton[catID]),FALSE);
}


//*******************************************************************
//
//
//
//
//
void on_mButtonShowHide_toggled(
	GtkButton *button,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *) userData;

	if (NULL == dlg)
		return;

   // flip the display indication
   dlg->mShowingOutlines = !(dlg->mShowingOutlines);
   // set display pointer used by Match class
   if (dlg->mShowingOutlines)
   		dlg->mMatch->setDisplay(dlg);
   else
	   dlg->mMatch->setDisplay(NULL);
}


//*******************************************************************
//
//
gboolean on_matchingDialog_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *) userData;

	if (NULL == dlg)
		return FALSE;

	dlg->mMatchCancelled = true; //***1.1 - will cause idle function to delete dialog

	return TRUE;
}

//*******************************************************************
//
//
void on_matchingButtonPauseContinue_clicked(
	GtkButton *button,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *) userData;

	if (NULL == dlg)
		return;

   // flip the paused indication
   dlg->mMatchPaused = !(dlg->mMatchPaused);

   // TOGGLE the button label (Continue/Pause) to match new function
   if (dlg->mMatchPaused)
      gtk_button_set_label(GTK_BUTTON(dlg->mButtonPauseContinue),"Continue");
   else 
      gtk_button_set_label(GTK_BUTTON(dlg->mButtonPauseContinue),"Pause");
   gtk_widget_show(dlg->mButtonPauseContinue);
}

//*******************************************************************
// void on_matchingButtonStartStop_clicked(...)
//
//    CALLBACK for the Start/Stop button.  This starts and stops the
//    matching process indirectly through the boolean mMatchRunning
//    that the Idle CALLBACK checks. Matches cannot be initialted unless
//    at least one catalog category is selected.
//
//          current state                           future state
//    mMatchRunning    mCategoriesSelected   mMatchRunning   ButtonLabel
//       false            1 or more            true            "Stop"
//       false               0                 false           "Start"
//       true                                  false           "Start"
//
void on_matchingButtonStartStop_clicked(
	GtkButton *button,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *) userData;

	if (NULL == dlg)
		return;

	// flip the running indication
	dlg->mMatchRunning = !(dlg->mMatchRunning);

	// TOGGLE the button label (Start/Stop) to match new function
	if (dlg->mMatchRunning)
		if (dlg->mCategoriesSelected > 0)
			gtk_button_set_label(GTK_BUTTON(dlg->mButtonStartStop),"Stop!");
		else
		{
			// no categories selected, so do NOT start
			showError("At least ONE Catalog Category\nMust be Selected\nBEFORE Starting Match!"); //***054
			dlg->mMatchRunning = false;
		}
	else
		gtk_button_set_label(GTK_BUTTON(dlg->mButtonStartStop),"Start");
	gtk_widget_show(dlg->mButtonStartStop);

	// for now START/STOP is treated the same as PAUSE/CONTINUE

}

//*******************************************************************
//
// void on_matchingButtonCancel_clicked(...)
//
//    CALLBACK for Cancel button.  This terminates all matching and
//    destroys the dialog (indirectly).
//
void on_matchingButtonCancel_clicked(
	GtkButton *button,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *) userData;

	if (NULL == dlg)
		return;

	dlg->mMatchCancelled = true;
}

//*******************************************************************
//
// gboolean matchingIdleFunction(...)
//
//    Idle CALLBACK.  This drives the matching process.  Actions are
//    driven by values of several booleans and the selected catalog
//    categories.
//
gboolean matchingIdleFunction(
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (NULL == dlg)
		return FALSE; // stop coming back to idle function
	
	if (dlg->mMatchCancelled) {
      // if the match has been cancelled, we end up here for cleanup
      // there is no need to explicitly delete any of the "pointed to" items
      // here.  the delete dlg causes all to be deleted in due time.
		delete dlg;
		return FALSE;
	}

	if (dlg->mMatchPaused || !dlg->mMatchRunning || (dlg->mCategoriesSelected == 0))
		return TRUE;  // do nothing, but keep coming back to this idle function

	// note: not quite sure what action to take when STOP is requested,
	// probably should be to reinitialize the match and be ready to start over
	// for now treat STOP the same as PAUSE

	try {
		float percentComplete = dlg->mMatch->matchSingleFin(
			                          dlg->mRegistrationMethod,
			                          dlg->mRegSegmentsUsed,
									  dlg->mCategoryToMatch,
									  dlg->mUseFullFinError, //***005ER
									  true); //***1.6 - use absolute database positions now, as with queue

		gtk_progress_set_value(
				GTK_PROGRESS(dlg->mProgressBar),
				percentComplete * 100);

		if (percentComplete < 1.0)
			return TRUE;

		// stop the matching without displaying results, since the
		// list of matching fins is EMPTY -- need a popup dialog to inform
		// user here
		if (dlg->mMatch->getMatchResults()->size() == 0)
		{
			showError("Selected Catalog Categories are \nALL EMPTY!"); //***054

			dlg->show(true); // redisplay as if returning from matchresultsWindow
			return FALSE;
		}

		//***1.5 - sort the results here, ONCE, rather than as list is built
		dlg->mMatch->getMatchResults()->sort();

		gtk_widget_hide(dlg->mDialog); // ***2.2 - to hide this behind match results window

		// matching is done and some match, so display results

		MatchResultsWindow *resultsWindow = new MatchResultsWindow(
		                    dlg->mFin,
		                    dlg->mMatch->getMatchResults(), // just a pointer
		                    dlg->mDatabase,
		                    dlg->mMainWin,    //***004CL
							dlg,              //***043MA
							NULL,
							//dlg->mDialog,     //***1.3
							"", //***1.6 - NOT loading results from file (no result filename passed)
		                    dlg->mOptions);
		resultsWindow->show();

		//delete dlg; defer this for MatchResultsWindow to initiate ***043MA

		return FALSE;
	} catch (Error e) {
		showError(e.errorString());
		delete dlg;
		return FALSE;
	}
}


//*******************************************************************
void on_radioOriginal_clicked(
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		dlg->mRegistrationMethod = ORIGINAL_3_POINT;
		dlg->mRegSegmentsUsed = ALL_POINTS;
		// also need to set ALL_POINTS radio button to active
#ifdef DEBUG
		printf("In on_radioOriginal_clicked() RegMethod = %d RegSegUsed = %d\n",
		dlg->mRegistrationMethod, dlg->mRegSegmentsUsed);
#endif
	}
}


//*******************************************************************
void on_radioTrimFixed_clicked(
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		dlg->mRegistrationMethod = TRIM_FIXED_PERCENT;
		//dlg->mRegSegmentsUsed = ALL_POINTS;
#ifdef DEBUG
		printf("In on_radioTrimFixed_clicked() RegMethod = %d RegSegUsed = %d\n",
		dlg->mRegistrationMethod, dlg->mRegSegmentsUsed);
#endif
	}
}

/**1.85 - removed
//*******************************************************************
void on_radioTrimOptimal_clicked(
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		dlg->mRegistrationMethod = TRIM_OPTIMAL;
		dlg->mRegSegmentsUsed = LEAD_TO_TIP_ONLY;
#ifdef DEBUG
		printf("In on_radioTrimOptimal_clicked() RegMethod = %d RegSegUsed = %d\n",
		dlg->mRegistrationMethod, dlg->mRegSegmentsUsed);
#endif
	}
}
*/
//*******************************************************************
void on_radioTrimOptimalTotal_clicked(
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		dlg->mRegistrationMethod = TRIM_OPTIMAL_TOTAL;
		dlg->mRegSegmentsUsed = ALL_POINTS;
#ifdef DEBUG
		printf("In on_radioTrimOptimal_clicked() RegMethod = %d RegSegUsed = %d\n",
		dlg->mRegistrationMethod, dlg->mRegSegmentsUsed);
#endif
	}
}

//*******************************************************************
void on_radioTrimOptimalTip_clicked(
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		dlg->mRegistrationMethod = TRIM_OPTIMAL_TIP;
		dlg->mRegSegmentsUsed = ALL_POINTS;
#ifdef DEBUG
		printf("In on_radioTrimOptimal_clicked() RegMethod = %d RegSegUsed = %d\n",
		dlg->mRegistrationMethod, dlg->mRegSegmentsUsed);
#endif
	}
}

//*******************************************************************
void on_radioTrimOptimalArea_clicked( //***1.85 - new area metric use in matching
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		dlg->mRegistrationMethod = TRIM_OPTIMAL_AREA;
		dlg->mRegSegmentsUsed = ALL_POINTS;
#ifdef DEBUG
		printf("In on_radioTrimOptimal_clicked() RegMethod = %d RegSegUsed = %d\n",
		dlg->mRegistrationMethod, dlg->mRegSegmentsUsed);
#endif
	}
}

//*******************************************************************
void on_radioTrimOptimalInOut_clicked(
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		dlg->mRegistrationMethod = TRIM_OPTIMAL_IN_OUT;
		dlg->mRegSegmentsUsed = ALL_POINTS;
#ifdef DEBUG
		printf("In on_radioTrimOptimal_clicked() RegMethod = %d RegSegUsed = %d\n",
		dlg->mRegistrationMethod, dlg->mRegSegmentsUsed);
#endif
	}
}

//*******************************************************************
void on_radioTrimOptimalInOutTip_clicked(
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		dlg->mRegistrationMethod = TRIM_OPTIMAL_IN_OUT_TIP;
		dlg->mRegSegmentsUsed = ALL_POINTS;
#ifdef DEBUG
		printf("In on_radioTrimOptimal_clicked() RegMethod = %d RegSegUsed = %d\n",
		dlg->mRegistrationMethod, dlg->mRegSegmentsUsed);
#endif
	}
}

//*******************************************************************
void on_radioAllPoints_clicked(
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		//dlg->mRegistrationMethod = TRIM_OPTIMAL;
		//dlg->mRegSegmentsUsed = ALL_POINTS; //***1.5 - replaced with line below

		//***1.5 - this button now has a different use, it changes the range of points
		// used in the final error computed.  This error is used ONLY in the final ranking
		// of results, NOT in the actual process of mapping of fins
		dlg->mUseFullFinError = true; //***1.5
	}
}

//*******************************************************************
void on_radioTrailingOnly_clicked(
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		//dlg->mRegistrationMethod = TRIM_OPTIMAL;
		//dlg->mRegSegmentsUsed = ALL_POINTS; //***1.5 - replaced with line below

		//***1.5 - this button now has a different use, it changes the range of points
		// used in the final error computed.  This error is used ONLY in the final ranking
		// of results, NOT in the actual process of mapping of fins
		dlg->mUseFullFinError = false; //***1.5
	}
}

//*******************************************************************
void on_radioLeadToTipOnly_clicked(
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		//dlg->mRegistrationMethod = TRIM_OPTIMAL;
		dlg->mRegSegmentsUsed = LEAD_TO_TIP_ONLY;
#ifdef DEBUG
		printf("In on_radioLeadToTipOnly_clicked() RegMethod = %d RegSegUsed = %d\n",
		dlg->mRegistrationMethod, dlg->mRegSegmentsUsed);
#endif
	}
}

//*******************************************************************
void on_radioLeadToNotchOnly_clicked(
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		//dlg->mRegistrationMethod = TRIM_OPTIMAL;
		dlg->mRegSegmentsUsed = LEAD_TO_NOTCH_ONLY;
#ifdef DEBUG
		printf("In on_radioLeadToNotchOnly_clicked() RegMethod = %d RegSegUsed = %d\n",
		dlg->mRegistrationMethod, dlg->mRegSegmentsUsed);
#endif
	}
}

//*******************************************************************
void on_radioLeadThenTrail_clicked(
	GtkObject *object,
	gpointer userData)
{
	MatchingDialog *dlg = (MatchingDialog *)userData;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(object)))
	{
		//dlg->mRegistrationMethod = TRIM_OPTIMAL;
		dlg->mRegSegmentsUsed = LEAD_THEN_TRAIL;
#ifdef DEBUG
		printf("In on_radioLeadThenTrail_clicked() RegMethod = %d RegSegUsed = %d\n",
		dlg->mRegistrationMethod, dlg->mRegSegmentsUsed);
#endif
	}
}
