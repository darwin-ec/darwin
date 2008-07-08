//*******************************************************************
//   file: ImageViewDialog.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (8/2/2005)
//         -- reformatting of code and addition of comment blocks
//         -- set initial size of image veiw dialog (800 x 600)
//
//*******************************************************************

#include <gdk/gdkkeysyms.h>
#include "../support.h"
#include "ImageViewDialog.h"
#include "../image_processing/transform.h"

#pragma warning (disable : 4305 4309)

#include "../../pixmaps/magnify_cursor.xbm"
#include "../../pixmaps/close.xpm"
#include "../../pixmaps/zoom_in.xpm"
#include "../../pixmaps/zoom_out.xpm"
#include "../../pixmaps/Hflip.xpm"    //***1.75

using namespace std;

static const int MAX_ZOOM = 1600;
static const int MIN_ZOOM = 6; //***1.95 - 6% of original size is lower limit

static int gNumReferences = 0;

//*******************************************************************
int getNumImageViewDialogReferences()
{
	return gNumReferences;
}

//*******************************************************************
ImageViewDialog::ImageViewDialog(const string &name, ColorImage *image)
	: //mDialog(createViewDialog(name)),
	  mNonZoomedImage(new ColorImage(image)),
	  mImage(new ColorImage(image)),
	  mFlippedImage(NULL),    //***1.75
	  mFlippedImageOK(false), //***1.75
	  mCursor(NULL),
	  mZoomRatio(100),
	  mIsFlipped(false) //***1.75
{
	mDialog = createViewDialog(name); //***1.8 - so size based on image size
	gNumReferences++;
}

//*******************************************************************
ImageViewDialog::~ImageViewDialog()
{
	gtk_widget_destroy(mDialog);
	delete mImage;
	delete mNonZoomedImage;
	if (NULL != mFlippedImage)
		delete mFlippedImage; //***1.75
	gNumReferences--;
}

//*******************************************************************
void ImageViewDialog::show()
{
	gtk_widget_show(mDialog);
	updateCursor();

	gtk_widget_set_usize(
			GTK_WIDGET(mScrolledWindow),
			mImage->getNumCols() + 10,
			mImage->getNumRows() + 10);

	gtk_drawing_area_size(
		GTK_DRAWING_AREA(mDrawingArea),
		mImage->getNumCols(),
		mImage->getNumRows()
		);
}

//*******************************************************************
void ImageViewDialog::updateCursor()
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

	// I'm paranoid, what can I say?
	if (NULL != mCursor && NULL != mDrawingArea)
		gdk_window_set_cursor(mDrawingArea->window, mCursor);
}

//*******************************************************************
void ImageViewDialog::zoomIn()
{
	if (mZoomRatio <= MAX_ZOOM) {
		mZoomRatio *= 2;
		//***1.95 force back from 24% to 25% going up in size
		if (mZoomRatio == 24)
			mZoomRatio ++; 
		
		zoomUpdate(true);
	}
}

//*******************************************************************
void ImageViewDialog::zoomOut()
{
	if (mZoomRatio > MIN_ZOOM) { //***1.95 - changed from low limit of 25%
		mZoomRatio /= 2;
		
		zoomUpdate(true);	
	}
}

