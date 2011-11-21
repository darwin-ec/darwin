//*******************************************************************
//   file: NoMatchWindow.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman
//         -- comment blocks added
//
// Pops up a dialog box that lets the user add a new fin to the database 
// after matching 
//
//*******************************************************************

#include <gdk/gdkkeysyms.h>
#include "../support.h"

#include "NoMatchWindow.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#include "../../pixmaps/add_database.xpm"
#include "../../pixmaps/cancel.xpm"

#include "../DatabaseFin.h"
#include "../image_processing/transform.h"

#include "ErrorDialog.h"
#include "ResizeDialog.h"
#include "SaveFileSelectionDialog.h"

#ifdef WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "/"
#endif

using namespace std;
static const string ERROR_MSG_NO_IDCODE = "You must enter an ID Code before\n"
                                           "you can add a fin to the database.";

static const int POINT_SIZE = 2;


static int gNumReferences = 0;

//*******************************************************************
//
//
int getNumNoMatchWindowReferences()
{
	return gNumReferences;
}

//*******************************************************************
//
//
NoMatchWindow::NoMatchWindow(
		DatabaseFin<ColorImage> *Fin,			   
		Database *db,
		MainWindow *m,					//***004CL
		Options *o                    //***054
)
//  : mNonZoomedImage(new ColorImage(Fin->mImageFilename)),
//    mImage(new ColorImage(Fin->mImageFilename)),
  : mNonZoomedImage(new ColorImage(Fin->mModifiedFinImage)), //***1.8 - use actual image from tracing/matching
    mImage(new ColorImage(Fin->mModifiedFinImage)),  //***1.8 - use actual image from tracing/matching
    mDatabase(db),
    mMainWin(m),
	mOptions(o),  //***054
    mFin(Fin),
    mFinOutline(Fin->mFinOutline),    //***003TW just pass pointer
    //mWindow(createNoMatchWindow(Fin->mImageFilename)), //***054 - moved below
    mQuestionDialog(NULL),
    mIgnoreConfigureEventCnt(0),
    mSWWidthMax(-1),
    mSWHeightMax(-1),
    mImagefilename(Fin->mImageFilename),
	mSaveAsAlternate(Fin->mIsAlternate) //***1.95
{
	mWindow = createNoMatchWindow(Fin->mImageFilename);
	gNumReferences++;
}


//*******************************************************************
//
//
NoMatchWindow::~NoMatchWindow()
{
	if (NULL != mWindow)
		gtk_widget_destroy(mWindow);

	delete mImage;
	delete mNonZoomedImage;
        delete mFin;
	//delete mContour;   //***003TW use pointer instead of new contour
	if (NULL != mQuestionDialog)
		gtk_widget_destroy(mQuestionDialog);

	gNumReferences--;
}

//*******************************************************************
//
//
void NoMatchWindow::show()
{
	gtk_widget_show(mWindow);

	//***1.8 - must set drawable size so entire image is drawn,
	//         otherwise, the scrolling will not work since the "hidden"
	//         part of the image is never drawn
	gtk_drawing_area_size(
				GTK_DRAWING_AREA(mDrawingArea),
				mImage->getNumCols(),
				mImage->getNumRows());

}

//*******************************************************************
//
//
void NoMatchWindow::zoomUpdate(bool setSize, int x, int y)
{
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

	if (setSize) {
		if (x < 0) x = 0;
		if (y < 0) y = 0;

		GtkAdjustment *vAdj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(mScrolledWindow));
		GtkAdjustment *hAdj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(mScrolledWindow));
		gtk_adjustment_set_value(hAdj, x - mScrolledWindow->allocation.width / 2);
		gtk_adjustment_set_value(vAdj, y - mScrolledWindow->allocation.height / 2);
	}

}


//*******************************************************************
//
//
void NoMatchWindow::refreshImage()
{
	on_nomatchDrawingArea_expose_event(NULL, NULL, (void *)this);
}


