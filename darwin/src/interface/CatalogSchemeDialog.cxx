//*******************************************************************
//   file: CatalogSchemeDialog.cxx
//
// author: J H Stewman (7/11/2006)
//
//*******************************************************************

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#include "../support.h"
#include "CatalogSchemeDialog.h"
#include "ErrorDialog.h"
#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#include "../../pixmaps/ok.xpm"
#include "../../pixmaps/logo_small.xpm"

#include <cstdio>

using namespace std;

// number of currently open AboutDialog widgets
static int gNumReferences = 0; 


//*******************************************************************
//
// getNumCatalogSchemeDialogReferences()
//
//    Returns number of currently open CatalogSchemeDialog widgets.  Used by 
//    MainWindow to prevent opening more than one at a time.
//
int getNumCatalogSchemeDialogReferences()
{
	return gNumReferences;
}


//*******************************************************************
//
// CatalogSchemeDialog::CatalogSchemeDialog()
//
//    Creates the AboutDialog widget and increments reference counter.
//
CatalogSchemeDialog::CatalogSchemeDialog(GtkWidget *parentWindow, Options *o, int openMode)
: 
	mParentWindow(parentWindow), // so we can set transient_for MainWin
	mSchemeList(NULL),
	mCategoryList(NULL),
	mNewSchemeName(NULL),
	mNewCategoryNames(NULL),
	mOptions(o),
	mOpenMode(openMode),
	mEditable(false), //***1.95
	mNumberOfLocalSchemes(o->mNumberOfDefinedCatalogSchemes), //***1.95
	mLocalSchemeName(o->mDefinedCatalogSchemeName), //***1.95
	mLocalCategoryNamesMax(o->mDefinedCatalogCategoryNamesMax), //***1.95
	mLocalCategoryName(o->mDefinedCatalogCategoryName), //***1.95
	mLocalDefaultCatalogScheme(o->mCurrentDefaultCatalogScheme) //***1.95
{
	mDialog = createCatalogSchemeDialog(); // must follow setting of parent window above
	gNumReferences++;
}


//*******************************************************************
//
// CatalogSchemeDialog::~CatalogSchemeDialog()
//
//    Destroys the existing CatalogSchemeDialog widget and decrements reference 
//    counter.
//
CatalogSchemeDialog::~CatalogSchemeDialog()
{
	if (NULL != mDialog)
		gtk_widget_destroy(mDialog);
	
	gNumReferences--;
}


//*******************************************************************
//
// void CatalogSchemeDialog::show()
//
//    Shows the CatalogSchemeDialog widget.
//
void CatalogSchemeDialog::show()
{
	gtk_widget_show(mDialog);
}


