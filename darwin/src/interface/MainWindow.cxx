//*******************************************************************
//   file: MainWindow.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (8/2/2005)
//         -- reformatting of code and addition of comment blocks
//         -- corrected mnemonic & accelerator code in main window menus
//
//*******************************************************************

#include <time.h> //***1.85
#include <set>

#include <gdk/gdkkeysyms.h>

#include <pango/pango-font.h>

#include "../support.h"
#include "MainWindow.h"

#include "ModifyDatabaseWindow.h"
#include "AboutDialog.h"
#include "CatalogSchemeDialog.h" //***1.4
#include "ContourInfoDialog.h"
#include "ImageViewDialog.h"
#include "ErrorDialog.h"
#include "MatchingQueueDialog.h"
#include "OpenFileSelectionDialog.h"
#include "OpenFileChooserDialog.h" //***1.4
#include "SaveFileChooserDialog.h" //***1.99
#include "OptionsDialog.h"
#include "CreateDatabaseDialog.h" //***1.85 
#include "DataExportDialog.h" //***1.9
#include "ExportFinzDialog.h" //***1.99
#include "../CatalogSupport.h" //***1.99

#include "../thumbnail.h" //***1.85

#include "../image_processing/ImageMod.h" //***1.8

#pragma warning (disable : 4305 4309)
#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "../../pixmaps/add_database.xpm"
#include "../../pixmaps/about_small.xpm"
#include "../../pixmaps/exit.xpm"
#include "../../pixmaps/exit_small.xpm"
#include "../../pixmaps/icon.xpm"
#include "../../pixmaps/magnify_cursor.xbm"
#include "../../pixmaps/matching_queue.xpm"
#include "../../pixmaps/matching_queue_small.xpm"
#include "../../pixmaps/next.xpm"
#include "../../pixmaps/open_image.xpm"
#include "../../pixmaps/open_image_small.xpm"
#include "../../pixmaps/open_trace.xpm"
#include "../../pixmaps/open_trace_small.xpm"
#include "../../pixmaps/options.xpm"
#include "../../pixmaps/options_small.xpm"
#include "../../pixmaps/previous.xpm"
#include "../../pixmaps/fin.xpm" //***1.5
#include "../../pixmaps/magnify.xpm" //***1.9

#include "../image_processing/transform.h"

#ifdef HAVE_CONFIG_H
   #include <config.h>
#endif

using namespace std;

static const int IMAGE_WIDTH = 400, IMAGE_HEIGHT = 300; //***1.7 - these were 300 x 250
static const int POINT_SIZE = 1;
static const char *NONE_SUBSTITUTE_STRING = _("(Not Entered)");

//*******************************************************************
MainWindow::MainWindow(Database *db, Options *o)
	: mDBCurEntry(0),
	  mSelectedFin(NULL),
	  mDatabase(db),
	  mImage(NULL),
	  mOrigImage(NULL), //***1.99
	  mImageFullsize(NULL), //***2.01
	  mOrigImageFullsize(NULL), //***2.01
	  mCList(NULL), //***1.95
	  mCurImageHeight(IMAGE_HEIGHT),
	  mCurImageWidth(IMAGE_WIDTH),
	  mCurContourHeight(IMAGE_HEIGHT),
	  mCurContourWidth(IMAGE_WIDTH),
	  mGC(NULL),
	  mCursor(NULL),
	  mOptions(o),
	  mOldSort(DB_SORT_ID), //***1.85
	  mNewSort(DB_SORT_ID),  //***1.85
	  mImportFromFilename(""), //***1.85
	  mExportToFilename(""), //***1.85
	  mShowAlternates(false), //***1.95
	  mDBCurEntryOffset(0) //***1.96a
{ 
	// do this here so the database filename can be placed on the window title
	mWindow = createMainWindow(o->mToolbarDisplay); //***1.85

	// create an emergency backup of DB file, as long as it was successfully loaded
	if (mDatabase->status() == Database::loaded)
	{
		string area = mOptions->mCurrentSurveyArea;
		area = area.substr(area.rfind(PATH_SLASH) + 1);
		string backupName = mOptions->mDarwinHome + PATH_SLASH
			+ "system" + PATH_SLASH + "lastLoadOf_" + area + "_"
			+ mOptions->mDatabaseFileName.substr(
					mOptions->mDatabaseFileName.rfind(PATH_SLASH) + 1);
#ifdef WIN32
		string command = "copy /Y /V \"" + mOptions->mDatabaseFileName
#else
		string command = "cp \"" + mOptions->mDatabaseFileName //***2.22 - for Mac
#endif
			+ "\" \"" + backupName + "\" >nul";
		system(command.c_str());
	}
}

//*******************************************************************
MainWindow::~MainWindow()
{
	if (NULL != mWindow)
		gtk_widget_destroy(mWindow);
	
	delete mImage;
	delete mOrigImage; //***1.99
	delete mImageFullsize; //***2.01
	delete mOrigImageFullsize; //***2.01
	delete mSelectedFin;
	delete mDatabase;

	if (NULL != mGC)
		gdk_gc_unref(mGC);

	if (NULL != mCursor)
		gdk_cursor_destroy(mCursor);

	gtk_main_quit();
}

//*******************************************************************

void MainWindow::setDatabasePtr(Database *db) //***1.85 - used when opening new DB
{
	mDatabase = db; // assume existing database was deleted by caller
}

//*******************************************************************
	
void MainWindow::setExportFilename(std::string filename) //***1.99
{
	mExportToFilename = filename;
}

//*******************************************************************

int MainWindow::getSelectedRow() //***1.96a - new function
{
	return mDBCurEntry;
}

//*******************************************************************
void MainWindow::show()
{
	//***1.85 - this function is now called ONLY after a database is intially loaded
	// so we reset entries, list sort order, etc. here

	mOldSort = DB_SORT_ID;
	mNewSort = DB_SORT_ID;
	mDatabase->sort(DB_SORT_ID);    //***003MR

	gtk_clist_freeze(GTK_CLIST(mCList)); //***1.85

	clearText(); //***!.85 - clear all text entry fields
	refreshOptions(true, false); // ***1.96a - this is the initial showing
	refreshDatabaseDisplayNew(true); //***1.85 - intial CList has not been built yet

	// do not show (first time) until CList is built from database file
	gtk_widget_show(mWindow); 

	updateGC();
	updateCursor();
	//refreshOptions(); 
	refreshImage();                 //***1.85
	refreshOutline();               //***1.85

	if (mDatabase->size() > 0){				//***002DB
		gtk_clist_select_row(GTK_CLIST(mCList), 0, 0);
		//selectFromCList(0); //***1.7CL
		gtk_widget_set_sensitive(mButtonModify, TRUE);  //***002DB
        }							//***002DB
	else {
		gtk_widget_set_sensitive(mButtonNext, FALSE);
		gtk_widget_set_sensitive(mButtonPrev, FALSE);
		gtk_widget_set_sensitive(mButtonModify, FALSE); //***002DB
	}

	//this->displayStatusMessage("Ready."); //***2.01
	char numEntriesStr[64];
	sprintf(numEntriesStr,"Ready: %d fins loaded",mDatabase->size());
	this->displayStatusMessage(numEntriesStr); //***2.01 - display size too
		
	gtk_clist_thaw(GTK_CLIST(mCList)); //***1.85

	//gtk_widget_show(mWindow); 

}

//*******************************************************************
//***1.85 - new function to set up after loading or importing DB
//
void MainWindow::resetTitleButtonsAndBackupOnDBLoad()
{
	// reset the database filename for the window
	string MainWinTitle = "DARWIN - ";
	switch (mDatabase->status())
	{
	case Database::loaded :
		MainWinTitle += mOptions->mDatabaseFileName;
		break;
	case Database::fileNotFound :
		MainWinTitle += "Database file not found";
		break;
	case Database::errorCreating :
		MainWinTitle += "Database file could not be created";
		break;
	case Database::errorLoading :
		MainWinTitle += "Error loading database file";
		break;
	case Database::oldDBVersion :
		MainWinTitle += "OLD database file version - not supported";
		break;
	default :
		MainWinTitle += "unknown error with database file";
		break;
	}
	gtk_window_set_title(GTK_WINDOW (mWindow), _(MainWinTitle.c_str()));

	// set sensitivity of menu items and buttons depending on success
	if (mDatabase->status() == Database::loaded)
	{
		gtk_widget_set_sensitive(mOpenImageMenuItem, TRUE);
		gtk_widget_set_sensitive(mOpenFinMenuItem, TRUE);
		gtk_widget_set_sensitive(mQueueMenuItem, TRUE);
		gtk_widget_set_sensitive(mBackupMenuItem, TRUE);
		gtk_widget_set_sensitive(mExportDBMenuItem, TRUE);
		gtk_widget_set_sensitive(mOpenImageButton, TRUE);
		gtk_widget_set_sensitive(mOpenFinButton, TRUE);
		gtk_widget_set_sensitive(mQueueButton, TRUE);
		gtk_widget_set_sensitive(mExportSubMenuItem, TRUE);
		gtk_widget_set_sensitive(mImportFinzMenuItem, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(mOpenImageMenuItem, FALSE);
		gtk_widget_set_sensitive(mOpenFinMenuItem, FALSE);
		gtk_widget_set_sensitive(mQueueMenuItem, FALSE);
		gtk_widget_set_sensitive(mBackupMenuItem, FALSE);
		gtk_widget_set_sensitive(mExportDBMenuItem, FALSE);
		gtk_widget_set_sensitive(mOpenImageButton, FALSE);
		gtk_widget_set_sensitive(mOpenFinButton, FALSE);
		gtk_widget_set_sensitive(mQueueButton, FALSE);
		gtk_widget_set_sensitive(mExportSubMenuItem, FALSE);
		gtk_widget_set_sensitive(mImportFinzMenuItem, FALSE);
	}
	
	// create an emergency backup of DB file, as long as it was successfully loaded
	if (mDatabase->status() == Database::loaded)
	{
		string area = mOptions->mCurrentSurveyArea;
		area = area.substr(area.rfind(PATH_SLASH) + 1);
		string backupName = mOptions->mDarwinHome + PATH_SLASH
			+ "system" + PATH_SLASH + "lastLoadOf_" + area + "_"
			+ mOptions->mDatabaseFileName.substr(
					mOptions->mDatabaseFileName.rfind(PATH_SLASH) + 1);
#ifdef WIN32
		string command = "copy /Y /V \"" + mOptions->mDatabaseFileName
#else
		string command = "cp \"" + mOptions->mDatabaseFileName //***2.22 - for Mac
#endif
			+ "\" \"" + backupName + "\" >nul";
		system(command.c_str());
	}
	mDBCurEntryOffset=0; //We have a new database, clear the offset to force image rebuilt, etc.//SAH-DB
}

//*******************************************************************
void MainWindow::refreshImage()
{
	if (TRUE == GDK_IS_DRAWABLE(mDrawingAreaImage->window)) // notebook page is visible
		on_mainDrawingAreaImage_expose_event(mDrawingAreaImage, NULL, (void *) this);
}

//*******************************************************************
void MainWindow::refreshOrigImage()
{
	if (TRUE == GDK_IS_DRAWABLE(mDrawingAreaOrigImage->window)) // notebook page is visible
		on_mainDrawingAreaOrigImage_expose_event(mDrawingAreaOrigImage, NULL, (void *) this);
}

//*******************************************************************
void MainWindow::refreshOutline()
{
	if (TRUE == GDK_IS_DRAWABLE(mDrawingAreaOutline->window)) // notebook page is visible
		on_mainDrawingAreaOutline_expose_event(mDrawingAreaOutline, NULL, (void *) this);
}

//*******************************************************************
void MainWindow::clearText()				//***002DB - nf
{
	gtk_entry_set_text(GTK_ENTRY(mSearchID), ""); //***1.85 - always clear this

  	if(mDatabase->size()!=0)
    		return;

  	gtk_entry_set_text(GTK_ENTRY(mEntryID), " ");
  	gtk_entry_set_text(GTK_ENTRY(mEntryName), " ");
  	gtk_entry_set_text(GTK_ENTRY(mEntryDate), " ");
  	gtk_entry_set_text(GTK_ENTRY(mEntryRoll), " ");
  	gtk_entry_set_text(GTK_ENTRY(mEntryLocation), " ");
  	gtk_entry_set_text(GTK_ENTRY(mEntryDamage), " ");
  	gtk_entry_set_text(GTK_ENTRY(mEntryDescription), " ");

  	gtk_widget_set_sensitive(mEntryID, FALSE);
  	gtk_widget_set_sensitive(mEntryName, FALSE);
  	gtk_widget_set_sensitive(mEntryDate, FALSE);
  	gtk_widget_set_sensitive(mEntryRoll, FALSE);
  	gtk_widget_set_sensitive(mEntryLocation, FALSE);
  	gtk_widget_set_sensitive(mEntryDamage, FALSE);
  	gtk_widget_set_sensitive(mEntryDescription, FALSE);

  	gtk_frame_set_label(GTK_FRAME(mFrameMod), "ID Code");
  	gtk_frame_set_label(GTK_FRAME(mFrameOrig), "ID Code");

  	return;

}

//*******************************************************************
void MainWindow::refreshDatabaseDisplay()
{
	if (NULL == mDatabase)
		return;

	try {
		unsigned numEntries = mDatabase->size();

		// Some variables for the pixmap display
		GdkPixmap *pixmap = NULL;
		GdkBitmap *mask = NULL;

		gtk_clist_freeze(GTK_CLIST(mCList));
		gtk_clist_clear(GTK_CLIST(mCList));

	  	if (mDatabase->size() == 0){            //***002DB >
	    		gtk_widget_set_sensitive(mButtonNext, FALSE);
	    		gtk_widget_set_sensitive(mButtonPrev, FALSE);
	    		gtk_widget_set_sensitive(mButtonModify, FALSE);
	  	}					//***002DB <
		else if (mDatabase->size() == 1)        //***003MR   
				gtk_widget_set_sensitive(mButtonModify, TRUE); //**003MR

		//***1.85 - set font as currently selected 

		gtk_widget_modify_font(
			mCList,
			(pango_font_description_from_string(mOptions->mCurrentFontName.c_str())));

		for (unsigned i = 0; i < numEntries; i++) {
			DatabaseFin<ColorImage> *fin = mDatabase->getItem(i);
			create_gdk_pixmap_from_data(
					mCList,
					&pixmap,
					&mask,
					fin->mThumbnailPixmap);

			//***2.2 - diagnostic (ID and primary key from SQL
			cout << "Fin ID : " << fin->getID();
			cout << "Fin key: " << fin->mDataPos;

			// make a copy of the thumbnail to store as data within the GTK pixmap
			char **thumbCopy = copy_thumbnail(fin->mThumbnailPixmap);

			//***1.85 - attach thumbnail copy to drawable
			gdk_drawable_set_data(GDK_DRAWABLE(pixmap),"thumb",thumbCopy,free_thumbnail);

			gchar *idCode, *name, *damage, *date, *location;

			if (mOptions->mHideIDs) //***1.65 - hide IDs if needed
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
					i,
					0,
					pixmap,
					mask);

			delete[] idCode;
			delete[] name;
			delete[] damage;
			delete[] date;
			delete[] location;

			delete fin;

			if (NULL != pixmap)
				gdk_pixmap_unref(pixmap);
	
			if (NULL != mask)
				gdk_bitmap_unref(mask);
		}
	
		gtk_clist_thaw(GTK_CLIST(mCList));
	} catch (Error e) {
		showError(_("The database seems to be corrupted.\n"
			  "Some (or all) entries may not appear\n"
			  "correctly."));
	}
}
//*******************************************************************
//***1.85 - uses the existing CList entries rather than reloading the 
//          entire database file
//
void MainWindow::refreshDatabaseDisplayNew(bool sizeChanged)
{
	if (NULL == mDatabase)
		return;

	try {
		unsigned numEntries = mDatabase->size();

		//gtk_clist_freeze(GTK_CLIST(mCList));
		//gtk_clist_clear(GTK_CLIST(mCList)); //***1.85 - do this differently below

	  	if (mDatabase->size() == 0){            //***002DB >
	    		gtk_widget_set_sensitive(mButtonNext, FALSE);
	    		gtk_widget_set_sensitive(mButtonPrev, FALSE);
	    		gtk_widget_set_sensitive(mButtonModify, FALSE);

				// we may be loading an empty database so make sure leftovers
				// from previous database are freed
				if (NULL != mSelectedFin)
				{
					delete mSelectedFin;
					mSelectedFin = NULL;
				}
				if (NULL != mImage)
				{
					delete mImage;
					mImage = NULL;
				}
	  	}					//***002DB <
		else //if (mDatabase->size() == 1)        //***003MR 
			//SAH -- Set modify active no matter what. It is possible to import multiple finzs and jump from 0 to X fins in a database all at once (where X>1)			
				gtk_widget_set_sensitive(mButtonModify, TRUE); //**003MR

		//***1.85 - set font as currently selected 

		gtk_widget_modify_font(
			mCList,
			(pango_font_description_from_string(mOptions->mCurrentFontName.c_str())));

		if (sizeChanged) // true if fin was added to or deleted from database
		{
		
			// Some variables for the pixmap display
			GdkPixmap *pixmap = NULL;
			GdkBitmap *mask = NULL;

			gtk_clist_clear(GTK_CLIST(mCList)); // clear and then rebuild from database

			mRow2Id.clear(); //***1.95
			mId2Row.clear(); //***1.95

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

				if (mOptions->mHideIDs) //***1.65 - hide IDs if needed
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
		}
		else // no change in size of database, so use existing CList entries
		{
			//***1.96a - if we have already dealt with the re-sorting of the list
			// do NOT do it again
			if (mOldSort == mNewSort)
				return;

			//***1.95 - copy for now - we will update as we process to get new translations
			vector<int> 
				newRow2Id(mRow2Id), 
				newId2Row(mId2Row);

			unsigned i;

			// ARRAYS for temporary storage of line info for new CList

			GdkPixmap **pixmap = new GdkPixmap* [numEntries];
			GdkBitmap **mask = new GdkBitmap* [numEntries];
			
			gchar** *itemInfo = new gchar** [numEntries];
			for (i = 0; i < numEntries; i++)
				itemInfo[i] = new gchar* [6];

			// do NOT clear existing CList until entries extracted

			//GtkWidget *tempCList = gtk_clist_new(6);

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

			for (i = 0; i < numEntries; i++)
				delete [] itemInfo[i];
			delete [] itemInfo; 

			delete [] pixmap;
			delete [] mask;
		}

		//gtk_clist_thaw(GTK_CLIST(mCList));

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
void MainWindow::selectFromCList(int newCurEntry)       //***004CL
{
	//gtk_clist_select_row(GTK_CLIST(mCList), newCurEntry, 0);

	//*** 1.7CL - all that follows

	const int lineHeight = DATABASEFIN_THUMB_HEIGHT + 1;
	static int lastEntry = 0;

	int pageEntries = mScrollable->allocation.height / lineHeight;

	//if ((int)mDatabase->size() == 0)
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
			//float where = (double)newCurEntry / (int)mDatabase->size();
			float where = (double)newCurEntry / (int)mRow2Id.size(); //***1.96
			adj->value = where * (adj->upper - adj->lower);
			//if (adj->value > adj->upper - adj->page_size)
			//	adj->value = adj->upper - adj->page_size;
			topEntry = newCurEntry;
		}

		//g_print("scroll value = %5.2f (%d %d)\n",
		//	adj->value,
		//	(newCurEntry-topEntry)*lineHeight,
		//	mScrollable->allocation.height);

		// this should be called but seems to be meaningless and scroll update
		// works without it - JHS
		//gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(mScrollable),adj);

		gtk_adjustment_value_changed(adj);
	}
	lastEntry = newCurEntry;
}

