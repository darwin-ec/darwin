using Darwin.Database;
using Darwin.Matching;
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

        private RegistrationMethodType _registrationMethod;
        public RegistrationMethodType RegistrationMethod
        {
            get => _registrationMethod;
            set
            {
                _registrationMethod = value;
                RaisePropertyChanged("RegistrationMethod");
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

        private bool _matchRunning;
        public bool MatchRunning
        {
            get => _matchRunning;
            set
            {
                _matchRunning = value;
                RaisePropertyChanged("MatchRunning");
            }
        }

        private bool _pauseMatching;
        public bool PauseMatching
        {
            get => _pauseMatching;
            set
            {
                _pauseMatching = value;
                RaisePropertyChanged("PauseMatching");
            }
        }

        private bool _cancelMatching;
        public bool CancelMatching
        {
            get => _cancelMatching;
            set
            {
                _cancelMatching = value;
                RaisePropertyChanged("CancelMatching");
            }
        }

        private int _matchProgressPercent;
        public int MatchProgressPercent
        {
            get => _matchProgressPercent;
            set
            {
                _matchProgressPercent = value;
                RaisePropertyChanged("MatchProgressPercent");
            }
        }

        private Match _match;
        public Match Match
        {
            get => _match;
            set
            {
                _match = value;
                RaisePropertyChanged("Match");
            }
        }

        private DarwinDatabase _database;
        public DarwinDatabase Database
        {
            get => _database;
            set
            {
                _database = value;
                RaisePropertyChanged("Database");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public MatchingWindowViewModel(DatabaseFin databaseFin,
            DarwinDatabase database,
            ObservableCollection<DBDamageCategory> categories)
        {
            DatabaseFin = databaseFin;
            Categories = categories;
            RegistrationMethod = RegistrationMethodType.TrimOptimalTip;
            RangeOfPoints = RangeOfPointsType.AllPoints;
            Database = database;

            Match = new Match(DatabaseFin, Database);

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
