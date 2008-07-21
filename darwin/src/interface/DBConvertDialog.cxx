//*******************************************************************
//   file: DBConvertDialog.cxx
//
// author: J H Stewman (7/10/2008)
//
// This dialog enables conversion of an old Version 3 database to 
// an SQLite database.  Options include full backup, copy of old
// database as "*.olddb" and / or retreat without change.
//
//*******************************************************************

#pragma warning(disable:4786) // removes debug warnings
#include <vector>

#include "MainWindow.h"
#include "DBConvertDialog.h"
#include "OpenFileChooserDialog.h"

static int gNumReferences = 0;

using namespace std;

//*******************************************************************

int getNumDBConvertDialogReferences()
{
	return gNumReferences;
}

//*******************************************************************

DBConvertDialog::DBConvertDialog(
	MainWindow *m,
	string oldDBFilename,
	Database **dbPtr)
:
	mMainWin(m),
	mDBFilename(oldDBFilename),
	mOpType(toSQLite),
	mDBptr(dbPtr)
{
	mWindow = createDBConvertDialog();
}

//*******************************************************************

DBConvertDialog::~DBConvertDialog()
{
	gtk_widget_destroy(mWindow);
}

//*******************************************************************
	
void DBConvertDialog::run_and_respond()
{
	gint response = gtk_dialog_run (GTK_DIALOG (this->mWindow));

	switch (response)
	{
	case GTK_RESPONSE_CANCEL :
		on_dbConvert_CNX_button_clicked(NULL, this);
		break;
	case GTK_RESPONSE_ACCEPT :
		on_dbConvert_OK_button_clicked(NULL, this);
		break;
	default :
		g_print("Nada\n");
		// no action required 
	}
}


//*******************************************************************

bool DBConvertDialog::convert2MySQL(int degreeOfBackup)
{
	// just a stub for now

	// backup(db, degreeOfBackup); ?????

	return false;
}

//*******************************************************************

bool DBConvertDialog::convert2SQLite(int degreeOfBackup)
{
	Options o;
	o.mDarwinHome = gOptions->mDarwinHome; //getenv("DARWINHOME"); //SAH--2008-07-18

	// COPY database file by copying *.db to *.olddb
	// this is a minimal backup AND is required so OldDatabase and new
	// can be opened at the same time for conversion

	string oldDbFilename = mDBFilename;
	oldDbFilename = oldDbFilename.substr(0,oldDbFilename.rfind('.')) + ".olddb";
	string command = "move /Y "; // overwrite w/o prompt
	command = command 
		+ "\""
		+ mDBFilename
		+ "\" \""
		+ oldDbFilename
		+ "\"";
	system(command.c_str());

	OldDatabase *db = NULL;
	
	o.mDatabaseFileName = oldDbFilename;

	CatalogScheme dummyCat; // empty

	db = new OldDatabase(&o, dummyCat, false);

	if (degreeOfBackup == 2)
		backupCatalog(db);

	*mDBptr = duplicateDatabase(&o, db, mDBFilename);

	delete db;

	if (degreeOfBackup == 0)
	{
		// remove the old db copy
		command = "erase /F ";
		command = command 
			+ "\""
			+ oldDbFilename
			+ "\"";
		system(command.c_str());
	}

	return true;
}


//*******************************************************************