//*******************************************************************
void MainWindow::selectFromReorderedCList(std::string filename){  //***004CL
	if (NULL == mDatabase)
		return;
	try {
		unsigned numEntries = mDatabase->size();
                unsigned i = 0;
                bool found = false;
		while ((i < numEntries) && (!found)) {
			//DatabaseFin<ColorImage> *fin = mDatabase->getItem(i);
			DatabaseFin<ColorImage> *fin = mDatabase->getItem(mRow2Id[i]); //***1.95
			if (fin->mImageFilename == filename){
				found = true;
				gtk_clist_select_row(GTK_CLIST(mCList), i, 0); //***1.7
				//selectFromCList(i);
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
void MainWindow::selectFromReorderedCListNew(std::string selectedIdPlusOffset){  //***004CL
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
//***SAH: This function is called on each expose event for the notebook pages with Outline,Image,and OrigImage
//        It is also called once in mainWindowCreate()
void MainWindow::updateCursor()
{
    GdkBitmap *bitmap, *mask;
	GdkColor white = {0,0xFFFF,0xFFFF,0xFFFF};
	GdkColor black = {0,0x0000,0x0000,0x0000};
	
	//if (NULL != mCursor)
		//gdk_cursor_destroy(mCursor);

	if (mCursor==NULL){
		bitmap = gdk_bitmap_create_from_data(NULL,
						  magnify_cursor, magnify_cursor_width,
						  magnify_cursor_height);
		mask = gdk_bitmap_create_from_data(NULL,
						  magnify_mask, magnify_cursor_width,
						  magnify_cursor_height);
		mCursor = gdk_cursor_new_from_pixmap(
						  bitmap, mask, &black, &white,
						  magnify_xhot, magnify_yhot);
	}
 
	// I'm paranoid, what can I say?
	if (NULL != mCursor && NULL != mDrawingAreaOutline &&
		NULL != mDrawingAreaOutline->window) //***1.99 - notebook page may be hidden
		gdk_window_set_cursor(mDrawingAreaOutline->window, mCursor);

	if (NULL != mCursor && NULL != mDrawingAreaImage &&
		NULL != mDrawingAreaImage->window) //***1.99 - notebook page may be hidden
		gdk_window_set_cursor(mDrawingAreaImage->window, mCursor);

	if (NULL != mCursor && NULL != mDrawingAreaOrigImage &&
		NULL != mDrawingAreaOrigImage->window) //***1.99 - notebook page may be hidden
		gdk_window_set_cursor(mDrawingAreaOrigImage->window, mCursor);

}

//*******************************************************************
void MainWindow::updateGC()
{
	if (NULL == mGC) {
		if (NULL == mDrawingAreaOutline || 
			NULL == mDrawingAreaOutline->window) //***1.99 - notebook page may  be hidden
			return;

		mGC = gdk_gc_new(mDrawingAreaOutline->window);
	}

	updateGCColor();
}

//*******************************************************************
void MainWindow::updateGCColor()
{
	if (NULL == mGC)
		return;

	GdkColormap *colormap;
	GdkColor gdkColor;

	//gdkColor.red = (gushort)(0xFFFFF * mOptions->mCurrentColor[0]);
	//gdkColor.green = (gushort)(0xFFFFF * mOptions->mCurrentColor[1]);
	//gdkColor.blue = (gushort)(0xFFFFF * mOptions->mCurrentColor[2]);

	//***1.7 - use BLACK now
	gdkColor.red = 0;
	gdkColor.green = 0;
	gdkColor.blue = 0;
	
	colormap = gdk_colormap_get_system();
	gdk_color_alloc(colormap, &gdkColor);

	gdk_gc_set_foreground(mGC, &gdkColor);
}

//*******************************************************************
void MainWindow::refreshOptions(bool initialShow, bool forceReload)
{
	updateGCColor();
	if (! initialShow)
		refreshOutline();

	switch (mOptions->mToolbarDisplay) {
		case TEXT:
			gtk_toolbar_set_style(
					GTK_TOOLBAR(mToolBar),
					GTK_TOOLBAR_TEXT);
			break;
		case PICTURES:
			gtk_toolbar_set_style(
					GTK_TOOLBAR(mToolBar),
					GTK_TOOLBAR_ICONS);
			break;
		default:
			gtk_toolbar_set_style(
					GTK_TOOLBAR(mToolBar),
					GTK_TOOLBAR_BOTH);
			break;
	}

	//***1.96a - reload if ID show/hide state changes
	if (forceReload)
	{
		refreshDatabaseDisplayNew(true); // force reload from DB
		gtk_clist_select_row(GTK_CLIST(mCList), mDBCurEntry, 0);
	}

	//***1.96a - need to force "new" font to be displayed
	gtk_widget_modify_font(
			mCList,
			(pango_font_description_from_string(mOptions->mCurrentFontName.c_str())));
}

//*******************************************************************
void MainWindow::displayStatusMessage(const string &msg)
{
	gtk_statusbar_pop(GTK_STATUSBAR(mStatusBar), mContextID);
	gtk_statusbar_push(GTK_STATUSBAR(mStatusBar), mContextID, msg.c_str());
}

//*******************************************************************
GtkWidget *MainWindow::getWindow()
{
	return mWindow;
}

//*******************************************************************
GtkWidget* MainWindow::createMainWindow(toolbarDisplayType toolbarDisplay)
{
	GtkWidget *mainWindow;
	GtkWidget *mainVBox;
	GtkWidget *mainMenuHandleBox;
	GtkWidget *manMenuBar;
	//guint tmp_key;
	GtkWidget *file;
	GtkWidget *file_menu;
	GtkWidget *open;
	GtkWidget *matching_queue;
	GtkWidget *separator1;
	GtkWidget *exit;
	GtkWidget *catalog;        //***1.4
	GtkWidget *catalog_menu;   //***1.4
	GtkWidget *catalog_new;    //***1.4
	GtkWidget *catalog_view;   //***1.4
	GtkWidget *catalog_select; //***1.4
	GtkWidget *settings;
	GtkWidget *settings_menu;
	GtkWidget *options;
	GtkWidget *help;
	GtkWidget *help_menu;
	GtkWidget *about;
	GtkWidget *mainToolbarHandleBox;
	GtkWidget *tmp_toolbar_icon;
	GtkWidget *mainButtonOpen;
	GtkWidget *mainButtonOpenTrace;  //***1.5
	GtkWidget *mainButtonMatchingQueue;
	//GtkWidget *mainButtonOptions;
	GtkWidget *mainButtonExit;
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
	//guint mainButtonPrev_key;
	//guint mainButtonNext_key;
	//guint mainButtonModify_key;		//***002DB
	GtkWidget *mainInfoTable;
	GtkWidget *mainLabelID;
	GtkWidget *mainLabelName;
	GtkWidget *mainLabelDate;
	GtkWidget *mainLabelRoll;
	GtkWidget *mainLabelLocation;
	GtkWidget *mainLabelDamage;
	GtkWidget *mainLabelDescription;
	GtkWidget *mainRightVBox;
	GtkWidget *mainEventBoxImage;
	GtkWidget *mainFrameOutline;
	GtkWidget *mainEventBoxOutline;
	GtkAccelGroup *accel_group;
	GtkTooltips *tooltips;
	GtkWidget *tearoff;
	GtkWidget *tmpLabel, *tmpIcon, *tmpBox;
	GtkWidget *leftFrame, *rightFrame;
  

	string MainWinTitle = "DARWIN - ";
	//***1.85 - set the database filename / message for the window
	switch (mDatabase->status())
	{
	case Database::loaded :
		MainWinTitle += mOptions->mDatabaseFileName;
		break;
	case Database::fileNotFound :
		MainWinTitle += "Database file not found";
		break;
	case Database::errorCreating :
		MainWinTitle += "Database file could not be created";
		break;
	case Database::errorLoading :
		MainWinTitle += "Error loading database file";
		break;
	case Database::oldDBVersion :
		MainWinTitle += "OLD database file version - not supported";
		break;
	default :
		MainWinTitle += "unknown error with database file";
		break;
	}

	tooltips = gtk_tooltips_new ();

	accel_group = gtk_accel_group_new ();

	mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_object_set_data(GTK_OBJECT (mainWindow), "mainWindow", mainWindow);
	gtk_window_set_title(GTK_WINDOW (mainWindow), _(MainWinTitle.c_str())); //***1.85
	gtk_window_set_policy(GTK_WINDOW (mainWindow), TRUE, TRUE, FALSE);
	gtk_window_set_wmclass(GTK_WINDOW(mainWindow), "darwin_main", "DARWIN");
	gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(mainWindow), 800, 600); //***1.7

	mainVBox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (mainVBox);
	gtk_container_add (GTK_CONTAINER (mainWindow), mainVBox);

	mainMenuHandleBox = gtk_handle_box_new ();
	gtk_widget_show (mainMenuHandleBox);
	gtk_box_pack_start (GTK_BOX (mainVBox), mainMenuHandleBox, FALSE, TRUE, 0);

	manMenuBar = gtk_menu_bar_new();
	gtk_widget_show(manMenuBar);
	gtk_container_add(GTK_CONTAINER(mainMenuHandleBox), manMenuBar);

	// create "File" menu item (mnemonic ALT-F)

	file = gtk_menu_item_new_with_mnemonic(_("_File"));
	gtk_widget_show (file);
	gtk_container_add (GTK_CONTAINER (manMenuBar), file);

	file_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (file), file_menu);
  
	// allow File menu to be detachable

	tearoff = gtk_tearoff_menu_item_new();
	gtk_menu_append(GTK_MENU(file_menu), tearoff);
	gtk_widget_show(tearoff);

	// create "New Database" item in File submenu (mnemonic N, accelerator Ctrl-N)

	GtkWidget *newdb = gtk_menu_item_new();
	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpLabel = gtk_label_new_with_mnemonic(_("    _New Database       Ctrl-N"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), newdb);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(newdb), tmpBox);

	gtk_widget_show(newdb);
	gtk_container_add(GTK_CONTAINER (file_menu), newdb);
	gtk_tooltips_set_tip(tooltips, newdb, _("Create a new empty database using the current default\n"
		                                    "catalog scheme."), NULL);
	gtk_widget_add_accelerator(newdb, "activate", accel_group,
	                           GDK_N, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);

	// create a separator line in submenu

	separator1 = gtk_menu_item_new ();
	gtk_widget_show (separator1);
	gtk_container_add (GTK_CONTAINER (file_menu), separator1);
	gtk_widget_set_sensitive (separator1, FALSE);

	// create "Open Image" menu item in File submenu (mnemonic O, accelerator Ctrl-O)
  
	open = gtk_menu_item_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, open_image_small_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("_Open Image          Ctrl-O"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), open);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(open), tmpBox);

	gtk_widget_show(open);
	gtk_container_add(GTK_CONTAINER (file_menu), open);
	gtk_tooltips_set_tip(tooltips, open, _("Open a dorsal fin image."), NULL);
	gtk_widget_add_accelerator(open, "activate", accel_group,
	                           GDK_O, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);

	//***1.85 - if database load failed do not allow this menu option
	if (mDatabase->status() != Database::loaded)
		gtk_widget_set_sensitive(open, FALSE);
	mOpenImageMenuItem = open;

	// create "Open Traced Fin" menu item in File submenu (mnemonic T, accelerator Ctrl-T)
  
	GtkWidget *openTFin = gtk_menu_item_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, open_trace_small_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("Open _Traced Fin    Ctrl-T"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), openTFin);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(openTFin), tmpBox);

	gtk_widget_show(openTFin);
	gtk_container_add(GTK_CONTAINER (file_menu), openTFin);
	gtk_tooltips_set_tip(tooltips, openTFin, _("Open a previously saved dorsal fin tracing."), NULL);
	gtk_widget_add_accelerator(openTFin, "activate", accel_group,
	                           GDK_T, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);

	//***1.85 - if database load failed do not allow this menu option
	if (mDatabase->status() != Database::loaded)
		gtk_widget_set_sensitive(openTFin, FALSE);
	mOpenFinMenuItem = openTFin;

	// create "Open Database" item in File submenu (mnemonic D, accelerator Ctrl-D)

	GtkWidget *opendb = gtk_menu_item_new();
	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpLabel = gtk_label_new_with_mnemonic(_("    Open _Database     Ctrl-D"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), opendb);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(opendb), tmpBox);

	gtk_widget_show(opendb);
	gtk_container_add(GTK_CONTAINER (file_menu), opendb);
	gtk_tooltips_set_tip(tooltips, opendb, _("Open a different database."), NULL);
	gtk_widget_add_accelerator(opendb, "activate", accel_group,
	                           GDK_D, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);

	// create a separator line in submenu

	separator1 = gtk_menu_item_new ();
	gtk_widget_show (separator1);
	gtk_container_add (GTK_CONTAINER (file_menu), separator1);
	gtk_widget_set_sensitive (separator1, FALSE);

	// create "Matching Queue" item in File submenu (mnemonic Q, accelerator Ctrl-Q)

	matching_queue = gtk_menu_item_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, matching_queue_small_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("Matching _Queue    Ctrl-Q"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), matching_queue);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(matching_queue), tmpBox);

	gtk_widget_show (matching_queue);
	gtk_container_add (GTK_CONTAINER (file_menu), matching_queue);
	gtk_tooltips_set_tip(tooltips, matching_queue, 
	                     _("Create queues of saved fin traces, run queued matches in batch mode, and review saved match results."), 
	                     NULL);
	gtk_widget_add_accelerator(matching_queue, "activate", accel_group,
	                           GDK_Q, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);

		//***1.85 - if database load failed do not allow this menu option
	if (mDatabase->status() != Database::loaded)
		gtk_widget_set_sensitive(matching_queue, FALSE);
	mQueueMenuItem = matching_queue;

	// create a separator line in submenu

	separator1 = gtk_menu_item_new ();
	gtk_widget_show (separator1);
	gtk_container_add (GTK_CONTAINER (file_menu), separator1);
	gtk_widget_set_sensitive (separator1, FALSE);

	//***1.85 - menu options for backup and restore

	GtkWidget *backup = gtk_menu_item_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpLabel = gtk_label_new_with_mnemonic(_("    _Backup                 Ctrl-B"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), backup);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(backup), tmpBox);

	gtk_widget_show (backup);
	gtk_container_add (GTK_CONTAINER (file_menu), backup);
	gtk_tooltips_set_tip (tooltips, backup, _("Backup the currently open database."), NULL);
	gtk_widget_add_accelerator(backup, "activate", accel_group,
	                           GDK_B, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);

	//***1.85 - if database load failed do not allow this menu option
	if (mDatabase->status() != Database::loaded)
		gtk_widget_set_sensitive(backup, FALSE);
	mBackupMenuItem = backup;

	GtkWidget *restore = gtk_menu_item_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpLabel = gtk_label_new_with_mnemonic(_("    _Restore                 Ctrl-R"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), restore);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(restore), tmpBox);

	gtk_widget_show (restore);
	gtk_container_add (GTK_CONTAINER (file_menu), restore);
	gtk_tooltips_set_tip (tooltips, restore, _("Restore a database from previous backup."), NULL);
	gtk_widget_add_accelerator(restore, "activate", accel_group,
	                           GDK_R, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);


	// create a separator line in submenu

	separator1 = gtk_menu_item_new ();
	gtk_widget_show (separator1);
	gtk_container_add (GTK_CONTAINER (file_menu), separator1);
	gtk_widget_set_sensitive (separator1, FALSE);

	////////////////////////

	//***1.85 - menu options for import and export

	/*GtkWidget *importDB = gtk_menu_item_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpLabel = gtk_label_new(_("    Import"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), importDB);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(importDB), tmpBox);

	gtk_widget_show (importDB);
	gtk_container_add (GTK_CONTAINER (file_menu), importDB);
	gtk_tooltips_set_tip (tooltips, importDB, 
		_("Import a database ...\n"
		"(into a NEW user specified Survey Area.)"), NULL);*/


	// new Import submenu

	GtkWidget *import = gtk_menu_item_new_with_mnemonic(_("    Import"));
	gtk_widget_show (import);
	gtk_container_add (GTK_CONTAINER (file_menu), import);

	GtkWidget *importSub = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (import), importSub);

	// Import Catalog submenu item

	GtkWidget *importDB = gtk_menu_item_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpLabel = gtk_label_new(_("    Import Catalog"));

	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), importDB);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(importDB), tmpBox);

	gtk_widget_show (importDB);

	gtk_container_add(GTK_CONTAINER(importSub), importDB);

	gtk_tooltips_set_tip (tooltips, importDB, 
		_("Import a database ...\n"
		"(into a NEW user specified Survey Area.)"), NULL);

	mImportDBMenuItem = importDB;

	// Import Fin (*.finz) submenu item

	mImportFinzMenuItem = gtk_menu_item_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpLabel = gtk_label_new(_("    Import Fin (*.finz)"));

	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), mImportFinzMenuItem);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(mImportFinzMenuItem), tmpBox);

	gtk_widget_show (mImportFinzMenuItem);

	gtk_container_add(GTK_CONTAINER(importSub), mImportFinzMenuItem);

	gtk_tooltips_set_tip (tooltips, mImportFinzMenuItem, 
		_("Import one or more fins ...\n"
		"(from user specified *.finz files)"), NULL);

	//***1.85 - if database load failed do not allow this menu option
	if (mDatabase->status() != Database::loaded)
		gtk_widget_set_sensitive(mImportFinzMenuItem, FALSE);

	//***************************************************************//

	// new Export submenu

	mExportSubMenuItem = gtk_menu_item_new_with_mnemonic(_("    Export"));
	gtk_widget_show (mExportSubMenuItem);
	gtk_container_add (GTK_CONTAINER (file_menu), mExportSubMenuItem);

	// if database load failed do not allow this menu option
	if (mDatabase->status() != Database::loaded)
		gtk_widget_set_sensitive(mExportSubMenuItem, FALSE);

	GtkWidget *exportSubMenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (mExportSubMenuItem), exportSubMenu);

	// Export Catalog submenu item

	mExportDBMenuItem = gtk_menu_item_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpLabel = gtk_label_new(_("    Export Catalog"));

	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), mExportDBMenuItem);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(mExportDBMenuItem), tmpBox);

	gtk_widget_show (mExportDBMenuItem);

	gtk_container_add(GTK_CONTAINER(exportSubMenu), mExportDBMenuItem);

	gtk_tooltips_set_tip (tooltips, mExportDBMenuItem, 
		_("Export the currently open database ...\n"
		"(to a user specified *.zip file)"), NULL);

	// Export Fin (*.finz) submenu item

	mExportFinzMenuItem = gtk_menu_item_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpLabel = gtk_label_new(_("    Export Fin (*.finz)"));

	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), mExportFinzMenuItem);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(mExportFinzMenuItem), tmpBox);

	gtk_widget_show (mExportFinzMenuItem);

	gtk_container_add(GTK_CONTAINER(exportSubMenu), mExportFinzMenuItem);

	gtk_tooltips_set_tip (tooltips, mExportFinzMenuItem, 
		_("Export a fin ...\n"
		"(to a user specified *.finz file)"), NULL);

	//***2.02 - new menu item to allow generation of FULL-SIZE modified images

	// Export FullSzImgs (*FullSize.png) submenu item

	mExportFullSzImgsMenuItem = gtk_menu_item_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpLabel = gtk_label_new(_("    Export Images (Full-Size)"));

	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), mExportFullSzImgsMenuItem);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(mExportFullSzImgsMenuItem), tmpBox);

	gtk_widget_show (mExportFullSzImgsMenuItem);

	gtk_container_add(GTK_CONTAINER(exportSubMenu), mExportFullSzImgsMenuItem);

	gtk_tooltips_set_tip (tooltips, mExportFullSzImgsMenuItem, 
		_("Export Modified Images from Catalog ...\n"
		"(to a user specified location)"), NULL);

	// create a separator line in submenu

	separator1 = gtk_menu_item_new ();
	gtk_widget_show (separator1);
	gtk_container_add (GTK_CONTAINER (file_menu), separator1);
	gtk_widget_set_sensitive (separator1, FALSE);

	////////////////////////
	// create "Exit" menu item in File submenu (mnemonic X, accelerator Ctrl-X)

	exit = gtk_menu_item_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, exit_small_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("E_xit                      Ctrl-X"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), exit);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(exit), tmpBox);

	gtk_widget_show (exit);
	gtk_container_add (GTK_CONTAINER (file_menu), exit);
	gtk_tooltips_set_tip (tooltips, exit, _("Exit the DARWIN program."), NULL);
	gtk_widget_add_accelerator(exit, "activate", accel_group,
	                           GDK_X, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);

	// create "Settings" menu item (mnemonic ALT-S)

	settings = gtk_menu_item_new_with_mnemonic(_("_Settings"));
	gtk_widget_show (settings);
	gtk_container_add (GTK_CONTAINER (manMenuBar), settings);

	settings_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (settings), settings_menu);

	// allow Settings submenu to be detachable

	tearoff = gtk_tearoff_menu_item_new();
	gtk_menu_append(GTK_MENU(settings_menu), tearoff);
	gtk_widget_show(tearoff);

	// create "Options" menu item in Settings submenu (mnemonic P, accelerator Ctrl-P)

	options = gtk_menu_item_new();
  
	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, options_small_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("O_ptions        Ctrl-P"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), options);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(options), tmpBox);

	gtk_widget_show(options);
	gtk_container_add(GTK_CONTAINER (settings_menu), options);
	gtk_tooltips_set_tip(tooltips, options, 
	                     _("Settings that change the way DARWIN operates."),
	                     NULL);
	gtk_widget_add_accelerator(options, "activate", accel_group,
	                           GDK_P, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);

	//***1.4 - create "Catalog Scheme" menu item

	catalog = gtk_menu_item_new_with_mnemonic(_("_Catalog Scheme"));
	gtk_widget_show (catalog);
	gtk_container_add (GTK_CONTAINER (settings_menu), catalog);

	catalog_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (catalog), catalog_menu);

	// allow Catalog submenu to be detachable

	//tearoff = gtk_tearoff_menu_item_new();
	//gtk_menu_append(GTK_MENU(catalog_menu), tearoff);
	//gtk_widget_show(tearoff);


	// create "Define New" menu item in Catalog Scheme submenu

	catalog_new = gtk_menu_item_new();
  
	tmpBox = gtk_hbox_new(FALSE, 0);
	//tmpIcon = create_pixmap_from_data(tmpBox, options_small_xpm);
	//gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	//gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("Define New _Scheme     Ctrl-S"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), catalog_new);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(catalog_new), tmpBox);

	gtk_widget_show(catalog_new);
	gtk_container_add(GTK_CONTAINER (catalog_menu), catalog_new);
	gtk_tooltips_set_tip(tooltips, catalog_new, 
	                     _("Define the details of a new Catalog Scheme."),
	                     NULL);
	gtk_widget_add_accelerator(catalog_new, "activate", accel_group,
	                           GDK_S, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);

	// create "View/Edit" menu item in Catalog Scheme submenu

	catalog_view = gtk_menu_item_new();
  
	tmpBox = gtk_hbox_new(FALSE, 0);
	//tmpIcon = create_pixmap_from_data(tmpBox, options_small_xpm);
	//gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	//gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("_View/Edit                      Ctrl-V"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), catalog_view);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(catalog_view), tmpBox);

	gtk_widget_show(catalog_view);
	gtk_container_add(GTK_CONTAINER (catalog_menu), catalog_view);
	gtk_tooltips_set_tip(tooltips, catalog_view, 
	                     _("View or Edit the details of all defined Catalog Schemes."),
	                     NULL);
	gtk_widget_add_accelerator(catalog_view, "activate", accel_group,
	                           GDK_V, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);

	// create "Select Active" menu item in Catalog Scheme submenu

	catalog_select = gtk_menu_item_new();
  
	tmpBox = gtk_hbox_new(FALSE, 0);
	//tmpIcon = create_pixmap_from_data(tmpBox, options_small_xpm);
	//gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	//gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("Select _Active               Ctrl-A"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), catalog_select);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(catalog_select), tmpBox);

	gtk_widget_show(catalog_select);
	gtk_container_add(GTK_CONTAINER (catalog_menu), catalog_select);
	gtk_tooltips_set_tip(tooltips, catalog_select, 
	                     _("Select the currently active Catalog Scheme.\n"
						   "This affects the organization of any NEW database created."),
	                     NULL);
	gtk_widget_add_accelerator(catalog_select, "activate", accel_group,
	                           GDK_A, GDK_CONTROL_MASK,
	                           GTK_ACCEL_VISIBLE);


	//***1.9 - create "Data" menu item (mnemonic ALT-T)

	GtkWidget *data = gtk_menu_item_new_with_mnemonic(_("Da_ta"));
	gtk_widget_show (data);
	gtk_container_add (GTK_CONTAINER (manMenuBar), data);

	GtkWidget *data_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (data), data_menu);

	GtkWidget *exportData = gtk_menu_item_new();
  
	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, magnify_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("Export Selected Da_ta"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), exportData);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(exportData), tmpBox);

	gtk_widget_show(exportData);
	gtk_container_add(GTK_CONTAINER (data_menu), exportData);
	gtk_tooltips_set_tip(tooltips, exportData, _("Export selected data in <tab> separated format."), NULL);

	// create "Help" menu item (mnemonic ALT-H)

	help = gtk_menu_item_new_with_mnemonic (_("_Help"));
	gtk_menu_item_right_justify(GTK_MENU_ITEM(help));
	gtk_widget_show (help);
	gtk_container_add (GTK_CONTAINER (manMenuBar), help);

	help_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (help), help_menu);

	// allow Help submenu to be detachable

	tearoff = gtk_tearoff_menu_item_new();
	gtk_menu_append(GTK_MENU(help_menu), tearoff);
	gtk_widget_show(tearoff);
  
	// create "Documentation" menu item in Help Submenu (mnemonic D)

	GtkWidget *docs = gtk_menu_item_new();
  
	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, about_small_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("_Documentation"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), docs);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(docs), tmpBox);

	gtk_widget_show(docs);
	gtk_container_add(GTK_CONTAINER (help_menu), docs);
	gtk_tooltips_set_tip(tooltips, docs, _("DARWIN Online Documentation."), NULL);

	// create "About" menu item in Help Submenu (mnemonic A)

	about = gtk_menu_item_new();
  
	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, about_small_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("_About"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), about);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, FALSE, FALSE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(about), tmpBox);

	gtk_widget_show(about);
	gtk_container_add(GTK_CONTAINER (help_menu), about);
	gtk_tooltips_set_tip(tooltips, about, _("About DARWIN."), NULL);

	// create a toolbar with commonly used buttons

	mainToolbarHandleBox = gtk_handle_box_new ();
	gtk_widget_show (mainToolbarHandleBox);
	gtk_box_pack_start (GTK_BOX (mainVBox), mainToolbarHandleBox, FALSE, FALSE, 0);
	gtk_handle_box_set_shadow_type (GTK_HANDLE_BOX (mainToolbarHandleBox), GTK_SHADOW_NONE);

	switch (toolbarDisplay) {
		case TEXT:
			mToolBar = gtk_toolbar_new();
  			gtk_toolbar_set_orientation (GTK_TOOLBAR(mToolBar),GTK_ORIENTATION_HORIZONTAL);
  			gtk_toolbar_set_style (GTK_TOOLBAR(mToolBar), GTK_TOOLBAR_TEXT);
			break;
		case PICTURES:
			mToolBar = gtk_toolbar_new();
  			gtk_toolbar_set_orientation (GTK_TOOLBAR(mToolBar),GTK_ORIENTATION_HORIZONTAL);
  			gtk_toolbar_set_style (GTK_TOOLBAR(mToolBar), GTK_TOOLBAR_ICONS);
			break;
		default:
			mToolBar = gtk_toolbar_new();
  			gtk_toolbar_set_orientation (GTK_TOOLBAR(mToolBar),GTK_ORIENTATION_HORIZONTAL);
  			gtk_toolbar_set_style (GTK_TOOLBAR(mToolBar), GTK_TOOLBAR_BOTH);
			break;
	}

	gtk_widget_show(mToolBar);
	gtk_container_add(GTK_CONTAINER (mainToolbarHandleBox), mToolBar);

	// create "Open" button (shortcut to Open File Dialog - opening a fin image)

	tmp_toolbar_icon = create_pixmap_from_data(mainWindow, open_image_xpm);
	mainButtonOpen = gtk_toolbar_append_element (GTK_TOOLBAR (mToolBar),
	                            GTK_TOOLBAR_CHILD_BUTTON,
	                            NULL,
	                            _("Open"),
	                            _("Open a dorsal fin image."), NULL,
	                            tmp_toolbar_icon, NULL, NULL);
	gtk_button_set_relief (GTK_BUTTON (mainButtonOpen), GTK_RELIEF_NONE);
	gtk_widget_show (mainButtonOpen);

	//***1.85 - if database load failed do not allow this menu option
	if (mDatabase->status() != Database::loaded)
		gtk_widget_set_sensitive(mainButtonOpen, FALSE);
	mOpenImageButton = mainButtonOpen;

	// create another "Open" button (shortcut to Open File Chooser Dialog - opening a fin trace)

	tmp_toolbar_icon = create_pixmap_from_data(mainWindow, open_trace_xpm);
	mainButtonOpenTrace = gtk_toolbar_append_element (GTK_TOOLBAR (mToolBar),
	                            GTK_TOOLBAR_CHILD_BUTTON,
	                            NULL,
	                            _("Open"),
	                            _("Open a previously saved dorsal fin tracing."), NULL,
	                            tmp_toolbar_icon, NULL, NULL);
	gtk_button_set_relief (GTK_BUTTON (mainButtonOpenTrace), GTK_RELIEF_NONE);
	gtk_widget_show (mainButtonOpenTrace);

	//***1.85 - if database load failed do not allow this menu option
	if (mDatabase->status() != Database::loaded)
		gtk_widget_set_sensitive(mainButtonOpenTrace, FALSE);
  	mOpenFinButton = mainButtonOpenTrace;

	// create "Queue" button (shortcut to Matching Queue Dialog)

	tmp_toolbar_icon = create_pixmap_from_data(mainWindow, matching_queue_xpm);
	mainButtonMatchingQueue = gtk_toolbar_append_element (GTK_TOOLBAR (mToolBar),
	                            GTK_TOOLBAR_CHILD_BUTTON,
	                            NULL,
	                            _("Queue"),
	                            _("Create queues of saved fin traces, run queued matches in batch mode, and review saved match results."), NULL,
								tmp_toolbar_icon, NULL, NULL);
	gtk_button_set_relief (GTK_BUTTON (mainButtonMatchingQueue), GTK_RELIEF_NONE);
	gtk_widget_show (mainButtonMatchingQueue);

	//***1.85 - if database load failed do not allow this menu option
	if (mDatabase->status() != Database::loaded)
		gtk_widget_set_sensitive(mainButtonMatchingQueue, FALSE);
	mQueueButton = mainButtonMatchingQueue;

	// create "Options" button (shortcut to Options Dialog)

	/*
	//***1.5 - eliminated this button
	tmp_toolbar_icon = create_pixmap_from_data(mainWindow, options_xpm);
	mainButtonOptions = gtk_toolbar_append_element (GTK_TOOLBAR (mToolBar),
	                            GTK_TOOLBAR_CHILD_BUTTON,
	                            NULL,
	                            _("Options"),
	                            _("A number of settings that change the way DARWIN operates."), NULL,
	                            tmp_toolbar_icon, NULL, NULL);
	gtk_button_set_relief (GTK_BUTTON (mainButtonOptions), GTK_RELIEF_NONE);
	gtk_widget_show (mainButtonOptions);
	*/

	// create "Exit" button (Shortcut to Exit program)

	tmp_toolbar_icon = create_pixmap_from_data(mainWindow, exit_xpm);
	mainButtonExit = gtk_toolbar_append_element (GTK_TOOLBAR (mToolBar),
	                            GTK_TOOLBAR_CHILD_BUTTON,
	                            NULL,
	                            _("Exit"),
	                            _("Quit DARWIN."), NULL,
	                            tmp_toolbar_icon, NULL, NULL);
	gtk_button_set_relief (GTK_BUTTON (mainButtonExit), GTK_RELIEF_NONE);
	gtk_widget_show (mainButtonExit);

	/*
	//***1.85 - new box for tools above Clist and Image
	GtkWidget *tempHbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start (GTK_BOX (mainVBox), tempHbox, FALSE, FALSE, 0);
	gtk_widget_show(tempHbox);

	//***1.95 - new radio buttons to select view PRIMARY images only or view ALL
	GtkWidget *button = gtk_radio_button_new_with_label(
				NULL,
				_("Show ONLY Primary Images"));
	gtk_box_pack_start (GTK_BOX (tempHbox), button, FALSE, FALSE, 0);
	gtk_widget_show(button);
	button = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(button),
				_("Show ALL Images"));
	gtk_box_pack_start (GTK_BOX (tempHbox), button, FALSE, FALSE, 0);
	gtk_widget_show(button);
*/
	/*
	//***1.85 - new FindDolphin by ID tool
	tempHbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start (GTK_BOX (mainVBox), tempHbox, FALSE, FALSE, 0);
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
	*/

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

	//***1.96 - new radio buttons to select view PRIMARY images only or view ALL
	GtkWidget *tempHbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start (GTK_BOX (mainLeftVBox), tempHbox, FALSE, FALSE, 0);
	//gtk_widget_show(tempHbox); //***1.96a - do NOT show/allow this option for now

	GtkWidget *showLabel = gtk_label_new(_("Showing: "));
	gtk_box_pack_start (GTK_BOX (tempHbox), showLabel, FALSE, FALSE, 0);
	gtk_widget_show(showLabel);

	GtkWidget *button = gtk_radio_button_new_with_label(
				NULL,
				_("ONLY Primary Images"));
	gtk_box_pack_start (GTK_BOX (tempHbox), button, FALSE, FALSE, 0);
	gtk_widget_show(button);

	button = gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(button),
				_("ALL Images"));
	// note: we only care about toggling the Show ALL button
	gtk_signal_connect (GTK_OBJECT (button), "toggled",
	                    GTK_SIGNAL_FUNC (on_mainButtonShowAllImages_toggled),
	                    (void *) this);
	gtk_box_pack_start (GTK_BOX (tempHbox), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
	gtk_widget_show(button);
	//***1.96 - end 

	mainScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (mainScrolledWindow);
	gtk_box_pack_start (GTK_BOX (mainLeftVBox), mainScrolledWindow, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mainScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	mScrollable = mainScrolledWindow; //***1.7CL

	mCList = gtk_clist_new (6);
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
	tempHbox = gtk_hbox_new(FALSE, 5);
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

	// create button box with "Previous", "Next" and "Modify Database" buttons

	hbuttonbox1 = gtk_hbutton_box_new();
	gtk_widget_show(hbuttonbox1);
	gtk_box_pack_start(GTK_BOX(mainLeftVBox), hbuttonbox1, FALSE, TRUE, 5);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox1), GTK_BUTTONBOX_SPREAD);

	// create Previous button (accelerator ALT-P)

	mButtonPrev = gtk_button_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, previous_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("_Previous"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), mButtonPrev);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);
  
	gtk_container_add(GTK_CONTAINER(mButtonPrev), tmpBox);

	gtk_widget_show(mButtonPrev);
	gtk_container_add(GTK_CONTAINER(hbuttonbox1), mButtonPrev);
	GTK_WIDGET_SET_FLAGS(mButtonPrev, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip(tooltips, mButtonPrev, 
	                     _("Cycle to the previous fin in the database."), NULL);

	// create Next button (mnemonic ALT-N)

	mButtonNext = gtk_button_new();

	tmpBox = gtk_hbox_new(FALSE, 0);
	tmpIcon = create_pixmap_from_data(tmpBox, next_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("_Next"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), mButtonNext);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);
  
	gtk_container_add(GTK_CONTAINER(mButtonNext), tmpBox);

	gtk_widget_show(mButtonNext);
	gtk_container_add (GTK_CONTAINER (hbuttonbox1), mButtonNext);
	GTK_WIDGET_SET_FLAGS (mButtonNext, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip (tooltips, mButtonNext, _("Cycle to the next fin in the database."), NULL);

	// create Modify Database button (mnemonic ALT-M)

	//***002DB >
	mButtonModify = gtk_button_new();

	tmpBox = gtk_hbox_new(FALSE, 0);			
	tmpIcon = create_pixmap_from_data(tmpBox, add_database_xpm);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpIcon, FALSE, FALSE, 0);
	gtk_widget_show(tmpIcon);
	tmpLabel = gtk_label_new_with_mnemonic(_("_Modify"));
	gtk_label_set_mnemonic_widget(GTK_LABEL(tmpLabel), mButtonModify);
	gtk_box_pack_start(GTK_BOX(tmpBox), tmpLabel, TRUE, TRUE, 0);
	gtk_widget_show(tmpLabel);
	gtk_widget_show(tmpBox);

	gtk_container_add(GTK_CONTAINER(mButtonModify), tmpBox);

	gtk_widget_show(mButtonModify);
	//***1.99 - this button goes with the data in the new notebook page below
	//gtk_container_add (GTK_CONTAINER (hbuttonbox1), mButtonModify);
	GTK_WIDGET_SET_FLAGS (mButtonModify, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip (tooltips, mButtonModify, _("Modify selected entry in the database."), NULL);
	//***002DB <


	mainInfoTable = gtk_table_new (7, 2, FALSE);
	gtk_widget_show(mainInfoTable);
	//***1.99 - the data table now goes inside a notebook page in the tabbed
	// view code below
	//gtk_box_pack_start (GTK_BOX (mainLeftVBox), mainInfoTable, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (mainInfoTable), 3);
	gtk_table_set_row_spacings (GTK_TABLE (mainInfoTable), 4);
	gtk_table_set_col_spacings (GTK_TABLE (mainInfoTable), 5);

	mEntryName = gtk_entry_new ();
	gtk_widget_show (mEntryName);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mEntryName, 1, 2, 1, 2,
	                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_editable (GTK_ENTRY (mEntryName), FALSE);

	mEntryDate = gtk_entry_new ();
	gtk_widget_show (mEntryDate);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mEntryDate, 1, 2, 2, 3,
	                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_editable (GTK_ENTRY (mEntryDate), FALSE);

	mEntryRoll = gtk_entry_new ();
	gtk_widget_show (mEntryRoll);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mEntryRoll, 1, 2, 3, 4,
	                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_editable (GTK_ENTRY (mEntryRoll), FALSE);

	mEntryLocation = gtk_entry_new ();
	gtk_widget_show (mEntryLocation);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mEntryLocation, 1, 2, 4, 5,
	                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_editable (GTK_ENTRY (mEntryLocation), FALSE);

	mEntryDamage = gtk_entry_new ();
	gtk_widget_show (mEntryDamage);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mEntryDamage, 1, 2, 5, 6,
	                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_editable (GTK_ENTRY (mEntryDamage), FALSE);

	mEntryDescription = gtk_entry_new ();
	gtk_widget_show (mEntryDescription);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mEntryDescription, 1, 2, 6, 7,
	                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_editable (GTK_ENTRY (mEntryDescription), FALSE);

	mainLabelID = gtk_label_new (_("ID Code"));
	gtk_widget_show (mainLabelID);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mainLabelID, 0, 1, 0, 1,
	                (GtkAttachOptions) (0),
	                (GtkAttachOptions) (0), 0, 0);

	mainLabelName = gtk_label_new (_("Name"));
	gtk_widget_show (mainLabelName);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mainLabelName, 0, 1, 1, 2,
	                (GtkAttachOptions) (0),
	                (GtkAttachOptions) (0), 0, 0);

	mainLabelDate = gtk_label_new (_("Date of Sighting"));
	gtk_widget_show (mainLabelDate);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mainLabelDate, 0, 1, 2, 3,
	                (GtkAttachOptions) (0),
	                (GtkAttachOptions) (0), 0, 0);

	mainLabelRoll = gtk_label_new (_("Roll and Frame"));
	gtk_widget_show (mainLabelRoll);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mainLabelRoll, 0, 1, 3, 4,
	                (GtkAttachOptions) (0),
	                (GtkAttachOptions) (0), 0, 0);

	mainLabelLocation = gtk_label_new (_("Location Code"));
	gtk_widget_show (mainLabelLocation);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mainLabelLocation, 0, 1, 4, 5,
	                (GtkAttachOptions) (0),
	                (GtkAttachOptions) (0), 0, 0);

	mainLabelDamage = gtk_label_new (_("Damage Category"));
	gtk_widget_show (mainLabelDamage);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mainLabelDamage, 0, 1, 5, 6,
	                (GtkAttachOptions) (0),
	                (GtkAttachOptions) (0), 0, 0);

	mainLabelDescription = gtk_label_new (_("Short Description"));
	gtk_widget_show (mainLabelDescription);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mainLabelDescription, 0, 1, 6, 7,
	                (GtkAttachOptions) (0),
	                (GtkAttachOptions) (0), 0, 0);

	mEntryID = gtk_entry_new ();
	gtk_widget_show (mEntryID);
	gtk_table_attach (GTK_TABLE (mainInfoTable), mEntryID, 1, 2, 0, 1,
	                (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                (GtkAttachOptions) (0), 0, 0);
	gtk_entry_set_editable (GTK_ENTRY (mEntryID), FALSE);

	//***1.99 - the right frame will now contain a TABBED area for the
	// modified image, original image, data fields, outline, ...

	rightFrame = gtk_frame_new(NULL);
	gtk_widget_show(rightFrame);
	gtk_frame_set_shadow_type(GTK_FRAME(rightFrame), GTK_SHADOW_IN);
	gtk_paned_pack2(GTK_PANED(mainHPaned), rightFrame, TRUE, TRUE);
  
	mainRightVBox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (mainRightVBox);
	gtk_container_add(GTK_CONTAINER(rightFrame), mainRightVBox);

	//*** - here is the NEW tabbed notebook

	GtkWidget *notebook = gtk_notebook_new();

	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
	gtk_box_pack_start(GTK_BOX (mainRightVBox), notebook, TRUE, TRUE, 0);
	gtk_widget_show(notebook);

	GtkWidget *tempLabel; // for tab text

	// now create a framed view for the MODIFIED IMAGE and put it in the notebook
	// as page number 1

	mFrameMod = gtk_frame_new(_("(ID Code)"));
	gtk_container_set_border_width(GTK_CONTAINER(mFrameMod), 3);
	gtk_frame_set_label_align(GTK_FRAME(mFrameMod), 1.0, 0.0);
	gtk_widget_show(mFrameMod);
	tempLabel = gtk_label_new(_("Modified Image"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), mFrameMod, tempLabel);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),0); // this is default page

	mainEventBoxImage = gtk_event_box_new ();
	gtk_widget_show (mainEventBoxImage);
	gtk_container_add (GTK_CONTAINER (mFrameMod), mainEventBoxImage);

	mDrawingAreaImage = gtk_drawing_area_new ();
	gtk_widget_show (mDrawingAreaImage);
	gtk_container_add (GTK_CONTAINER (mainEventBoxImage), mDrawingAreaImage);

	gtk_drawing_area_size(GTK_DRAWING_AREA(mDrawingAreaImage), IMAGE_WIDTH, IMAGE_HEIGHT);

	// now create a framed view for the ORIGINAL IMAGE and put it in the notebook
	// as page number 2

	mFrameOrig = gtk_frame_new(_("(ID Code)"));
	gtk_container_set_border_width(GTK_CONTAINER(mFrameOrig), 3);
	gtk_frame_set_label_align(GTK_FRAME(mFrameOrig), 1.0, 0.0);
	gtk_widget_show(mFrameOrig);
	tempLabel = gtk_label_new(_("Original Image"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), mFrameOrig, tempLabel);
	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),0); // this is default page

	GtkWidget *mainEventBoxOrigImage = gtk_event_box_new ();
	gtk_widget_show (mainEventBoxOrigImage);
	gtk_container_add (GTK_CONTAINER (mFrameOrig), mainEventBoxOrigImage);

	mDrawingAreaOrigImage = gtk_drawing_area_new ();
	gtk_widget_show (mDrawingAreaOrigImage);
	gtk_container_add (GTK_CONTAINER (mainEventBoxOrigImage), mDrawingAreaOrigImage);

	gtk_drawing_area_size(GTK_DRAWING_AREA(mDrawingAreaOrigImage), IMAGE_WIDTH, IMAGE_HEIGHT);

	// now create a framed view for the fin OUTLINE and put it in the notebook
	// as page number 3

	mainFrameOutline = gtk_frame_new (_("Fin Outline"));
	gtk_container_set_border_width(GTK_CONTAINER(mainFrameOutline), 3);
	gtk_frame_set_label_align(GTK_FRAME(mainFrameOutline), 1.0, 0.0);
	gtk_widget_show (mainFrameOutline);
	tempLabel = gtk_label_new(_("Fin Outline"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), mainFrameOutline, tempLabel);

	mainEventBoxOutline = gtk_event_box_new ();
	gtk_widget_show (mainEventBoxOutline);
	gtk_container_add (GTK_CONTAINER (mainFrameOutline), mainEventBoxOutline);

	mDrawingAreaOutline = gtk_drawing_area_new ();
	gtk_widget_show (mDrawingAreaOutline);
	gtk_container_add (GTK_CONTAINER (mainEventBoxOutline), mDrawingAreaOutline);

	gtk_drawing_area_size(GTK_DRAWING_AREA(mDrawingAreaOutline), IMAGE_WIDTH/4, IMAGE_HEIGHT/4);

	// now create a framed view for the fin DATA and put it in the notebook
	// as page number 4
	GtkWidget *dataFrame = gtk_frame_new (_("Fin Data"));
	gtk_container_set_border_width(GTK_CONTAINER(dataFrame), 3);
	gtk_frame_set_label_align(GTK_FRAME(dataFrame), 1.0, 0.0);
	gtk_widget_show (dataFrame);
	tempLabel = gtk_label_new(_("Fin Data"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), dataFrame, tempLabel);
	// connect data table to this new frame
	gtk_container_add (GTK_CONTAINER (dataFrame), mainInfoTable);

	GtkWidget *hbuttonbox2 = gtk_hbutton_box_new();
	gtk_widget_show(hbuttonbox2);
	gtk_container_add(GTK_CONTAINER(mainRightVBox), hbuttonbox2);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox2), GTK_BUTTONBOX_SPREAD);
	
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), mButtonModify);


	// FINALLY, at bottom of main window is a status bar
	
	mStatusBar = gtk_statusbar_new ();
	gtk_widget_show(mStatusBar);

	mContextID = gtk_statusbar_get_context_id(GTK_STATUSBAR(mStatusBar), "mainWin");

	gtk_box_pack_start(GTK_BOX(mainVBox), mStatusBar, FALSE, FALSE, 0);

	gtk_signal_connect (GTK_OBJECT (mainWindow), "delete_event",
	                    GTK_SIGNAL_FUNC (on_mainWindow_delete_event),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (open), "activate",
	                    GTK_SIGNAL_FUNC (on_open_activate),
	                    (void *) this);
	//***1.4 - new callback for opening previously saved fin tracing
	gtk_signal_connect (GTK_OBJECT (openTFin), "activate",
	                    GTK_SIGNAL_FUNC (on_open_fin_trace_activate),
	                    (void *) this);
	//***1.85 - new callback for opening a database
	gtk_signal_connect (GTK_OBJECT (opendb), "activate",
	                    GTK_SIGNAL_FUNC (on_open_database_activate),
	                    (void *) this);
	//***1.85 - new callback for creating a database
	gtk_signal_connect (GTK_OBJECT (newdb), "activate",
	                    GTK_SIGNAL_FUNC (on_new_database_activate),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (matching_queue), "activate",
	                    GTK_SIGNAL_FUNC (on_matching_queue_activate),
	                    (void *) this);
	//***1.85 - new callbacks for backing up or exporting a database
	gtk_signal_connect (GTK_OBJECT (backup), "activate",
	                    GTK_SIGNAL_FUNC (on_backup_database_activate),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mExportDBMenuItem), "activate",
	                    GTK_SIGNAL_FUNC (on_export_database_activate),
	                    (void *) this);
	//***1.99 - callback for exporting a Fin
	gtk_signal_connect (GTK_OBJECT (mExportFinzMenuItem), "activate",
	                    GTK_SIGNAL_FUNC (on_export_finz_activate),
	                    (void *) this);
	//***2.02 - callback for exporting Full-Size Modified Images
	gtk_signal_connect (GTK_OBJECT (mExportFullSzImgsMenuItem), "activate",
	                    GTK_SIGNAL_FUNC (on_export_fullSzImgs_activate),
	                    (void *) this);

	//***1.99 - callback for importing Fin
	gtk_signal_connect (GTK_OBJECT (mImportFinzMenuItem), "activate",
	                    GTK_SIGNAL_FUNC (on_import_finz_activate),
	                    (void *) this);


	//***1.85 - new callbacks for restoring or importing a database
	gtk_signal_connect (GTK_OBJECT (restore), "activate",
	                    GTK_SIGNAL_FUNC (on_restore_database_activate),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (importDB), "activate",
	                    GTK_SIGNAL_FUNC (on_import_database_activate),
	                    (void *) this);

	gtk_signal_connect (GTK_OBJECT (exit), "activate",
	                    GTK_SIGNAL_FUNC (on_exit_activate),
	                    (void *) this);

	//***1.4 - three new Catalog Scheme related callbacks
	gtk_signal_connect (GTK_OBJECT (catalog_new), "activate",
	                    GTK_SIGNAL_FUNC (on_catalog_new_activate),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (catalog_view), "activate",
	                    GTK_SIGNAL_FUNC (on_catalog_view_activate),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (catalog_select), "activate",
	                    GTK_SIGNAL_FUNC (on_catalog_select_activate),
	                    (void *) this);

	//***1.9 - callback for data export in spreadsheet format
	gtk_signal_connect (GTK_OBJECT (exportData), "activate",
	                    GTK_SIGNAL_FUNC (on_exportData_select_activate),
	                    (void *) this);

	gtk_signal_connect (GTK_OBJECT (options), "activate",
	                    GTK_SIGNAL_FUNC (on_options_activate),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (about), "activate",
	                    GTK_SIGNAL_FUNC (on_about_activate),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (docs), "activate",
	                    GTK_SIGNAL_FUNC (on_docs_activate),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mainButtonOpen), "clicked",
	                    GTK_SIGNAL_FUNC (on_mainButtonOpen_clicked),
	                    (void *) this);
	//***1.85 -new callback
	gtk_signal_connect (GTK_OBJECT (findNow), "clicked",
	                    GTK_SIGNAL_FUNC (on_mainButtonFindNow_clicked),
	                    (void *) this);
	//***1.5 - new callback
	gtk_signal_connect (GTK_OBJECT (mainButtonOpenTrace), "clicked",
	                    GTK_SIGNAL_FUNC (on_mainButtonOpenTrace_clicked),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mainButtonMatchingQueue), "clicked",
	                    GTK_SIGNAL_FUNC (on_mainButtonMatchingQueue_clicked),
	                    (void *) this);
	//gtk_signal_connect (GTK_OBJECT (mainButtonOptions), "clicked",
	//                    GTK_SIGNAL_FUNC (on_mainButtonOptions_clicked),
	//                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mainButtonExit), "clicked",
	                    GTK_SIGNAL_FUNC (on_mainButtonExit_clicked),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mCList), "click_column",
	                    GTK_SIGNAL_FUNC (on_mainCList_click_column),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mCList), "select_row",
	                    GTK_SIGNAL_FUNC (on_mainCList_select_row),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mButtonPrev), "clicked",
	                    GTK_SIGNAL_FUNC (on_mainButtonPrev_clicked),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mButtonNext), "clicked",
	                    GTK_SIGNAL_FUNC (on_mainButtonNext_clicked),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mButtonModify), "clicked",	//***002DB
	                    GTK_SIGNAL_FUNC (on_mainButtonModify_clicked),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mainEventBoxImage), "button_press_event",
	                    GTK_SIGNAL_FUNC (on_mainEventBoxImage_button_press_event),
	                    (void *) this);
	//***1.99 - new callback for popup viewer containing Original Image
	gtk_signal_connect (GTK_OBJECT (mainEventBoxOrigImage), "button_press_event",
	                    GTK_SIGNAL_FUNC (on_mainEventBoxOrigImage_button_press_event),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mDrawingAreaImage), "expose_event",
	                    GTK_SIGNAL_FUNC (on_mainDrawingAreaImage_expose_event),
	                    (void *) this);
	//***1.99 - new callback for exposure of Original image
	gtk_signal_connect (GTK_OBJECT (mDrawingAreaOrigImage), "expose_event",
	                    GTK_SIGNAL_FUNC (on_mainDrawingAreaOrigImage_expose_event),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mainEventBoxOutline), "button_press_event",
	                    GTK_SIGNAL_FUNC (on_mainEventBoxOutline_button_press_event),
	                    (void *) this);
	gtk_signal_connect (GTK_OBJECT (mDrawingAreaOutline), "expose_event",
	                    GTK_SIGNAL_FUNC (on_mainDrawingAreaOutline_expose_event),
	                    (void *) this);

	gtk_signal_connect (GTK_OBJECT(mDrawingAreaImage), "configure_event",
	                    GTK_SIGNAL_FUNC(on_mainDrawingAreaImage_configure_event),
	                    (void *) this);
	//***1.99 - new callback for showing original image
	gtk_signal_connect (GTK_OBJECT(mDrawingAreaOrigImage), "configure_event",
	                    GTK_SIGNAL_FUNC(on_mainDrawingAreaOrigImage_configure_event),
	                    (void *) this);

	gtk_signal_connect (GTK_OBJECT(mDrawingAreaOutline), "configure_event",
	                    GTK_SIGNAL_FUNC(on_mainDrawingAreaOutline_configure_event),
	                    (void *) this);

	gtk_widget_grab_default (mButtonNext);
	gtk_object_set_data (GTK_OBJECT (mainWindow), "tooltips", tooltips);

	gtk_window_add_accel_group (GTK_WINDOW (mainWindow), accel_group);

	return mainWindow;
}

