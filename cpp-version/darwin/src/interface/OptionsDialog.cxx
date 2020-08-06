//*******************************************************************
//   file: OptionsDialog.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (1/26/2008)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include <gdk/gdkkeysyms.h>
#include <string.h>

#include "../Error.h"
#include "OptionsDialog.h"
#include "../support.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef DEBUG
#include <iostream>
#endif
#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#include "../../pixmaps/cancel.xpm"
#include "../../pixmaps/ok.xpm"

gboolean on_optionsDialog_delete_event(
				GtkWidget *widget,
				GdkEvent *event,
				gpointer userData);

void on_colorButtonChoose_clicked(
        GtkButton *button,
        gpointer userData);

void on_optionsButtonOK_clicked(
        GtkButton *button,
        gpointer userData);

void on_optionsButtonCancel_clicked(
        GtkButton *button,
        gpointer userData);

gboolean on_colorSelectionDialog_delete_event(
        GtkWidget *widget,
        GdkEvent *event,
        gpointer userData);

void on_colorSelectOKButton_clicked(
        GtkButton *button,
        gpointer userData);

void on_colorSelectCancelButton_clicked(
        GtkButton *button,
        gpointer userData);

void on_colorSelectHelpButton_clicked(
        GtkButton *button,
        gpointer userData);

void on_fontSelectButton_clicked( //***1.85
        GtkButton *button,
        gpointer userData);

void on_fontSelectDialog_OK_Button_clicked( //***1.85
        GtkButton *button,
        gpointer userData);

void on_fontSelectDialog_CNX_Button_clicked( //***1.85
        GtkButton *button,
        gpointer userData);

using namespace std;

static int gNumReferences = 0;

int getNumOptionsDialogReferences()
{
    return gNumReferences;
}

//**************************************************************

OptionsDialog::OptionsDialog(MainWindow *m, Options * o)
	: mDialog(createOptionsDialog(o)),
	  mColorSelectionDialog(NULL),
	  mFontSelectionDialog(NULL), //***1.85
	  mMainWin(m),
	  mOptions(o)
{
    if (NULL == o)
    	throw Error("NULL argument to OptionsDialog ctor.");

    memcpy(mColor, o->mCurrentColor, 4 * sizeof(double));

    gNumReferences++;
}

//**************************************************************

OptionsDialog::~OptionsDialog()
{
    gtk_widget_destroy(mDialog);
    gNumReferences--;
}

void OptionsDialog::show()
{
    gtk_widget_show(mDialog);
}

//**************************************************************