//*******************************************************************
//
// GtkWidget* CatalogSchemeDialog::createCatalogSchemeDialog()
//
//    Friend function to create the GTK Widget for the AboutDialog.
//
GtkWidget* CatalogSchemeDialog::createCatalogSchemeDialog()
{
	GtkWidget *catalogSchemeDialog;
	GtkWidget *catalogSchemeHBox;
	GtkWidget *catalogSchemeVBoxMain;
	GtkWidget *catalogSchemeVBox;
	GtkWidget *catalogSchemePixmap;
	GtkWidget *catalogSchemeListLabel;
	GtkWidget *catalogSchemeList;
	GtkWidget *catalogSchemeCategoryListLabel;
	GtkWidget *catalogSchemeCategoryList;

	GtkWidget *catalogSchemeActionArea;
	GtkWidget *catalogSchemeHButtonBox;
	GtkWidget *catalogSchemeButtonOK;
	GtkWidget *tmpBox, *tmpLabel, *tmpIcon;

	int catIDnum;

	catalogSchemeDialog = gtk_dialog_new ();
	gtk_object_set_data (GTK_OBJECT(catalogSchemeDialog), "catalogSchemeDialog", catalogSchemeDialog);
	gtk_window_set_title (GTK_WINDOW (catalogSchemeDialog), _("Catalog Schemes..."));
	GTK_WINDOW (catalogSchemeDialog)->type = WINDOW_DIALOG;
	gtk_window_set_position (GTK_WINDOW (catalogSchemeDialog), GTK_WIN_POS_CENTER);
	gtk_window_set_policy (GTK_WINDOW (catalogSchemeDialog), TRUE, TRUE, TRUE);
	gtk_window_set_wmclass(GTK_WINDOW(catalogSchemeDialog), "darwin_catalogScheme", "DARWIN");

	gtk_window_set_modal(GTK_WINDOW(catalogSchemeDialog), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(catalogSchemeDialog), GTK_WINDOW(mParentWindow));

	catalogSchemeVBoxMain = GTK_DIALOG(catalogSchemeDialog)->vbox;
	gtk_object_set_data (GTK_OBJECT (catalogSchemeDialog), "catalogSchemeVBoxMain", catalogSchemeVBoxMain);
	gtk_widget_show (catalogSchemeVBoxMain);

	catalogSchemeHBox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(catalogSchemeHBox);
	gtk_box_pack_start(GTK_BOX(catalogSchemeVBoxMain), catalogSchemeHBox, TRUE, TRUE, 0);

	catalogSchemePixmap = create_pixmap_from_data(catalogSchemeDialog, logo_small_xpm);
	gtk_widget_show(catalogSchemePixmap);
	gtk_box_pack_start(GTK_BOX(catalogSchemeHBox), catalogSchemePixmap, TRUE, TRUE, 0);

	catalogSchemeVBox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(catalogSchemeVBox);
	gtk_box_pack_start(GTK_BOX(catalogSchemeHBox), catalogSchemeVBox, TRUE, TRUE, 10);

	switch (mOpenMode)
	{
	case defineNewScheme :
		catalogSchemeListLabel = gtk_label_new("New Catalog Scheme Name");
		gtk_widget_show (catalogSchemeListLabel);
		gtk_box_pack_start (GTK_BOX (catalogSchemeVBox), catalogSchemeListLabel, FALSE, FALSE, 10);

		mNewSchemeName = gtk_entry_new();
		gtk_widget_show(mNewSchemeName);
		gtk_box_pack_start(GTK_BOX(catalogSchemeVBox), mNewSchemeName, FALSE, FALSE, 0);

		gtk_entry_set_text(GTK_ENTRY (mNewSchemeName), "No Name Specified");

		catalogSchemeCategoryListLabel = gtk_label_new("New Scheme Category Names");
		gtk_widget_show (catalogSchemeCategoryListLabel);
		gtk_box_pack_start (GTK_BOX (catalogSchemeVBox), catalogSchemeCategoryListLabel, FALSE, FALSE, 10);

		GtkWidget *view;

		view = gtk_text_view_new ();

		mBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

		gtk_text_buffer_set_text (mBuffer, "No Categories", -1);
		gtk_widget_show (view);
		gtk_box_pack_start(GTK_BOX(catalogSchemeVBox), view, FALSE, FALSE, 0);

		//***1.85 - note added below text box
		catalogSchemeCategoryListLabel = gtk_label_new(
			"\"Unspecified\" is appended to list.");
		gtk_widget_show (catalogSchemeCategoryListLabel);
		gtk_box_pack_start (GTK_BOX (catalogSchemeVBox), catalogSchemeCategoryListLabel, FALSE, FALSE, 10);
		break;

	case viewExistingSchemes :

		// list the Defined Catalog Scheme Names

		catalogSchemeListLabel = gtk_label_new("Catalog Schemes");
		gtk_widget_show (catalogSchemeListLabel);
		gtk_box_pack_start (GTK_BOX (catalogSchemeVBox), catalogSchemeListLabel, FALSE, FALSE, 10);

		catalogSchemeList = gtk_clist_new(1);
		gtk_widget_show(catalogSchemeList);
		gtk_box_pack_start(GTK_BOX(catalogSchemeVBox), catalogSchemeList, FALSE, FALSE, 0);

		mSchemeList = catalogSchemeList;

		gtk_clist_freeze(GTK_CLIST (catalogSchemeList));

		for (catIDnum = 0; catIDnum < mOptions->mNumberOfDefinedCatalogSchemes; catIDnum++)
		{
			//gchar *entry[1] = {_(mOptions->mDefinedCatalogSchemeName[catIDnum].c_str())}; //***2.22 - g++4.5 complains
			gchar *entry[1] = {const_cast<char*>(_(mOptions->mDefinedCatalogSchemeName[catIDnum].c_str()))};

			gtk_clist_append(
				GTK_CLIST(catalogSchemeList),
				entry);
		}

		gtk_clist_thaw(GTK_CLIST (catalogSchemeList));

		//***1.95 - message about editablity of Schemes
		mEditMessage = gtk_label_new(_("\n\n\nClick Edit Scheme(s)\nto allow Editing!"));
		gtk_widget_show(mEditMessage);
		gtk_box_pack_start(GTK_BOX(catalogSchemeVBox), mEditMessage, FALSE, FALSE, 0);

		// List the defined category names for selected scheme

		catalogSchemeVBox = gtk_vbox_new(FALSE, 0);
		gtk_widget_show(catalogSchemeVBox);
		gtk_box_pack_start(GTK_BOX(catalogSchemeHBox), catalogSchemeVBox, TRUE, TRUE, 10);

		catalogSchemeCategoryListLabel = gtk_label_new("Category Names");
		gtk_widget_show (catalogSchemeCategoryListLabel);
		gtk_box_pack_start (GTK_BOX (catalogSchemeVBox), catalogSchemeCategoryListLabel, FALSE, FALSE, 10);

		catalogSchemeCategoryList = gtk_clist_new(1);
		gtk_widget_show(catalogSchemeCategoryList);
		gtk_box_pack_start(GTK_BOX(catalogSchemeVBox), catalogSchemeCategoryList, FALSE, FALSE, 0);

		mCategoryList = catalogSchemeCategoryList;
		break;

	case setDefaultScheme :
		catalogSchemeListLabel = gtk_label_new("Default Catalog Scheme");
		gtk_widget_show (catalogSchemeListLabel);
		gtk_box_pack_start (GTK_BOX (catalogSchemeVBox), catalogSchemeListLabel, FALSE, FALSE, 10);
			
		// create a pulldown list to set current default scheme name

		catalogSchemeList = gtk_combo_box_new_text();
		gtk_widget_show(catalogSchemeList);
		gtk_box_pack_start(GTK_BOX(catalogSchemeVBox), catalogSchemeList, FALSE,
				FALSE, 0);

		mSchemeList = catalogSchemeList;

		for (int catIDnum = 0; catIDnum < mOptions->mNumberOfDefinedCatalogSchemes; catIDnum++)
			gtk_combo_box_append_text(
					GTK_COMBO_BOX(mSchemeList),
					_(mOptions->mDefinedCatalogSchemeName[catIDnum].c_str()));

		// List the defined category names for selected scheme

		catalogSchemeVBox = gtk_vbox_new(FALSE, 0);
		gtk_widget_show(catalogSchemeVBox);
		gtk_box_pack_start(GTK_BOX(catalogSchemeHBox), catalogSchemeVBox, TRUE, TRUE, 10);

		catalogSchemeCategoryListLabel = gtk_label_new("Category Names");
		gtk_widget_show (catalogSchemeCategoryListLabel);
		gtk_box_pack_start (GTK_BOX (catalogSchemeVBox), catalogSchemeCategoryListLabel, FALSE, FALSE, 10);

		catalogSchemeCategoryList = gtk_clist_new(1);
		gtk_widget_show(catalogSchemeCategoryList);
		gtk_box_pack_start(GTK_BOX(catalogSchemeVBox), catalogSchemeCategoryList, FALSE, FALSE, 0);

		mCategoryList = catalogSchemeCategoryList;

			// create a button to cancel operation
		break;
	}

	catalogSchemeActionArea = GTK_DIALOG (catalogSchemeDialog)->action_area;
	gtk_object_set_data (GTK_OBJECT (catalogSchemeDialog), "catalogSchemeActionArea", catalogSchemeActionArea);
	gtk_widget_show (catalogSchemeActionArea);
	gtk_container_set_border_width (GTK_CONTAINER (catalogSchemeActionArea), 10);

	catalogSchemeHButtonBox = gtk_hbutton_box_new ();
	gtk_widget_show (catalogSchemeHButtonBox);
	gtk_box_pack_start (GTK_BOX (catalogSchemeActionArea), catalogSchemeHButtonBox, TRUE, TRUE, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (catalogSchemeHButtonBox), GTK_BUTTONBOX_END);

	//***1.95 - new button to allow EDITING of schemes
	GtkWidget 
		*catalogSchemeButtonEdit;
	if (viewExistingSchemes == mOpenMode)
	{
		catalogSchemeButtonEdit = gtk_button_new_with_label("Edit Scheme(s)");
		gtk_widget_show (catalogSchemeButtonEdit);
		gtk_container_add (GTK_CONTAINER (catalogSchemeHButtonBox), catalogSchemeButtonEdit);
	}

	//***1.95 - add CANCEL button
	GtkWidget *catalogSchemeButtonCancel = gtk_button_new_with_label("Cancel");
	gtk_widget_show (catalogSchemeButtonCancel);
	gtk_container_add (GTK_CONTAINER (catalogSchemeHButtonBox), catalogSchemeButtonCancel);

	// button for clicking OK
	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, ok_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new(_("OK"));
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);
  
	catalogSchemeButtonOK = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(catalogSchemeButtonOK), tmpBox);
	gtk_widget_show (catalogSchemeButtonOK);
	gtk_container_add (GTK_CONTAINER (catalogSchemeHButtonBox), catalogSchemeButtonOK);
	GTK_WIDGET_SET_FLAGS (catalogSchemeButtonOK, GTK_CAN_DEFAULT);

	gtk_signal_connect (GTK_OBJECT (catalogSchemeDialog), "delete_event",
						GTK_SIGNAL_FUNC (on_catalogSchemeDialog_delete_event),
						(void*)this);
	//***!.95 - new cancel button action same as delete dialog event
	gtk_signal_connect (GTK_OBJECT (catalogSchemeButtonCancel), "clicked",
						GTK_SIGNAL_FUNC (on_catalogSchemeButtonCancel_clicked),
						(void*)this);
	gtk_signal_connect (GTK_OBJECT (catalogSchemeButtonOK), "clicked",
						GTK_SIGNAL_FUNC (on_catalogSchemeButtonOK_clicked),
						(void*)this);

	switch (mOpenMode)
	{
	case defineNewScheme :
		break;

	case viewExistingSchemes :
		//***!.95 - new Edit button action - enables editing
		gtk_signal_connect (GTK_OBJECT (catalogSchemeButtonEdit), "clicked",
							GTK_SIGNAL_FUNC (on_catalogSchemeButtonAllowEdit_clicked),
							(void*)this);

		//***1.95 - callback for EDIT/DELETE action on selected Scheme
		gtk_clist_set_button_actions(GTK_CLIST(mSchemeList),2,GTK_BUTTON_SELECTS); // right button
		gtk_signal_connect (GTK_OBJECT (mSchemeList), "select-row",
							GTK_SIGNAL_FUNC (on_catSchemeCList_select_row),
							(void *) this);

		gtk_clist_select_row(GTK_CLIST(mSchemeList), 0, 0);
		mLocalSelectedScheme = 0; //***1.95

		//***1.95 - callback for EDIT/DELETE action on selected category name
		gtk_clist_set_button_actions(GTK_CLIST(mCategoryList),2,GTK_BUTTON_SELECTS); // right button
		gtk_signal_connect (GTK_OBJECT (mCategoryList), "select-row",
							GTK_SIGNAL_FUNC (on_catSchemeCategoryList_select_row),
							(void *) this);
		break;

	case setDefaultScheme :
		gtk_signal_connect (GTK_OBJECT (mSchemeList), "changed",
							GTK_SIGNAL_FUNC (on_catSchemeComboBox_select_row),
							(void *) this);

		gtk_combo_box_set_active(GTK_COMBO_BOX(mSchemeList), mOptions->mCurrentDefaultCatalogScheme);
		break;
	}

	gtk_widget_grab_default (catalogSchemeButtonOK);
	return catalogSchemeDialog;
}


