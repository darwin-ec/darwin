using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Features
{
    public class OutlineFeaturePoint : FeaturePoint, INotifyPropertyChanged
    {
        public static new readonly OutlineFeaturePoint Empty = new OutlineFeaturePoint { IsEmpty = true };

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

        public OutlineFeaturePoint()
            : base()
        {
        }
    }
}
