using Darwin.Collections;
using Darwin.Database;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;
using System.Windows.Media;
using Darwin.Wpf.Extensions;
using Darwin.Matching;

namespace Darwin.Wpf.ViewModel
{
    public class MatchingQueueViewModel : INotifyPropertyChanged
    {
        private List<Match> _matches;
        public List<Match> Matches
        {
            get
            {
                if (_matches == null)
                    _matches = new List<Match>();

                return _matches;
            }
            set
            {
                _matches = value;
                RaisePropertyChanged("Matches");
            }
        }

        private ObservableNotifiableCollection<DatabaseFin> _fins;
        public ObservableNotifiableCollection<DatabaseFin> Fins
        {
            get
            {
                if (_fins == null)
                    _fins = new ObservableNotifiableCollection<DatabaseFin>();

                return _fins;
            }
            set
            {
                _fins = value;
                RaisePropertyChanged("Fins");
            }
        }

        private DatabaseFin _selectedFin;
        public DatabaseFin SelectedFin
        {
            get => _selectedFin;
            set
            {
                _selectedFin = value;

                RaisePropertyChanged("SelectedFin");

                if (_selectedFin == null)
                    SelectedImageSource = null;
                else
                    SelectedImageSource = _selectedFin.ModifiedFinImage.ToImageSource();
            }
        }

        private ImageSource _selectedImageSource;
        public ImageSource SelectedImageSource
        {
            get => _selectedImageSource;
            set
            {
                _selectedImageSource = value;
                RaisePropertyChanged("SelectedImageSource");
            }
        }

        private int _queueProgressPercent;
        public int QueueProgressPercent
        {
            get => _queueProgressPercent;
            set
            {
                _queueProgressPercent = value;
                RaisePropertyChanged("QueueProgressPercent");
            }
        }

        private int _currentUnknownPercent;
        public int CurrentUnknownPercent
        {
            get => _currentUnknownPercent;
            set
            {
                _currentUnknownPercent = value;
                RaisePropertyChanged("CurrentUnknownPercent");
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

        public MatchingQueueViewModel()
        {
            Database = CatalogSupport.OpenDatabase(Options.CurrentUserOptions.DatabaseFileName, Options.CurrentUserOptions, false);
            RegistrationMethod = RegistrationMethodType.TrimOptimalTip;
            RangeOfPoints = RangeOfPointsType.AllPoints;
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
