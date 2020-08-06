//*******************************************************************
//   file: DataExportDialog.cxx
//
// author: J H Stewman (5/15/2007)
//
// This dialog enables selective export of data from the database
// in <tab> separated spreadsheet format to file or possibly
// to a printer for printing.
//
//*******************************************************************

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <vector>

#include "MainWindow.h"
#include "DataExportDialog.h"
#include "OpenFileChooserDialog.h"

static int gNumReferences = 0;

using namespace std;

void on_mDataFieldButton_toggled(
				GtkButton *button,
				gpointer userData);

void on_dataFieldCheckButtonAll_clicked(
			GtkButton *button,
			gpointer userData);

void on_dataFieldCheckButtonClear_clicked(
			GtkButton *button,
			gpointer userData);

void on_dataFieldOrigImgButton_toggled(
			GtkButton *button,
			gpointer userData);

void on_dataFieldModImgButton_toggled(
			GtkButton *button,
			gpointer userData);

void on_radioOutToFile_toggled(
			GtkButton *button,
			gpointer userData);

void on_radioOutToPrinter_toggled(
			GtkButton *button,
			gpointer userData);

void on_data_export_CNX_button_clicked(
			GtkButton *button,
			gpointer userData);

void on_data_export_OK_button_clicked(
			GtkButton *button,
			gpointer userData);

//*******************************************************************

int getNumDataExportDialogReferences()
{
	return gNumReferences;
}

//*******************************************************************

DataExportDialog::DataExportDialog(
	Database *db,
	MainWindow *m,
	Options *o,
	DataExportDialog::operationType opType)
:
	mDatabase(db),
	mMainWin(m),
	mOptions(o),
	mOpType(opType),
	mDataFieldsSelected(0)
{
	mWindow = createDataExportDialog();
}

//*******************************************************************

DataExportDialog::~DataExportDialog()
{
	gtk_widget_destroy(mWindow);
}

//*******************************************************************
	
void DataExportDialog::run_and_respond()
{
	// lots of stuff here

	//gtk_widget_show(mWindow);

	gint response = gtk_dialog_run (GTK_DIALOG (this->mWindow));

	switch (response)
	{
	case GTK_RESPONSE_CANCEL :
		on_data_export_CNX_button_clicked(NULL, this);
		break;
	case GTK_RESPONSE_ACCEPT :
		on_data_export_OK_button_clicked(NULL, this);
		break;
	default :
		g_print("Nada\n");
		// no other action required ??
	}
}

//*******************************************************************

