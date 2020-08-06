// This file is part of DARWIN.
// Copyright (C) 1994 - 2020
//
// DARWIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DARWIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DARWIN.  If not, see<https://www.gnu.org/licenses/>.

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