using Darwin.Database;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Wpf.Model
{
    public enum SearchMethodType
    {
        AlignQuickDirty = 0,
        AlighIteratively = 1
    }

    public enum RangeOfPointsType
    {
        AllPoints = 0,
        TrailingEdgeOnly = 1
    }

    public class SelectableDBDamageCategory : INotifyPropertyChanged
    {
        private string _name;
        public string Name
        {
            get => _name;

            set
            {
                _name = value;
                RaisePropertyChanged("Name");
            }
        }

        private bool _isSelected;
        public bool IsSelected
        {
            get => _isSelected;
            
            set
            {
                _isSelected = value;
                RaisePropertyChanged("IsSelected");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
