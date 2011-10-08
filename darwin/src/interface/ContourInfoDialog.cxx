//*******************************************************************
//   file: ContourInfoDialog.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (4/12/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <cmath>
#include <cstdio>

#include "../support.h"
#include "ContourInfoDialog.h"
#include "../Chain.h" //***008OL
#include "../utility.h"
#include "../feature.h"
#include "../../pixmaps/close.xpm"
#include "../waveletUtil.h"

using namespace std;

static const double LE_ANGLE_LINE_RADIUS = 150.0;
static const int DA_WIDTH = 300, DA_HEIGHT = 300;
static const int POINT_SIZE = 2;
static const float CHAIN_POINTS_DISTANCE = 3.0; //***005CI

static int gNumReferences = 0;


//*******************************************************************
//
// int getNumContourInfoDialogReferences()
//
//    Return count of ContourInfoDialogs currently existing.
//
int getNumContourInfoDialogReferences()
{
	return gNumReferences;
}


//*******************************************************************
//
// ContourInfoDialog::ContourInfoDialog(...)
//
//    CONSTRUCTOR
//
ContourInfoDialog::ContourInfoDialog(
		string name,
		Outline *oL, //***08OL - replaced Contour
		double outlineColor[4]
)
	: mName(name),
	  mDialog(createInfoDialog()),
	  mFinOutline(new Outline(oL)), //***1.3 - make a copy now
	  mContourGC(NULL),
	  mChainGC(NULL),
	  mHighlightGC(NULL)
{
	if (NULL == oL)
		throw EmptyArgumentError("ContourInfoDialog ctor.");

	memcpy(mTraceColor, outlineColor, 4 * sizeof(double));

	//***008OL Chain and feature points are now part of Outline 
	//         and there is no longer any need to compute them here

	gNumReferences++;
}