GtkWidget* DBConvertDialog::createDBConvertDialog()
{
	// lots of stuff here, based on the MatchingDialog code

	GtkWidget
		*dbConvertDialog,
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
		*actionCheckButton,
		*radioButtonBox,
		*radioButton;

	GSList 
		*radio_group;

	vector<GtkWidget*>
		mCategoryButton;

	int actionID;

	GtkWidget *win = NULL;
	if (NULL != mMainWin)
		win = mMainWin->getWindow();
	dbConvertDialog = gtk_dialog_new_with_buttons (
				"Conversion of Database ...",
				GTK_WINDOW(win),
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
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dbConvertDialog)->vbox), hpaned, TRUE, TRUE, 5);

	// create stuff for the action area -- will contain buttons to ...
	// 1 - Perform Full Backup
	// 2 - Backup Old Database file ONLY
	dialog_action_area1 = GTK_DIALOG (dbConvertDialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (dbConvertDialog), "dialog_action_area1", dialog_action_area1);
	gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 6);

	// vertically paned container on left
	paneLeft = gtk_vpaned_new();
	gtk_widget_show(paneLeft);
	gtk_paned_add1(GTK_PANED(hpaned), paneLeft);

	// a framed, vertically organized box in the TOP of the paneLeft container.
	// this is where the action fields and associated buttons go
	frame = gtk_frame_new(_("Preliminary Actions:"));
	gtk_widget_show(frame);
	gtk_paned_add1 (GTK_PANED (paneLeft), frame);
	frameVbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(frameVbox);
	gtk_container_add (GTK_CONTAINER (frame), frameVbox);

   	// a horizontally arranged box for the actual Action checkboxes
	hpanedTop = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hpanedTop);
	gtk_box_pack_start(GTK_BOX(frameVbox), hpanedTop, TRUE, TRUE, 6);

	// a framed, horizontally arranged box in the BOTTOM of the paneLeft container.
	// this is where the selection of conversion Method goes
	frame = gtk_frame_new(_("Conversion To Perform:"));
	gtk_widget_show(frame);
	gtk_paned_add2 (GTK_PANED (paneLeft), frame);
	hpanedBottom = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hpanedBottom);
	gtk_container_add (GTK_CONTAINER (frame), hpanedBottom);

	// the right pane -- it contains ...
	// 1 - nothing now, maybe later ...
	paneRight = gtk_vbox_new(FALSE,0);
	gtk_paned_add2(GTK_PANED(hpaned), paneRight);

	// fill in the actions related selections in the TOP pane
			
	// create a new vbox for the next column of buttons
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(hpanedTop), vbox);

	// create a button for SAVE *.db file
	actionID = dbBackup; // 0
	mActionButton.push_back(gtk_check_button_new_with_label(_("Backup Catalog Database file")));
	gtk_box_pack_start(GTK_BOX(vbox), mActionButton[actionID], FALSE, TRUE, 5);
	gtk_widget_show(mActionButton[actionID]);

	// set button as active
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mActionButton[actionID]),TRUE);
	mActionToUse.push_back(TRUE);
		
	gtk_signal_connect (GTK_OBJECT(mActionButton[actionID]),"toggled",
		GTK_SIGNAL_FUNC (on_mActionButton_toggled),
		(void *) this);

	// create a button for SAVE images
	actionID = imgBackup; // 1
	mActionButton.push_back(gtk_check_button_new_with_label(_("Backup All Catalog Images")));
	gtk_box_pack_start(GTK_BOX(vbox), mActionButton[actionID], FALSE, TRUE, 5);
	gtk_widget_show(mActionButton[actionID]);

	// set button as active
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mActionButton[actionID]),TRUE);
	mActionToUse.push_back(TRUE); // do not save images by default
		
	gtk_signal_connect (GTK_OBJECT(mActionButton[actionID]),"toggled",
		GTK_SIGNAL_FUNC (on_mActionButton_toggled),
		(void *) this);
		
	// fill in the type of conversion in the BOTTOM pane

	radioButtonBox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(radioButtonBox);
	gtk_container_add(GTK_CONTAINER(hpanedBottom), radioButtonBox);

	// button for CONVERT to SQLite file
	radioButton = gtk_radio_button_new_with_label(NULL,_("Convert to SQLite database."));
	gtk_widget_show(radioButton);
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 5);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (radioButton), TRUE);

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
	                    GTK_SIGNAL_FUNC (on_radioConvert2SQLite_toggled),
	                    (void *) this);

	// button for CONVERT to MySQL database
	radioButton = gtk_radio_button_new_with_label(radio_group,_("Convert to MySQL database."));
	gtk_widget_show(radioButton);
	gtk_box_pack_start(GTK_BOX(radioButtonBox), radioButton, FALSE,TRUE, 5);
	radio_group = gtk_radio_button_group(GTK_RADIO_BUTTON(radioButton));

	gtk_widget_set_sensitive(radioButton, FALSE);

	gtk_signal_connect (GTK_OBJECT(radioButton),"toggled",
		                GTK_SIGNAL_FUNC (on_radioConvert2MySQL_toggled),
                        (void *) this);

	gtk_widget_show(hpaned);

	return dbConvertDialog;
}

//*******************************************************************
void on_mActionButton_toggled(
	GtkButton *button,
	gpointer userData)
{
	DBConvertDialog *dlg = (DBConvertDialog *) userData;

	if (NULL == dlg)
		return;

	// some ONE data field button was toggled, so make sure all flags 
	// are consistent with current check button states

	for (int actionID=0; actionID < dlg->mActionButton.size(); actionID++)
	{
		bool checked = gtk_toggle_button_get_active(
			                    GTK_TOGGLE_BUTTON(dlg->mActionButton[actionID]));
		
		if (! (checked == dlg->mActionToUse[actionID]))
		{
			dlg->mActionToUse[actionID] = checked;
		}

#ifdef DEBUG
		g_print("%d ",dlg->mActionToUse[actionID]);
#endif
	}
}


//*******************************************************************

void on_radioConvert2SQLite_toggled(
				GtkButton *button,
				gpointer userData)
{
	DBConvertDialog *dlg = (DBConvertDialog *) userData;

	if (NULL == dlg)
		return;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
		dlg->mOpType = DBConvertDialog::toSQLite;
}

//*******************************************************************

void on_radioConvert2MySQL_toggled(
				GtkButton *button,
				gpointer userData)
{
	DBConvertDialog *dlg = (DBConvertDialog *) userData;

	if (NULL == dlg)
		return;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
		dlg->mOpType = DBConvertDialog::toMySQL;
}

//*******************************************************************

void on_dbConvert_CNX_button_clicked(
				GtkButton *button,
				gpointer userData)
{
	delete (DBConvertDialog *) userData;
}
//*******************************************************************

void on_dbConvert_OK_button_clicked(
				GtkButton *button,
				gpointer userData)
{
	DBConvertDialog *dlg = (DBConvertDialog *) userData;

	if (NULL == dlg)
		return;

	int degree = 
		(dlg->mActionToUse[DBConvertDialog::imgBackup]) // everything
		? 2 
		: ((dlg->mActionToUse[DBConvertDialog::dbBackup]) // just copy *.db
			? 1
			: 0); // nothing
		
	cout << "Operation:" << endl;
	if (dlg->mOpType == DBConvertDialog::toSQLite)
		dlg->convert2SQLite(degree);
	else if (dlg->mOpType == DBConvertDialog::toMySQL)
		dlg->convert2MySQL(degree);
	else
		cout << "ERROR:" << endl;

	delete dlg;
}


