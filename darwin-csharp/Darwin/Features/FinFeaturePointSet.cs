using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Features
{
    public class FinFeaturePointSet : FeaturePointSet
    {
        public FinFeaturePointSet()
        {
            FeaturePoints = new List<FeaturePoint>()
            {
                new FeaturePoint
                {
                    Name = "Dorsal Fin Tip",
                    Type = FeaturePointType.Tip
                }
            };
        }
    }
}
