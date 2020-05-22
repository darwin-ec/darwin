using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin
{
    public enum FeaturePointType
    {
        NoFeature = -1,
        LeadingEdgeBegin = 1,
        LeadingEdgeEnd = 2,
        Tip = 3,
        Notch = 4,
        PointOfInflection = 5
    }
}
