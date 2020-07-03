using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Features
{
    public class CoordinateFeaturePoint : FeaturePoint, INotifyPropertyChanged
    {
        public static new readonly OutlineFeaturePoint Empty = new OutlineFeaturePoint { IsEmpty = true };

        private Point _coordinate;

        public Point Coordinate
        {
            get => _coordinate;
            set
            {
                _coordinate = value;
                RaisePropertyChanged("Coordinate");
            }
        }

        public CoordinateFeaturePoint()
            : base()
        {
        }
    }
}
