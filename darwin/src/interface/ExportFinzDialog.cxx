//
//   file: ExportFinzDialog.cxx
//
// author: J H Stewman
//
//   date: 7/22/2008
//
// This class provides the user interface for the export of multiple
// *.FinZ files.  The output format may either be a single FinZ file,
// a collection of several FinZ files or an actual Catalog which is a
// SUBSET of the currently open DARWIN catalog.
//

#include "../thumbnail.h"
#include "ExportFinzDialog.h"

#include "../../pixmaps/add_database.xpm"
#include "../../pixmaps/exit.xpm"
#include "../../pixmaps/fin.xpm"
#include "SaveFileChooserDialog.h"

static const char *NONE_SUBSTITUTE_STRING = _("(Not Entered)");

static anchorRow = -1;

using namespace std;


//-------------- support function for determining selections ------------

set<int> selectedRows(GtkCList *clist)
{	
	set<int> rows;

	GList *p = clist->selection;
	while (NULL != p)
	{
		cout << "Selected item: " << p->data  << endl;
		rows.insert((int)p->data);
		p = p->next;
	}

	return rows;
}

//----------------------- the member functions -------------------------

//*******************************************************************
ExportFinzDialog::ExportFinzDialog(Database *db, GtkWidget *parent)
:	mDialog(NULL),
	mDatabase(db),
	mParentWindow(parent),
	mOldSort(DB_SORT_ID),
	mNewSort(DB_SORT_ID),
	mShowAlternates(false) // for now

{
	mDBCurEntry.clear();
	mDialog = createDialog();
}

//*******************************************************************
ExportFinzDialog::~ExportFinzDialog()
{
	if (NULL != mDialog)
		gtk_widget_destroy(mDialog);

}

//*******************************************************************
void ExportFinzDialog::show()
{
	refreshDatabaseDisplayNew(true);

	gtk_widget_show(mDialog);
}

