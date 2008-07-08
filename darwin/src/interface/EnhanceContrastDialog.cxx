//*******************************************************************
//   file: EnhanceContrastDialog.cxx
//
// author: 
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include "../support.h"
#include "EnhanceContrastDialog.h"
#include "../Error.h"
#include "../image_processing/transform.h"
#include "../image_processing/ImageMod.h"

#include "../../pixmaps/ok.xpm"
#include "../../pixmaps/cancel.xpm"

using namespace std;

static int gNumReferences = 0;

//*******************************************************************
//
//
int getNumEnhanceContrastDialogReferences()
{
	return gNumReferences;
}

//*******************************************************************
//
//
EnhanceContrastDialog::EnhanceContrastDialog(
		TraceWindow *t,
		ColorImage **i
)
	: mDialog(createEnhanceContrastDialog()),
	  mTraceWin(t),
	  mOriginalImage(new ColorImage(*i)),
	  mImage(i)
{
	if (NULL == t)
		throw EmptyArgumentError("EnhanceContrastDialog ctor");

	gNumReferences++;
}

//*******************************************************************
//
//
EnhanceContrastDialog::~EnhanceContrastDialog()
{
	delete mOriginalImage;
	
	gtk_widget_destroy(mDialog);
	
	gNumReferences--;
}

//*******************************************************************
//
//
void EnhanceContrastDialog::show()
{
	gtk_widget_show(mDialog);
}

//*******************************************************************
//
//
void EnhanceContrastDialog::updateContrast(unsigned minLevel, unsigned maxLevel)
{
	delete *mImage;

	*mImage = enhanceContrast(mOriginalImage, minLevel, maxLevel);

	mTraceWin->zoomUpdate(false);
}