//*******************************************************************
gboolean on_mainWindow_delete_event(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return FALSE;

	delete mainWin;

	return TRUE;
}

//*******************************************************************
void on_open_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	if (getNumOpenFileChooserDialogReferences() < 1) {
		OpenFileChooserDialog *dlg = new OpenFileChooserDialog(
				mainWin->mDatabase,
				mainWin,
				mainWin->mOptions,
				OpenFileChooserDialog::openFinImage);
		dlg->run_and_respond();
	}
}

//*******************************************************************
//***1.4 - new
//
void on_open_fin_trace_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	if (getNumOpenFileChooserDialogReferences() < 1) {
		OpenFileChooserDialog *dlg = new OpenFileChooserDialog(
				mainWin->mDatabase,
				mainWin,
				mainWin->mOptions,
				OpenFileChooserDialog::openFinTrace);
		dlg->run_and_respond();
	}
}
//*******************************************************************
//***1.85 - new
//
void on_open_database_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	if (getNumOpenFileChooserDialogReferences() < 1) {
		OpenFileChooserDialog *dlg = new OpenFileChooserDialog(
				mainWin->mDatabase,
				mainWin,
				mainWin->mOptions,
				OpenFileChooserDialog::openDatabase);
		dlg->run_and_respond();
	}
		
	// reset the database filename for the window
	// set sensitivity of menu items and buttons depending on success
	// and create an emergency backup of DB file, as long as it was successfully loaded

	mainWin->resetTitleButtonsAndBackupOnDBLoad();
}

