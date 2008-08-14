//*******************************************************************
//   file: TraceWindow.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (9/29/2005)
//         -- code to allow user movement of feature points
//         -- drop-down selection of catalog category
//         -- major revision of trace phases and displayed buttons/messages
//
//	 mods: S. A. Hale (05/2007)
//			-- code to incorporate IntensityContours (AutoTrace)
//			-- General bug fixes
//
//*******************************************************************

#include <gdk/gdkkeysyms.h>
#include "../support.h"

#include "TraceWindow.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "/"
#endif

#include <ctime>   //***1.95
#include <fstream> //***1.95
#include <iomanip> //***1.95

#include <iostream> //***06PM for debugging

#pragma warning (disable : 4305 4309)

#include "../../pixmaps/fin.xpm"
#include "../../pixmaps/add_database.xpm"
#include "../../pixmaps/addpoint.xpm"
#include "../../pixmaps/addpoint_cursor.xbm"
#include "../../pixmaps/brightness_button.xpm"
#include "../../pixmaps/cancel.xpm"
#include "../../pixmaps/contrast_button.xpm"
#include "../../pixmaps/crop_button.xpm"
#include "../../pixmaps/despeckle.xpm"
#include "../../pixmaps/eraser.xpm"
#include "../../pixmaps/eraser_cursor.xbm"
#include "../../pixmaps/finger.xpm"
#include "../../pixmaps/Hflip.xpm"
//#include "../../pixmaps/grip.xpm"
#include "../../pixmaps/light_bulb.xpm"
#include "../../pixmaps/magnify.xpm"
#include "../../pixmaps/magnify_cursor.xbm"
#include "../../pixmaps/chopoutline_button.xpm"  //** 1.5 krd
#include "../../pixmaps/chopoutline_cursor.xbm"  //** 1.5 krd
#include "../../pixmaps/open_hand.xbm"
#include "../../pixmaps/autotrace_cursor.xbm"//103AT SAH
#include "../../pixmaps/autotrace.xpm"//103AT SAH
#include "../../pixmaps/pencil.xpm"
#include "../../pixmaps/redo_button.xpm"
#include "../../pixmaps/resize_button.xpm"
#include "../../pixmaps/rotate_button.xpm"
#include "../../pixmaps/save.xpm"
#include "../../pixmaps/smooth.xpm"
#include "../../pixmaps/undo_button.xpm"
#include "../../pixmaps/Vflip.xpm"
#include "../../pixmaps/ok.xpm"
#include "../../pixmaps/trace.xpm"
#include "../../pixmaps/view_points.xpm"
#include "../../pixmaps/unlock_trace.xpm" //***1.95 - JHS

#include "../image_processing/conversions.h"
#include "../DatabaseFin.h"
#include "../image_processing/edge_detect.h"
#include "../snake.h"
#include "../image_processing/transform.h"
#include "../feature.h"   //***006PD

#include "AlterBrightnessDialog.h"
#include "DeleteOutlineDialog.h"
#include "EnhanceContrastDialog.h"
#include "ErrorDialog.h"
#include "MatchingDialog.h"
#include "ResizeDialog.h"
#include "SaveFileChooserDialog.h" //***1.99
#include "../IntensityContour.h" //101AT
#include "../IntensityContourCyan.h" //103AT SAH

using namespace std;

extern Options *gOptions;

static const string ERROR_MSG_NO_CONTOUR = "Your fin must be traced before\n"
					   "it may be saved or added to the database.";
static const string ERROR_MSG_NO_IDCODE = "You must enter an ID Code before\n"
                                           "you can add a fin to the database.";
static const string DELETE_OUTLINE  = "Delete the current outline?";

static const int POINT_SIZE = 2;
static const int SPACE_BETWEEN_POINTS = 3;
static const int ERASER_BRUSH_SIZE = 9;		// krd 10/28/05
static const int MAX_ZOOM = 1600;
static const int MIN_ZOOM = 6; //***1.95 - minimum is now 6% of original size

//#define TIMING_ENABLED
#ifdef TIMING_ENABLED
static clock_t startTimerValue, endTimerValue; //***1.95
static double duration; //***1.95
extern ofstream timingOutFile; //***1.95
#endif

static int gNumReferences = 0;

int getNumTraceWindowReferences()
{
	return gNumReferences;
}

TraceWindow::TraceWindow(
		MainWindow *m,
		const string &fileName,
		const ColorImage *image,
		Database *db,
		Options *o
)
	: mMainWin(m),
	  mOriginalImage(new ColorImage(image)),
	  mNonZoomedImage(new ColorImage(image)),
	  mUndoImage(NULL),
	  mImage(new ColorImage(image)),
	  mImagefilename(fileName),		//***001DB
	  mDatabase(db),
	  mFin(NULL), //***1.4 - no fin loaded here
	  mContour(NULL),
	  mUndoContour(NULL),
	  mOutline(NULL), //***008OL
      mImageLocked(false),         //***006FC
	  mTraceLocked(false),         //***006FC
	  mFeaturePointsLocked(false), //***006FC
	  //mWindow(createTraceWindow(fileName)),
	  mQuestionDialog(NULL),
	  mTraceCursor(NULL),
	  mGC(NULL),
	  mMovingGC(NULL), //***051TW
	  mTraceFinalized(false),
	  mTraceSnapped(false), //***051TW
	  mCurTraceTool(TRACE_MAGNIFY),
	  mZoomScale(1.0),
	  mZoomRatio(100),
	  mZoomXOffset(0),
	  mZoomYOffset(0),
	  mNormScale(1.0),             //***006CN
	  mIgnoreConfigureEventCnt(0),
	  mMovePosition(0),
	  mMoveInit(false),
	  mSWWidthMax(-1),
	  mSWHeightMax(-1),
	  mOptions(o),
	  mXCropStart(0),
	  mYCropStart(0),
	  mXCropEnd(0),
	  mYCropEnd(0),
	  mXCropPrev(0),
	  mYCropPrev(0),
	  mXCropPrevLen(0),
	  mYCropPrevLen(0),
	  mRotateStartAngle(0.0f),
	  mRotateXStart(0),
	  mRotateYStart(0),
	  mRotateXCenter(0),
	  mRotateYCenter(0),
	  mRotateOriginalImage(NULL),
	  mLoadingFinNow(false), // ***1.4 - not loading a fin here
	  mSavedFinFilename(""), //***1.6
	  mIsFlipped(false), //***1.8 - initially false, unless image indicates otherwise
	  mLastMod(ImageMod::IMG_none), //***1.95
	  mFinWasLoaded(false) //***1.95
	  //mVirtualTop(0), //***1.96
	  //mVirtualLeft(0) //***1.96
{
	mWindow = createTraceWindow(fileName);

	// make sure that if a _wDarwinmods.png file was loaded
	// we pretend that the original was loaded and that the modifications
	// were applied -- this is required so that we do NOT have a missing
	// original file at some point down the line

	if (image->mBuiltFromMods)
	{
		cout << "LOADED: " << fileName << endl;
		cout << "  ORIG: " << mOriginalImage->mOriginalImageFilename << endl;

		mImageMods = mOriginalImage->mImageMods; // leave mLastMod as NONE -- cannot undo these loaded mods
		mIsFlipped = mOriginalImage->mImageMods.imageIsReversed();
		// should the mOriginalImage->mImageMods be cleared here???
		string temp = fileName.substr(0,fileName.rfind(PATH_SLASH)+1);
		mImagefilename = temp + mOriginalImage->mOriginalImageFilename;
		mOriginalImage->mOriginalImageFilename = "";
		mNonZoomedImage->mOriginalImageFilename = "";
		mImage->mOriginalImageFilename = "";
	}

	gNumReferences++;
}

/////////////////////////////////////////////////////////////////////////
//***1.4 - new constructor for loading previously traced fin
//
TraceWindow::TraceWindow(
		MainWindow *m,
		const string &fileName,
		DatabaseFin<ColorImage> *fin,
		Database *db,
		Options *o
)
	: mMainWin(m),
	  mUndoImage(NULL),
	  mImagefilename(fin->mImageFilename),
	  mDatabase(db),
	  mFin(fin), //***1.4 - JUST a pointer to loaded fin (MUST delete fin when done)
	  mContour(new Contour(fin->mFinOutline->getFloatContour())),
	  mUndoContour(NULL),
	  mOutline(new Outline(fin->mFinOutline)),
      mImageLocked(true),
	  mTraceLocked(true),
	  mFeaturePointsLocked(true),
	  mQuestionDialog(NULL),
	  mTraceCursor(NULL),
	  mGC(NULL),
	  mMovingGC(NULL),
	  mTraceFinalized(true),
	  mTraceSnapped(true),
	  mCurTraceTool(TRACE_MAGNIFY),
	  mZoomScale(1.0),
	  mZoomRatio(100),
	  mZoomXOffset(0),
	  mZoomYOffset(0),
	  mNormScale(1.0),
	  mIgnoreConfigureEventCnt(0), // must be 1 so problems with order of CG & drawable don't arise
	  mMovePosition(0),
	  mMoveInit(false),
	  mSWWidthMax(-1),
	  mSWHeightMax(-1),
	  mOptions(o),
	  mXCropStart(0),
	  mYCropStart(0),
	  mXCropEnd(0),
	  mYCropEnd(0),
	  mXCropPrev(0),
	  mYCropPrev(0),
	  mXCropPrevLen(0),
	  mYCropPrevLen(0),
	  mRotateStartAngle(0.0f),
	  mRotateXStart(0),
	  mRotateYStart(0),
	  mRotateXCenter(0),
	  mRotateYCenter(0),
	  mRotateOriginalImage(NULL),
	  mLoadingFinNow(true), // we are loading a fin here
	  mSavedFinFilename(fin->mFinFilename), //***1.8 - set it since fin file exists
	  mIsFlipped(false), //***1.8 - initially false, unless image indicates otherwise
	  mLastMod(ImageMod::IMG_none), //***1.95
	  mFinWasLoaded(true) //***1.95
	  //mVirtualTop(0), //***1.96
	  //mVirtualLeft(0) //***1.96
{
	if (NULL == fin->mModifiedFinImage) //***1.5 - if none exists this is an old fin file
	{
		// these three statements are the original code to set up images - pre version 1.5
		mImage = new ColorImage(fin->mImageFilename);
	  
		mOriginalImage = new ColorImage(mImage);
		mNonZoomedImage = new ColorImage(mImage);
	}
	else //***1.5 - this is a previously saved fin with (possibly) modified image
	{
		mImage = new ColorImage(fin->mModifiedFinImage);

		mImageMods = mFin->mImageMods; //***1.96a - copy image mod sequence
	  
		mOriginalImage = new ColorImage(mImage);
		mNonZoomedImage = new ColorImage(mImage);

		mNormScale = mImage->mNormScale; //***1.5
	}

	mIsFlipped = mImage->mImageMods.imageIsReversed(); //***1.8

	mWindow = createTraceWindow(fileName);
	setupForLoadedFin();
	gNumReferences++;
}


TraceWindow::~TraceWindow()
{
	if (NULL != mWindow)
		gtk_widget_destroy(mWindow);

	delete mImage;
	delete mNonZoomedImage;
	delete mOriginalImage;

	if (NULL != mTraceCursor)
		gdk_cursor_destroy(mTraceCursor);

	if (NULL != mGC)
		gdk_gc_unref(mGC);

	delete mContour;

	if (NULL != mQuestionDialog)
		gtk_widget_destroy(mQuestionDialog);

	delete mUndoContour;

	//delete mChain;    //***006PD removed

  //***008OL no need to delete the Outline *mOutline HERE
  // as it is being passed on to other areas of program
  // where it will eventually be deleted when merged with
  // the database or when discarded as the unknown fin - JHS

	//***1.0LK - I think the outline DOES need to be deleted here
	// a copy of it is made in the calls to DatabaseFin(...) and
	// so we are done with this original copy now

	delete mOutline; //***1.0LK - ????????

	//***1.65 - since OpenFileSelectionDialog does not delete fin after
	// opening a fin file, we MUST do it here
	if (NULL != mFin)
		delete mFin;

	delete mUndoImage;

	gNumReferences--;

	if (NULL == mMainWin) {//if there is not a main window to return to, then quit (e.g. opened a finz file)
		gtk_main_quit();
	}
}

/////////////////////////////////////////////////////////////////////
//
//***1.8 - new function to return ptr to list so it can be accessed/modified
//
ImageModList& TraceWindow::theImageMods()
{
	return mImageMods;
}

/////////////////////////////////////////////////////////////////////
//
void TraceWindow::show()
{

	gtk_widget_show(mWindow);

	updateCursor();
	updateGC();

	//***1.4 - call zoomUpdate here instead of from on_mDrawingArea_configure_event()
	// and reset flag to allow normal calls to zoomUpdate later when user changes
	// to mDrawingArea occur
	if (mLoadingFinNow)
	{
		mLoadingFinNow = false;
		zoomUpdate(true);
	}

	//***1.4 - force lock of features if loading previously saved fin trace
	if (mOutline)
		on_traceButtonFeaturePointsOK_clicked(NULL,(void *)this); //***1.4

}

//////////////////////////////////////////////////////////////////////////
// traceReset: used by DeleteContourDialog object to prepare for
//             redrawing the contour
//
void TraceWindow::traceReset()
{
	if (NULL != mContour) {
		delete mContour;
		mContour = NULL;
	}

	//***1.4 - new code to remove Outline, if loaded from fin file
	if (NULL != mOutline) {
		delete mOutline;
		mOutline = NULL;
	}

	//***1.95 - new code to remove DatabaseFin, if loaded from fin file
	// or if created for a previous Save or Add To Database operation
	// from which we have returned to setp 1 (Modifiy the image)
	//
	if (NULL != mFin) {
		delete mFin;
		mFin = NULL;
	}

	mTraceSnapped = false; //***051TW
	mTraceLocked = false; //***051TW
	mTraceFinalized = false;
	mNormScale = 1.0f; //***051TW

	this->refreshImage(); //***051TW
}

void TraceWindow::traceSave(const string &fileName)
{
	try {
		string
			id = gtk_entry_get_text(GTK_ENTRY(mEntryID)),
			name = gtk_entry_get_text(GTK_ENTRY(mEntryName)),
			date = gtk_entry_get_text(GTK_ENTRY(mEntryDate)),
			roll = gtk_entry_get_text(GTK_ENTRY(mEntryRoll)),
			location = gtk_entry_get_text(GTK_ENTRY(mEntryLocation)),
			damage = gtk_entry_get_text(GTK_ENTRY(mEntryDamage)),
			description = gtk_entry_get_text(GTK_ENTRY(mEntryDescription));

		DatabaseFin<ColorImage> newFin(
				mImagefilename,		//***001DB
				mOutline, //***008OL
				id,
				name,
				date,
				roll,
				location,
				damage,
				description);

		newFin.save(fileName);

	} catch (Error e) {
		showError(e.errorString());
	}
}

bool TraceWindow::pointInImageBounds(int x, int y)
{
	if (NULL == mImage)
		return false;

	if ((x >= 0) && (x < (int)mImage->getNumCols()) && (y >= 0) && (y < (int)mImage->getNumRows()))
		return true;

	return false;
}

void TraceWindow::updateCursor()
{
	GdkColor white = {0,0xFFFF,0xFFFF,0xFFFF};
	GdkColor black = {0,0x0000,0x0000,0x0000};

        GdkBitmap *bitmap, *mask;

	if (NULL != mTraceCursor)
		gdk_cursor_destroy(mTraceCursor);

	switch (mCurTraceTool) {

		case TRACE_AUTOTRACE: //103AT
			bitmap = gdk_bitmap_create_from_data(NULL,			//103AT SAH
						autotrace_cursor, autotrace_cursor_width, //103AT SAH
						autotrace_cursor_height);				//103AT SAH
			mask = gdk_bitmap_create_from_data(NULL,			//103AT SAH
						autotrace_mask, autotrace_cursor_width,	//103AT SAH
						autotrace_cursor_height);				//103AT SAH
			mTraceCursor = gdk_cursor_new_from_pixmap(			//103AT SAH
						bitmap, mask, &white, &black,   	//103AT SAH
						autotrace_xhot, autotrace_yhot);		//103AT SAH

			break;//103AT SAH

		case TRACE_PENCIL:
			mTraceCursor = gdk_cursor_new(GDK_PENCIL);
			break;

		case TRACE_ADD_POINT:
			bitmap = gdk_bitmap_create_from_data(NULL,
					addpoint_cursor, addpoint_cursor_width,
					addpoint_cursor_height);
			mask = gdk_bitmap_create_from_data(NULL,
					addpoint_mask, addpoint_cursor_width,
					addpoint_cursor_height);
			mTraceCursor = gdk_cursor_new_from_pixmap(
					bitmap, mask, &white, &black,
					addpoint_xhot, addpoint_yhot);
			break;

		case TRACE_MOVE_POINT:
		case TRACE_MOVE_FEATURE:
			mTraceCursor = gdk_cursor_new(GDK_HAND2);
			break;

		case TRACE_ERASER:
			bitmap = gdk_bitmap_create_from_data(NULL,
						eraser_cursor, eraser_cursor_width,
						eraser_cursor_height);
			mask = gdk_bitmap_create_from_data(NULL,
						eraser_mask, eraser_cursor_width,
						eraser_cursor_height);
			mTraceCursor = gdk_cursor_new_from_pixmap(
						bitmap, mask, &black, &white,
						eraser_xhot, eraser_yhot);
			break;

		case TRACE_CROP:
			mTraceCursor = gdk_cursor_new(GDK_SIZING);
			break;

		case TRACE_ROTATE:
			bitmap = gdk_bitmap_create_from_data(NULL,
						open_hand, open_hand_width,
						open_hand_height);
			mask = gdk_bitmap_create_from_data(NULL,
						open_hand_mask, open_hand_width,
						open_hand_height);
			mTraceCursor = gdk_cursor_new_from_pixmap(
						bitmap, mask, &black, &white,
						open_hand_xhot, open_hand_yhot);
			break;

		case TRACE_MAGNIFY:
			bitmap = gdk_bitmap_create_from_data(NULL,
						magnify_cursor, magnify_cursor_width,
						magnify_cursor_height);
			mask = gdk_bitmap_create_from_data(NULL,
						magnify_mask, magnify_cursor_width,
						magnify_cursor_height);
			mTraceCursor = gdk_cursor_new_from_pixmap(
						bitmap, mask, &black, &white,
						magnify_xhot, magnify_yhot);
			break;

                case TRACE_CHOPOUTLINE:    //*** 1.5 krd
                        bitmap = gdk_bitmap_create_from_data(NULL,
                                                chopoutline_cursor, chopoutline_cursor_width,
                                                chopoutline_cursor_height);
                        mask = gdk_bitmap_create_from_data(NULL,
                                                chopoutline_mask, chopoutline_cursor_width,
                                                chopoutline_cursor_height);
                        mTraceCursor = gdk_cursor_new_from_pixmap(
                                                bitmap, mask, &black, &white,
                                                chopoutline_xhot, chopoutline_yhot);
                        break;

		default:
			// Setting it temporarily to the pencil image....
			mTraceCursor = gdk_cursor_new(GDK_PENCIL);
			break;
	}

	// I'm paranoid, what can I say?
	if (NULL != mTraceCursor && NULL != mDrawingArea)
		gdk_window_set_cursor(mDrawingArea->window, mTraceCursor);
}

void TraceWindow::updateGC()
{
	if (NULL == mDrawingArea)
		return;

	// traced Contour color & graphics context

	if (NULL == mGC)
		mGC = gdk_gc_new(mDrawingArea->window);

	updateGCColor();

	// graphics context & color for feature points being moved

	if (NULL == mMovingGC)
		mMovingGC = gdk_gc_new(mDrawingArea->window);

	double movingColor[] = {1.0, 0.0, 0.0, 0.0};
	updateGCColor(mMovingGC, movingColor);

}

void TraceWindow::updateGCColor()
{
	if (NULL == mGC)
		return;

	GdkColormap *colormap;
	GdkColor gdkColor;

	gdkColor.red = (gushort)(0xFFFFF * mOptions->mCurrentColor[0]);
	gdkColor.green = (gushort)(0xFFFFF * mOptions->mCurrentColor[1]);
	gdkColor.blue = (gushort)(0xFFFFF * mOptions->mCurrentColor[2]);

	colormap = gdk_colormap_get_system();
	gdk_color_alloc(colormap, &gdkColor);

	gdk_gc_set_foreground(mGC, &gdkColor);
}


//*******************************************************************
//
// void TraceWindow::updateGCColor(GdkGC *gc, double color[4])
//
//    Member Function.
//
void TraceWindow::updateGCColor(GdkGC *gc, double color[4])
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
//***1.96 - two new functions to manage virtual cropping of image prior
// to and following a trace

//*******************************************************************
void TraceWindow::createVirtualImageFromNonZoomed()
{
}

//*******************************************************************
void TraceWindow::restoreNonZoomedFromVirtualImage()
{
}

//*******************************************************************
void TraceWindow::getViewedImageBoundsNonZoomed(int &left, int &top,
								   int &right, int &bottom)
{
	int 
		width = mImage->getNumCols(), 
		height = mImage->getNumRows();

	left = 0;
	top = 0;
	right = left + width - 1; //SAH--BUG FIX , to ;
	bottom = top + height - 1;

	GtkAdjustment 
		*adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(mScrolledWindow));

	if (adj->page_size < height)
	{
		height = adj->page_size;
		top = adj->value;
		if (top < 0)
			top = 0;
		bottom = top + height - 1;
		if (bottom >= mImage->getNumRows())
			bottom = mImage->getNumRows() - 1;
	}
		
	adj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(mScrolledWindow));
		
	if (adj->page_size < width)
	{
		width = adj->page_size;
		left = adj->value;
		if (left < 0)
			left = 0;
		right = left + width - 1;
		if (right >= mImage->getNumCols())
			right = mImage->getNumCols() - 1;
	}

	// since we are in a scrollable window, the offsets are 0,0 and the
	// drawable width & height are the same as the image rows & cols

	this->zoomMapPointsToOriginal(left, top);
	this->zoomMapPointsToOriginal(right, bottom);
}

//*******************************************************************
// Point clicked for AutoTrace -- 103AT SAH
void TraceWindow::traceAddAutoTracePoint(int x, int y, bool bolShift)//AT103 SAH
{

	if (!pointInImageBounds(x, y))
		return;

	if (NULL == mContour)
		mContour = new Contour();

	zoomMapPointsToOriginal(x, y);

	mContour->addPoint(x, y);

	int zoomedPointSize = zoomPointSize();

	zoomMapPointsToZoomed(x, y);

	gdk_draw_rectangle(
		mDrawingArea->window,
		mGC,
		TRUE,
		x - zoomedPointSize / 2 + mZoomXOffset,
		y - zoomedPointSize / 2 + mZoomYOffset,
		zoomedPointSize,
		zoomedPointSize);

	if (mContour->length() > 1) {

		//Check that the first point is further left than the second (otherwise we crash)
		//that is that [0].x < [1].x
		if ((*mContour)[0].x>=(*mContour)[1].x) {
			ErrorDialog *errDiag = new ErrorDialog("Please click the start of the leading edge first and the end of the trailing edge second.\n\nNote: the dolphin must swim to your left.");//103AT
			errDiag->show();//103AT
			
			//remove current marks
			traceReset();
			return;
		}

#ifdef TIMING_ENABLED
		startTimerValue = clock(); //***1.95
#endif
		//run trace
		g_print("PLEASE WAIT:\n   Automatically detecting rough fin outline ....");

		//***1.96 - figure out how to pass the image the user sees ONLY
		int left, top, right, bottom;
		getViewedImageBoundsNonZoomed(left,top,right,bottom);

		//Perform intensity trace (SCOTT) (Aug/2005) Code: 101AT
		Contour *trace;
		
		if (!bolShift)
			//trace = new IntensityContour(mNonZoomedImage,mContour); //101AT --Changed IntensityContour declaration to Contour
			trace = new IntensityContour(
				mNonZoomedImage,
				mContour,
				left,top,right,bottom); //***1.96 added window bounds 

			/* test trimAndReorder
			// reverse and trim off 20 points
			trace->trimAndReorder((*trace)[trace->length()-10],(*trace)[10]);
			printf("%d %d %d %d\n",(*trace)[0].x,(*trace)[0].y,
				(*trace)[trace->length()-1].x,(*trace)[trace->length()-1].y);
			// trim off 20 more points
			trace->trimAndReorder((*trace)[10],(*trace)[trace->length()-10]);
			printf("%d %d %d %d\n",(*trace)[0].x,(*trace)[0].y,
				(*trace)[trace->length()-1].x,(*trace)[trace->length()-1].y);
			// reverse again trim off 20 more points
			trace->trimAndReorder((*trace)[trace->length()-10],(*trace)[10]);
			printf("%d %d %d %d\n",(*trace)[0].x,(*trace)[0].y,
				(*trace)[trace->length()-1].x,(*trace)[trace->length()-1].y);
			*/

		if (!bolShift && trace->length() > 100 ) {//101AT -- changed min to 100 pt contour - JHS
			g_print("\n   Using edge detection and active contours to refine outline placement ....\n");
			delete mContour; //***1.96 - must free memory before re-assigning pointer (JHS)
			mContour = trace;//101AT
			traceSnapToFin(false, left, top, right, bottom);//101AT
			refreshImage();//101AT
		} else {//101AT
			g_print("\n   Trying Cyan Intensity AutoTrace ...");
			//102AT Add hooks for cyan intensity trace
			if (!bolShift)	
				delete (IntensityContour*)trace;//102AT //mod add if 103AT
			//trace = new IntensityContourCyan(mNonZoomedImage,mContour);//, mContour);//102AT
			trace = new IntensityContourCyan(
				mNonZoomedImage,
				mContour,
				left,top,right,bottom); //***1.96 added window bounds 

			if (trace->length() > 100 ) {//102AT
				g_print("\n   Using edge detection and active contours to refine outline placement\n   (with cyan intensity image) ....\n");
				delete mContour; //***1.96 - must free memory before re-assigning pointer (JHS)
				mContour = trace;//102AT
				traceSnapToFin(true, left, top, right, bottom);//102AT
				refreshImage();//102AT
			} else {
				delete (IntensityContourCyan*)trace; //***1.96 - must free memory since autotrace failed (JHS)
				ErrorDialog *errDiag = new ErrorDialog("Auto trace could not determine the fin outline.\n\nPlease trace outline by hand.");//101AT
				errDiag->show();//101AT

				//remove contour
				traceReset();

				//select pencil
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mRadioButtonPencil),TRUE);
				mCurTraceTool = TRACE_PENCIL;
				updateCursor();

				// set status label
				gtk_label_set_text(GTK_LABEL(mStatusLabel),
					_("Please hand trace the fin outline below."));		

				return;
			}//102AT

		}
		
		//successful trace
		
		//select eraser
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mRadioButtonEraser),TRUE);
		mCurTraceTool = TRACE_ERASER;
		updateCursor();

		// set status label
		gtk_label_set_text(GTK_LABEL(mStatusLabel),
				_("Make any Needed Corrections to Outline Trace using Tools Below."));		

	}//102AT
}


