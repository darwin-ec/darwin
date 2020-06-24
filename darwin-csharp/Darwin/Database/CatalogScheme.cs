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

using Darwin.Features;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Text;

namespace Darwin.Database
{
    public class Category : INotifyPropertyChanged
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

        public Category()
        {

        }

        public Category(string name)
        {
            Name = name;
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }

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

        private ObservableCollection<Category> _categories;
		public ObservableCollection<Category> Categories
        {
            get => _categories;
            set
            {
                _categories = value;
                RaisePropertyChanged("Categories");
            }
        }

        private FeatureSetType _featureSetType;
        public FeatureSetType FeatureSetType
        {
            get => _featureSetType;
            set
            {
                _featureSetType = value;
                RaisePropertyChanged("FeatureSetType");
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

        private bool _isBuiltIn;
        public bool IsBuiltIn
        {
            get => _isBuiltIn;
            set
            {
                _isBuiltIn = value;
                RaisePropertyChanged("IsBuiltIn");
            }
        }

        public CatalogScheme()
		{
			SchemeName = string.Empty;
			Categories = new ObservableCollection<Category>();
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