//*******************************************************************
//***1.85 - new
//
void on_new_database_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	CreateDatabaseDialog *dlg = new CreateDatabaseDialog(
				mainWin,
				mainWin->mOptions,
				""); // no archive being IMPORTED

	dlg->show();
}

//*******************************************************************
//***1.85 - new
//
void on_restore_database_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	// go find backup or previously exported archive
	// NOTE: difference between a backup and an export is whether or not the
	// additional folders (tracedFins, ...) are included

	OpenFileChooserDialog 
		*open = new OpenFileChooserDialog(
						mainWin->mDatabase,
						mainWin,
						mainWin->mOptions,
						OpenFileChooserDialog::restoreDatabase);

	open->run_and_respond();

	// ALL OF THE FOLLOWING ...
	// is done in OpenFileChooserDialog::on_fileChooserButtonOK_clicked()
	//
	// determine which survey area it goes into
	// NOTE: this is NOT necessary for restore but is for import
	// build file structure, if needed
	// NOTE: again, not necessary for restore, but is for import
	// extract files into catalog folder
	// default action is to "NOT overwrite" existing files and to "NOT duplicate"
	// existing files 

	//***1.982 - we assume new database was opened after restore so ...
	// reset the database filename for the window
	// set sensitivity of menu items and buttons depending on success
	// and create an emergency backup of DB file, as long as it was successfully loaded
	// NOTE: the "keep_above" calls are to keep MainWindow on top of Console Window
	// which for some reason it wants to hide behind here

	gtk_window_set_keep_above(GTK_WINDOW(mainWin->getWindow()), TRUE);
	mainWin->resetTitleButtonsAndBackupOnDBLoad();
	gtk_window_set_keep_above(GTK_WINDOW(mainWin->getWindow()), FALSE);
}

