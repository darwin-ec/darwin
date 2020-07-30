using Darwin.Collections;
using Darwin.Utilities;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace Darwin.Features
{
    public class CoordinateFeaturePoint : FeaturePoint, INotifyPropertyChanged
    {
        public static new readonly CoordinateFeaturePoint Empty = new CoordinateFeaturePoint { IsEmpty = true };

        private Point _coordinate;

        public Point Coordinate
        {
            get => _coordinate;
            set
            {
                if (_coordinate != null)
                    _coordinate.PropertyChanged -= OnPointPropertyChanged;

                _coordinate = value;

                if (_coordinate != null)
                    IsEmpty = false;

                RaisePropertyChanged("Coordinate");

                _coordinate.PropertyChanged += OnPointPropertyChanged;
            }
        }

        public CoordinateFeaturePoint()
            : base()
        {
        }

        public CoordinateFeaturePoint(CoordinateFeaturePoint coordinateFeaturePoint)
            : base(coordinateFeaturePoint)
        {
            _coordinate = new Point(coordinateFeaturePoint._coordinate.X, coordinateFeaturePoint._coordinate.Y);
        }

        public static CoordinateFeaturePoint FindClosestCoordinateFeaturePointWithDistance(ObservableNotifiableCollection<CoordinateFeaturePoint> points, PointF p, out float distance)
        {
            return FindClosestCoordinateFeaturePointWithDistance(points, p.X, p.Y, out distance);
        }

        public static CoordinateFeaturePoint FindClosestCoordinateFeaturePointWithDistance(ObservableNotifiableCollection<CoordinateFeaturePoint> points, float X, float Y, out float distance)
        {
            distance = 5000f;
            var minFeature = CoordinateFeaturePoint.Empty;

            if (points == null || points.Count < 1)
                return minFeature;

            double? minDist = null;

            foreach (var f in points.Where(fp => !fp.Ignore && !fp.IsEmpty))
            {
                if (f.Coordinate == null)
                    continue;

                var distToFeature = MathHelper.GetDistance(X, Y, f.Coordinate.X, f.Coordinate.Y);

                if (minDist == null || distToFeature < minDist)
                {
                    minDist = distToFeature;
                    distance = (float)distToFeature;
                    minFeature = f;
                }
            }

            return minFeature;
        }

        protected void OnPointPropertyChanged(object sender, PropertyChangedEventArgs args)
        {
            RaisePropertyChanged("Coordinate");
        }
    }
}