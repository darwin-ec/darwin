using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Wavelet
{
    public class WL_Struct
    {
        public string Type { get; set; }
        public string Name { get; set; }
        public Dictionary<string, string> PList { get; set; }
    }

    public class WL_Filter
    {
        public WL_Struct Info { get; set; }
        public int Length { get; set; }
        public int Offset { get; set; }
        public double[] Coefs { get; set; }
    }

    public class WL_SubbandFilter
    {
        public WL_Struct Info { get; set; }
        private WL_Filter[] _filters = new WL_Filter[4];
        public WL_Filter[] Filters
        {
            get
            {
                return _filters;
            }
            set
            {
                _filters = value;
            }
        }
    }

    public class WL_Matrix
    {
        public WL_Struct Info { get; set; }
        public int Rows { get; set; }
        public int Cols { get; set; }
        public double[,] Data { get; set; }
    }

    /* WL_Vector
       1-D Vector data type
    */
    public class WL_Vector
    {
        public WL_Struct Info { get; set; }
        public int Length { get; set; }
        public double[] Data { get; set; }     /* data array */
    }

    public class WL_Volume
    {
        public WL_Struct Info { get; set; }
        public int Rows { get; set; }
        public int Cols { get; set; }
        public int Depth { get; set; }
        public double[,,] Data { get; set; }
    }
}
