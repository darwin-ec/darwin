using Darwin.Collections;
using Darwin.Database;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;
using System.Windows.Media;
using Darwin.Wpf.Extensions;

namespace Darwin.Wpf.ViewModel
{
    public class MatchingQueueViewModel : INotifyPropertyChanged
    {
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

        public event PropertyChangedEventHandler PropertyChanged;

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