bool DataExportDialog::saveData()
{
	mMainWin->mExportToFilename = "";

	OpenFileChooserDialog *open = new OpenFileChooserDialog(
		mDatabase,
		mMainWin,
		mOptions,
		OpenFileChooserDialog::exportDataFields);

	open->run_and_respond();

	// at this point the main window export filename is set to the location
	// to write the file
	//
	// it is UGLY, but will work given the way the OpenFileChooser class is currently
	// written - JHS

	int count;
	ofstream outfile;
	
	if ("" == mMainWin->mExportToFilename)
	{
		// user did not specify filename or cancelled save, fail quietly
		return false;
	}

	outfile.open(mMainWin->mExportToFilename.c_str());

	if (outfile.fail())
	{
		cout << "Failed to create data field export file ...\n  ";
		cout << mMainWin->mExportToFilename << endl;
		return false;
	}

	cout << "To FILE" << endl;

	cout << "Saving " << mDataFieldsSelected << " fields ..." << endl;

	count = 0;
	int num = mDataFieldToUse.size();
	for (int i = 0; i < num; i++)
	{
		if (this->mDataFieldToUse[i])
		{
			cout << "  " << mDataFieldName[i] << endl;
			outfile << mDataFieldName[i];
			count++;
			if (count < mDataFieldsSelected)
				outfile << "\t";
		}
	}
	outfile << endl;

	for (int j = 0; j < this->mDatabase->size(); j++)
	{
		string str;

		ImageFile<ColorImage> img;

		DatabaseFin<ColorImage> *fin = mDatabase->getItem(j);

		count = 0;

		for (int k = 0; k < num; k++)
			if (mDataFieldToUse[k])
			{
				switch (k) // eventually this will tie into user configurable fields
				{
				case 0 :
					// should be ID Code
					str = fin->getID();
					break;
				case 1 :
					// should be Name
					str = fin->getName();
					break;
				case 2 :
					// should be Date
					str = fin->getDate();
					break;
				case 3 :
					// should be Roll & Frame
					str = fin->getRoll();
					break;
				case 4 :
					// should be Location
					str = fin->getLocation();
					break;
				case 5 :
					// should be Damage Category
					str = fin->getDamage();
					// UGLY hack for existing situation where label is "Unspecified"
					// but valu passed around in program and in database is "NONE"
					if (str == "NONE")
						str = "Unspecified";
					break;
				case 6 :
					// should be comment
					str = fin->getShortDescription();
					break;
				case 7 :
					// should be MODIFIED image name

					// in versions 1.75 and earlier the fin->mImageFilename is the
					// ORIGINAL image name and there MAY BE a modified image file
					// with form *_withDarwinMods.PPM ... However, these PPM files
					// mostly exist in the tracedFins folder and did NOT become
					// part of the catalog

					str = fin->mImageFilename;
					str = str.substr(str.rfind(PATH_SLASH)+1);
					break;			
				case 8 :
					// should be ORIGINAL image name
					if ("" != fin->mOriginalImageFilename)
					{
						str = fin->mOriginalImageFilename;
					}
					else 
					{
						string ext = fin->mImageFilename.substr(fin->mImageFilename.length() - 3);
						ext[0] = tolower(ext[0]);
						ext[1] = tolower(ext[1]);
						ext[2] = tolower(ext[2]);

						if (ext == "png")
						{
							if (img.loadPNGcommentsOnly(fin->mImageFilename))
							{
								// messy, but the original image filname must be extracted 
								// from the actual PNG modified image file
								str = img.mOriginalImageFilename;
							}
							else
								str = "None_Found";
						}
						else if (ext == "ppm")
						{
							// the original image has the same root name
							str = "None_Found";
						}
						else
						{
							// no PNG or PPM image file found, so the original image name 
							// is the actual mImageFilename NO modified image actually exists
							str = fin->mImageFilename;
							str = str.substr(str.rfind(PATH_SLASH)+1);
						}
					}
					break;			
				}

				if (str == "NONE")
					str = "";

				cout << str;
				outfile << str;

				count++;

				if (count < mDataFieldsSelected)
				{
					cout << "\t";
					outfile << "\t";
				}
			}

		cout << endl;
		outfile << endl;

		delete fin;
	}

	outfile.close();

	return true;
}

//*******************************************************************
	
bool DataExportDialog::printData()
{
	// lots of stuff here

	cout << "To PRINTER" << endl;

	return false;
}

//*******************************************************************