//*******************************************************************
//
// ContourInfoDialog::~ContourInfoDialog()
//
//    DESTRUCTOR.
//
ContourInfoDialog::~ContourInfoDialog()
{
	if (NULL != mDialog)
		gtk_widget_destroy(mDialog);

	//***1.3 - delete Outline copy
	if (NULL != mFinOutline)
		delete mFinOutline;

	if (NULL != mContourGC)
		gdk_gc_unref(mContourGC);
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
void ContourInfoDialog::show()
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
void ContourInfoDialog::updateGC()
{
	if (NULL == mContourGC) {
		if (NULL == mDrawingAreaContour)
			return;

		mContourGC = gdk_gc_new(mDrawingAreaContour->window);
	}

	updateGCColor(mContourGC);

	if (NULL == mHighlightGC) {
		if (NULL == mDrawingAreaContour)
			return;

		mHighlightGC = gdk_gc_new(mDrawingAreaContour->window);
	}
	double highlightColor[] = {1.0, 0.0, 0.0, 0.0};
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
void ContourInfoDialog::updateGCColor(GdkGC *gc, double color[4])
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
void ContourInfoDialog::updateGCColor(GdkGC *gc)
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
void ContourInfoDialog::updateInfo()
{
	char info[2048];
	sprintf(
		info,
		"%s\n\n"
		"Outline - Number of Points : %d\n"
		"Chain Code - Number of Points : %d\n\n"
		"\tTip at chain position %d\n"
		"\tLargest notch on trailing edge at chain position: %d\n"
		"\tAngle of leading edge: %f\n"
		"\tLeading edge cutoff: %d\n"
		"\tLeading edge end: %d\n"
		"\tPoint of inflection on trailing edge: %d",
		mName.c_str(),
    mFinOutline->length(), //***008OOL
    mFinOutline->length(), //***008OL
		mFinOutline->getFeaturePoint(TIP), //***008OOL
 		mFinOutline->getFeaturePoint(NOTCH), //***008OOL
 		mFinOutline->getLEAngle(), //***008OOL
 		mFinOutline->getFeaturePoint(LE_BEGIN), //***008OOL
 		mFinOutline->getFeaturePoint(LE_END), //***008OOL
 		mFinOutline->getFeaturePoint(POINT_OF_INFLECTION) //***008OOL
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
GtkWidget* ContourInfoDialog::createInfoDialog()
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
  gtk_window_set_wmclass(GTK_WINDOW(infoDialog), "darwin_continfo", "DARWIN");

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

  infoButtonGenCoeffFiles = gtk_button_new_with_label(_("Generate Coefficient Files"));
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
                      GTK_SIGNAL_FUNC (on_infoDialog_delete_event),
                      (void *) this);
  gtk_signal_connect (GTK_OBJECT (mDrawingAreaContour), "expose_event",
                      GTK_SIGNAL_FUNC (on_infoContourDrawingArea_expose_event),
                      (void *) this);
  gtk_signal_connect (GTK_OBJECT (mDrawingAreaChain), "expose_event",
                      GTK_SIGNAL_FUNC (on_infoChainDrawingArea_expose_event),
                      (void *) this);
  gtk_signal_connect (GTK_OBJECT (infoButtonOK), "clicked",
                      GTK_SIGNAL_FUNC (on_infoButtonOK_clicked),
                      (void *) this);

  gtk_signal_connect(GTK_OBJECT(infoButtonGenCoeffFiles), "clicked",
		  GTK_SIGNAL_FUNC(on_infoButtonGenCoeffFiles_clicked),
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
gboolean on_infoDialog_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	ContourInfoDialog *infoDialog = (ContourInfoDialog *)userData;

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
gboolean on_infoContourDrawingArea_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData)
{
	ContourInfoDialog *infoDialog = (ContourInfoDialog *)userData;

	if (NULL == infoDialog)
		return FALSE;

	if (NULL == infoDialog->mFinOutline) //***008OL
		return FALSE;

	FloatContour *c = infoDialog->mFinOutline->getFloatContour(); //***008OL;

	int
		xMax = c->maxX(),
		yMax = c->maxY(),
		xMin = c->minX(),
		yMin = c->minY();

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
		
	for (unsigned i = 0; i < numPoints; i++) {	
		int xCoord = (int) round(((*c)[i].x - xMin) * ratio + xOffset);
		int yCoord = (int) round(((*c)[i].y - yMin) * ratio + yOffset);
		
		gdk_draw_rectangle(
			infoDialog->mDrawingAreaContour->window,
			infoDialog->mContourGC,
			TRUE,
			xCoord - POINT_SIZE / 2,
			yCoord - POINT_SIZE / 2,
			POINT_SIZE,
			POINT_SIZE);
	}

	point_t p = infoDialog->mFinOutline->getSavedPoint(infoDialog->mFinOutline->getFeaturePoint(TIP)); //***008OL

	int xC = (int) round((p.x - xMin) * ratio + xOffset);
	int yC = (int) round((p.y - yMin) * ratio + yOffset);
	int highlightPointSize = POINT_SIZE * 2;
	gdk_draw_rectangle(
		infoDialog->mDrawingAreaContour->window,
		infoDialog->mHighlightGC,
		TRUE,
		xC - highlightPointSize / 2,
		yC - highlightPointSize / 2,
		highlightPointSize,
		highlightPointSize);
	
	p = infoDialog->mFinOutline->getSavedPoint(infoDialog->mFinOutline->getFeaturePoint(NOTCH)); //***008OL
	xC = (int) round((p.x - xMin) * ratio + xOffset);
	yC = (int) round((p.y - yMin) * ratio + yOffset);

	gdk_draw_rectangle(
		infoDialog->mDrawingAreaContour->window,
		infoDialog->mHighlightGC,
		TRUE,
		xC - highlightPointSize / 2,
		yC - highlightPointSize / 2,
		highlightPointSize,
		highlightPointSize);

	p = infoDialog->mFinOutline->getSavedPoint(infoDialog->mFinOutline->getFeaturePoint(POINT_OF_INFLECTION)); //***008OL
	xC = (int) round((p.x - xMin) * ratio + xOffset);
	yC = (int) round((p.y - yMin) * ratio + yOffset);

	gdk_draw_rectangle(
		infoDialog->mDrawingAreaContour->window,
		infoDialog->mHighlightGC,
		TRUE,
		xC - highlightPointSize / 2,
		yC - highlightPointSize / 2,
		highlightPointSize,
		highlightPointSize);

	p = infoDialog->mFinOutline->getSavedPoint(infoDialog->mFinOutline->getFeaturePoint(LE_BEGIN)); //***008OL
	xC = (int) round((p.x - xMin) * ratio + xOffset);
	yC = (int) round((p.y - yMin) * ratio + yOffset);

	gdk_draw_rectangle(
		infoDialog->mDrawingAreaContour->window,
		infoDialog->mHighlightGC,
		TRUE,
		xC - highlightPointSize / 2,
		yC - highlightPointSize / 2,
		highlightPointSize,
		highlightPointSize);

	p = infoDialog->mFinOutline->getSavedPoint(infoDialog->mFinOutline->getFeaturePoint(LE_END)); //***008OL
	xC = (int) round((p.x - xMin) * ratio + xOffset);
	yC = (int) round((p.y - yMin) * ratio + yOffset);

	gdk_draw_rectangle(
		infoDialog->mDrawingAreaContour->window,
		infoDialog->mHighlightGC,
		TRUE,
		xC - highlightPointSize / 2,
		yC - highlightPointSize / 2,
		highlightPointSize,
		highlightPointSize);


	// BEGIN code to draw a line indicating the angle of the leading edge.
	
	// First, we need to find a point to center it on.  We'll do
	// this by taking the point half way in between the tip and the
	// beginning of the leading edge, and finding the point on the
	// contour which is closest to this one.

	int lineXCent, lineYCent;

	p = infoDialog->mFinOutline->getSavedPoint(infoDialog->mFinOutline->getFeaturePoint(TIP));

	if (p.x > (*c)[0].x)
		lineXCent = p.x - (*c)[0].x;
	else
		lineXCent = (*c)[0].x - p.x;

	if (p.y > (*c)[0].y)
		lineYCent = p.y = (*c)[0].y;
	else
		lineYCent = (*c)[0].y - p.y;

	lineXCent /= 2;
	lineYCent /= 2;

	int pos = infoDialog->mFinOutline->getFeaturePoint(LE_END) / 2; // "halfway" up leading edge

	lineXCent = (int) round(((*c)[pos].x - xMin) * ratio + xOffset);
	lineYCent = (int) round(((*c)[pos].y - yMin) * ratio + yOffset);

	// Now that we have the center of the line, we need to figure
	// some endpoints.
	int
		preCos = (int) round(cos(dtor(infoDialog->mFinOutline->getLEAngle())) * LE_ANGLE_LINE_RADIUS),
		preSin = (int) round(sin(dtor(infoDialog->mFinOutline->getLEAngle())) * LE_ANGLE_LINE_RADIUS);
	
	int
		xEnd = preCos + lineXCent,
		xBegin = lineXCent - preCos,
		yEnd = lineYCent + preSin,
		yBegin = lineYCent - preSin;

	/*
	gdk_draw_line(
		infoDialog->mDrawingAreaContour->window,
		infoDialog->mHighlightGC,
		xBegin,
		yBegin,
		xEnd,
		yEnd);
	*/
	// END code for line indicating angle of leading edge

	return TRUE;
}


//*******************************************************************
//
// gboolean on_infoChainDrawingArea_expose_event(...)
//
//    Friend function to process ChainDrawingArea_expose events.
//
gboolean on_infoChainDrawingArea_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData)
{
	ContourInfoDialog *infoDialog = (ContourInfoDialog *)userData;

	if (NULL == infoDialog)
		return FALSE;

	if (NULL == infoDialog->mFinOutline)
		return FALSE;

	//Chain *chain = infoDialog->mFinOutline->getChain(); //***008OL
	Chain *chain = new Chain(infoDialog->mFinOutline->getChain()); //***1.5

	chain->smooth7(); //***1.5 - show smoothed version - it's more informative

	double
		yMin = chain->min(), //***008OL
		yMax = chain->max(); //***008OL

	double vertRatio, angle;
        
	if (fabs(yMax) > fabs(yMin)) {
		vertRatio = (double)DA_HEIGHT / (fabs(yMax) * 2);
		angle = fabs(yMax);
	} else {
		vertRatio =  (double)DA_HEIGHT / (fabs(yMin) * 2);
		angle = fabs(yMin);
	}

	double horizRatio =  ((double)DA_WIDTH) / chain->length();
	
	int
		prevXCoord = 0,
		prevYCoord = (int) round(fabs((*chain)[0] - angle) * vertRatio); //***008OL

	for (int i = 1; i < chain->length(); i++) {	 //***008OL
		int xCoord = (int) round(i * horizRatio);
		int yCoord = (int) round(fabs((*chain)[i] - angle) * vertRatio); //***008OL
		gdk_draw_line(
			infoDialog->mDrawingAreaChain->window,
			infoDialog->mChainGC,
			prevXCoord,
			prevYCoord,
			xCoord,
			yCoord);

		prevXCoord = xCoord;
		prevYCoord = yCoord;
	}
	
	delete chain; //***1.5

	return TRUE;
}


//*******************************************************************
//
// void on_infoButtonOK_clicked(...)
//
//    Friend function to process "OK" button click events.
//
void on_infoButtonOK_clicked(
	GtkButton *button,
	gpointer userData)
{
	ContourInfoDialog *infoDialog = (ContourInfoDialog *)userData;

	if (NULL == infoDialog)
		return;

	delete infoDialog;
}


//*******************************************************************
//
// void on_infoButtonGenCoeffFiles_clicked(...)
//
//    Friend function to process "GenCoeffFiles" button click events.
//
void on_infoButtonGenCoeffFiles_clicked(
	GtkButton *button,
	gpointer userData)
{
	ContourInfoDialog *infoDialog = (ContourInfoDialog *)userData;

	if (NULL == infoDialog)
		return;

	int numLevels = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(infoDialog->mSpinButton));

	waveGenCoeffFiles(
			infoDialog->mName,
			infoDialog->mFinOutline->getChain(), //***008OL
			numLevels);
}