// Point added to end of contour during normal trace of outline
void TraceWindow::traceAddNormalPoint(int x, int y)
{

	if (!pointInImageBounds(x, y))
		return;

	if (NULL == mContour)
		mContour = new Contour();

	zoomMapPointsToOriginal(x, y);

	mContour->addPoint(x, y);

	int zoomedPointSize = zoomPointSize();

	zoomMapPointsToZoomed(x, y);

	gdk_draw_rectangle(
		mDrawingArea->window,
		mGC,
		TRUE,
		x - zoomedPointSize / 2 + mZoomXOffset,
		y - zoomedPointSize / 2 + mZoomYOffset,
		zoomedPointSize,
		zoomedPointSize);
}

// Point added to middle of contour after trace has been finalized
void TraceWindow::traceAddExtraPoint(int x, int y)
{
	if (NULL == mContour)
		return;

	if (!pointInImageBounds(x, y))
		return;

	zoomMapPointsToOriginal(x, y);

	this->addUndo(mContour);

        // use MovePoint code to display newly added point
	mMoveInit = true;
	mMovePosition = mContour->addPointInOrder(x, y);
	traceMovePointDisplay();
}

void TraceWindow::traceChopOutline(int x, int y)
{
        zoomMapPointsToOriginal(x, y);
        int start, stop;

        if (NULL == mContour) {
                mMovePosition = -1;
                return;
        }

        mChopInit = true;

        this->addUndo(mContour);

        unsigned numPoints = mContour->length();
        if (!mContour->findPositionOfClosestPoint(x, y, mChopPosition))
                mChopPosition = -1;
        else
        {

                if ((numPoints - mChopPosition) > mChopPosition){
                        mChopLead = 1;
                        start = 0;
                        stop = mChopPosition;
                }
                else {
                        mChopLead = 0;
                        start = mChopPosition;
                        stop = numPoints;
                }
        }

        // what if mChopPosition = -1?

        int zoomedPointSize = zoomPointSize();


        for (unsigned i = start; i < stop; i++) {
                int zoomedX = (*mContour)[i].x;
                int zoomedY = (*mContour)[i].y;

                zoomMapPointsToZoomed(zoomedX, zoomedY);

                gdk_draw_rectangle(
                                mDrawingArea->window,
                                mMovingGC,
                                TRUE,
                                zoomedX - zoomedPointSize / 2 + mZoomXOffset,
                                zoomedY - zoomedPointSize / 2 + mZoomYOffset,
                                zoomedPointSize,
                                zoomedPointSize);
    }
}

void TraceWindow::traceChopOutlineFinal()
{
        if (NULL == mContour || -1 == mChopPosition)
                return;

        int start, stop;

        if (mChopLead){
                start = 0;
                stop = mChopPosition;
        }
        else {
                start = mChopPosition;
                stop = mContour->length();
        }
        for (unsigned i = start; i < stop; i++){
            //    cout << "removing " << i << endl;
            mContour->removePoint(start);
        }

        if (NULL == mDrawingArea)
                return;

        refreshImage();
}

void TraceWindow::traceErasePoint(int x, int y)
{
	if(mContour == NULL) return;

	this->addUndo(mContour);

	int
		numRows = (int)mNonZoomedImage->getNumRows(),
		numCols = (int)mNonZoomedImage->getNumCols();

	zoomMapPointsToOriginal(x, y);

	if (x >= numCols || y >= numRows)
		return;

	int
		row_start, col_start,
		row_end, col_end,
		offset;

	offset = (int) round(ERASER_BRUSH_SIZE/ mZoomScale / (float)2);  // krd 10/28/05

	row_start = y - offset;
	col_start = x - offset;

	if (row_start < 0) row_start = 0;
	if (col_start < 0) col_start = 0;

	row_end = y + offset;
	col_end = x + offset;

	if (row_end >= (int)numRows) row_end = numRows - 1;
	if (col_end >= (int)numCols) col_end = numCols - 1;

	for (int r = row_start; r <= row_end; r++)
		for (int c = col_start; c <= col_end; c++)
			mContour->removePoint(c, r);

	//***1.4 - remove empty Contour so other functions do not have to check for same
	if (0 == mContour->length())
	{
		delete mContour;
		mContour = NULL;
	}

	if (NULL == mDrawingArea)
		return;

	refreshImage();
}

void TraceWindow::traceMovePointInit(int x, int y)
{
	zoomMapPointsToOriginal(x, y);

	if (NULL == mContour) {
		mMovePosition = -1;
		return;
	}

	mMoveInit = true;

	this->addUndo(mContour);

	if (!mContour->findPositionOfClosestPoint(x, y, mMovePosition))
		mMovePosition = -1;
	else
		traceMovePointDisplay();
}


void TraceWindow::traceMovePointUpdate(int x, int y)
{
	if (NULL == mContour || -1 == mMovePosition)
		return;

	zoomMapPointsToOriginal(x, y);
	ensurePointInBounds(x, y);

	(*mContour)[mMovePosition].x = x;
	(*mContour)[mMovePosition].y = y;

	traceMovePointDisplay();
}



void TraceWindow::traceMovePointFinalize(int x, int y)
{
	if (NULL == mContour || -1 == mMovePosition)
		return;

	zoomMapPointsToOriginal(x, y);
	ensurePointInBounds(x, y);

	(*mContour)[mMovePosition].x = x;
	(*mContour)[mMovePosition].y = y;

	refreshImage();
}

void TraceWindow::traceMovePointDisplay()
{
	if (NULL == mContour || -1 == mMovePosition)
		return;

	int contourLength = mContour->length();
	int nextPosition, previousPosition;

	if (0 == mMovePosition) {
		nextPosition = 1;
		previousPosition = 1;          // if mMovePosition is endpoint make same
	} else if ((contourLength - 1) == mMovePosition) {
		nextPosition = mMovePosition - 1;   // if mMovePosition is endpoint make same
		previousPosition = mMovePosition - 1;
	} else {
		nextPosition = mMovePosition + 1;
		previousPosition = mMovePosition - 1;
	}

	int
		x0 = (*mContour)[previousPosition].x,
		x1 = (*mContour)[mMovePosition].x,
		y0 = (*mContour)[previousPosition].y,
		y1 = (*mContour)[mMovePosition].y;

	int
		xLeft, xRight, yTop, yBot;

	static int
		lastXLeft, lastXRight, lastYTop, lastYBot;

	int halfPointSize = zoomPointSize() / 2 + 1;

	if (x0 < x1) {
		xLeft = x0;
		xRight = x1;
	} else {
		xLeft = x1;
		xRight = x0;
	}

	if (y0 < y1) {
		yTop = y0;
		yBot = y1;
	} else {
		yTop = y1;
		yBot = y0;
	}

	int
		x2 = (*mContour)[nextPosition].x,
		y2 = (*mContour)[nextPosition].y;

	if (x2 < xLeft)
		xLeft = x2;
	else if (x2 > xRight)
		xRight = x2;

	if (y2 < yTop)
		yTop = y2;
	else if (y2 > yBot)
		yBot = y2;

	xLeft -= halfPointSize;
	xRight += halfPointSize;
	yBot += halfPointSize;
	yTop -= halfPointSize;

	if (xLeft < 0)
		xLeft = 0;
	if (yTop < 0)
		yTop = 0;
	if (xRight >= (int) mNonZoomedImage->getNumCols())
		xRight = mNonZoomedImage->getNumCols() - 1;
	if (yBot >= (int) mNonZoomedImage->getNumRows())
		yBot = mNonZoomedImage->getNumRows() - 1;

	zoomMapPointsToZoomed(xLeft, yTop);
	zoomMapPointsToZoomed(xRight, yBot);

	static bool firstRun = true;

	if (mMoveInit) {
		firstRun = true;
		mMoveInit = false;
	}

	if (firstRun)
		firstRun = false;
	else {
		if (lastXLeft < xLeft)
			xLeft = lastXLeft;

		if (lastXRight > xRight)
			xRight = lastXRight;

		if (lastYTop < yTop)
			yTop = lastYTop;

		if (lastYBot > yBot)
			yBot = lastYBot;
	}

	lastXLeft = xLeft;
	lastXRight = xRight;
	lastYTop = yTop;
	lastYBot = yBot;

	gdk_draw_rgb_image(
		mDrawingArea->window,
		mDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
		xLeft + mZoomXOffset, yTop + mZoomYOffset,
		xRight - xLeft,
		yBot - yTop,
		GDK_RGB_DITHER_NONE,
		(guchar*)(mImage->getData() + yTop * mImage->getNumCols() + xLeft),
		mImage->getNumCols() * mImage->bytesPerPixel());

	int zoomedPointSize = zoomPointSize();

	unsigned numPoints = mContour->length();

	for (unsigned i = 0; i < numPoints; i++) {
		int zoomedX = (*mContour)[i].x;
		int zoomedY = (*mContour)[i].y;

		zoomMapPointsToZoomed(zoomedX, zoomedY);

		if (zoomedX >= xLeft && zoomedX <= xRight && zoomedY >= yTop && zoomedY <= yBot)
			gdk_draw_rectangle(
				mDrawingArea->window,
				mGC,
				TRUE,
				zoomedX - zoomedPointSize / 2 + mZoomXOffset,
				zoomedY - zoomedPointSize / 2 + mZoomYOffset,
				zoomedPointSize,
				zoomedPointSize);
	}

	zoomMapPointsToZoomed(x0, y0);
	zoomMapPointsToZoomed(x1, y1);

	x0 += mZoomXOffset;
	y0 += mZoomYOffset;
	y1 += mZoomYOffset;
	x1 += mZoomXOffset;

	gdk_draw_line(
			mDrawingArea->window,
			mGC,
			x0,
			y0,
			x1,
			y1);

	if (nextPosition != previousPosition){  // krd - endpoint
		zoomMapPointsToZoomed(x2, y2);

		x2 += mZoomXOffset;
		y2 += mZoomYOffset;

		gdk_draw_line(
			mDrawingArea->window,
			mGC,
			x1,
			y1,
			x2,
			y2);
	}
}


//*******************************************************************
//
// void TraceWindow::traceMoveFeaturePointInit(int x, int y)
//
//    Finds identity of closest feature point and initiates movement
//    of same.
//
void TraceWindow::traceMoveFeaturePointInit(int x, int y)
{
	zoomMapPointsToOriginal(x, y);

	// bale out if feature points have not already been identified

	if ((NULL == mContour) || (NULL == mOutline) || (! mTraceFinalized))
		return;

	mMoveInit = true;

	point_t p;
	p.x = x * mNormScale;
	p.y = y * mNormScale;

	mMoveFeature = mOutline->findClosestFeaturePoint(p);

	if (NO_FEATURE == mMoveFeature)
		return;
	
	//***054 - indicate which point is being moved

	char *temp; // reference to label string
	gtk_label_get(GTK_LABEL(mStatusLabel),&temp);
	mSavedLabel = temp; // now we have a copy of the label string

	switch (mMoveFeature)
	{
	case TIP : 
		gtk_label_set_text(GTK_LABEL(mStatusLabel),
				_("Moving TIP -- Drag into position and release mouse button."));
		break;
	case NOTCH :
		gtk_label_set_text(GTK_LABEL(mStatusLabel),
				_("Moving NOTCH -- Drag into position and release mouse button."));
		break;
	case POINT_OF_INFLECTION :
		gtk_label_set_text(GTK_LABEL(mStatusLabel),
				_("Moving END OF OUTLINE -- Drag into position and release mouse button."));
		break;
	case LE_BEGIN :
		gtk_label_set_text(GTK_LABEL(mStatusLabel),
				_("Moving BEGINNING OF OUTLINE -- Drag into position and release mouse button."));
		break;
	case LE_END :
		//***1.8 - no longer show or move LE_END -- force it to TIP
		mMoveFeature = TIP;
		gtk_label_set_text(GTK_LABEL(mStatusLabel),
				_("Moving TIP -- Drag into position and release mouse button."));
		//gtk_label_set_text(GTK_LABEL(mStatusLabel),
		//		_("Moving END OF LEADING EDGE -- Drag into position and release mouse button."));
		break;
	default :
		gtk_label_set_text(GTK_LABEL(mStatusLabel),
				_("Cannot determine selected feature. Try Again!"));		
	}

	mMovePosition = mOutline->getFeaturePoint(mMoveFeature);

	p.x = (int)round((*mContour)[mMovePosition].x / mNormScale);
	p.y = (int)round((*mContour)[mMovePosition].y / mNormScale);

	
	traceMoveFeaturePointDisplay(p.x,p.y);
}


//*******************************************************************
//
// void TraceWindow::traceMoveFeaturePointFinalize(int x, int y)
//
//
//
void TraceWindow::traceMoveFeaturePointFinalize(int x, int y)
{
	if (NULL == mContour || NO_FEATURE == mMovePosition)
		return;

	// set new location of feature
	mOutline->setFeaturePoint(mMoveFeature,mMovePosition);

	// reset message label
	gtk_label_set_text(GTK_LABEL(mStatusLabel),_(mSavedLabel.c_str()));

	refreshImage();
}


//*******************************************************************
//
// void TraceWindow::traceMoveFeaturePointUpdate(int x, int y)
//
//
//
void TraceWindow::traceMoveFeaturePointUpdate(int x, int y)
{
	if (NULL == mContour || NO_FEATURE == mMoveFeature)
		return;

	zoomMapPointsToOriginal(x, y);
	ensurePointInBounds(x, y);

	x = (int)round(x * mNormScale);
	y = (int)round(y * mNormScale);

	// find location of closest contour point

	int posit;

	if (!mContour->findPositionOfClosestPoint(x , y, posit))
		return;

	// enforce constraints on feature movement so their order cannot be changed

	switch (mMoveFeature)
	{
	case LE_BEGIN :
		if ((0 < posit) && (posit < mOutline->getFeaturePoint(mMoveFeature + 1)))
			mMovePosition = posit;
		break;
	//***1.8 - moving of LE_END no longer supported
	//case LE_END :
	//***1.8 - moving of TIP now constrained by LE_BEGIN and NOTCH
	case TIP :
		if ((mOutline->getFeaturePoint(LE_BEGIN) < posit) &&
		    (posit < mOutline->getFeaturePoint(mMoveFeature + 1)))
			mMovePosition = posit;
		break;
	case NOTCH :
		if ((mOutline->getFeaturePoint(mMoveFeature - 1) < posit) &&
		    (posit < mOutline->getFeaturePoint(mMoveFeature + 1)))
			mMovePosition = posit;
		break;
	case POINT_OF_INFLECTION :
		if ((mOutline->getFeaturePoint(mMoveFeature - 1) < posit) &&
		    (posit < (mOutline->length() - 1)))
			mMovePosition = posit;
		break;
	}

	x = (int)round((*mContour)[mMovePosition].x / mNormScale);
	y = (int)round((*mContour)[mMovePosition].y / mNormScale);

	traceMoveFeaturePointDisplay(x,y);
}

//*******************************************************************
//
// void TraceWindow::traceMoveFeaturePointDisplay(int xC, int yC)
//
//    Handles display of the feature point at each point as it is being
//    dragged to new location.
//
void TraceWindow::traceMoveFeaturePointDisplay(int xC, int yC)
{
	if (NULL == mContour || NO_FEATURE == mMoveFeature)
		return;

	int contourLength = mContour->length();

	int
		xLeft, xRight, yTop, yBot;

	static int
		lastXLeft, lastXRight, lastYTop, lastYBot;

	int highlightPointSize = 4 * POINT_SIZE;

	if (mZoomRatio != 100)
		highlightPointSize = 4 * zoomPointSize();

	int halfHighPtSize = highlightPointSize / 2;

	zoomMapPointsToZoomed(xC, yC);

	xLeft = xC - halfHighPtSize;
	xRight = xC + halfHighPtSize;
	yBot = yC + halfHighPtSize;
	yTop = yC - halfHighPtSize;

	/*
	//***055TW - removed code to prevent artifacts in images with
	// mZoomRatio less than 100% -- these bounds checks are not
	// needed ???? -- in any case they must be done in zoomed image
	// coordinates
	if (xLeft < 0)
		xLeft = 0;
	if (yTop < 0)
		yTop = 0;
	if (xRight >= (int) mNonZoomedImage->getNumCols())
		xRight = mNonZoomedImage->getNumCols() - 1;
	if (yBot >= (int) mNonZoomedImage->getNumRows())
		yBot = mNonZoomedImage->getNumRows() - 1;

	zoomMapPointsToZoomed(xLeft, yTop);
	zoomMapPointsToZoomed(xRight, yBot);
    */

	// save bounds of this box for next round, even though we may widen
	// the box a bit below to cover last box and this one
	int 
		saveXLeft(xLeft), saveXRight(xRight), 
		saveYTop(yTop), saveYBot(yBot);

	if (mMoveInit) 
	{
		mMoveInit = false;
	}
	else 
	{
		if (lastXLeft < xLeft)
			xLeft = lastXLeft;

		if (lastXRight > xRight)
			xRight = lastXRight;

		if (lastYTop < yTop)
			yTop = lastYTop;

		if (lastYBot > yBot)
			yBot = lastYBot;
	}

	lastXLeft = saveXLeft;
	lastXRight = saveXRight;
	lastYTop = saveYTop;
	lastYBot = saveYBot;

	// redraw image within tiny window 

	gdk_draw_rgb_image(
		mDrawingArea->window,
		mDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
		xLeft + mZoomXOffset, yTop + mZoomYOffset,
		xRight - xLeft,
		yBot - yTop,
		GDK_RGB_DITHER_NONE,
		(guchar*)(mImage->getData() + yTop * mImage->getNumCols() + xLeft),
		mImage->getNumCols() * mImage->bytesPerPixel());

	// redraw portion of contour within tiny window

	int zoomedPointSize = zoomPointSize();

	unsigned numPoints = mContour->length();

	float nscale = 1.0/mNormScale;

	for (unsigned i = 0; i < numPoints; i++) {
		int zoomedX = nscale*(*mContour)[i].x;
		int zoomedY = nscale*(*mContour)[i].y;

		zoomMapPointsToZoomed(zoomedX, zoomedY);

		if ((zoomedX >= xLeft) && (zoomedX <= xRight) && (zoomedY >= yTop) && (zoomedY <= yBot))
			gdk_draw_rectangle(
				mDrawingArea->window,
				mGC,
				TRUE,
				zoomedX - zoomedPointSize / 2 + mZoomXOffset,
				zoomedY - zoomedPointSize / 2 + mZoomYOffset,
				zoomedPointSize,
				zoomedPointSize);
	}

	// redraw two adjacent features if they are inside the box

	if ((LE_BEGIN < mMoveFeature) && (mMoveFeature - 1 != LE_END)) //***1.8 not LE_END
	{
		point_t p = mOutline->getFeaturePointCoords(mMoveFeature - 1);
		int xC = (int) round(nscale*p.x);
		int yC = (int) round(nscale*p.y);
		if (mZoomRatio != 100)
			zoomMapPointsToZoomed(xC, yC);
		gdk_draw_rectangle(
			mDrawingArea->window,
			mGC,
			TRUE,
			xC - halfHighPtSize + mZoomXOffset,
			yC - halfHighPtSize + mZoomYOffset,
			highlightPointSize,
			highlightPointSize);
	}

	if ((mMoveFeature < POINT_OF_INFLECTION) && (mMoveFeature + 1 != LE_END)) //***1.8 not LE_END
	{
		point_t p = mOutline->getFeaturePointCoords(mMoveFeature + 1);
		int xC = (int) round(nscale*p.x);
		int yC = (int) round(nscale*p.y);
		if (mZoomRatio != 100)
			zoomMapPointsToZoomed(xC, yC);
		gdk_draw_rectangle(
			mDrawingArea->window,
			mGC,
			TRUE,
			xC - halfHighPtSize + mZoomXOffset,
			yC - halfHighPtSize + mZoomYOffset,
			highlightPointSize,
			highlightPointSize);
	}
		
	// Lastly, draw feature point in current (dragged to) location

	gdk_draw_rectangle(
		mDrawingArea->window,
		mMovingGC,
		TRUE,
		xC - halfHighPtSize + mZoomXOffset,
		yC - halfHighPtSize + mZoomYOffset,
		highlightPointSize,
		highlightPointSize);


}



void TraceWindow::ensurePointInBounds(int &x, int &y)
{
	if (NULL == mNonZoomedImage)
		return;

	if (x >= (int)mNonZoomedImage->getNumCols())
		x = (int)mNonZoomedImage->getNumCols() - 1;
	else if (x < 0)
		x = 0;
	if (y >= (int)mNonZoomedImage->getNumRows())
		y = (int)mNonZoomedImage->getNumRows() - 1;
	else if (y < 0)
		y = 0;
}

void TraceWindow::ensurePointInZoomedBounds(int &x, int &y)
{
	if (NULL == mImage)
		return;

	if (x >= (int)mImage->getNumCols())
		x = (int)mImage->getNumCols() - 1;
	else if (x < 0)
		x = 0;
	if (y >= (int)mImage->getNumRows())
		y = (int)mImage->getNumRows() - 1;
	else if (y < 0)
		y = 0;
}

/***1.1 - replaced with new multiscale version
/////////////////////////////////////////////////////////////////////
// traceSnapToFin: called to perform the active contour based fit of
//    trace to fin outline and remove knots.  This is called after
//    the initial trace but before user has oportunity to clean
//    up the trace, add/remove points, etc
//
void TraceWindow::traceSnapToFin()
{
	if ((NULL == mContour) || (mTraceLocked)) //***006FC
		return;

	GrayImage *edgeMagImage;
	GrayImage *temp = convColorToGray(mNonZoomedImage);
	GrayImage *edgeImage = edgeDetect_canny(
			temp,
			&edgeMagImage,
			mOptions->mGaussianStdDev,
			mOptions->mLowThreshold,
			mOptions->mHighThreshold);
	delete temp;

	float energyWeights[] = {
		mOptions->mContinuityWeight,
		mOptions->mLinearityWeight,
		mOptions->mEdgeWeight
	};

	double spacing; //***005CM declaration moved ouside if() below
	// contour is initially spaced with larger of 1) space for 200 points or
	// 2) at three pixels (200 points limits hi res pics from having many points)
	if (mContour->length() > 2) {

		spacing = mContour->totalDistanceAlongContour()/200.0;
		if (spacing < SPACE_BETWEEN_POINTS)
			spacing = SPACE_BETWEEN_POINTS;

		Contour *evenContour = mContour->evenlySpaceContourPoints(spacing);
		delete mContour;
		mContour = evenContour;
	}

	for (int i = 0; i < mOptions->mMaxIterations; i++) {

		moveContour(mContour, edgeMagImage, energyWeights);
		if (i%5==0)
			this->refreshImage();
	}

	// features such as glare spots may cause outline points to bunch and wrap
	// during active contour process
	mContour->removeKnots(spacing); //***005CM

	mTraceSnapped = true; //***051TW

	delete edgeImage;
	delete edgeMagImage;
}
*/


