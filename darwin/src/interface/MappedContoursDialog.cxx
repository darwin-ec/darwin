//*******************************************************************
//   file: MappedContoursDialog.cxx
//
// author: J H Stewman (1/24/2006)
//
//   mods: 
//
//*******************************************************************

#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <cmath>
#include <cstdio>

#include "../support.h"
#include "MappedContoursDialog.h"
#include "../Chain.h" //***008OL
#include "../utility.h"
#include "../feature.h"
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "../../pixmaps/close.xpm"
#include "../waveletUtil.h"

using namespace std;

static const double LE_ANGLE_LINE_RADIUS = 150.0;
static const int DA_WIDTH = 450, DA_HEIGHT = 300;
static const int POINT_SIZE = 2;
static const float CHAIN_POINTS_DISTANCE = 3.0; //***005CI

static int gNumReferences = 0;


//*******************************************************************
//
// int getNumContourInfoDialogReferences()
//
//    Return count of ContourInfoDialogs currently existing.
//
int getNumMappedContourDialogReferences()
{
	return gNumReferences;
}


//*******************************************************************
//
// ContourInfoDialog::ContourInfoDialog(...)
//
//    CONSTRUCTOR
//
MappedContoursDialog::MappedContoursDialog(
		string ident1,          // unknown fin identifier
		FloatContour *c1,       // unknown
		int b1, int t1, int e1, // shifted unknown feature points
		string ident2,          // database fin identifier
		FloatContour *c2,       // database
		int b2, int t2, int e2  // shifted database feature points
)
	: mNameUnk(ident1),
	  mNameDB(ident2),
	  mDialog(createDialog()),
	  mUnknownContourGC(NULL),
	  mDatabaseContourGC(NULL),
	  mChainGC(NULL),
	  mHighlightGC(NULL),
	  //mFinOutline(oL), //***008OL
	  mUnkContour(new FloatContour(*c1)),
	  mUnkBegin(b1), mUnkTip(t1), mUnkEnd(e1),
	  mDBContour(new FloatContour(*c2)),
	  mDBBegin(b2), mDBTip(t2), mDBEnd(e2),
	  mShowErrPts(false) //***1.75
{

	mTraceColor[0] = 1.0;
	mTraceColor[1] = 0.1;
	mTraceColor[2] = 0.3;
	mTraceColor[3] = 0.0;

	findPointPairsUsedInMSECalulation( //***1.75
		mUnkContour, mUnkBegin, mUnkEnd,
		mDBContour, mDBBegin, mDBEnd);

	gNumReferences++;
}


//*******************************************************************
//
// ContourInfoDialog::~ContourInfoDialog()
//
//    DESTRUCTOR.
//
MappedContoursDialog::~MappedContoursDialog()
{
	if (NULL != mDialog)
		gtk_widget_destroy(mDialog);

	if (NULL != mUnkContour)
		delete mUnkContour;
	if (NULL != mDBContour)
		delete mDBContour;

	if (NULL != mUnknownContourGC)
		gdk_gc_unref(mUnknownContourGC);
	if (NULL != mDatabaseContourGC)
		gdk_gc_unref(mDatabaseContourGC);
	if (NULL != mChainGC)
		gdk_gc_unref(mChainGC);
	if (NULL != mHighlightGC)
		gdk_gc_unref(mHighlightGC);

	gNumReferences--;
}


//*******************************************************************
//
// void ContourInfoDialog::show()
//
//    Member function that actually "shows" the contour info dialog.
//
void MappedContoursDialog::show()
{
	gtk_widget_show(mDialog);

	updateGC();
	updateInfo();
}


//*******************************************************************
//
// void ContourInfoDialog::updateGC()
//
//    Member function to manage graphics context for contour info
//    dialog.
//
void MappedContoursDialog::updateGC()
{
	if (NULL == mUnknownContourGC) {
		if (NULL == mDrawingAreaContour)
			return;
		mUnknownContourGC = gdk_gc_new(mDrawingAreaContour->window);
	}
	double color1[] = {1.0, 0.0, 0.0, 0.0};
	updateGCColor(mUnknownContourGC, color1);

	if (NULL == mDatabaseContourGC) {
		if (NULL == mDrawingAreaContour)
			return;
		mDatabaseContourGC = gdk_gc_new(mDrawingAreaContour->window);
	}
	double color2[] = {0.0, 0.0, 1.0, 0.0};
	updateGCColor(mDatabaseContourGC, color2);

	if (NULL == mHighlightGC) {
		if (NULL == mDrawingAreaContour)
			return;
		mHighlightGC = gdk_gc_new(mDrawingAreaContour->window);
	}
	double highlightColor[] = {1.0, 1.0, 0.4, 0.0};
	updateGCColor(mHighlightGC, highlightColor);

	if (NULL == mChainGC) {
		if (NULL == mDrawingAreaChain)
			return;
		mChainGC = gdk_gc_new(mDrawingAreaChain->window);
	}
	updateGCColor(mChainGC);
}


