using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Matching
{
    public abstract class MatchOptions
    {
    }

    public class FinFlagsMatchOptions : MatchOptions
    {
        public bool MoveTip { get; set; }
        public bool MoveEndsInAndOut { get; set; }
        public bool UseFullFinError { get; set; }
    }
}