//*******************************************************************
//***1.85 - uses the existing CList entries rather than reloading the 
//          entire database file
//
void ExportFinzDialog::refreshDatabaseDisplayNew(bool sizeChanged)
{
	if (NULL == mDatabase)
		return;

	try {
		unsigned numEntries = mDatabase->size();

		//***1.85 - set font as currently selected 

		gtk_widget_modify_font(
			mCList,
			(pango_font_description_from_string(gOptions->mCurrentFontName.c_str())));

		if (sizeChanged) // true if fin was added to or deleted from database
		{
		
			// Some variables for the pixmap display
			GdkPixmap *pixmap = NULL;
			GdkBitmap *mask = NULL;

			gtk_clist_clear(GTK_CLIST(mCList)); // clear and then rebuild from database

			mRow2Id.clear(); //***1.95
			mId2Row.clear(); //***1.95

			mDBCurEntry.clear();

			unsigned row(0); //***1.95 - for position in CList (no longer same as i)

			for (unsigned i = 0; i < numEntries; i++) {

				if (0 == i % 10)
					cout << ".";

				DatabaseFin<ColorImage> *fin = mDatabase->getItem(i);
				create_gdk_pixmap_from_data(
						mCList,
						&pixmap,
						&mask,
						fin->mThumbnailPixmap);

				mId2Row.push_back(-1); //***1.95 - default value (maybe -1 is better?)

				//***1.95 - restrict list now
				if ((! fin->mIsAlternate) || ((fin->mIsAlternate) && mShowAlternates))
				{

				mRow2Id.push_back(i); //***1.95 - save id that goes with row
				mId2Row[i] = row; //***1.95

				// make a copy of the thumbnail to store as data within the GTK pixmap)
				char **thumbCopy = copy_thumbnail(fin->mThumbnailPixmap);

				//***1.85 - attach thumbnail copy to drawable
				gdk_drawable_set_data(GDK_DRAWABLE(pixmap),"thumb",thumbCopy,free_thumbnail);
	
				gchar *idCode, *name, *damage, *date, *location;

				if (gOptions->mHideIDs) //***1.65 - hide IDs if needed
				{
					idCode = new gchar[5];
					strcpy(idCode, "****");
				}
				else if ("NONE" == fin->mIDCode)
					idCode = NULL;
				else 
				{
					idCode = new gchar[fin->mIDCode.length() + 1];
					strcpy(idCode, fin->mIDCode.c_str());
				}

				if ("NONE" == fin->mName)
					name = NULL;
				else {
					name = new gchar[fin->mName.length() + 1];
					strcpy(name, fin->mName.c_str());
				}
			
				//***055DB - NONE is a valid damage category now but appears in
				// interface as "Unspecified"
				if ("NONE" == fin->mDamageCategory) {
					damage = new gchar[12];
					strcpy(damage, "Unspecified");
				} else {
					damage = new gchar[fin->mDamageCategory.length() + 1];
					strcpy(damage, fin->mDamageCategory.c_str());
				}

				if ("NONE" == fin->mDateOfSighting)
					date = NULL;
				else {
					date = new gchar[fin->mDateOfSighting.length() + 1];
					strcpy(date, fin->mDateOfSighting.c_str());
				}
		
				if ("NONE" == fin->mLocationCode)
					location = NULL;
				else {
					location = new gchar[fin->mLocationCode.length() + 1];
					strcpy(location, fin->mLocationCode.c_str());
				}

				gchar *itemInfo[6] = {
					NULL,
					idCode,
					name,
					damage,
					date,
					location
				};

				gtk_clist_append(GTK_CLIST(mCList), itemInfo);

				if (NULL != pixmap)
					gtk_clist_set_pixmap(
						GTK_CLIST(mCList),
						//i,
						row++, //***1.95 - use and increment
						0,
						pixmap,
						mask);

				delete[] idCode;
				delete[] name;
				delete[] damage;
				delete[] date;
				delete[] location;

				} //***1.95 - end of restriction on list

				delete fin;

				if (NULL != pixmap)
					gdk_pixmap_unref(pixmap);
	
				if (NULL != mask)
					gdk_bitmap_unref(mask);

			}
			cout << "!" << endl;

			//mDBCurEntry.insert(0); // do NOT select an initial entry
		}
		else // no change in size of database, so use existing CList entries
		{
			//***1.96a - if we have already dealt with the re-sorting of the list
			// do NOT do it again
			if (mOldSort == mNewSort) {
				mDBCurEntry.clear(); //SAH -- clear this list. Otherwise, old selections will be reselected event if user has unselected them.
				return;
			}

			//***1.95 - copy for now - we will update as we process to get new translations
			vector<int> 
				newRow2Id(mRow2Id), 
				newId2Row(mId2Row);

			set<int>
				newDBCurEntry;

			unsigned i;

			// build set of selected row numbers from the CList
			mDBCurEntry = selectedRows(GTK_CLIST(mCList));

			// ARRAYS for temporary storage of line info for new CList

			GdkPixmap **pixmap = new GdkPixmap* [numEntries];
			GdkBitmap **mask = new GdkBitmap* [numEntries];
			
			gchar** *itemInfo = new gchar** [numEntries];
			for (i = 0; i < numEntries; i++)
				itemInfo[i] = new gchar* [6];

			// do NOT clear existing CList until entries extracted

			// this loop needs to go through the whole DB like the one above
			// so that non-clist items can be skipped and counters updated correctly

			int id = 0; //***1.95 - fin position in new clist
			for (i = 0; i < numEntries; i++) // for each fin position in database
			{
				// get item(i) from new sort list 
				string entry = mDatabase->getItemEntryFromList(mNewSort,i);

				// find the entry in the old sort list having the same offset
				// NOTE: the called function grabs the offset from the entry passed to it
				int pos = mDatabase->getItemListPosFromOffset(mOldSort, entry);
				
				int row = mId2Row[pos]; //***1.95 -- the entry to be moved is on this row in clist

				if (row == -1)
				{	// then this is a fin that was not in the clist (an alternate view)
					// so skip over it
					continue;
				}

				newId2Row[i] = id; //***1.95
				newRow2Id[id] = i; //***1.95
				
				if (mDBCurEntry.find(row) != mDBCurEntry.end()) // selected in OLD SORT
					newDBCurEntry.insert(i);                    // so select in NEW SORT

				// grab a copy of the data from line(row) of the Clist

				// NOTE: all use if index i in rest of this loop body is replaced by id

				itemInfo[id][0] = NULL;

				for (int k = 1; k < 6; k++)
				{
					gchar *lineItem = NULL;
					gtk_clist_get_text(GTK_CLIST(mCList), row, k, &lineItem); //***1.95 - pos becomes row
				
					if (lineItem == NULL)
						itemInfo[id][k] = NULL;
					else
					{
						itemInfo[id][k] = new gchar [strlen(lineItem) + 1];
						strcpy(itemInfo[id][k],lineItem);
					}
				}

				gtk_clist_get_pixmap(
					GTK_CLIST(mCList),
					row, //***1.95 - pos becomes row
					0,
					&(pixmap[id]),
					&(mask[id]));

				char **thumbnail = (char **) gdk_drawable_get_data(GDK_DRAWABLE(pixmap[id]),"thumb");
				create_gdk_pixmap_from_data(
					mCList,
					&(pixmap[id]),
					&(mask[id]),
					thumbnail);

				// make a copy of the thumbnail to store as data within the GTK pixmap
				char **thumbCopy = copy_thumbnail(thumbnail);

				//***1.85 - attach thumbnail copy to drawable
				gdk_drawable_set_data(GDK_DRAWABLE(pixmap[id]),"thumb",thumbCopy,free_thumbnail);

				id++; //***1.95 - increment position in new clist
			}

			gtk_clist_clear(GTK_CLIST(mCList)); // this wipes out data in list

			for (i = 0; i < /*numEntries*/mRow2Id.size(); i++) //***1.95
			{
				// append the new data for the redisplay of the CList
				gtk_clist_append(GTK_CLIST(mCList), itemInfo[i]);

				if (NULL != pixmap[i])
					gtk_clist_set_pixmap(
						GTK_CLIST(mCList),
						i,
						0,
						pixmap[i],
						mask[i]);

				for (int k = 0; k < 6; k++)
					delete [] itemInfo[i][k];

				if (NULL != pixmap[i])
					gdk_pixmap_unref(pixmap[i]);
	
				if (NULL != mask[i])
					gdk_bitmap_unref(mask[i]);

			}
			
			/* debug code
			for (int r = 0; r < mId2Row.size(); r++)
			{
				if (r < mRow2Id.size())
					g_print("%2d %2d %2d %2d %2d\n",r,mId2Row[r],newId2Row[r],mRow2Id[r],newRow2Id[r]);
				else
					g_print("%2d %2d %2d\n",r,mId2Row[r],newId2Row[r]);
			}
			*/

			mRow2Id = newRow2Id; //***1.95
			mId2Row = newId2Row; //***1.95
			mDBCurEntry = newDBCurEntry; 

			for (i = 0; i < numEntries; i++)
				delete [] itemInfo[i];
			delete [] itemInfo; 

			delete [] pixmap;
			delete [] mask;
		}

	} catch (Error e) {
		showError(_("The database seems to be corrupted.\n"
			  "Some (or all) entries may not appear\n"
			  "correctly."));
	}
}