//*******************************************************************
//***1.85 - new
//
void on_import_database_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	// go find backup or previously exported archive
	// NOTE: difference between a backup and an export is whether or not the
	// additional folders (tracedFins, ...) are included

	mainWin->mImportFromFilename = "";

	OpenFileChooserDialog 
		*open = new OpenFileChooserDialog(
						mainWin->mDatabase,
						mainWin,
						mainWin->mOptions,
						OpenFileChooserDialog::importDatabase);

	open->run_and_respond();

	// if no filename was acqured, then import was cancelled
	if ("" == mainWin->mImportFromFilename)
		return;

	// otherwise, continue with import process
	CreateDatabaseDialog *dlg = new CreateDatabaseDialog(
				mainWin,
				mainWin->mOptions,
				mainWin->mImportFromFilename); // name of archive being IMPORTED

	dlg->show();

	// ALL OF THE FOLLOWING ...
	// is done in OpenFileChooserDialog::on_fileChooserButtonOK_clicked()
	//
	// determine which survey area it goes into
	// NOTE: this is NOT necessary for restore but is for import
	// build file structure, if needed
	// NOTE: again, not necessary for restore, but is for import
	// extract files into catalog folder
	// default action is to "NOT overwrite" existing files and to "NOT duplicate"
	// existing files 

	//***1.982 - we assume new database was opened after restore so ...
	// reset the database filename for the window
	// set sensitivity of menu items and buttons depending on success
	// and create an emergency backup of DB file, as long as it was successfully loaded
	// NOTE: the "keep_above" calls are to keep MainWindow on top of Console Window
	// which for some reason it wants to hide behind here

	gtk_window_set_keep_above(GTK_WINDOW(mainWin->getWindow()), TRUE);
	mainWin->resetTitleButtonsAndBackupOnDBLoad();
	gtk_window_set_keep_above(GTK_WINDOW(mainWin->getWindow()), FALSE);

}

