using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Features
{
    public class BearFeatureSet : FeatureSet
    {
        public BearFeatureSet()
        {
            FeatureSetType = FeatureSetType.Bear;
        }

        public BearFeatureSet(Chain chain, FloatContour chainPoints)
        {
            FeatureSetType = FeatureSetType.Bear;
            FeaturePoints = new Dictionary<FeaturePointType, FeaturePoint>()
            {
                {
                    FeaturePointType.Tip, new FeaturePoint { Name = "Tip of Nose", Type = FeaturePointType.Tip }
                }
            };
        }

        public override void SetFeaturePointPosition(FeaturePointType featurePointType, int position)
        {
            throw new NotImplementedException();
        }
    }
}