GtkWidget* DataExportDialog::createDataExportDialog()
{
	// lots of stuff here, based on the MatchingDialog code

	GtkWidget
		*dataExDialog,
		*hpaned,
		*dialog_action_area1,
		*paneLeft,
		*frame,
		*hpanedTop,
		*frameVbox,
		*paneRight,
		*hpanedBottom,
		*vbox,
		*hbox,
		*buttonBox,
		*dataFieldCheckButton,
		*radioButtonBox,
		*radioButton;

	GSList 
		*radio_group;

	vector<GtkWidget*>
		mCategoryButton;

	dataExDialog = gtk_dialog_new_with_buttons (
				"Exporting Selected Data ...",
				GTK_WINDOW(mMainWin->getWindow()),
				(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
				GTK_STOCK_OK,
				GTK_RESPONSE_ACCEPT,
				GTK_STOCK_CANCEL,
				GTK_RESPONSE_CANCEL,
				NULL);

	//----------------------------------------
	// the dialog has two predefined sections 
	// the top is a vbox
	// the bottom is an action area 

	// create horizontally paned container and place it in the vbox of the dialog
	hpaned = gtk_hpaned_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dataExDialog)->vbox), hpaned, TRUE, TRUE, 5);

	// create stuff for the action area -- will contain buttons to ...
	// 1 - initiate, pause, contine, cancel
	// 2 - return to trace window ???
	dialog_action_area1 = GTK_DIALOG (dataExDialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (dataExDialog), "dialog_action_area1", dialog_action_area1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 6);

	// vertically paned container on left
	paneLeft = gtk_vpaned_new();
	gtk_widget_show(paneLeft);
	gtk_paned_add1(GTK_PANED(hpaned), paneLeft);

	// a framed, vertically organized box in the TOP of the paneLeft container.
	// this is where the data fields and associated buttons go
	frame = gtk_frame_new(_("Export ONLY Selected Data Fields:"));
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
	// this is where the selection of Data  Export Method goes
	frame = gtk_frame_new(_("Select a Data Export Method:"));
	gtk_widget_show(frame);
	gtk_paned_add2 (GTK_PANED (paneLeft), frame);
	hpanedBottom = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hpanedBottom);
	gtk_container_add (GTK_CONTAINER (frame), hpanedBottom);

	// the right pane -- it contains ...
	// 1 - nothing now, maybe later ...
	paneRight = gtk_vbox_new(FALSE,0);
	gtk_paned_add2(GTK_PANED(hpaned), paneRight);

	// fill in the dta field related selections in the TOP pane

	// NOTE: These fields are currently based on the Eckerd College database

	// set up data fields and buttons in 4 columns
	int columnHeight = 1 + (mOptions->mDataFieldNamesMax / 2);
	int fieldID;
	for (fieldID=0; fieldID < mOptions->mDataFieldNamesMax; fieldID++)
	{
		if (fieldID % columnHeight == 0)
		{
			// create a new vbox for the next column of buttons
			vbox = gtk_vbox_new(FALSE, 0);
			gtk_widget_show(vbox);
			gtk_container_add(GTK_CONTAINER(hpanedTop), vbox);
		}
		// create a button for the next category
		if ("NONE" == mOptions->mDataFieldName[fieldID])
		{
			mDataFieldButton.push_back(gtk_check_button_new_with_label(_("Unspecified")));
			mDataFieldName.push_back(_("Unspecified"));
		}
		else
		{
			mDataFieldButton.push_back(gtk_check_button_new_with_label(
				_(mOptions->mDataFieldName[fieldID].c_str())));
			mDataFieldName.push_back(_(mOptions->mDataFieldName[fieldID].c_str()));
		}
		gtk_container_add(GTK_CONTAINER(vbox), mDataFieldButton[fieldID]);
		gtk_widget_show(mDataFieldButton[fieldID]);
		// set all buttons as active
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mDataFieldButton[fieldID]),TRUE);
		mDataFieldToUse.push_back(TRUE);
		mDataFieldsSelected++;
		
		gtk_signal_connect (GTK_OBJECT(mDataFieldButton[fieldID]),"toggled",
			GTK_SIGNAL_FUNC (on_mDataFieldButton_toggled),
			(void *) this);
	}

	// now create buttons for image filenames

	// create a new vbox for the next column of buttons
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(hpanedTop), vbox);

	// create a button for the MODIFIED image filename
	fieldID = mDataFieldButton.size();
	mDataFieldButton.push_back(gtk_check_button_new_with_label(_("Modified Image Name")));
	mDataFieldName.push_back(_("Modified Image Name"));
	gtk_container_add(GTK_CONTAINER(vbox), mDataFieldButton[fieldID]);
	gtk_widget_show(mDataFieldButton[fieldID]);

	// set button as active
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mDataFieldButton[fieldID]),TRUE);
	mDataFieldToUse.push_back(TRUE);
	mDataFieldsSelected++;
		
	gtk_signal_connect (GTK_OBJECT(mDataFieldButton[fieldID]),"toggled",
		GTK_SIGNAL_FUNC (on_mDataFieldButton_toggled),
		(void *) this);

	// create a button for the ORIGINAL image filename
	fieldID++;
	mDataFieldButton.push_back(gtk_check_button_new_with_label(_("Original Image Name")));
	mDataFieldName.push_back(_("Original Image Name"));
	gtk_container_add(GTK_CONTAINER(vbox), mDataFieldButton[fieldID]);
	gtk_widget_show(mDataFieldButton[fieldID]);

	// set button as active
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mDataFieldButton[fieldID]),TRUE);
	mDataFieldToUse.push_back(TRUE);
	mDataFieldsSelected++;
		
	gtk_signal_connect (GTK_OBJECT(mDataFieldButton[fieldID]),"toggled",
		GTK_SIGNAL_FUNC (on_mDataFieldButton_toggled),
		(void *) this);

	// drop down and put ALL or NONE buttons below category check boxes
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(frameVbox), hbox, TRUE, TRUE, 6);

	buttonBox = gtk_hbutton_box_new ();
	gtk_widget_show (buttonBox);
	gtk_box_pack_start (GTK_BOX (hbox), buttonBox, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonBox), GTK_BUTTONBOX_SPREAD);

	// ALL or NONE
	dataFieldCheckButton = gtk_button_new_with_label(_("Select All"));
	gtk_container_add(GTK_CONTAINER(buttonBox), dataFieldCheckButton);
	gtk_widget_show(dataFieldCheckButton);

	gtk_signal_connect (GTK_OBJECT(dataFieldCheckButton),"clicked",
			GTK_SIGNAL_FUNC (on_dataFieldCheckButtonAll_clicked),
			(void *) this);

	dataFieldCheckButton = gtk_button_new_with_label(_("Clear All"));
	gtk_container_add(GTK_CONTAINER(buttonBox), dataFieldCheckButton);
	gtk_widget_show(dataFieldCheckButton);
		
	// fill in the type of output selection in the BOTTOM pane

	radioButtonBox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(radioButtonBox);
	gtk_container_add(GTK_CONTAINER(hpanedBottom), radioButtonBox);

	// button for output to <tab> separated file
	radioButton = gtk_radio_button_new_with_label(NULL,_("Create <tab> separated file."));
	gtk_widget_show(radioButton);
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 5);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (radioButton), TRUE);

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                    GTK_SIGNAL_FUNC (on_radioOutToFile_toggled),
	                    (void *) this);

	// button for output to printer
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Send to printer."));
	gtk_widget_show(radioButton);
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 5);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_widget_set_sensitive(radioButton, FALSE);

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
		                GTK_SIGNAL_FUNC (on_radioOutToPrinter_toggled),
                        (void *) this);

	gtk_widget_show(hpaned);

	gtk_signal_connect (GTK_OBJECT(dataFieldCheckButton),"clicked",
			GTK_SIGNAL_FUNC (on_dataFieldCheckButtonClear_clicked),
			(void *) this);

	return dataExDialog;
}