//*******************************************************************
//
// gboolean on_catalogSchemeButtonAllowEdit_clicked()
//
//    Friend function to process "edit" events.
//
void on_catalogSchemeButtonAllowEdit_clicked(
		GtkWidget *button,
		gpointer userData)
{
	if (NULL == userData)
		return;

	CatalogSchemeDialog *dlg = (CatalogSchemeDialog*)userData;
	dlg->mEditable = true;

	gtk_widget_hide(dlg->mEditMessage);
	gtk_label_set_text(
				GTK_LABEL(dlg->mEditMessage),
				_("\nSchemes are Editable!\n\nClick Right Mouse Button\non item to Modify!"));
	gtk_widget_show(dlg->mEditMessage);
}


//*******************************************************************
//
// gboolean on_catalogSchemeButtonCancel_clicked()
//
//    Friend function to process "cancel" events.
//
gboolean on_catalogSchemeButtonCancel_clicked(
		GtkWidget *button,
		gpointer userData)
{
	if (NULL == userData)
		return FALSE;

	delete (CatalogSchemeDialog*)userData;
	
	return TRUE;
}


//*******************************************************************
//
// gboolean on_catalogSchemeDialog_delete_event()
//
//    Friend function to process "delete" events.
//
gboolean on_catalogSchemeDialog_delete_event(
		GtkWidget *widget,
		GdkEvent *event,
		gpointer userData)
{
	if (NULL == userData)
		return FALSE;

	delete (CatalogSchemeDialog*)userData;
	
	return TRUE;
}


