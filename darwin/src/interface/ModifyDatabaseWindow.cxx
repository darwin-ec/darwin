//*******************************************************************
//   file: ModifyDatabaseWindow.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman
//         -- comment blocks added
//         -- major changes to user interface
//         J H Stewman (9/2/2005)
//         -- pulldown selection of damage categories
//
// Pops up a dialog box that lets the user modify the database or delete a fin
//
//*******************************************************************

#include <gdk/gdkkeysyms.h>
#include "../support.h"

#include "ModifyDatabaseWindow.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../../pixmaps/add_database.xpm"
#include "../../pixmaps/question.xpm"
#include "../../pixmaps/yes.xpm"
#include "../../pixmaps/cancel.xpm"
#include "../../pixmaps/trash.xpm"
#include "../../pixmaps/save.xpm"

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

static const string ERROR_MSG_NO_IDCODE = "You must enter an ID Code!";

static const int POINT_SIZE = 2;

static int gNumReferences = 0;

//*******************************************************************
//
//
int getNumModifyDatabaseWindowReferences()
{
	return gNumReferences;
}

//*******************************************************************
//
//
ModifyDatabaseWindow::ModifyDatabaseWindow(
		int DBCurEntry,
		MainWindow *m,				//***004CL
		DatabaseFin<ColorImage> *Fin,
		Database *db,
		Options *o                  //***054
)
  : mNonZoomedImage(new ColorImage(Fin->mImageFilename)),
    mImage(new ColorImage(Fin->mImageFilename)),
    mDatabase(db),
    mFin(Fin),
    mDBCurEntry(DBCurEntry),
    mMainWin(m),
	mOptions(o), //***054 - must preceed creation of window
    //mWindow(createModifyDatabaseWindow(Fin->mImageFilename)),
    mQuestionDialog(NULL),
    mIgnoreConfigureEventCnt(0),
    mSWWidthMax(-1),
    mSWHeightMax(-1),
    mImagefilename(Fin->mImageFilename)
{
	mWindow = createModifyDatabaseWindow(Fin->mImageFilename); //***054 
	gNumReferences++;
}


//*******************************************************************
//
//
ModifyDatabaseWindow::~ModifyDatabaseWindow()
{
	if (NULL != mWindow)
		gtk_widget_destroy(mWindow);

	delete mImage;
	delete mNonZoomedImage;

	if (NULL != mQuestionDialog)
		gtk_widget_destroy(mQuestionDialog);

	gNumReferences--;
}

//*******************************************************************
//
//
void ModifyDatabaseWindow::show()
{
	gtk_widget_show(mWindow);

	//***1.8 - must set drawable size so entire image is drawn,
	//         otherwise, the scrolling will not work since the "hidden"
	//         part of the image is not ever drawn
	gtk_drawing_area_size(
				GTK_DRAWING_AREA(mDrawingArea),
				mImage->getNumCols(),
				mImage->getNumRows());
}


//*******************************************************************
//
//
void ModifyDatabaseWindow::zoomUpdate(bool setSize, int x, int y)
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
void ModifyDatabaseWindow::refreshImage()
{
	on_modifyDrawingArea_expose_event(NULL, NULL, (void *)this);
}