//*******************************************************************
//***1.85 - new
//
void on_backup_database_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	// let user know what is happening and then close iconify main window so user
	// can see backup progress in console window
	//***1.93 - begin new code segment
	GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(mainWin->getWindow()),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_INFO,
                                  GTK_BUTTONS_OK,
                                  "BACKUP starting ... \nDetails will appear in CONSOLE window.");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	gtk_window_iconify(GTK_WINDOW(mainWin->getWindow())); //***1.93
	//***1.93 - end new code segment

	if (! backupCatalog(mainWin->mDatabase))
	{
		ErrorDialog *err = new ErrorDialog("Database backup failed.");
		err->show();
	}

	// must check here to ensure database reopened correctly
	if (! mainWin->mDatabase->isOpen())
	{
		ErrorDialog *err = new ErrorDialog("Database failed to reopen");
		err->show();
	}

	// reopen main window in front of the console window
	gtk_window_set_keep_above(GTK_WINDOW(mainWin->getWindow()), TRUE); //***1.93
	gtk_window_deiconify(GTK_WINDOW(mainWin->getWindow())); //***1.93
	gtk_window_set_keep_above(GTK_WINDOW(mainWin->getWindow()), FALSE); //***1.93

}

//*******************************************************************
//***1.99 - new
//
void on_export_finz_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	//Save the currently selected fin for the moment (should show dialog allowing for multiple fin selection (or all fins))
	MainWindow *mainWin = (MainWindow *)userData;
	/*CatalogSupport*///saveFinz(mainWin->mSelectedFin,"test.finz");
	ExportFinzDialog *dlg = new ExportFinzDialog(
			mainWin->mDatabase,
			mainWin->mWindow,
			mainWin->mCList,
			mainWin->mNewSort,
			mainWin->mRow2Id,
			mainWin->mId2Row,
			ExportFinzDialog::exportFinz); //2.02 - set operating mode
	dlg->show();
}

//*******************************************************************
//***2.02 - new
//
void on_export_fullSzImgs_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	// Save the modified image of the currently selected fin FULL_SIZED

	MainWindow *mainWin = (MainWindow *)userData;

	/*
	// get filename for modified image (including path)
	string imageSaveFileName = mainWin->mSelectedFin->mImageFilename;
	// strip extension and append "_fullSize.png" 
	imageSaveFileName = imageSaveFileName.substr(0,imageSaveFileName.rfind("."));
	imageSaveFileName += "_fullSize.png";
	
	// save full-sized version in catalog folder alongside thumbnail-only version
	mainWin->mImageFullsize->save(imageSaveFileName);
	*/

	ExportFinzDialog *dlg = new ExportFinzDialog(

			mainWin->mDatabase,
			mainWin->mWindow,
			mainWin->mCList,
			mainWin->mNewSort,
			mainWin->mRow2Id,
			mainWin->mId2Row,
			ExportFinzDialog::exportFullSizeModImages); //2.02 - set operating mode
	dlg->show();
}


//*******************************************************************
//***1.99 - new
//
void on_import_finz_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;
	if (NULL == mainWin)
		return;
	
	OpenFileChooserDialog *dlg = new OpenFileChooserDialog(
				mainWin->mDatabase,
				mainWin,
				mainWin->mOptions,
				OpenFileChooserDialog::directlyImportFinz);
	dlg->run_and_respond();
	

	mainWin->refreshDatabaseDisplayNew(true);
	gtk_clist_select_row(GTK_CLIST(mainWin->mCList), 0, 0); //Select first menu item

}


//*******************************************************************
//***1.85 - new
//
void on_export_database_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	cout << "\nEXPORTING Database ...\n  " << mainWin->mOptions->mDatabaseFileName << endl;

	mainWin->mExportToFilename = "";

	// let user decide where to put the EXPORT - now uses SaveFileChooserDialog

	SaveFileChooserDialog 
		*save = new SaveFileChooserDialog(
						mainWin->mDatabase,
						NULL, // no DatabaseFin to pass
						mainWin,
						NULL, // no TraceWindow involved
						mainWin->mOptions,
						mainWin->mWindow,
						SaveFileChooserDialog::exportDatabase);

	save->run_and_respond();

	// now mainWin->mExportToFilename is either "" or it contains the name
	// of a file into which the archive will be written

	if (mainWin->mExportToFilename == "")
	{
		cout << "EXPORT aborted - User cancel, or NO filename specified!" << endl;
		return;
	}
		
	if (! exportCatalogTo(mainWin->mDatabase, mainWin->mOptions, mainWin->mExportToFilename))
	{
		ErrorDialog *err = new ErrorDialog("Catalog EXPORT failed.");
		err->show();
	}

	if (! mainWin->mDatabase->isOpen())
	{
		ErrorDialog *err = new ErrorDialog("Catalog failed to reopen");
		err->show();
	}
}

//*******************************************************************
void on_matching_queue_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	//***1.0 -- swap commented / uncommented code below as needed
	//       for production / testing versions

	//***1.1 - let's get matching queue to work - JHS
	//showError("The Matching Queue option\nis not available in this\nsoftware version!"); //***054
	
	if (getNumMatchingQueueDialogReferences() < 1) {
		MatchingQueueDialog *dlg =
			new MatchingQueueDialog(mainWin, mainWin->mDatabase,mainWin->mOptions);
		dlg->show();
	}
}

//*******************************************************************
void on_exit_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	delete mainWin;
}

//*******************************************************************
void on_options_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	if (getNumOptionsDialogReferences() > 0)
		return;

	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;
	
	OptionsDialog *dlg = new OptionsDialog(mainWin, mainWin->mOptions);
	dlg->show();
}

//*******************************************************************
//
//***1.4 - new function
//
void on_catalog_new_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	if (getNumCatalogSchemeDialogReferences() > 0)
		return;

	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;
	
	CatalogSchemeDialog *dlg = new CatalogSchemeDialog(
			mainWin->getWindow(),
			mainWin->mOptions,
			CatalogSchemeDialog::defineNewScheme);
	dlg->show();
}

//*******************************************************************
//
//***1.4 - new function
//
void on_catalog_view_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	if (getNumCatalogSchemeDialogReferences() > 0)
		return;

	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;
	
	CatalogSchemeDialog *dlg = new CatalogSchemeDialog(
			mainWin->getWindow(),
			mainWin->mOptions,
			CatalogSchemeDialog::viewExistingSchemes);
	dlg->show();
}

//*******************************************************************
//
//***1.4 - new function
//
void on_catalog_select_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	if (getNumCatalogSchemeDialogReferences() > 0)
		return;

	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;
	
	CatalogSchemeDialog *dlg = new CatalogSchemeDialog(
			mainWin->getWindow(),
			mainWin->mOptions,
			CatalogSchemeDialog::setDefaultScheme);
	dlg->show();
}

//*******************************************************************
//***1.9 - new callback
//
void on_exportData_select_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	if (getNumDataExportDialogReferences() > 0)
		return;

	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;
	
	DataExportDialog *dlg = new DataExportDialog(
			mainWin->mDatabase,
			mainWin,
			mainWin->mOptions,
			DataExportDialog::saveToFile);

	dlg->run_and_respond();
}