//*******************************************************************
//
// void on_catalogSchemeButtonOK_clicked()
//
//    Friend function to process "OK button" events
//
void on_catalogSchemeButtonOK_clicked(
		GtkButton *button,
		gpointer userData)
{
	if (NULL == userData)
		return;

	CatalogSchemeDialog *dlg = (CatalogSchemeDialog *) userData;

	int n, cc;
	GtkTextIter iter1, iter2;
	string name;
	gboolean moreLines;
	gchar firstC; 

	string schemeName;
	vector<string> categoryNames;

	switch (dlg->mOpenMode)
	{
	case CatalogSchemeDialog::defineNewScheme :
		schemeName = (char *) gtk_entry_get_text(GTK_ENTRY (dlg->mNewSchemeName));
		cc = gtk_text_buffer_get_char_count(GTK_TEXT_BUFFER (dlg->mBuffer));

		if ((schemeName == "") || (cc == 0))
		{
			//***2.22 - added mDialog
			//ErrorDialog *err = new ErrorDialog(dlg->mDialog,"Scheme must have a Name\nand at least one Category.");
			//err->show();
			//***2.22 - replacing own ErrorDialog with GtkMessageDialogs
			GtkWidget *errd = gtk_message_dialog_new (GTK_WINDOW(dlg->mDialog),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "Scheme must have a Name\nand at least one Category.");
			gtk_dialog_run (GTK_DIALOG (errd));
			gtk_widget_destroy (errd);
			return; // must have a scheme name and at least one category
		}

		// build list of category names

		categoryNames.push_back("NONE"); //***1.85 - always have a NONE/Unspecified category

		gtk_text_buffer_get_start_iter(dlg->mBuffer,&iter2);
		iter1 = iter2;
		while (FALSE != gtk_text_iter_forward_to_line_end(&iter2))
		{
			// there is a category name between iter1 and iter2
			name = (char *)gtk_text_iter_get_text(&iter1,&iter2);
			g_print(name.c_str());
			if (name != "")
				categoryNames.push_back(name);

			do
			{
				moreLines = gtk_text_iter_forward_line(&iter2);
				cc = gtk_text_iter_get_chars_in_line(&iter2);
				firstC = gtk_text_iter_get_char(&iter2);
				if (firstC == 0xa || firstC == 0xd)
					cc = 0;
			}
			while ((moreLines != FALSE) && (cc == 0));

			iter1 = iter2;
		}
		name = (char *)gtk_text_iter_get_text(&iter1,&iter2);
		if (name != "")
		{
			// do not want an empty last category name
			g_print(name.c_str());
			categoryNames.push_back(name);
		}

		// the following should never occur since there were some characters in the name list

		if (categoryNames.size() == 0)
		{
			//***2.22 - added mDialog
			//ErrorDialog *err = new ErrorDialog(dlg->mDialog,"Scheme must have a Name\nand at least one Category.");
			//err->show();
			//***2.22 - replacing own ErrorDialog with GtkMessageDialogs
			GtkWidget *errd = gtk_message_dialog_new (GTK_WINDOW(dlg->mDialog),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "Scheme must have a Name\nand at least one Category.");
			gtk_dialog_run (GTK_DIALOG (errd));
			gtk_widget_destroy (errd);
			return; // must have a scheme name and at least one category
		}

		// finally, add new scheme to Options 

		n = (dlg->mOptions->mNumberOfDefinedCatalogSchemes)++;

		dlg->mOptions->mDefinedCatalogSchemeName.push_back(schemeName);

		dlg->mOptions->mDefinedCatalogCategoryNamesMax.push_back(categoryNames.size());

		dlg->mOptions->mDefinedCatalogCategoryName.push_back(categoryNames);

		break;

	case CatalogSchemeDialog::viewExistingSchemes: //***1.95
		if (dlg->mEditable)
		{
			// commit changes to local Options Catalog Schemes back to main program Options 
			// NO changes allowed to scheme[0] -- the Eckerd Scheme

			int r;
			for (r = 1; r < dlg->mNumberOfLocalSchemes; r++)
			{
				if (r < dlg->mOptions->mNumberOfDefinedCatalogSchemes) // copy OVER
				{
					// copy local scheme name to global Options
					dlg->mOptions->mDefinedCatalogSchemeName[r] = dlg->mLocalSchemeName[r];
					// copy following category names down
					dlg->mOptions->mDefinedCatalogCategoryName[r] = dlg->mLocalCategoryName[r];
					// copy following number of category names down
					dlg->mOptions->mDefinedCatalogCategoryNamesMax[r] = dlg->mLocalCategoryNamesMax[r];
				}
				else // APPEND
				{
					// append local scheme name to global Options
					dlg->mOptions->mDefinedCatalogSchemeName.push_back(dlg->mLocalSchemeName[r]);
					// append following category names down
					dlg->mOptions->mDefinedCatalogCategoryName.push_back(dlg->mLocalCategoryName[r]);
					// append following number of category names down
					dlg->mOptions->mDefinedCatalogCategoryNamesMax.push_back(dlg->mLocalCategoryNamesMax[r]);
				}
			}
			for (r = dlg->mNumberOfLocalSchemes; r < dlg->mOptions->mNumberOfDefinedCatalogSchemes; r++)
			{
				// remove schemes that are no longer defined
				dlg->mOptions->mDefinedCatalogSchemeName.pop_back();
				dlg->mOptions->mDefinedCatalogCategoryName.pop_back();
				dlg->mOptions->mDefinedCatalogCategoryNamesMax.pop_back();
			}
			dlg->mOptions->mNumberOfDefinedCatalogSchemes = dlg->mNumberOfLocalSchemes;
		
			// reset the value of the current catalog scheme
			dlg->mOptions->mCurrentDefaultCatalogScheme = dlg->mLocalDefaultCatalogScheme;
		}
		break;

	case CatalogSchemeDialog::setDefaultScheme :
		// reset the value of the current catalog scheme
		dlg->mOptions->mCurrentDefaultCatalogScheme = 
				gtk_combo_box_get_active(GTK_COMBO_BOX (dlg->mSchemeList));
		break;
	}

	delete dlg;
}