//*******************************************************************
//
// Function simply adjusts the scrolling CList so that the selected fin
// is visible.  Size of the CList is indicated by the size of the
// size of the mRow2Id vector, rather than the size of the database.
// The two sizes are equal only if ALL images (primary and alternate
// are being displayed.
//
void ExportFinzDialog::selectFromCList(int newCurEntry)       //***004CL
{
	//*** 1.7CL - all that follows

	const int lineHeight = DATABASEFIN_THUMB_HEIGHT + 1;
	static int lastEntry = 0;

	int pageEntries = mScrollable->allocation.height / lineHeight;

	if ((int)mRow2Id.size() == 0) //***1.96
		return;

	// else force scrollable window to scroll down to clist entry

	GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(mScrollable));
	int topEntry = (int)(adj->value) / lineHeight;
		
	//g_print("top/last/this = (%d %d %d)\n", topEntry, lastEntry,newCurEntry);

	if ((newCurEntry > topEntry + pageEntries - 3) || (newCurEntry < topEntry))
	{
		// NOTE: the (-2) in the test above prevents highlighting of partially visible
		// item at bottom of page when pressing NEXT button

		if ((lastEntry + 1 == newCurEntry) && 
			(newCurEntry > topEntry) && (newCurEntry == topEntry + pageEntries - 2))
			adj->value += lineHeight; // just scroll down one line
		else
		{
			// reposition list so newCurEntry is at top
			float where = (double)newCurEntry / (int)mRow2Id.size(); //***1.96
			adj->value = where * (adj->upper - adj->lower);
			topEntry = newCurEntry;
		}

		// this should be called but seems to be meaningless and scroll update
		// works without it - JHS
		//gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(mScrollable),adj);

		gtk_adjustment_value_changed(adj);
	}
	lastEntry = newCurEntry;
}

