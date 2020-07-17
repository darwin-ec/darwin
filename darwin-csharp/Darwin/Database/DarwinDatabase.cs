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

using Darwin.Features;
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

    public abstract class DarwinDatabase
    {
        public string Filename { get; set; }

        //***1.99 - the catalog scheme for this database (moved from Options)

        public abstract CatalogScheme CatalogScheme { get; }
        public abstract ObservableCollection<Category> Categories { get; }

        public abstract List<DatabaseFin> AllFins { get; }

        public abstract void CreateEmptyDatabase(CatalogScheme catalogScheme);

        public abstract long Add(DatabaseFin data);
        public abstract void Update(DatabaseFin data);
        public abstract void UpdateIndividual(DatabaseFin data);
        public abstract void UpdateOutline(DatabaseFin data, bool preventInvalidate = false);
        public abstract void Delete(DatabaseFin fin);

        public abstract bool ContainsAllFeatureTypes(List<FeatureType> featureTypes);
        public abstract bool ContainsAllFeaturePointTypes(List<FeaturePointType> featurePointTypes);

        public abstract void InvalidateCache();

        public abstract void SetCatalogScheme(CatalogScheme catalogScheme);
        
        //private abstract void UpdateDBIndividual(DBIndividual individual);

        public abstract DatabaseFin GetFin(long id);
        public abstract List<DatabaseFin> GetAllFins();

        //public abstract DatabaseFin GetItemAbsolute(int pos);

        //public abstract DatabaseFin GetItem(int pos);
        // virtual DatabaseFin<ColorImage>* getItemByName(std::string name) = 0;

        protected DatabaseStatusType mDBStatus; //***1.85
    }
}
