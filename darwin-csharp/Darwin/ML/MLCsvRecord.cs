using CsvHelper.Configuration.Attributes;
using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.ML
{
    public class MLCsvRecord
    {
        [Index(0)]
        public string image { get; set; }

        [Index(1)]
        public float eye_x { get; set; }

        [Index(2)]
        public float eye_y { get; set; }
    }
}