//*******************************************************************
void on_about_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	if (getNumAboutDialogReferences() < 1) {
		AboutDialog *about = new AboutDialog(mainWin->mWindow);
		about->show();
	}
}

//*******************************************************************
void on_docs_activate(
	GtkMenuItem *menuitem,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	
	//SAH 2008-07-18
	string cmd;
#ifdef WIN32
	cmd = "explorer.exe ";
	//system("explorer.exe %DARWINHOME%\\docs\\usersguide.htm&");
#else
	cmd = "firefox ";
	//system("firefox file:$DARWINHOME/docs/usersguide.htm&");
#endif
	cmd += "\"" + gOptions->mDarwinHome + PATH_SLASH + "docs" + PATH_SLASH + "usersguide.htm\" &";
	system(cmd.c_str());
}

//*******************************************************************
void on_mainButtonOpen_clicked(
	GtkButton *button,
	gpointer userData)
{
	on_open_activate(NULL, userData);
}

//*******************************************************************
void on_mainButtonOpenTrace_clicked(
	GtkButton *button,
	gpointer userData)
{
	on_open_fin_trace_activate(NULL, userData);
}

//*******************************************************************
void on_mainButtonMatchingQueue_clicked(
	GtkButton *button,
	gpointer userData)
{
	on_matching_queue_activate(NULL, userData);
}

//*******************************************************************
void on_mainButtonOptions_clicked(
	GtkButton *button,
	gpointer userData)
{
	on_options_activate(NULL, userData);
}

//*******************************************************************
void on_mainButtonExit_clicked(
	GtkButton *button,
	gpointer userData)
{
	on_exit_activate(NULL, userData);
}

//*******************************************************************
//***1.85 - new
//
void on_mainButtonFindNow_clicked(
	GtkButton *button,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	string idString = gtk_entry_get_text(GTK_ENTRY(mainWin->mSearchID));

	// find in the ID list
	int posit = mainWin->mDatabase->getIDListPosit(idString);

	if (NOT_IN_LIST == posit)
		return;

	// grab the ID : Offset pair out of the ID list
	string idPlusOffset = mainWin->mDatabase->getItemEntryFromList(DB_SORT_ID, posit);

	// find where the offset is in the CURRENT sort of the CCList
	posit = mainWin->mDatabase->getItemListPosFromOffset(
		mainWin->mNewSort,
		idPlusOffset);

	if (NOT_IN_LIST == posit)
		return;

	// and finally go there
	gtk_clist_select_row(GTK_CLIST(mainWin->mCList), posit, 0);
}

//*******************************************************************
void on_mainCList_click_column(
	GtkCList *clist,
	gint column,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	if (NULL == mainWin->mDatabase)
		return;
	
	if (0 == mainWin->mDatabase->size()) //***1.85 - don't sort if empty
		return;

	//mainWin->mOldSort = mainWin->mNewSort; //***1.85 - remember this

	switch (column) {
		case 1:
			mainWin->mDatabase->sort(DB_SORT_ID);
			mainWin->mNewSort = DB_SORT_ID; //***1.85
			break;
		case 2:
			mainWin->mDatabase->sort(DB_SORT_NAME);
			mainWin->mNewSort = DB_SORT_NAME; //***1.8
			break;
		case 3:
			mainWin->mDatabase->sort(DB_SORT_DAMAGE);
			mainWin->mNewSort = DB_SORT_DAMAGE; //***1.8
			break;
		case 4:
			mainWin->mDatabase->sort(DB_SORT_DATE);
			mainWin->mNewSort = DB_SORT_DATE; //***1.8
			break;
		case 5:
			mainWin->mDatabase->sort(DB_SORT_LOCATION);
			mainWin->mNewSort = DB_SORT_LOCATION; //***1.8
			break;
	}
	
	gtk_clist_freeze(GTK_CLIST(mainWin->mCList)); //***1.85

	mainWin->refreshDatabaseDisplayNew(false); //***1.85 - just a sort, no change in # of fins

	// make sure CList repositions itself for previously selected item to appear
	string selectedIdPlusOffset = mainWin->mDatabase->getItemEntryFromList(
				mainWin->mOldSort,
				mainWin->mDBCurEntry);
	mainWin->selectFromReorderedCListNew(selectedIdPlusOffset);

	mainWin->mOldSort = mainWin->mNewSort; //***1.96a - moved from above

	gtk_clist_thaw(GTK_CLIST(mainWin->mCList)); //***1.85

	//mainWin->selectFromReorderedCList(mainWin->mSelectedFin->mImageFilename); //***004CL
}

//*******************************************************************
void on_mainCList_select_row(
	GtkCList *clist,
	gint row,
	gint column,
	GdkEvent *event,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	delete mainWin->mSelectedFin;

	try {
		if (row == 0)
			gtk_widget_set_sensitive(mainWin->mButtonPrev, FALSE);
		else
			gtk_widget_set_sensitive(mainWin->mButtonPrev, TRUE);

		//if (row == (int)mainWin->mDatabase->size() - 1)
		if (row == mainWin->mRow2Id.size() - 1) //***1.96
			gtk_widget_set_sensitive(mainWin->mButtonNext, FALSE);
		else
			gtk_widget_set_sensitive(mainWin->mButtonNext, TRUE);

		//mainWin->mSelectedFin = mainWin->mDatabase->getItem(row);
		mainWin->mSelectedFin = mainWin->mDatabase->getItem(mainWin->mRow2Id[row]); //***1.95

		if (NULL == mainWin->mSelectedFin)
			throw Error("Internal Error:\nProblem retrieving fin from database.");

		gchar 
			*idcode,
			*name,
			*date,
			*roll,
			*location,
			*damage,
			*shortdescription;

		DatabaseFin<ColorImage> *t = mainWin->mSelectedFin;

		if (t->mIDCode == "NONE") {
			idcode = new gchar[strlen(NONE_SUBSTITUTE_STRING) + 1];
			strcpy(idcode, NONE_SUBSTITUTE_STRING);
			gtk_widget_set_sensitive(mainWin->mEntryID, FALSE);
		} else {
			idcode = new gchar[t->mIDCode.length() + 1];
			strcpy(idcode, t->mIDCode.c_str());
			gtk_widget_set_sensitive(mainWin->mEntryID, TRUE);
		}

		if (t->mName == "NONE") {
			name = new gchar[strlen(NONE_SUBSTITUTE_STRING) + 1];
			strcpy(name, NONE_SUBSTITUTE_STRING);
			gtk_widget_set_sensitive(mainWin->mEntryName, FALSE);
		} else {
			name = new gchar[t->mName.length() + 1];
			strcpy(name, t->mName.c_str());
			gtk_widget_set_sensitive(mainWin->mEntryName, TRUE);
		}
		
		if (t->mDateOfSighting == "NONE") {
			date = new gchar[strlen(NONE_SUBSTITUTE_STRING) + 1];
			strcpy(date, NONE_SUBSTITUTE_STRING);
			gtk_widget_set_sensitive(mainWin->mEntryDate, FALSE);
		} else {
			date = new gchar[t->mDateOfSighting.length() + 1];
			strcpy(date, t->mDateOfSighting.c_str());
			gtk_widget_set_sensitive(mainWin->mEntryDate, TRUE);
		}

		if (t->mRollAndFrame == "NONE") {
			roll = new gchar[strlen(NONE_SUBSTITUTE_STRING) + 1];
			strcpy(roll, NONE_SUBSTITUTE_STRING);
			gtk_widget_set_sensitive(mainWin->mEntryRoll, FALSE);
		} else {
			roll = new gchar[t->mRollAndFrame.length() + 1];
			strcpy(roll, t->mRollAndFrame.c_str());
			gtk_widget_set_sensitive(mainWin->mEntryRoll, TRUE);
		}

		if (t->mLocationCode == "NONE") {
			location = new gchar[strlen(NONE_SUBSTITUTE_STRING) + 1];
			strcpy(location, NONE_SUBSTITUTE_STRING);
			gtk_widget_set_sensitive(mainWin->mEntryLocation, FALSE);
		} else {
			location = new gchar[t->mLocationCode.length() + 1];
			strcpy(location, t->mLocationCode.c_str());
			gtk_widget_set_sensitive(mainWin->mEntryLocation, TRUE);
		}

		if (t->mDamageCategory == "NONE") {
			//damage = new gchar[strlen(NONE_SUBSTITUTE_STRING) + 1];
			//strcpy(damage, NONE_SUBSTITUTE_STRING);
			//***055DB - change fo category default (display as "Unspecified"
			damage = new gchar[12];
			strcpy(damage, _("Unspecified"));
			gtk_widget_set_sensitive(mainWin->mEntryDamage, /*FALSE*/ TRUE);
		} else {
			damage = new gchar[t->mDamageCategory.length() + 1];
			strcpy(damage, t->mDamageCategory.c_str());
			gtk_widget_set_sensitive(mainWin->mEntryDamage, TRUE);
		}

		if (t->mShortDescription == "NONE") {
			shortdescription = new gchar[strlen(NONE_SUBSTITUTE_STRING) + 1];
			strcpy(shortdescription, NONE_SUBSTITUTE_STRING);
			gtk_widget_set_sensitive(mainWin->mEntryDescription, FALSE);
		} else {
			shortdescription = new gchar[t->mShortDescription.length() + 1];
			strcpy(shortdescription, t->mShortDescription.c_str());
			gtk_widget_set_sensitive(mainWin->mEntryDescription, TRUE);
		}
		//***1.65 - show or hide ID as setting indicates
		if (mainWin->mOptions->mHideIDs)
			gtk_entry_set_text(GTK_ENTRY(mainWin->mEntryID), "****");
		else
			gtk_entry_set_text(GTK_ENTRY(mainWin->mEntryID), idcode);
		gtk_entry_set_text(GTK_ENTRY(mainWin->mEntryName), name);
		gtk_entry_set_text(GTK_ENTRY(mainWin->mEntryDate), date);
		gtk_entry_set_text(GTK_ENTRY(mainWin->mEntryRoll), roll);
		gtk_entry_set_text(GTK_ENTRY(mainWin->mEntryLocation), location);
		gtk_entry_set_text(GTK_ENTRY(mainWin->mEntryDamage), damage);
		gtk_entry_set_text(GTK_ENTRY(mainWin->mEntryDescription), shortdescription);

		gtk_frame_set_label(GTK_FRAME(mainWin->mFrameMod), idcode);
		gtk_frame_set_label(GTK_FRAME(mainWin->mFrameOrig), idcode);

		delete[] idcode;
		delete[] name;
		delete[] date;
		delete[] roll;
		delete[] location;
		delete[] damage;
		delete[] shortdescription;
	
		mainWin->mDBCurEntry = row;

		// this is a hack to ensure a configure event
		// for the drawing area every time a new row is selected
		//gtk_drawing_area_size(
				//GTK_DRAWING_AREA(mainWin->mDrawingAreaImage),
				////mainWin->mCurImageWidth,
				////mainWin->mCurImageHeight);    
				//mainWin->mDrawingAreaImage->allocation.width, //***1.7
				//mainWin->mDrawingAreaImage->allocation.height); //***1.7

		//***1.96a - we want to use the mDataPos offsets to prevent
		// reloading the same fin (and rebuilding its modified image) unless
		// a real change of selected fin has occurred
		if (mainWin->mDBCurEntryOffset != mainWin->mSelectedFin->mDataPos)
		{
			// newly selected fin IS NOT the same fin as was previously selected
			// so remember offset and force reloading of fin image
			if (mainWin->mWindow->window != NULL) //***1.85
			{
				on_mainDrawingAreaImage_configure_event(mainWin->mDrawingAreaImage,NULL,userData); //***1.7
				on_mainDrawingAreaOrigImage_configure_event(mainWin->mDrawingAreaOrigImage,NULL,userData); //***1.7
				// configure call above reloads image, so remember the offset of the fin
				// with the currently loaded image, but do so AFTER the configure event
			}
			mainWin->mDBCurEntryOffset = mainWin->mSelectedFin->mDataPos; //***1.96a
		}

	} catch (Error e) {
		showError(e.errorString());
	}
	
	mainWin->refreshImage();
	mainWin->refreshOrigImage(); //***1.99
	mainWin->refreshOutline();
	// the following now ONLY updates the scrolling list to display correctly
	mainWin->selectFromCList(mainWin->mDBCurEntry); //***1.7
}

//*******************************************************************
//***1.95 - so main clist can show ALL images or ONLY primary images
//
void on_mainButtonShowAllImages_toggled(
	GtkButton *button,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	// change state variable
	mainWin->mShowAlternates = (! mainWin->mShowAlternates);

	if (NULL == mainWin->mDatabase)
		return;

	// now redisplay Clist, if it exists

	if (NULL == mainWin->mCList)
		return;

	//***1.9 - new approach to reselect
	gtk_clist_freeze(GTK_CLIST(mainWin->mCList));

	int curId = mainWin->mRow2Id[mainWin->mDBCurEntry]; //***1.96 - save current DB Fin ID

	mainWin->refreshDatabaseDisplayNew(true); // force to read DB again & rebuild list

	int newCurRow = mainWin->mId2Row[curId]; //***1.96 - recover DB Fin position in new list

	//***1.96 - some bounds checks
	if (newCurRow == -1) // previous selection is NOT in new list
		newCurRow = 0; // force selection back to fin 0

	//if(mainWin->mDatabase->size() > 0)
	if(mainWin->mRow2Id.size() > 0) //***1.96
	{
		gtk_clist_select_row(
					GTK_CLIST(mainWin->mCList), 
					newCurRow, 0); //***1.96 - keep same selection as before, if possible
	}
		
	gtk_clist_thaw(GTK_CLIST(mainWin->mCList));
	//***1.9 - end new approach
}

//*******************************************************************
void on_mainButtonPrev_clicked(
	GtkButton *button,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	if (NULL == mainWin->mDatabase)
		return;

	if (mainWin->mDBCurEntry > 0)
		gtk_clist_select_row(GTK_CLIST(mainWin->mCList), mainWin->mDBCurEntry - 1, 0);
		//mainWin->selectFromCList(mainWin->mDBCurEntry - 1); //***1.7CL
}

//*******************************************************************
void on_mainButtonNext_clicked(
	GtkButton *button,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return;

	if (NULL == mainWin->mDatabase)
		return;

	//if (mainWin->mDBCurEntry < (int)mainWin->mDatabase->size() - 1)
	if (mainWin->mDBCurEntry < mainWin->mRow2Id.size() - 1) //***1.96
		gtk_clist_select_row(GTK_CLIST(mainWin->mCList), mainWin->mDBCurEntry + 1, 0);
		//mainWin->selectFromCList(mainWin->mDBCurEntry + 1);//***1.7CL
}

