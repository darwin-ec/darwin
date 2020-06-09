using Darwin.Database;
using Darwin.Matching;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Wpf.ViewModel
{
    public class MatchingResultsWindowViewModel : INotifyPropertyChanged
    {
        private bool _hideInfo;
        public bool HideInfo
        {
            get => _hideInfo;
            set
            {
                _hideInfo = value;
                RaisePropertyChanged("HideInfo");
            }
        }

        private bool _hideIDs;
        public bool HideIDs
        {
            get => _hideIDs;
            set
            {
                _hideIDs = value;
                RaisePropertyChanged("HideIDs");
            }
        }

        private bool _autoScroll;
        public bool AutoScroll
        {
            get => _autoScroll;
            set
            {
                _autoScroll = value;
                RaisePropertyChanged("AutoScroll");
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

        private MatchResults _matchResults;
        public MatchResults MatchResults
        {
            get => _matchResults;
            set
            {
                _matchResults = value;
                RaisePropertyChanged("MatchResults");
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

        public MatchingResultsWindowViewModel(
            DatabaseFin unknownFin,
            MatchResults matchResults,
            DarwinDatabase database)
        {
            if (unknownFin == null)
                throw new ArgumentNullException(nameof(unknownFin));

            if (matchResults == null)
                throw new ArgumentNullException(nameof(matchResults));

            if (database == null)
                throw new ArgumentNullException(nameof(database));

            HideInfo = false;
            HideIDs = false;
            AutoScroll = false;

            DatabaseFin = unknownFin;
            MatchResults = matchResults;
            Database = database;
        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