GtkWidget *OptionsDialog::createOptionsDialog(Options *o)
{
    GtkWidget *optionsDialog;
    GtkWidget *optionsVBox;
    GtkWidget *optionsNotebook;
    GtkWidget *optionsGeneralVBox;
    GtkWidget *optionsToolbarFrame;
    GtkWidget *optionsToolbarVBox;
    GSList *toolbarGroup_group = NULL;
    GSList *idGroup_group = NULL; //***1.65
    GtkWidget *optionsGeneralLabel;
    GtkWidget *optionsTracingVBox;
    GtkWidget *optionsColorFrame;
    GtkWidget *optionsColorHBox;
    GtkWidget *optionsColorLabel;
    GtkWidget *colorButtonChoose;
    GtkWidget *optionsActiveContourFrame;
    GtkWidget *activeContourTable;
    GtkWidget *acIterationsLabel;
    GtkWidget *acContinuityLabel;
    GtkWidget *acLinearityLabel;
    GtkWidget *acEdgeLabel;
    GtkObject *mACContinuitySpinButton_adj;
    GtkObject *mACLinearitySpinButton_adj;
    GtkObject *mACEdgeSpinButton_adj;
    GtkObject *mACIterationsSpinButton_adj;
    GtkWidget *optionsTracingLabel;
    GtkWidget *optionsImageProVBox;
    GtkWidget *ceFrame;
    GtkWidget *ceTable;
    GtkWidget *ceGaussianStdDevLabel;
    GtkWidget *ceLowThresholdLabel;
    GtkWidget *ceHighThreshold;
    GtkObject *mCEGaussianStdDevSpinButton_adj;
    GtkObject *mCELowThresholdSpinButton_adj;
    GtkObject *mCEHighThresholdSpinButton_adj;
    GtkWidget *optionsImageProLabel;
    GtkWidget *optionsMatchingVBox;
    GtkWidget *optionsMatchingLabel;
    GtkWidget *dialog_action_area1;
    GtkWidget *optionsHButtonBox;
    guint optionsButtonOK_key;
    GtkWidget *optionsButtonOK;
    guint optionsButtonCancel_key;
    GtkWidget *optionsButtonCancel;
    GtkAccelGroup *accel_group;
    GtkWidget *tmpBox, *tmpLabel, *tmpIcon;

    accel_group = gtk_accel_group_new();

    optionsDialog = gtk_dialog_new();
    gtk_object_set_data(GTK_OBJECT(optionsDialog), "optionsDialog",
			optionsDialog);
    gtk_window_set_title(GTK_WINDOW(optionsDialog), _("Options"));
    GTK_WINDOW(optionsDialog)->type = WINDOW_DIALOG;
    gtk_window_set_policy(GTK_WINDOW(optionsDialog), TRUE, TRUE, FALSE);
    gtk_window_set_wmclass(GTK_WINDOW(optionsDialog), "darwin_options", "DARWIN");

    optionsVBox = GTK_DIALOG(optionsDialog)->vbox;
    gtk_object_set_data(GTK_OBJECT(optionsDialog), "optionsVBox",
			optionsVBox);
    gtk_widget_show(optionsVBox);
    gtk_container_set_border_width(GTK_CONTAINER(optionsVBox), 3);

    optionsNotebook = gtk_notebook_new();
    gtk_widget_show(optionsNotebook);
    gtk_box_pack_start(GTK_BOX(optionsVBox), optionsNotebook, TRUE, TRUE,
		       2);

    optionsGeneralVBox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(optionsGeneralVBox);
    gtk_container_add(GTK_CONTAINER(optionsNotebook), optionsGeneralVBox);

    optionsToolbarFrame = gtk_frame_new(_("Toolbar Settings"));
    gtk_widget_show(optionsToolbarFrame);
    gtk_box_pack_start(GTK_BOX(optionsGeneralVBox), optionsToolbarFrame,
		       FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(optionsToolbarFrame), 3);
    gtk_frame_set_shadow_type(GTK_FRAME(optionsToolbarFrame),
			      GTK_SHADOW_IN);

    optionsToolbarVBox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(optionsToolbarVBox);
    gtk_container_add(GTK_CONTAINER(optionsToolbarFrame),
		      optionsToolbarVBox);

    mToolbarRadioButtonText =
	gtk_radio_button_new_with_label(toolbarGroup_group,
					_("Text Only"));
    toolbarGroup_group =
	gtk_radio_button_group(GTK_RADIO_BUTTON(mToolbarRadioButtonText));
    gtk_widget_show(mToolbarRadioButtonText);
    gtk_box_pack_start(GTK_BOX(optionsToolbarVBox),
		       mToolbarRadioButtonText, FALSE, FALSE, 0);

    mToolbarRadioButtonPictures =
	gtk_radio_button_new_with_label(toolbarGroup_group,
					_("Pictures Only"));
    toolbarGroup_group =
	gtk_radio_button_group(GTK_RADIO_BUTTON
			       (mToolbarRadioButtonPictures));
    gtk_widget_show(mToolbarRadioButtonPictures);
    gtk_box_pack_start(GTK_BOX(optionsToolbarVBox),
		       mToolbarRadioButtonPictures, FALSE, FALSE, 0);

    mToolbarRadioButtonBoth =
	gtk_radio_button_new_with_label(toolbarGroup_group,
					_("Both Pictures and Text"));
    toolbarGroup_group =
	gtk_radio_button_group(GTK_RADIO_BUTTON(mToolbarRadioButtonBoth));
    gtk_widget_show(mToolbarRadioButtonBoth);
    gtk_box_pack_start(GTK_BOX(optionsToolbarVBox),
		       mToolbarRadioButtonBoth, FALSE, FALSE, 0);

    switch (o->mToolbarDisplay) {
	    case BOTH:
		    gtk_toggle_button_set_active(
				    GTK_TOGGLE_BUTTON(mToolbarRadioButtonBoth),
				    TRUE);
		    break;
	    case PICTURES:
		    gtk_toggle_button_set_active(
				    GTK_TOGGLE_BUTTON(mToolbarRadioButtonPictures),
				    TRUE);
		    break;
	    case TEXT:
		    gtk_toggle_button_set_active(
				    GTK_TOGGLE_BUTTON(mToolbarRadioButtonText),
				    TRUE);
		    break;
    }

	//***1.65 - BEGINNING OF NEW CODE for  Show/Hide ID's

	optionsToolbarFrame = gtk_frame_new(_("Dolphin ID Settings (for testing)"));
    gtk_widget_show(optionsToolbarFrame);
    gtk_box_pack_start(GTK_BOX(optionsGeneralVBox), optionsToolbarFrame,
		       FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(optionsToolbarFrame), 3);
    gtk_frame_set_shadow_type(GTK_FRAME(optionsToolbarFrame),
			      GTK_SHADOW_IN);

    optionsToolbarVBox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(optionsToolbarVBox);
    gtk_container_add(GTK_CONTAINER(optionsToolbarFrame),
		      optionsToolbarVBox);

    mShowIDsButton =
	gtk_radio_button_new_with_label(idGroup_group,
					_("Show Dolphin IDs in all windows"));
    idGroup_group =
	gtk_radio_button_group(GTK_RADIO_BUTTON(mShowIDsButton));
    gtk_widget_show(mShowIDsButton);
    gtk_box_pack_start(GTK_BOX(optionsToolbarVBox),
		       mShowIDsButton, FALSE, FALSE, 0);

    mHideIDsButton =
	gtk_radio_button_new_with_label(idGroup_group,
					_("Hide / Use fake IDs (for blind testing)"));
    idGroup_group =
	gtk_radio_button_group(GTK_RADIO_BUTTON(mHideIDsButton));
    gtk_widget_show(mHideIDsButton);
    gtk_box_pack_start(GTK_BOX(optionsToolbarVBox),
		       mHideIDsButton, FALSE, FALSE, 0);

    if (! o->mHideIDs) 
		gtk_toggle_button_set_active(
				    GTK_TOGGLE_BUTTON(mShowIDsButton),
					TRUE);
	else
		gtk_toggle_button_set_active(
				    GTK_TOGGLE_BUTTON(mHideIDsButton),
					TRUE);

	//***1.65 - END OF NEW CODE

    optionsGeneralLabel = gtk_label_new(_("General"));
    gtk_widget_show(optionsGeneralLabel);
    gtk_notebook_set_tab_label(GTK_NOTEBOOK(optionsNotebook),
			       gtk_notebook_get_nth_page(GTK_NOTEBOOK
							 (optionsNotebook),
							 0),
			       optionsGeneralLabel);

    optionsTracingVBox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(optionsTracingVBox);
    gtk_container_add(GTK_CONTAINER(optionsNotebook), optionsTracingVBox);

    optionsColorFrame = gtk_frame_new(_("Color Selection"));
    gtk_widget_show(optionsColorFrame);
    gtk_box_pack_start(GTK_BOX(optionsTracingVBox), optionsColorFrame,
		       FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(optionsColorFrame), 3);
    gtk_frame_set_shadow_type(GTK_FRAME(optionsColorFrame), GTK_SHADOW_IN);

    optionsColorHBox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(optionsColorHBox);
    gtk_container_add(GTK_CONTAINER(optionsColorFrame), optionsColorHBox);
    gtk_container_set_border_width(GTK_CONTAINER(optionsColorHBox), 3);

    optionsColorLabel = gtk_label_new(_("Tracing Color:"));
    gtk_widget_show(optionsColorLabel);
    gtk_box_pack_start(GTK_BOX(optionsColorHBox), optionsColorLabel, FALSE,
		       FALSE, 8);

    colorButtonChoose = gtk_button_new_with_label(_("Choose..."));
    gtk_widget_show(colorButtonChoose);
    gtk_box_pack_start(GTK_BOX(optionsColorHBox), colorButtonChoose, FALSE,
		       FALSE, 0);

    optionsActiveContourFrame = gtk_frame_new(_("Active Contour"));
    gtk_widget_show(optionsActiveContourFrame);
    gtk_box_pack_start(GTK_BOX(optionsTracingVBox),
		       optionsActiveContourFrame, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER
				   (optionsActiveContourFrame), 3);
    gtk_frame_set_shadow_type(GTK_FRAME(optionsActiveContourFrame),
			      GTK_SHADOW_IN);

    activeContourTable = gtk_table_new(4, 2, FALSE);
    gtk_widget_show(activeContourTable);
    gtk_container_add(GTK_CONTAINER(optionsActiveContourFrame),
		      activeContourTable);
    gtk_container_set_border_width(GTK_CONTAINER(activeContourTable), 3);

    acIterationsLabel = gtk_label_new(_("Maximum Number of Iterations: "));
    gtk_widget_show(acIterationsLabel);
    gtk_table_attach(GTK_TABLE(activeContourTable), acIterationsLabel, 0,
		     1, 0, 1, (GtkAttachOptions) (0),
		     (GtkAttachOptions) (0), 0, 0);

    acContinuityLabel = gtk_label_new(_("Continuity Energy Weight: "));
    gtk_widget_show(acContinuityLabel);
    gtk_table_attach(GTK_TABLE(activeContourTable), acContinuityLabel, 0,
		     1, 1, 2, (GtkAttachOptions) (0),
		     (GtkAttachOptions) (0), 0, 0);

    acLinearityLabel = gtk_label_new(_("Linearity Energy Weight: "));
    gtk_widget_show(acLinearityLabel);
    gtk_table_attach(GTK_TABLE(activeContourTable), acLinearityLabel, 0, 1,
		     2, 3, (GtkAttachOptions) (0), (GtkAttachOptions) (0),
		     0, 0);

    acEdgeLabel = gtk_label_new(_("Edge Energy Weight: "));
    gtk_widget_show(acEdgeLabel);
    gtk_table_attach(GTK_TABLE(activeContourTable), acEdgeLabel, 0, 1, 3,
		     4, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0,
		     0);

    mACContinuitySpinButton_adj =
	gtk_adjustment_new(o->mContinuityWeight, 0, 20, 0.1, 0.5, 0.5);
    mACContinuitySpinButton =
	gtk_spin_button_new(GTK_ADJUSTMENT(mACContinuitySpinButton_adj), 1,
			    1);
    gtk_widget_show(mACContinuitySpinButton);
    gtk_table_attach(GTK_TABLE(activeContourTable),
		     mACContinuitySpinButton, 1, 2, 1, 2,
		     (GtkAttachOptions) (GTK_EXPAND),
		     (GtkAttachOptions) (0), 0, 0);

    mACLinearitySpinButton_adj = gtk_adjustment_new(o->mLinearityWeight, 0, 20, 0.1, 1, 1);
    mACLinearitySpinButton =
	gtk_spin_button_new(GTK_ADJUSTMENT(mACLinearitySpinButton_adj), 1,
			    1);
    gtk_widget_show(mACLinearitySpinButton);
    gtk_table_attach(GTK_TABLE(activeContourTable), mACLinearitySpinButton,
		     1, 2, 2, 3, (GtkAttachOptions) (GTK_EXPAND),
		     (GtkAttachOptions) (0), 0, 0);

    mACEdgeSpinButton_adj = gtk_adjustment_new(o->mEdgeWeight, 0, 20, 0.1, 1, 1);
    mACEdgeSpinButton =
	gtk_spin_button_new(GTK_ADJUSTMENT(mACEdgeSpinButton_adj), 1, 1);
    gtk_widget_show(mACEdgeSpinButton);
    gtk_table_attach(GTK_TABLE(activeContourTable), mACEdgeSpinButton, 1,
		     2, 3, 4, (GtkAttachOptions) (GTK_EXPAND),
		     (GtkAttachOptions) (0), 0, 0);

    mACIterationsSpinButton_adj =
	gtk_adjustment_new(o->mMaxIterations, 0, 100, 1, 10, 10);
    mACIterationsSpinButton =
	gtk_spin_button_new(GTK_ADJUSTMENT(mACIterationsSpinButton_adj), 1,
			    0);
    gtk_widget_show(mACIterationsSpinButton);
    gtk_table_attach(GTK_TABLE(activeContourTable),
		     mACIterationsSpinButton, 1, 2, 0, 1,
		     (GtkAttachOptions) (GTK_EXPAND),
		     (GtkAttachOptions) (0), 0, 0);

    optionsTracingLabel = gtk_label_new(_("Tracing"));
    gtk_widget_show(optionsTracingLabel);
    gtk_notebook_set_tab_label(GTK_NOTEBOOK(optionsNotebook),
			       gtk_notebook_get_nth_page(GTK_NOTEBOOK
							 (optionsNotebook),
							 1),
			       optionsTracingLabel);

    optionsImageProVBox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(optionsImageProVBox);
    gtk_container_add(GTK_CONTAINER(optionsNotebook), optionsImageProVBox);

    ceFrame = gtk_frame_new(_("Canny Edge Detector"));
    gtk_widget_show(ceFrame);
    gtk_box_pack_start(GTK_BOX(optionsImageProVBox), ceFrame, FALSE, TRUE,
		       0);
    gtk_container_set_border_width(GTK_CONTAINER(ceFrame), 3);
    gtk_frame_set_shadow_type(GTK_FRAME(ceFrame), GTK_SHADOW_IN);

    ceTable = gtk_table_new(3, 2, FALSE);
    gtk_widget_show(ceTable);
    gtk_container_add(GTK_CONTAINER(ceFrame), ceTable);
    gtk_container_set_border_width(GTK_CONTAINER(ceTable), 3);

    ceGaussianStdDevLabel =
	gtk_label_new(_("Gaussian Standard Deviation: "));
    gtk_widget_show(ceGaussianStdDevLabel);
    gtk_table_attach(GTK_TABLE(ceTable), ceGaussianStdDevLabel, 0, 1, 0, 1,
		     (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);

    ceLowThresholdLabel = gtk_label_new(_("Low Threshold: "));
    gtk_widget_show(ceLowThresholdLabel);
    gtk_table_attach(GTK_TABLE(ceTable), ceLowThresholdLabel, 0, 1, 1, 2,
		     (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);

    ceHighThreshold = gtk_label_new(_("High Threshold: "));
    gtk_widget_show(ceHighThreshold);
    gtk_table_attach(GTK_TABLE(ceTable), ceHighThreshold, 0, 1, 2, 3,
		     (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);

    mCEGaussianStdDevSpinButton_adj =
	gtk_adjustment_new(o->mGaussianStdDev, 0, 2, 0.01, 0.2, 0.2);
    mCEGaussianStdDevSpinButton =
	gtk_spin_button_new(GTK_ADJUSTMENT
			    (mCEGaussianStdDevSpinButton_adj), 1, 2);
    gtk_widget_show(mCEGaussianStdDevSpinButton);
    gtk_table_attach(GTK_TABLE(ceTable), mCEGaussianStdDevSpinButton, 1, 2,
		     0, 1, (GtkAttachOptions) (GTK_EXPAND),
		     (GtkAttachOptions) (0), 0, 0);

    mCELowThresholdSpinButton_adj =
	gtk_adjustment_new(o->mLowThreshold, 0, 1, 0.01, 0.1, 0.1);
    mCELowThresholdSpinButton =
	gtk_spin_button_new(GTK_ADJUSTMENT(mCELowThresholdSpinButton_adj),
			    1, 2);
    gtk_widget_show(mCELowThresholdSpinButton);
    gtk_table_attach(GTK_TABLE(ceTable), mCELowThresholdSpinButton, 1, 2,
		     1, 2, (GtkAttachOptions) (GTK_EXPAND),
		     (GtkAttachOptions) (0), 0, 0);

    mCEHighThresholdSpinButton_adj =
	gtk_adjustment_new(o->mHighThreshold, 0, 1, 0.01, 0.1, 0.1);
    mCEHighThresholdSpinButton =
	gtk_spin_button_new(GTK_ADJUSTMENT(mCEHighThresholdSpinButton_adj),
			    1, 2);
    gtk_widget_show(mCEHighThresholdSpinButton);
    gtk_table_attach(GTK_TABLE(ceTable), mCEHighThresholdSpinButton, 1, 2,
		     2, 3, (GtkAttachOptions) (GTK_EXPAND),
		     (GtkAttachOptions) (0), 0, 0);

    optionsImageProLabel = gtk_label_new(_("Image Processing"));
    gtk_widget_show(optionsImageProLabel);
    gtk_notebook_set_tab_label(GTK_NOTEBOOK(optionsNotebook),
			       gtk_notebook_get_nth_page(GTK_NOTEBOOK
							 (optionsNotebook),
							 2),
			       optionsImageProLabel);

    optionsMatchingVBox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(optionsMatchingVBox);
    gtk_container_add(GTK_CONTAINER(optionsNotebook), optionsMatchingVBox);

    optionsMatchingLabel = gtk_label_new(_("Matching"));
    gtk_widget_show(optionsMatchingLabel);
    gtk_notebook_set_tab_label(GTK_NOTEBOOK(optionsNotebook),
			       gtk_notebook_get_nth_page(GTK_NOTEBOOK
							 (optionsNotebook),
							 3),
			       optionsMatchingLabel);

	//***1.85 - new tab for font selection

    GtkWidget *optionsFontVBox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(optionsFontVBox), 20);
    gtk_widget_show(optionsFontVBox);
    gtk_container_add(GTK_CONTAINER(optionsNotebook), optionsFontVBox);

	GtkWidget *fontLabel = gtk_label_new("Current List Font");
	gtk_widget_show(fontLabel);
	gtk_box_pack_start(GTK_BOX(optionsFontVBox),fontLabel,FALSE,FALSE,10);

	mFontName = gtk_entry_new();
	gtk_entry_set_width_chars(GTK_ENTRY(mFontName), o->mCurrentFontName.length());
	gtk_entry_set_text(GTK_ENTRY(mFontName), o->mCurrentFontName.c_str());
	gtk_entry_set_editable(GTK_ENTRY(mFontName), FALSE); // no change allowed
	gtk_widget_show(mFontName);
	gtk_box_pack_start(GTK_BOX(optionsFontVBox),mFontName,FALSE,FALSE,10);

	fontLabel = gtk_label_new("Sample Text in Font:");
	gtk_widget_show(fontLabel);
	gtk_box_pack_start(GTK_BOX(optionsFontVBox),fontLabel,FALSE,FALSE,10);

	mFontSample = gtk_entry_new();
	gtk_widget_modify_font(
			mFontSample,
			pango_font_description_from_string(o->mCurrentFontName.c_str()));
	gtk_entry_set_text(GTK_ENTRY(mFontSample), "ABCLMNOP abclmnop 012345");
	gtk_entry_set_editable(GTK_ENTRY(mFontSample), FALSE); // no change allowed
	gtk_widget_show(mFontSample);
	gtk_box_pack_start(GTK_BOX(optionsFontVBox),mFontSample,FALSE,FALSE,10);

	GtkWidget *fontSelectButton = gtk_button_new_with_label("Change Font used in Lists");
	gtk_widget_show(fontSelectButton);
	gtk_box_pack_start(GTK_BOX(optionsFontVBox),fontSelectButton,FALSE,FALSE,10);

    GtkWidget *optionsFontLabel = gtk_label_new(_("Fonts"));
    gtk_widget_show(optionsFontLabel);
    gtk_notebook_set_tab_label(GTK_NOTEBOOK(optionsNotebook),
			       gtk_notebook_get_nth_page(GTK_NOTEBOOK
							 (optionsNotebook),
							 4),
			       optionsFontLabel);


    dialog_action_area1 = GTK_DIALOG(optionsDialog)->action_area;
    gtk_object_set_data(GTK_OBJECT(optionsDialog), "dialog_action_area1",
			dialog_action_area1);
    gtk_widget_show(dialog_action_area1);
    gtk_container_set_border_width(GTK_CONTAINER(dialog_action_area1), 5);

    optionsHButtonBox = gtk_hbutton_box_new();
    gtk_widget_show(optionsHButtonBox);
    gtk_box_pack_start(GTK_BOX(dialog_action_area1), optionsHButtonBox,
		       TRUE, TRUE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(optionsHButtonBox),
			      GTK_BUTTONBOX_END);

  tmpBox = gtk_hbox_new(FALSE, 0);
  tmpIcon = create_pixmap_from_data(tmpBox, ok_xpm);
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
  gtk_widget_show(tmpIcon);
  tmpLabel = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
  gtk_widget_show(tmpLabel);
  gtk_widget_show(tmpBox);

    optionsButtonOK = gtk_button_new();
    optionsButtonOK_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
			      _("_OK"));
    gtk_widget_add_accelerator(optionsButtonOK, "clicked", accel_group,
			       optionsButtonOK_key, GDK_MOD1_MASK,
			       (GtkAccelFlags) 0);
    gtk_container_add(GTK_CONTAINER(optionsButtonOK), tmpBox);
    gtk_widget_show(optionsButtonOK);
    gtk_container_add(GTK_CONTAINER(optionsHButtonBox), optionsButtonOK);
    GTK_WIDGET_SET_FLAGS(optionsButtonOK, GTK_CAN_DEFAULT);
    gtk_widget_add_accelerator(optionsButtonOK, "clicked", accel_group,
			       GDK_O, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

  tmpBox = gtk_hbox_new(FALSE, 0);
  tmpIcon = create_pixmap_from_data(tmpBox, cancel_xpm);
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
  gtk_widget_show(tmpIcon);
  tmpLabel = gtk_label_new("");
  gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
  gtk_widget_show(tmpLabel);
  gtk_widget_show(tmpBox);

    optionsButtonCancel = gtk_button_new();
    optionsButtonCancel_key =
	gtk_label_parse_uline(GTK_LABEL(tmpLabel),
			      _("_Cancel"));
    gtk_widget_add_accelerator(optionsButtonCancel, "clicked", accel_group,
			       optionsButtonCancel_key, GDK_MOD1_MASK,
			       (GtkAccelFlags) 0);
    gtk_container_add(GTK_CONTAINER(optionsButtonCancel), tmpBox);
    gtk_widget_show(optionsButtonCancel);
    gtk_container_add(GTK_CONTAINER(optionsHButtonBox),
		      optionsButtonCancel);
    GTK_WIDGET_SET_FLAGS(optionsButtonCancel, GTK_CAN_DEFAULT);
    gtk_widget_add_accelerator(optionsButtonCancel, "clicked", accel_group,
			       GDK_C, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(optionsButtonCancel, "clicked", accel_group,
			       GDK_Escape, (GdkModifierType)0, GTK_ACCEL_VISIBLE);

    gtk_signal_connect(GTK_OBJECT(optionsDialog), "delete_event",
		       GTK_SIGNAL_FUNC(on_optionsDialog_delete_event),
		       (void *) this);
	//***1.85 - new for font selection
    gtk_signal_connect(GTK_OBJECT(fontSelectButton), "clicked",
		       GTK_SIGNAL_FUNC(on_fontSelectButton_clicked),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(colorButtonChoose), "clicked",
		       GTK_SIGNAL_FUNC(on_colorButtonChoose_clicked),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(optionsButtonOK), "clicked",
		       GTK_SIGNAL_FUNC(on_optionsButtonOK_clicked),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(optionsButtonCancel), "clicked",
		       GTK_SIGNAL_FUNC(on_optionsButtonCancel_clicked),
		       (void *) this);

    gtk_widget_grab_default(optionsButtonOK);
    gtk_window_add_accel_group(GTK_WINDOW(optionsDialog), accel_group);

    return optionsDialog;
}

//**************************************************************

GtkWidget *OptionsDialog::createColorSelectionDialog()
{
    GtkWidget *colorSelectionDialog;
    GtkWidget *colorSelectOKButton;
    GtkWidget *colorSelectCancelButton;
    GtkWidget *colorSelectHelpButton;
    GtkAccelGroup *accel_group;

    accel_group = gtk_accel_group_new();

    colorSelectionDialog =
	gtk_color_selection_dialog_new(_("Select Color"));
    gtk_object_set_data(GTK_OBJECT(colorSelectionDialog),
			"colorSelectionDialog", colorSelectionDialog);
    gtk_container_set_border_width(GTK_CONTAINER(colorSelectionDialog),
				   10);
    GTK_WINDOW(colorSelectionDialog)->type = WINDOW_DIALOG;

    colorSelectOKButton =
	GTK_COLOR_SELECTION_DIALOG(colorSelectionDialog)->ok_button;
    gtk_object_set_data(GTK_OBJECT(colorSelectionDialog),
			"colorSelectOKButton", colorSelectOKButton);
    gtk_widget_show(colorSelectOKButton);
    GTK_WIDGET_SET_FLAGS(colorSelectOKButton, GTK_CAN_DEFAULT);
    gtk_widget_add_accelerator(colorSelectOKButton, "clicked", accel_group,
			       GDK_O, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

    colorSelectCancelButton =
	GTK_COLOR_SELECTION_DIALOG(colorSelectionDialog)->cancel_button;
    gtk_object_set_data(GTK_OBJECT(colorSelectionDialog),
			"colorSelectCancelButton",
			colorSelectCancelButton);
    gtk_widget_show(colorSelectCancelButton);
    GTK_WIDGET_SET_FLAGS(colorSelectCancelButton, GTK_CAN_DEFAULT);
    gtk_widget_add_accelerator(colorSelectCancelButton, "clicked",
			       accel_group, GDK_C, GDK_MOD1_MASK,
			       GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator(colorSelectCancelButton, "clicked",
			       accel_group, GDK_Escape, (GdkModifierType)0,
			       GTK_ACCEL_VISIBLE);

    colorSelectHelpButton =
	GTK_COLOR_SELECTION_DIALOG(colorSelectionDialog)->help_button;
    gtk_object_set_data(GTK_OBJECT(colorSelectionDialog),
			"colorSelectHelpButton", colorSelectHelpButton);
    gtk_widget_show(colorSelectHelpButton);
    GTK_WIDGET_SET_FLAGS(colorSelectHelpButton, GTK_CAN_DEFAULT);

    gtk_signal_connect(GTK_OBJECT(colorSelectionDialog), "delete_event",
		       GTK_SIGNAL_FUNC
		       (on_colorSelectionDialog_delete_event),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(colorSelectOKButton), "clicked",
		       GTK_SIGNAL_FUNC(on_colorSelectOKButton_clicked),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(colorSelectCancelButton), "clicked",
		       GTK_SIGNAL_FUNC(on_colorSelectCancelButton_clicked),
		       (void *) this);
    gtk_signal_connect(GTK_OBJECT(colorSelectHelpButton), "clicked",
		       GTK_SIGNAL_FUNC(on_colorSelectHelpButton_clicked),
		       (void *) this);

    gtk_widget_grab_default(colorSelectOKButton);
    gtk_window_add_accel_group(GTK_WINDOW(colorSelectionDialog),
			       accel_group);

    return colorSelectionDialog;
}

//**************************************************************

gboolean on_optionsDialog_delete_event(GtkWidget * widget,
				       GdkEvent * event, gpointer userData)
{
    OptionsDialog *dlg = (OptionsDialog *) userData;

    if (NULL == dlg)
	return FALSE;

    dlg->mMainWin->displayStatusMessage(_("Options change canceled."));
    
    delete dlg;

    return TRUE;
}

//**************************************************************

void on_colorButtonChoose_clicked(GtkButton * button, gpointer userData)
{
    OptionsDialog *dlg = (OptionsDialog *) userData;

    if (NULL == dlg)
	return;

    if (NULL == dlg->mColorSelectionDialog) {
	dlg->mColorSelectionDialog = dlg->createColorSelectionDialog();
	gtk_widget_show(dlg->mColorSelectionDialog);
    }
}

//**************************************************************

void on_optionsButtonOK_clicked(GtkButton * button, gpointer userData)
{
    OptionsDialog *dlg = (OptionsDialog *) userData;

    if (NULL == dlg)
	return;

    int numIterations;

    float cont, lin, edge, high, low, gauss;

    numIterations =
	gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON
					 (dlg->mACIterationsSpinButton));
    cont =
	gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON
					   (dlg->mACContinuitySpinButton));
    lin =
	gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON
					   (dlg->mACLinearitySpinButton));
    edge =
	gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON
					   (dlg->mACEdgeSpinButton));

    high =
	gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON
					   (dlg->mCEHighThresholdSpinButton));
    low =
	gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON
					   (dlg->mCELowThresholdSpinButton));
    gauss =
	gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON
					   (dlg->mCEGaussianStdDevSpinButton));

    toolbarDisplayType tb;

    if (GTK_TOGGLE_BUTTON(dlg->mToolbarRadioButtonText)->active)
	tb = TEXT;
    else if (GTK_TOGGLE_BUTTON(dlg->mToolbarRadioButtonPictures)->active)
	tb = PICTURES;
    else
	tb = BOTH;

	//***1.96a - detect so we can force reloading of ID's in main window
	bool showHideChanged = 
		(dlg->mOptions->mHideIDs == GTK_TOGGLE_BUTTON(dlg->mShowIDsButton)->active);

	//***1.65 - set Show/Hide ID state
	if (GTK_TOGGLE_BUTTON(dlg->mShowIDsButton)->active)
		dlg->mOptions->mHideIDs = false;
	else
		dlg->mOptions->mHideIDs = true;

    memcpy(dlg->mOptions->mCurrentColor, dlg->mColor, 4 * sizeof(double));

    dlg->mOptions->mToolbarDisplay = tb;

    dlg->mOptions->mMaxIterations = numIterations;
    dlg->mOptions->mContinuityWeight = cont;
    dlg->mOptions->mLinearityWeight = lin;
    dlg->mOptions->mEdgeWeight = edge;

    dlg->mOptions->mHighThreshold = high;
    dlg->mOptions->mLowThreshold = low;
    dlg->mOptions->mGaussianStdDev = gauss;

	dlg->mOptions->mCurrentFontName = gtk_entry_get_text(GTK_ENTRY(dlg->mFontName)); //***1.85

	dlg->mMainWin->refreshOptions(false, showHideChanged); //***1.96a

	// NOTE: if at a later date we make the show all / show primary an Option
	// that is selectable here, then a way to refresh the database display
	// and select the new current row will have to be found (JHS)

    dlg->mMainWin->displayStatusMessage(_("Options changed."));

    delete dlg;
}

