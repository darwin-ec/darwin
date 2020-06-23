using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Matching
{
    public class Result
    {
        //  005CM - contours used in final match
        public FloatContour unknownContour;
        public FloatContour dbContour;

        public string ThumbnailFilenameUri { get; set; }
        public int mThumbnailRows;

        public string ImageFilename { get; set; }    //  001DB - image file for database fin
        public int Position { get; set; }            // position (index) of database fin in database file

        public long DatabaseID { get; set; }
        public double Error { get; set; }
        public string IDCode { get; set; }
        public string Name { get; set; }
        public string Damage { get; set; }
        public string Date { get; set; }
        public string Location { get; set; }
        public int Rank { get; set; } //  1.5

        //  1.1 - new members to track three point correspondences for final mapping in match
        public int UnkShiftedLEBegin { get; set; }
        public int UnkShiftedTip { get; set; }
        public int UnkShiftedTEEnd { get; set; }
        public int DBShiftedLEBegin { get; set; }
        public int DBShiftedTip { get; set; }
        public int DBShiftedTEEnd { get; set; }

        public Result(
            FloatContour unknown, //  005CM
            FloatContour db, //  005CM
            long databaseID,
            string filename,   //  001DB
            string thumbnailFilenameUri, //  1.0
            int position,
            double error,
            string idcode,
            string name,
            string damage,
            string date,
            string location
        )
        {
            unknownContour = new FloatContour(unknown); //  1.3 - Mem Leak - make copies now
            dbContour = new FloatContour(db);           //  1.3 - Mem Leak - make copies now
            DatabaseID = databaseID;
            ImageFilename = filename; //  001DB
            Position = position;
            Error = error;
            IDCode = idcode;
            Name = name;
            Damage = damage;
            Date = date;
            Location = location;
            Rank = 0; //  1.5
            UnkShiftedLEBegin = 0; //  1.1 - following indices set to defaults by constructor
            UnkShiftedTip = 0;
            UnkShiftedTEEnd = 0;
            DBShiftedLEBegin = 0;
            DBShiftedTip = 0;
            DBShiftedTEEnd = 0;

            ThumbnailFilenameUri = thumbnailFilenameUri;
        }

        public Result(Result r)
        {
            DatabaseID = r.DatabaseID;
            ImageFilename = r.ImageFilename;   //  001DB
            Position = r.Position;
            Error = r.Error;
            IDCode = r.IDCode;
            Name = r.Name;
            Damage = r.Damage;
            Location = r.Location;
            Rank = r.Rank; //  1.5
            unknownContour = new FloatContour(r.unknownContour); //  1.3 - Mem Leak - make copies now
            dbContour = new FloatContour(r.dbContour);           //  1.3 - Mem Leak - make copies now
            UnkShiftedLEBegin = r.UnkShiftedLEBegin;
            UnkShiftedTip = r.UnkShiftedTip;
            UnkShiftedTEEnd = r.UnkShiftedTEEnd;
            DBShiftedLEBegin = r.DBShiftedLEBegin;
            DBShiftedTip = r.DBShiftedTip;
            DBShiftedTEEnd = r.DBShiftedTEEnd;
            ThumbnailFilenameUri = r.ThumbnailFilenameUri;
        }

        //  1.1 - sets six indices for points used in final contour mapping
        public void SetMappingControlPoints(
                int unkLEBegin, int unkTip, int unkTEEnd,
                int dbLEBegin, int dbTip, int dbTEEnd)
        {
            UnkShiftedLEBegin = unkLEBegin;
            UnkShiftedTip = unkTip;
            UnkShiftedTEEnd = unkTEEnd;
            DBShiftedLEBegin = dbLEBegin;
            DBShiftedTip = dbTip;
            DBShiftedTEEnd = dbTEEnd;
        }

        //  1.1 - gets six indices for points used in final contour mapping
        public void GetMappingControlPoints(
                out int unkLEBegin, out int unkTip, out int unkTEEnd,
                out int dbLEBegin, out int dbTip, out int dbTEEnd)
        {
            unkLEBegin = UnkShiftedLEBegin;
            unkTip = UnkShiftedTip;
            unkTEEnd = UnkShiftedTEEnd;
            dbLEBegin = DBShiftedLEBegin;
            dbTip = DBShiftedTip;
            dbTEEnd = DBShiftedTEEnd;
        }
    }
}