//***1.96 - modified with virtual cropping bounds
//***1.1 - new multiscale version
/////////////////////////////////////////////////////////////////////
// traceSnapToFin: called to perform the active contour based fit of
//    trace to fin outline and remove knots.  This is called after
//    the initial trace but before user has oportunity to clean
//    up the trace, add/remove points, etc
//
//  Modified for multiscale active contour processing 01/23/06 krd
//
void TraceWindow::traceSnapToFin(bool useCyan, int left, int top, int right, int bottom)
{
	if ((NULL == mContour) || (mTraceLocked)) //***006FC
		return;
	
	float energyWeights[] = {
		mOptions->mContinuityWeight,
		mOptions->mLinearityWeight,
		mOptions->mEdgeWeight
	};

    // full size and currently viewed scale edgeMagImage
	GrayImage *EdgeMagImage, *smallEdgeMagImage; 
	GrayImage *temp;
	if (useCyan)
		temp = convColorToCyan(mNonZoomedImage); //***1.96 - copy to cyan
	else
		temp = convColorToGray(mNonZoomedImage); //***1.96 - copy to grayscale
	GrayImage *temp2 = crop(temp,left,top,right,bottom); //***1.96 - then crop
	GrayImage *EdgeImage = edgeDetect_canny(
			//temp,
			temp2, //***1.96
			&EdgeMagImage,
			mOptions->mGaussianStdDev,
			mOptions->mLowThreshold,
			mOptions->mHighThreshold);
	delete temp2;        //***1.96
	delete EdgeImage; // only need EdgeMagImage from this

	//***1.96 - copy edgeMagImage into area of entire temp image bounded by 
	//          left, top, right, bottom
	for (int r = 0; r < EdgeMagImage->mRows; r++)
		for (int c = 0; c < EdgeMagImage->mCols; c++)
		{
			// set up area within *temp as the real EdgeMagImage
			(*temp).mData[(top+r)*temp->mCols+(left+c)] = (*EdgeMagImage).mData[r*EdgeMagImage->mCols+c];
		}
	delete EdgeMagImage; //***1.96
	EdgeMagImage = temp; //***1.96
    
	//EdgeMagImage->save("EdgeMagImg.png");

    // create initial evenly spaced contour scaled to zoomed image
	double spacing; //***005CM declaration moved ouside if() below
	// contour is initially spaced with larger of 1) space for 200 points or
	// 2) at three pixels (200 points limits hi res pics from having many points)

	//scale mContour to current scale of analysis (image) first time thru
	Contour *smallContour; // current scale contour
	if (mContour->length() > 2) {

		spacing = mContour->totalDistanceAlongContour()/200.0;
		if (spacing < SPACE_BETWEEN_POINTS)
			spacing = SPACE_BETWEEN_POINTS;

		Contour *evenContour = mContour->evenlySpaceContourPoints(spacing);
		smallContour = evenContour->createScaledContour(mZoomRatio/100.0,0,0); // small sized countour
		delete evenContour;
	}

	int ratio = mZoomRatio;	 
	int chunkSize;  // used to divide up iterations among the scales
	if (ratio <= 100)
		chunkSize = (int)(mOptions->mMaxIterations / (100.0/ratio*2-1));
	else 
        chunkSize = mOptions->mMaxIterations;
	int tripNum = 1;  // one trip at each scale

	// at each ZoomRatio, create an EdgeMagImage and a contour at that scale
	// and process with active contour
	while (ratio <= 100 || tripNum == 1){ // if ratio > 100 take at least one trip
		int iterations = (int)(pow(2,tripNum-1) * chunkSize);

		// resize EdgeMagImage to current scale
		if (ratio != 100)
            smallEdgeMagImage = resizeNN(EdgeMagImage, ratio);
		else
			smallEdgeMagImage = EdgeMagImage;

	    for (int i = 0; i < iterations; i++) {
		    moveContour(smallContour, smallEdgeMagImage, energyWeights);
		    if (i%5==0){
				// scale repositioned contour to viewed scale for display (mContour)
			    delete mContour;
			    mContour = smallContour->createScaledContour(100.0/ratio, 0, 0);
			    this->refreshImage();
			}		    
		} // end for (i=0; i< iterations)
		delete smallEdgeMagImage;

		// scale repositioned contour to next larger scale for more repositioning
		ratio *= 2;  // double ratio for next level		
		if (ratio <= 100){
		   Contour *tempContour;   
		   tempContour = smallContour->createScaledContour(2.0, 0, 0);
		   delete smallContour;
		   smallContour = tempContour;
		} 
		tripNum++;
		
	} // end while  

	// features such as glare spots may cause outline points to bunch and wrap
	// during active contour process
	mContour->removeKnots(spacing); //***005CM

	delete smallContour;  
}


/////////////////////////////////////////////////////////////////////
// traceFinalize:  Locks in trace after user cleanup, but before
//    user is allowed to move feature points (tip, notch, etc)
//
void TraceWindow::traceFinalize() {

	if ((NULL == mContour) || (! mTraceLocked)) //***006FC
		return;

	// after even spacing and normalization fin height will be approx 600 units

	mNormScale = normalizeContour(mContour); //***006CN
	mContour->removeKnots(3.0);       //***006CN
	Contour *evenContour = mContour->evenlySpaceContourPoints(3.0); //***006CN
	delete mContour;
	mContour = evenContour;

	if (NULL != mOutline) //***008OL
		delete mOutline;    //***008OL

	mOutline = new Outline(mContour,3.0); // ***008OL

	mTraceFinalized = true; //***006PD moved from beginning of function

	this->refreshImage();
}

////////////////////////////////////////////////////////////////////
// outlineCreate:  Creates outline object from trace.  The outline
//    contains a FloatContour, a Chain, and locations of all
//    feature points.  The outline is used throughout the rest
//    of the system processes.
//
void TraceWindow::outlineCreate()
{
  //***008OL do this in traceFinalize() above - JHS ??
}

void TraceWindow::zoomIn(int mouseX, int mouseY)
{
	if (mZoomRatio <= MAX_ZOOM) {
		mZoomRatio *= 2;
		//***1.95 force back from 24% to 25% going up in size
		if (mZoomRatio == 24)
			mZoomRatio ++; 

		zoomUpdate(true, mouseX, mouseY); //***1.75
	}
}

void TraceWindow::zoomOut(int mouseX, int mouseY)
{
	if (mZoomRatio > MIN_ZOOM) { //***1.95 - generalized restriction
		mZoomRatio /= 2;

		zoomUpdate(true, mouseX, mouseY); //***1.75
	}
}

void TraceWindow::zoomUpdate(bool setSize, int x, int y)
{

	double whereX, whereY; //***1.75

	if (setSize)
	{
		//***1.75 - find current position x,y as percent of current image size or drawable size
		//          this must be done BEFORE resizing
		whereX = (double)(mZoomXOffset+x) / mDrawingArea->allocation.width;
		whereY = (double)(mZoomYOffset+y) / mDrawingArea->allocation.height;
		if (whereX < 0.0) 
			whereX = 0.0;
		if (whereY < 0.0) 
			whereY = 0.0;
		//***1.75 - end
	}

	mZoomScale = mZoomRatio / 100.0;

	delete mImage;

	mImage = resizeNN(mNonZoomedImage, (float)mZoomRatio);

	if (setSize && NULL != mDrawingArea && NULL != mImage) {
		mIgnoreConfigureEventCnt++;

		if (mScrolledWindow->allocation.width != mSWWidthMax ||
		    mScrolledWindow->allocation.height != mSWHeightMax) {
			gtk_widget_set_usize(
				mScrolledWindow,
				((int)mImage->getNumCols() < mSWWidthMax) ? mImage->getNumCols() : mSWWidthMax,
				((int)mImage->getNumRows() < mSWHeightMax) ? mImage->getNumRows() : mSWHeightMax);

			mIgnoreConfigureEventCnt++;
		}

		gtk_drawing_area_size(
				GTK_DRAWING_AREA(mDrawingArea),
				mImage->getNumCols(),
				mImage->getNumRows());
	}

	gdk_draw_rectangle(
		mDrawingArea->window,
		mDrawingArea->style->bg_gc[GTK_STATE_NORMAL],
		TRUE,
		0,
		0,
		mDrawingArea->allocation.width,
		mDrawingArea->allocation.height);

	this->refreshImage();

	if (setSize) 
	{
		if (x < 0) x = 0;
		if (y < 0) y = 0;

		GtkAdjustment *vAdj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(mScrolledWindow));
		GtkAdjustment *hAdj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(mScrolledWindow));
		double newX = whereX * mImage->getNumCols();  //***1.75
		double newY = whereY * mImage->getNumRows();  //***1.75
		hAdj->value = newX - mScrolledWindow->allocation.width / 2;
		vAdj->value = newY - mScrolledWindow->allocation.height / 2;
		gtk_adjustment_value_changed(hAdj); //***1.75
		gtk_adjustment_value_changed(vAdj); //***1.75
	}
	else // must be clipping or flipping
	{
		//***1.75 - find location of desired image center and reposition scrollbar
		//          settings to center image as desired

		double centerX = mImageCenterX * mImage->getNumCols();
		double centerY = mImageCenterY * mImage->getNumRows();

		double left = centerX - mScrolledWindow->allocation.width / 2;
		double right = centerX + mScrolledWindow->allocation.width / 2;
		double top = centerY - mScrolledWindow->allocation.height / 2;
		double bottom = centerY + mScrolledWindow->allocation.height / 2;

		if (left < 0.0) 
			left = 0.0;
		if (top < 0.0) 
			top = 0.0;

		if (mScrolledWindow->allocation.height < mImage->getNumRows())
		{
			GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(
				GTK_SCROLLED_WINDOW(mScrolledWindow));
			adj->value = top;
			gtk_adjustment_value_changed(adj);
		}

		if (mScrolledWindow->allocation.width < mImage->getNumCols())
		{
			GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment(
				GTK_SCROLLED_WINDOW(mScrolledWindow));
			adj->value = left;
			gtk_adjustment_value_changed(adj);
		}
	}

	char mag[100];
	if (mIsFlipped)
		sprintf(mag, "    %d%%    \n(reversed)", mZoomRatio);
	else
		sprintf(mag, "    %d%%    ", mZoomRatio);
	gtk_label_set(GTK_LABEL(mLabelMagnification), mag);
}

void TraceWindow::zoomMapPointsToOriginal(int &x, int &y)
{
	/*BUG FIX: SAH: Below original code incorrectly maps the extreme pixel (farthest down row/rightmost col)
		to a pixel out of bounds in the original if mZoomScale>1 and the extream pixel coordinate is odd.
	x = (int) round(x / mZoomScale);
	y = (int) round(y / mZoomScale);*/

	if (mZoomScale<1.0) {
		//Same as before
		x = (int) round(x / mZoomScale);
		y = (int) round(y / mZoomScale);
	} else if (mZoomScale>1.0) {
		x = (int) floor(x / mZoomScale);
		y = (int) floor(y / mZoomScale);
	}
}

void TraceWindow::zoomMapPointsToZoomed(int &x, int &y)
{
	x = (int) round(x * mZoomScale);
	y = (int) round(y * mZoomScale);
}

int TraceWindow::zoomPointSize()
{
	if (mZoomRatio > 50)
      return (int) round(POINT_SIZE * mZoomScale);
    else
      return POINT_SIZE;   // krd 10/29/05 limit smallest point size to 2x2
}

void TraceWindow::refreshImage()
{
	on_traceDrawingArea_expose_event(NULL, NULL, (void *)this);
}


//*******************************************************************
//
// void TraceWindow::addUndo(ColorImage *u)
//
//    In response to new image modification, SAVE previous image
//    so current changes can be undone later.  Only ONE image or contour
//    can be saved at a time.
//
void TraceWindow::addUndo(ColorImage *u)
{
	gtk_widget_set_sensitive(mButtonUndo, TRUE);
	gtk_widget_set_sensitive(mButtonRedo, FALSE);

	if (mUndoImage != NULL)
		delete mUndoImage;

	if (mUndoContour != NULL) {
		delete mUndoContour;
		mUndoContour = NULL;
	}

	mUndoImage = new ColorImage(u);
}

//*******************************************************************
//
// void TraceWindow::addUndo(Contour *u)
//
//    In response to new contour tracing, SAVE previous contour
//    so current changes can be undone later.  Only ONE image or contour
//    can be saved at a time.
//
void TraceWindow::addUndo(Contour *u)
{
	gtk_widget_set_sensitive(mButtonUndo, TRUE);
	gtk_widget_set_sensitive(mButtonRedo, FALSE);

	if (mUndoImage != NULL) {
		delete mUndoImage;
		mUndoImage = NULL;
	}

	delete mUndoContour;

	mUndoContour = new Contour(u);
}

//*******************************************************************
//
// void TraceWindow::undo()
//
//    Undo the last change to EITHER the image or the contour.  Only
//    ONE change is currently stored at a time.
//
void TraceWindow::undo()
{
	gtk_widget_set_sensitive(mButtonUndo, FALSE);
	gtk_widget_set_sensitive(mButtonRedo, TRUE);

	if (mUndoImage != NULL) {

		ColorImage *temp = mNonZoomedImage;
		mNonZoomedImage = mUndoImage;
		mUndoImage = temp;

		//***1.95 - remove the last ImageMod from the list, but save it in case
		//          REDO is clicked 
		if (mImageMods.last(mLastMod))
			mImageMods.remove();

	} else if (mUndoContour != NULL) {

		Contour *temp = mContour;
		mContour = mUndoContour;
		mUndoContour = temp;
	}

	this->zoomUpdate(true);
}

//*******************************************************************
//
// void TraceWindow::redo()
//
//    Undo the last UNDO to EITHER the image or the contour.  Only
//    ONE change is currently stored at a time.
//
void TraceWindow::redo()
{
	gtk_widget_set_sensitive(mButtonUndo, TRUE);
	gtk_widget_set_sensitive(mButtonRedo, FALSE);

	if (mUndoImage != NULL) {

		ColorImage *temp = mNonZoomedImage;
		mNonZoomedImage = mUndoImage;
		mUndoImage = temp;

		//***1.95 - put the last Image Mod back into the list
		mImageMods.add(mLastMod); 
		mLastMod.set(ImageMod::IMG_none);

	} else if (mUndoContour != NULL) {

		Contour *temp = mContour;
		mContour = mUndoContour;
		mUndoContour = temp;

	}

	this->zoomUpdate(true);
}

void TraceWindow::cropInit(int x, int y)
{
	this->ensurePointInZoomedBounds(x, y); //***1.8 - must start in bounds

	addUndo(mNonZoomedImage);

	mXCropStart = x;
	mYCropStart = y;

	mXCropPrev = mYCropPrev = mXCropPrevLen = mYCropPrevLen = 0;
}

void TraceWindow::cropUpdate(int x, int y)
{
	this->ensurePointInZoomedBounds(x, y);

	int rowStart, colStart, numCropCols, numCropRows;

	if (x < mXCropStart) {
		colStart = x;
		numCropCols = mXCropStart - x;
	} else {
		colStart = mXCropStart;
		numCropCols = x - mXCropStart;
	}

	if (y < mYCropStart) {
		rowStart = y;
		numCropRows = mYCropStart - y;
	} else {
		rowStart = mYCropStart;
		numCropRows = y - mYCropStart;
	}

	int rowstride = mImage->getNumCols() * mImage->bytesPerPixel();
	// remove top row
	gdk_draw_rgb_image(
		mDrawingArea->window,
		mDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
		mXCropPrev + mZoomXOffset, mYCropPrev + mZoomYOffset,
		mXCropPrevLen + 1, 1,
		GDK_RGB_DITHER_NONE,
		(guchar*)(mImage->getData() + mYCropPrev * mImage->getNumCols() + mXCropPrev),
		rowstride);

	//remove bottom row of last crop box
	gdk_draw_rgb_image(
		mDrawingArea->window,
		mDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
		mXCropPrev + mZoomXOffset, mYCropPrev + mYCropPrevLen + mZoomYOffset,
		mXCropPrevLen + 1, 1,
		GDK_RGB_DITHER_NONE,
		(guchar*)(mImage->getData() + (mYCropPrev+mYCropPrevLen)* mImage->getNumCols() + mXCropPrev),
		rowstride);

	//remove left edge of last crop box...
	gdk_draw_rgb_image(
		mDrawingArea->window,
		mDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
		mXCropPrev + mZoomXOffset, mYCropPrev + mZoomYOffset,
		1, mYCropPrevLen + 1,
		GDK_RGB_DITHER_NONE,
		(guchar*)(mImage->getData() + mYCropPrev * mImage->getNumCols() + mXCropPrev),
		rowstride);

	//remove right edge of last crop box...
	gdk_draw_rgb_image(
		mDrawingArea->window,
		mDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
		mXCropPrev + mXCropPrevLen + mZoomXOffset, mYCropPrev + mZoomYOffset,
		1, mYCropPrevLen + 1,
		GDK_RGB_DITHER_NONE,
		(guchar*)(mImage->getData() + mYCropPrev * mImage->getNumCols() + (mXCropPrev + mXCropPrevLen)),
		rowstride);

	gdk_draw_rectangle(
			mDrawingArea->window,
			mGC,
			FALSE,
			colStart + mZoomXOffset,
			rowStart + mZoomYOffset,
			numCropCols,
			numCropRows);

	mXCropEnd = x;
	mYCropEnd = y;

	mXCropPrev = colStart;
	mYCropPrev = rowStart;

	mXCropPrevLen = numCropCols;
	mYCropPrevLen = numCropRows;
}

void TraceWindow::cropFinalize(int x, int y)
{
	ColorImage *temp = mNonZoomedImage;

	int xMin, xMax, yMin, yMax;

	this->ensurePointInZoomedBounds(x, y);

	if (x < mXCropStart) {
		xMin = x;
		xMax = mXCropStart;
	} else {
		xMin = mXCropStart;
		xMax = x;
	}

	if (y < mYCropStart) {
		yMin = y;
		yMax = mYCropStart;
	} else {
		yMin = mYCropStart;
		yMax = y;
	}
	this->zoomMapPointsToOriginal(xMin, yMin);
	this->zoomMapPointsToOriginal(xMax, yMax);

	if ((xMin < xMax) && (yMin < yMax)) //***1.8 - don't crop if size ridiculous
	{
		mNonZoomedImage = crop(temp, xMin, yMin, xMax, yMax);
		delete temp;

		//***1.8 - add the CROP modification to list
		ImageMod imod(ImageMod::IMG_crop, xMin, yMin, xMax, yMax);
		mImageMods.add(imod);
	
		zoomUpdate(true);
	}
}

void TraceWindow::rotateInit(int x, int y)
{
	mRotateXCenter = mImage->getNumCols() / 2;
	mRotateYCenter = mImage->getNumRows() / 2;

	mRotateXStart = x;
	mRotateYStart = mImage->getNumRows() - y;

	mRotateStartAngle = atan2(
			(double)(mRotateYStart - mRotateYCenter),
			(double)(mRotateXStart - mRotateXCenter));

	mRotateOriginalImage = new ColorImage(mImage);
}

void TraceWindow::rotateUpdate(int x, int y)
{
	float angle = atan2(
			(double)(mImage->getNumRows() - y - mRotateYCenter),
			(double)(x - mRotateXCenter))

			- mRotateStartAngle;

	delete mImage;
	mImage = rotateNN(mRotateOriginalImage, angle);

	refreshImage();
}

void TraceWindow::rotateFinal(int x, int y)
{
	float angle = atan2(
			(double)(mImage->getNumRows() - y - mRotateYCenter),
			(double)(x - mRotateXCenter))

			- mRotateStartAngle;

	delete mRotateOriginalImage;

	addUndo(mNonZoomedImage);
	ColorImage *temp = mNonZoomedImage;
	mNonZoomedImage = rotate(mNonZoomedImage, angle);
	delete temp;
	zoomUpdate(false);
}

