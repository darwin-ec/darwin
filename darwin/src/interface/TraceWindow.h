//*******************************************************************
//   file: TraceWindow.h
//
// author: Adam Russell
//
//   mods:
// 
// This file is a little uglier than it should be.
// It also tries to do too much different stuff. - AR
//
//*******************************************************************

#ifndef TRACEWINDOW_H
#define TRACEWINDOW_H

#include <gtk/gtk.h>
#include "GtkCompat.h"

#include "../image_processing/ColorImage.h"
#include "../image_processing/exif.h" //***1.1
#include "../Contour.h"
#include "../Outline.h" //***008OL
#include "../Database.h"
#include "../Options.h"
#include "../Chain.h"   //***006PD
//#include "../CatalogCategories.h"
#include "MainWindow.h"
#include "../image_processing/ImageMod.h"

int getNumTraceWindowReferences();

typedef enum {
	TRACE_MAGNIFY,
	TRACE_AUTOTRACE,//103AT SAH
	TRACE_PENCIL,
	TRACE_ADD_POINT,
	TRACE_MOVE_POINT,
	TRACE_MOVE_FEATURE, //***006PM
	TRACE_ERASER,
	TRACE_CHOPOUTLINE,  //*** 1.5 krd
	TRACE_CROP,
	TRACE_ROTATE,
} traceToolType;

class TraceWindow
{
	public:
		// Constructor
		// 	Sets up all the gtk widgets and such, but
		// 	doesn't show the dialog.  show() must later be
		// 	called to do that.
		//
		// 	Will create a new ColorImage, so the one passed
		// 	in may be deleted later.
		//
		// 	Does a pointer copy of db and o
		TraceWindow(
				MainWindow *m,
				const std::string &fileName,
				const ColorImage *image,
				Database *db,
				Options *o);

		//***1.4 - new version for loading single previously traced fin file into TraceWindow
		TraceWindow::TraceWindow(
				MainWindow *m,
				const string &fileName,
				DatabaseFin<ColorImage> *unkFin,
				Database *db,
				Options *o);

		// Destructor
		// 	Destroys the dialog if it's open and frees
		// 	resources.
		~TraceWindow();

		// show
		// 	Simply draws the dialog on the screen.
		void show();

		//***1.4 - new function to setup buttons and interface for loaded fin trace
		void TraceWindow::setupForLoadedFin();

		// used to reset mContour, mChain and mTraceFinalized for new tracing
		void traceReset(); //***006PD

		void traceSave(const std::string &fileName);

		bool pointInImageBounds(int x, int y);

		void zoomUpdate(bool setSize, int x = 0, int y = 0);

		// Rescales Contour to uniform distance between LE & Tip
		// returns scale factor used to normalize *c relative to origin
		friend float normalizeContour(Contour *c); //***006CM

		void setSavedFinFilename(string fname); //***1.6

		ImageModList& theImageMods(); //***1.8 - ref to list so it can be accessed/modified

		// GTK+ callback functions

		friend gboolean on_traceWindow_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_traceButtonUndo_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_traceButtonRedo_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_traceButtonFlipHorizontally_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_traceButtonFlipVertically_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_traceButtonEnhanceContrast_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_traceButtonAlterBrightness_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_traceButtonDespeckle_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_traceButtonSmooth_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_traceButtonResize_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_traceRadioButtonMagnify_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

		friend void on_traceRadioButtonAutoTrace_toggled(//103AT SAH
				GtkToggleButton *togglebutton,
				gpointer userData);

		friend void on_traceRadioButtonPencil_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

		friend void on_traceRadioButtonAddPoint_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

		friend void on_traceRadioButtonMovePoint_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

		//***06PM new function to move Features
		friend void on_traceRadioButtonMoveFeature_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

                //*** 1.5 krd
                friend void on_traceRadioButtonChopoutline_toggled(
                             GtkToggleButton * togglebutton,
                                 gpointer userData);

		friend void on_traceRadioButtonEraser_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

		friend void on_traceRadioButtonCrop_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

		friend void on_traceRadioButtonRotate_toggled(
				GtkToggleButton *togglebutton,
				gpointer userData);

		//***006FC next three functions are new

		friend void on_traceButtonImageOK_clicked(
				GtkButton * button,
				gpointer userData);

		friend void on_traceButtonTraceOK_clicked(
				GtkButton * button,
				gpointer userData);

		friend void on_traceButtonFeaturePointsOK_clicked(
				GtkButton * button,
				gpointer userData);

