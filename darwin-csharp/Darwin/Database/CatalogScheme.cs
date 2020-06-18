/////////////////////////////////////////////////////////////////////
//   file: CatalogScheme.h
//
// author: J H Stewman
//
//   date: 7/18/2008
//
// new catalog scheme class - begin integrating with all code - JHS
//
/////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Text;

namespace Darwin.Database
{
    public class CatalogScheme : INotifyPropertyChanged
    {
        private string _schemeName;
		public string SchemeName
        {
            get => _schemeName;
            set
            {
                _schemeName = value;
                RaisePropertyChanged("SchemeName");
            }
        }

        private ObservableCollection<string> _categoryNames;
		public ObservableCollection<string> CategoryNames
        {
            get => _categoryNames;
            set
            {
                _categoryNames = value;
                RaisePropertyChanged("CategoryNames");
            }
        }

        private bool _isDefault;
		public bool IsDefault
        {
            get => _isDefault;
            set
            {
                _isDefault = value;
                RaisePropertyChanged("IsDefault");
            }
        }

		public CatalogScheme()
		{
			SchemeName = string.Empty;
			CategoryNames = new ObservableCollection<string>();
            IsDefault = false;
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