GtkWidget *TraceWindow::createTraceWindow(const string &title)
{
    GtkWidget *traceWindow;
    GtkWidget *traceHBoxMain;
    GtkWidget *traceAlignment;
    GtkWidget *traceVBoxLeft;
    GtkWidget *traceHBoxTraceCommands;
    GtkWidget *traceHandleBoxTransformCommands;
    GtkWidget *traceToolBarTransformCommands;
    GtkWidget *tmp_toolbar_icon;
    GtkWidget *traceButtonFlipHorizontally;
    GtkWidget *traceButtonFlipVertically;
    GtkWidget *traceButtonEnhanceContrast;
    GtkWidget *traceButtonAlterBrightness;
    GtkWidget *traceButtonDespeckle;
    GtkWidget *traceButtonSmooth;
    GtkWidget *traceButtonResize;
    GtkWidget *hseparator1;
    GtkWidget *traceHBoxTraceTools;
    GtkWidget *traceHandleBoxTraceTools;
    GtkWidget *traceToolBarTraceTools;
    GtkWidget *traceRadioButtonMagnify;
    GtkWidget *traceRadioButtonAutoTrace; //103AT SAH
	GtkWidget *traceRadioButtonPencil;//103AT SAH
    GtkWidget *traceRadioButtonAddPoint;
    GtkWidget *traceRadioButtonMovePoint;
    GtkWidget *traceRadioButtonMoveFeature; //***006PM
    GtkWidget *traceRadioButtonEraser;
    GtkWidget *traceRadioButtonChopoutline; //*** 1.5 krd
    GtkWidget *traceRadioButtonCrop;
    GtkWidget *traceRadioButtonRotate;
    GtkWidget *traceFrameMagnification;
    GtkWidget *traceFramePosition;
    GtkWidget *traceViewPort;
    GtkWidget *traceEventBox;
    GtkWidget *traceRightFrame;
    GtkWidget *traceVBoxRight;
    GtkWidget *traceLabelKnownInfo;
    GtkWidget *traceLabelIDCode;
    GtkWidget *traceLabelName;
    GtkWidget *traceLabelDate;
    GtkWidget *traceLabelRoll;
    GtkWidget *traceLabelLocation;
    GtkWidget *traceLabelDamage;
    GtkWidget *traceLabelDescription;
    GtkWidget *traceVButtonBox;
    guint traceButtonMatch_key;
    GtkWidget *traceButtonMatch;
    guint traceButtonSave_key;
    GtkWidget *traceButtonSave;
    guint traceButtonAddToDatabase_key;
    GtkWidget *traceButtonAddToDatabase;
    guint traceButtonCancel_key;
    GtkWidget *traceButtonCancel;
    GtkAccelGroup *accel_group;
    GtkTooltips *tooltips;
    GtkWidget *tmpBox, *tmpLabel, *tmpIcon;

    tooltips = gtk_tooltips_new();

    accel_group = gtk_accel_group_new();

    traceWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_object_set_data(GTK_OBJECT(traceWindow), "traceWindow",
			traceWindow);
    gtk_window_set_policy(GTK_WINDOW(traceWindow), TRUE, TRUE, FALSE);
    gtk_window_set_title(GTK_WINDOW(traceWindow), title.c_str());
    gtk_window_set_wmclass(GTK_WINDOW(traceWindow), "darwin_trace", "DARWIN");
	gtk_window_set_default_size(GTK_WINDOW(traceWindow), 912, 700);
	// center near top of screen - assumes 1024x768 or 1024x800 min screen
	gint x, y;
	gtk_window_get_size(GTK_WINDOW(traceWindow),&x, &y);
	gtk_window_move(
		GTK_WINDOW(traceWindow),
		(gdk_screen_width() - x) / 2, // left edge of window on screen
		10);                          // top of window on screen

    traceHBoxMain = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(traceHBoxMain);
    gtk_container_add(GTK_CONTAINER(traceWindow), traceHBoxMain);

    traceAlignment = gtk_alignment_new(0.5, 0.5, 1, 1);
    gtk_widget_show(traceAlignment);
    gtk_box_pack_start(GTK_BOX(traceHBoxMain), traceAlignment, TRUE, TRUE,
		       0);

    traceVBoxLeft = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(traceVBoxLeft);
    gtk_container_add(GTK_CONTAINER(traceAlignment), traceVBoxLeft);

    traceHBoxTraceCommands = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(traceHBoxTraceCommands);
    gtk_box_pack_start(GTK_BOX(traceVBoxLeft), traceHBoxTraceCommands,
		       FALSE, TRUE, 0);

		// The first line of buttons contains the permanent buttons
		// (magnify, undo, redo) and the various acceptance buttons
		// (accept image, accept trace, accept fetaure points).

    traceHandleBoxTransformCommands = gtk_handle_box_new();
    gtk_widget_show(traceHandleBoxTransformCommands);
    gtk_box_pack_start(GTK_BOX(traceHBoxTraceCommands),
		       traceHandleBoxTransformCommands, FALSE, TRUE, 0);

   traceToolBarTransformCommands = gtk_toolbar_new();
 	gtk_toolbar_set_orientation (GTK_TOOLBAR(traceToolBarTransformCommands),
 		       GTK_ORIENTATION_HORIZONTAL);
 	gtk_toolbar_set_style (GTK_TOOLBAR(traceToolBarTransformCommands), 
 		       GTK_TOOLBAR_ICONS);

    gtk_widget_show(traceToolBarTransformCommands);
    gtk_container_add(GTK_CONTAINER(traceHandleBoxTransformCommands),
		      traceToolBarTransformCommands);
    gtk_toolbar_set_style(GTK_TOOLBAR
			  (traceToolBarTransformCommands),
			  GTK_TOOLBAR_BOTH);

		// UNDO button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, undo_button_xpm);
		mButtonUndo =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTransformCommands),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Undo"),
				_("Undo the changes of your last action."),
				NULL, tmp_toolbar_icon, NULL, NULL);

		gtk_widget_set_sensitive(mButtonUndo, FALSE);
      gtk_button_set_relief (GTK_BUTTON (mButtonUndo), GTK_RELIEF_NONE);
    gtk_widget_show(mButtonUndo);

		// REDO button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, redo_button_xpm);
		mButtonRedo =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTransformCommands),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Redo"),
				_("Redo the changes you've just undone."),
				NULL, tmp_toolbar_icon, NULL, NULL);

		gtk_widget_set_sensitive(mButtonRedo, FALSE);
      gtk_button_set_relief (GTK_BUTTON (mButtonRedo), GTK_RELIEF_NONE);
    gtk_widget_show(mButtonRedo);

		// a SPACER in the row

		gtk_toolbar_append_space(GTK_TOOLBAR(traceToolBarTransformCommands));

		//***051 - IMAGE Processing Button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, fin_xpm);
		mButtonImageMod =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTransformCommands),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Modify Image"),
				_("Click here to manipulate the image."),
				NULL, tmp_toolbar_icon, NULL, NULL);

		gtk_widget_set_sensitive(mButtonImageMod, FALSE); //***054
		gtk_button_set_relief (GTK_BUTTON (mButtonImageMod), GTK_RELIEF_NONE);
		//gtk_widget_hide(mButtonImageMod); //***054 //**100HCI SAH

		// ACCEPT IMAGE button  (now labeled TRACE)  // krd

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, trace_xpm);
		mButtonImageOK =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTransformCommands),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Trace Outline"),
				_("Click here to trace outline."),
				NULL, tmp_toolbar_icon, NULL, NULL);

		gtk_widget_set_sensitive(mButtonImageOK, TRUE);
      gtk_button_set_relief (GTK_BUTTON (mButtonImageOK), GTK_RELIEF_NONE);
    gtk_widget_show(mButtonImageOK);

		// ACCEPT TRACE button - (now labeled VIEW POINTS)  

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, view_points_xpm);
		mButtonTraceOK =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTransformCommands),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Locate Features"),
				_("Click here to view feature points."),
				NULL, tmp_toolbar_icon, NULL, NULL);

		gtk_widget_set_sensitive(mButtonTraceOK, FALSE);
      gtk_button_set_relief (GTK_BUTTON (mButtonTraceOK), GTK_RELIEF_NONE);
    //gtk_widget_hide(mButtonTraceOK); //***054 //**100HCI SAH

	//***1.4 - new button for loaded, previously saved fin trace
		
	// UNLOCK TRACE button - allows user to revise previously traced fin

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, unlock_trace_xpm);
		mButtonTraceUnlock =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTransformCommands),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Unlock Traced Fin"),
				_("Click here to allow modification of trace and feature point locations."),
				NULL, tmp_toolbar_icon, NULL, NULL);

		gtk_widget_set_sensitive(mButtonTraceUnlock, FALSE);
      gtk_button_set_relief (GTK_BUTTON (mButtonTraceUnlock), GTK_RELIEF_NONE);
    gtk_widget_hide(mButtonTraceUnlock); //***054

	//***054TW - new message display box
	
	hseparator1 = gtk_hseparator_new();
    gtk_widget_show(hseparator1);
    gtk_box_pack_start(GTK_BOX(traceVBoxLeft), hseparator1, FALSE, TRUE, 0);

    traceHBoxTraceTools = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(traceHBoxTraceTools);
    gtk_box_pack_start(GTK_BOX(traceVBoxLeft),
				traceHBoxTraceTools, FALSE,TRUE, 0);

	mStatusLabel = gtk_label_new(_("Modify Image Using Tools Below (Note: Dolphin MUST swim to your LEFT!)"));
	gtk_widget_show(mStatusLabel);
	gtk_box_pack_start(GTK_BOX(traceHBoxTraceTools),
				mStatusLabel, FALSE, FALSE, 0);

		// Second line of BUTTONS -- this will change as different stage
		// of process is entered
		// (1) Image Manipulation
		// (2) Initial tracing and cleanup of outline
		// (3) User changes to feature points


		hseparator1 = gtk_hseparator_new();
    gtk_widget_show(hseparator1);
    gtk_box_pack_start(GTK_BOX(traceVBoxLeft), hseparator1, FALSE, TRUE, 0);

    traceHBoxTraceTools = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(traceHBoxTraceTools);
    gtk_box_pack_start(GTK_BOX(traceVBoxLeft),
				traceHBoxTraceTools, FALSE,TRUE, 0);

	// the tools for each phase

    traceHandleBoxTraceTools = gtk_handle_box_new();
    gtk_widget_show(traceHandleBoxTraceTools);
    gtk_box_pack_start(GTK_BOX(traceHBoxTraceTools),
				traceHandleBoxTraceTools, FALSE, TRUE, 0);

    traceToolBarTraceTools = gtk_toolbar_new();
  	 gtk_toolbar_set_orientation (GTK_TOOLBAR(traceToolBarTraceTools),
  		       GTK_ORIENTATION_HORIZONTAL);
  	 gtk_toolbar_set_style (GTK_TOOLBAR(traceToolBarTraceTools), 
  		       GTK_TOOLBAR_ICONS);

    // do not know what to replace this with yet - JHS
    //gtk_toolbar_set_space_style(GTK_TOOLBAR(traceToolBarTraceTools), GTK_TOOLBAR_SPACE_LINE);

    gtk_widget_show(traceToolBarTraceTools);
    gtk_container_add(GTK_CONTAINER(traceHandleBoxTraceTools),
		      traceToolBarTraceTools);
    //gtk_toolbar_set_button_relief(GTK_TOOLBAR(traceToolBarTraceTools),GTK_RELIEF_NONE);

		// MAGNIFY button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, magnify_xpm);
		traceRadioButtonMagnify =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_RADIOBUTTON, NULL,
				_("Mag"),
				_("Zoom in or out on the image.\n<Left Click> - zoom in; <Shift + Left Click> zoom out."),
				NULL, tmp_toolbar_icon, NULL, NULL);
		gtk_button_set_relief (GTK_BUTTON (traceRadioButtonMagnify), GTK_RELIEF_NONE);
		gtk_widget_show(traceRadioButtonMagnify);
		mRadioButtonMagnify = traceRadioButtonMagnify; //***006FC
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(traceRadioButtonMagnify),TRUE);
		gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(traceRadioButtonMagnify),FALSE);

		// a SPACER in the row

		gtk_toolbar_append_space(GTK_TOOLBAR(traceToolBarTraceTools));

		// The IMAGE PROCESSING options - for phase one
		// --------------------------------------------

		// FLIP HORIZONTALLY button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, flip_h_xpm);
		traceButtonFlipHorizontally =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Flip H"),
				_("Flip the image horizontally."), NULL,
				tmp_toolbar_icon, NULL, NULL);
		gtk_button_set_relief (GTK_BUTTON (traceButtonFlipHorizontally), GTK_RELIEF_NONE);
		gtk_widget_show(traceButtonFlipHorizontally);
		mButtonFlipHorizontally = traceButtonFlipHorizontally; //***006FC

		
		// FLIP VERTICALLY button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, flip_v_xpm);
		traceButtonFlipVertically =
			gtk_toolbar_append_element(GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Flip V"),
				_("Flip the image vertically."), NULL,
				tmp_toolbar_icon, NULL, NULL);

		gtk_button_set_relief (GTK_BUTTON (traceButtonFlipVertically), GTK_RELIEF_NONE);
		//***1.2 - hidden now	
		gtk_widget_hide(traceButtonFlipVertically);
		mButtonFlipVertically = traceButtonFlipVertically; //***006FC


		// CONTRAST button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, contrast_button_xpm);
		traceButtonEnhanceContrast =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Enhance C"),
				_("Enhance the image's contrast."),
				NULL, tmp_toolbar_icon, NULL, NULL);

		gtk_button_set_relief (GTK_BUTTON (traceButtonEnhanceContrast), GTK_RELIEF_NONE);
		gtk_widget_show(traceButtonEnhanceContrast);
		mButtonEnhanceContrast = traceButtonEnhanceContrast; //***006FC

		// BRIGHTNESS button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, brightness_button_xpm);
		traceButtonAlterBrightness =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Alt Bright"),
				_("Alter the image's brightness."),
				NULL, tmp_toolbar_icon, NULL, NULL);

		gtk_button_set_relief (GTK_BUTTON (traceButtonAlterBrightness), GTK_RELIEF_NONE);
		gtk_widget_show(traceButtonAlterBrightness);
		mButtonAlterBrightness = traceButtonAlterBrightness; //***006FC

		// DESPECKLE button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, despeckle_xpm);
		traceButtonDespeckle =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Despeck"),
				_("Despeckle the image."),
				NULL, tmp_toolbar_icon, NULL, NULL);

		gtk_button_set_relief (GTK_BUTTON (traceButtonDespeckle), GTK_RELIEF_NONE);
		gtk_widget_hide(traceButtonDespeckle); //***1.5
		mButtonDespeckle = traceButtonDespeckle; //***006FC
		
		// SMOOTH button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, smooth_xpm);
		traceButtonSmooth =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Smooth"),
				_("Smooth the image."),
				NULL, tmp_toolbar_icon, NULL, NULL);

      gtk_button_set_relief (GTK_BUTTON (traceButtonSmooth), GTK_RELIEF_NONE);
	//***1.2 - hidden now	
      gtk_widget_hide(traceButtonSmooth);
		mButtonSmooth = traceButtonSmooth; //***006FC


		// RESIZE button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, resize_button);
		traceButtonResize =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_BUTTON, NULL,
				_("Resize"),
				_("Resize the image."),
				NULL, tmp_toolbar_icon, NULL, NULL);

      gtk_button_set_relief (GTK_BUTTON (traceButtonResize), GTK_RELIEF_NONE);
	//***1.2 - hidden now	
      gtk_widget_hide(traceButtonResize);
		mButtonResize = traceButtonResize; //***006FC

		// CROP IMAGE button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, crop_button_xpm);
		traceRadioButtonCrop =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_RADIOBUTTON, traceRadioButtonMagnify,
				_("Crop"),
				_("Crop Tool - Left click and drag a box around the area to crop.  Then, left click inside the box to crop the image."),
				NULL, tmp_toolbar_icon, NULL, NULL);

      gtk_button_set_relief (GTK_BUTTON (traceRadioButtonCrop), GTK_RELIEF_NONE);
		gtk_widget_show(traceRadioButtonCrop);
		mRadioButtonCrop = traceRadioButtonCrop; //***006FC
      gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(traceRadioButtonCrop),FALSE);

		// ROTATE IMAGE button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, rotate_button_xpm);
		traceRadioButtonRotate =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_RADIOBUTTON, traceRadioButtonMagnify,
				_("Rotate"),
				_("Rotate Tool - Left click and carefully drag in a circular motion to rotate the image."),
				NULL, tmp_toolbar_icon, NULL, NULL);

      gtk_button_set_relief (GTK_BUTTON (traceRadioButtonRotate), GTK_RELIEF_NONE);
	//***1.2 - hidden now	
		gtk_widget_hide(traceRadioButtonRotate);
		mRadioButtonRotate = traceRadioButtonRotate; //***006FC
      gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(traceRadioButtonRotate),FALSE);

		// The TRACING options -- for phase two
		// ------------------------------------

      //---Begin Insert---  103AT SAH
	  //AutoTrace (Star burst?) Button -- given start and end we can create a trace of the full fin
	  
	  tmp_toolbar_icon = create_pixmap_from_data(traceWindow, autotrace_xpm);
		traceRadioButtonAutoTrace =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_RADIOBUTTON, traceRadioButtonMagnify,
				_("AutoTrace"),
				_("Click the start of the fin and then the end, and the computer will provide a trace of the fin."),
				NULL, tmp_toolbar_icon, NULL, NULL);

    gtk_widget_hide(traceRadioButtonAutoTrace);
		mRadioButtonAutoTrace = traceRadioButtonAutoTrace; //***006FC
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(traceRadioButtonAutoTrace),FALSE);
	  //---End Insert----  103AT SAH
	  
	  // PENCIL button -- for tracing fin outline

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, pencil_xpm);
		traceRadioButtonPencil =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_RADIOBUTTON, traceRadioButtonMagnify,
				_("Pencil"),
				_("Provide an initial outline of the fin.  Left click and drag to draw."),
				NULL, tmp_toolbar_icon, NULL, NULL);

    gtk_widget_hide(traceRadioButtonPencil);
		mRadioButtonPencil = traceRadioButtonPencil; //***006FC
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(traceRadioButtonPencil),FALSE);

		// ADD POINT button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, addpoint_xpm);
		traceRadioButtonAddPoint =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_RADIOBUTTON, traceRadioButtonMagnify,
				_("Add Point"),
				_("Add points after fin has been traced.  Left click where you'd like to add a point."),
				NULL, tmp_toolbar_icon, NULL, NULL);

    gtk_widget_hide(traceRadioButtonAddPoint);
		mRadioButtonAddPoint = traceRadioButtonAddPoint; //***006FC
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(traceRadioButtonAddPoint),FALSE);

		// ADJUST POINTS button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, finger_xpm);
		traceRadioButtonMovePoint =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_RADIOBUTTON, traceRadioButtonMagnify,
				_("Move"),
				_("Adjust points in your trace.  Left click near any point and drag it to the desired position."),
				NULL, tmp_toolbar_icon, NULL, NULL);

		gtk_widget_hide(traceRadioButtonMovePoint);
		mRadioButtonMovePoint = traceRadioButtonMovePoint; //***006FC
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(traceRadioButtonMovePoint), FALSE);

		// ERASE POINTS button

		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, eraser_xpm);
		traceRadioButtonEraser =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_RADIOBUTTON, traceRadioButtonMagnify,
				_("Erase"),
				_("Remove points in your trace.  Left click and drag over the points you'd like to erase."),
				NULL, tmp_toolbar_icon, NULL, NULL);

    gtk_widget_hide(traceRadioButtonEraser);
		mRadioButtonEraser = traceRadioButtonEraser; //***006FC
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(traceRadioButtonEraser),FALSE);

                        // CHOP OUTLINE POINTS button *** 1.5 krd

                tmp_toolbar_icon = create_pixmap_from_data(traceWindow, chopoutline_xpm);
                traceRadioButtonChopoutline =
                        gtk_toolbar_append_element(
                                GTK_TOOLBAR(traceToolBarTraceTools),
                                GTK_TOOLBAR_CHILD_RADIOBUTTON, traceRadioButtonMagnify,
                                _("Chop"),
                                _("Remove all points from here to end of trace."),
                                NULL, tmp_toolbar_icon, NULL, NULL);

    gtk_widget_hide(traceRadioButtonChopoutline);
                mRadioButtonChopoutline = traceRadioButtonChopoutline; //***006FC
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(traceRadioButtonChopoutline),FALSE);


		// FEATURE POINT MOVEMENT options -- for phase three
		// -------------------------------------------------

		// MOVE FEATURE button

		//***006PM begin additions for button to move notch
		tmp_toolbar_icon = create_pixmap_from_data(traceWindow, finger_xpm);
    traceRadioButtonMoveFeature =
			gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_RADIOBUTTON, traceRadioButtonMagnify,
				_("Move Feature Point"),
				_("To Move a Feature, Left click near feature and drag it to the desired position."),
				NULL, tmp_toolbar_icon, NULL, NULL);

		gtk_widget_hide(traceRadioButtonMoveFeature);
		mRadioButtonMoveFeature = traceRadioButtonMoveFeature; //***006FC
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(traceRadioButtonMoveFeature), FALSE);
		//***006PM end additions

	//***1.4 - Two new buttons for moving and scaling trace of previously saved fin
	//
	// TRACE Scale and Re-placement options -- for phase four
	// ---------------------------------------------------------

	//Support for moving and scaling a trace unneccessary and dropped in v2.0


	// MOVE TRACE button

	//***006PM begin additions for button to move Trace
	/*tmp_toolbar_icon = create_pixmap_from_data(traceWindow, finger_xpm);
	mButtonSlideTrace =
		gtk_toolbar_append_element(
				GTK_TOOLBAR(traceToolBarTraceTools),
				GTK_TOOLBAR_CHILD_RADIOBUTTON, traceRadioButtonMagnify,
				_("Move Fin Trace"),
				_("To Move the trace click near trace and drag it to the desired position."),
				NULL, tmp_toolbar_icon, NULL, NULL);
	gtk_widget_hide(mButtonSlideTrace);
	gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(mButtonSlideTrace), FALSE);

	// a SPACER in the row

	gtk_toolbar_append_space(GTK_TOOLBAR(traceToolBarTraceTools));

	// SCALE TRACE Slider

	//***006PM begin additions for button to change scale
	mButtonScaleTrace = gtk_hscale_new_with_range(0.25,3.5,0.05);
	gtk_range_set_increments(GTK_RANGE(mButtonScaleTrace),0.01,0.05);
	gtk_range_set_value(GTK_RANGE(mButtonScaleTrace),mNormScale);
    gtk_box_pack_start(GTK_BOX(traceHBoxTraceTools),
				mButtonScaleTrace, TRUE, TRUE, 0);
	gtk_widget_hide(mButtonScaleTrace);*/

	// end of buttons

    traceFrameMagnification = gtk_frame_new((gchar *) NULL);
    gtk_widget_show(traceFrameMagnification);
    gtk_box_pack_end(GTK_BOX(traceHBoxTraceTools),
		       traceFrameMagnification, FALSE, TRUE, 0);

    mLabelMagnification = gtk_label_new(_("100%"));
    gtk_widget_show(mLabelMagnification);
    gtk_container_add(GTK_CONTAINER(traceFrameMagnification),
		      mLabelMagnification);

    traceFramePosition = gtk_frame_new((gchar *) NULL);
    gtk_widget_show(traceFramePosition);
    gtk_box_pack_end(GTK_BOX(traceHBoxTraceTools), traceFramePosition,
		       FALSE, TRUE, 0);

    mLabelPosition = gtk_label_new(_("(0, 0)"));
    gtk_widget_show(mLabelPosition);
    gtk_container_add(GTK_CONTAINER(traceFramePosition),
		      mLabelPosition);
    gtk_misc_set_padding(GTK_MISC(mLabelPosition), 6, 0);

    mScrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_show(mScrolledWindow);
    gtk_box_pack_start(GTK_BOX(traceVBoxLeft), mScrolledWindow, TRUE,
		       TRUE, 0);

    traceViewPort = gtk_viewport_new(NULL, NULL);
    gtk_widget_show(traceViewPort);
    gtk_container_add(GTK_CONTAINER(mScrolledWindow), traceViewPort);

    traceEventBox = gtk_event_box_new();
    gtk_widget_show(traceEventBox);
    gtk_container_add(GTK_CONTAINER(traceViewPort), traceEventBox);

    mDrawingArea = gtk_drawing_area_new();
    gtk_widget_show(mDrawingArea);
    gtk_container_add(GTK_CONTAINER(traceEventBox), mDrawingArea);

    traceVBoxRight = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(traceVBoxRight), 4);
    gtk_widget_show(traceVBoxRight);

    traceRightFrame = gtk_frame_new(NULL);
    gtk_widget_show(traceRightFrame);
    gtk_container_add(GTK_CONTAINER(traceRightFrame), traceVBoxRight);

    gtk_box_pack_start(GTK_BOX(traceHBoxMain), traceRightFrame, FALSE, TRUE, 0);


    // select font here - JHS
    GtkStyle *infoStyle = gtk_style_new();
    GdkFont *infoFont = gdk_font_load("-*-helvetica-bold-r-*-*-*-160-*-*-*-*-*-*");
    if (!infoFont)
       infoFont = gdk_font_load("fixed");
    gtk_style_set_font(infoStyle, infoFont);


    traceLabelKnownInfo = gtk_label_new(_("Known Information"));
    gtk_widget_set_style(traceLabelKnownInfo, infoStyle);
    gtk_widget_show(traceLabelKnownInfo);
    gtk_style_unref(infoStyle);

    gtk_box_pack_start(GTK_BOX(traceVBoxRight), traceLabelKnownInfo, FALSE,
		       FALSE, 5);

	// ID CODE Entry

    traceLabelIDCode = gtk_label_new(_("ID Code"));
    gtk_widget_show(traceLabelIDCode);
    gtk_box_pack_start(GTK_BOX(traceVBoxRight), traceLabelIDCode, FALSE,
		       FALSE, 2);

	mEntryID = gtk_entry_new();
	//***1.4 - load ID from fin data if loading previously traced fin
	if ((NULL != mFin) && (mFin->mIDCode != "NONE"))
		gtk_entry_set_text(GTK_ENTRY(mEntryID), mFin->mIDCode.c_str());
	//***1.65 - if ID is to be hidden, replace ID with ****
	if ((NULL != mFin) && (mOptions->mHideIDs))
		gtk_entry_set_text(GTK_ENTRY(mEntryID), "****");
    gtk_widget_show(mEntryID);
    gtk_box_pack_start(GTK_BOX(traceVBoxRight), mEntryID, FALSE, FALSE,
		       0);

	// NAME Entry

    traceLabelName = gtk_label_new(_("Name"));
    gtk_widget_show(traceLabelName);
    gtk_box_pack_start(GTK_BOX(traceVBoxRight), traceLabelName, FALSE,
		       FALSE, 2);

    mEntryName = gtk_entry_new();
	//***1.4 - load name from fin data if loading previously traced fin
	if ((NULL != mFin) && (mFin->mName != "NONE"))
		gtk_entry_set_text(GTK_ENTRY(mEntryName), mFin->mName.c_str());
    gtk_widget_show(mEntryName);
    gtk_box_pack_start(GTK_BOX(traceVBoxRight), mEntryName, FALSE,
		       FALSE, 0);

	// DATE Entry

    traceLabelDate = gtk_label_new(_("Date of Sighting"));
    gtk_widget_show(traceLabelDate);
    gtk_box_pack_start(GTK_BOX(traceVBoxRight), traceLabelDate, FALSE,
		       FALSE, 2);

    mEntryDate = gtk_entry_new();
	//***1.4 - load date from fin data if loading previously traced fin
	if (NULL != mFin) 
	{
		if (mFin->mDateOfSighting != "NONE")
			gtk_entry_set_text(GTK_ENTRY(mEntryDate), mFin->mDateOfSighting.c_str());
	}
	else
	{
		//***1.1 - set initial date entry from image file EXIF data, if available
		c_Exif *Exif_Extractor= new c_Exif(mImagefilename.c_str());

		string Image_Date = Exif_Extractor->GetDate();
		delete Exif_Extractor;
		gtk_entry_set_text(GTK_ENTRY(mEntryDate),Image_Date.c_str());
	}
    gtk_widget_show(mEntryDate);
    gtk_box_pack_start(GTK_BOX(traceVBoxRight), mEntryDate, FALSE,
		       FALSE, 0);

	// ROLL & FRAME Entry

    traceLabelRoll = gtk_label_new(_("Roll and Frame"));
    gtk_widget_show(traceLabelRoll);
    gtk_box_pack_start(GTK_BOX(traceVBoxRight), traceLabelRoll, FALSE,
		       FALSE, 3);

    mEntryRoll = gtk_entry_new();
	//***1.4 - load roll & frame from fin data if loading previously traced fin
	if ((NULL != mFin) && (mFin->mRollAndFrame != "NONE"))
		gtk_entry_set_text(GTK_ENTRY(mEntryRoll), mFin->mRollAndFrame.c_str());

    gtk_widget_show(mEntryRoll);
    gtk_box_pack_start(GTK_BOX(traceVBoxRight), mEntryRoll, FALSE,
		       FALSE, 0);

	// LOCATION CODE Entry

    traceLabelLocation = gtk_label_new(_("Location Code"));
    gtk_widget_show(traceLabelLocation);
    gtk_box_pack_start(GTK_BOX(traceVBoxRight), traceLabelLocation, FALSE,
		       FALSE, 2);

    mEntryLocation = gtk_entry_new();
	//***1.4 - load location from fin data if loading previously traced fin
	if ((NULL != mFin) && (mFin->mLocationCode != "NONE"))
		gtk_entry_set_text(GTK_ENTRY(mEntryLocation), mFin->mLocationCode.c_str());

    gtk_widget_show(mEntryLocation);
    gtk_box_pack_start(GTK_BOX(traceVBoxRight), mEntryLocation, FALSE,
		       FALSE, 0);

	// DAMAGE CATEGORY Selection

    traceLabelDamage = gtk_label_new(_("Damage Category"));
    gtk_widget_show(traceLabelDamage);
    gtk_box_pack_start(GTK_BOX(traceVBoxRight), traceLabelDamage, FALSE,
		       FALSE, 2);

	// damage category using a scroll-down menu


	if (NULL!=mDatabase) {
		//We have a full-blown database, create a drop-down

		mEntryDamage = gtk_combo_box_new_text();
		gtk_widget_show(mEntryDamage);
		gtk_box_pack_start(GTK_BOX(traceVBoxRight), mEntryDamage, FALSE,
				FALSE, 0);

		int activeNum = 0; //***1.4

		for (int catIDnum = 0; catIDnum < mDatabase->catCategoryNamesMax(); catIDnum++)
		{
			if ("NONE" == mDatabase->catCategoryName(catIDnum))
				gtk_combo_box_append_text(
					GTK_COMBO_BOX(mEntryDamage),
					_("Unspecified"));
			else
				gtk_combo_box_append_text(
					GTK_COMBO_BOX(mEntryDamage),
					_(mDatabase->catCategoryName(catIDnum).c_str()));

			//***1.4 - set category from fin data if loading previously traced fin
			if ((NULL != mFin) && (mFin->mDamageCategory == mDatabase->catCategoryName(catIDnum)))
				activeNum = catIDnum;
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(mEntryDamage), activeNum); //***1.4 - use activeNum

	} else {//just give a label, we are probably opening a finz file directly from command-line (e.g. stand alone viewer)
		//Label with mDatabaseFin->mDamageCategory (string)
		mEntryDamage = gtk_label_new(_(mFin->mDamageCategory.c_str()));
		gtk_widget_show(mEntryDamage);
		gtk_box_pack_start(GTK_BOX(traceVBoxRight), mEntryDamage,
				   FALSE, FALSE, 2);
	}

	traceLabelDescription = gtk_label_new(_("Short Description"));
	gtk_widget_show(traceLabelDescription);
	gtk_box_pack_start(GTK_BOX(traceVBoxRight), traceLabelDescription,
			   FALSE, FALSE, 2);

	// Short Description TEXT ENTRY

    mEntryDescription = gtk_entry_new();
	//***1.4 - load short description from fin data if loading previously traced fin
	if ((NULL != mFin) && (mFin->mShortDescription != "NONE"))
		gtk_entry_set_text(GTK_ENTRY(mEntryDescription), mFin->mShortDescription.c_str());
    gtk_widget_show(mEntryDescription);
    gtk_box_pack_start(GTK_BOX(traceVBoxRight), mEntryDescription,
		       FALSE, FALSE, 0);

    traceVButtonBox = gtk_vbutton_box_new();
    gtk_widget_show(traceVButtonBox);
    gtk_box_pack_end(GTK_BOX(traceVBoxRight), traceVButtonBox, FALSE, TRUE,
		     0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(traceVButtonBox),
			      GTK_BUTTONBOX_END);
    gtk_button_box_set_spacing(GTK_BUTTON_BOX(traceVButtonBox), 3);

	// Dump/Export Sighting Data to File Button

    GtkWidget *traceButtonDumpData = gtk_button_new_with_label("");
    unsigned int traceButtonDump_key =
	gtk_label_parse_uline(GTK_LABEL(GTK_BIN(traceButtonDumpData)->child),
			      _("_Export Data"));
    gtk_widget_add_accelerator(traceButtonDumpData, "clicked", accel_group,
			       traceButtonDump_key, GDK_MOD1_MASK,
			       (GtkAccelFlags) 0);
    gtk_widget_show(traceButtonDumpData);
    gtk_container_add(GTK_CONTAINER(traceVButtonBox), traceButtonDumpData);
    GTK_WIDGET_SET_FLAGS(traceButtonDumpData, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, traceButtonDumpData,
			 _
			 ("Export the sighting data.  Append it to the survey area's master data file."),
			 NULL);
    gtk_widget_add_accelerator(traceButtonDumpData, "clicked", accel_group,
			       GDK_e, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

	// MATCH Fin to Database Button

    traceButtonMatch = gtk_button_new_with_label("");
    traceButtonMatch_key =
	gtk_label_parse_uline(GTK_LABEL(GTK_BIN(traceButtonMatch)->child),
			      _("_Match"));
    gtk_widget_add_accelerator(traceButtonMatch, "clicked", accel_group,
			       traceButtonMatch_key, GDK_MOD1_MASK,
			       (GtkAccelFlags) 0);
    gtk_widget_show(traceButtonMatch);
    gtk_container_add(GTK_CONTAINER(traceVButtonBox), traceButtonMatch);
    GTK_WIDGET_SET_FLAGS(traceButtonMatch, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, traceButtonMatch,
			 _
			 ("Attempt to match this fin against those in the database."),
			 NULL);
    gtk_widget_add_accelerator(traceButtonMatch, "clicked", accel_group,
			       GDK_m, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

	if ((title.substr(0,8) == "No Match") || (title.substr(0,8) == "Matches "))
		gtk_widget_set_sensitive(traceButtonMatch, FALSE); // do not re-match a trace here

	// SAVE Fin Trace Button

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, save_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

    traceButtonSave = gtk_button_new();
    traceButtonSave_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
			      _("_Save"));
    gtk_widget_add_accelerator(traceButtonSave, "clicked", accel_group,
			       traceButtonSave_key, GDK_MOD1_MASK,
			       (GtkAccelFlags) 0);
    gtk_container_add(GTK_CONTAINER(traceButtonSave), tmpBox);
    gtk_widget_show(traceButtonSave);
    gtk_container_add(GTK_CONTAINER(traceVButtonBox), traceButtonSave);
    GTK_WIDGET_SET_FLAGS(traceButtonSave, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, traceButtonSave,
			 _("Save this traced fin to a separate file.\n"
			   "(NOTE: If you desire to build a match queue or access this traced fin for "
			   "processing at a later time, then use this option BEFORE adding fin to "
			   "database or attempting a match.)"
			   ), NULL);
    gtk_widget_add_accelerator(traceButtonSave, "clicked", accel_group,
			       GDK_S, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

	if (NULL != mFin)
		gtk_widget_set_sensitive(traceButtonSave, FALSE); // cannot re-save a loaded trace

	// ADD Fin to DATABASE Button

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, add_database_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

    traceButtonAddToDatabase = gtk_button_new();
    traceButtonAddToDatabase_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
			      _("Add to _Database"));
    gtk_widget_add_accelerator(traceButtonAddToDatabase, "clicked",
			       accel_group, traceButtonAddToDatabase_key,
			       GDK_MOD1_MASK, (GtkAccelFlags) 0);
    gtk_container_add(GTK_CONTAINER(traceButtonAddToDatabase), tmpBox);
    gtk_widget_show(traceButtonAddToDatabase);
    gtk_container_add(GTK_CONTAINER(traceVButtonBox),
		      traceButtonAddToDatabase);
    GTK_WIDGET_SET_FLAGS(traceButtonAddToDatabase, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, traceButtonAddToDatabase,
			 _
			 ("Add this fin to the database without attempting to match it."),
			 NULL);
    gtk_widget_add_accelerator(traceButtonAddToDatabase, "clicked",
			       accel_group, GDK_D, GDK_MOD1_MASK,
			       GTK_ACCEL_VISIBLE);

	// CANCEL Button

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

    traceButtonCancel = gtk_button_new();
    traceButtonCancel_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
			      _("_Close"));
    gtk_widget_add_accelerator(traceButtonCancel, "clicked", accel_group,
			       traceButtonCancel_key, GDK_MOD1_MASK,
			       (GtkAccelFlags) 0);
    gtk_container_add(GTK_CONTAINER(traceButtonCancel), tmpBox);
    gtk_widget_show(traceButtonCancel);
    gtk_container_add(GTK_CONTAINER(traceVButtonBox), traceButtonCancel);
    GTK_WIDGET_SET_FLAGS(traceButtonCancel, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip(tooltips, traceButtonCancel,
			 _
			 ("Close this window and discard any work \nnot already saved or added to database."),
			 NULL);
    gtk_widget_add_accelerator(traceButtonCancel, "clicked", accel_group,
			       GDK_C, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

	gtk_signal_connect(
		    GTK_OBJECT(traceWindow),
		    "delete_event",
		    GTK_SIGNAL_FUNC(on_traceWindow_delete_event),
		    (void*)this
		    );


	//***051 new function below

    gtk_signal_connect(GTK_OBJECT(mButtonImageMod), "clicked",
		       GTK_SIGNAL_FUNC(on_traceButtonImageMod_clicked), (void *) this);

	//***006FC next three callbacks are new

    gtk_signal_connect(GTK_OBJECT(mButtonImageOK), "clicked",
		       GTK_SIGNAL_FUNC(on_traceButtonImageOK_clicked), (void *) this);
    gtk_signal_connect(GTK_OBJECT(mButtonTraceOK), "clicked",
		       GTK_SIGNAL_FUNC(on_traceButtonTraceOK_clicked), (void *) this);


    gtk_signal_connect(GTK_OBJECT(mButtonUndo), "clicked",
		       GTK_SIGNAL_FUNC(on_traceButtonUndo_clicked), (void *) this);
    gtk_signal_connect(GTK_OBJECT(mButtonRedo), "clicked",
		       GTK_SIGNAL_FUNC(on_traceButtonRedo_clicked), (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceButtonFlipHorizontally), "clicked",
		       GTK_SIGNAL_FUNC
		       (on_traceButtonFlipHorizontally_clicked), (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceButtonFlipVertically), "clicked",
		       GTK_SIGNAL_FUNC
		       (on_traceButtonFlipVertically_clicked), (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceButtonEnhanceContrast), "clicked",
		       GTK_SIGNAL_FUNC
		       (on_traceButtonEnhanceContrast_clicked), (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceButtonAlterBrightness), "clicked",
		       GTK_SIGNAL_FUNC
		       (on_traceButtonAlterBrightness_clicked), (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceButtonDespeckle), "clicked",
		       GTK_SIGNAL_FUNC(on_traceButtonDespeckle_clicked),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceButtonSmooth), "clicked",
		       GTK_SIGNAL_FUNC(on_traceButtonSmooth_clicked),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceButtonResize), "clicked",
		       GTK_SIGNAL_FUNC(on_traceButtonResize_clicked),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceRadioButtonMagnify), "toggled",
		       GTK_SIGNAL_FUNC(on_traceRadioButtonMagnify_toggled),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceRadioButtonAutoTrace), "toggled",	//103AT SAH
		       GTK_SIGNAL_FUNC(on_traceRadioButtonAutoTrace_toggled),		 //103AT SAH
		       (void *) this);    
	gtk_signal_connect(GTK_OBJECT(traceRadioButtonPencil), "toggled",
		       GTK_SIGNAL_FUNC(on_traceRadioButtonPencil_toggled),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceRadioButtonAddPoint), "toggled",
		       GTK_SIGNAL_FUNC(on_traceRadioButtonAddPoint_toggled),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceRadioButtonMovePoint), "toggled",
		       GTK_SIGNAL_FUNC(on_traceRadioButtonMovePoint_toggled),
		       (void *) this);
	 //***006PM addition to move Notch
    gtk_signal_connect(GTK_OBJECT(traceRadioButtonMoveFeature), "toggled",
		       GTK_SIGNAL_FUNC(on_traceRadioButtonMoveFeature_toggled),
		       (void *) this);
	 //***006PM end of additional code
    gtk_signal_connect(GTK_OBJECT(traceRadioButtonEraser), "toggled",
		       GTK_SIGNAL_FUNC(on_traceRadioButtonEraser_toggled),
		       (void *) this);
         //***1.5 new trace button
    gtk_signal_connect(GTK_OBJECT(traceRadioButtonChopoutline), "toggled",
                       GTK_SIGNAL_FUNC(on_traceRadioButtonChopoutline_toggled),
                       (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceRadioButtonCrop), "toggled",
		       GTK_SIGNAL_FUNC(on_traceRadioButtonCrop_toggled),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(traceRadioButtonRotate), "toggled",
		       GTK_SIGNAL_FUNC(on_traceRadioButtonRotate_toggled),
		       (void *) this);

	//***1.4 - three new callbacks
