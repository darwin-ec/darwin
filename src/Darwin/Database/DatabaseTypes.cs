// This file is part of DARWIN.
// Copyright (C) 1994 - 2020
//
// DARWIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DARWIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DARWIN.  If not, see<https://www.gnu.org/licenses/>.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Database
{
    public class DBIndividual
    {
        public long id { get; set; }
        public string idcode { get; set; }
        public string name { get; set; }
        public long fkdamagecategoryid { get; set; }
    }

    public class DBImage
    {
        public long id { get; set; }
        public long fkindividualid { get; set; }
        public string imagefilename { get; set; }
        public string original_imagefilename { get; set; }
        public string dateofsighting { get; set; }
        public string rollandframe { get; set; }
        public string locationcode { get; set; }
        public string shortdescription { get; set; }
    }

    public class DBThumbnail
    {
        public long id { get; set; }
        public int rows { get; set; }
        public string pixmap { get; set; }
        public long fkimageid { get; set; }
    }
    public class DBOutline
    {
        public long id { get; set; }
        public double scale { get; set; }
        public int tipposition { get; set; }
        public int beginle { get; set; }
        public int endle { get; set; }
        public int notchposition { get; set; }
        public int endte { get; set; }
        public long fkindividualid { get; set; }
    }

    public class DBPoint
    {
        public long id { get; set; }
        public float xcoordinate { get; set; }
        public float ycoordinate { get; set; }
        public long fkoutlineid { get; set; }
        public int orderid { get; set; }
    }

    public class DBInfo
    {
        public string key { get; set; }
        public string value { get; set; }
    }

    public class DBImageModification
    {
        public long id { get; set; }
        public int operation { get; set; }
        public int value1 { get; set; }
        public int value2 { get; set; }
        public int value3 { get; set; }
        public int value4 { get; set; }
        public int orderid { get; set; }
        public long fkimageid { get; set; }
    }
}