//*******************************************************************
//
// void ContourInfoDialog::updateGCColor(GdkGC *gc, double color[4])
//
//    Member Function.
//
void MappedContoursDialog::updateGCColor(GdkGC *gc, double color[4])
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
// void ContourInfoDialog::updateGCColor(GdkGC *gc)
//
//    Member function.
//
void MappedContoursDialog::updateGCColor(GdkGC *gc)
{
	updateGCColor(gc, mTraceColor);
}


//*******************************************************************
//
// void ContourInfoDialog::updateInfo()
//
//    Member function to update information displayed along with 
//    image of contour.
//
void MappedContoursDialog::updateInfo()
{
	char info[2048];
	sprintf(
		info,
		"Database Fin ID: %s\n"
		" Unknown Fin ID: %s\n\n"
		"Database Outline - Number of Points : %d\n"
		"\tShifted Outline Beginning at position: %d\n"
		"\tShifted Outline Tip at position %d\n"
		"\tShifted Outline End at position: %d\n"
		"Unknown Outline - Number of Points : %d\n"
		"\tShifted Outline Beginning at position: %d\n"
		"\tShifted Outline Tip at position %d\n"
		"\tShifted Outline End at position: %d",
		mNameDB.c_str(),
		mNameUnk.c_str(),
		mDBContour->length(),
		mDBBegin,
		mDBTip,
 		mDBEnd,
		mUnkContour->length(),
		mUnkBegin,
		mUnkTip,
 		mUnkEnd
    );
			
	gtk_text_freeze(GTK_TEXT(mTextBox)); //*** 2.2 - correct display issue
	gtk_text_insert(
			GTK_TEXT (mTextBox),
			NULL, NULL, NULL,
			info,
			strlen(info));
	gtk_text_thaw(GTK_TEXT(mTextBox)); //*** 2.2 - correct display issue
}


