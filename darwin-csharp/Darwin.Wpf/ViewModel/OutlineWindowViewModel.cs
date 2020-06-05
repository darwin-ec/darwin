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

        public OutlineWindowViewModel(DatabaseFin databaseFin)
        {
            DatabaseFin = databaseFin;
            NumWaveletLevels = 7;

            SetupDisplayElements();
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void SetupDisplayElements()
        {
            if (DatabaseFin == null)
                return;

            var featurePositions = DatabaseFin.FinOutline?.FeaturePointPositions;

            var clippedContour = new Contour(DatabaseFin.FinOutline.ChainPoints);
            clippedContour?.ClipToBounds();

            clippedContour?.SetFeaturePointPositions(featurePositions);

            DisplayContour = clippedContour;
        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