//*******************************************************************
//
// void on_catSchemeCList_select_row()
//
//    Friend function to process "scheme selection" events
//
void on_catSchemeCList_select_row(
	GtkCList *clist,
	gint row,
	gint column,
	GdkEvent *event,
	gpointer userData)
{
	CatalogSchemeDialog *catSWin = (CatalogSchemeDialog *) userData;

	if (NULL == catSWin)
		return;

	//***1.95 - distinction made between mouse buttons now

	if ((NULL == event) || (1 == ((GdkEventButton*)event)->button) ||
		(((3 == ((GdkEventButton*)event)->button)) && (catSWin->mEditable)))
	{
		catSWin->mLocalSelectedScheme = row; //***1.95
		
		// this is the left mouse button and we are selecting diferent Scheme

		gtk_clist_freeze(GTK_CLIST (catSWin->mCategoryList));

		gtk_clist_clear(GTK_CLIST (catSWin->mCategoryList));
	
		//***1.95 - new approach using LOCAL copies of Schemes

		for (int catIDnum = 0; catIDnum < catSWin->mLocalCategoryNamesMax[row]; catIDnum++)
			if ("NONE" == catSWin->mLocalCategoryName[row][catIDnum])
			{
				gchar *entry[1] = {_("Unspecified")};

				gtk_clist_append(
						GTK_CLIST(catSWin->mCategoryList),
						entry);
			}
			else
			{
				//gchar *entry[1] = {_(catSWin->mLocalCategoryName[row][catIDnum].c_str())}; //***2.22 - gtk4.5 complains
				gchar *entry[1] = {const_cast<char*>(_(catSWin->mLocalCategoryName[row][catIDnum].c_str()))};

				gtk_clist_append(
						GTK_CLIST(catSWin->mCategoryList),
						entry);
			}

		gtk_clist_thaw(GTK_CLIST (catSWin->mCategoryList));

		if ((NULL != event) && 
			(((3 == ((GdkEventButton*)event)->button)) && (catSWin->mEditable)))
		{

		if (row == 0)
		{
			GtkWidget *msgBox = gtk_message_dialog_new(
									GTK_WINDOW(catSWin->mDialog),
									GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_INFO,
									GTK_BUTTONS_CLOSE,
									"The Eckerd College Scheme cannot be \nModified or Removed!");
			gtk_dialog_run (GTK_DIALOG (msgBox));
			gtk_widget_destroy (msgBox);
			return;
		}
		// this is the right mouse button and we are allowing Scheme RENAME and DELETE
					
		gchar *itemTextPtr;
		gtk_clist_get_text(clist,row,0,&itemTextPtr);
		string itemText = itemTextPtr;
		g_print("r(%d) c(%d) item(%s)\n",row, column, itemText.c_str());

		catSWin->mSelectedRow = row;

		GtkWidget *menu = gtk_menu_new();

		GtkWidget *rename = gtk_menu_item_new_with_label("Rename Scheme");
		gtk_widget_show(rename);
		gtk_menu_append(GTK_MENU(menu),rename);
		gtk_signal_connect (GTK_OBJECT (rename), "activate",
							GTK_SIGNAL_FUNC (on_schemeRename_activate),
							(void *) userData);

		GtkWidget *remove = gtk_menu_item_new_with_label("Remove Scheme");
		gtk_widget_show(remove);
		gtk_menu_append(GTK_MENU(menu),remove);
		gtk_signal_connect (GTK_OBJECT (remove), "activate",
							GTK_SIGNAL_FUNC (on_schemeRemove_activate),
							(void *) userData);

		gtk_widget_show(menu);

		gtk_menu_popup(GTK_MENU(menu),NULL,NULL,NULL,NULL,2,gtk_get_current_event_time());
	}
	}
	else //***1.95 - keep focus on previously selected scheme
		gtk_clist_select_row(GTK_CLIST(catSWin->mSchemeList),
					catSWin->mLocalSelectedScheme,
					0);
}

