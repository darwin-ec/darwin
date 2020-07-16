using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Features
{
    public enum FeatureType
    {
        LeadingEdgeAngle = 0,
        HasMouthDent = 1,
        BrowCurvature = 2
    }

    public class Feature : BaseEntity, INotifyPropertyChanged
    {
        public static readonly Feature Empty = new Feature { IsEmpty = true };
        private bool _isEmpty;
        private FeatureType _type;
        private string _name;
        private double? _value;

        public string Name
        {
            get => _name;
            set
            {
                _name = value;
                RaisePropertyChanged("Name");
                IsEmpty = false;
            }
        }

        public FeatureType Type
        {
            get => _type;
            set
            {
                _type = value;
                RaisePropertyChanged("Type");
                IsEmpty = false;
            }
        }

        public double? Value
        {
            get => _value;
            set
            {
                _value = value;
                RaisePropertyChanged("Value");
                IsEmpty = false;
            }
        }

        public bool IsEmpty
        {
            get => _isEmpty;
            set
            {
                _isEmpty = value;
                RaisePropertyChanged("IsEmpty");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public Feature()
        {
            IsEmpty = true;
        }

        protected virtual void RaisePropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