//*******************************************************************
//
//
GtkWidget* EnhanceContrastDialog::createEnhanceContrastDialog()
{
    GtkWidget *dialog_enhance_contrast;
    GtkWidget *dialog_vbox1;
    GtkWidget *vbox1;
    GtkWidget *label1;
    GtkWidget *hscale1;
    GtkWidget *dialog_action_area1;
    GtkWidget *hbuttonbox1;
    GtkWidget *button1;
    GtkWidget *button2;
    GtkWidget *button3;
    GtkWidget *sliderhbox;
    GtkWidget *tmpLabel, *tmpBox, *tmpIcon;

    dialog_enhance_contrast = gtk_dialog_new();
    gtk_object_set_data(GTK_OBJECT(dialog_enhance_contrast),
			"dialog_enhance_contrast",
			dialog_enhance_contrast);
    gtk_widget_set_usize(dialog_enhance_contrast, 400, 150);
    gtk_window_set_title(GTK_WINDOW(dialog_enhance_contrast),
			 _("Enhance Contrast"));
    GTK_WINDOW(dialog_enhance_contrast)->type = WINDOW_DIALOG;
    gtk_window_set_policy(GTK_WINDOW(dialog_enhance_contrast), TRUE, TRUE,
			  FALSE);
    gtk_window_set_wmclass(GTK_WINDOW(dialog_enhance_contrast), "darwin_alter", "DARWIN");

    dialog_vbox1 = GTK_DIALOG(dialog_enhance_contrast)->vbox;
    gtk_object_set_data(GTK_OBJECT(dialog_enhance_contrast),
			"dialog_vbox1", dialog_vbox1);
    gtk_widget_show(dialog_vbox1);

    vbox1 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox1);
    gtk_box_pack_start(GTK_BOX(dialog_vbox1), vbox1, TRUE, TRUE, 0);

    label1 =
	gtk_label_new(_
		      ("Move the two slider bars to change the contrast.\n"));
    gtk_widget_show(label1);
    gtk_box_pack_start(GTK_BOX(vbox1), label1, FALSE, FALSE, 10);
    gtk_widget_set_usize(label1, 265, 30);

    sliderhbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), sliderhbox, TRUE, FALSE, 0);
    gtk_widget_show(sliderhbox);

    label1 = gtk_label_new("Minimum");
    gtk_widget_show(label1);
    gtk_box_pack_start(GTK_BOX(sliderhbox), label1, FALSE, FALSE, 10);

    hscale1 =
	gtk_hscale_new(GTK_ADJUSTMENT
		       (gtk_adjustment_new(0, 0, 255, 5, 5, 0)));
    gtk_scale_set_digits(GTK_SCALE(hscale1), 0);
    gtk_scale_set_draw_value(GTK_SCALE(hscale1), FALSE);
    gtk_range_set_update_policy(GTK_RANGE(hscale1),
				GTK_UPDATE_DISCONTINUOUS);
    mAdjustment1 = gtk_range_get_adjustment(GTK_RANGE(hscale1));
    gtk_widget_show(hscale1);
    gtk_box_pack_start(GTK_BOX(sliderhbox), hscale1, TRUE, TRUE, 10);

   
    dialog_action_area1 = GTK_DIALOG(dialog_enhance_contrast)->action_area;
    gtk_object_set_data(GTK_OBJECT(dialog_enhance_contrast),
			"dialog_action_area1", dialog_action_area1);
    gtk_widget_show(dialog_action_area1);
    gtk_container_set_border_width(GTK_CONTAINER(dialog_action_area1), 10);
    
      
    sliderhbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), sliderhbox, TRUE, FALSE, 0);
    gtk_widget_show(sliderhbox);
    
    label1 = gtk_label_new("Maximum");
    gtk_widget_show(label1);
    gtk_box_pack_start(GTK_BOX(sliderhbox), label1, FALSE, FALSE, 10);
    
    hscale1 =
	gtk_hscale_new(GTK_ADJUSTMENT
		       (gtk_adjustment_new(255, 0, 255, 5, 5, 0)));
    gtk_scale_set_digits(GTK_SCALE(hscale1), 0);
    gtk_scale_set_draw_value(GTK_SCALE(hscale1), FALSE);
    gtk_range_set_update_policy(GTK_RANGE(hscale1),
				GTK_UPDATE_DISCONTINUOUS);
    mAdjustment2 = gtk_range_get_adjustment(GTK_RANGE(hscale1));
    gtk_widget_show(hscale1);
    gtk_box_pack_start(GTK_BOX(sliderhbox), hscale1, TRUE, TRUE, 10);

    dialog_action_area1 = GTK_DIALOG(dialog_enhance_contrast)->action_area;
    gtk_object_set_data(GTK_OBJECT(dialog_enhance_contrast),
			"dialog_action_area1", dialog_action_area1);
    gtk_widget_show(dialog_action_area1);
    gtk_container_set_border_width(GTK_CONTAINER(dialog_action_area1), 10);

    hbuttonbox1 = gtk_hbutton_box_new();
    gtk_widget_show(hbuttonbox1);
    gtk_box_pack_start(GTK_BOX(dialog_action_area1), hbuttonbox1, TRUE,
		       TRUE, 0);

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, ok_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("OK");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

    button1 = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(button1), tmpBox);
    gtk_widget_show(button1);
    gtk_container_add(GTK_CONTAINER(hbuttonbox1), button1);

    button3 = gtk_button_new_with_label("Reset Image");
    gtk_widget_show(button3);
    gtk_container_add(GTK_CONTAINER(hbuttonbox1), button3);

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new("Cancel");
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

    button2 = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(button2), tmpBox);
    gtk_widget_show(button2);
    gtk_container_add(GTK_CONTAINER(hbuttonbox1), button2);

    //The signals:

    gtk_signal_connect(GTK_OBJECT(mAdjustment1), "value_changed",
		       GTK_SIGNAL_FUNC(on_enhance_contrast_minSlider_moved),
		       (void *) this);

    gtk_signal_connect(GTK_OBJECT(mAdjustment2), "value_changed",
		       GTK_SIGNAL_FUNC(on_enhance_contrast_maxSlider_moved),
		       (void *) this);

    gtk_signal_connect(GTK_OBJECT(dialog_enhance_contrast), "delete_event",
		       GTK_SIGNAL_FUNC(on_enhance_contrast_delete_event),
		       (void *) this);

    gtk_signal_connect(GTK_OBJECT(button1), "clicked",
		       GTK_SIGNAL_FUNC
		       (on_enhance_contrast_button_ok_clicked),
		       (void *) this);

    gtk_signal_connect(GTK_OBJECT(button2), "clicked",
		       GTK_SIGNAL_FUNC
		       (on_enhance_contrast_button_cancel_clicked), 
		       (void *) this);

    gtk_signal_connect(GTK_OBJECT(button3), "clicked",
		       GTK_SIGNAL_FUNC
		       (on_enhance_contrast_button_reset_clicked),
		       (void *) this);
    return dialog_enhance_contrast;
}