//*******************************************************************
void ExportFinzDialog::selectFromReorderedCList(std::string filename){  //***004CL
	if (NULL == mDatabase)
		return;
	try {
		unsigned numEntries = mDatabase->size();
                unsigned i = 0;
                bool found = false;
		while ((i < numEntries) && (!found)) {
			DatabaseFin<ColorImage> *fin = mDatabase->getItem(mRow2Id[i]); //***1.95
			if (fin->mImageFilename == filename){
				found = true;
				gtk_clist_select_row(GTK_CLIST(mCList), i, 0); //***1.7
			}
			delete fin; //***1.0LK - this fin is only used to find position in clist
			i++;
		}
	} catch (Error e) {
		showError(_("The database seems to be corrupted.\n"
			  "Some (or all) entries may not appear\n"
			  "correctly."));
	}

}

//*******************************************************************
//***1.85 - new function
//
void ExportFinzDialog::selectFromReorderedCListNew(std::string selectedIdPlusOffset){  //***004CL
	if (NULL == mDatabase)
		return;
	try {
		string offset = selectedIdPlusOffset.substr(1 + selectedIdPlusOffset.rfind(" "));
		int pos = mDatabase->getItemListPosFromOffset(mNewSort,offset);

		pos = mId2Row[pos]; //***1.95 - must map to actual Clist from master database list posit

		if (pos != NOT_IN_LIST)
			gtk_clist_select_row(GTK_CLIST(mCList), pos, 0);

	} catch (Error e) {
		showError(_("The database seems to be corrupted.\n"
			  "Some (or all) entries may not appear\n"
			  "correctly."));
	}

}
//*******************************************************************
void ExportFinzDialog::displayStatusMessage(const string &msg)
{
	gtk_statusbar_pop(GTK_STATUSBAR(mStatusBar), mContextID);
	gtk_statusbar_push(GTK_STATUSBAR(mStatusBar), mContextID, msg.c_str());
}

//------------------------- createDialog -------------------------------