//*******************************************************************
//
// GtkWidget* ContourInfoDialog::createInfoDialog()
//
//    Member function to create the infoDialog. This creates and arranges
//    all GTK widgets needed to display the fin contour and selected
//    information (# of points, feature point positions, etc)
//
GtkWidget* MappedContoursDialog::createDialog()
{
	GtkWidget *infoDialog;
	GtkWidget *infoMainVBox;
	GtkWidget *infoVBox;
	GtkWidget *infoTopHBox;
	GtkWidget *infoContourFrame;
	GtkWidget *infoChainFrame;
	GtkWidget *infoScrolledWindow;
	GtkWidget *dialog_action_area1;
	GtkWidget *infoHButtonBox;
	GtkWidget *infoMidHBox;
	guint infoButtonOK_key;
	GtkWidget *infoButtonOK;
	GtkWidget *infoButtonGenCoeffFiles;
	GtkAccelGroup *accel_group;
	GtkWidget *infoLabelNumLevels;
	GtkObject *infoSpinButtonAdj;
	GtkWidget *tmpIcon, *tmpLabel, *tmpBox;

	accel_group = gtk_accel_group_new ();

	infoDialog = gtk_dialog_new ();
	gtk_object_set_data (GTK_OBJECT (infoDialog), "infoDialog", infoDialog);
	gtk_window_set_title (GTK_WINDOW (infoDialog), _("Outline Information"));
	gtk_window_set_policy (GTK_WINDOW (infoDialog), TRUE, TRUE, FALSE);
	gtk_window_set_position(GTK_WINDOW(infoDialog), GTK_WIN_POS_CENTER); //***1.8
	gtk_window_set_wmclass(GTK_WINDOW(infoDialog), "darwin_continfo", "DARWIN");
  
	//gtk_window_set_default_size(GTK_WINDOW(infoDialog), (gint)800, (gint)600);

	infoMainVBox = GTK_DIALOG (infoDialog)->vbox;
	gtk_object_set_data (GTK_OBJECT (infoDialog), "infoMainVBox", infoMainVBox);
	gtk_widget_show (infoMainVBox);

	infoVBox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (infoVBox);
	gtk_box_pack_start (GTK_BOX (infoMainVBox), infoVBox, TRUE, TRUE, 0);

	infoTopHBox = gtk_hbox_new (FALSE, 5);
	gtk_widget_show (infoTopHBox);
	gtk_box_pack_start (GTK_BOX (infoVBox), infoTopHBox, TRUE, TRUE, 4);

	infoContourFrame = gtk_frame_new (_("Fin Outline"));
	gtk_widget_show (infoContourFrame);
	gtk_box_pack_start (GTK_BOX (infoTopHBox), infoContourFrame, TRUE, TRUE, 0);

	mDrawingAreaContour = gtk_drawing_area_new ();
	gtk_widget_show (mDrawingAreaContour);
	gtk_container_add (GTK_CONTAINER (infoContourFrame), mDrawingAreaContour);

	gtk_drawing_area_size(GTK_DRAWING_AREA(mDrawingAreaContour), DA_WIDTH, DA_HEIGHT);

	infoChainFrame = gtk_frame_new (_("Chain Code"));
	gtk_widget_show (infoChainFrame);
	gtk_box_pack_start (GTK_BOX (infoTopHBox), infoChainFrame, TRUE, TRUE, 0);

	mDrawingAreaChain = gtk_drawing_area_new ();
	gtk_widget_show (mDrawingAreaChain);
	gtk_container_add (GTK_CONTAINER (infoChainFrame), mDrawingAreaChain);

	gtk_drawing_area_size(GTK_DRAWING_AREA(mDrawingAreaChain), DA_WIDTH, DA_HEIGHT);

	infoMidHBox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(infoMidHBox);
	gtk_box_pack_start(GTK_BOX(infoVBox), infoMidHBox, TRUE, TRUE, 0);

	infoButtonGenCoeffFiles = gtk_button_new_with_label("Generate Coefficient Files");
	gtk_widget_show(infoButtonGenCoeffFiles);
	GTK_WIDGET_SET_FLAGS(infoButtonGenCoeffFiles, GTK_CAN_DEFAULT);
	gtk_box_pack_start(GTK_BOX(infoMidHBox), infoButtonGenCoeffFiles, FALSE, FALSE, 0);

	infoLabelNumLevels = gtk_label_new (_(" Number of Levels: "));
	gtk_widget_show(infoLabelNumLevels);
	gtk_box_pack_start(GTK_BOX(infoMidHBox), infoLabelNumLevels, FALSE, FALSE, 0);

	//infoSpinButtonAdj = gtk_adjustment_new(7, 1, 9, 1, 1, 1);
	infoSpinButtonAdj = gtk_adjustment_new(7, 1, 9, 1, 1, 0); //*** 2.2 - fixes display issue
	mSpinButton = gtk_spin_button_new(GTK_ADJUSTMENT (infoSpinButtonAdj), 1, 0);
	gtk_widget_show (mSpinButton);
	gtk_box_pack_start(GTK_BOX(infoMidHBox), mSpinButton, FALSE, TRUE, 0);

	infoScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (infoScrolledWindow);
	gtk_box_pack_start (GTK_BOX (infoVBox), infoScrolledWindow, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (infoScrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_widget_set_usize(GTK_WIDGET(infoScrolledWindow), 300, 200);

	mTextBox = gtk_text_new (NULL, NULL);
	gtk_widget_show (mTextBox);
	gtk_container_add (GTK_CONTAINER (infoScrolledWindow), mTextBox);
 
	dialog_action_area1 = GTK_DIALOG (infoDialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (infoDialog), "dialog_action_area1", dialog_action_area1);
	gtk_widget_show (dialog_action_area1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 10);

	infoHButtonBox = gtk_hbutton_box_new ();
	gtk_widget_show (infoHButtonBox);
	gtk_box_pack_start (GTK_BOX (dialog_action_area1), infoHButtonBox, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (infoHButtonBox), GTK_BUTTONBOX_END);

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, close_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);
  
	infoButtonOK = gtk_button_new();
	infoButtonOK_key = gtk_label_parse_uline(GTK_LABEL(tmpLabel),
					_("_Close"));
	gtk_widget_add_accelerator (infoButtonOK, "clicked", accel_group,
					infoButtonOK_key, GDK_MOD1_MASK, (GtkAccelFlags)0);
	gtk_container_add(GTK_CONTAINER(infoButtonOK), tmpBox);
	gtk_widget_show (infoButtonOK);
	gtk_container_add (GTK_CONTAINER (infoHButtonBox), infoButtonOK);
	GTK_WIDGET_SET_FLAGS (infoButtonOK, GTK_CAN_DEFAULT);
	gtk_widget_add_accelerator (infoButtonOK, "clicked", accel_group,
					GDK_C, GDK_MOD1_MASK,
					GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (infoButtonOK, "clicked", accel_group,
					GDK_Escape, (GdkModifierType)0,
					GTK_ACCEL_VISIBLE);

	gtk_signal_connect (GTK_OBJECT (infoDialog), "delete_event",
					GTK_SIGNAL_FUNC (on_mappedContoursDialog_delete_event),
					(void *) this);
	gtk_signal_connect (GTK_OBJECT (mDrawingAreaContour), "expose_event",
					GTK_SIGNAL_FUNC (on_mappedDrawingArea_expose_event),
					(void *) this);
	gtk_signal_connect (GTK_OBJECT (mDrawingAreaChain), "expose_event",
					GTK_SIGNAL_FUNC (on_mappedChainDrawingArea_expose_event),
					(void *) this);
	gtk_signal_connect (GTK_OBJECT (infoButtonOK), "clicked",
					GTK_SIGNAL_FUNC (on_mappedButtonOK_clicked),
					(void *) this);

	gtk_widget_grab_default (infoButtonOK);
	gtk_window_add_accel_group (GTK_WINDOW (infoDialog), accel_group);

	return infoDialog;
}


//*******************************************************************
//
// gboolean on_infoDialog_delete_event(...)
//
//    Friend function to process infoDialog delete events.
//
gboolean on_mappedContoursDialog_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	MappedContoursDialog *infoDialog = (MappedContoursDialog *)userData;

	if (NULL == infoDialog)
		return FALSE;

	delete infoDialog;
	
	return TRUE;
}


//*******************************************************************
//
// gboolean on_infoContourDrawingArea_expose_event(...)
//
//    Friend function to process ContourDrawingArea expose events.
//
gboolean on_mappedDrawingArea_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData)
{
	MappedContoursDialog *dialog = (MappedContoursDialog *)userData;

	if (NULL == dialog)
		return FALSE;

	if ((NULL == dialog->mUnkContour) || (NULL == dialog->mDBContour))
		return FALSE;

	FloatContour *c = dialog->mUnkContour;

	int
		xMax = c->maxX(),
		yMax = c->maxY(),
		xMin = c->minX(),
		yMin = c->minY();

	//***1.5 - now check other contour
	if (dialog->mDBContour->maxX() > xMax)
		xMax = dialog->mDBContour->maxX();
	if (dialog->mDBContour->maxY() > yMax)
		yMax = dialog->mDBContour->maxY();
	if (dialog->mDBContour->minX() < xMin)
		xMin = dialog->mDBContour->minX();
	if (dialog->mDBContour->minY() < yMin)
		yMin = dialog->mDBContour->minY();

	//***1.5 - make a little space around the outlines
	xMin -= 4;
	xMax += 4;
	yMin -= 4;
	yMax += 4;

	int
		xRange = xMax - xMin,
		yRange = yMax - yMin;

	float
		heightRatio = DA_HEIGHT / ((float)yRange),
		widthRatio = DA_WIDTH / ((float)xRange);

	float ratio;

	int xOffset = 0, yOffset = 0;
	
	if (heightRatio < widthRatio) {
		ratio = heightRatio;
		xOffset = (int) round((DA_WIDTH - ratio * xRange) / 2);	
	} else {
		ratio = widthRatio;
		yOffset = (int) round((DA_HEIGHT - ratio * yRange) / 2);
	}

	unsigned numPoints = c->length();
		
	unsigned i;
	for (i = 0; i < numPoints; i++) {	
		int xCoord = (int) round(((*c)[i].x - xMin) * ratio + xOffset);
		int yCoord = (int) round(((*c)[i].y - yMin) * ratio + yOffset);
		
		gdk_draw_rectangle(
			dialog->mDrawingAreaContour->window,
			dialog->mUnknownContourGC,
			TRUE,
			xCoord - POINT_SIZE / 2,
			yCoord - POINT_SIZE / 2,
			POINT_SIZE,
			POINT_SIZE);
	}

	c = dialog->mDBContour;
	numPoints = c->length();

	for (i = 0; i < numPoints; i++) {	
		int xCoord = (int) round(((*c)[i].x - xMin) * ratio + xOffset);
		int yCoord = (int) round(((*c)[i].y - yMin) * ratio + yOffset);
		
		gdk_draw_rectangle(
			dialog->mDrawingAreaContour->window,
			dialog->mDatabaseContourGC,
			TRUE,
			xCoord - POINT_SIZE / 2,
			yCoord - POINT_SIZE / 2,
			POINT_SIZE,
			POINT_SIZE);
	}

	if (dialog->mShowErrPts) // show the point pairs used in error calculation
	{
	 	for (i = 0; i < dialog->mUnkErrPts.length(); i += 4)
		{
			// draw the segment between corresponding points
			int xCoord = (int) round(((dialog->mUnkErrPts)[i].x - xMin) * ratio + xOffset);
			int yCoord = (int) round(((dialog->mUnkErrPts)[i].y - yMin) * ratio + yOffset);
			int xCoord2 = (int) round(((dialog->mSelErrPts)[i].x - xMin) * ratio + xOffset);
			int yCoord2 = (int) round(((dialog->mSelErrPts)[i].y - yMin) * ratio + yOffset);
			gdk_draw_line(
				dialog->mDrawingAreaContour->window,
				dialog->mDatabaseContourGC,
				xCoord, yCoord,
				xCoord2, yCoord2);
		}
				
	 	/*
		for (i = 0; i < dialog->mMidPts.length(); i += 4)
		{
			// draw medial axis point
			point_t p = dialog->mMidPts[i];
			int xC = (int) round((p.x - xMin) * ratio + xOffset);
			int yC = (int) round((p.y - yMin) * ratio + yOffset);
			int highlightPointSize = POINT_SIZE * 4;
			gdk_draw_rectangle(
				dialog->mDrawingAreaContour->window,
				dialog->mUnknownContourGC,
				TRUE,
				xC - highlightPointSize / 4,
				yC - highlightPointSize / 4,
				highlightPointSize / 2,
				highlightPointSize / 2);
		}
		*/
	}
	
	int highlightPointSize = POINT_SIZE * 4;

	point_t p;

	// draw Shifted Tip Points - DB large and yellow, UNK small and red

	p = (*dialog->mDBContour)[dialog->mDBTip];
	int xC = (int) round((p.x - xMin) * ratio + xOffset);
	int yC = (int) round((p.y - yMin) * ratio + yOffset);
	gdk_draw_rectangle(
		dialog->mDrawingAreaContour->window,
		dialog->mHighlightGC,
		TRUE,
		xC - highlightPointSize / 2,
		yC - highlightPointSize / 2,
		highlightPointSize,
		highlightPointSize);

	p = (*dialog->mUnkContour)[dialog->mUnkTip];
	xC = (int) round((p.x - xMin) * ratio + xOffset);
	yC = (int) round((p.y - yMin) * ratio + yOffset);
	gdk_draw_rectangle(
		dialog->mDrawingAreaContour->window,
		dialog->mUnknownContourGC,
		TRUE,
		xC - highlightPointSize / 4,
		yC - highlightPointSize / 4,
		highlightPointSize / 2,
		highlightPointSize / 2);

	// draw Shifted BEGIN Points - DB large and yellow, UNK small and red

	p = (*dialog->mDBContour)[dialog->mDBBegin];
	xC = (int) round((p.x - xMin) * ratio + xOffset);
	yC = (int) round((p.y - yMin) * ratio + yOffset);

	gdk_draw_rectangle(
		dialog->mDrawingAreaContour->window,
		dialog->mHighlightGC,
		TRUE,
		xC - highlightPointSize / 2,
		yC - highlightPointSize / 2,
		highlightPointSize,
		highlightPointSize);

	p = (*dialog->mUnkContour)[dialog->mUnkBegin];
	xC = (int) round((p.x - xMin) * ratio + xOffset);
	yC = (int) round((p.y - yMin) * ratio + yOffset);

	gdk_draw_rectangle(
		dialog->mDrawingAreaContour->window,
		dialog->mUnknownContourGC,
		TRUE,
		xC - highlightPointSize / 4,
		yC - highlightPointSize / 4,
		highlightPointSize / 2,
		highlightPointSize / 2);

	// draw Shifted END Points - DB large and yellow, UNK small and red

	p = (*dialog->mDBContour)[dialog->mDBEnd];
	xC = (int) round((p.x - xMin) * ratio + xOffset);
	yC = (int) round((p.y - yMin) * ratio + yOffset);

	gdk_draw_rectangle(
		dialog->mDrawingAreaContour->window,
		dialog->mHighlightGC,
		TRUE,
		xC - highlightPointSize / 2,
		yC - highlightPointSize / 2,
		highlightPointSize,
		highlightPointSize);

	p = (*dialog->mUnkContour)[dialog->mUnkEnd];
	xC = (int) round((p.x - xMin) * ratio + xOffset);
	yC = (int) round((p.y - yMin) * ratio + yOffset);

	gdk_draw_rectangle(
		dialog->mDrawingAreaContour->window,
		dialog->mUnknownContourGC,
		TRUE,
		xC - highlightPointSize / 4,
		yC - highlightPointSize / 4,
		highlightPointSize / 2,
		highlightPointSize / 2);


	return TRUE;
}