//*******************************************************************
//
// void on_catSchemeCategoryList_select_row()
//
//    Friend function to process "scheme selection" events
//
void on_catSchemeCategoryList_select_row(
	GtkCList *clist,
	gint row,
	gint column, // JHS - this is always -1, assume actual column is 0
	GdkEvent *event,
	gpointer userData)
{
	CatalogSchemeDialog *catSWin = (CatalogSchemeDialog *) userData;

	// right mouse button clicked - allows edit/delete actions on list item
	if ((3 == ((GdkEventButton*)event)->button) && (catSWin->mEditable))
	{
		if (catSWin->mLocalSelectedScheme == 0)
		{
			GtkWidget *msgBox = gtk_message_dialog_new(
									GTK_WINDOW(catSWin->mDialog),
									GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_INFO,
									GTK_BUTTONS_CLOSE,
									"The Eckerd College Scheme cannot be \nModified or Removed!");
			gtk_dialog_run (GTK_DIALOG (msgBox));
			gtk_widget_destroy (msgBox);
			return;
		}

		gchar *itemTextPtr;
		gtk_clist_get_text(clist,row,0,&itemTextPtr);
		string itemText = itemTextPtr;
		g_print("r(%d) c(%d) item(%s)\n",row, column, itemText.c_str());

		catSWin->mSelectedRow = row;

		GtkWidget *menu = gtk_menu_new();

		GtkWidget *rename = gtk_menu_item_new_with_label("Rename Category");
		gtk_widget_show(rename);
		gtk_menu_append(GTK_MENU(menu),rename);
		gtk_signal_connect (GTK_OBJECT (rename), "activate",
							GTK_SIGNAL_FUNC (on_categoryRename_activate),
							(void *) userData);

		GtkWidget *insert = gtk_menu_item_new_with_label("Insert NEW Category");
		gtk_widget_show(insert);
		gtk_menu_append(GTK_MENU(menu),insert);
		gtk_signal_connect (GTK_OBJECT (insert), "activate",
							GTK_SIGNAL_FUNC (on_categoryInsert_activate),
							(void *) userData);

		GtkWidget *remove = gtk_menu_item_new_with_label("Remove Category");
		gtk_widget_show(remove);
		gtk_menu_append(GTK_MENU(menu),remove);
		gtk_signal_connect (GTK_OBJECT (remove), "activate",
							GTK_SIGNAL_FUNC (on_categoryRemove_activate),
							(void *) userData);

		gtk_widget_show(menu);

		gtk_menu_popup(GTK_MENU(menu),NULL,NULL,NULL,NULL,2,gtk_get_current_event_time());

	}
}


//*******************************************************************
//
//
void on_categoryRename_activate( //***1.95
	GtkMenuItem *item,
	gpointer userData)
{
	CatalogSchemeDialog *catSWin = (CatalogSchemeDialog *) userData;
	GtkCList *clist = GTK_CLIST(catSWin->mCategoryList);
	int row = catSWin->mSelectedRow;

	g_print("RENAME selected!\n");

	gchar *itemTextPtr;
	gtk_clist_get_text(clist,row,0,&itemTextPtr);
	string itemText = itemTextPtr;
	g_print("r(%d) item(%s)\n",row, itemText.c_str());

	catSWin->mEntryDialog = new CatEntryDialog(
					catSWin->mDialog,
					CatEntryDialog::renameCategory,
					itemText.c_str());
	string newText = catSWin->mEntryDialog->run();

	g_print("new text - %s\n",newText.c_str());

	if ((" OK:" == newText.substr(0,4)) && (newText.length() > 4))
	{
		gtk_clist_set_text(clist,row,0,newText.substr(4).c_str());
		// make sure to change local Scheme copy
		catSWin->mLocalCategoryName[catSWin->mLocalSelectedScheme][row] = newText.substr(4); 
	}
	else // we are deleting existing category name
	{
		g_print("NO new name for category!\n");
	}

	delete catSWin->mEntryDialog;
}

//*******************************************************************
//
//
void on_categoryInsert_activate( //***1.95
	GtkMenuItem *item,
	gpointer userData)
{
	CatalogSchemeDialog *catSWin = (CatalogSchemeDialog *) userData;
	GtkCList *clist = GTK_CLIST(catSWin->mCategoryList);
	int row = catSWin->mSelectedRow;

	g_print("INSERT selected!\n");

	gchar *itemTextPtr;
	gtk_clist_get_text(clist,row,0,&itemTextPtr);
	string itemText = itemTextPtr;
	g_print("r(%d) item(%s)\n",row, itemText.c_str());

	catSWin->mEntryDialog = new CatEntryDialog(
				catSWin->mDialog,
				CatEntryDialog::insertCategory,
				""); // initially empty
	string newText = catSWin->mEntryDialog->run();

	g_print("new text - %s\n",newText.c_str());

	if ((" OK:" == newText.substr(0,4)) && (newText.length() > 4))
	{
		char *temp[1];
		temp[0] = new char[newText.length()];
		strcpy(temp[0],(newText.substr(4)).c_str());
		gtk_clist_insert(clist,row,temp);
		delete [] temp[0];
		// make sure to change local Scheme copy
		vector<string>::iterator it = catSWin->mLocalCategoryName[catSWin->mLocalSelectedScheme].begin();
		for (int i = 0; i < row; i++)
			++it;
		catSWin->mLocalCategoryName[catSWin->mLocalSelectedScheme].insert(it,newText.substr(4)); 
		catSWin->mLocalCategoryNamesMax[catSWin->mLocalSelectedScheme]++;
	}
	else // we are deleting existing category name
	{
		g_print("NO name for new category!\n");
	}

	delete catSWin->mEntryDialog;
}

