using Darwin.Database;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Wpf.ViewModel
{
    public class OutlineWindowViewModel : INotifyPropertyChanged
    {
        private Contour _displayContour;
        public Contour DisplayContour
        {
            get => _displayContour;
            set
            {
                _displayContour = value;
                RaisePropertyChanged("DisplayContour");
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

        private int _numWaveletLevels;
        public int NumWaveletLevels
        {
            get => _numWaveletLevels;
            set
            {
                _numWaveletLevels = value;
                RaisePropertyChanged("NumWaveletLevels");
            }
        }

        public OutlineWindowViewModel()
        {
            NumWaveletLevels = 7;
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