GtkWidget* ExportFinzDialog::createDialog()
{
	GtkWidget *mainWindow;
	GtkWidget *mainVBox;
	GtkWidget *mainHPaned;
	GtkWidget *mainLeftVBox;
	GtkWidget *mainScrolledWindow;
	GtkWidget *mainCListLabelNull;
	GtkWidget *mainCListLabelID;
	GtkWidget *mainCListLabelName;
	GtkWidget *mainCListLabelDamage;
	GtkWidget *mainCListLabelDate;
	GtkWidget *mainCListLabelLocation;
	GtkWidget *hbuttonbox1;
	GtkWidget *mainInfoTable;
	GtkWidget *mainRightVBox;
	GtkTooltips *tooltips;
	GtkWidget *tmpLabel, *tmpIcon, *tmpBox;
	GtkWidget *leftFrame, *rightFrame;

	string MainWinTitle = "DARWIN - Export FINZ";
	//***1.85 - set the database filename / message for the window
	switch (mDatabase->status())
	{
	case Database::loaded :
		MainWinTitle += " (from " + mDatabase->getFilename() + ")";
		break;
	default :
		MainWinTitle += " (unknown error with database file)";
		break;
	}

	tooltips = gtk_tooltips_new ();


	mDialog = gtk_dialog_new ();
	gtk_object_set_data (GTK_OBJECT(mDialog), "exportDialog", mDialog);
	gtk_window_set_title (GTK_WINDOW (mDialog), _(MainWinTitle.c_str()));
	GTK_WINDOW (mDialog)->type = WINDOW_DIALOG;
	gtk_window_set_position (GTK_WINDOW (mDialog), GTK_WIN_POS_CENTER);
	gtk_window_set_policy (GTK_WINDOW (mDialog), TRUE, TRUE, TRUE);
	gtk_window_set_wmclass(GTK_WINDOW(mDialog), "darwin_export_finz", "DARWIN");
	gtk_window_set_default_size(GTK_WINDOW(mDialog), 500, 600);

	gtk_window_set_modal(GTK_WINDOW(mDialog), TRUE); //***1.3
	gtk_window_set_transient_for(GTK_WINDOW(mDialog), GTK_WINDOW(mParentWindow));

	mainVBox = GTK_DIALOG(mDialog)->vbox;

	// set up rest of main window

	mainHPaned = gtk_hpaned_new();
	gtk_widget_show(mainHPaned);
	gtk_box_pack_start(GTK_BOX(mainVBox), mainHPaned, TRUE, TRUE, 0);

	leftFrame = gtk_frame_new(NULL);
	gtk_widget_show(leftFrame);
	gtk_frame_set_shadow_type(GTK_FRAME(leftFrame), GTK_SHADOW_IN);
	gtk_paned_pack1(GTK_PANED(mainHPaned), leftFrame, TRUE, TRUE);

	//***1.96 - new box for view list options
	mainLeftVBox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (mainLeftVBox);
	gtk_container_add(GTK_CONTAINER(leftFrame), mainLeftVBox);
	gtk_container_set_border_width (GTK_CONTAINER (mainLeftVBox), 4);

	mainScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (mainScrolledWindow);
	gtk_box_pack_start (GTK_BOX (mainLeftVBox), mainScrolledWindow, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mainScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	mScrollable = mainScrolledWindow; //***1.7CL

	mCList = gtk_clist_new (6);
	gtk_clist_set_selection_mode(GTK_CLIST(mCList), GTK_SELECTION_MULTIPLE);

	gtk_clist_set_row_height(GTK_CLIST(mCList), DATABASEFIN_THUMB_HEIGHT);
	gtk_clist_column_title_passive(GTK_CLIST(mCList), 0);
	gtk_widget_show (mCList);
	gtk_container_add (GTK_CONTAINER (mainScrolledWindow), mCList);

	for (int i = 0; i < 7; i++)
		gtk_clist_set_column_auto_resize(GTK_CLIST(mCList), i, TRUE);

	gtk_clist_column_titles_show (GTK_CLIST (mCList));

	mainCListLabelNull = gtk_label_new("");
	gtk_widget_show(mainCListLabelNull);
	gtk_clist_set_column_widget (GTK_CLIST (mCList), 0, mainCListLabelNull);

	mainCListLabelID = gtk_label_new(_("ID Code"));
	gtk_widget_show(mainCListLabelID);
	gtk_clist_set_column_widget(GTK_CLIST (mCList), 1, mainCListLabelID);

	mainCListLabelName = gtk_label_new(_("Name"));
	gtk_widget_show(mainCListLabelName);
	gtk_clist_set_column_widget(GTK_CLIST(mCList), 2, mainCListLabelName);

	mainCListLabelDamage = gtk_label_new (_("Damage"));
	gtk_widget_show (mainCListLabelDamage);
	gtk_clist_set_column_widget (GTK_CLIST (mCList), 3, mainCListLabelDamage);

	mainCListLabelDate = gtk_label_new (_("Date"));
	gtk_widget_show (mainCListLabelDate);
	gtk_clist_set_column_widget (GTK_CLIST (mCList), 4, mainCListLabelDate);

	mainCListLabelLocation = gtk_label_new (_("Location"));
	gtk_widget_show (mainCListLabelLocation);
	gtk_clist_set_column_widget (GTK_CLIST (mCList), 5, mainCListLabelLocation);

	//***1.85 - new FindDolphin by ID tool
	//***1.95 - moved from above Clist to below
	GtkWidget *tempHbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start (GTK_BOX (mainLeftVBox), tempHbox, FALSE, FALSE, 5);
	gtk_widget_show(tempHbox);

	GtkWidget *findLabel = gtk_label_new("Find Dolphin by ID:");
	gtk_box_pack_start (GTK_BOX (tempHbox), findLabel, FALSE, FALSE, 0);
	gtk_widget_show(findLabel);

	GtkWidget *findEntry = gtk_entry_new();
	gtk_box_pack_start (GTK_BOX (tempHbox), findEntry, FALSE, FALSE, 0);
	gtk_widget_show(findEntry);

	mSearchID = findEntry;

	GtkWidget *findNow = gtk_button_new_with_label("Goto");
	gtk_box_pack_start (GTK_BOX (tempHbox), findNow, FALSE, FALSE, 0);
	gtk_widget_show(findNow);

	// create button box with "SaveFin(s)", "SaveAsCatalog" & "Cancel" buttons

	hbuttonbox1 = gtk_hbutton_box_new();
	gtk_widget_show(hbuttonbox1);
	gtk_box_pack_start(GTK_BOX(mainLeftVBox), hbuttonbox1, FALSE, TRUE, 5);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox1), GTK_BUTTONBOX_SPREAD);

	// create Save button

	mButtonSave = gtk_button_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, fin_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("_Save Fin(s)"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), mButtonSave);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);
  
	gtk_container_add(GTK_CONTAINER(mButtonSave), tmpBox);

	gtk_widget_show(mButtonSave);
	gtk_container_add(GTK_CONTAINER(hbuttonbox1), mButtonSave);
	GTK_WIDGET_SET_FLAGS(mButtonSave, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip(tooltips, mButtonSave, 
	                     _("Save selected fin(s) as *.finz files."), NULL);

	// create SaveAsCatalog button

	mButtonSaveAsCatalog = gtk_button_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, add_database_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("_Save as Catalog"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), mButtonSaveAsCatalog);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);
  
	gtk_container_add(GTK_CONTAINER(mButtonSaveAsCatalog), tmpBox);

	// for now, this option does not exist -- JHS
	//gtk_widget_show(mButtonSaveAsCatalog);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), mButtonSaveAsCatalog);
	GTK_WIDGET_SET_FLAGS (mButtonSaveAsCatalog, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip (tooltips, mButtonSaveAsCatalog, _("Save selected fin(s) as a new catalog."), NULL);

	// create Cancel button

	mButtonCancel = gtk_button_new();

	tmpBox = gtk_hbox_new(FALSE, 0);			
	tmpIcon = create_pixmap_from_data(tmpBox, exit_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("_Cancel"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), mButtonCancel);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(mButtonCancel), tmpBox);

	gtk_widget_show(mButtonCancel);
	//***1.99 - this button goes with the data in the new notebook page below
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), mButtonCancel);
	GTK_WIDGET_SET_FLAGS (mButtonCancel, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip (tooltips, mButtonCancel, _("Cancel without saving any FinZ."), NULL);


	//***1.99 - the right frame will now contain a TABBED area for the
	// modified image, original image, data fields, outline, ...

	// for now this area is NOT USED - JHS

	rightFrame = gtk_frame_new(NULL);
	gtk_widget_show(rightFrame);
	gtk_frame_set_shadow_type(GTK_FRAME(rightFrame), GTK_SHADOW_IN);
	gtk_paned_pack2(GTK_PANED(mainHPaned), rightFrame, TRUE, TRUE);
  
	mainRightVBox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (mainRightVBox);
	gtk_container_add(GTK_CONTAINER(rightFrame), mainRightVBox);


	// FINALLY, at bottom of main window is a status bar
	
	mStatusBar = gtk_statusbar_new ();
	gtk_widget_show(mStatusBar);

	mContextID = gtk_statusbar_get_context_id(GTK_STATUSBAR(mStatusBar), "mainWin");

	gtk_box_pack_start(GTK_BOX(mainVBox), mStatusBar, FALSE, FALSE, 0);


	gtk_signal_connect (GTK_OBJECT (mCList), "click_column",
	                    GTK_SIGNAL_FUNC (on_finzCList_click_column),
	                    (void *) this);
/*
 *** These functions cause issues with proper selection and unselection
     of ranges and multiple selections so we use ONLY the default callbacks
	 and access the clist->selection list directly as needed upon invocation
	 of a save operation - JHS

  gtk_signal_connect (GTK_OBJECT (mCList), "select_row",
	                    GTK_SIGNAL_FUNC (on_finzCList_select_row),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mCList), "button_release_event",
	                    GTK_SIGNAL_FUNC (on_finzCList_button_release_event),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mCList), "start_selection",
	                    GTK_SIGNAL_FUNC (on_finzCList_select_first_row),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mCList), "end_selection",
	                    GTK_SIGNAL_FUNC (on_finzCList_select_last_row),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mCList), "unselect_row",
	                    GTK_SIGNAL_FUNC (on_finzCList_unselect_row),
	                    (void *) this);
*/
	gtk_signal_connect (GTK_OBJECT (findNow), "clicked",
	                    GTK_SIGNAL_FUNC (on_finzButtonFindNow_clicked),
	                    (void *) this);

	gtk_signal_connect (GTK_OBJECT (mDialog), "delete_event",
	                    GTK_SIGNAL_FUNC (on_finzDialog_delete_event),
	                    (void *) this);

	gtk_signal_connect (GTK_OBJECT (mButtonSave), "clicked",
	                    GTK_SIGNAL_FUNC (on_finzDialogButtonSaveFinz_clicked),
	                    (void *) this);

	gtk_signal_connect (GTK_OBJECT (mButtonSaveAsCatalog), "clicked",
	                    GTK_SIGNAL_FUNC (on_finzDialogButtonSaveAsCatalog_clicked),
	                    (void *) this);

	gtk_signal_connect (GTK_OBJECT (mButtonCancel), "clicked",	//***002DB
	                    GTK_SIGNAL_FUNC (on_finzDialogButtonCancel_clicked),
	                    (void *) this);

	gtk_widget_grab_default (mButtonCancel);
	gtk_object_set_data (GTK_OBJECT (mDialog), "tooltips", tooltips);

	return mDialog;
}

