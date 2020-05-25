//*******************************************************************
//   file: Database.h
//
// author: J H Stewman (7/8/2008)
//
// This ABSTRACT class is the parent for ALL present and future
// types of database implementations.  Current derived classes
// include ...
//    OldDatabase -- the flat file version
//    SQLiteDatabase -- the first SQL relational database version
//
//*******************************************************************

using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Database
{
    public enum DatabaseSortType
    {
        DB_SORT_NAME,
        DB_SORT_ID,
        DB_SORT_DATE,
        DB_SORT_ROLL,
        DB_SORT_LOCATION,
        DB_SORT_DAMAGE,
        DB_SORT_DESCRIPTION
    };

    public enum DatabaseStatusType
    {
        loaded = 0,
        fileNotFound,
        errorLoading,
        errorCreating,
        oldDBVersion
    };

    //******************************************************************
    // Function Definitions
    //******************************************************************
    public abstract class DarwinDatabase
    {
        //		public Database(Options o, CatalogScheme cat, bool createEmptyDB);

        public abstract void createEmptyDatabase(object o);

        public abstract long add(DatabaseFin data);
        public abstract void Delete(DatabaseFin Fin);

        public abstract DatabaseFin getItemAbsolute(uint pos);

        public abstract DatabaseFin getItem(uint pos);
        // virtual DatabaseFin<ColorImage>* getItemByName(std::string name) = 0;  

        public abstract void sort(DatabaseSortType sortBy);

        public abstract uint size();
        public abstract uint sizeAbsolute(); //***1.3 - size of absolute offset list
        public abstract bool isEmpty();

        public abstract DatabaseSortType currentSort(); //***1.85

        public abstract DatabaseStatusType status(); //***1.85
        public abstract int getIDListPosit(string id); //***1.85

        //***1.85 - new functions for processing lists IN MEMORY without file access

        public abstract string getItemEntryFromList(DatabaseSortType whichList, uint pos); //***1.85

        public abstract int getItemListPosFromOffset(DatabaseSortType whichList, string item); //***1.85

        public abstract string getFilename(); //***1.85

        public abstract bool openStream();
        public abstract bool closeStream();

        //***1.99 - new access functions for catalog scheme moved from Options
        public abstract string catCategoryName(int id);
        public abstract string catSchemeName();
        public abstract int catCategoryNamesMax();
        public abstract void appendCategoryName(string name);
        public abstract void setCatSchemeName(string name);
        public abstract void clearCatalogScheme();
        public abstract CatalogScheme catalogScheme();

        public bool dbOpen;

        protected DatabaseStatusType mDBStatus; //***1.85

        protected string mFilename;

        //***1.99 - the catalog scheme for this database (moved from Options)
        protected List<string> mCatCategoryNames;     // names of catalog categories
        protected string mCatSchemeName;       // name of catalog scheme

        protected long mFooterPos;
        protected long mDataSize;
        protected long mHeaderSize; //***054

        /*
         * These store strings of the format "value pos" where value holds 
         * whatever the list name refers to.  "NONE" is used for empty values.
         * pos originally referred to the offset in the catalogue file. Now,
         * pos refers to the id field of the Individuals table in the db.
         */
        protected List<string>
            mNameList,
            mIDList,
            mDateList,
            mRollList,
            mLocationList,
            mDamageList,
            mDescriptionList;

        //***1.3 - absolute file locations of all fins (even deleted holes)
        protected List<long> mAbsoluteOffset;

        protected DatabaseSortType mCurrentSort;

        //virtual DatabaseFin<ColorImage>* getItem(unsigned pos, std::vector<std::string>* theList) = 0;
    }
}
