using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Features
{
    public class FeaturePoint
    {
        public string Name { get; set; }
        public FeaturePointType Type { get; set; }
        public int Position { get; set; }
    }
}