//*******************************************************************
//
//
void on_categoryRemove_activate( //***1.95
	GtkMenuItem *item,
	gpointer userData)
{
	CatalogSchemeDialog *catSWin = (CatalogSchemeDialog *) userData;
	GtkCList *clist = GTK_CLIST(catSWin->mCategoryList);
	int row = catSWin->mSelectedRow;

	g_print("REMOVE selected!\n");

	gchar *itemTextPtr;
	gtk_clist_get_text(clist,row,0,&itemTextPtr);
	string itemText = itemTextPtr;
	g_print("r(%d) item(%s)\n",row, itemText.c_str());

	catSWin->mEntryDialog = new CatEntryDialog(
				catSWin->mDialog,
				CatEntryDialog::removeCategory,
				itemText.c_str());
	string newText = catSWin->mEntryDialog->run();

	g_print("new text - %s\n",newText.c_str());

	if ((" OK:" == newText.substr(0,4)) && (newText.length() > 4))
	{
		gtk_clist_remove(clist,row);
		// make sure to change local Scheme copy
		vector<string>::iterator it = catSWin->mLocalCategoryName[catSWin->mLocalSelectedScheme].begin();
		for (int i = 0; i < row; i++)
			++it;
		catSWin->mLocalCategoryName[catSWin->mLocalSelectedScheme].erase(it); 
		catSWin->mLocalCategoryNamesMax[catSWin->mLocalSelectedScheme]--;
	}
	// else nothing to delete

	delete catSWin->mEntryDialog;
}


//*******************************************************************
//
//
void on_schemeRename_activate( //***1.95
	GtkMenuItem *item,
	gpointer userData)
{
	CatalogSchemeDialog *catSWin = (CatalogSchemeDialog *) userData;
	GtkCList *clist = GTK_CLIST(catSWin->mSchemeList);
	int row = catSWin->mSelectedRow;

	g_print("RENAME Scheme selected!\n");

	gchar *itemTextPtr;
	gtk_clist_get_text(clist,row,0,&itemTextPtr);
	string itemText = itemTextPtr;
	g_print("r(%d) item(%s)\n",row, itemText.c_str());

	catSWin->mEntryDialog = new CatEntryDialog(
					catSWin->mDialog,
					CatEntryDialog::renameScheme,
					itemText.c_str());
	string newText = catSWin->mEntryDialog->run();

	g_print("new text - %s\n",newText.c_str());

	if ((" OK:" == newText.substr(0,4)) && (newText.length() > 4))
	{
		gtk_clist_set_text(clist,row,0,newText.substr(4).c_str());
		// make sure to change local Scheme copy
		catSWin->mLocalSchemeName[row] = newText.substr(4); 
	}
	else // we are deleting existing scheme name
	{
		g_print("NO new name for scheme!\n");
	}

	delete catSWin->mEntryDialog;

}


//*******************************************************************
//
//
void on_schemeRemove_activate( //***1.95
	GtkMenuItem *item,
	gpointer userData)
{
	CatalogSchemeDialog *catSWin = (CatalogSchemeDialog *) userData;
	GtkCList *clist = GTK_CLIST(catSWin->mSchemeList);
	int row = catSWin->mSelectedRow;

	g_print("REMOVE Scheme selected!\n");

	gchar *itemTextPtr;
	gtk_clist_get_text(clist,row,0,&itemTextPtr);
	string itemText = itemTextPtr;
	g_print("r(%d) item(%s)\n",row, itemText.c_str());

	catSWin->mEntryDialog = new CatEntryDialog(
				catSWin->mDialog,
				CatEntryDialog::removeScheme,
				itemText.c_str());
	string newText = catSWin->mEntryDialog->run();

	g_print("new text - %s\n",newText.c_str());

	if ((" OK:" == newText.substr(0,4)) && (newText.length() > 4))
	{
		gtk_clist_remove(clist,row);
		// make sure to change local Scheme copy
		for (int r = row; r+1 < catSWin->mNumberOfLocalSchemes; r++)
		{
			// copy following scheme name down
			catSWin->mLocalSchemeName[r] = catSWin->mLocalSchemeName[r+1];
			// copy following category names down
			catSWin->mLocalCategoryName[r] = catSWin->mLocalCategoryName[r+1];
			// copy following number of category names down
			catSWin->mLocalCategoryNamesMax[r] = catSWin->mLocalCategoryNamesMax[r+1];
		}
		// decrement number of schemes & pop the last scheme name, categories, and category count
		catSWin->mNumberOfLocalSchemes--;
		catSWin->mLocalSchemeName.pop_back();
		catSWin->mLocalCategoryName[catSWin->mNumberOfLocalSchemes].pop_back();
		catSWin->mLocalCategoryNamesMax.pop_back();

		 // location of the default scheme moves down if a preceding scheme is deleted
		if (row < catSWin->mLocalDefaultCatalogScheme)
			catSWin->mLocalDefaultCatalogScheme--;

		if (row == catSWin->mNumberOfLocalSchemes)
			row--;
		gtk_clist_select_row(GTK_CLIST(catSWin->mSchemeList),row,0); // select & redisplay scheme
		
	}
	// else nothing to delete

	delete catSWin->mEntryDialog;
}


