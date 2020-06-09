using Darwin.Database;
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

        public MatchingResultsWindowViewModel(DarwinDatabase database)
        {
            HideInfo = false;
            HideIDs = false;
            AutoScroll = false;

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