//*******************************************************************
//
// gboolean on_infoChainDrawingArea_expose_event(...)
//
//    Friend function to process ChainDrawingArea_expose events.
//
gboolean on_mappedChainDrawingArea_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData)
{
	MappedContoursDialog *infoDialog = (MappedContoursDialog *)userData;

	if (NULL == infoDialog)
		return FALSE;

	if (NULL == infoDialog->mDBContour)
		return FALSE;

	if (NULL == infoDialog->mUnkContour)
		return FALSE;

	Chain 
		*dbChain = new Chain(infoDialog->mDBContour),
		*unkChain = new Chain(infoDialog->mUnkContour);

	
	dbChain->smooth7();
	unkChain->smooth7();

	double
		yMin = dbChain->min(),
		yMax = dbChain->max();

	double vertRatio, angle;
        
	if (fabs(yMax) > fabs(yMin)) {
		vertRatio = (double)DA_HEIGHT / (fabs(yMax) * 2);
		angle = fabs(yMax);
	} else {
		vertRatio =  (double)DA_HEIGHT / (fabs(yMin) * 2);
		angle = fabs(yMin);
	}

	double 
		dbArcLength = 0.0,
		unkArcLength = 0.0;
	int j;
	for (j=infoDialog->mDBBegin; j<infoDialog->mDBEnd; j++)
	{
		dbArcLength += distance(
				(*infoDialog->mDBContour)[j].x,
				(*infoDialog->mDBContour)[j].y,
				(*infoDialog->mDBContour)[j+1].x,
				(*infoDialog->mDBContour)[j+1].y);
	}
	for (j=infoDialog->mUnkBegin; j<infoDialog->mUnkEnd; j++)
	{
		unkArcLength += distance(
				(*infoDialog->mUnkContour)[j].x,
				(*infoDialog->mUnkContour)[j].y,
				(*infoDialog->mUnkContour)[j+1].x,
				(*infoDialog->mUnkContour)[j+1].y);
	}

	double horizRatio =  ((double)DA_WIDTH) / dbArcLength;

	int
		i,
		x=0, // is this correct initial value? - JHS
		prevXCoord = 0,
		prevYCoord = (int) round(fabs((*dbChain)[0] - angle) * vertRatio); //***008OL

	double whereOnArc = 0.0;

	for (i = infoDialog->mDBBegin; i < infoDialog->mDBEnd; i++) {	 //***008OL
		whereOnArc += distance(
				(*infoDialog->mDBContour)[i].x,
				(*infoDialog->mDBContour)[i].y,
				(*infoDialog->mDBContour)[i+1].x,
				(*infoDialog->mDBContour)[i+1].y);
		int xCoord = (int) round(whereOnArc * horizRatio);
		int yCoord = (int) round(fabs((*dbChain)[i] - angle) * vertRatio); //***008OL
		gdk_draw_line(
			infoDialog->mDrawingAreaChain->window,
			infoDialog->mDatabaseContourGC,
			prevXCoord,
			prevYCoord,
			xCoord,
			yCoord);

		prevXCoord = xCoord;
		prevYCoord = yCoord;
	}
	
	horizRatio =  ((double)DA_WIDTH) / unkArcLength;

	prevXCoord = 0,
	prevYCoord = (int) round(fabs((*unkChain)[0] - angle) * vertRatio); //***008OL

	whereOnArc = 0.0;

	for (i = infoDialog->mUnkBegin; i < infoDialog->mUnkEnd; i++) {	 //***008OL
		whereOnArc += distance(
				(*infoDialog->mUnkContour)[i].x,
				(*infoDialog->mUnkContour)[i].y,
				(*infoDialog->mUnkContour)[i+1].x,
				(*infoDialog->mUnkContour)[i+1].y);
		int xCoord = (int) round(whereOnArc * horizRatio);
		x++;
		int yCoord = (int) round(fabs((*unkChain)[i] - angle) * vertRatio); //***008OL
		gdk_draw_line(
			infoDialog->mDrawingAreaChain->window,
			infoDialog->mUnknownContourGC,
			prevXCoord,
			prevYCoord,
			xCoord,
			yCoord);

		prevXCoord = xCoord;
		prevYCoord = yCoord;
	}

	delete dbChain;
	delete unkChain;

	return TRUE;
}


