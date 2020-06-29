using Darwin.Utilities;
using Darwin.Wavelet;
using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Features
{
    public class FinFeatureSet : FeatureSet
    {
        private static Dictionary<FeaturePointType, string> FeaturePointNameMapping = new Dictionary<FeaturePointType, string>()
        {
            { FeaturePointType.Tip, "Fin Tip" },
            { FeaturePointType.Notch, "Notch" },
            { FeaturePointType.LeadingEdgeBegin, "Beginning of Leading Edge" },
            { FeaturePointType.LeadingEdgeEnd, "End of Leading Edge" },
            { FeaturePointType.PointOfInflection, "End of Trailing Edge" }
        };

        // TODO: Valid features mapping?

        public FinFeatureSet()
        {
            FeatureSetType = FeatureSetType.DorsalFin;

            FeaturePoints = new Dictionary<FeaturePointType, ContourFeaturePoint>()
            {
                { FeaturePointType.Tip, new ContourFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Tip], Type = FeaturePointType.Tip, IsEmpty = true } },
                { FeaturePointType.Notch, new ContourFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.Notch], Type = FeaturePointType.Notch, IsEmpty = true } },
                { FeaturePointType.LeadingEdgeBegin, new ContourFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.LeadingEdgeBegin], Type = FeaturePointType.LeadingEdgeBegin, IsEmpty = true } },
                { FeaturePointType.LeadingEdgeEnd, new ContourFeaturePoint { Ignore = true, Name = FeaturePointNameMapping[FeaturePointType.LeadingEdgeEnd], Type = FeaturePointType.LeadingEdgeEnd, IsEmpty = true } },
                { FeaturePointType.PointOfInflection, new ContourFeaturePoint { Name = FeaturePointNameMapping[FeaturePointType.PointOfInflection], Type = FeaturePointType.PointOfInflection, IsEmpty = true } }
            };

            Features = new Dictionary<FeatureType, Feature>()
            {
                { FeatureType.LeadingEdgeAngle, new Feature { Name = "Angle of Leading Edge", Type = FeatureType.LeadingEdgeAngle, IsEmpty = true } }
            };
        }

        public FinFeatureSet(List<ContourFeaturePoint> featurePoints)
            : this()
        {
            if (featurePoints == null)
                throw new ArgumentNullException(nameof(featurePoints));

            foreach (var fp in featurePoints)
            {
                fp.Name = FeaturePointNameMapping[fp.Type];
                FeaturePoints[fp.Type] = fp;
            }
        }

        public FinFeatureSet(Chain chain, FloatContour chainPoints)
            : this()
        {
            int tipPos = FindTip(chain, chainPoints);
            int notchPos = FindNotch(chain, tipPos);
            int beginLE = FindBeginLE(chain, tipPos);
            int endLE = FindEndLE(chain, beginLE, tipPos);
            int endTE = FindPointOfInflection(chain, tipPos);

            FeaturePoints[FeaturePointType.Tip].Position = tipPos;
            FeaturePoints[FeaturePointType.Notch].Position = notchPos;
            FeaturePoints[FeaturePointType.LeadingEdgeBegin].Position = beginLE;
            FeaturePoints[FeaturePointType.LeadingEdgeEnd].Position = endLE;
            FeaturePoints[FeaturePointType.PointOfInflection].Position = endTE;

            double leAngle = FindLEAngle(chain, tipPos, endLE);
            Features[FeatureType.LeadingEdgeAngle].Value = leAngle;
        }

        public override void SetFeaturePointPosition(FeaturePointType featurePointType, int position)
        {
            if (!FeaturePoints.ContainsKey(featurePointType))
                throw new ArgumentOutOfRangeException(nameof(featurePointType));

            FeaturePoints[featurePointType].Position = position;
            FeaturePoints[featurePointType].UserSetPosition = true;
        }
    }
}
