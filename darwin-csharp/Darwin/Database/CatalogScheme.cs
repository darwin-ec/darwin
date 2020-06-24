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
            FeatureSetType = FeatureSetType.DorsalFin;
			SchemeName = string.Empty;
			Categories = new ObservableCollection<Category>();
            IsDefault = false;
		}

        public CatalogScheme(string name, FeatureSetType featureSetType, List<Category> categories)
        {
            SchemeName = name;
            Categories = new ObservableCollection<Category>(categories);
            FeatureSetType = featureSetType;
            IsDefault = true;
        }

        public CatalogScheme(CatalogScheme scheme)
        {
            SchemeName = scheme.SchemeName;
            Categories = new ObservableCollection<Category>(scheme.Categories);
            FeatureSetType = scheme.FeatureSetType;
            IsDefault = scheme.IsDefault;
            IsBuiltIn = scheme.IsBuiltIn;
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