//*******************************************************************
void on_mainButtonModify_clicked(		//***002DB - nf
	GtkButton *button,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;
	
	if (NULL == mainWin)
		return;

	if (NULL == mainWin->mDatabase)
		return;
	
	ModifyDatabaseWindow *ModWin= new ModifyDatabaseWindow(
				       mainWin->mDBCurEntry, 
				       mainWin,                  //***004CL
				       mainWin->mSelectedFin, 
				       mainWin->mDatabase,
					   mainWin->mOptions);
	ModWin->show();	
}

//*******************************************************************
gboolean on_mainEventBoxImage_button_press_event(
	GtkWidget *widget,
	GdkEventButton *event,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return FALSE;

	if (NULL == mainWin->mSelectedFin)
		return FALSE;

   // make sure the image exists before creating the ImageViewDialog
   //if (NULL == mainWin->mSelectedFin->mFinImage)
   //   mainWin->mSelectedFin->mFinImage  = new ColorImage(mainWin->mSelectedFin->mImageFilename);

	//ImageViewDialog *dlg = new ImageViewDialog(
	//		mainWin->mSelectedFin->mIDCode,
	//		mainWin->mSelectedFin->mFinImage);
	ImageViewDialog *dlg = new ImageViewDialog(
			mainWin->mSelectedFin->mIDCode,
			mainWin->mImageFullsize); //***2.01 - use new fullsize image already in memory
	dlg->show();

	return TRUE;
}

//*******************************************************************
//***1.99 - new
//
gboolean on_mainEventBoxOrigImage_button_press_event(
	GtkWidget *widget,
	GdkEventButton *event,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return FALSE;

	if (NULL == mainWin->mSelectedFin)
		return FALSE;

   // make sure the image exists before creating the ImageViewDialog
   //if (NULL == mainWin->mSelectedFin->mFinImage)
   //   mainWin->mSelectedFin->mFinImage  = new ColorImage(mainWin->mSelectedFin->mImageFilename);

	//ImageViewDialog *dlg = new ImageViewDialog(
	//		mainWin->mSelectedFin->mIDCode,
	//		mainWin->mSelectedFin->mFinImage); // was reloading and using modified image - OOPS
	ImageViewDialog *dlg = new ImageViewDialog(
			mainWin->mSelectedFin->mIDCode,
			mainWin->mOrigImageFullsize); //***2.01 - use new fullsize image already in memory
	dlg->show();

	return TRUE;
}

//*******************************************************************
gboolean on_mainDrawingAreaImage_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return FALSE;

	if(mainWin->mDatabase->size()==0){		//***002DB >
		gdk_draw_rectangle(
			mainWin->mDrawingAreaImage->window,
			mainWin->mDrawingAreaImage->style->bg_gc[GTK_STATE_NORMAL],
			TRUE,
			0,
			0,
			widget->allocation.width,
			widget->allocation.height);
	 	//mainWin->clearText();              //***1.85 unnecessary - now done in show() 
		//mainWin->refreshDatabaseDisplay(); //***1.85 unnecessary - now done in show()        
		return TRUE;
	}						//***002DB <

	//***1.85 - moved from above backgound redraw
	if (NULL == mainWin->mDrawingAreaImage || NULL == mainWin->mImage)
		return FALSE;

	gdk_draw_rgb_image(
		mainWin->mDrawingAreaImage->window,
		mainWin->mDrawingAreaImage->style->fg_gc[GTK_STATE_NORMAL],
		0, 0,
		mainWin->mImage->getNumCols(),
		mainWin->mImage->getNumRows(),
		GDK_RGB_DITHER_NONE,
		(guchar*)mainWin->mImage->getData(),
		mainWin->mImage->bytesPerPixel() * mainWin->mImage->getNumCols()
		);

	mainWin->updateCursor();
	return TRUE;
}

//*******************************************************************
gboolean on_mainDrawingAreaOrigImage_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return FALSE;

	if(mainWin->mDatabase->size()==0){		//***002DB >
		gdk_draw_rectangle(
			mainWin->mDrawingAreaOrigImage->window,
			mainWin->mDrawingAreaOrigImage->style->bg_gc[GTK_STATE_NORMAL],
			TRUE,
			0,
			0,
			widget->allocation.width,
			widget->allocation.height);
		return TRUE;
	}						//***002DB <

	//***1.85 - moved from above backgound redraw
	if (NULL == mainWin->mDrawingAreaOrigImage || NULL == mainWin->mOrigImage)
		return FALSE;

	gdk_draw_rgb_image(
		mainWin->mDrawingAreaOrigImage->window,
		mainWin->mDrawingAreaOrigImage->style->fg_gc[GTK_STATE_NORMAL],
		0, 0,
		mainWin->mOrigImage->getNumCols(),
		mainWin->mOrigImage->getNumRows(),
		GDK_RGB_DITHER_NONE,
		(guchar*)mainWin->mOrigImage->getData(),
		mainWin->mOrigImage->bytesPerPixel() * mainWin->mOrigImage->getNumCols()
		);

	mainWin->updateCursor();
	return TRUE;
}

//*******************************************************************
gboolean on_mainEventBoxOutline_button_press_event(
	GtkWidget *widget,
	GdkEventButton *event,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return FALSE;

	if (NULL == mainWin->mSelectedFin)
		return FALSE;

	if (getNumContourInfoDialogReferences() < 1) {
		ContourInfoDialog *dlg = new ContourInfoDialog(
				mainWin->mSelectedFin->mIDCode,
				//mainWin->mSelectedFin->mFinContour, removed 008OL
				mainWin->mSelectedFin->mFinOutline, //***008OL
				mainWin->mOptions->mCurrentColor);
		dlg->show();
	}

	return TRUE;
}

//*******************************************************************
gboolean on_mainDrawingAreaOutline_expose_event(
	GtkWidget *widget,
	GdkEventExpose *event,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return FALSE;

	if ((NULL == mainWin->mSelectedFin) && (mainWin->mDatabase->size() > 0)) //***1.85 - size constraint
		return FALSE;

	// Redraw the background
	
	gdk_draw_rectangle(
		mainWin->mDrawingAreaOutline->window,
		mainWin->mDrawingAreaOutline->style->bg_gc[GTK_STATE_NORMAL],
		TRUE,
		0,
		0,
		mainWin->mCurContourWidth,
		mainWin->mCurContourHeight);

	if(mainWin->mDatabase->size()==0)		//***002DB
		return TRUE;				//***002DB
	//Contour *c = mainWin->mSelectedFin->mFinContour; removed 008OL
	FloatContour *c = mainWin->mSelectedFin->mFinOutline->getFloatContour(); //***008OL

	int
		xMax = c->maxX(),
		yMax = c->maxY(),
		xMin = c->minX(),
		yMin = c->minY();

	int
		xRange = xMax - xMin,
		yRange = yMax - yMin;

	float
		heightRatio = mainWin->mCurContourHeight / ((float)yRange),
		widthRatio = mainWin->mCurContourWidth / ((float)xRange);

	float ratio;

	int xOffset = 0, yOffset = 0;
	
	if (heightRatio < widthRatio) {
		ratio = heightRatio;
		xOffset = (int) round((mainWin->mCurContourWidth - ratio * xRange) / 2);	
	} else {
		ratio = widthRatio;
		yOffset = (int) round((mainWin->mCurContourHeight - ratio * yRange) / 2);
	}

	unsigned numPoints = c->length();
		
	for (unsigned i = 0; i < numPoints; i++) {	
		int xCoord = (int) round(((*c)[i].x - xMin) * ratio + xOffset);
		int yCoord = (int) round(((*c)[i].y - yMin) * ratio + yOffset);
		
		gdk_draw_rectangle(
			mainWin->mDrawingAreaOutline->window,
			mainWin->mGC,
			TRUE,
			xCoord - POINT_SIZE / 2,
			yCoord - POINT_SIZE / 2,
			POINT_SIZE,
			POINT_SIZE);
	}

	mainWin->updateCursor();
	return TRUE;
}

//*******************************************************************
gboolean on_mainDrawingAreaOutline_configure_event(
	GtkWidget *widget,
	GdkEventConfigure *event,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return FALSE;

	mainWin->mCurContourHeight = widget->allocation.height;
	mainWin->mCurContourWidth = widget->allocation.width;

	mainWin->updateGC(); //***1.99 - in cae notebook page was hidden initially

	mainWin->refreshOutline();

	return TRUE;
}

//*******************************************************************
gboolean on_mainDrawingAreaImage_configure_event(
	GtkWidget *widget,
	GdkEventConfigure *event,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return FALSE;

	//mainWin->mCurImageHeight = widget->allocation.height;
	//mainWin->mCurImageWidth = widget->allocation.width;

	if (NULL == mainWin->mSelectedFin)
		return TRUE;

	//***1.96a - no change in image so no need to reconfigure (reload image)
	// second condition makes sure this was called by us on Clist row selection
	// and NOT by an actual window size/state change
	if ((mainWin->mSelectedFin->mDataPos == mainWin->mDBCurEntryOffset) &&
		(event == NULL))
		return TRUE;

	delete mainWin->mImage;

	// since resizeWithBorder creates a new image, we must delete this one after it is used
	ColorImage *tempImage = new ColorImage(mainWin->mSelectedFin->mImageFilename);

	if (NULL != mainWin->mImageFullsize) //***2.01
		delete mainWin->mImageFullsize;  //***2.01

	mainWin->mImage = resizeWithBorder(		
			tempImage,
			//mainWin->mCurImageHeight,
			//mainWin->mCurImageWidth);
			mainWin->mDrawingAreaImage->allocation.height, //***1.7
			mainWin->mDrawingAreaImage->allocation.width); //***1.7

	//***1.99 - since the resizeWithBorder DOES NOT preserve any image
	// attributes except the basi image pixel data, we must copy the info
	// over here -- this should be fixed in the image classes and their support
	// functions - JHS

	mainWin->mImage->mOriginalImageFilename = tempImage->mOriginalImageFilename;
	mainWin->mImage->mNormScale = tempImage->mNormScale; // this may not be usefull
	mainWin->mImage->mImageMods = tempImage->mImageMods;

	//***1.8 - if information available, set up FRAME label to indicate any image reversal
	gchar *frameLabel = new gchar[mainWin->mSelectedFin->mIDCode.length() + 12]; // leave room for " (reversed)"
	strcpy(frameLabel,mainWin->mSelectedFin->mIDCode.c_str());
	if (tempImage->mImageMods.imageIsReversed())
		strcat(frameLabel, " (reversed)");
	gtk_frame_set_label(GTK_FRAME(mainWin->mFrameMod), frameLabel);
	delete[] frameLabel;
	//***1.8 - end

#ifdef DEBUG
	cout << "Scale: " << tempImage->mNormScale << endl;

	//***1.9 - check validity of image mods read from file
	if (0 < tempImage->mImageMods.size())
	{
		ImageMod mod(ImageMod::IMG_none);

		ImageMod::ImageModType op;
		int v1, v2, v3, v4;

		tempImage->mImageMods.first(mod);
		mod.get(op, v1, v2, v3, v4);
		cout << "ImgMod: " << (int)op << " " << v1 << " " << v2 << " " << v3 << " " << v4 << endl;

		while (tempImage->mImageMods.next(mod))
		{
			mod.get(op, v1, v2, v3, v4);
			cout << "ImgMod: " << (int)op << " " << v1 << " " << v2 << " " << v3 << " " << v4 << endl;
		}
	}
#endif

	// delete here or memory leak results
	//delete tempImage;
	mainWin->mImageFullsize = tempImage; //***2.01 - deleted as needed above and in destructor now

	mainWin->refreshImage();

	return TRUE;
}

//*******************************************************************
gboolean on_mainDrawingAreaOrigImage_configure_event(
	GtkWidget *widget,
	GdkEventConfigure *event,
	gpointer userData)
{
	MainWindow *mainWin = (MainWindow *) userData;

	if (NULL == mainWin)
		return FALSE;

	if (NULL == mainWin->mSelectedFin)
		return TRUE;

	//***1.96a - no change in image so no need to reconfigure (reload image)
	// second condition makes sure this was called by us on Clist row selection
	// and NOT by an actual window size/state change
	if ((mainWin->mSelectedFin->mDataPos == mainWin->mDBCurEntryOffset) &&
		(event == NULL))
		return TRUE;

	// the only way we know the original image name is if the modifed image
	// is already loaded, so if not we bail
	if (NULL == mainWin->mImage)
		return FALSE;

	// if the original image name is not available, then mImage is not
	// a darwin modified image, so again we bail
	if ("" == mainWin->mImage->mOriginalImageFilename)
		return FALSE;

	// set the selected fin's original filename
	string path = mainWin->mSelectedFin->mImageFilename;
	path = path.substr(0,path.rfind(PATH_SLASH)+1); // don't strip slash
	mainWin->mSelectedFin->mOriginalImageFilename =
			path + 
			mainWin->mImage->mOriginalImageFilename; // short name

	cout << "ORIG: " << mainWin->mSelectedFin->mOriginalImageFilename << endl;

	delete mainWin->mOrigImage;

	// since resizeWithBorder creates a new image, we must delete this one after it is used
	ColorImage *tempImage = new ColorImage(mainWin->mSelectedFin->mOriginalImageFilename);

	if (NULL != mainWin->mOrigImageFullsize) //***2.01
		delete mainWin->mOrigImageFullsize;  //***2.01

	mainWin->mOrigImage = resizeWithBorder(		
			tempImage,
			mainWin->mDrawingAreaOrigImage->allocation.height, //***1.7
			mainWin->mDrawingAreaOrigImage->allocation.width); //***1.7

	//indicat that this is the original image
	gchar *frameLabel = new gchar[mainWin->mSelectedFin->mIDCode.length() + 12]; // leave room for " (reversed)"
	strcpy(frameLabel,mainWin->mSelectedFin->mIDCode.c_str());
	strcat(frameLabel, " (original)");
	gtk_frame_set_label(GTK_FRAME(mainWin->mFrameOrig), frameLabel);
	delete[] frameLabel;

	// delete here or memory leak results
	//delete tempImage;
	mainWin->mOrigImageFullsize = tempImage; //***2.01 - deleted as needed above and in destructor now

	mainWin->refreshOrigImage();

	return TRUE;
}