//*******************************************************************
void on_mDataFieldButton_toggled(
	GtkButton *button,
	gpointer userData)
{
	DataExportDialog *dlg = (DataExportDialog *) userData;

	if (NULL == dlg)
		return;

	// some ONE data field button was toggled, so make sure all flags 
	// are consistent with current check button states

	for (int fieldID=0; fieldID < dlg->mDataFieldButton.size(); fieldID++)
	{
		bool checked = gtk_toggle_button_get_active(
			                    GTK_TOGGLE_BUTTON(dlg->mDataFieldButton[fieldID]));
		
		if (! (checked == dlg->mDataFieldToUse[fieldID]))
		{
			dlg->mDataFieldToUse[fieldID] = checked;
			dlg->mDataFieldsSelected = (checked) 
				? dlg->mDataFieldsSelected + 1 
				: dlg->mDataFieldsSelected - 1;
		}

#ifdef DEBUG
		g_print("%d ",dlg->mDataFieldToUse[fieldID]);
#endif
	}
}

//*******************************************************************

void on_dataFieldCheckButtonAll_clicked(
				GtkButton *button,
				gpointer userData)
{
	DataExportDialog *dlg = (DataExportDialog *) userData;

	if (NULL == dlg)
		return;

	// make sure all flags and check buttons are TRUE (checked)

	for (int fieldID=0; fieldID < dlg->mDataFieldButton.size(); fieldID++)
		if (! dlg->mDataFieldToUse[fieldID])
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->mDataFieldButton[fieldID]),TRUE);

}