//------------------------- the friend functions -----------------------
//*******************************************************************
//***1.85 - new
//
void on_finzButtonFindNow_clicked(
	GtkButton *button,
	gpointer userData)
{
	ExportFinzDialog *dlg = (ExportFinzDialog *) userData;

	string idString = gtk_entry_get_text(GTK_ENTRY(dlg->mSearchID));

	// find in the ID list
	int posit = dlg->mDatabase->getIDListPosit(idString);

	if (NOT_IN_LIST == posit)
		return;

	// grab the ID : Offset pair out of the ID list
	string idPlusOffset = dlg->mDatabase->getItemEntryFromList(DB_SORT_ID, posit);

	// find where the offset is in the CURRENT sort of the CCList
	posit = dlg->mDatabase->getItemListPosFromOffset(
		dlg->mNewSort,
		idPlusOffset);

	if (NOT_IN_LIST == posit)
		return;

	// and finally go there
	gtk_clist_select_row(GTK_CLIST(dlg->mCList), posit, 0);
}

//*******************************************************************
void on_finzCList_click_column(
	GtkCList *clist,
	gint column,
	gpointer userData)
{
	ExportFinzDialog *dlg = (ExportFinzDialog *) userData;

	if (NULL == dlg)
		return;

	if (NULL == dlg->mDatabase)
		return;
	
	if (0 == dlg->mDatabase->size()) //***1.85 - don't sort if empty
		return;

	switch (column) {
		case 1:
			dlg->mDatabase->sort(DB_SORT_ID);
			dlg->mNewSort = DB_SORT_ID; //***1.85
			break;
		case 2:
			dlg->mDatabase->sort(DB_SORT_NAME);
			dlg->mNewSort = DB_SORT_NAME; //***1.8
			break;
		case 3:
			dlg->mDatabase->sort(DB_SORT_DAMAGE);
			dlg->mNewSort = DB_SORT_DAMAGE; //***1.8
			break;
		case 4:
			dlg->mDatabase->sort(DB_SORT_DATE);
			dlg->mNewSort = DB_SORT_DATE; //***1.8
			break;
		case 5:
			dlg->mDatabase->sort(DB_SORT_LOCATION);
			dlg->mNewSort = DB_SORT_LOCATION; //***1.8
			break;
	}
	
	gtk_clist_freeze(GTK_CLIST(dlg->mCList)); //***1.85

	dlg->refreshDatabaseDisplayNew(false); //***1.85 - just a sort, no change in # of fins

	// make sure CList repositions itself for previously selected item to appear
	set<int>::iterator it;
	for (it = dlg->mDBCurEntry.begin(); it != dlg->mDBCurEntry.end(); it++)
	{
		//cout << "Selected row " << *it << endl;
		gtk_clist_select_row(GTK_CLIST(dlg->mCList), *it, 0);
	}

	dlg->mOldSort = dlg->mNewSort; //***1.96a - moved from above

	gtk_clist_thaw(GTK_CLIST(dlg->mCList)); //***1.85
}