/*    gtk_signal_connect(GTK_OBJECT(mButtonSlideTrace), "toggled",
		       GTK_SIGNAL_FUNC(on_traceButtonSlideTrace_toggled),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(mButtonScaleTrace), "value-changed",
		       GTK_SIGNAL_FUNC(on_traceButtonScaleTrace_changed),
		       (void *) this);*/
    gtk_signal_connect(GTK_OBJECT(mButtonTraceUnlock), "clicked",
		       GTK_SIGNAL_FUNC(on_traceButtonTraceUnlock_clicked),
		       (void *) this);


    gtk_signal_connect(
		    GTK_OBJECT(traceEventBox),
		    "button_press_event",
		    GTK_SIGNAL_FUNC(on_traceEventBox_button_press_event),
		    (void *) this);

    gtk_signal_connect(
		    GTK_OBJECT(traceEventBox),
		    "button_release_event",
		    GTK_SIGNAL_FUNC(on_traceEventBox_button_release_event),
		    (void *)this);

    gtk_signal_connect(
		    GTK_OBJECT(traceEventBox),
		    "motion_notify_event",
		    GTK_SIGNAL_FUNC(on_traceEventBox_motion_notify_event),
		    (void *) this);

    gtk_signal_connect(
		    GTK_OBJECT(mDrawingArea),
		    "expose_event",
		    GTK_SIGNAL_FUNC(on_traceDrawingArea_expose_event),
		    (void *) this);

    gtk_signal_connect(
		    GTK_OBJECT(mDrawingArea),
		    "configure_event",
		    GTK_SIGNAL_FUNC(on_mDrawingArea_configure_event),
		    (void *) this);

    gtk_signal_connect(
		    GTK_OBJECT(mScrolledWindow),
		    "configure_event",
		    GTK_SIGNAL_FUNC(on_mScrolledWindow_configure_event),
		    (void *) this);

	//***1.96a - new callback
    gtk_signal_connect(
		    GTK_OBJECT(traceButtonDumpData),
		    "clicked",
		    GTK_SIGNAL_FUNC(on_traceButtonDumpData_clicked),
		    (void *) this);

    gtk_signal_connect(
		    GTK_OBJECT(traceButtonMatch),
		    "clicked",
		    GTK_SIGNAL_FUNC(on_traceButtonMatch_clicked),
		    (void *) this);

    gtk_signal_connect(
		    GTK_OBJECT(traceButtonSave),
		    "clicked",
		    GTK_SIGNAL_FUNC(on_traceButtonSave_clicked),
		    (void *) this);

    gtk_signal_connect(
		    GTK_OBJECT(traceButtonAddToDatabase),
		    "clicked",
		    GTK_SIGNAL_FUNC(on_traceButtonAddToDatabase_clicked),
		    (void *) this);

    gtk_signal_connect(
		    GTK_OBJECT(traceButtonCancel),
		    "clicked",
		    GTK_SIGNAL_FUNC(on_traceButtonCancel_clicked),
		    (void *) this);

    gtk_widget_grab_default(traceButtonMatch);
    gtk_object_set_data(GTK_OBJECT(traceWindow), "tooltips", tooltips);

    gtk_window_add_accel_group(GTK_WINDOW(traceWindow), accel_group);


	//SAH--Allow trace window to launch independently
	if (NULL == mDatabase ) {
		//Disable Match, Add to Database, Export Data buttons
		//Could use gtk_widget_hide(wiget) to hide completely
		gtk_widget_set_sensitive(traceButtonMatch,FALSE);
		gtk_widget_set_sensitive(traceButtonAddToDatabase,FALSE);
		gtk_widget_set_sensitive(traceButtonDumpData,FALSE);
	}

    return traceWindow;
}

GtkWidget* TraceWindow::createQuestionDialog()
{
  GtkWidget *questionDialog;
  GtkWidget *questionVBox;
  GtkWidget *questionHBox;
  GtkWidget *questionPixmap;
  GtkWidget *questionLabel;
  GtkWidget *dialog_action_area1;
  GtkWidget *questionHButtonBox;
  guint questionButtonYes_key;
  GtkWidget *questionButtonYes;
  guint questionButtonCancel_key;
  GtkWidget *questionButtonCancel;
  GtkAccelGroup *accel_group;
  GtkWidget *tmpBox, *tmpLabel, *tmpIcon;

  accel_group = gtk_accel_group_new ();

  questionDialog = gtk_dialog_new ();
  gtk_object_set_data (GTK_OBJECT (questionDialog), "questionDialog", questionDialog);
  gtk_window_set_title (GTK_WINDOW (questionDialog), _("Question"));
  GTK_WINDOW (questionDialog)->type = WINDOW_DIALOG;
  gtk_window_set_position (GTK_WINDOW (questionDialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (questionDialog), TRUE);
  gtk_window_set_policy (GTK_WINDOW (questionDialog), TRUE, TRUE, FALSE);
  gtk_window_set_wmclass(GTK_WINDOW(questionDialog), "darwin_question", "DARWIN");

  questionVBox = GTK_DIALOG (questionDialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (questionDialog), "questionVBox", questionVBox);
  gtk_widget_show (questionVBox);

  questionHBox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (questionHBox);
  gtk_box_pack_start (GTK_BOX (questionVBox), questionHBox, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (questionHBox), 10);

  questionPixmap = create_pixmap_from_data(questionDialog, light_bulb_xpm);
  gtk_widget_show (questionPixmap);
  gtk_box_pack_start (GTK_BOX (questionHBox), questionPixmap, TRUE, TRUE, 0);

  questionLabel = gtk_label_new (_("There are no fins in the database,\nso there's really nothing to match your\nfin against.  Would you like to just\nadd your fin to the database?"));
  gtk_widget_show (questionLabel);
  gtk_box_pack_start (GTK_BOX (questionHBox), questionLabel, FALSE, FALSE, 0);

  dialog_action_area1 = GTK_DIALOG (questionDialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (questionDialog), "dialog_action_area1", dialog_action_area1);
  gtk_widget_show (dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 10);

  questionHButtonBox = gtk_hbutton_box_new ();
  gtk_widget_show (questionHButtonBox);
  gtk_box_pack_start (GTK_BOX (dialog_action_area1), questionHButtonBox, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (questionHButtonBox), GTK_BUTTONBOX_END);

  questionButtonYes = gtk_button_new_with_label ("");
  questionButtonYes_key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (questionButtonYes)->child),
                                   _("_Yes"));
  gtk_widget_add_accelerator (questionButtonYes, "clicked", accel_group,
                              questionButtonYes_key, GDK_MOD1_MASK, (GtkAccelFlags)0);
  gtk_widget_show (questionButtonYes);
  gtk_container_add (GTK_CONTAINER (questionHButtonBox), questionButtonYes);
  GTK_WIDGET_SET_FLAGS (questionButtonYes, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (questionButtonYes, "clicked", accel_group,
                              GDK_Y, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  tmpBox = gtk_hbox_new(FALSE, 0);
  tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
  gtk_widget_show(tmpIcon);
  tmpLabel = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
  gtk_widget_show(tmpLabel);
  gtk_widget_show(tmpBox);

  questionButtonCancel = gtk_button_new();
  questionButtonCancel_key = gtk_label_parse_uline(GTK_LABEL(tmpLabel),
                                   _("_Cancel"));
  gtk_widget_add_accelerator (questionButtonCancel, "clicked", accel_group,
                              questionButtonCancel_key, GDK_MOD1_MASK, (GtkAccelFlags)0);

  gtk_container_add(GTK_CONTAINER(questionButtonCancel), tmpBox);
  gtk_widget_show (questionButtonCancel);
  gtk_container_add (GTK_CONTAINER (questionHButtonBox), questionButtonCancel);
  GTK_WIDGET_SET_FLAGS (questionButtonCancel, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (questionButtonCancel, "clicked", accel_group,
                              GDK_C, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (questionButtonCancel, "clicked", accel_group,
                              GDK_Escape, (GdkModifierType)0,
                              GTK_ACCEL_VISIBLE);

  gtk_signal_connect (GTK_OBJECT (questionDialog), "delete_event",
                      GTK_SIGNAL_FUNC (on_questionDialog_delete_event),
                      (void *)this);
  gtk_signal_connect (GTK_OBJECT (questionButtonYes), "clicked",
                      GTK_SIGNAL_FUNC (on_questionButtonYes_clicked),
                      (void *)this);
  gtk_signal_connect (GTK_OBJECT (questionButtonCancel), "clicked",
                      GTK_SIGNAL_FUNC (on_questionButtonCancel_clicked),
                      (void *)this);

  gtk_window_add_accel_group (GTK_WINDOW (questionDialog), accel_group);

  return questionDialog;
}

gboolean on_traceWindow_delete_event(GtkWidget * widget,
				     GdkEvent * event, gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return FALSE;

	delete traceWin;

	return TRUE;
}

void on_traceButtonUndo_clicked(GtkButton * button, gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	traceWin->undo();
}

void on_traceButtonRedo_clicked(GtkButton * button, gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	traceWin->redo();
}

void on_traceButtonFlipHorizontally_clicked(GtkButton * button,
					    gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	// ***1.75 - set location of center based on slider
	if (traceWin->mScrolledWindow->allocation.height < traceWin->mImage->getNumRows())
	{
		GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(
			GTK_SCROLLED_WINDOW(traceWin->mScrolledWindow));
		double half = 0.5 * traceWin->mScrolledWindow->allocation.height;
		traceWin->mImageCenterY = (adj->value + half) / (adj->upper - adj->lower);
	}
	if (traceWin->mScrolledWindow->allocation.width < traceWin->mImage->getNumCols())
	{
		GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment(
			GTK_SCROLLED_WINDOW(traceWin->mScrolledWindow));
		double half = 0.5 * traceWin->mScrolledWindow->allocation.width;
		traceWin->mImageCenterX = (adj->value + half) / (adj->upper - adj->lower);
	}
	//***1.75 - flip desired / current center
	traceWin->mImageCenterX = 1.0 - traceWin->mImageCenterX;


	traceWin->addUndo(traceWin->mNonZoomedImage);

	ColorImage *temp = traceWin->mNonZoomedImage;

	traceWin->mNonZoomedImage = flipHorizontally(temp);

	delete temp;

	ImageMod imod(ImageMod::IMG_flip); //***1.8 - add modification to list
	traceWin->mImageMods.add(imod); //***1.8 - add modification to list

	traceWin->zoomUpdate(false);
}

void on_traceButtonFlipVertically_clicked(GtkButton * button,
					  gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	traceWin->addUndo(traceWin->mNonZoomedImage);

	ColorImage *temp = traceWin->mNonZoomedImage;

	traceWin->mNonZoomedImage = flipVertically(temp);

	delete temp;

	traceWin->zoomUpdate(false);
}


void on_traceButtonEnhanceContrast_clicked(GtkButton * button,
					   gpointer userData)
{
	if (getNumEnhanceContrastDialogReferences() > 0)
		return;

	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	// ***1.75 - set location of center based on slider
	if (traceWin->mScrolledWindow->allocation.height < traceWin->mImage->getNumRows())
	{
		GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(
			GTK_SCROLLED_WINDOW(traceWin->mScrolledWindow));
		double half = 0.5 * traceWin->mScrolledWindow->allocation.height;
		traceWin->mImageCenterY = (adj->value + half) / (adj->upper - adj->lower);
	}
	if (traceWin->mScrolledWindow->allocation.width < traceWin->mImage->getNumCols())
	{
		GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment(
			GTK_SCROLLED_WINDOW(traceWin->mScrolledWindow));
		double half = 0.5 * traceWin->mScrolledWindow->allocation.width;
		traceWin->mImageCenterX = (adj->value + half) / (adj->upper - adj->lower);
	}

	traceWin->addUndo(traceWin->mNonZoomedImage);

	EnhanceContrastDialog *dlg = new EnhanceContrastDialog(traceWin, &traceWin->mNonZoomedImage);
	dlg->show();
}


void on_traceButtonAlterBrightness_clicked(GtkButton * button,
					   gpointer userData)
{
	if (getNumAlterBrightnessDialogReferences() > 0)
		return;

	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	// ***1.75 - set location of center based on slider
	if (traceWin->mScrolledWindow->allocation.height < traceWin->mImage->getNumRows())
	{
		GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(
			GTK_SCROLLED_WINDOW(traceWin->mScrolledWindow));
		double half = 0.5 * traceWin->mScrolledWindow->allocation.height;
		traceWin->mImageCenterY = (adj->value + half) / (adj->upper - adj->lower);
	}
	if (traceWin->mScrolledWindow->allocation.width < traceWin->mImage->getNumCols())
	{
		GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment(
			GTK_SCROLLED_WINDOW(traceWin->mScrolledWindow));
		double half = 0.5 * traceWin->mScrolledWindow->allocation.width;
		traceWin->mImageCenterX = (adj->value + half) / (adj->upper - adj->lower);
	}

	traceWin->addUndo(traceWin->mNonZoomedImage);

	AlterBrightnessDialog *dlg = new AlterBrightnessDialog(traceWin, &traceWin->mNonZoomedImage);
	dlg->show();
}

void on_traceButtonDespeckle_clicked(GtkButton * button, gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	traceWin->addUndo(traceWin->mNonZoomedImage);

	ColorImage *temp = traceWin->mNonZoomedImage;

	traceWin->mNonZoomedImage = median(temp);

	delete temp;

	traceWin->zoomUpdate(false);
}

void on_traceButtonSmooth_clicked(GtkButton * button, gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	traceWin->addUndo(traceWin->mNonZoomedImage);

	ColorImage *temp = traceWin->mNonZoomedImage;

	traceWin->mNonZoomedImage = smooth(temp);

	delete temp;

	traceWin->zoomUpdate(false);
}

void on_traceButtonResize_clicked(GtkButton * button, gpointer userData)
{
	if (getNumResizeDialogReferences() > 0)
		return;

	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	traceWin->addUndo(traceWin->mNonZoomedImage);

	ResizeDialog *dlg = new ResizeDialog(traceWin, &traceWin->mNonZoomedImage);
	dlg->show();
}

//*******************************************************************
//***1.4 - new function
//
void TraceWindow::setupForLoadedFin()
{

	gtk_widget_set_sensitive(mButtonImageMod,FALSE);//**100HCI SAH
	gtk_widget_set_sensitive(mButtonImageOK,FALSE);//**100HCI SAH
	gtk_widget_set_sensitive(mButtonTraceOK,FALSE);//**100HCI SAH
	//gtk_widget_hide(mButtonImageMod); //**100HCI SAH
	//gtk_widget_hide(mButtonImageOK);//**100HCI SAH
	//gtk_widget_hide(mButtonTraceOK);//**100HCI SAH

	gtk_widget_show(mButtonTraceUnlock);
	gtk_widget_set_sensitive(mButtonTraceUnlock, TRUE);

	// set status label
	gtk_label_set_text(GTK_LABEL(mStatusLabel),
			_("Flip image and move/resize fin trace using tools below. OR unlock and retrace fin."));

	gtk_widget_show(mRadioButtonMagnify);
	//gtk_widget_show(mButtonFlipHorizontally); //Support Dropped v2.0
	//gtk_widget_show(mButtonScaleTrace);      //Support Dropped v2.0
	//gtk_widget_show(mButtonSlideTrace);

	// hide all other image processing related buttons

	gtk_widget_hide(mButtonFlipVertically);
	gtk_widget_hide(mButtonEnhanceContrast);
	gtk_widget_hide(mButtonAlterBrightness);
	gtk_widget_hide(mButtonDespeckle);
	gtk_widget_hide(mButtonSmooth);
	gtk_widget_hide(mButtonResize);
	gtk_widget_hide(mRadioButtonCrop);
	gtk_widget_hide(mRadioButtonRotate);
	gtk_widget_hide(mButtonFlipHorizontally);

	// HIDE the trace related buttons
	
	gtk_widget_hide(mRadioButtonAutoTrace);//103AT SAH
	gtk_widget_hide(mRadioButtonPencil);
	gtk_widget_hide(mRadioButtonAddPoint);
	gtk_widget_hide(mRadioButtonMovePoint);
	gtk_widget_hide(mRadioButtonEraser);
    gtk_widget_hide(mRadioButtonChopoutline);  //*** 1.5 krd

	gtk_widget_hide(mLabelPosition);

	// set active tool and cursor type

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mRadioButtonMagnify),TRUE);
	mCurTraceTool = TRACE_MAGNIFY;
	//updateCursor();

}

//*******************************************************************
//***1.4 - new function
//
void on_traceButtonSlideTrace_toggled(GtkButton * button, gpointer userData)
{

}

//*******************************************************************
//***1.4 - new function
//
gboolean 
on_traceButtonScaleTrace_changed(
		GtkRange *range,
//		GtkScrollType scroll,
//		gdouble value,
		gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return FALSE;

	//traceWin->mNormScale = value;
	traceWin->mNormScale = gtk_range_get_value(range);

	traceWin->refreshImage();

	return TRUE;
}

//*******************************************************************
//***1.4 - new function
//
// This is essentially the same as on_traceButtonTraceOK_clicked()
// but WITHOUT the call to raceFinalize() ... since the Outline
// already exists, we only want to unlock the feature points
//
void on_traceButtonTraceUnlock_clicked(GtkButton * button, gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	// button does NOTHING unless an outline and feature points exists

	if (! traceWin->mOutline)
		return;

	showError("This capability not available yet.");

	/*
	//**1.4 - this code works to a degree

	// set status label

	gtk_label_set_text(GTK_LABEL(traceWin->mStatusLabel),
			_("Reposition Feature Points Using \"Hand\" Tool Below."));

	// disable the button just clicked

	gtk_widget_set_sensitive(traceWin->mButtonTraceOK, FALSE);
	gtk_widget_hide(traceWin->mButtonTraceOK);

	// hide the image flip, trace position and scale slider tools
	gtk_widget_hide(traceWin->mButtonFlipHorizontally);
	gtk_widget_hide(traceWin->mButtonSlideTrace);
	gtk_widget_hide(traceWin->mButtonScaleTrace);

	// hide the UNDO & REDO buttons
	gtk_widget_hide(traceWin->mButtonUndo);
	gtk_widget_hide(traceWin->mButtonRedo);
	gtk_widget_set_sensitive(traceWin->mButtonUndo, FALSE);
	gtk_widget_set_sensitive(traceWin->mButtonRedo, FALSE);

	// discard the saved copies of the contour and image
	// if we go back from here, we are starting over (e.g., UNDO and REDO
	// get reinitialized
	if (NULL != traceWin->mUndoContour)
	{
		delete traceWin->mUndoContour;
		traceWin->mUndoContour = NULL;
	}
	if (NULL != traceWin->mUndoImage)
	{
		delete traceWin->mUndoImage;
		traceWin->mUndoImage = NULL;
	}


	// hide the TraceUnlock Button
	gtk_widget_hide(traceWin->mButtonTraceUnlock);
	gtk_widget_set_sensitive(traceWin->mButtonTraceUnlock, FALSE);

	// show the feature point related buttons

	gtk_widget_show(traceWin->mRadioButtonMoveFeature);
	gtk_widget_set_sensitive(traceWin->mRadioButtonMoveFeature, TRUE);

	// enable the image OK button to allow return to phase two

	gtk_widget_set_sensitive(traceWin->mButtonImageOK, TRUE);
	gtk_widget_show(traceWin->mButtonImageOK);

	// lock and finalize the trace (must be in this order)

	traceWin->mTraceLocked = false;
	traceWin->mTraceFinalized = true;
	traceWin->mFeaturePointsLocked = false;

	// set the active tool and cursor type

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(traceWin->mRadioButtonMoveFeature),TRUE);
	traceWin->mCurTraceTool = TRACE_MOVE_FEATURE;
	traceWin->updateCursor();
	*/
}


/////////////////////////////////////////////////////////////////////
// on_traceButtonImageMod_clicked:
//    calls show and hide to SHOW all image modification related
//    buttons and to re-enable further changes to image.  This 
//    essentially returns the tracing process to phase one.
//
void on_traceButtonImageMod_clicked(GtkButton * button, gpointer userData)
{

	TraceWindow *traceWin = (TraceWindow *) userData;

	// first find out where we are coming back from

	if (traceWin->mTraceLocked)
	{
		// BACKING UP: we have already locked the trace, and may even be
		// returning from the MatchingDialog.

		// show UNDO & REDO buttons, since they pertain to image modification, but disable them until a undo/redo is available
		gtk_widget_show(traceWin->mButtonUndo);//***1.9 SAH (fixed to call show instead of hide)
		gtk_widget_show(traceWin->mButtonRedo);//***1.9 SAH (fixed to call show instead of hide)
		gtk_widget_set_sensitive(traceWin->mButtonUndo, FALSE);//***1.9 SAH
		gtk_widget_set_sensitive(traceWin->mButtonRedo, FALSE);//***1.9 SAH

		// hide the feature points related buttons
		gtk_widget_hide(traceWin->mRadioButtonMoveFeature);
		//gtk_widget_hide(traceWin->mButtonTraceOK);//Hide the "Locate Features" ***1.9 SAH //100HCI SAH
		gtk_widget_set_sensitive(traceWin->mButtonTraceOK,FALSE);//100HCI SAH

		traceWin->mTraceLocked = false;
	}
	else if (traceWin->mImageLocked)
	{
		// BACKING UP: we have been in tracing mode but have not clicked
		// the "Show Feature Points" button to lock the trace

		// show UNDO & REDO buttons, since they pertain to image modification
		gtk_widget_show(traceWin->mButtonUndo);
		gtk_widget_show(traceWin->mButtonRedo);
		gtk_widget_set_sensitive(traceWin->mButtonUndo, FALSE);
		gtk_widget_set_sensitive(traceWin->mButtonRedo, FALSE);


		// HIDE the trace related buttons
		
		gtk_widget_hide(traceWin->mRadioButtonAutoTrace);//103AT SAH
		gtk_widget_hide(traceWin->mRadioButtonPencil);
		gtk_widget_hide(traceWin->mRadioButtonAddPoint);
		gtk_widget_hide(traceWin->mRadioButtonMovePoint);
		gtk_widget_hide(traceWin->mRadioButtonEraser);
		gtk_widget_hide(traceWin->mRadioButtonChopoutline); //***1.6
	}
		
	// set tatus label
	gtk_label_set_text(GTK_LABEL(traceWin->mStatusLabel),
			_("Modify Image Using Tools Below (Note: Dolphin MUST swim to your LEFT!)"));

	// disable the button just clicked

	gtk_widget_set_sensitive(traceWin->mButtonImageMod, FALSE);
	//gtk_widget_hide(traceWin->mButtonImageMod);//100HCI SAH

	// enable the image OK button so progression to phase two
	// is possible

	gtk_widget_set_sensitive(traceWin->mButtonImageOK, TRUE);
	//gtk_widget_show(traceWin->mButtonImageOK);//100HCI SAH

	// SHOW the image processing related buttons

	gtk_widget_show(traceWin->mButtonFlipHorizontally);
	//gtk_widget_show(traceWin->mButtonFlipVertically);
	gtk_widget_show(traceWin->mButtonEnhanceContrast);
	gtk_widget_show(traceWin->mButtonAlterBrightness);
	//gtk_widget_show(traceWin->mButtonDespeckle);
	//gtk_widget_show(traceWin->mButtonSmooth);
	//gtk_widget_show(traceWin->mButtonResize);
	gtk_widget_show(traceWin->mRadioButtonCrop);
	//gtk_widget_show(traceWin->mRadioButtonRotate);

	// disable buton allowing movement to phase three

	gtk_widget_set_sensitive(traceWin->mButtonTraceOK,FALSE);//100HCI SAH
	//gtk_widget_hide(traceWin->mButtonTraceOK);//100HCI SAH

	// set active tool and cursor type

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(traceWin->mRadioButtonMagnify),TRUE);
	traceWin->mCurTraceTool = TRACE_MAGNIFY;
	traceWin->updateCursor();

	traceWin->mImageLocked = false;

	traceWin->traceReset(); 
}