//*******************************************************************

void on_dataFieldCheckButtonClear_clicked(
				GtkButton *button,
				gpointer userData)
{
	DataExportDialog *dlg = (DataExportDialog *) userData;

	if (NULL == dlg)
		return;

	// make sure all flags and check buttons are FASLE (unchecked)

	for (int fieldID=0; fieldID < dlg->mDataFieldButton.size(); fieldID++)
		if (dlg->mDataFieldToUse[fieldID])
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dlg->mDataFieldButton[fieldID]),FALSE);

}

//*******************************************************************

void on_dataFieldOrigImgButton_toggled(
				GtkButton *button,
				gpointer userData)
{
	DataExportDialog *dlg = (DataExportDialog *) userData;

	if (NULL == dlg)
		return;

	// modified image filename button is the LAST button

	int buttonIndex = dlg->mDataFieldToUse.size() - 1;

	bool checked = gtk_toggle_button_get_active(
			                    GTK_TOGGLE_BUTTON(dlg->mDataFieldButton[buttonIndex]));

	if (checked != dlg->mDataFieldToUse[buttonIndex])
	{
		dlg->mDataFieldToUse[buttonIndex] = checked;
			
		dlg->mDataFieldsSelected = (checked) 
				? dlg->mDataFieldsSelected + 1 
				: dlg->mDataFieldsSelected - 1;
	}
}

//*******************************************************************

void on_dataFieldModImgButton_toggled(
				GtkButton *button,
				gpointer userData)
{
	DataExportDialog *dlg = (DataExportDialog *) userData;

	if (NULL == dlg)
		return;

	// modified image filename button is the NEXT to LAST button

	int buttonIndex = dlg->mDataFieldToUse.size() - 2;

	bool checked = gtk_toggle_button_get_active(
			                    GTK_TOGGLE_BUTTON(dlg->mDataFieldButton[buttonIndex]));

	if (checked != dlg->mDataFieldToUse[buttonIndex])
	{
		dlg->mDataFieldToUse[buttonIndex] = checked;
			
		dlg->mDataFieldsSelected = (checked) 
				? dlg->mDataFieldsSelected + 1 
				: dlg->mDataFieldsSelected - 1;
	}
}

//*******************************************************************

void on_radioOutToFile_toggled(
				GtkButton *button,
				gpointer userData)
{
	DataExportDialog *dlg = (DataExportDialog *) userData;

	if (NULL == dlg)
		return;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
		dlg->mOpType = DataExportDialog::saveToFile;
}

//*******************************************************************

void on_radioOutToPrinter_toggled(
				GtkButton *button,
				gpointer userData)
{
	DataExportDialog *dlg = (DataExportDialog *) userData;

	if (NULL == dlg)
		return;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
		dlg->mOpType = DataExportDialog::sendToPrinter;
}

//*******************************************************************

void on_data_export_CNX_button_clicked(
				GtkButton *button,
				gpointer userData)
{
	delete (DataExportDialog *) userData;
}
//*******************************************************************

void on_data_export_OK_button_clicked(
				GtkButton *button,
				gpointer userData)
{
	DataExportDialog *dlg = (DataExportDialog *) userData;

	if (NULL == dlg)
		return;

	cout << "Operation:" << endl;
	if (dlg->mOpType == DataExportDialog::sendToPrinter)
		dlg->printData();
	else if (dlg->mOpType == DataExportDialog::saveToFile)
		dlg->saveData();
	else
		cout << "ERROR:" << endl;

	delete dlg;
}


