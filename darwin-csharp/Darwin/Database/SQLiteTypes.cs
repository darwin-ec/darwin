using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Database
{
    public class DBIndividual
    {
        public long id;
        public string idcode;
        public string name;
        public long fkdamagecategoryid;
    }

    public class DBDamageCategory
    {
        public long id;
        public int orderid;
        public string name;
    }
    public class DBImage
    {
        public long id;
        public long fkindividualid;
        public string imagefilename;
        public string dateofsighting;
        public string rollandframe;
        public string locationcode;
        public string shortdescription;
    }

    public class DBThumbnail
    {
        public long id;
        public int rows;
        public string pixmap;
        public long fkimageid;
    }
    public class DBOutline
    {
        public long id;
        public int tipposition;
        public int beginle;
        public int endle;
        public int notchposition;
        public int endte;
        public long fkindividualid;
    }

    public class DBPoint
    {
        public long id;
        public float xcoordinate;
        public float ycoordinate;
        public long fkoutlineid;
        public int orderid;
    }
    public class DBInfo
    {
        public string key;
        public string value;
    }
    public class DBImageModification
    {
        public long id;
        public int operation;
        public int value1;
        public int value2;
        public int value3;
        public int value4;
        public int orderid;
        public long fkimageid;
    }
}
