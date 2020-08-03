using System;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Text;

namespace Darwin.ML
{
    public class MLImage
    {
        public Bitmap Image { get; set; }
        public float XRatio { get; set; }
        public float YRatio { get; set; }
        public int XMin { get; set; }
        public int YMin { get; set; }
        public int XMax { get; set; }
        public int YMax { get; set; }
    }
}