/////////////////////////////////////////////////////////////////////
// on_traceButtonImageOK_clicked:
//    calls show to show the trace related buttons and hide to 
//    hide all image modification related buttons and to disable 
//    further changes to image.  This essentially moves the tracing 
//    process to phase two.
//
void on_traceButtonImageOK_clicked(GtkButton * button, gpointer userData)
{

	TraceWindow *traceWin = (TraceWindow *) userData;

	// first find out where we are coming back from

	if (traceWin->mTraceLocked)
	{
		// BACKING UP: we have already locked the trace, and may even be
		// returning from the MatchingDialog.

		// show the UNDO & REDO buttons, since they pertain to contour tracing
		gtk_widget_show(traceWin->mButtonUndo);
		gtk_widget_show(traceWin->mButtonRedo);

		// hide the feature points related buttons

		gtk_widget_hide(traceWin->mRadioButtonMoveFeature);

		traceWin->mTraceLocked = false;

		traceWin->traceReset(); 
	}
	else if (! traceWin->mImageLocked)
	{
		// MOVING FORWARD: we have been in image manipulation mode (phase one)

		// disable the undo/redo buttons.  //***1.9 SAH
		//These buttons will be enabled again when trace undo/redos are available, but should no longer be available for image mod undos/redos
		gtk_widget_set_sensitive(traceWin->mButtonUndo, FALSE);
		gtk_widget_set_sensitive(traceWin->mButtonRedo, FALSE);

		// hide the image processing related buttons 

		gtk_widget_hide(traceWin->mButtonFlipHorizontally);
		gtk_widget_hide(traceWin->mButtonFlipVertically);
		gtk_widget_hide(traceWin->mButtonEnhanceContrast);
		gtk_widget_hide(traceWin->mButtonAlterBrightness);
		gtk_widget_hide(traceWin->mButtonDespeckle);
		gtk_widget_hide(traceWin->mButtonSmooth);
		gtk_widget_hide(traceWin->mButtonResize);
		gtk_widget_hide(traceWin->mRadioButtonCrop);
		gtk_widget_hide(traceWin->mRadioButtonRotate);

		traceWin->mImageLocked = true;
	}

/*	REMOVED //103AT SAH
 	g_print("PLEASE WAIT:\n   Automatically detecting rough fin outline ....\n");

	//Perform intensity trace (SCOTT) (Aug/2005) Code: 101AT
	Contour *trace = new IntensityContour(traceWin->mNonZoomedImage); //101AT --Changed IntensityContour declaration to Contour

	if (trace->length() > 100 ) {//101AT -- changed min to 100 pt contour - JHS
		g_print("   Using edge detection and active contours to refine outline placement ....\n");
		traceWin->mContour = trace;//101AT
		traceWin->traceSnapToFin();//101AT
		traceWin->refreshImage();//101AT
	} else {//101AT

		//102AT Add hooks for cyan intensity trace
		delete trace;//102AT
		trace = new IntensityContourCyan(traceWin->mNonZoomedImage);//102AT

		if (trace->length() > 100 ) {//102AT
			g_print("   Using edge detection and active contours to refine outline placement (cyan intensity image)....\n");
			traceWin->mContour = trace;//102AT
			traceWin->traceSnapToFin();//102AT
			traceWin->refreshImage();//102AT
		} else {
			ErrorDialog *errDiag = new ErrorDialog("Auto trace could not determine the fin outline.\n\nPlease trace outline by hand.");//101AT
			errDiag->show();//101AT
		}//102AT
	}//102AT*/

	// set status label

/*	Removed 103AT SAH
	gtk_label_set_text(GTK_LABEL(traceWin->mStatusLabel),
			_("Make any Needed Corrections to Outline Trace using Tools Below."));*/


	gtk_label_set_text(GTK_LABEL(traceWin->mStatusLabel),
			_("Please click the start and then end of the fin outline."));//103AT SAH


	// disable the button just clicked
	
	gtk_widget_set_sensitive(traceWin->mButtonImageOK, FALSE);
	//gtk_widget_hide(traceWin->mButtonImageOK);//100HCI SAH

	// enable the image manipulation button so return to phase one
	// is possible

	gtk_widget_set_sensitive(traceWin->mButtonImageMod, TRUE);
	//gtk_widget_show(traceWin->mButtonImageMod);//100HCI SAH

	// enable trace OK button so movement forward to phase three
	// (feture point adjustment) is possible

	gtk_widget_set_sensitive(traceWin->mButtonTraceOK, TRUE);
	//gtk_widget_show(traceWin->mButtonTraceOK);//100HCI SAH

	// show the tracing related buttons

	gtk_widget_show(traceWin->mRadioButtonAutoTrace);//103AT SAH
	gtk_widget_show(traceWin->mRadioButtonPencil);
	gtk_widget_show(traceWin->mRadioButtonAddPoint);
	gtk_widget_show(traceWin->mRadioButtonMovePoint);
	gtk_widget_show(traceWin->mRadioButtonEraser);
	gtk_widget_show(traceWin->mRadioButtonChopoutline);

	// set the active tool and cursor type

	/*
	Removed 103AT SAH
	if (trace->length() > 100 ) // AUTO trace was successful
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(traceWin->mRadioButtonChopoutline),TRUE); //***1.6
		traceWin->mCurTraceTool = TRACE_CHOPOUTLINE;
	}
	else // AUTO trace failed
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(traceWin->mRadioButtonPencil),TRUE);
		traceWin->mCurTraceTool = TRACE_PENCIL;
	}
	*/
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(traceWin->mRadioButtonAutoTrace),TRUE);//103AT SAH
	traceWin->mCurTraceTool = TRACE_AUTOTRACE;//103AT SAH
	
	traceWin->updateCursor();
}


/////////////////////////////////////////////////////////////////////
// on_traceButtonTraceOK_clicked:
//    calls show and hide to hide all image modification related
//    buttons and to prevent further changes to image.  This
//    also shows the buttons related to tracing the fin
//    outline, which is the next allowed step.
//
void on_traceButtonTraceOK_clicked(GtkButton * button, gpointer userData)
{

	TraceWindow *traceWin = (TraceWindow *) userData;

	// button does NOTHING unless a trace has actually been created

	if (! traceWin->mContour)
		return;

	// set status label

	gtk_label_set_text(GTK_LABEL(traceWin->mStatusLabel),
			_("Reposition Feature Points Using \"Hand\" Tool Below."));

	// disable the button just clicked

	gtk_widget_set_sensitive(traceWin->mButtonTraceOK, FALSE);
	//gtk_widget_hide(traceWin->mButtonTraceOK);//100HCI SAH

	// hide the tracing related buttons

	gtk_widget_hide(traceWin->mRadioButtonAutoTrace);//103AT SAH
	gtk_widget_hide(traceWin->mRadioButtonPencil);
	gtk_widget_hide(traceWin->mRadioButtonAddPoint);
	gtk_widget_hide(traceWin->mRadioButtonMovePoint);
	gtk_widget_hide(traceWin->mRadioButtonEraser);
	gtk_widget_hide(traceWin->mRadioButtonChopoutline); //***1.6

	// hide the UNDO & REDO buttons
	gtk_widget_hide(traceWin->mButtonUndo);
	gtk_widget_hide(traceWin->mButtonRedo);
	gtk_widget_set_sensitive(traceWin->mButtonUndo, FALSE);
	gtk_widget_set_sensitive(traceWin->mButtonRedo, FALSE);

	// discard the saved copies of the contour and image
	// if we go back from here, we are starting over (e.g., UNDO and REDO
	// get reinitialized
	if (NULL != traceWin->mUndoContour)
	{
		delete traceWin->mUndoContour;
		traceWin->mUndoContour = NULL;
	}
	if (NULL != traceWin->mUndoImage)
	{
		delete traceWin->mUndoImage;
		traceWin->mUndoImage = NULL;
	}


	// show the feature point related buttons

	gtk_widget_show(traceWin->mRadioButtonMoveFeature);

	// enable the image OK button to allow return to phase two

	gtk_widget_set_sensitive(traceWin->mButtonImageOK, TRUE);
	//gtk_widget_show(traceWin->mButtonImageOK);//100HCI SAH

	// lock and finalize the trace (must be in this order)

	traceWin->mTraceLocked = true;
	traceWin->traceFinalize();

	// set the active tool and cursor type

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(traceWin->mRadioButtonMoveFeature),TRUE);
	traceWin->mCurTraceTool = TRACE_MOVE_FEATURE;
	traceWin->updateCursor();
}


/////////////////////////////////////////////////////////////////////
// on_traceButtonFeaturePointsOK_clicked:
//    calls show and hide to hide all image modification related
//    buttons and to prevent further changes to image.  This
//    also shows the buttons related to tracing the fin
//    outline, which is the next allowed step.
//
void on_traceButtonFeaturePointsOK_clicked(GtkButton * button, gpointer userData)
{

	TraceWindow *traceWin = (TraceWindow *) userData;

	//gtk_widget_set_sensitive(traceWin->mButtonFeaturePointsOK, FALSE);

	traceWin->mFeaturePointsLocked = true;
}

void on_traceRadioButtonMagnify_toggled(GtkToggleButton * togglebutton,
				 gpointer userData)
{
	if (togglebutton->active) {
		TraceWindow *traceWin = (TraceWindow *) userData;

		if (NULL == traceWin)
			return;

		traceWin->mCurTraceTool = TRACE_MAGNIFY;
		traceWin->updateCursor();
	}
}

/*103AT SAH*/
void on_traceRadioButtonAutoTrace_toggled(GtkToggleButton * togglebutton,
				 gpointer userData)
{
	//g_print("AutoTrace toggled");
	if (togglebutton->active) {
		TraceWindow *traceWin = (TraceWindow *) userData;

		if (NULL == traceWin)
			return;

		traceWin->mCurTraceTool = TRACE_AUTOTRACE;
		traceWin->updateCursor();
	}
}

void on_traceRadioButtonPencil_toggled(GtkToggleButton * togglebutton,
				 gpointer userData)
{
	if (togglebutton->active) {
		TraceWindow *traceWin = (TraceWindow *) userData;

		if (NULL == traceWin)
			return;

		traceWin->mCurTraceTool = TRACE_PENCIL;
		traceWin->updateCursor();
	}
}

void on_traceRadioButtonAddPoint_toggled(GtkToggleButton * togglebutton,
				 gpointer userData)
{
	if (togglebutton->active) {
		TraceWindow *traceWin = (TraceWindow *) userData;

		if (NULL == traceWin)
			return;

		traceWin->mCurTraceTool = TRACE_ADD_POINT;
		traceWin->updateCursor();
	}
}

void on_traceRadioButtonMovePoint_toggled(GtkToggleButton * togglebutton,
				 gpointer userData)
{
	if (togglebutton->active) {
		TraceWindow *traceWin = (TraceWindow *) userData;

		if (NULL == traceWin)
			return;

		traceWin->mCurTraceTool = TRACE_MOVE_POINT;
		traceWin->updateCursor();
	}
}

//*******************************************************************
//*** 1.5 krd
//
// void on_traceRadioButtonChopoutline_toggled(...)
//
// allows user to chop a sequence of points from leading or trailing
// edge of outline with single click
//
void on_traceRadioButtonChopoutline_toggled(GtkToggleButton * togglebutton,
                                 gpointer userData)
{
        if (togglebutton->active) {
                TraceWindow *traceWin = (TraceWindow *) userData;

                if (NULL == traceWin)
                        return;

                traceWin->mCurTraceTool = TRACE_CHOPOUTLINE;
                traceWin->updateCursor();
        }
}

//*******************************************************************
//
// void on_traceRadioButtonMoveNotch_toggled(...)
//
//    allows user movement of Outline Feature points (TIP, NOTCH, etc)
//
void on_traceRadioButtonMoveFeature_toggled(GtkToggleButton * togglebutton,
				 gpointer userData)
{
	if (togglebutton->active) {
		TraceWindow *traceWin = (TraceWindow *) userData;

		if (NULL == traceWin)
			return;

		traceWin->mCurTraceTool = TRACE_MOVE_FEATURE;
		traceWin->updateCursor();
	}
}

void on_traceRadioButtonEraser_toggled(GtkToggleButton * togglebutton,
				 gpointer userData)
{
	if (togglebutton->active) {
		TraceWindow *traceWin = (TraceWindow *) userData;

		if (NULL == traceWin)
			return;

		traceWin->mCurTraceTool = TRACE_ERASER;
		traceWin->updateCursor();
	}
}

void on_traceRadioButtonCrop_toggled(GtkToggleButton * togglebutton,
				 gpointer userData)
{
	if (togglebutton->active) {
		TraceWindow *traceWin = (TraceWindow *) userData;

		if (NULL == traceWin)
			return;

		traceWin->mCurTraceTool = TRACE_CROP;
		traceWin->updateCursor();
	}
}

void on_traceRadioButtonRotate_toggled(GtkToggleButton * togglebutton,
				 gpointer userData)
{
	if (togglebutton->active) {
		TraceWindow *traceWin = (TraceWindow *) userData;

		if (NULL == traceWin)
			return;

		traceWin->mCurTraceTool = TRACE_ROTATE;
		traceWin->updateCursor();
	}
}

gboolean on_traceEventBox_button_press_event(GtkWidget * widget,
					     GdkEventButton * event,
					     gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return FALSE;

	bool shiftDown = false;

	if (event->state & GDK_SHIFT_MASK)
		shiftDown = true;

	int x, y;

	x = (int) event->x;
	y = (int) event->y;

	x -= traceWin->mZoomXOffset;
	y -= traceWin->mZoomYOffset;

	if (event->type == GDK_BUTTON_PRESS) {
		if (event->button == 1) {
			switch (traceWin->mCurTraceTool) {
				case TRACE_MAGNIFY:
					if (shiftDown)
						traceWin->zoomOut(x, y);
					else
						traceWin->zoomIn(x, y);

					return TRUE;
				case TRACE_AUTOTRACE://103AT SAH
					if (traceWin->mImageLocked && (!traceWin->mTraceLocked)) { //***051TW

						if ((NULL != traceWin->mContour) && 
							!(traceWin->mContour->length() < 2)) //If the it has 0 or 1 points, it's ok to use.
						{
							traceWin->addUndo(traceWin->mContour);
							if (getNumDeleteOutlineDialogReferences() > 0)
								return TRUE;
							DeleteOutlineDialog *dialog =
								new DeleteOutlineDialog(traceWin, &traceWin->mContour);
							dialog->show();
						}
						else
							traceWin->traceAddAutoTracePoint(x, y, shiftDown);//103AT SAH
					}
					return TRUE;
				case TRACE_PENCIL:
					if (traceWin->mImageLocked && (!traceWin->mTraceLocked)) { //***051TW

						if ((NULL != traceWin->mContour) && 
							(traceWin->mContour->length() != 0)) //***1.4TW - empty is OK to reuse
						{
							traceWin->addUndo(traceWin->mContour);
							if (getNumDeleteOutlineDialogReferences() > 0)
								return TRUE;
							DeleteOutlineDialog *dialog =
								new DeleteOutlineDialog(traceWin, &traceWin->mContour);
							dialog->show();
						}
						else
							traceWin->traceAddNormalPoint(x, y);
					}
					return TRUE;

				case TRACE_ADD_POINT:
					traceWin->traceAddExtraPoint(x, y);
					return TRUE;

				case TRACE_MOVE_POINT:
					traceWin->traceMovePointInit(x, y);
					return TRUE;

				case TRACE_MOVE_FEATURE: //***006PM new case to move Notch
					traceWin->traceMoveFeaturePointInit(x, y); //***051TW
					return TRUE;

				case TRACE_CROP:
					traceWin->cropInit(x, y);
					return TRUE;

				case TRACE_CHOPOUTLINE: //** 1.5 krd - chop outline to end of trace 
					traceWin->traceChopOutline(x, y);
					return TRUE;

				case TRACE_ROTATE:
					traceWin->rotateInit(x, y);
					return TRUE;

				default:
					break;
			}
		}
	}

	return FALSE;
}