//**************************************************************

void on_optionsButtonCancel_clicked(GtkButton * button, gpointer userData)
{
    OptionsDialog *dlg = (OptionsDialog *) userData;

    if (NULL == dlg)
	return;

    dlg->mMainWin->displayStatusMessage(_("Options change canceled."));

    delete dlg;
}

//**************************************************************

gboolean on_colorSelectionDialog_delete_event(GtkWidget * widget,
					      GdkEvent * event,
					      gpointer userData)
{
    OptionsDialog *dlg = (OptionsDialog *) userData;

    if (NULL == dlg)
	return FALSE;

    gtk_widget_destroy(dlg->mColorSelectionDialog);
    dlg->mColorSelectionDialog = NULL;

    return TRUE;
}

//**************************************************************

void on_colorSelectOKButton_clicked(GtkButton * button, gpointer userData)
{
    OptionsDialog *dlg = (OptionsDialog *) userData;

    if (NULL == dlg)
	return;

    gtk_color_selection_get_color(
		    GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dlg->mColorSelectionDialog)->colorsel),
		    dlg->mColor);

    gtk_widget_destroy(dlg->mColorSelectionDialog);
    dlg->mColorSelectionDialog = NULL;
}

//**************************************************************

void on_colorSelectCancelButton_clicked(GtkButton * button,
					gpointer userData)
{
    OptionsDialog *dlg = (OptionsDialog *) userData;

    if (NULL == dlg)
	return;

    gtk_widget_destroy(dlg->mColorSelectionDialog);
    dlg->mColorSelectionDialog = NULL;
}

