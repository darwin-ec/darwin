using Darwin.Database;
using Darwin.Wpf.Model;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Text;

namespace Darwin.Wpf.ViewModel
{
    public class MatchingWindowViewModel : INotifyPropertyChanged
    {
        private ObservableCollection<DBDamageCategory> _categories;
        public ObservableCollection<DBDamageCategory> Categories
        {
            get => _categories;
            set
            {
                _categories = value;
                RaisePropertyChanged("Categories");
            }
        }

        private ObservableCollection<SelectableDBDamageCategory> _selectableCategories;
        public ObservableCollection<SelectableDBDamageCategory> SelectableCategories
        {
            get => _selectableCategories;
            set
            {
                _selectableCategories = value;
                RaisePropertyChanged("SelectableCategories");
            }
        }

        private SearchMethodType _searchMethod;
        public SearchMethodType SearchMethod
        {
            get => _searchMethod;
            set
            {
                _searchMethod = value;
                RaisePropertyChanged("SearchMethod");
            }
        }

        private RangeOfPointsType _rangeOfPoints;
        public RangeOfPointsType RangeOfPoints
        {
            get => _rangeOfPoints;
            set
            {
                _rangeOfPoints = value;
                RaisePropertyChanged("RangeOfPoints");
            }
        }

        private DatabaseFin _databaseFin;
        public DatabaseFin DatabaseFin
        {
            get => _databaseFin;
            set
            {
                _databaseFin = value;
                RaisePropertyChanged("DatabaseFin");
            }
        }

        private bool _showRegistration;
        public bool ShowRegistration
        {
            get => _showRegistration;
            set
            {
                _showRegistration = value;
                RaisePropertyChanged("ShowRegistration");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public MatchingWindowViewModel(DatabaseFin databaseFin, ObservableCollection<DBDamageCategory> categories)
        {
            DatabaseFin = databaseFin;
            Categories = categories;
            SearchMethod = SearchMethodType.AlighIteratively;
            RangeOfPoints = RangeOfPointsType.AllPoints;
            InitializeSelectableCategories();
        }

        private void InitializeSelectableCategories()
        {
            SelectableCategories = new ObservableCollection<SelectableDBDamageCategory>();

            if (Categories != null)
            {
                //bool firstRun = true;
                foreach (var cat in Categories)
                {
                    SelectableCategories.Add(new SelectableDBDamageCategory
                    {
                        IsSelected = true,
                        Name = cat.name
                    });

                    //if (firstRun)
                    //    firstRun = false;
                }
            }
        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