//*******************************************************************
//
// void on_infoButtonOK_clicked(...)
//
//    Friend function to process "OK" button click events.
//
void on_mappedButtonOK_clicked(
	GtkButton *button,
	gpointer userData)
{
	MappedContoursDialog *infoDialog = (MappedContoursDialog *)userData;

	if (NULL == infoDialog)
		return;

	delete infoDialog;
}



//////////////////////////////////////////////////////////////////////////
//
//***1.75 - added to allow us to show medial axis / corresponding points
//
//   this was copied from Match class and has been modified to find
//   and save corresponding point pairs used in error caculation
//
//////////////////////////////////////////////////////////////////////////
//
// Really new stuff -- method to use perpendiculars to medial axis (of sorts)
// to find  better "shortest" distances between pairs of contour points

//*******************************************************************
//
// void Match::meanSquaredErrorBetweenOutlineSegments(...)
//
//    REVISED: 11/14/05
//    Computes the error between defined outline segments.
//    Returns the error as a double.  It is assumed that the unknown
//    has been mapped to the database fin outline prior to the call.
//
//    This uses a new approach.
//    Compute arc length from start to tip and from tip to end.
//    Use arc length ratios between database and unknown to step along
//    database points and compute corresponding unknown points.
//
//void meanSquaredErrorBetweenOutlineSegments( 
void MappedContoursDialog::findPointPairsUsedInMSECalulation(
		FloatContour *c1, // mapped unknown fin 
		int begin1,
		int end1,
		FloatContour *c2, // envenly spaced database fin //***0005CM
		int begin2,
		int end2)
{

	double 
		error = 50000.0,
		dbArcLength, unkArcLength;

	// saved segment lengths, each is length of edge entering indexed point
	vector<double> segLen1, segLen2;

	int k;

	// find length of unknown fin outline
	unkArcLength = 0.0;
	segLen1.push_back(0.0); // no segment entering first point
	for (k = 1; k < c1->length(); k++)
	{
		double dx = (*c1)[k].x - (*c1)[k-1].x;
		double dy = (*c1)[k].y - (*c1)[k-1].y;
		segLen1.push_back(sqrt(dx * dx + dy * dy));
		if ((begin1 < k) && (k <= end1))
			unkArcLength += segLen1[k];
	}

	// find length of database fin outline
	dbArcLength = 0.0;
	segLen2.push_back(0.0); // no segment entering first point
	for (k = 1; k < c2->length(); k++)
	{
		double dx = (*c2)[k].x - (*c2)[k-1].x;
		double dy = (*c2)[k].y - (*c2)[k-1].y;
		segLen2.push_back(sqrt(dx * dx + dy * dy));
		if ((begin2 < k) && (k <= end2))
			dbArcLength += segLen2[k];
	}

	double ratio = unkArcLength / dbArcLength;

	double sum = 0.0;
	int ptsFound = 0;

	// this process walks the database contour and at each point computes
	// the length of the step, scales it by the ratio and finds a point
	// at this scaled distance farther along on the unknown

	// we walk the database fin because the unknown has been mapped and 
	// therefore the spacing of sample points is no longer uniform on the
	// unknown.  The database fin outline should be evenly spaced at approx.
	// 3.0 unit intervals

	FloatContour *midPt = new FloatContour;

	// i is index on database 
	// j is index on unknown
	int i = begin2+1, j = begin1+1;

	double segLenUsed = 0.0;

	while (i < end2)
	{
		// find dist to next point on database fin, and scaled distance to
		// corresponding point on unknown
		double howFar = ratio * segLen2[i] + segLenUsed;

		while (segLen1[j] < howFar)
		{
			howFar -= segLen1[j];
			j++;
		}
		// remember for next iteration
		segLenUsed = howFar;

		// point is on segment j of unknown, so find it

		double s = (howFar/segLen1[j]);
		double dx = (*c1)[j].x - (*c1)[j-1].x;
		double dy = (*c1)[j].y - (*c1)[j-1].y;
		double x = (*c1)[j-1].x + s * dx;
		double y = (*c1)[j-1].y + s * dy;
		
		// save midpoint (part of medial axis) for use later

		midPt->addPoint(0.5 * ((*c2)[i].x + x),0.5 * ((*c2)[i].y + y));
		//***1.75 - also save medial axis point for display later
		mMidPts.addPoint(0.5 * ((*c2)[i].x + x),0.5 * ((*c2)[i].y + y)); 

		i++;
	}

	// now traverse medial axis and find length of perpendicular through
	// medial axis point with endpoints on the two fin outlines

	i = begin2 + 1; // index on database fin
	j = begin1 + 1; // index on uknown fin

	bool done = false;

	int backI, backJ; //***1.75

	//***1.75 - keep track of j & i values from previous point pair calculation
	int iPrev = i;
	int jPrev = j;

	for (k = 1; (k+1 < midPt->length()) && (! done); k++)
	{
		double 
			unkX, unkY,
			dbX, dbY;

		double
			mdx = (*midPt)[k+1].x - (*midPt)[k-1].x,
			mdy = (*midPt)[k+1].y - (*midPt)[k-1].y;
		bool 
			foundUnk = false,
			foundDB = false;

		backI = 0; //***1.75
		backJ = 0; //***1.75

		while ((! foundUnk) && (! done))
		{
			double 
				dot1 = ((*c1)[j-1].x - (*midPt)[k].x) * mdx
				     + ((*c1)[j-1].y - (*midPt)[k].y) * mdy,
				dot2 = ((*c1)[j].x - (*midPt)[k].x) * mdx
				     + ((*c1)[j].y - (*midPt)[k].y) * mdy;
			if (((dot1 <= 0.0) && (0.0 <= dot2)) || ((dot2 <= 0.0) && (0.0 <= dot1)))
			{
				// this segment contains a point of intersection with the perpendicular from
				// the medial axis at point k
				// slope of unknown fin outline segment between points j-1 and j 
				double
					dx1 = (*c1)[j].x - (*c1)[j-1].x,
					dy1 = (*c1)[j].y - (*c1)[j-1].y;

				double beta = 0.0;

				if ((mdx * dx1 + mdy * dy1) != 0.0)
					beta = - (mdx * ((*c1)[j-1].x - (*midPt)[k].x) + mdy * ((*c1)[j-1].y - (*midPt)[k].y))
					       / (mdx * dx1 + mdy * dy1) ;

				if ((0.0 <= beta) && (beta <= 1.0))
				{
					// found the point on this unknown segment
					unkX = beta * dx1 + (*c1)[j-1].x;
					unkY = beta * dy1 + (*c1)[j-1].y;
					foundUnk = true;
					//g_print("*");
				}
				else
					printf("Error in medial axis code UNK");
			}
			else if ((dot1 < 0.0) && (dot2 < 0.0))
			{
				// move forward along unknown
				j++;
				//g_print("F");
				if (j > end1)
					done = true;
			}
			else if ((dot1 > 0.0) && (dot2 > 0.0))
			{
				// back up on unknown
				if (backJ < 50)//***1.75 - new constraint
				{
					j--;
					backJ++;
				}
				//g_print("B");
				if ((j < 1) || (backJ >= 50)) // new constraint
					done = true;
			}
			else
				printf("Error in medial axis code UNK2");
		}

		while ((! foundDB) && (! done))
		{		
			double 
				dot1 = ((*c2)[i-1].x - (*midPt)[k].x) * mdx
				     + ((*c2)[i-1].y - (*midPt)[k].y) * mdy,
				dot2 = ((*c2)[i].x - (*midPt)[k].x) * mdx
				     + ((*c2)[i].y - (*midPt)[k].y) * mdy;
			if (((dot1 <= 0.0) && (0.0 <= dot2)) || ((dot2 <= 0.0) && (0.0 <= dot1)))
			{
				// this segment contains a point of intersection with the perpendicular from
				// the medial axis at point k
				// slope of database fin outline segment between points i-1 and i 
				double
					dx2 = (*c2)[i].x - (*c2)[i-1].x,
					dy2 = (*c2)[i].y - (*c2)[i-1].y;

				double beta = 0.0;

				if ((mdx * dx2 + mdy * dy2) != 0.0)
					beta = - (mdx * ((*c2)[i-1].x - (*midPt)[k].x) + mdy * ((*c2)[i-1].y - (*midPt)[k].y))
					       / (mdx * dx2 + mdy * dy2) ;

				if ((0.0 <= beta) && (beta <= 1.0))
				{
					// found the point on this database segment
					dbX = beta * dx2 + (*c2)[i-1].x;
					dbY = beta * dy2 + (*c2)[i-1].y;
					foundDB = true;
					//g_print("+");
				}
				else
					printf("Error in medial axis code DB");
			}
			else if ((dot1 < 0.0) && (dot2 < 0.0))
			{
				// move forward along database
				i++;
				//g_print("f");
				if (i > end2)
					done = true;
			}
			else if ((dot1 > 0.0) && (dot2 > 0.0))
			{
				// back up on database
				if (backI < 50) //***1.75 - new constraint
				{
					i--;
					backI++;
				}
				//g_print("b");
				if ((i < 1) || (backI >= 50)) // new constraint
					done = true;
			}
			else
				printf("Error in medial axis code DB2");
		}

		if (foundDB && foundUnk) // new constraint test
		{
			mUnkErrPts.addPoint(unkX, unkY); //***1.75
			mSelErrPts.addPoint(dbX, dbY);   //***1.75

			sum += ((dbX - unkX) * (dbX - unkX) + (dbY - unkY) * (dbY - unkY));
		       
			ptsFound++;

			//***1.75 - keep track of where we start next search
			iPrev = i;
			jPrev = j;
		}
		else
		{
			// could not find corresponding point on one of the outlines
			// so skip this medial axis point and move on to next one
			// but reset the unknonwn and database contour indices back to 
			// what they were BEFORE the failure
			i = iPrev;
			j = jPrev;
			//g_print("_");
			done = false; // force search to continue
		}

	}
 	
	delete midPt;

	// if no points found, the error stays the default
	if (ptsFound > 0)
		error = (sum / (double) ptsFound);

#ifdef DEBUG
	printf("pair matched (medial): %d points %f error\n",ptsFound,error);
#endif

}