//*******************************************************************
void ImageViewDialog::zoomUpdate(bool zoomChanged)
{
	if (mZoomRatio == MIN_ZOOM) //***1.95
		gtk_widget_set_sensitive(mButtonZoomOut, FALSE);
	else {
		gtk_widget_set_sensitive(mButtonZoomOut, TRUE);

		if (mZoomRatio == MAX_ZOOM)
			gtk_widget_set_sensitive(mButtonZoomIn, FALSE);
		else
			gtk_widget_set_sensitive(mButtonZoomIn, TRUE);
	}

	gchar zoomTxt[100];
	sprintf(zoomTxt, "Zoom Level: %d %%", mZoomRatio);
	gtk_label_set_text(GTK_LABEL(mLabelZoom), zoomTxt);

	if (NULL != mImage) {
		gdk_draw_rectangle(
				mDrawingArea->window,
				mDrawingArea->style->bg_gc[GTK_STATE_NORMAL],
				TRUE,
				0,
				0,
				mImage->getNumCols(),
				mImage->getNumRows());
		if (zoomChanged)
			delete mImage; //***1.75 - mImage just points to current scaled/flipped image
	}

	if (zoomChanged)
	{
		mImage = resizeNN(mNonZoomedImage, (float)mZoomRatio);
		if (NULL != mFlippedImage) //***1.75 - handle flipped image change of scale
		{
			delete mFlippedImage;
			mFlippedImage = flipHorizontally(mImage);
			mFlippedImageOK = true;
			if (mIsFlipped)
			{
				ColorImage *temp = mImage;
				mImage = mFlippedImage;
				mFlippedImage = temp;
			}
		}
	}

	if (mZoomRatio < 100)
		gtk_widget_set_usize(
				GTK_WIDGET(mScrolledWindow),
				mImage->getNumCols() + 15,
				mImage->getNumRows() + 15);

	if (NULL != mDrawingArea && NULL != mImage)
		gtk_drawing_area_size(
				GTK_DRAWING_AREA(mDrawingArea),
				mImage->getNumCols(),
				mImage->getNumRows()
				);
	
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

	this->refreshImage();
}


//*******************************************************************
void ImageViewDialog::refreshImage()
{
	on_viewDrawingArea_expose_event(mDrawingArea, NULL, (void *) this);
}