//*******************************************************************
//
//
GtkWidget *NoMatchWindow::createNoMatchWindow(const string &title)
{
	GtkWidget *nomatchWindow;
	GtkWidget *nomatchHBoxMain;
	GtkWidget *nomatchAlignment;
	GtkWidget *nomatchVBoxLeft;
	GtkWidget *hseparator1;
	GtkWidget *nomatchViewPort;
	GtkWidget *nomatchEventBox;
	GtkWidget *nomatchRightFrame;
	GtkWidget *nomatchVBoxRight;
	GtkWidget *nomatchLabelKnownInfo;
	GtkWidget *nomatchLabelIDCode;
	GtkWidget *nomatchLabelName;
	GtkWidget *nomatchLabelDate;
	GtkWidget *nomatchLabelRoll;
	GtkWidget *nomatchLabelLocation;
	GtkWidget *nomatchLabelDamage;
	GtkWidget *nomatchLabelDescription;
	GtkWidget *nomatchVButtonBox;
	guint nomatchButtonAddToDatabase_key;
	GtkWidget *nomatchButtonAddToDatabase;
	guint nomatchButtonCancel_key;
	GtkWidget *nomatchButtonCancel;
	GtkAccelGroup *accel_group;
	GtkTooltips *tooltips;
	GtkWidget *tmpBox, *tmpLabel, *tmpIcon;

	tooltips = gtk_tooltips_new();

	accel_group = gtk_accel_group_new();

	nomatchWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_object_set_data(GTK_OBJECT(nomatchWindow), "nomatchWindow",
				nomatchWindow);
	gtk_window_set_position (GTK_WINDOW (nomatchWindow), GTK_WIN_POS_CENTER);
	gtk_window_set_policy(GTK_WINDOW(nomatchWindow), TRUE, TRUE, FALSE);
	gtk_window_set_title(GTK_WINDOW(nomatchWindow), title.c_str()); //***1.95
	gtk_window_set_title(GTK_WINDOW(nomatchWindow), _("Add Fin Image to Database!")); //***1.95
	gtk_window_set_wmclass(GTK_WINDOW(nomatchWindow), "darwin_nomatch", "DARWIN");

	nomatchHBoxMain = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(nomatchHBoxMain);
	gtk_container_add(GTK_CONTAINER(nomatchWindow), nomatchHBoxMain);

	nomatchAlignment = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_widget_show(nomatchAlignment);
	gtk_box_pack_start(GTK_BOX(nomatchHBoxMain), nomatchAlignment, TRUE, TRUE,
				0);

	nomatchVBoxLeft = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(nomatchVBoxLeft);
	gtk_container_add(GTK_CONTAINER(nomatchAlignment), nomatchVBoxLeft);

	//***1.95 - put filename in as label
	string name = "IMAGE FILE: ";
	name += title.substr(title.rfind(PATH_SLASH)+1);
	GtkWidget *fnameLabel = gtk_label_new(name.c_str());
	gtk_widget_show(fnameLabel);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxLeft), fnameLabel, FALSE, TRUE,
				5);

	hseparator1 = gtk_hseparator_new();
	gtk_widget_show(hseparator1);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxLeft), hseparator1, FALSE, TRUE,
				0);

	mScrolledWindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(mScrolledWindow);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxLeft), mScrolledWindow, TRUE,
				TRUE, 0);
	//***1.8 - this allows the scroller bars to appear/disappear automatically
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mScrolledWindow), 
				GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	nomatchViewPort = gtk_viewport_new(NULL, NULL);
	gtk_widget_show(nomatchViewPort);
	gtk_container_add(GTK_CONTAINER(mScrolledWindow), nomatchViewPort);

	nomatchEventBox = gtk_event_box_new();
	gtk_widget_show(nomatchEventBox);
	gtk_container_add(GTK_CONTAINER(nomatchViewPort), nomatchEventBox);

	mDrawingArea = gtk_drawing_area_new();
	gtk_widget_show(mDrawingArea);
	gtk_container_add(GTK_CONTAINER(nomatchEventBox), mDrawingArea);

	nomatchVBoxRight = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(nomatchVBoxRight), 4);
	gtk_widget_show(nomatchVBoxRight);

	nomatchRightFrame = gtk_frame_new(NULL);
	gtk_widget_show(nomatchRightFrame);
	gtk_container_add(GTK_CONTAINER(nomatchRightFrame), nomatchVBoxRight);
    
	gtk_box_pack_start(GTK_BOX(nomatchHBoxMain), nomatchRightFrame, FALSE, TRUE, 0);

	// confusing changes in font management here - JHS
	GtkStyle *infoStyle = gtk_style_new();
	//gdk_font_unref(infoStyle->font);
	//infoStyle->font = gdk_font_load("-*-helvetica-bold-r-*-*-*-160-*-*-*-*-*-*");

	//if (!infoStyle->font)
	//   infoStyle->font = gdk_font_load("fixed");

	// new replacement code (next 4 lines) - JHS
	GdkFont *infoFont = gdk_font_load("-*-helvetica-bold-r-*-*-*-160-*-*-*-*-*-*");
	if (!infoFont)
		infoFont = gdk_font_load("fixed");
	gtk_style_set_font(infoStyle, infoFont);

	nomatchLabelKnownInfo = gtk_label_new(_("Known Information"));
	gtk_widget_set_style(nomatchLabelKnownInfo, infoStyle);
	gtk_widget_show(nomatchLabelKnownInfo);
	gtk_style_unref(infoStyle);

	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), nomatchLabelKnownInfo, FALSE,
				FALSE, 5);

	nomatchLabelIDCode = gtk_label_new(_("ID Code"));
	gtk_widget_show(nomatchLabelIDCode);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), nomatchLabelIDCode, FALSE,
				FALSE, 2);
    
	mEntryID = gtk_entry_new();
	if (mFin->getID() != "NONE")
		gtk_entry_set_text(GTK_ENTRY(mEntryID), mFin->getID().c_str());
	gtk_widget_show(mEntryID);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), mEntryID, FALSE, FALSE,
				0);

	nomatchLabelName = gtk_label_new(_("Name"));
	gtk_widget_show(nomatchLabelName);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), nomatchLabelName, FALSE,
				FALSE, 2);

	mEntryName = gtk_entry_new();
	if (mFin->getName() != "NONE")
		gtk_entry_set_text(GTK_ENTRY(mEntryName), mFin->getName().c_str());
	gtk_widget_show(mEntryName);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), mEntryName, FALSE,
				FALSE, 0);

	nomatchLabelDate = gtk_label_new(_("Date of Sighting"));
	gtk_widget_show(nomatchLabelDate);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), nomatchLabelDate, FALSE,
				FALSE, 2);

	mEntryDate = gtk_entry_new();
	if (mFin->getDate() != "NONE")
		gtk_entry_set_text(GTK_ENTRY(mEntryDate), mFin->getDate().c_str());
	gtk_widget_show(mEntryDate);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), mEntryDate, FALSE,
				FALSE, 0);

	nomatchLabelRoll = gtk_label_new(_("Roll/Frame or Lat/Long"));
	gtk_widget_show(nomatchLabelRoll);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), nomatchLabelRoll, FALSE,
				FALSE, 3);

	mEntryRoll = gtk_entry_new();
	if (mFin->getRoll() != "NONE")
		gtk_entry_set_text(GTK_ENTRY(mEntryRoll), mFin->getRoll().c_str());
	gtk_widget_show(mEntryRoll);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), mEntryRoll, FALSE,
				FALSE, 0);

	nomatchLabelLocation = gtk_label_new(_("Location Code"));
	gtk_widget_show(nomatchLabelLocation);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), nomatchLabelLocation, FALSE,
				FALSE, 2);

	mEntryLocation = gtk_entry_new();
	if (mFin->getLocation() != "NONE")
		gtk_entry_set_text(GTK_ENTRY(mEntryLocation), mFin->getLocation().c_str());
	gtk_widget_show(mEntryLocation);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), mEntryLocation, FALSE,
				FALSE, 0);

	nomatchLabelDamage = gtk_label_new(_("Damage Category"));
	gtk_widget_show(nomatchLabelDamage);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), nomatchLabelDamage, FALSE,
				FALSE, 2);

	//***054
	//
	// a new approach based on a scroll-down menu
	//
	mEntryDamage = gtk_combo_box_new_text();
	gtk_widget_show(mEntryDamage);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), mEntryDamage, FALSE,
				FALSE, 0);

	string damageStr = mFin->getDamage();

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
		if (damageStr == mDatabase->catCategoryName(catIDnum))
			gtk_combo_box_set_active(GTK_COMBO_BOX(mEntryDamage),catIDnum);
	}
	//
	// end of new code for damage categories
	//

	nomatchLabelDescription = gtk_label_new(_("Short Description"));
	gtk_widget_show(nomatchLabelDescription);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), nomatchLabelDescription,
				FALSE, FALSE, 2);

	mEntryDescription = gtk_entry_new();
	if (mFin->getShortDescription() != "NONE")
		gtk_entry_set_text(GTK_ENTRY(mEntryDescription), mFin->getShortDescription().c_str());
	gtk_widget_show(mEntryDescription);
	gtk_box_pack_start(GTK_BOX(nomatchVBoxRight), mEntryDescription,
				FALSE, FALSE, 0);

	//***1.95 -  checkbox to control designation of fin/image as PRIMARY/ALTERNATE
	GtkWidget *primAlt = gtk_check_button_new_with_label(_("Add Fin/Image as ALTERNATE!"));
	gtk_widget_show(primAlt);
	if (mSaveAsAlternate)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(primAlt), TRUE);
	gtk_box_pack_start(GTK_BOX (nomatchVBoxRight), primAlt, FALSE, FALSE, 6);

	gtk_signal_connect(GTK_OBJECT(primAlt), "toggled",
		       GTK_SIGNAL_FUNC
		       (on_primaryAlternateCheckButton_toggled), (void*)this);

	nomatchVButtonBox = gtk_vbutton_box_new();
	gtk_widget_show(nomatchVButtonBox);
	gtk_box_pack_end(GTK_BOX(nomatchVBoxRight), nomatchVButtonBox, FALSE, TRUE,
				0);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(nomatchVButtonBox),
				GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(nomatchVButtonBox), 3);

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, add_database_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);
    
	nomatchButtonAddToDatabase = gtk_button_new();
	nomatchButtonAddToDatabase_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
					_("_Add To Database"));
	gtk_widget_add_accelerator(nomatchButtonAddToDatabase, "clicked", accel_group,
					nomatchButtonAddToDatabase_key, GDK_MOD1_MASK,
					(GtkAccelFlags) 0);
	gtk_container_add(GTK_CONTAINER(nomatchButtonAddToDatabase), tmpBox);
	gtk_widget_show(nomatchButtonAddToDatabase);
	gtk_container_add(GTK_CONTAINER(nomatchVButtonBox), nomatchButtonAddToDatabase);
	GTK_WIDGET_SET_FLAGS(nomatchButtonAddToDatabase, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip(tooltips, nomatchButtonAddToDatabase,
					_("Add this fin to the database."), NULL);
	gtk_widget_add_accelerator(nomatchButtonAddToDatabase, "clicked", accel_group,
					GDK_S, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

    
	nomatchButtonCancel = gtk_button_new();
	nomatchButtonCancel_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
					_("_Cancel"));
	gtk_widget_add_accelerator(nomatchButtonCancel, "clicked", accel_group,
					nomatchButtonCancel_key, GDK_MOD1_MASK,
					(GtkAccelFlags) 0);
	gtk_container_add(GTK_CONTAINER(nomatchButtonCancel), tmpBox);
	gtk_widget_show(nomatchButtonCancel);
	gtk_container_add(GTK_CONTAINER(nomatchVButtonBox), nomatchButtonCancel);
	GTK_WIDGET_SET_FLAGS(nomatchButtonCancel, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip(tooltips, nomatchButtonCancel,
					_("Close this window and discard any work you've done."),
					NULL);
	gtk_widget_add_accelerator(nomatchButtonCancel, "clicked", accel_group,
					GDK_C, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

	gtk_signal_connect(
			GTK_OBJECT(nomatchWindow),
			"delete_event",
			GTK_SIGNAL_FUNC(on_nomatchWindow_delete_event),
			(void*)this);

    
	gtk_signal_connect(
			GTK_OBJECT(mDrawingArea),
			"expose_event",
			GTK_SIGNAL_FUNC(on_nomatchDrawingArea_expose_event),
			(void *) this);

	gtk_signal_connect(
			GTK_OBJECT(mDrawingArea),
			"configure_event",
			GTK_SIGNAL_FUNC(on_n_mDrawingArea_configure_event),
			(void *) this);

	gtk_signal_connect(
			GTK_OBJECT(mScrolledWindow),
			"configure_event",
			GTK_SIGNAL_FUNC(on_n_mScrolledWindow_configure_event),
			(void *) this);

	gtk_signal_connect(
			GTK_OBJECT(nomatchButtonAddToDatabase),
			"clicked",
			GTK_SIGNAL_FUNC(on_nomatchButtonAddToDatabase_clicked),
			(void *) this);

	gtk_signal_connect(
			GTK_OBJECT(nomatchButtonCancel),
			"clicked",
			GTK_SIGNAL_FUNC(on_nomatchButtonCancel_clicked),
			(void *) this);

	gtk_widget_grab_default(nomatchButtonAddToDatabase);
	gtk_object_set_data(GTK_OBJECT(nomatchWindow), "tooltips", tooltips);

	gtk_window_add_accel_group(GTK_WINDOW(nomatchWindow), accel_group);
	gtk_widget_set_usize(mScrolledWindow, 400, 400);
	return nomatchWindow;
}


//*******************************************************************
//
//
gboolean on_nomatchWindow_delete_event(GtkWidget * widget,
				     GdkEvent * event, gpointer userData)
{
	NoMatchWindow *nomatchWin = (NoMatchWindow *) userData;
	
	if (NULL == nomatchWin)
		return FALSE;

	delete nomatchWin;

	return TRUE;
}


//*******************************************************************
//
//
gboolean on_nomatchDrawingArea_expose_event(GtkWidget * widget,
					  GdkEventExpose * event,
					  gpointer userData)
{
	NoMatchWindow *nomatchWin = (NoMatchWindow *) userData;
	
	if (NULL == nomatchWin)
		return FALSE;

	if (NULL == nomatchWin->mDrawingArea || NULL == nomatchWin->mImage)
		return FALSE;

	nomatchWin->mZoomXOffset =
		(nomatchWin->mDrawingArea->allocation.width - 
		 (int) nomatchWin->mImage->getNumCols()) / 2;

	nomatchWin->mZoomYOffset =
		(nomatchWin->mDrawingArea->allocation.height -
		 (int) nomatchWin->mImage->getNumRows()) / 2;

	// Just in case some funky allocation comes through
	if (nomatchWin->mZoomXOffset < 0)
		nomatchWin->mZoomXOffset = 0;
	if (nomatchWin->mZoomYOffset < 0)
		nomatchWin->mZoomYOffset = 0;

	gdk_draw_rgb_image(
		nomatchWin->mDrawingArea->window,
		nomatchWin->mDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
		nomatchWin->mZoomXOffset, nomatchWin->mZoomYOffset,
		nomatchWin->mImage->getNumCols(),
		nomatchWin->mImage->getNumRows(),
		GDK_RGB_DITHER_NONE,
		(guchar*)nomatchWin->mImage->getData(),
		nomatchWin->mImage->bytesPerPixel() * nomatchWin->mImage->getNumCols());

	return TRUE;
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
string NoMatchWindow::copyImageToDestinationAs(string srcName, string destPath)
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
// string nextAvailableDestinationFilename(string destName)
//
// This finds an available name for the new image to be saved,
// by appending a number to the root image filename COPY such that 
// no existing image will be destroyed in the destination folder.  
// The available name of the new image file COPY in the destination 
// folder is returned.
//
// (srcName) should be the complete path/name of the proposed NEW image file
//
string NoMatchWindow::nextAvailableDestinationFilename(string destName)
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
void on_nomatchButtonAddToDatabase_clicked(GtkButton * button, gpointer userData)
{
	NoMatchWindow *nomatchWin = (NoMatchWindow *) userData;

	if (NULL == nomatchWin)
		return;
	
	// ***055 - must select a damage category
	int damageIDnum = gtk_combo_box_get_active(GTK_COMBO_BOX(nomatchWin->mEntryDamage));
	if (-1 == damageIDnum)
	{
		//showError("You must select a Catalog Category\nBEFORE saving a fin!");
		return;
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

		//normalizeContour(nomatchWin->mContour);

		temp = gtk_editable_get_chars(
				GTK_EDITABLE(nomatchWin->mEntryID),
				0, -1);
		id = temp;
		g_free(temp);

		temp = gtk_editable_get_chars(
				GTK_EDITABLE(nomatchWin->mEntryName),
				0, -1);
		name = temp;
		g_free(temp);

		temp = gtk_editable_get_chars(
				GTK_EDITABLE(nomatchWin->mEntryDate),
				0, -1);
		date = temp;
		g_free(temp);

		temp = gtk_editable_get_chars(
				GTK_EDITABLE(nomatchWin->mEntryRoll),
				0, -1);
		roll = temp;
		g_free(temp);

		temp = gtk_editable_get_chars(
				GTK_EDITABLE(nomatchWin->mEntryLocation),
				0, -1);
		location = temp;
		g_free(temp);

		/* old method
		temp = gtk_editable_get_chars(
				GTK_EDITABLE(nomatchWin->mEntryDamage),
				0, -1);
		damage = temp;
		g_free(temp);
		*/

		//***054 - new method with drop-down selection
		// cannot use gtk_combo_box_get_active_text() until we upgrade to GTK 2.6+
		//int damageIDnum = gtk_combo_box_get_active(GTK_COMBO_BOX(nomatchWin->mEntryDamage)); //***051
		damage = "";
		if (damageIDnum != -1)
			damage = nomatchWin->mDatabase->catCategoryName(damageIDnum); //***051


		temp = gtk_editable_get_chars(
				GTK_EDITABLE(nomatchWin->mEntryDescription),
				0, -1);
		description = temp;
		g_free(temp);


		//***1.4 - changed from ... if ("NONE" == id)
		if ("" == id)
		{
			showError (ERROR_MSG_NO_IDCODE);
			return;
		}

		//***1.4 - new test
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

		//***054 - code to copy image to catalog and use short filename

		//***1.8 - either we are copying existing MODIFIED and ORIGINAL image
		//         files to the catalog (mFin->mImageFilename != mFin->mOriginalImageFilename)
		//         OR we are copying an ORIGINAL and creating a MODIFIED file
		//         (when mFin->mImageFilename == mFin->mOriginalImageFilename)
		//

		// set up the destination PATH for the catalog

		//***1.85 - everything is now relative to the current survey area
		string catalogPath = gOptions->mCurrentSurveyArea;
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
			
		if ("" != nomatchWin->mFin->mOriginalImageFilename)
		{
			originalShortFilename = nomatchWin->mFin->mOriginalImageFilename;
			originalShortFilename = originalShortFilename.substr(
					originalShortFilename.rfind(PATH_SLASH)+1);

			copyFilename = nomatchWin->copyImageToDestinationAs(
					nomatchWin->mFin->mOriginalImageFilename,
					catalogPath);

			copyShortFilename = copyFilename.substr(copyFilename.rfind(PATH_SLASH)+1);
		}
		//***1.96a - end of new code segment

		string modFilename;
		if ((nomatchWin->mFin->mImageFilename == nomatchWin->mFin->mOriginalImageFilename) ||
			(copyShortFilename != originalShortFilename)) //***1.96a - name changed during copy
		{
			//***1.96a - either the original image filename changed during copy,
			// or there never was a modified image saved -- in both cases we
			// must create and save a new modified image file containing the correct
			// name of the original image file COPY in the catalog folder

			// create a new MODIFIED image file in the catalog folder
			if ((NULL != nomatchWin->mFin) &&
				(NULL != nomatchWin->mFin->mModifiedFinImage))
			{
				// base the filename of the modified image on the renamed original image copy
				int pos = copyFilename.rfind('.');
				modFilename = copyFilename.substr(0,pos) + "_wDarwinMods.png";

				modFilename = nomatchWin->nextAvailableDestinationFilename(modFilename); //***1.96a

				printf("creating new imagefile: \"%s\"\n",modFilename.c_str());

				// save modified image to catalog folder
				nomatchWin->mFin->mModifiedFinImage->save_wMods( //***1.8 - new save modified image call
						modFilename,    // the filename of the modified image
						copyShortFilename,   //***1.96a - the filename of the original image
						nomatchWin->mFin->mImageMods); // the list of image modifications
			}

		}
		else
		{

			// copy the existing MODIFIED image file to the catalog folder

			modFilename = nomatchWin->copyImageToDestinationAs(
					nomatchWin->mFin->mImageFilename, 
					catalogPath);
		}

		// now add the new FIN to the database file, with the MODIFIED image filename
		// as a part of the entry

		DatabaseFin<ColorImage> *newFin = new DatabaseFin<ColorImage>(
				modFilename, //***1.8
				nomatchWin->mFinOutline, //***008OL
				id,
				name,
				date,
				roll,
				location,
				damage,
				description);

		newFin->mIsAlternate = nomatchWin->mSaveAsAlternate; //***1.95 - add with status

		nomatchWin->mDatabase->add(newFin);
		delete newFin;
	
		nomatchWin->mMainWin->refreshDatabaseDisplayNew(true); //***1.96a
		nomatchWin->mMainWin->selectFromReorderedCList(modFilename); //***1.8


		delete nomatchWin;
	} catch (Error e) {
		showError(e.errorString());
		delete nomatchWin;
	}
}

//*******************************************************************
//
//
void on_nomatchButtonCancel_clicked(GtkButton * button, gpointer userData)
{
	NoMatchWindow *nomatchWin = (NoMatchWindow *) userData;
	
	if (NULL == nomatchWin)
		return;

	delete nomatchWin;
}


//*******************************************************************
//
//
gboolean on_n_mDrawingArea_configure_event(
		GtkWidget *widget,
		GdkEventConfigure *event,
		gpointer userData)
{
	NoMatchWindow *nomatchWin = (NoMatchWindow *)userData;

	if (NULL == nomatchWin)
		return FALSE;

	// We want to ignore configure events right after a zoom.
	// zoomUpdate will set the ignore flag when it runs.  (We only
	// want to handle configure events that are generated by the
	// user resizing the window.)
	if (nomatchWin->mIgnoreConfigureEventCnt) {
		nomatchWin->mIgnoreConfigureEventCnt--;
		return TRUE;
	}	

	// Try to figure out what the zoom ratio should be to fit
	// the image inside the drawing area
	float heightRatio, widthRatio;
	
	heightRatio = (float)nomatchWin->mDrawingArea->allocation.height /
		nomatchWin->mNonZoomedImage->getNumRows();
	widthRatio = (float)nomatchWin->mDrawingArea->allocation.width / 
		nomatchWin->mNonZoomedImage->getNumRows();

	if (heightRatio < 1.0f && widthRatio < 1.0f) {
		if (widthRatio < heightRatio) {
			// the width is the restrictive dimension
			if (widthRatio >= 0.5f)
				nomatchWin->mZoomRatio = 50;
			else
				nomatchWin->mZoomRatio = 25;
		} else if (heightRatio >= 0.5f)
			nomatchWin->mZoomRatio = 50;
		else
			nomatchWin->mZoomRatio = 25;
	} else
		nomatchWin->mZoomRatio = 100;

	nomatchWin->zoomUpdate(false);

	return TRUE;
}

//*******************************************************************
//
//
gboolean on_n_mScrolledWindow_configure_event(
	GtkWidget *widget,
	GdkEventConfigure *event,
	gpointer userData)
{
	NoMatchWindow *nomatchWin = (NoMatchWindow *) userData;

	if (widget->allocation.width > nomatchWin->mSWWidthMax)
		nomatchWin->mSWWidthMax = widget->allocation.width;

	if (widget->allocation.height > nomatchWin->mSWHeightMax)
		nomatchWin->mSWHeightMax = widget->allocation.height;

	return TRUE;
}

//*******************************************************************
//
//
void on_primaryAlternateCheckButton_toggled( //***1.95
	GtkButton *button,
	gpointer userData)
{
	NoMatchWindow *dlg = (NoMatchWindow *) userData;

	if (NULL == dlg)
		return;

   // flip the display indication
   dlg->mSaveAsAlternate = !(dlg->mSaveAsAlternate);
}

