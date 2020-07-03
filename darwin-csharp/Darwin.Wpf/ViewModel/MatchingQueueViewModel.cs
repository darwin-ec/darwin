using Darwin.Collections;
using Darwin.Database;
using System;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;
using System.Text;
using System.Windows.Media;
using Darwin.Wpf.Extensions;
using Darwin.Matching;
using System.Diagnostics;

namespace Darwin.Wpf.ViewModel
{
    public class MatchingQueueViewModel : INotifyPropertyChanged
    {
        private static object selectedFinSync = new object();

        private DatabaseFin _selectedFin;
        public DatabaseFin SelectedFin
        {
            get => _selectedFin;
            set
            {
                lock (selectedFinSync)
                {
                    try
                    {
                        _selectedFin = value;

                        RaisePropertyChanged("SelectedFin");

                        if (_selectedFin == null)
                            SelectedImageSource = null;
                        else
                            SelectedImageSource = _selectedFin.FinImage.ToImageSource();
                    }
                    catch (Exception ex)
                    {
                        // The above can throw exceptions if the bitmaps are large,
                        // since we're running an update on a separate thread and 
                        // possibly modifying the Bitmaps elsewhere.
                        Trace.WriteLine(ex);
                    }
                    MatchingQueue.CheckQueueRunnable();
                }
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

        private MatchingQueue _matchingQueue;
        public MatchingQueue MatchingQueue
        {
            get => _matchingQueue;
            set
            {
                _matchingQueue = value;
                RaisePropertyChanged("MatchingQueue");
            }
        }

        public string IndividualTerminology
        {
            get
            {
                if (_database == null)
                    return string.Empty;

                return _database.CatalogScheme.IndividualTerminology;
            }
        }

        public string IndividualTerminologyInitialCaps
        {
            get
            {
                if (_database == null)
                    return string.Empty;

                return _database.CatalogScheme.IndividualTerminologyInitialCaps;
            }
        }

        private DarwinDatabase _database;

        public MatchingQueueViewModel()
        {
            _database = CatalogSupport.OpenDatabase(Options.CurrentUserOptions.DatabaseFileName,
                Options.CurrentUserOptions.DefaultCatalogScheme, false);
            MatchingQueue = new MatchingQueue(
                _database,
                RegistrationMethodType.TrimOptimalTip,
                RangeOfPointsType.AllPoints);
        }

        // Pass-through
        public void SaveQueue(string filename)
        {
            MatchingQueue.SaveQueue(filename);
        }

        // Pass-through
        public void LoadQueue(string filename)
        {
            SelectedFin = null;
            MatchingQueue.LoadQueue(filename);

            if (MatchingQueue.Fins?.Count > 0)
                SelectedFin = MatchingQueue.Fins.First();
        }

        // Pass-through
        public MatchResults LoadMatchResults(string filename, out DarwinDatabase database, out DatabaseFin databaseFin)
        {
            return MatchResults.Load(filename, out database, out databaseFin);
        }

        public void SaveMatchResults()
        {
            MatchingQueue.SaveMatchResults(Options.CurrentUserOptions.CurrentMatchQueueResultsPath);
        }

        public string GetMatchSummary()
        {
            return MatchingQueue.GetSummary();
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
