using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Features
{
    public class FeaturePoint : INotifyPropertyChanged
    {
        public static readonly FeaturePoint Empty = new FeaturePoint { IsEmpty = true };

        private string _name;
        private FeaturePointType _type;
        private int _position;
        private bool _userSetPosition;
        private bool _isEmpty;
        private bool _ignore;

        public bool IsEmpty
        {
            get => _isEmpty;
            set
            {
                _isEmpty = value;
                RaisePropertyChanged("IsEmpty");
            }
        }

        public string Name
        {
            get => _name;
            set
            {
                _name = value;
                RaisePropertyChanged("Name");
            }
        }

        public FeaturePointType Type
        {
            get => _type;
            set
            {
                _type = value;
                RaisePropertyChanged("Type");
            }
        }
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

        public bool UserSetPosition
        {
            get => _userSetPosition;
            set
            {
                _userSetPosition = value;
                RaisePropertyChanged("UserSetPosition");
                IsEmpty = false;
            }
        }

        public bool Ignore
        {
            get => _ignore;
            set
            {
                _ignore = value;
                RaisePropertyChanged("Ignore");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public FeaturePoint()
        {
            IsEmpty = true;
        }

        protected virtual void RaisePropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