//*******************************************************************
//
//
gboolean on_enhance_contrast_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	EnhanceContrastDialog *dlg = (EnhanceContrastDialog *) userData;

	if (NULL == dlg)
		return FALSE;
	
	dlg->updateContrast(0,255);

	delete dlg;

	return TRUE;
}

//*******************************************************************
//
//
void on_enhance_contrast_button_ok_clicked(
	GtkButton *button,
	gpointer userData)
{
	EnhanceContrastDialog *dlg = (EnhanceContrastDialog *) userData;
	
	if (NULL == dlg)
		return;

	dlg->updateContrast((unsigned) round(dlg->mAdjustment1->value), 
			(unsigned) round(dlg->mAdjustment2->value));

	//***1.8 - add ONLY final contrast modification to list
	ImageMod 
		imod(ImageMod::IMG_contrast,
			(int) round(dlg->mAdjustment1->value),
			(int) round(dlg->mAdjustment2->value));
	dlg->mTraceWin->theImageMods().add(imod);

	delete dlg;
}

//*******************************************************************
//
//
void on_enhance_contrast_button_cancel_clicked(
	GtkButton *button,
	gpointer userData)
{
	EnhanceContrastDialog *dlg = (EnhanceContrastDialog *) userData;

	if (NULL == dlg)
		return;

	dlg->updateContrast(0, 255);

	delete dlg;
}

//*******************************************************************
//
//
void on_enhance_contrast_minSlider_moved(
	GtkButton *button,
	gpointer userData)
{
	EnhanceContrastDialog *dlg = (EnhanceContrastDialog *) userData;
	
	if (NULL == dlg)
		return;
	
	if ((dlg->mAdjustment1->value) < (dlg->mAdjustment2->value))
		dlg->updateContrast((unsigned) round(dlg->mAdjustment1->value), 
				(unsigned) round(dlg->mAdjustment2->value));

	else
		gtk_adjustment_set_value(dlg->mAdjustment1, (dlg->mAdjustment1->value)-1);
}

//*******************************************************************
//
//
void on_enhance_contrast_maxSlider_moved(
	GtkButton *button,
	gpointer userData)
{
	EnhanceContrastDialog *dlg = (EnhanceContrastDialog *) userData;

	if (NULL == dlg)
		return;

	if ((dlg->mAdjustment2->value) > (dlg->mAdjustment1->value))
		dlg->updateContrast((unsigned) round(dlg->mAdjustment1->value), 
			(unsigned) round(dlg->mAdjustment2->value));

	else 
		gtk_adjustment_set_value(dlg->mAdjustment2, (dlg->mAdjustment2->value)+1);
}


//*******************************************************************
//
//
void on_enhance_contrast_button_reset_clicked(
	GtkButton *button,
	gpointer userData)
{
	EnhanceContrastDialog *dlg = (EnhanceContrastDialog *) userData;

	if (NULL == dlg)
		return;

	dlg->updateContrast(0, 255);
	gtk_adjustment_set_value(dlg->mAdjustment1, 0);
	gtk_adjustment_set_value(dlg->mAdjustment2, 255);
}