gboolean on_traceEventBox_button_release_event(GtkWidget * widget,
					       GdkEventButton * event,
					       gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return FALSE;

	int
		x = (int) event->x,
		y = (int) event->y;

	x -= traceWin->mZoomXOffset;
	y -= traceWin->mZoomYOffset;

	switch (traceWin->mCurTraceTool) {
		case TRACE_PENCIL:
			if (! traceWin->mTraceSnapped) { //***051TW
				// it is possible for user to trace completely OUTSIDE of the image, so Contour is never created
				if (NULL == traceWin->mContour) //***1.982 - fail quietly
					return FALSE;
				//***1.4TW - shorter contours cannot be procesed without errors in following 
				// processes, so reset trace and force retrace
				if (traceWin->mContour->length() < 100)
				{
					traceWin->traceReset();
					ErrorDialog *errDialog = new ErrorDialog("This outline trace is too short to\nprocess further.  Please retrace.");
					errDialog->show();
				}
				else
				{
					int left, top, right, bottom; //***1.96
					traceWin->getViewedImageBoundsNonZoomed(left,top,right,bottom); //***1.96
					traceWin->traceSnapToFin(false, left, top, right, bottom); //***006FC,***1.96
				}
			}
			return TRUE;
		case TRACE_ADD_POINT:   // krd - treat add point as move point, erase lines
		case TRACE_MOVE_POINT:
			traceWin->traceMovePointFinalize(x, y);
			return TRUE;

		case TRACE_MOVE_FEATURE: //***006PM new case to move Notch
			traceWin->traceMoveFeaturePointFinalize(x, y); //***051TW
			return TRUE;

		case TRACE_CROP:
			traceWin->cropFinalize(x, y);
			return TRUE;

                case TRACE_CHOPOUTLINE: // *** 1.5 krd - chop outline to end of trace 
                        traceWin->traceChopOutlineFinal();
                        return TRUE;

		case TRACE_ROTATE:
			traceWin->rotateFinal(x, y);
			return TRUE;
		default:
			break;
	}

	return FALSE;
}

gboolean on_traceEventBox_motion_notify_event(GtkWidget * widget,
					      GdkEventMotion * event,
					      gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return FALSE;

    gint x, y;
    GdkModifierType state;

    if (event->is_hint)
	gdk_window_get_pointer(event->window, &x, &y, &state);

    else {
	x = (gint) event->x;
	y = (gint) event->y;
	state = (GdkModifierType) event->state;
    }

    x -= traceWin->mZoomXOffset;
    y -= traceWin->mZoomYOffset;

    char pos[100];
    sprintf(pos, "(%d, %d)", x, y);
    gtk_label_set_text(GTK_LABEL(traceWin->mLabelPosition), pos);

    traceWin->updateCursor();     // krd temp fix 10/25/05                  
	switch (traceWin->mCurTraceTool) {
		case TRACE_PENCIL:
			   traceWin->traceAddNormalPoint(x, y);
			   break;

		case TRACE_ERASER:
			   traceWin->traceErasePoint(x, y);
			   break;

		case TRACE_ADD_POINT:   // krd - treat add point as move point
		case TRACE_MOVE_POINT:
			   traceWin->traceMovePointUpdate(x, y);
			   break;

		case TRACE_MOVE_FEATURE: //***006PM new case to move Notch
			traceWin->traceMoveFeaturePointUpdate(x, y); //***051TW
			return TRUE;

		case TRACE_CROP:
			   traceWin->cropUpdate(x, y);
			   break;

		case TRACE_ROTATE:
			   traceWin->rotateUpdate(x, y);
			   break;

		default:
			    break;
	}

    return TRUE;
}

gboolean on_traceDrawingArea_expose_event(GtkWidget * widget,
					  GdkEventExpose * event,
					  gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return FALSE;

	if (NULL == traceWin->mDrawingArea || NULL == traceWin->mImage)
		return FALSE;

	traceWin->mZoomXOffset =
		(traceWin->mDrawingArea->allocation.width -
		 (int) traceWin->mImage->getNumCols()) / 2;

	traceWin->mZoomYOffset =
		(traceWin->mDrawingArea->allocation.height -
		 (int) traceWin->mImage->getNumRows()) / 2;

	// Just in case some funky allocation comes through
	if (traceWin->mZoomXOffset < 0)
		traceWin->mZoomXOffset = 0;
	if (traceWin->mZoomYOffset < 0)
		traceWin->mZoomYOffset = 0;

	//***1.4 - new values for bounds checking loaded Outline trace during positioning
	float width = traceWin->mImage->getNumCols();
	float height = traceWin->mImage->getNumRows();
	float x, y;
	gint dWidth, dHeight;
	
	gdk_drawable_get_size(GDK_DRAWABLE(traceWin->mDrawingArea->window),&dWidth,&dHeight);

	gdk_draw_rgb_image(
		traceWin->mDrawingArea->window,
		traceWin->mDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
		traceWin->mZoomXOffset, traceWin->mZoomYOffset,
		traceWin->mImage->getNumCols(),
		traceWin->mImage->getNumRows(),
		GDK_RGB_DITHER_NONE,
		(guchar*)traceWin->mImage->getData(),
		traceWin->mImage->bytesPerPixel() * traceWin->mImage->getNumCols());

	//traceWin->mZoomXOffset += (int)(traceWin->mVirtualLeft * traceWin->mZoomScale); //***1.96 - JHS
	//traceWin->mZoomYOffset += (int)(traceWin->mVirtualTop * traceWin->mZoomScale); //***1.96 - JHS

	// Now, draw the outline over the image...
	float nscale = 1.0/traceWin->mNormScale;     //***006CN, moved ***1.4
	if ((NULL != traceWin->mContour) && (traceWin->mContour->length() > 0)) { //***1.4 - new length test
		if (traceWin->mZoomRatio == 100) {
			unsigned numPoints = traceWin->mContour->length();

			for (unsigned i = 1; i < numPoints-1; i++) {         // krd -- draw first & last point in red

				x = nscale*(*traceWin->mContour)[i].x - POINT_SIZE / 2;
				y = nscale*(*traceWin->mContour)[i].y - POINT_SIZE / 2;

				if ((0.0 <= x) && ((x + POINT_SIZE) <= width) && 
					(0.0 <= y) && ((y + POINT_SIZE) <= height))
				{
				gdk_draw_rectangle(
					traceWin->mDrawingArea->window,
					traceWin->mGC,
					TRUE,
					x + traceWin->mZoomXOffset,
					y + traceWin->mZoomYOffset,
					POINT_SIZE,
					POINT_SIZE);
				}
			}

			x = nscale*(*traceWin->mContour)[0].x - POINT_SIZE / 2;
			y = nscale*(*traceWin->mContour)[0].y - POINT_SIZE / 2;

			gdk_draw_rectangle(
				traceWin->mDrawingArea->window,
				traceWin->mMovingGC,
				TRUE,
				x + traceWin->mZoomXOffset,
				y + traceWin->mZoomYOffset,
				POINT_SIZE,
				POINT_SIZE);

			x = nscale*(*traceWin->mContour)[numPoints - 1].x - POINT_SIZE / 2;
			y = nscale*(*traceWin->mContour)[numPoints - 1].y - POINT_SIZE / 2;

			gdk_draw_rectangle(
				traceWin->mDrawingArea->window,
				traceWin->mMovingGC,
				TRUE,
				x + traceWin->mZoomXOffset,
				y + traceWin->mZoomYOffset,
				POINT_SIZE,
				POINT_SIZE);


		} else {
			int zoomedPointSize = traceWin->zoomPointSize();

			unsigned numPoints = traceWin->mContour->length();

			for (unsigned i = 1; i < numPoints-1; i++) {         // krd -- draw first & last point in red

				int zoomedX = nscale*(*traceWin->mContour)[i].x; //***006CN -- nscale added
				int zoomedY = nscale*(*traceWin->mContour)[i].y; //***006CN -- nscale added

				traceWin->zoomMapPointsToZoomed(zoomedX, zoomedY);

				x = zoomedX - zoomedPointSize / 2;
				y = zoomedY - zoomedPointSize / 2;

				if ((0.0 <= x) && ((x + zoomedPointSize) <= width) && 
					(0.0 <= y) && ((y + zoomedPointSize) <= height))
				{
				gdk_draw_rectangle(
					traceWin->mDrawingArea->window,
					traceWin->mGC,
					TRUE,
					x + traceWin->mZoomXOffset,
					y + traceWin->mZoomYOffset,
					zoomedPointSize,
					zoomedPointSize);
				}
			}
			int zoomedX = nscale*(*traceWin->mContour)[0].x; //***006CN -- nscale added
			int zoomedY = nscale*(*traceWin->mContour)[0].y; //***006CN -- nscale added

			traceWin->zoomMapPointsToZoomed(zoomedX, zoomedY);
				
			x = zoomedX - zoomedPointSize / 2;
			y = zoomedY - zoomedPointSize / 2;

			if ((0.0 <= x) && ((x + POINT_SIZE) <= width) && 
				(0.0 <= y) && ((y + POINT_SIZE) <= height))
			{
			gdk_draw_rectangle(
				traceWin->mDrawingArea->window,
				traceWin->mMovingGC,
				TRUE,
				x + traceWin->mZoomXOffset,
				y + traceWin->mZoomYOffset,
				zoomedPointSize,
				zoomedPointSize);
			}

            zoomedX = nscale*(*traceWin->mContour)[numPoints-1].x; //***006CN -- nscale added
			zoomedY = nscale*(*traceWin->mContour)[numPoints-1].y; //***006CN -- nscale added

			traceWin->zoomMapPointsToZoomed(zoomedX, zoomedY);
			
			x = zoomedX - zoomedPointSize / 2;
			y = zoomedY - zoomedPointSize / 2;
			
			if ((0.0 <= x) && ((x + POINT_SIZE) <= width) && 
				(0.0 <= y) && ((y + POINT_SIZE) <= height))
			{
			gdk_draw_rectangle(
				traceWin->mDrawingArea->window,
				traceWin->mMovingGC,
				TRUE,
				x + traceWin->mZoomXOffset,
				y + traceWin->mZoomYOffset,
				zoomedPointSize,
				zoomedPointSize);
			}
		}
		//***006PD code to draw all key points along fin outline (JHS JHS)
		if (traceWin->mTraceFinalized) {
			int highlightPointSize = 4 * POINT_SIZE;
			if (traceWin->mZoomRatio != 100)
				highlightPointSize = 4 * traceWin->zoomPointSize();
			int xC, yC;
			point_t p;

			p = traceWin->mOutline->getFeaturePointCoords(TIP); //***008OL
			xC = (int) round(nscale*p.x); //***006CN -- nscale added
			yC = (int) round(nscale*p.y); //***006CN -- nscale added
			if (traceWin->mZoomRatio != 100)
				traceWin->zoomMapPointsToZoomed(xC, yC);

			x = xC - highlightPointSize / 2;
			y = yC - highlightPointSize / 2;

			if ((0.0 <= x) && ((x + highlightPointSize) <= width) && 
				(0.0 <= y) && ((y + highlightPointSize) <= height))
			{
				gdk_draw_rectangle(
					traceWin->mDrawingArea->window,
					traceWin->mGC,
					TRUE,
					x + traceWin->mZoomXOffset,
					y + traceWin->mZoomYOffset,
					highlightPointSize,
					highlightPointSize);
			}

			p = traceWin->mOutline->getFeaturePointCoords(NOTCH); //***008OL
			xC = (int) round(nscale*p.x); //***006CN -- nscale added
			yC = (int) round(nscale*p.y); //***006CN -- nscale added
			if (traceWin->mZoomRatio != 100)
				traceWin->zoomMapPointsToZoomed(xC, yC);

			x = xC - highlightPointSize / 2;
			y = yC - highlightPointSize / 2;

			if ((0.0 <= x) && ((x + highlightPointSize) <= width) && 
				(0.0 <= y) && ((y + highlightPointSize) <= height))
			{
				gdk_draw_rectangle(
					traceWin->mDrawingArea->window,
					traceWin->mGC,
					TRUE,
					x + traceWin->mZoomXOffset,
					y + traceWin->mZoomYOffset,
					highlightPointSize,
					highlightPointSize);
			}

			p = traceWin->mOutline->getFeaturePointCoords(POINT_OF_INFLECTION); //***008OL
			xC = (int) round(nscale*p.x); //***006CN -- nscale added
			yC = (int) round(nscale*p.y); //***006CN -- nscale added
			if (traceWin->mZoomRatio != 100)
				traceWin->zoomMapPointsToZoomed(xC, yC);

			x = xC - highlightPointSize / 2;
			y = yC - highlightPointSize / 2;

			if ((0.0 <= x) && ((x + highlightPointSize) <= width) && 
				(0.0 <= y) && ((y + highlightPointSize) <= height))
			{
				gdk_draw_rectangle(
					traceWin->mDrawingArea->window,
					traceWin->mGC,
					TRUE,
					x + traceWin->mZoomXOffset,
					y + traceWin->mZoomYOffset,
					highlightPointSize,
					highlightPointSize);
			}

			p = traceWin->mOutline->getFeaturePointCoords(LE_BEGIN); //***008OL
			xC = (int) round(nscale*p.x); //***006CN -- nscale added
			yC = (int) round(nscale*p.y); //***006CN -- nscale added
			if (traceWin->mZoomRatio != 100)
				traceWin->zoomMapPointsToZoomed(xC, yC);

			x = xC - highlightPointSize / 2;
			y = yC - highlightPointSize / 2;

			if ((0.0 <= x) && ((x + highlightPointSize) <= width) && 
				(0.0 <= y) && ((y + highlightPointSize) <= height))
			{
				gdk_draw_rectangle(
					traceWin->mDrawingArea->window,
					traceWin->mGC,
					TRUE,
					x + traceWin->mZoomXOffset,
					y + traceWin->mZoomYOffset,
					highlightPointSize,
					highlightPointSize);
			}

			//***1.8 - no longer draw LE_END
			/*
			p = traceWin->mOutline->getFeaturePointCoords(LE_END); //***008OL
			xC = (int) round(nscale*p.x); //***006CN -- nscale added
			yC = (int) round(nscale*p.y); //***006CN -- nscale added
			if (traceWin->mZoomRatio != 100)
				traceWin->zoomMapPointsToZoomed(xC, yC);

			x = xC - highlightPointSize / 2;
			y = yC - highlightPointSize / 2;

			if ((0.0 <= x) && ((x + highlightPointSize) <= width) && 
				(0.0 <= y) && ((y + highlightPointSize) <= height))
			{
				gdk_draw_rectangle(
					traceWin->mDrawingArea->window,
					traceWin->mGC,
					TRUE,
					x + traceWin->mZoomXOffset,
					y + traceWin->mZoomYOffset,
					highlightPointSize,
					highlightPointSize);
			}
			*/
		} //***006PD end of key point drawing addition
			
	}
		
	//traceWin->mZoomXOffset -= traceWin->mVirtualLeft; //***1.96 - JHS
	//traceWin->mZoomYOffset -= traceWin->mVirtualTop; //***1.96 - JHS

	return TRUE;
}

//***008OL
/* this function has been rewritten to normalize based on distance
** from midpoint of baseline (1st point to last point) and the
** middle point on the Contour (near the tip).  This is approximate
** but should give a good enough normalization for following
** conputations. -- JHS
//////////////////////////////////////////////////////////////////////////
// normalizeContour: called to scale Contour up or down to standard
//    size (600 units from beginning of leading edge to tip of fin.
//
float normalizeContour(Contour *c){

  Chain *temp = new Chain(c,3.0);

  int tipPos = findTip(temp);
  int beginLE = findLECutoff(temp,tipPos);
  point_t
		//tipPosPoint = temp->getSavedPoint(tipPos), removed
    //beginLEPoint = temp->getSavedPoint(beginLE); removed
    tipPosPoint = traceWin->mOutline->getFeaturePointCoords(LE_BEGIN), //***008OL
    beginLEPoint = traceWin->mOutline->getFeaturePointCoords(LE_END); //***008OL


	delete temp;

  // Get X,Y Position of tip and LE
  float tipx = tipPosPoint.x , tipy = tipPosPoint.y;
  float LEx = beginLEPoint.x , LEy  = beginLEPoint.y;
  float distance, factor;

  // Calculate distance
  distance = sqrt((tipx - LEx) * (tipx - LEx)
    + ((tipy - LEy) * (tipy - LEy)));

	// Compute rescaling factor
  factor = 600/distance;

  Contour_node_t *cur = c->getHead();

  int length = c->length(), i ;

  for( i = 0; i< length ; i++) {
    cur->data.x = (int) (( cur->data.x * factor)+0.5);
    cur->data.y = (int) ((cur->data.y * factor)+0.5);
    cur = cur->next;
  }

	return factor; //***006NC

}
//***008OL end of deleted function
*/

//////////////////////////////////////////////////////////////////////////
// normalizeContour: called to scale Contour up or down to standard
//    size (600 units from beginning of leading edge to tip of fin.
//
//***008OL new version of function
//
float normalizeContour(Contour *c){

  // Get X,Y Position of pseudo tip (middle point on Contour)
  float tipx = (*c)[c->length()/2].x, tipy = (*c)[c->length()/2].y;  
  float basex = 0.5 * ((*c)[0].x + (*c)[c->length()-1].x), 
        basey = 0.5 * ((*c)[0].y + (*c)[c->length()-1].y);
  float distance, factor;

  // Calculate distance
  float dx = tipx - basex, dy = tipy - basey;
  distance = sqrt(dx * dx + dy * dy);

	// Compute rescaling factor
  factor = 600/distance;

  Contour_node_t *cur = c->getHead();

  int length = c->length(), i ;

  for( i = 0; i< length ; i++) {
    cur->data.x = (int) (( cur->data.x * factor)+0.5);
    cur->data.y = (int) ((cur->data.y * factor)+0.5);
    cur = cur->next;
  }

	return factor; //***006NC

}

//*******************************************************************
//
// This exports the unknwn fin's sighting data and image filenames to a
// <tab>-separated text file that is a master sighting data file for the given
// SurveyArea.
//
void on_traceButtonDumpData_clicked(
		GtkButton * button, 
		gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	// get damage category id
	int damageIDnum = gtk_combo_box_get_active(GTK_COMBO_BOX(traceWin->mEntryDamage));

	string dbName = traceWin->mOptions->mDatabaseFileName;
	dbName = dbName.substr(dbName.rfind(PATH_SLASH)+1);
	dbName = dbName.substr(0,dbName.rfind('.'));

	string surveyArea = traceWin->mOptions->mCurrentSurveyArea;
	string areaName = surveyArea.substr(surveyArea.rfind(PATH_SLASH)+1);

	string dataFilename = surveyArea + PATH_SLASH + "sightings" + PATH_SLASH;
	dataFilename += "SightingDataLogForArea_";
	dataFilename += areaName + ".txt";

	string
		id = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryID)),
		name = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryName)),
		date = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryDate)),
		roll = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryRoll)),
		location = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryLocation)),
		description = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryDescription));

	// cannot use gtk_combo_box_get_active_text() until we upgrade to GTK 2.6+
	string damage = "";
	if (damageIDnum != -1)
		damage = traceWin->mDatabase->catCategoryName(damageIDnum); //***051

	//***1.65 - use ID from mFin instead if hiding ID's
	if ((NULL != traceWin->mFin) && (traceWin->mOptions->mHideIDs))
	{
		id = traceWin->mFin->mIDCode;
	}

	if ("" == id)
		id = "NONE";
	if ("" == name)
		name = "NONE";
	if ("" == date)
		date = "NONE";
	if ("" == roll)
		roll = "NONE";
	if ("" == location)
		location = "NONE";
	if ("" == damage)
		damage = "NONE";
	if ("" == description)
		description = "NONE";

	//***1.8 - the name of the ORIGINAL image may be in one of two places now
	string 
		origName,
		modName;

	if (NULL == traceWin->mFin)
	{
		modName = "NONE";
		origName = traceWin->mImagefilename;
	}
	else
	{
		modName = traceWin->mImagefilename;
		origName = traceWin->mFin->mOriginalImageFilename;
	}

	if ("" == origName) // OLD fin file and original name is lost, not in PPM file
		origName = traceWin->mImagefilename; // so use the only name we have

	// just use short filenames (strip paths)

	modName = modName.substr(modName.rfind(PATH_SLASH)+1);
	origName = origName.substr(origName.rfind(PATH_SLASH)+1);

	//***1.98 - just in case someone PASTES <CR><LF> characters in the
	// data entry areas - convert them to spaces so they don't corrupt the database
	stripCRLF(id);
	stripCRLF(name);
	stripCRLF(date);
	stripCRLF(roll);
	stripCRLF(location);
	stripCRLF(damage);
	stripCRLF(description);
	stripCRLF(modName);
	stripCRLF(origName);

	cout 
		<< id  << "\t"
		<< name << "\t"
		<< date << "\t"
		<< roll << "\t"
		<< location << "\t"
		<< damage << "\t"
		<< description << "\t"
		<< origName << "\t"
		<< modName << endl;

	ofstream outFile;
	outFile.open(dataFilename.c_str(),ios_base::out | ios_base::app);

	outFile 
		<< id  << "\t"
		<< name << "\t"
		<< date << "\t"
		<< roll << "\t"
		<< location << "\t"
		<< damage << "\t"
		<< description << "\t"
		<< origName << "\t"
		<< modName << endl;

	outFile.close();

	gtk_widget_set_sensitive(GTK_WIDGET(button),false);
}

//*******************************************************************
//
//
//
//
//
void on_traceButtonMatch_clicked(GtkButton * button, gpointer userData)
{
	if (getNumMatchingDialogReferences() > 0)
		return;

	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	//***1.4 - Outline may exist without Contour if fin traced loaded rather than image
	if ((NULL == traceWin->mContour) && (NULL == traceWin->mOutline)) {
		showError("You must trace your fin\nbefore it may be matched.");
		return;
	}

	// ***055 - must choose a category before attempting match, can be revised later
	int damageIDnum = gtk_combo_box_get_active(GTK_COMBO_BOX(traceWin->mEntryDamage));
	if (-1 == damageIDnum)
	{
		showError("You must select a Catalog Category\nBEFORE attempting a match!");
		return;
	}

	//***008OL added the following in case Contour is done but not finalized yet
	if (NULL == traceWin->mOutline) {
		traceWin->mTraceLocked=true;
		traceWin->traceFinalize();
	}

	if (traceWin->mDatabase->isEmpty()) {
		// Well, the database is empty, so there's no point in
		// trying to match this fin, so ask the user if he or
		// she would just like to add it to the database.

		traceWin->mQuestionDialog = traceWin->createQuestionDialog();
		gtk_widget_show(traceWin->mQuestionDialog);
		return;
	}

	string
		id = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryID)),
		name = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryName)),
		date = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryDate)),
		roll = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryRoll)),
		location = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryLocation)),
		description = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryDescription));

	// cannot use gtk_combo_box_get_active_text() until we upgrade to GTK 2.6+
	//int damageIDnum = gtk_combo_box_get_active(GTK_COMBO_BOX(traceWin->mEntryDamage)); //***051
	string damage = "";
	if (damageIDnum != -1)
		damage = traceWin->mDatabase->catCategoryName(damageIDnum); //***051

	//***1.65 - use ID from mFin instead if hiding ID's
	if ((NULL != traceWin->mFin) && (traceWin->mOptions->mHideIDs))
	{
		id = traceWin->mFin->mIDCode;
		//g_print(id.c_str());
	}

	if ("" == id)
		id = "NONE";
	if ("" == name)
		name = "NONE";
	if ("" == date)
		date = "NONE";
	if ("" == roll)
		roll = "NONE";
	if ("" == location)
		location = "NONE";
	if ("" == damage)
		damage = "NONE";
	if ("" == description)
		description = "NONE";

	//***1.98 - just in case someone PASTES <CR><LF> characters in the
	// data entry areas - convert them to spaces so they don't corrupt the database
	stripCRLF(id);
	stripCRLF(name);
	stripCRLF(date);
	stripCRLF(roll);
	stripCRLF(location);
	stripCRLF(damage);
	stripCRLF(description);

   //***008OL step below is redundant now - happens in traceFinalize()
	//traceWin->mNormScale = normalizeContour(traceWin->mContour); //***006CM

	//***1.8 - the name of the ORIGINAL image may be in one of two places now
	string imageFilename;
	if (NULL == traceWin->mFin)
		imageFilename = traceWin->mImagefilename;
	else
		imageFilename = traceWin->mFin->mOriginalImageFilename;

	if ("" == imageFilename) // OLD fin file and original name is lost, not in PPM file
		imageFilename = traceWin->mImagefilename; // so use the only name we have

	//***1.8 - the various data are assumed to be passed AS PART OF A DATABASEFIN
	//         through the matching process to the match results and beyond in this fashion
	//
	// newFin->mImagefilename 
	//    is the name of the MODIFIED image file if one exists
	//    otherwise it is the name of the ORIGINAL image file
	// newFin->mFinImage 
	//    is the ORIGINAL image
	// newFin->mModifiedImage
	//    is the MODIFIED image
	// newFin->mOriginalImageFilename 
	//    is the name of the ORIGINAL image
	// newFin->mImageMods
	//    is the list of modifications made to the ORIGINAL image
	//    to produce the MODIFIED image
	//

	DatabaseFin<ColorImage> *newFin = new DatabaseFin<ColorImage>(
			imageFilename, //***1.8
			//traceWin->mImagefilename,   //***001DB
			//traceWin->mContour->evenlySpaceContourPoints(3.0), //***006CM
			traceWin->mOutline, //***008OL
			id,
			name,
			date,
			roll,
			location,
			damage,
			description
			);

	// at this point the CONSTRUCTOR above has created the mFinImage 
	// and the mImageFilename has been set to the ORIGINAL image filename

	// if fin file has been saved, set the fin file name in newFin so it is passed on
	if (traceWin->mSavedFinFilename != "")
		newFin->mFinFilename = traceWin->mSavedFinFilename;

	//***1.5 - copy of image with any modifications, to be passed on during process
	//***1.8   NOTE: if we ever alow UNLOCKING of previously loaded fin OR
	//         allow additional modifications to fin image or trace AFTER a save
	//         then this code will not work !!!!!!!!!
	newFin->mModifiedFinImage = new ColorImage(traceWin->mNonZoomedImage); //***1.5
	newFin->mModifiedFinImage->mNormScale = traceWin->mNormScale; //***1.5
	if (NULL == traceWin->mFin) //***1.8
	{
		//***1.8 - copy the image mod list & original image filename
		//         modified image has NOT been saved yet
		newFin->mImageMods = traceWin->mImageMods;
		newFin->mModifiedFinImage->mImageMods = traceWin->mImageMods; //***1.96a
		newFin->mOriginalImageFilename = imageFilename;
	}
	else
	{
		//***1.8 - copy the image mod list & both image filenames
		//         modified image HAS ALREADY been saved
		newFin->mImageMods = traceWin->mFin->mImageMods;
		newFin->mImageFilename = traceWin->mFin->mImageFilename;
		newFin->mOriginalImageFilename = traceWin->mFin->mOriginalImageFilename;
	}

	//***1.9 - check validity of image mods read from file
	if (0 < newFin->mImageMods.size())
	{
		ImageMod mod(ImageMod::IMG_none);

		ImageMod::ImageModType op;
		int v1, v2, v3, v4;

		newFin->mImageMods.first(mod);
		mod.get(op, v1, v2, v3, v4);
		cout << "ImgMod: " << (int)op << " " << v1 << " " << v2 << " " << v3 << " " << v4 << endl;

		while (newFin->mImageMods.next(mod))
		{
			mod.get(op, v1, v2, v3, v4);
			cout << "ImgMod: " << (int)op << " " << v1 << " " << v2 << " " << v3 << " " << v4 << endl;
		}
	}





	MatchingDialog *dlg = new MatchingDialog(
			newFin,
			traceWin->mDatabase,
			traceWin->mMainWin,		//***004CL
			traceWin->mOptions);
	
	// don't want to delete newFin here.
	delete traceWin;

	dlg->show(false); // first time
}

