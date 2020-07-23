using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Matching
{
    public abstract class MatchOptions
    {
    }

    public class OutlineMatchOptions : MatchOptions
    {
        public bool MoveTip { get; set; }
        public bool MoveEndsInAndOut { get; set; }
        public bool UseFullFinError { get; set; }
        public bool TrimBeginLeadingEdge { get; set; }
        public float JumpDistancePercentage { get; set; }
        public bool TryAlternateControlPoint3 { get; set; }
    }

    public class FeatureSetMatchOptions : MatchOptions
    {
        public bool UseRemappedOutline { get; set; }
    }
}