//**************************************************************

void on_colorSelectHelpButton_clicked(GtkButton * button,
				      gpointer userData)
{
#ifdef DEBUG
    cout << "WARNING: no help here." << endl;
#endif
}

//**************************************************************

void on_fontSelectButton_clicked( //***1.85
				GtkButton *button,
				gpointer userData)
{
	OptionsDialog *dlg = (OptionsDialog *)userData;

	dlg->mFontSelectionDialog = gtk_font_selection_dialog_new("Select a font ...");
			
	gtk_font_selection_dialog_set_font_name(
		GTK_FONT_SELECTION_DIALOG(dlg->mFontSelectionDialog),
		dlg->mOptions->mCurrentFontName.c_str());

	gtk_widget_show(GTK_WIDGET(dlg->mFontSelectionDialog));

    gtk_signal_connect(
			GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(dlg->mFontSelectionDialog)->ok_button), 
			"clicked",
			GTK_SIGNAL_FUNC(on_fontSelectDialog_OK_Button_clicked),
			(void *) userData);

    gtk_signal_connect(
			GTK_OBJECT(GTK_FONT_SELECTION_DIALOG(dlg->mFontSelectionDialog)->cancel_button), 
			"clicked",
			GTK_SIGNAL_FUNC(on_fontSelectDialog_CNX_Button_clicked),
			(void *) userData);

}

//**************************************************************

void on_fontSelectDialog_OK_Button_clicked( //***1.85
				GtkButton *button,
				gpointer userData)
{
	OptionsDialog *dlg = (OptionsDialog *)userData;

	string fontName = gtk_font_selection_dialog_get_font_name(
			GTK_FONT_SELECTION_DIALOG(dlg->mFontSelectionDialog));

	gtk_widget_destroy(dlg->mFontSelectionDialog);

	gtk_entry_set_text(GTK_ENTRY(dlg->mFontName), fontName.c_str());

	gtk_widget_modify_font(
			dlg->mFontSample,
			pango_font_description_from_string(fontName.c_str()));
	gtk_entry_set_text(GTK_ENTRY(dlg->mFontSample), "ABCLMNOP abclmnop 012345");
}

//**************************************************************

void on_fontSelectDialog_CNX_Button_clicked( //***1.85
				GtkButton *button,
				gpointer userData)
{
	OptionsDialog *dlg = (OptionsDialog *)userData;

	gtk_widget_destroy(dlg->mFontSelectionDialog);
}

