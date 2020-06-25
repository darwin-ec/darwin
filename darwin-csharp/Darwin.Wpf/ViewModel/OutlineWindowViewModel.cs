using Darwin.Database;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Security.RightsManagement;
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

        private Chain _displayChain;
        public Chain DisplayChain
        {
            get => _displayChain;
            set
            {
                _displayChain = value;
                RaisePropertyChanged("DisplayChain");
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

        public string FeaturePointPositionsDisplay
        {
            get
            {
                if (DatabaseFin == null || DatabaseFin.FinOutline == null || DatabaseFin.FinOutline.FeatureSet == null)
                    return string.Empty;

                StringBuilder sb = new StringBuilder();

                if (DatabaseFin.FinOutline.FeatureSet.FeaturePointList != null)
                {
                    foreach (var fp in DatabaseFin.FinOutline.FeatureSet.FeaturePointList)
                    {
                        sb.AppendLine(fp.Name + ": " + fp.Position.ToString());
                    }
                }

                if (DatabaseFin.FinOutline.FeatureSet.Features != null)
                {
                    foreach (var f in DatabaseFin.FinOutline.FeatureSet.Features.Values)
                    {
                        sb.AppendLine(f.Name + ": " + f.Value.ToString("N2"));
                    }
                }

                return sb.ToString();
            }
        }

        public OutlineWindowViewModel()
        {
            NumWaveletLevels = 7;
        }

        public OutlineWindowViewModel(DarwinDatabase database, DatabaseFin databaseFin)
        {
            Database = database;
            DatabaseFin = new DatabaseFin(databaseFin);
            NumWaveletLevels = 7;

            SetupDisplayElements();
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public void RediscoverFeatures()
        {
            if (Database != null)
            {
                var newContour = new Contour(DatabaseFin.FinOutline.ChainPoints);
                DatabaseFin.FinOutline = new Outline(newContour, Database.CatalogScheme.FeatureSetType);

                newContour?.ClipToBounds();

                DisplayContour = newContour;

                RaisePropertyChanged("FeaturePointPositionsDisplay");
            }
        }

        private void SetupDisplayElements()
        {
            if (DatabaseFin == null)
                return;

            var featurePositions = DatabaseFin.FinOutline?.FeaturePointPositions;

            var clippedContour = new Contour(DatabaseFin.FinOutline.ChainPoints);
            clippedContour?.ClipToBounds();

            clippedContour?.SetFeaturePointPositions(featurePositions);

            DisplayContour = clippedContour;

            Chain newChain = new Chain(DatabaseFin.FinOutline?.Chain);

            newChain.Smooth7();

            DisplayChain = newChain;
        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