//*******************************************************************
GtkWidget* ImageViewDialog::createViewDialog(const string &title)
{
  GtkWidget *viewDialog;
  GtkWidget *viewVBox;
  GtkWidget *viewHBox;
  GtkWidget *viewHandleBox;
  GtkWidget *viewToolBar;
  GtkWidget *tmp_toolbar_icon;
  GtkWidget *viewViewPort;
  GtkWidget *viewEventBox;
  GtkWidget *dialog_action_area1;
  GtkWidget *viewHButtonBox;
  guint viewButtonClose_key;
  GtkWidget *viewButtonClose;
  GtkAccelGroup *accel_group;
  GtkWidget *tmpIcon, *tmpLabel, *tmpBox;

  accel_group = gtk_accel_group_new ();

  viewDialog = gtk_dialog_new();
  gtk_object_set_data (GTK_OBJECT (viewDialog), "viewDialog", viewDialog);
  gtk_window_set_title(GTK_WINDOW (viewDialog), title.c_str());
  gtk_window_set_policy (GTK_WINDOW (viewDialog), TRUE, TRUE, TRUE);
  gtk_window_set_wmclass(GTK_WINDOW(viewDialog), "darwin_imageview", "DARWIN");
  //***1.8 - set size based on smaller of image size or 800x600
  int 
	height = (600 < mNonZoomedImage->mRows) ? 600 : mNonZoomedImage->mRows, 
	width = (800 < mNonZoomedImage->mCols) ? 800 : mNonZoomedImage->mCols;
  height += 85; // add space for buttons & borders 
  width += 8;   // add space for borders
  gtk_window_set_default_size(GTK_WINDOW(viewDialog), (gint)width, (gint)height);
	gtk_window_set_position(GTK_WINDOW(viewDialog), GTK_WIN_POS_CENTER); //***1.8

  viewVBox = GTK_DIALOG (viewDialog)->vbox;
  gtk_object_set_data (GTK_OBJECT (viewDialog), "viewVBox", viewVBox);
  gtk_widget_show (viewVBox);

  viewHBox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (viewHBox);
  gtk_box_pack_start (GTK_BOX (viewVBox), viewHBox, FALSE, TRUE, 0);

  viewHandleBox = gtk_handle_box_new ();
  gtk_widget_show (viewHandleBox);
  gtk_box_pack_start (GTK_BOX (viewHBox), viewHandleBox, FALSE, TRUE, 0);
  gtk_handle_box_set_shadow_type (GTK_HANDLE_BOX (viewHandleBox), GTK_SHADOW_NONE);

  viewToolBar = gtk_toolbar_new ();
  gtk_toolbar_set_orientation (GTK_TOOLBAR(viewToolBar),GTK_ORIENTATION_HORIZONTAL);
  gtk_toolbar_set_style (GTK_TOOLBAR(viewToolBar), GTK_TOOLBAR_ICONS);
  gtk_widget_show (viewToolBar);
  gtk_container_add (GTK_CONTAINER (viewHandleBox), viewToolBar);

  tmp_toolbar_icon = create_pixmap_from_data(viewDialog, zoom_in_xpm);
  mButtonZoomIn = gtk_toolbar_append_element (GTK_TOOLBAR (viewToolBar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                _("Zoom In"),
                                _("Zoom In"), NULL,
                                tmp_toolbar_icon, NULL, NULL);
  gtk_button_set_relief(GTK_BUTTON(mButtonZoomIn), GTK_RELIEF_NONE);
  gtk_widget_show (mButtonZoomIn);

  tmp_toolbar_icon = create_pixmap_from_data(viewDialog, zoom_out_xpm);
  mButtonZoomOut = gtk_toolbar_append_element (GTK_TOOLBAR (viewToolBar),
                                GTK_TOOLBAR_CHILD_BUTTON,
                                NULL,
                                _("Zoom Out"),
                                _("Zoom Out"), NULL,
                                tmp_toolbar_icon, NULL, NULL);
  gtk_button_set_relief(GTK_BUTTON(mButtonZoomOut), GTK_RELIEF_NONE);
  gtk_widget_show(mButtonZoomOut);

	//***1.75 - new FLIP HORIZONTALLY button

	tmp_toolbar_icon = create_pixmap_from_data(viewDialog, flip_h_xpm);
	mButtonFlipHorizontally =
		gtk_toolbar_append_element(
			GTK_TOOLBAR(viewToolBar),
			GTK_TOOLBAR_CHILD_BUTTON, NULL,
			_("Flip H"),
			_("Flip the image horizontally."), NULL,
			tmp_toolbar_icon, NULL, NULL);
	gtk_button_set_relief (GTK_BUTTON (mButtonFlipHorizontally), GTK_RELIEF_NONE);
	gtk_widget_show(mButtonFlipHorizontally);

  mLabelZoom = gtk_label_new (_("Zoom Level: 100 %"));
  gtk_widget_show (mLabelZoom);
  gtk_box_pack_end (GTK_BOX (viewHBox), mLabelZoom, FALSE, FALSE, 0);

  mScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (mScrolledWindow);
  gtk_box_pack_start (GTK_BOX (viewVBox), mScrolledWindow, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  viewViewPort = gtk_viewport_new (NULL, NULL);
  gtk_widget_show (viewViewPort);
  gtk_container_add (GTK_CONTAINER (mScrolledWindow), viewViewPort);

  viewEventBox = gtk_event_box_new ();
  gtk_widget_show (viewEventBox);
  gtk_container_add (GTK_CONTAINER (viewViewPort), viewEventBox);

  mDrawingArea = gtk_drawing_area_new ();
  gtk_widget_show(mDrawingArea);
  gtk_container_add (GTK_CONTAINER (viewEventBox), mDrawingArea);

  dialog_action_area1 = GTK_DIALOG (viewDialog)->action_area;
  gtk_object_set_data (GTK_OBJECT (viewDialog), "dialog_action_area1", dialog_action_area1);
  gtk_widget_show (dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 2);

  viewHButtonBox = gtk_hbutton_box_new ();
  gtk_widget_show (viewHButtonBox);
  gtk_box_pack_start (GTK_BOX (dialog_action_area1), viewHButtonBox, TRUE, TRUE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (viewHButtonBox), GTK_BUTTONBOX_END);

  tmpBox = gtk_hbox_new(FALSE, 0);
  tmpIcon = create_pixmap_from_data(tmpBox, close_xpm);
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
  gtk_widget_show(tmpIcon);
  tmpLabel = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
  gtk_widget_show(tmpLabel);
  gtk_widget_show(tmpBox);
  
  viewButtonClose = gtk_button_new();
  viewButtonClose_key = gtk_label_parse_uline(GTK_LABEL(tmpLabel),
                                   _("_Close"));
  gtk_widget_add_accelerator (viewButtonClose, "clicked", accel_group,
                              viewButtonClose_key, GDK_MOD1_MASK, (GtkAccelFlags)0);
  gtk_container_add(GTK_CONTAINER(viewButtonClose), tmpBox);
  gtk_widget_show (viewButtonClose);
  gtk_container_add (GTK_CONTAINER (viewHButtonBox), viewButtonClose);
  GTK_WIDGET_SET_FLAGS (viewButtonClose, GTK_CAN_DEFAULT);
  gtk_widget_add_accelerator (viewButtonClose, "clicked", accel_group,
                              GDK_C, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);
  gtk_widget_add_accelerator (viewButtonClose, "clicked", accel_group,
                              GDK_Escape, (GdkModifierType)0,
                              GTK_ACCEL_VISIBLE);

  gtk_signal_connect (GTK_OBJECT (viewDialog), "delete_event",
                      GTK_SIGNAL_FUNC (on_viewDialog_delete_event),
                      (void *) this);
  gtk_signal_connect (GTK_OBJECT (mButtonZoomIn), "clicked",
                      GTK_SIGNAL_FUNC (on_viewButtonZoomIn_clicked),
                      (void *) this);
  gtk_signal_connect (GTK_OBJECT (mButtonZoomOut), "clicked",
                      GTK_SIGNAL_FUNC (on_viewButtonZoomOut_clicked),
                      (void *) this);

	//***1.75 - new callback
	gtk_signal_connect(GTK_OBJECT(mButtonFlipHorizontally), "clicked",
		       GTK_SIGNAL_FUNC
		       (on_viewButtonFlipHorizontally_clicked), (void *) this);

  gtk_signal_connect (GTK_OBJECT (viewEventBox), "button_press_event",
                      GTK_SIGNAL_FUNC (on_viewEventBox_button_press_event),
                      (void *) this);
  gtk_signal_connect (GTK_OBJECT (mDrawingArea), "expose_event",
                      GTK_SIGNAL_FUNC (on_viewDrawingArea_expose_event),
                      (void *) this);
  gtk_signal_connect (GTK_OBJECT (viewButtonClose), "clicked",
                      GTK_SIGNAL_FUNC (on_viewButtonClose_clicked),
                      (void *) this);

  gtk_widget_grab_default(viewButtonClose);
  gtk_window_add_accel_group (GTK_WINDOW (viewDialog), accel_group);

  return viewDialog;
}

//*******************************************************************
gboolean on_viewDialog_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	ImageViewDialog *dlg = (ImageViewDialog *) userData;

	if (NULL == dlg)
		return FALSE;

	delete dlg;

	return TRUE;
}

//*******************************************************************
void on_viewButtonZoomIn_clicked(
	GtkButton *button,
	gpointer userData)
{
	ImageViewDialog *dlg = (ImageViewDialog *) userData;

	if (NULL == dlg)
		return;
	
	// ***1.75 - set location of center based on slider
	if (dlg->mScrolledWindow->allocation.height < dlg->mImage->getNumRows())
	{
		GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(
			GTK_SCROLLED_WINDOW(dlg->mScrolledWindow));
		double half = 0.5 * dlg->mScrolledWindow->allocation.height;
		dlg->mImageCenterY = (adj->value + half) / (adj->upper - adj->lower);
	}
	if (dlg->mScrolledWindow->allocation.width < dlg->mImage->getNumCols())
	{
		GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment(
			GTK_SCROLLED_WINDOW(dlg->mScrolledWindow));
		double half = 0.5 * dlg->mScrolledWindow->allocation.width;
		dlg->mImageCenterX = (adj->value + half) / (adj->upper - adj->lower);
	}

	dlg->zoomIn();
}

//*******************************************************************
void on_viewButtonZoomOut_clicked(
	GtkButton *button,
	gpointer userData)
{
	ImageViewDialog *dlg = (ImageViewDialog *) userData;

	if (NULL == dlg)
		return;

	// ***1.75 - set location of center based on slider
	if (dlg->mScrolledWindow->allocation.height < dlg->mImage->getNumRows())
	{
		GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(
			GTK_SCROLLED_WINDOW(dlg->mScrolledWindow));
		double half = 0.5 * dlg->mScrolledWindow->allocation.height;
		dlg->mImageCenterY = (adj->value + half) / (adj->upper - adj->lower);
	}
	if (dlg->mScrolledWindow->allocation.width < dlg->mImage->getNumCols())
	{
		GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment(
			GTK_SCROLLED_WINDOW(dlg->mScrolledWindow));
		double half = 0.5 * dlg->mScrolledWindow->allocation.width;
		dlg->mImageCenterX = (adj->value + half) / (adj->upper - adj->lower);
	}

	dlg->zoomOut();
}

//*******************************************************************
//
//***1.75 - copied from TraceWindow - new functionality
//
void on_viewButtonFlipHorizontally_clicked(
		GtkButton * button,
		gpointer userData)
{
	ImageViewDialog *dlg = (ImageViewDialog *) userData;

	if (NULL == dlg)
		return;

	// ***1.75 - set location of center based on slider
	if (dlg->mScrolledWindow->allocation.height < dlg->mImage->getNumRows())
	{
		GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(
			GTK_SCROLLED_WINDOW(dlg->mScrolledWindow));
		double half = 0.5 * dlg->mScrolledWindow->allocation.height;
		dlg->mImageCenterY = (adj->value + half) / (adj->upper - adj->lower);
	}
	if (dlg->mScrolledWindow->allocation.width < dlg->mImage->getNumCols())
	{
		GtkAdjustment *adj = gtk_scrolled_window_get_hadjustment(
			GTK_SCROLLED_WINDOW(dlg->mScrolledWindow));
		double half = 0.5 * dlg->mScrolledWindow->allocation.width;
		dlg->mImageCenterX = (adj->value + half) / (adj->upper - adj->lower);
	}
	//***1.75 - flip desired / current center
	dlg->mImageCenterX = 1.0 - dlg->mImageCenterX;

	// if the flipped image is not at the right scale or does not exist
	// then  create it from the already rescaled dlg->mImage
	if (! dlg->mFlippedImageOK)
	{
		if (NULL != dlg->mFlippedImage)
			delete dlg->mFlippedImage;
		dlg->mFlippedImage = flipHorizontally(dlg->mImage);
		dlg->mFlippedImageOK = true;
	}
	
	//***1.75 - now just swap pointers to point to correct already ZOOMED image
	ColorImage *temp = dlg->mImage;
	dlg->mImage = dlg->mFlippedImage;
	dlg->mFlippedImage = temp;

	dlg->mIsFlipped = ! dlg->mIsFlipped; //***1.75

	dlg->zoomUpdate(false); // and redraw but do NOT resize mImage
}

//*******************************************************************
gboolean on_viewEventBox_button_press_event(
	GtkWidget *widget,
	GdkEventButton *event,
	gpointer userData)
{
	ImageViewDialog *dlg = (ImageViewDialog *) userData;

	if (NULL == dlg)
		return FALSE;

	if (event->button == 1) {
		
		//***1.75 - find position of cursor as percent of image size
		//g_print("x=%f y=%f\n",event->x, event->y);
		dlg->mImageCenterX = event->x / dlg->mImage->getNumCols();
		dlg->mImageCenterY = event->y / dlg->mImage->getNumRows();
		//g_print("x=%f y=%f\n",dlg->mImageCenterX, dlg->mImageCenterY);

		if (event->state & GDK_SHIFT_MASK)
		{
			dlg->zoomOut();
		}
		else
		{
			dlg->zoomIn();
		}

		return TRUE;
	}
	return FALSE;
}

//*******************************************************************
gboolean on_viewDrawingArea_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData)
{
	ImageViewDialog *dlg = (ImageViewDialog *) userData;

	if (NULL == dlg)
		return FALSE;

	if (NULL == dlg->mDrawingArea)
		return FALSE;

	gdk_draw_rgb_image(
		dlg->mDrawingArea->window,
		dlg->mDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
		0, 0,
		dlg->mImage->getNumCols(),
		dlg->mImage->getNumRows(),
		GDK_RGB_DITHER_NONE,
		(guchar*)dlg->mImage->getData(),
		dlg->mImage->bytesPerPixel() * dlg->mImage->getNumCols()
		);

	return TRUE;
}

//*******************************************************************
void on_viewButtonClose_clicked(
	GtkButton *button,
	gpointer userData)
{
	ImageViewDialog *dlg = (ImageViewDialog *) userData;

	delete dlg;
}