//*******************************************************************
//
//
GtkWidget *ModifyDatabaseWindow::createModifyDatabaseWindow(const string &title)
{
	GtkWidget *modifyWindow;
	GtkWidget *modifyHBoxMain;
	GtkWidget *modifyAlignment;
	GtkWidget *modifyVBoxLeft;
	GtkWidget *hseparator1;
	GtkWidget *modifyViewPort;
	GtkWidget *modifyEventBox;
	GtkWidget *modifyRightFrame;
	GtkWidget *modifyVBoxRight;
	GtkWidget *modifyLabelKnownInfo;
	GtkWidget *modifyLabelIDCode;
	GtkWidget *modifyLabelName;
	GtkWidget *modifyLabelDate;
	GtkWidget *modifyLabelRoll;
	GtkWidget *modifyLabelLocation;
	GtkWidget *modifyLabelDamage;
	GtkWidget *modifyLabelDescription;
	GtkWidget *modifyVButtonBox;
	guint modifyButtonSave_key;
	GtkWidget *modifyButtonSave;
	guint modifyButtonDeleteFin_key;
	GtkWidget *modifyButtonDeleteFin;
	guint modifyButtonCancel_key;
	GtkWidget *modifyButtonCancel;
	GtkAccelGroup *accel_group;
	GtkTooltips *tooltips;
	GtkWidget *tmpBox, *tmpLabel, *tmpIcon;

	tooltips = gtk_tooltips_new();

	accel_group = gtk_accel_group_new();

	modifyWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_object_set_data(GTK_OBJECT(modifyWindow), "modifyWindow", modifyWindow);
	gtk_window_set_policy(GTK_WINDOW(modifyWindow), TRUE, TRUE, FALSE);
	gtk_window_set_modal(GTK_WINDOW (modifyWindow), TRUE);	//***004CL
	gtk_window_set_title(GTK_WINDOW(modifyWindow), title.c_str());
	gtk_window_set_wmclass(GTK_WINDOW(modifyWindow), "darwin_modify", "DARWIN");

	modifyHBoxMain = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(modifyHBoxMain);
	gtk_container_add(GTK_CONTAINER(modifyWindow), modifyHBoxMain);

	modifyAlignment = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_widget_show(modifyAlignment);
	gtk_box_pack_start(GTK_BOX(modifyHBoxMain), 
			modifyAlignment, TRUE, TRUE, 0);

	modifyVBoxLeft = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(modifyVBoxLeft);
	gtk_container_add(GTK_CONTAINER(modifyAlignment), modifyVBoxLeft);

	hseparator1 = gtk_hseparator_new();
	gtk_widget_show(hseparator1);
	gtk_box_pack_start(GTK_BOX(modifyVBoxLeft), 
			hseparator1, FALSE, TRUE, 0);

	mScrolledWindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(mScrolledWindow);
	gtk_box_pack_start(GTK_BOX(modifyVBoxLeft), 
			mScrolledWindow, TRUE, TRUE, 0);
	//***1.8 - this allows the scroller bars to appear/disappear automatically
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mScrolledWindow), 
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	modifyViewPort = gtk_viewport_new(NULL, NULL);
	gtk_widget_show(modifyViewPort);
	gtk_container_add(GTK_CONTAINER(mScrolledWindow), modifyViewPort);

	modifyEventBox = gtk_event_box_new();
	gtk_widget_show(modifyEventBox);
	gtk_container_add(GTK_CONTAINER(modifyViewPort), modifyEventBox);

	mDrawingArea = gtk_drawing_area_new();
	gtk_widget_show(mDrawingArea);
	gtk_container_add(GTK_CONTAINER(modifyEventBox), mDrawingArea);


	modifyVBoxRight = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(modifyVBoxRight), 4);
	gtk_widget_show(modifyVBoxRight);

	modifyRightFrame = gtk_frame_new(NULL);
	gtk_widget_show(modifyRightFrame);
    gtk_container_add(GTK_CONTAINER(modifyRightFrame), modifyVBoxRight);

	gtk_box_pack_start(GTK_BOX(modifyHBoxMain), modifyRightFrame, FALSE, TRUE, 0);

	// confusing changes in font management here - JHS
	GtkStyle *infoStyle = gtk_style_new();
	//gdk_font_unref(infoStyle->font);
	//infoStyle->font = gdk_font_load("-*-helvetica-bold-r-*-*-*-160-*-*-*-*-*-*");

	//if (!infoStyle->font)
	//   infoStyle->font = gdk_font_load("fixed");

	// new replacement code (next 4 lines) - JHS
	//GdkFont *infoFont = gdk_font_load("-*-helvetica-bold-r-*-*-*-160-*-*-*-*-*-*");
	GdkFont *infoFont = gdk_font_load("-*-tahoma-*-r-*-*-*-160-*-*-*-*-*-*");
	if (!infoFont)
		infoFont = gdk_font_load("fixed");
	gtk_style_set_font(infoStyle, infoFont);

	modifyLabelKnownInfo = gtk_label_new(_("Known Information"));
	gtk_widget_set_style(modifyLabelKnownInfo, infoStyle);
	gtk_widget_show(modifyLabelKnownInfo);
	gtk_style_unref(infoStyle);

	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), 
			modifyLabelKnownInfo, FALSE, FALSE, 5);

	modifyLabelIDCode = gtk_label_new(_("ID Code"));
	gtk_widget_show(modifyLabelIDCode);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), 
			modifyLabelIDCode, FALSE, FALSE, 2);

	mEntryID = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(mEntryID), mFin->getID().c_str());
	gtk_widget_show(mEntryID);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), 
			mEntryID, FALSE, FALSE, 0);

	modifyLabelName = gtk_label_new(_("Name"));
	gtk_widget_show(modifyLabelName);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), 
			modifyLabelName, FALSE,	FALSE, 2);

	mEntryName = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(mEntryName), mFin->getName().c_str());
	gtk_widget_show(mEntryName);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), mEntryName, FALSE, FALSE, 0);

	modifyLabelDate = gtk_label_new(_("Date of Sighting"));
	gtk_widget_show(modifyLabelDate);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), modifyLabelDate, FALSE, FALSE, 2);

	mEntryDate = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(mEntryDate), mFin->getDate().c_str());
	gtk_widget_show(mEntryDate);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), mEntryDate, FALSE, FALSE, 0);

	modifyLabelRoll = gtk_label_new(_("Roll and Frame"));
	gtk_widget_show(modifyLabelRoll);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), modifyLabelRoll, FALSE, FALSE, 3);

	mEntryRoll = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(mEntryRoll), mFin->getRoll().c_str());
	gtk_widget_show(mEntryRoll);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), mEntryRoll, FALSE, FALSE, 0);

	modifyLabelLocation = gtk_label_new(_("Location Code"));
	gtk_widget_show(modifyLabelLocation);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), modifyLabelLocation, FALSE, FALSE, 2);

	mEntryLocation = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(mEntryLocation), mFin->getLocation().c_str());
	gtk_widget_show(mEntryLocation);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), mEntryLocation, FALSE, FALSE, 0);

	modifyLabelDamage = gtk_label_new(_("Damage Category"));
	gtk_widget_show(modifyLabelDamage);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), modifyLabelDamage, FALSE, FALSE, 2);
	//
	// a new approach based on a scroll-down menu
	//
	mEntryDamage = gtk_combo_box_new_text();
	gtk_widget_show(mEntryDamage);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), mEntryDamage, FALSE, FALSE, 0);

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

	modifyLabelDescription = gtk_label_new(_("Short Description"));
	gtk_widget_show(modifyLabelDescription);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), modifyLabelDescription,
			FALSE, FALSE, 2);

	mEntryDescription = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(mEntryDescription), mFin->getShortDescription().c_str());
	gtk_widget_show(mEntryDescription);
	gtk_box_pack_start(GTK_BOX(modifyVBoxRight), mEntryDescription,
			FALSE, FALSE, 0);

	modifyVButtonBox = gtk_vbutton_box_new();
	gtk_widget_show(modifyVButtonBox);
	gtk_box_pack_end(GTK_BOX(modifyVBoxRight), modifyVButtonBox, FALSE, TRUE,
			0);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(modifyVButtonBox),
				GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing(GTK_BUTTON_BOX(modifyVButtonBox), 3);

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, save_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	modifyButtonSave = gtk_button_new();
	modifyButtonSave_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
				_("_Save"));
	gtk_widget_add_accelerator(modifyButtonSave, "clicked", accel_group,
				modifyButtonSave_key, GDK_MOD1_MASK,
				(GtkAccelFlags) 0);
	gtk_container_add(GTK_CONTAINER(modifyButtonSave), tmpBox);
	gtk_widget_show(modifyButtonSave);
	gtk_container_add(GTK_CONTAINER(modifyVButtonBox), modifyButtonSave);
	GTK_WIDGET_SET_FLAGS(modifyButtonSave, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip(tooltips, modifyButtonSave,
				_("Save this fin to the database."), NULL);
	gtk_widget_add_accelerator(modifyButtonSave, "clicked", accel_group,
				GDK_S, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, trash_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	modifyButtonDeleteFin = gtk_button_new();
	modifyButtonDeleteFin_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
					_("Delete Fin"));
	gtk_widget_add_accelerator(modifyButtonDeleteFin, "clicked",
					accel_group, modifyButtonDeleteFin_key,
					GDK_MOD1_MASK, (GtkAccelFlags) 0);
	gtk_container_add(GTK_CONTAINER(modifyButtonDeleteFin), tmpBox);
	gtk_widget_show(modifyButtonDeleteFin);
	gtk_container_add(GTK_CONTAINER(modifyVButtonBox),
					modifyButtonDeleteFin);
	GTK_WIDGET_SET_FLAGS(modifyButtonDeleteFin, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip(tooltips, modifyButtonDeleteFin,
					_("Delete this fin from the database."),
					NULL);
	gtk_widget_add_accelerator(modifyButtonDeleteFin, "clicked",
					accel_group, GDK_D, GDK_MOD1_MASK,
					GTK_ACCEL_VISIBLE);

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);


	modifyButtonCancel = gtk_button_new();
	modifyButtonCancel_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
					_("_Cancel"));
	gtk_widget_add_accelerator(modifyButtonCancel, "clicked", accel_group,
					modifyButtonCancel_key, GDK_MOD1_MASK,
					(GtkAccelFlags) 0);
	gtk_container_add(GTK_CONTAINER(modifyButtonCancel), tmpBox);
	gtk_widget_show(modifyButtonCancel);
	gtk_container_add(GTK_CONTAINER(modifyVButtonBox), modifyButtonCancel);
	GTK_WIDGET_SET_FLAGS(modifyButtonCancel, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip(tooltips, modifyButtonCancel,
					_("Close this window and discard any work you've done."),
					NULL);
	gtk_widget_add_accelerator(modifyButtonCancel, "clicked", accel_group,
					GDK_C, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

	gtk_signal_connect(
				GTK_OBJECT(modifyWindow),
				"delete_event",
				GTK_SIGNAL_FUNC(on_modifyWindow_delete_event),
				(void*)this);

	gtk_signal_connect(
				GTK_OBJECT(mDrawingArea),
				"expose_event",
				GTK_SIGNAL_FUNC(on_modifyDrawingArea_expose_event),
				(void *) this);

	gtk_signal_connect(
				GTK_OBJECT(mDrawingArea),
				"configure_event",
				GTK_SIGNAL_FUNC(on_m_mDrawingArea_configure_event),
				(void *) this);

	gtk_signal_connect(
				GTK_OBJECT(mScrolledWindow),
				"configure_event",
				GTK_SIGNAL_FUNC(on_m_mScrolledWindow_configure_event),
				(void *) this);

	gtk_signal_connect(
				GTK_OBJECT(modifyButtonSave),
				"clicked",
				GTK_SIGNAL_FUNC(on_modifyButtonSave_clicked),
				(void *) this);

	gtk_signal_connect(
				GTK_OBJECT(modifyButtonDeleteFin),
				"clicked",
				GTK_SIGNAL_FUNC(on_modifyButtonDeleteFin_clicked),
				(void *) this);

	gtk_signal_connect(
				GTK_OBJECT(modifyButtonCancel),
				"clicked",
				GTK_SIGNAL_FUNC(on_modifyButtonCancel_clicked),
				(void *) this);

	gtk_widget_grab_default(modifyButtonSave);
	gtk_object_set_data(GTK_OBJECT(modifyWindow), "tooltips", tooltips);

	gtk_window_add_accel_group(GTK_WINDOW(modifyWindow), accel_group);
	gtk_widget_set_usize(mScrolledWindow, 400, 400);
	return modifyWindow;
}

//*******************************************************************
//
//
GtkWidget* ModifyDatabaseWindow::createQuestionDialog()
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
	gtk_window_set_title (GTK_WINDOW (questionDialog), _("Delete Fin?"));
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

	questionPixmap = create_pixmap_from_data(questionDialog, question_xpm);
	gtk_widget_show (questionPixmap);
	gtk_box_pack_start (GTK_BOX (questionHBox), questionPixmap, TRUE, TRUE, 0);

	questionLabel = gtk_label_new (_("Are you sure you want to delete\nthis fin from the database?"));
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

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, yes_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	questionButtonYes = gtk_button_new();
	questionButtonYes_key = gtk_label_parse_uline(GTK_LABEL(tmpLabel),
								_("_Yes"));
	gtk_widget_add_accelerator (questionButtonYes, "clicked", accel_group,
								questionButtonYes_key, GDK_MOD1_MASK, (GtkAccelFlags)0);

	gtk_container_add(GTK_CONTAINER(questionButtonYes), tmpBox);
	gtk_widget_show (questionButtonYes);
	gtk_container_add (GTK_CONTAINER (questionHButtonBox), questionButtonYes);
	GTK_WIDGET_SET_FLAGS (questionButtonYes, GTK_CAN_DEFAULT);
	gtk_widget_add_accelerator (questionButtonYes, "clicked", accel_group,
								GDK_C, GDK_MOD1_MASK,
								GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (questionButtonYes, "clicked", accel_group,
								GDK_Escape, (GdkModifierType)0,
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
						GTK_SIGNAL_FUNC (on_m_questionDialog_delete_event),
						(void *)this);
	gtk_signal_connect (GTK_OBJECT (questionButtonYes), "clicked",
						GTK_SIGNAL_FUNC (on_m_questionButtonYes_clicked),
						(void *)this);
	gtk_signal_connect (GTK_OBJECT (questionButtonCancel), "clicked",
						GTK_SIGNAL_FUNC (on_m_questionButtonCancel_clicked),
						(void *)this);

	gtk_window_add_accel_group (GTK_WINDOW (questionDialog), accel_group);

	return questionDialog;
}

//*******************************************************************
//
//
gboolean on_modifyWindow_delete_event(GtkWidget * widget,
				     GdkEvent * event, gpointer userData)
{
	ModifyDatabaseWindow *modifyWin = (ModifyDatabaseWindow *) userData;

	if (NULL == modifyWin)
		return FALSE;

	delete modifyWin;

	return TRUE;
}


//*******************************************************************
//
//
gboolean on_modifyDrawingArea_expose_event(GtkWidget * widget,
					  GdkEventExpose * event,
					  gpointer userData)
{
	ModifyDatabaseWindow *modifyWin = (ModifyDatabaseWindow *) userData;

	if (NULL == modifyWin)
		return FALSE;

	if (NULL == modifyWin->mDrawingArea || NULL == modifyWin->mImage)
		return FALSE;

	modifyWin->mZoomXOffset =
		(modifyWin->mDrawingArea->allocation.width -
		 (int) modifyWin->mImage->getNumCols()) / 2;

	modifyWin->mZoomYOffset =
		(modifyWin->mDrawingArea->allocation.height -
		 (int) modifyWin->mImage->getNumRows()) / 2;

	// Just in case some funky allocation comes through
	if (modifyWin->mZoomXOffset < 0)
		modifyWin->mZoomXOffset = 0;
	if (modifyWin->mZoomYOffset < 0)
		modifyWin->mZoomYOffset = 0;

	gdk_draw_rgb_image(
		modifyWin->mDrawingArea->window,
		modifyWin->mDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
		modifyWin->mZoomXOffset, modifyWin->mZoomYOffset,
		modifyWin->mImage->getNumCols(),
		modifyWin->mImage->getNumRows(),
		GDK_RGB_DITHER_NONE,
		(guchar*)modifyWin->mImage->getData(),
		modifyWin->mImage->bytesPerPixel() * modifyWin->mImage->getNumCols());

	return TRUE;
}



//*******************************************************************
//
//
void on_modifyButtonSave_clicked(GtkButton * button, gpointer userData)
{
	//if (getNumSaveFileSelectionDialogReferences() >= 1)
	//	return;

	ModifyDatabaseWindow *modifyWin = (ModifyDatabaseWindow *) userData;

	if (NULL == modifyWin)
	  return;

	// ***055 - must select a damage category before saving
	int damageIDnum = gtk_combo_box_get_active(GTK_COMBO_BOX(modifyWin->mEntryDamage));
	if (-1 == damageIDnum)
	{
		showError("You must select a Catalog Category\nBEFORE saving changes to this fin!");
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

		temp = gtk_editable_get_chars(
				GTK_EDITABLE(modifyWin->mEntryID),
				0, -1);
		id = temp;
		g_free(temp);

		temp = gtk_editable_get_chars(
					GTK_EDITABLE(modifyWin->mEntryName),
					0, -1);
		name = temp;
		g_free(temp);

		temp = gtk_editable_get_chars(
					GTK_EDITABLE(modifyWin->mEntryDate),
					0, -1);
		date = temp;
		g_free(temp);

		temp = gtk_editable_get_chars(
					      GTK_EDITABLE(modifyWin->mEntryRoll),
					      0, -1);
		roll = temp;
		g_free(temp);

		temp = gtk_editable_get_chars(
				GTK_EDITABLE(modifyWin->mEntryLocation),
				0, -1);
		location = temp;
		g_free(temp);

/*		temp = gtk_editable_get_chars(
					      GTK_EDITABLE(modifyWin->mEntryDamage),
					      0, -1);
		damage = temp;
		g_free(temp);
*/
		// cannot use gtk_combo_box_get_active_text() until we upgrade to GTK 2.6+
		//int damageIDnum = gtk_combo_box_get_active(GTK_COMBO_BOX(modifyWin->mEntryDamage)); //***051
		if (damageIDnum != -1)
			damage = modifyWin->mDatabase->catCategoryName(damageIDnum); //***051

		temp = gtk_editable_get_chars(
					      GTK_EDITABLE(modifyWin->mEntryDescription),
				0, -1);
		description = temp;
		g_free(temp);

		if ("" == id)
		  {
		    showError (ERROR_MSG_NO_IDCODE);
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


#ifdef DEBUG
		cout << "Fname : " << modifyWin->mImagefilename << endl;
#endif

		//***054 - copy unknown image to catalog and use short filename in database
		/*
		don't think this is needed here, just make sure the size of record
		calculation in the databasefin::delete() function is using the SHORT
		filename that is actually in the file

		string shortFilename = modifyWin->mImagefilename;
		int pos = shortFilename.find_last_of(PATH_SLASH);
		if (pos >= 0)
		{
			shortFilename = shortFilename.substr(pos+1);
		}

		printf("saving \"%s\"\n",shortFilename.c_str());

		string copyFilename = getenv("DARWINHOME");
		copyFilename += PATH_SLASH;
		copyFilename += "catalog";
		copyFilename += PATH_SLASH;
		copyFilename += shortFilename;

		// copy image over into catalog folder

		string command = "copy ";
		command += modifyWin->mImagefilename;
		command += " ";
		command += copyFilename;

		printf("copy command: \"%s\"",command.c_str());

		system(command.c_str());
		*/
		DatabaseFin<ColorImage> *newFin  = new DatabaseFin<ColorImage>(
				modifyWin->mImagefilename,
				//copyFilename, //***054
				modifyWin->mFin->mFinOutline, //***008OL
				id,
				name,
				date,
				roll,
				location,
				damage,
				description);
		modifyWin->mDatabase->Delete(modifyWin->mFin);
		unsigned long addOffset = modifyWin->mDatabase->add(newFin);
		delete newFin;

		// OLD approach
		//modifyWin->mMainWin->refreshDatabaseDisplay();                 //***004CL
        //modifyWin->mMainWin->selectFromCList(modifyWin->mDBCurEntry);  //***004CL

		//***1.9 - new approach to reselect
		gtk_clist_freeze(GTK_CLIST(modifyWin->mMainWin->mCList));

		modifyWin->mMainWin->refreshDatabaseDisplayNew(true);
		if(modifyWin->mDatabase->size() > 0)
		{
			gtk_clist_select_row(
						GTK_CLIST(modifyWin->mMainWin->mCList), 
						(modifyWin->mDBCurEntry), 0);
		}
		
		gtk_clist_thaw(GTK_CLIST(modifyWin->mMainWin->mCList));
		//***1.9 - end new approach

		delete modifyWin;
	} catch (Error e) {
	  showError(e.errorString());
	  delete modifyWin;
	}

}

//*******************************************************************
//
//
void on_modifyButtonDeleteFin_clicked(GtkButton * button,
					 gpointer userData)
{
	ModifyDatabaseWindow *modifyWin = (ModifyDatabaseWindow *) userData;

	if (NULL == modifyWin)
		return;
	if (NULL == modifyWin->mDatabase)
	        return;
	modifyWin->mQuestionDialog= modifyWin->createQuestionDialog();
	gtk_widget_show(modifyWin->mQuestionDialog);

}

//*******************************************************************
//
//
void on_modifyButtonCancel_clicked(GtkButton * button, gpointer userData)
{
	ModifyDatabaseWindow *modifyWin = (ModifyDatabaseWindow *) userData;

	if (NULL == modifyWin)
		return;

	delete modifyWin;
}

//*******************************************************************
//
//
gboolean on_m_questionDialog_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	ModifyDatabaseWindow *modifyWin = (ModifyDatabaseWindow *)userData;

	if (NULL == modifyWin)
		return FALSE;

	gtk_widget_destroy(modifyWin->mQuestionDialog);
	modifyWin->mQuestionDialog = NULL;

	return TRUE;
}

//*******************************************************************
//
//
void on_m_questionButtonYes_clicked(
	GtkButton *button,
	gpointer userData)
{
	ModifyDatabaseWindow *modifyWin = (ModifyDatabaseWindow *)userData;

	if (NULL == modifyWin)
		return;
	gtk_widget_destroy(modifyWin->mQuestionDialog);
	modifyWin->mQuestionDialog = NULL;
	modifyWin->mDatabase->Delete(modifyWin->mFin);
		
	gtk_clist_freeze(GTK_CLIST(modifyWin->mMainWin->mCList)); //***1.85

	//modifyWin->mMainWin->refreshDatabaseDisplay(); // old version
	modifyWin->mMainWin->refreshDatabaseDisplayNew(true); //***1.85
	if(modifyWin->mDatabase->size() > 0)                  //***004CL next6, ***1.85
	{
		if (modifyWin->mDBCurEntry > 0)
			//modifyWin->mMainWin->selectFromCList((modifyWin->mDBCurEntry)-1);
			gtk_clist_select_row(
					GTK_CLIST(modifyWin->mMainWin->mCList), 
					(modifyWin->mDBCurEntry)-1, 0);  //***1.85
		else
			//modifyWin->mMainWin->selectFromCList(0);
			gtk_clist_select_row(
					GTK_CLIST(modifyWin->mMainWin->mCList), 
					0, 0);  //***1.85
	}                                                       //***004CL
		
	gtk_clist_thaw(GTK_CLIST(modifyWin->mMainWin->mCList)); //***1.85

	delete modifyWin;
}

//*******************************************************************
//
//
void on_m_questionButtonCancel_clicked(
	GtkButton *button,
	gpointer userData)
{
	ModifyDatabaseWindow *modifyWin = (ModifyDatabaseWindow *)userData;

	if (NULL == modifyWin)
		return;

	gtk_widget_destroy(modifyWin->mQuestionDialog);
	modifyWin->mQuestionDialog = NULL;
}

//*******************************************************************
//
//
gboolean on_m_mDrawingArea_configure_event(
		GtkWidget *widget,
		GdkEventConfigure *event,
		gpointer userData)
{
	ModifyDatabaseWindow *modifyWin = (ModifyDatabaseWindow *)userData;

	if (NULL == modifyWin)
		return FALSE;

	// We want to ignore configure events right after a zoom.
	// zoomUpdate will set the ignore flag when it runs.  (We only
	// want to handle configure events that are generated by the
	// user resizing the window.)
	if (modifyWin->mIgnoreConfigureEventCnt) {
		modifyWin->mIgnoreConfigureEventCnt--;
		return TRUE;
	}

	// Try to figure out what the zoom ratio should be to fit
	// the image inside the drawing area
	float heightRatio, widthRatio;

	heightRatio = (float)modifyWin->mDrawingArea->allocation.height /
		modifyWin->mNonZoomedImage->getNumRows();
	widthRatio = (float)modifyWin->mDrawingArea->allocation.width /
		modifyWin->mNonZoomedImage->getNumCols(); //***1.8 - was Rows (in error)

	if (heightRatio < 1.0f && widthRatio < 1.0f) {
		if (widthRatio < heightRatio) {
			// the width is the restrictive dimension
			if (widthRatio >= 0.5f)
				modifyWin->mZoomRatio = 50;
			else
				modifyWin->mZoomRatio = 25;
		} else if (heightRatio >= 0.5f)
			modifyWin->mZoomRatio = 50;
		else
			modifyWin->mZoomRatio = 25;
	} else
		modifyWin->mZoomRatio = 100;

	//printf("Zoom %d\n",modifyWin->mZoomRatio); // debug

	modifyWin->zoomUpdate(false);

	return TRUE;
}

//*******************************************************************
//
//
gboolean on_m_mScrolledWindow_configure_event(
	GtkWidget *widget,
	GdkEventConfigure *event,
	gpointer userData)
{
	ModifyDatabaseWindow *modifyWin = (ModifyDatabaseWindow *) userData;

	if (widget->allocation.width > modifyWin->mSWWidthMax)
		modifyWin->mSWWidthMax = widget->allocation.width;

	if (widget->allocation.height > modifyWin->mSWHeightMax)
		modifyWin->mSWHeightMax = widget->allocation.height;

	return TRUE;
}
