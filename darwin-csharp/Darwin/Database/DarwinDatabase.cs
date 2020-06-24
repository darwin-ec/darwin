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

using Darwin.Matching;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Runtime.CompilerServices;
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

        public abstract void CreateEmptyDatabase(CatalogScheme catalogScheme);

        public abstract long Add(DatabaseFin data);
        public abstract void Update(DatabaseFin data);
        public abstract void UpdateIndividual(DatabaseFin data);
        public abstract void Delete(DatabaseFin fin);

        //private abstract void UpdateDBIndividual(DBIndividual individual);

        public abstract DatabaseFin GetFin(long id);
        public abstract List<DatabaseFin> GetAllFins();

        //public abstract DatabaseFin GetItemAbsolute(int pos);

        //public abstract DatabaseFin GetItem(int pos);
        // virtual DatabaseFin<ColorImage>* getItemByName(std::string name) = 0;  

        public void sort(DatabaseSortType sortBy)
        {
            mCurrentSort = sortBy;
        }

        //public int size()
        //{
        //    if (mNameList == null)
        //        return 0;

        //    return mNameList.Count;
        //}

        //public int SizeAbsolute() //***1.3 - size of absolute offset list
        //{
        //    if (mAbsoluteOffset == null)
        //        return 0;

        //    return mAbsoluteOffset.Count;
        //}

        public bool isEmpty()
        {
            return mNameList == null || mNameList.Count == 0;
        }


        public DatabaseSortType currentSort() //***1.85
        {
            return mCurrentSort;
        }

        public DatabaseStatusType status() //***1.85
        {
            return mDBStatus;
        }

        //***1.85 - new functions for processing lists IN MEMORY without file access

        public int getItemListPosFromOffset(DatabaseSortType whichList, string item) //***1.85
        {
            throw new NotImplementedException();
        }

        //public abstract bool openStream();
        //public abstract bool closeStream();

        //***1.99 - new access functions for catalog scheme moved from Options
        //public string catCategoryName(int id)
        //{
        //    string name = string.Empty;

        //    if (0 <= id && id < mCatCategoryNames.Count)
        //        name = mCatCategoryNames[id];

        //    return name;
        //}

        //public string catSchemeName()
        //{
        //    return mCatSchemeName;
        //}

        //public int catCategoryNamesMax()
        //{
        //    if (mCatCategoryNames == null)
        //        return 0;

        //    return mCatCategoryNames.Count;
        //}

        //public void appendCategoryName(string name)
        //{
        //    if (mCatCategoryNames == null)
        //        mCatCategoryNames = new List<string>();

        //    mCatCategoryNames.Add(name);
        //}

        //public void setCatSchemeName(string name)
        //{
        //    mCatSchemeName = name;
        //}

        //public void clearCatalogScheme()
        //{
        //    if (mCatCategoryNames == null)
        //        mCatCategoryNames = new List<string>();
        //    else
        //        mCatCategoryNames.Clear();
        //}

        //public CatalogScheme catalogScheme()
        //{
        //    return new CatalogScheme {
        //        SchemeName = mCatSchemeName,
        //        CategoryNames = mCatCategoryNames
        //    };
        //}

        public bool dbOpen;

        protected DatabaseStatusType mDBStatus; //***1.85

        public string Filename { get; set; }

        //***1.99 - the catalog scheme for this database (moved from Options)

        public abstract CatalogScheme CatalogScheme { get; }
        public abstract ObservableCollection<Category> Categories { get; }

        public abstract List<DatabaseFin> AllFins { get; }

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

        //*******************************************************************
        //
        // Returns item from list at given position
        //

        //public string getItemEntryFromList(DatabaseSortType whichList, int pos)
        //{
        //    if (pos > this.size())
        //        throw new ArgumentOutOfRangeException(nameof(pos));

        //    switch (whichList)
        //    {
        //        case DatabaseSortType.DB_SORT_NAME:
        //            return mNameList[pos];

        //        case DatabaseSortType.DB_SORT_ID:
        //            return mIDList[pos];

        //        case DatabaseSortType.DB_SORT_DATE:
        //            return mDateList[pos];

        //        case DatabaseSortType.DB_SORT_ROLL:
        //            return mRollList[pos];

        //        case DatabaseSortType.DB_SORT_LOCATION:
        //            return mLocationList[pos];

        //        case DatabaseSortType.DB_SORT_DAMAGE:
        //            return mDamageList[pos];

        //        case DatabaseSortType.DB_SORT_DESCRIPTION:
        //            return mDescriptionList[pos];

        //        default: // it's not a valid sort type
        //            return string.Empty;
        //    }
        //}

        //protected long listEntryToID(string entry)
        //{
        //    if (string.IsNullOrEmpty(entry))
        //        return 0;

        //    var split = entry.Split(' ');

        //    return Convert.ToInt64(split[split.Length - 1]);
        //}

        //*******************************************************************
        //***1.85 - returns position of id in mIDList as currently sorted
        //

        //public int getIDListPosit(string id)
        //{
        //    // NOTE: this list in the database contains strings, but the strings
        //    // are made up of two parts (The Dolphin ID -- a string -- and
        //    // an integer that either the data offset in the old database
        //    // or the unique numerical id of the fin 
        //    // in the Invidiuals table in the SQLite database.)
        //    if (mIDList == null)
        //        return -1;

        //    for (int i = 0; i < mIDList.Count; i++)
        //    {
        //        if (listEntryToID(mIDList[i]).ToString() == id)
        //        {
        //            return i;
        //        }
        //    }

        //    return -1;
        //}
    }
}