//*******************************************************************
// This is NOT USED 
//
void on_finzCList_select_row(
	GtkCList *clist,
	gint row,
	gint column,
	GdkEvent *event,
	gpointer userData)
{
	ExportFinzDialog *dlg = (ExportFinzDialog *) userData;

	if (NULL == dlg)
		return;

	cout << "in Select Row" << endl;

	// this function ONLY seems to be called when a CList row is selected
	// and it has NOT been previously selected - IE, reselects and unselects
	// do NOT end up here.  SHIFT selects do not end up here either
	try {

		if (NULL == event)
		{
			// must be here from using arrow keys not mouse, so no event
			cout << "focus row " << row << endl;
			return;
		}

		switch (event->type)
		{
		case GDK_BUTTON_RELEASE :
			if (((GdkEventButton*)event)->state == GDK_CONTROL_MASK)
			{
				// we add this line to set of selected lines
				cout << "CTRL key pressed - ";
			}
			else if (((GdkEventButton*)event)->state == GDK_SHIFT_MASK)
			{
				cout << "SHIFT key pressed - ";
			}
			break;
		default :
			break;
		}

		cout << "ROW Selected: " << row << endl;

		// if we select a row that is already selected then unselect it
		// else add it to selected row set
		if (dlg->mDBCurEntry.find(row) != dlg->mDBCurEntry.end())
			dlg->mDBCurEntry.erase(row);
		else
			dlg->mDBCurEntry.insert(row);

	} catch (Error e) {
		showError(e.errorString());
	}
}