//*******************************************************************
//
//
//
//
//
void on_traceButtonSave_clicked(GtkButton * button, gpointer userData)
{
	if (getNumSaveFileChooserDialogReferences() >= 1)
		return;

	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	if ((NULL == traceWin->mContour) && (NULL == traceWin->mOutline)) { //***1.4
		showError(ERROR_MSG_NO_CONTOUR);
		return;
	}

	//if (NULL != traceWin->mFin) { //***1.65
	if (NULL != traceWin->mFinWasLoaded) { //***1.95
		showError("Save Fin option is NOT available for previously loaded fin file!");
		return;
	}

	if (NULL != traceWin->mFin) //***2.0
	{
		// this means the Fin Trace was already saved, or was at least
		// created as a DatabaseFin and was passed to the SaveFileChooserDialog,
		// so if we are re-saving, delete the old fin and start over
		delete traceWin->mFin;
		traceWin->mFin = NULL;
	}

	//***008OL added the following in case Contour is done but not finalized yet
	if (NULL == traceWin->mOutline) {
		traceWin->mTraceLocked=true;
		traceWin->traceFinalize();
	}

#ifdef TIMING_ENABLED
	//***1.95 - timing check
	endTimerValue = clock();
	duration = (double)(endTimerValue - startTimerValue) / CLOCKS_PER_SEC;
	double size = traceWin->mImage->mRows * traceWin->mImage->mCols * 3;
	g_print("File: %s TraceStart: %d TraceStop: %d TraceTime: %5.2f seconds\n",
		traceWin->mImagefilename.substr(traceWin->mImagefilename.rfind(PATH_SLASH)+1).c_str(), 
		startTimerValue, endTimerValue, duration);
	timingOutFile << "File: " 
		<< traceWin->mImagefilename.substr(traceWin->mImagefilename.rfind(PATH_SLASH)+1)
		<< " ImageSize: " << fixed << setprecision(2) << size / 1024 << " Mb "
		<< " TraceStart: " << startTimerValue
		<< " TraceStop: " << endTimerValue
		<< " TraceTime: " << fixed << setprecision(2) << duration 
		<< " seconds\n";
	//***1.95 - end timing check
#endif

	string
		id = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryID)),
		name = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryName)),
		date = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryDate)),
		roll = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryRoll)),
		location = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryLocation)),
		description = gtk_entry_get_text(GTK_ENTRY(traceWin->mEntryDescription));

	// cannot use gtk_combo_box_get_active_text() until we upgrade to GTK 2.6+
	int damageIDnum = gtk_combo_box_get_active(GTK_COMBO_BOX(traceWin->mEntryDamage)); //***051
	string damage = "";
	if (damageIDnum != -1)
		damage = traceWin->mDatabase->catCategoryName(damageIDnum); //***051

	if ("" == id)
		id = "NONE";
	if ("" == name)
		name = "NONE";
	if ("" == date)
		date = "NONE";
	if ("" == roll)
		roll = "NONE";
	if ("" == location)
		location = "NONE";
	if ("" == damage)
		damage = "NONE";
	if ("" == description)
		description = "NONE";

	//***1.98 - just in case someone PASTES <CR><LF> characters in the
	// data entry areas - convert them to spaces so they don't corrupt the database
	stripCRLF(id);
	stripCRLF(name);
	stripCRLF(date);
	stripCRLF(roll);
	stripCRLF(location);
	stripCRLF(damage);
	stripCRLF(description);

	//***1.1 - removed line, not needed, as it happens in TraceFinalize() call above
	//traceWin->mNormScale = normalizeContour(traceWin->mContour); //***006CM

	///////////////////

	//***1.1 - saved fin traces now go in a standard location, but this will
	// be handled in the SaveFileSelectionDialog class and the DatabaseFin class
	// when the file is actually saved

	//DatabaseFin<ColorImage> *newFin = new DatabaseFin<ColorImage>(
	traceWin->mFin = new DatabaseFin<ColorImage>( //***1.8
			traceWin->mImagefilename,   //***001DB
			//copyFilename, //***1.1 - path gets stripped later in DatabaseFin::save(...)
			//traceWin->mContour->evenlySpaceContourPoints(3.0), //***006CM removed
			traceWin->mOutline, //***008OL
			id,
			name,
			date,
			roll,
			location,
			damage,
			description
			);

	//***1.5 - pass along nonZoomed, but modified image to be saved along with fin file
	/*newFin*/traceWin->mFin->mModifiedFinImage = new ColorImage(traceWin->mNonZoomedImage); //***1.5 
	/*newFin*/traceWin->mFin->mModifiedFinImage->mNormScale = traceWin->mNormScale; //***1.5

	/*newFin*/traceWin->mFin->mImageMods = traceWin->mImageMods; //***1.8 - copy the image mod list

	// the original filename must match, just in case the SAVE is aborted
	// in which case the traceWin->mFin already exists and this will not
	// be built prior to any attempted Match -- it is important that the
	// original image filename be set BEFORE any match is attmepted
	traceWin->mFin->mOriginalImageFilename = traceWin->mImagefilename; //***1.98

	SaveFileChooserDialog *dlg = new SaveFileChooserDialog(
			traceWin->mDatabase,
			/*newFin*/traceWin->mFin, 
			NULL, // no MainWindow
			traceWin,
			traceWin->mOptions,
			traceWin->mWindow,
			SaveFileChooserDialog::saveFin);

	// note that we don't want to delete newFin here...the save dialog does it
	
	dlg->run_and_respond();
}

//*******************************************************************
// string copyImageToDestinationAs(string srcName, string destPath)
//
// This copies the source Image from (srcFile) to an image file in the
// destination folder (destPath) appending a number to the root image
// filename COPY such that no existing image is destroyed in the
// destination folder.  The name of the new image file COPY in the
// destination floder is returned.
//
// (srcName) should be the complete path/name of the source image file
// (destPath) should be a slash terminated, existing folder
//
string TraceWindow::copyImageToDestinationAs(string srcName, string destPath)
{
	string shortFilename = srcName;
	int pos = shortFilename.rfind(PATH_SLASH);
	if (pos >= 0)
	{
		shortFilename = shortFilename.substr(pos+1);
	}

	// prevent copying OVER existing image file inside catalog

	pos = shortFilename.rfind('.');
	string rootName = shortFilename.substr(0,pos);
	string extension = shortFilename.substr(pos);

	// if no path specified, then fail quietly and return no destination filename
	if (destPath.size() ==0)
		return "";

	// make sure destination path ends in slash
	if (destPath.rfind(PATH_SLASH) != destPath.size()-1)
		return destPath += PATH_SLASH;

	string destName = destPath + shortFilename;

	// this loop prevents overwriting previous fin and associated images
	int i = 1;
	char num[8];
	ifstream infile;
	//printf("Checking: %s ", destName.c_str());
	infile.open(destName.c_str());
	while (! infile.fail())
	{
		infile.close();
		//printf(" - file exists.\n");
		i++;
		sprintf(num,"[%d]",i);
		destName = destPath + rootName + num + extension;
		//printf("Checking: %s ", destName.c_str());
		infile.open(destName.c_str());
	}

	printf("copying \"%s\" to destination folder\n",shortFilename.c_str());
	printf("     as \"%s\"\n",destName.c_str());

	// copy image over into catalog folder

#ifdef WIN32
	string command = "copy \"";
#else
	string command = "cp \"";
#endif
	command += srcName;
	command += "\" \"";
	command += destName;
	command += "\"";

#ifdef DEBUG
	printf("copy command: \"%s\"",command.c_str());
#endif
	system(command.c_str());

	return destName;
}

//*******************************************************************
// string TraceWindow::nextAvailableDestinationFilename(string destName)
//
// This finds an available name for the new image to be saved,
// by appending a number to the root image filename COPY such that 
// no existing image will be destroyed in the destination folder.  
// The available name of the new image file COPY in the destination 
// folder is returned.
//
// (srcName) should be the complete path/name of the proposed NEW image file
//
string TraceWindow::nextAvailableDestinationFilename(string destName)
{
	int pos = destName.rfind('.');
	string rootName = destName.substr(0,pos); // path and root filename
	string extension = destName.substr(pos);  // just the extension (with dot)

	string availName = destName; // first try (as is)

	// this loop prevents overwriting previous fin and associated images
	int i = 1;
	char num[8];
	ifstream infile;
	//printf("Checking: %s ", destName.c_str());
	infile.open(availName.c_str());
	while (! infile.fail())
	{
		infile.close();
		//printf(" - file exists.\n");
		i++;
		sprintf(num,"[%d]",i);
		availName = rootName + num + extension;
		//printf("Checking: %s ", destName.c_str());
		infile.open(availName.c_str());
	}
	return availName;
}

//*******************************************************************
//
//
//
//
//
void on_traceButtonAddToDatabase_clicked(GtkButton * button,
					 gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	if (NULL == traceWin->mContour) {
		showError(ERROR_MSG_NO_CONTOUR);
		return;
	}
	
	//***008OL added the following in case Contour is done but not finalized yet
	if (NULL == traceWin->mOutline) {
		traceWin->mTraceLocked=true;
		traceWin->traceFinalize();
	}

	try {
		string
			id,
			name,
			date,
			roll,
			location,
			damage,
			description;

		gchar *temp;

		//***1.65 - use ID from mFin instead if hiding ID's
		if ((NULL != traceWin->mFin) && (traceWin->mOptions->mHideIDs))
		{
			id = traceWin->mFin->mIDCode;
			//g_print(id.c_str());
		}
		else
		{
			//g_print(traceWin->mFin->mIDCode.c_str());
			temp = gtk_editable_get_chars(
					GTK_EDITABLE(traceWin->mEntryID),
					0, -1);
			id = temp;
			g_free(temp);
		}

		temp = gtk_editable_get_chars(
				GTK_EDITABLE(traceWin->mEntryName),
				0, -1);
		name = temp;
		g_free(temp);

		temp = gtk_editable_get_chars(
				GTK_EDITABLE(traceWin->mEntryDate),
				0, -1);
		date = temp;
		g_free(temp);

		temp = gtk_editable_get_chars(
				GTK_EDITABLE(traceWin->mEntryRoll),
				0, -1);
		roll = temp;
		g_free(temp);

		temp = gtk_editable_get_chars(
				GTK_EDITABLE(traceWin->mEntryLocation),
				0, -1);
		location = temp;
		g_free(temp);

		// cannot use gtk_combo_box_get_active_text() until we upgrade to GTK 2.6+
		int damageIDnum = gtk_combo_box_get_active(GTK_COMBO_BOX(traceWin->mEntryDamage)); //***051
		damage = "";
		if (damageIDnum != -1)
			damage = traceWin->mDatabase->catCategoryName(damageIDnum); //***051

		temp = gtk_editable_get_chars(
				GTK_EDITABLE(traceWin->mEntryDescription),
				0, -1);
		description = temp;
		g_free(temp);

		if ("" == id)
		{
			showError (ERROR_MSG_NO_IDCODE);
			return;
		}
		if (-1 == damageIDnum)
		{
			showError ("You must select a Catalog Category\nBEFORE attempting a match!");
			return;

		}
		if ("" == name)
			name = "NONE";
		if ("" == date)
			date = "NONE";
		if ("" == roll)
			roll = "NONE";
		if ("" == location)
			location = "NONE";
		if ("" == damage)
			damage = "NONE";
		if ("" == description)
			description = "NONE";

		//***1.98 - just in case someone PASTES <CR><LF> characters in the
		// data entry areas - convert them to spaces so they don't corrupt the database
		stripCRLF(id);
		stripCRLF(name);
		stripCRLF(date);
		stripCRLF(roll);
		stripCRLF(location);
		stripCRLF(damage);
		stripCRLF(description);

		//traceWin->mNormScale = normalizeContour(traceWin->mContour); //***006CM,***1.1

		//***054 - copy unknown image to catalog and use short filename in database

		if (NULL == traceWin->mFin)
		{
			//***1.8 - this is the way to do it if the FIN has never been
			// loaded from file NOR has it been saved to file ...
			// in this case, the mImagefilename is the long name of the
			// ORIGINALLY loaded image and the MODIFIED image will have to be
			// created and saved in the catalog folder alongside the copy
			// of the original image

			//***19.6a - start new
			// make sure COPY of original image does NOT OVERWRITE
			// an existing catalog image with the same name
			string catalogPath = gOptions->mCurrentSurveyArea; //***1.96a
			catalogPath += PATH_SLASH;
			catalogPath += "catalog";
			catalogPath += PATH_SLASH;

			string copyFilename = traceWin->copyImageToDestinationAs( //***1.96a
					traceWin->mImagefilename,
					catalogPath);
			string copyShortFilename = copyFilename.substr(copyFilename.rfind(PATH_SLASH)+1);
			//***1.96a - end new

			//***1.8 - save modified image alongside original
			//
			string modFilename;
			if (NULL != traceWin->mImage)
			{
				// create filename with modified image extension
				int pos = copyFilename.rfind('.'); //***1.96a
				string modRootName = copyFilename.substr(0,pos);
				modRootName += "_wDarwinMods";
				modFilename = modRootName + ".png";

				modFilename = traceWin->nextAvailableDestinationFilename(modFilename); //***1.96a

				//***1.85 & 1.9 - MUST set scale before saving
				traceWin->mNonZoomedImage->mNormScale = traceWin->mNormScale;

				// save modified image
				traceWin->mNonZoomedImage->save_wMods( //***1.8 - new save modified image call
						modFilename,    // the filename of the modified image
						copyShortFilename,   //***1.96a - the filename of the original image
						traceWin->mImageMods); // the list of image modifications
			}

			//***1.8 - now we are saving the filename of the modified image as part
			//         of the database entry.  We will look in the comments of the
			//         modified file to retrieve the name of the original image file
			//         so that it can be retrieved as needed
	
			DatabaseFin<ColorImage> *newFin = new DatabaseFin<ColorImage>(
					modFilename, //***1.8 - save modified image name now
					traceWin->mOutline, //***008OL
					id,
					name,
					date,
					roll,
					location,
					damage,
					description);

			//***1.8 - pass along nonZoomed, but modified image, scaleChange, & image mod list
			newFin->mModifiedFinImage = new ColorImage(traceWin->mNonZoomedImage);
			newFin->mOriginalImageFilename = copyShortFilename; //***1.96a
			newFin->mModifiedFinImage->mNormScale = traceWin->mNormScale;
			newFin->mImageMods = traceWin->mImageMods;
			//***1.8 - end of new code section

			traceWin->mDatabase->add(newFin);

			delete newFin;

			traceWin->mMainWin->refreshDatabaseDisplayNew(true); //***1.96a
			traceWin->mMainWin->selectFromReorderedCList(modFilename); //***1.8
		}
		else // traceWin->mFin DOES exist
		{
			// so now the fin file, the modified image, and a copy of the
			// original image ALL exist in the tracedFins folder.
			// in this case, these images are simply copied to the catalog
			// before the FIN is saved to the database

			string catalogPath = gOptions->mCurrentSurveyArea; //***1.85
			catalogPath += PATH_SLASH;
			catalogPath += "catalog";
			catalogPath += PATH_SLASH;

			// first copy the ORIGINAL image to the catalog
			// NOTE: if the original image name is empty then the fin file was
			// saved with the older PPM image file and the original image filename
			// has been lost

			//***1.96a - new code segment
			string 
				copyFilename(""), 
				copyShortFilename(""),
				originalShortFilename("");
			
			if ("" != traceWin->mFin->mOriginalImageFilename)
			{
				originalShortFilename = traceWin->mFin->mOriginalImageFilename;
				originalShortFilename = originalShortFilename.substr(
						originalShortFilename.rfind(PATH_SLASH)+1);

				copyFilename = traceWin->copyImageToDestinationAs(
					traceWin->mFin->mOriginalImageFilename,
					catalogPath);

				copyShortFilename = copyFilename.substr(copyFilename.rfind(PATH_SLASH)+1);
			}
			//***1.96a - end of new code segment

			// now copy the MODIFIED image to the catalog

			//***1.96a - prevent copying over existing modified image in catalog 

			string modFilename;
			if ((originalShortFilename != copyShortFilename) ||
				(traceWin->mFin->mImageFilename == traceWin->mFin->mOriginalImageFilename))
			{
				// then comment in modified image file identifying original image
				// file must be changed -- this means recreating the modified image
				// file
				//
				// OR the modified image was never previously saved and must be 
				// created/saved now
							
				// base the filename of the modified image on the renamed original image copy
				int pos = copyFilename.rfind('.');
				modFilename = copyFilename.substr(0,pos) + "_wDarwinMods.png";

				modFilename = traceWin->nextAvailableDestinationFilename(modFilename); //***1.96a

				//***1.85 & 1.9 - MUST set scale before saving
				traceWin->mNonZoomedImage->mNormScale = traceWin->mNormScale;

				// save modified image
				traceWin->mNonZoomedImage->save_wMods( //***1.8 - new save modified image call
						modFilename,    // the filename of the modified image
						copyShortFilename,   //***1.96a - the filename of the original image
						traceWin->mImageMods); // the list of image modifications
			}
			else
			{
				// simply copy the modified image file over to the catalog folder

				modFilename = traceWin->copyImageToDestinationAs(
						traceWin->mFin->mImageFilename, 
						catalogPath);
			}

			//***1.8 - now we are saving the filename of the modified image as part
			//         of the database entry.  We will look in the comments of the
			//         modified file to retrieve the name of the original image file
			//         so that it can be retrieved as needed

			DatabaseFin<ColorImage> *newFin = new DatabaseFin<ColorImage>(
					modFilename, //***1.8 - save modified image name now
					traceWin->mOutline, //***008OL
					id,
					name,
					date,
					roll,
					location,
					damage,
					description);

			//***1.8 - pass along nonZoomed, but modified image, scaleChange, & image mod list
			newFin->mModifiedFinImage = new ColorImage(traceWin->mNonZoomedImage);
			newFin->mOriginalImageFilename = copyShortFilename; //***1.96
			newFin->mModifiedFinImage->mNormScale = traceWin->mNormScale;
			newFin->mImageMods = traceWin->mImageMods;
			//***1.8 - end of new code section

			traceWin->mDatabase->add(newFin);

			delete newFin;

			traceWin->mMainWin->refreshDatabaseDisplayNew(true); //***1.96a
			traceWin->mMainWin->selectFromReorderedCList(modFilename); //***1.8
		}


		delete traceWin;
	} catch (Error e) {
		showError(e.errorString());
		delete traceWin;
	}
}

//*******************************************************************
//
//
//
//
//
void on_traceButtonCancel_clicked(GtkButton * button, gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (NULL == traceWin)
		return;

	delete traceWin;
}

//*******************************************************************
//
//
//
//
//
gboolean on_questionDialog_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *)userData;

	if (NULL == traceWin)
		return FALSE;

	gtk_widget_destroy(traceWin->mQuestionDialog);
	traceWin->mQuestionDialog = NULL;

	return TRUE;
}

//*******************************************************************
//
//
//
//
//
void on_questionButtonYes_clicked(
	GtkButton *button,
	gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *)userData;

	if (NULL == traceWin)
		return;

	gtk_widget_destroy(traceWin->mQuestionDialog);
	traceWin->mQuestionDialog = NULL;

	on_traceButtonAddToDatabase_clicked(NULL, (void *)traceWin);
}

//*******************************************************************
//
//
//
//
//
void on_questionButtonCancel_clicked(
	GtkButton *button,
	gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *)userData;

	if (NULL == traceWin)
		return;

	gtk_widget_destroy(traceWin->mQuestionDialog);
	traceWin->mQuestionDialog = NULL;
}

//*******************************************************************
//
//
//
//
//
gboolean on_mDrawingArea_configure_event(
		GtkWidget *widget,
		GdkEventConfigure *event,
		gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *)userData;

	if (NULL == traceWin)
		return FALSE;

	// We want to ignore configure events right after a zoom.
	// zoomUpdate will set the ignore flag when it runs.  (We only
	// want to handle configure events that are generated by the
	// user resizing the window.)
	if (traceWin->mIgnoreConfigureEventCnt) {
		traceWin->mIgnoreConfigureEventCnt--;
		return TRUE;
	}

	// Try to figure out what the zoom ratio should be to fit
	// the image inside the drawing area
	float heightRatio, widthRatio;

	heightRatio = (float)traceWin->mDrawingArea->allocation.height /
		traceWin->mNonZoomedImage->getNumRows();
	widthRatio = (float)traceWin->mDrawingArea->allocation.width /
		traceWin->mNonZoomedImage->getNumRows();

	if (heightRatio < 1.0f && widthRatio < 1.0f) {
		if (widthRatio < heightRatio) {
			// the width is the restrictive dimension
			if (widthRatio >= 0.5f)
				traceWin->mZoomRatio = 50;
			else
				traceWin->mZoomRatio = 25;
		} else if (heightRatio >= 0.5f)
			traceWin->mZoomRatio = 50;
		else
			traceWin->mZoomRatio = 25;
	} else
		traceWin->mZoomRatio = 100;

	//***1.4 - new flag to 
	// prevent problem with order of mCG, mDrawingArea->window creation
	// the flag is set true in the new constructor and set false here without 
	// redrawing the image and trace BEFORE the graphics context is created
	if (! traceWin->mLoadingFinNow)
		traceWin->zoomUpdate(true); //***1.1 - true, so sliders reconfigure

	return TRUE;
}

//*******************************************************************
//
//
//
//
//
gboolean on_mScrolledWindow_configure_event(
	GtkWidget *widget,
	GdkEventConfigure *event,
	gpointer userData)
{
	TraceWindow *traceWin = (TraceWindow *) userData;

	if (widget->allocation.width > traceWin->mSWWidthMax)
		traceWin->mSWWidthMax = widget->allocation.width;

	if (widget->allocation.height > traceWin->mSWHeightMax)
		traceWin->mSWHeightMax = widget->allocation.height;

	return TRUE;
}

//****************************************************************
//***1.6 - new function used to set name of saved fin file
//
//
void TraceWindow::setSavedFinFilename(string fname)
{
	mSavedFinFilename = fname;
}
