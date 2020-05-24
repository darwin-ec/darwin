using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Matching
{
    public class RefPoint
    {
        public double Parameter { get; set; }
        public int SegId { get; set; } // index of segment
        public int SegOp { get; set; } // operation (0 means add, 1 means remove from active segment list)
    }
}