//*******************************************************************
//
// void on_catSchemeComboBox_select_row()
//
//    Friend function to process "category selection" events
//
void on_catSchemeComboBox_select_row(
	GtkComboBox *cbox,
	gpointer userData)
{
	CatalogSchemeDialog *catSWin = (CatalogSchemeDialog *) userData;

	if (NULL == catSWin)
		return;

	int row = gtk_combo_box_get_active(cbox);

	if (-1 == row)
		return;

	gtk_clist_freeze(GTK_CLIST (catSWin->mCategoryList));

	gtk_clist_clear(GTK_CLIST (catSWin->mCategoryList));

	Options *o = catSWin->mOptions;

	for (int catIDnum = 0; catIDnum < o->mDefinedCatalogCategoryNamesMax[row]; catIDnum++)
		if ("NONE" == o->mDefinedCatalogCategoryName[row][catIDnum])
		{
			gchar *entry[1] = {_("Unspecified")};

			gtk_clist_append(
				GTK_CLIST(catSWin->mCategoryList),
				entry);
		}
		else
		{
			//gchar *entry[1] = {_(o->mDefinedCatalogCategoryName[row][catIDnum].c_str())}; //***2.22 - g++4.5 complains
			gchar *entry[1] = {const_cast<char*>(_(o->mDefinedCatalogCategoryName[row][catIDnum].c_str()))};

			gtk_clist_append(
				GTK_CLIST(catSWin->mCategoryList),
				entry);
		}

	gtk_clist_thaw(GTK_CLIST (catSWin->mCategoryList));

}

//*******************************************************************
//******************* dialog for item entry/removal *****************
//*******************************************************************

//*******************************************************************
//
//
CatEntryDialog::CatEntryDialog(GtkWidget *parentWindow, int openMode, string Msg)
	: mDialog(createCatEntryDialog(parentWindow,openMode, Msg))
{
}

//*******************************************************************
//
//
CatEntryDialog::~CatEntryDialog()
{
	if (NULL != mDialog)
		gtk_widget_destroy(mDialog);
}

//*******************************************************************
//
//
string CatEntryDialog::run()
{
	gint result = gtk_dialog_run (GTK_DIALOG (mDialog));
  
	switch (result)
    {
	case GTK_RESPONSE_OK:
		// replace entry or remove it

		// need to know context -- are we removing a catalog scheme
		// or just modifying or removing a single category name

		mEntryText = " OK:";
		mEntryText += gtk_entry_get_text(GTK_ENTRY(mEntry));
		break;
	case GTK_RESPONSE_CANCEL:
	default:
		mEntryText = "CNX:";
		// nada -- since dialog cancelled
		break;
	}
	// mDialog will be detroyed in class destructor
	return mEntryText;
}

//*******************************************************************
//
//
GtkWidget* CatEntryDialog::createCatEntryDialog(GtkWidget *parent, int openMode, string text)
{
	GtkWidget *entryDialog;
	GtkWidget *entryVBox;
	GtkWidget *entryLabel;
  
	string banner, msg;
	switch (openMode)
	{
	case removeScheme:
		banner = "REMOVE Catalog Scheme ...";
		msg = "\n  To REMOVE the Existing Catalog Scheme named below ...  \n\n  Click OK!  \n";
		break;
	case renameScheme:
		banner = "RENAME Catalog Scheme ...";
		msg = "\n  MODIFY the Existing Catalog Scheme Name below and ...  \n\n  Click OK!  \n";
		break;
	case removeCategory:
		banner = "REMOVE Category ...";
		msg = "\n  To REMOVE the Existing Category Name below ...  \n\n  Click OK!  \n";
		break;
	case insertCategory:
		banner = "INSERT Category ...";
		msg = "\n  ENTER a NEW Category Name below and ...  \n\n  Click OK!  \n";
		break;
	case renameCategory:
		banner = "RENAME Category ...";
		msg = "\n  MODIFY the Existing Category Name below and ...  \n\n  Click OK!  \n";
		break;
	}

	entryDialog = gtk_dialog_new_with_buttons (
					banner.c_str(),
					GTK_WINDOW(parent),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_STOCK_CANCEL,
					GTK_RESPONSE_CANCEL,
					GTK_STOCK_OK,
					GTK_RESPONSE_OK,
					NULL);

	entryVBox = GTK_DIALOG (entryDialog)->vbox;

	entryLabel = gtk_label_new(_(msg.c_str()));
	gtk_widget_show (entryLabel);
	gtk_container_add(GTK_CONTAINER(entryVBox),entryLabel);

	mEntry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(mEntry),text.c_str());
	if ((removeCategory == openMode) || (removeScheme == openMode))
		gtk_entry_set_editable(GTK_ENTRY(mEntry), FALSE);
	gtk_widget_show(mEntry);
	gtk_container_add(GTK_CONTAINER(entryVBox),mEntry);

	gtk_widget_show (entryVBox);
	
	return entryDialog;
}
