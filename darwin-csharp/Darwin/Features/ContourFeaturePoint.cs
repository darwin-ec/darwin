using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Features
{
    public class ContourFeaturePoint : FeaturePoint, INotifyPropertyChanged
    {
        public static new readonly ContourFeaturePoint Empty = new ContourFeaturePoint { IsEmpty = true };

        private int _position;

        public int Position
        {
            get => _position;
            set
            {
                _position = value;
                RaisePropertyChanged("Position");
                IsEmpty = false;
            }
        }

        public ContourFeaturePoint()
            : base()
        {
        }
    }
}