//*******************************************************************
//
void on_finzDialog_delete_event(
		GtkWidget *widget,
		GdkEvent *event,
		gpointer userData)
{
	// essentially the same as a CANCEL button click

	ExportFinzDialog *dlg = (ExportFinzDialog *) userData;

	if (NULL == dlg)
		return;

	delete dlg;
}
//*******************************************************************
// NOT used
void on_finzCList_select_first_row(
		GtkCList *clist,
		gpointer userData)
{
	cout << "Found the first row?" << endl;
}

//*******************************************************************
// NOT used
void on_finzCList_select_last_row(
		GtkCList *clist,
		gpointer userData)
{
	cout << "Found the last row?" << endl;
}

//*******************************************************************
// NOT used
void on_finzCList_unselect_row(
		GtkCList *clist,
		gint row,
		gint column,
		GdkEvent *event,
		gpointer userData)
{
	cout << "ROW Unselected: " << row << endl;
}

//*******************************************************************
// NOT used - was an experiment
gboolean on_finzCList_button_release_event(
		GtkCList *clist,
		GdkEvent *event,
		gpointer userData)
{
	// this captures ALL button releases on the GtkCList

	ExportFinzDialog *dlg = (ExportFinzDialog*) userData;

	cout << "Button Released on CList!" << endl;

	gdouble 
		x, y;

	x = ((GdkEventButton*)event)->x;
	y = ((GdkEventButton*)event)->y;

	gint 
		row, col;

	gtk_clist_get_selection_info(clist, x, y, &row, &col);

	cout << "Selection : " << clist->selection << endl;

	if (((GdkEventButton*)event)->state == GDK_SHIFT_MASK)
	{
		cout << "SHIFT key down, so forcing selection!" << endl;

		if (anchorRow != -1)
		{
			gtk_signal_emit_by_name(GTK_OBJECT(clist),"button_release_event", 
					event, userData);
		}
		else
		{
			anchorRow = row;
			gtk_signal_emit_by_name(GTK_OBJECT(clist),"select_row", 
					row, col, event, userData);
			//gtk_signal_emit_stop_by_name(GTK_OBJECT(clist),"button_release_event");
		}

		return FALSE;

	}

	return FALSE;
}

//*******************************************************************
//
void on_finzDialogButtonSaveFinz_clicked(
		GtkButton *button,
		gpointer userData)
{
	ExportFinzDialog *dlg = (ExportFinzDialog *) userData;

	if (NULL == dlg)
		return;

	bool saved = false;
	set<int> selectedFins = selectedRows(GTK_CLIST(dlg->mCList));

	if (selectedFins.empty()) {//idiot, selected something first for export
		return;
	} else if(selectedFins.size() == 1) {
		set<int>::iterator it = selectedFins.begin();
		DatabaseFin<ColorImage>* fin;
		int id = dlg->mRow2Id[*it];
		fin = dlg->mDatabase->getItem(id);

		SaveFileChooserDialog *fsChooserDlg = new SaveFileChooserDialog(dlg->mDatabase,
												fin,
												NULL,
												NULL,
												dlg->mOptions,
												dlg->mDialog,
												SaveFileChooserDialog::saveFin);
		saved=fsChooserDlg->run_and_respond();
		
		delete fin;

	} else {//more than one selected
		set<int>::iterator it;
		vector<DatabaseFin<ColorImage>* > fins;
		for (it = selectedFins.begin(); it != selectedFins.end(); it++) {
			int id = dlg->mRow2Id[*it];
			fins.push_back(dlg->mDatabase->getItem(id));
		}

		SaveFileChooserDialog *fsChooserDlg = new SaveFileChooserDialog(dlg->mDatabase,
												NULL,
												NULL,
												NULL,
												dlg->mOptions,
												dlg->mDialog,
												SaveFileChooserDialog::saveMultipleFinz,
												&fins);
		saved=fsChooserDlg->run_and_respond();

		vector<DatabaseFin<ColorImage>* >::iterator finsit;
		for (finsit = fins.begin(); finsit != fins.end(); finsit++) {
			delete ((DatabaseFin<ColorImage>*) *finsit);
		}
		
	}

	if (saved)
		delete dlg;

}

//*******************************************************************
//
void on_finzDialogButtonSaveAsCatalog_clicked(
		GtkButton *button,
		gpointer userData)
{
	ExportFinzDialog *dlg = (ExportFinzDialog *) userData;

	if (NULL == dlg)
		return;

	// do somthing here to make catalog creation happen

}

//*******************************************************************
//
void on_finzDialogButtonCancel_clicked(
		GtkButton *button,
		gpointer userData)
{
	ExportFinzDialog *dlg = (ExportFinzDialog *) userData;

	if (NULL == dlg)
		return;

	delete dlg;
}