		//***051TW new function below

		friend void on_traceButtonImageMod_clicked(
				GtkButton * button,
				gpointer userData);

		//***1.4 - three new functions

		friend void on_traceButtonSlideTrace_toggled(
				GtkButton * button,
				gpointer userData);

		friend gboolean on_traceButtonScaleTrace_changed(
				GtkRange *range,
//				GtkScrollType scroll,
//				gdouble value,
				gpointer userData);

		friend void on_traceButtonTraceUnlock_clicked(
				GtkButton * button,
				gpointer userData);


		friend gboolean on_traceEventBox_button_press_event(
				GtkWidget *widget,
				GdkEventButton *event,
				gpointer userData);

		friend gboolean on_traceEventBox_button_release_event(
				GtkWidget *widget,
				GdkEventButton *event,
				gpointer userData);

		friend gboolean on_traceEventBox_motion_notify_event(
				GtkWidget *widget,
				GdkEventMotion  *event,
				gpointer userData);

		friend gboolean on_traceDrawingArea_expose_event(
				GtkWidget *widget,
				GdkEventExpose *event,
				gpointer userData);

		friend void on_traceButtonDumpData_clicked( //***1.96a - new callback
				GtkButton * button, 
				gpointer userData);

		friend void on_traceButtonMatch_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_traceButtonSave_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_traceButtonAddToDatabase_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_traceButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);

		friend gboolean on_questionDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

		friend void on_questionButtonYes_clicked(
				GtkButton *button,
				gpointer userData);

		friend void on_questionButtonCancel_clicked(
				GtkButton *button,
				gpointer userData);

		friend gboolean on_mDrawingArea_configure_event(
				GtkWidget *widget,
				GdkEventConfigure *event,
				gpointer userData);

		friend gboolean on_mScrolledWindow_configure_event(
				GtkWidget *widget,
				GdkEventConfigure *event,
				gpointer userData);

	private:
		MainWindow *mMainWin;

		ColorImage
			*mOriginalImage,
			*mNonZoomedImage,
			*mUndoImage,
			*mImage;

		 //***1.8 - list of modifications applied to mOriginalImage to produce mImage
		ImageModList mImageMods;

		ImageMod mLastMod; //***1.95

		bool mIsFlipped; //***1.8

		std::string mImagefilename; //***001DB

		std::string mSavedFinFilename; //***1.6

		Database *mDatabase;

		DatabaseFin<ColorImage> *mFin;  //***1.4 - pointer to loaded or saved fin trace

		Contour
			*mContour,
			*mUndoContour;

		Outline
			*mOutline; //***008OL

		string mSavedLabel;   //***054

		//***006FC new member widgets

		GtkWidget
			*mButtonFlipHorizontally,
			*mButtonFlipVertically,
			*mButtonEnhanceContrast,
			*mButtonAlterBrightness,
			*mButtonDespeckle,
			*mButtonSmooth,
			*mButtonResize,
			*mRadioButtonCrop,
			*mRadioButtonRotate,
			*mRadioButtonMagnify,
			*mRadioButtonAutoTrace,//103AT SAH
			*mRadioButtonPencil,
			*mRadioButtonAddPoint,
			*mRadioButtonMovePoint,
			*mRadioButtonEraser,
			*mRadioButtonChopoutline,  //*** 1.5 krd
			*mRadioButtonMoveFeature,
			*mButtonImageMod, //***051TW
			*mButtonImageOK,
			*mButtonTraceOK,
			//*mButtonFeaturePointsOK, //***1.96 - is not used at this time
			*mButtonSlideTrace, //***1.4
			*mButtonScaleTrace, //***1.4
			*mButtonTraceUnlock, //***1.4
			*mStatusLabel; //***054TW

		//***006FC end of new member widgets
		GtkWidget
			*mWindow,
			*mQuestionDialog,
			*mDrawingArea,
			*mScrolledWindow,
			*mLabelPosition,
			*mLabelMagnification,

			// Text entries
			*mEntryID,
			*mEntryName,
			*mEntryDate,
			*mEntryRoll,
			*mEntryLocation,
			*mEntryDamage,
			*mEntryDescription,

			// buttons
			*mButtonUndo,
			*mButtonRedo;

		GdkCursor *mTraceCursor;

		GdkGC 
			*mGC,
			*mMovingGC; //***051TW (red - used while moving a feature point)

		bool
			mTraceSnapped,        //***051TW
			mTraceFinalized,
			mImageLocked,         //***006FC
			mTraceLocked,         //***006FC
			mFeaturePointsLocked, //***006FC
			mLoadingFinNow,       //***1.4
			mFinWasLoaded;        //***1.95

		traceToolType mCurTraceTool;

		float mZoomScale;
		int mZoomRatio;

		// scale difference between original contour and normalized one
		float mNormScale;       //***006NC

		// offsets for zoomed images
		int mZoomXOffset, mZoomYOffset;
		//int mVirtualTop, mVirtualLeft; //***1.96 - JHS
		int mIgnoreConfigureEventCnt;

		//***1.75 - location of window center in image 
		//          (as percent of image dimension ... 0.0 to 1.0)
		double
			mImageCenterX,  
			mImageCenterY;


		int mMovePosition;
		bool mMoveInit;

		bool mChopInit;   // ***krd 1.5 - next 3 lines
		int mChopPosition;
		int mChopLead;

		int mMoveFeature; //***051TW (type of feature being moved, e.g., TIP)

		// stuff to handle scrolled window configuration events
		// (these variables limit the size that the scrolled
		// window will be set to.)
		int mSWWidthMax, mSWHeightMax;

		Options *mOptions;

		// crop vars
		int
			mXCropStart,
			mYCropStart,
			mXCropEnd,
			mYCropEnd,
			mXCropPrev,
			mYCropPrev,
			mXCropPrevLen,
			mYCropPrevLen;

		// rotate variables
		float mRotateStartAngle;
		int mRotateXStart, mRotateYStart,
			mRotateXCenter, mRotateYCenter;
		ColorImage *mRotateOriginalImage;

		//////////////////////////
		// Private utility functions:

		GtkWidget* createTraceWindow(const std::string &title);
		GtkWidget* createQuestionDialog();

		void refreshImage();

		void updateCursor();
		void updateGC();
		void updateGCColor();
		void updateGCColor(GdkGC *gc, double color[4]); //***051TW


		// utility functions to handle tracing
		void traceAddAutoTracePoint(int x, int y, bool bolShift); //103AT SAH
		void traceAddNormalPoint(int x, int y);
		void traceAddExtraPoint(int x, int y);
		void traceErasePoint(int x, int y);
                void traceChopOutline(int x, int y);  // *** 1.5 krd
                void traceChopOutlineFinal();         // *** 1.5 krd

		// .. moving points stuff
		void traceMovePointInit(int x, int y);
		void traceMovePointUpdate(int x, int y);
		void traceMovePointFinalize(int x, int y);
		void traceMovePointDisplay();
		void ensurePointInBounds(int &x, int &y);
		void ensurePointInZoomedBounds(int &x, int &y);

		void traceMoveFeaturePointInit(int x, int y); //***051WT
		void traceMoveFeaturePointUpdate(int x, int y); //***051WT
		void traceMoveFeaturePointFinalize(int x, int y); //***051WT
		void traceMoveFeaturePointDisplay(int x, int y); //***051WT

		 //***006FC, ***1.96 (params added)
		void traceSnapToFin(bool useCyan, 
		                    int left, int top, 
		                    int right, int bottom);

		void traceFinalize();
		void outlineCreate(); //***006FC new

		// utility functions for zoom
		void zoomIn(int mouseX, int mouseY);
		void zoomOut(int mouseX, int mouseY);

		void zoomMapPointsToOriginal(int &x, int &y);
		void zoomMapPointsToZoomed(int &x, int &y);
		int zoomPointSize();

		// Undo and redo stuff
		void addUndo(ColorImage *u);
		void addUndo(Contour *u);

		void undo();
		void redo();

		// crop stuff
		void cropInit(int x, int y);
		void cropUpdate(int x, int y);
		void cropFinalize(int x, int y);

		// rotate stuff
		void rotateInit(int x, int y);
		void rotateUpdate(int x, int y);
		void rotateFinal(int x, int y);

		//***1.96 - two new functions to manage virtual cropping of image prior
		// to and following a trace
		void createVirtualImageFromNonZoomed();
		void restoreNonZoomedFromVirtualImage();
		void getViewedImageBoundsNonZoomed(int &left, int &top, int &right, int &bottom);

		//***1.96a -- new functinos to support image copying to catalog
		std::string copyImageToDestinationAs(std::string srcName, std::string destPath);
		std::string nextAvailableDestinationFilename(std::string destName);

};

#endif
