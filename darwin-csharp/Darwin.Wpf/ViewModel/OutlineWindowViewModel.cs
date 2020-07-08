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
                RecalcXMaxYMax();
            }
        }

        private Contour _displayComparisonContour;
        public Contour DisplayComparisonContour
        {
            get => _displayComparisonContour;
            set
            {
                _displayComparisonContour = value;
                RaisePropertyChanged("DisplayComparisonContour");
                RecalcXMaxYMax();
            }
        }

        private int _xMax;
        public int XMax
        {
            get => _xMax;
            set
            {
                _xMax = value;
                RaisePropertyChanged("XMax");
            }
        }

        private int _yMax;
        public int YMax
        {
            get => _yMax;
            set
            {
                _yMax = value;
                RaisePropertyChanged("YMax");
            }
        }

        private int _contourPointSize;
        public int ContourPointSize
        {
            get => _contourPointSize;
            set
            {
                _contourPointSize = value;
                RaisePropertyChanged("ContourPointSize");
            }
        }

        private int _featurePointSize;
        public int FeaturePointSize
        {
            get => _featurePointSize;
            set
            {
                _featurePointSize = value;
                RaisePropertyChanged("FeaturePointSize");
            }
        }

        private int _comparisonFeaturePointSize;
        public int ComparisonFeaturePointSize
        {
            get => _comparisonFeaturePointSize;
            set
            {
                _comparisonFeaturePointSize = value;
                RaisePropertyChanged("ComparisonFeaturePointSize");
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

        private Chain _displayComparisonChain;
        public Chain DisplayComparisonChain
        {
            get => _displayComparisonChain;
            set
            {
                _displayComparisonChain = value;
                RaisePropertyChanged("DisplayComparisonChain");
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

        private DatabaseFin _comparisonFin;
        public DatabaseFin ComparisonFin
        {
            get => _comparisonFin;
            set
            {
                _comparisonFin = value;
                RaisePropertyChanged("ComparisonFin");
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

        private System.Windows.Visibility _rediscoverVisiblility;
        public System.Windows.Visibility RediscoverVisibility
        {
            get => _rediscoverVisiblility;
            set
            {
                _rediscoverVisiblility = value;
                RaisePropertyChanged("RediscoverVisibility");
            }
        }

        private System.Windows.Visibility _comparisonTextVisiblility;
        public System.Windows.Visibility ComparisonTextVisibility
        {
            get => _comparisonTextVisiblility;
            set
            {
                _comparisonTextVisiblility = value;
                RaisePropertyChanged("ComparisonTextVisibility");
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
                        if (!fp.Ignore)
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

        public string ComparisonFeaturePointPositionsDisplay
        {
            get
            {
                if (ComparisonFin == null || ComparisonFin.FinOutline == null || ComparisonFin.FinOutline.FeatureSet == null)
                    return string.Empty;

                StringBuilder sb = new StringBuilder();

                if (ComparisonFin.FinOutline.FeatureSet.FeaturePointList != null)
                {
                    foreach (var fp in ComparisonFin.FinOutline.FeatureSet.FeaturePointList)
                    {
                        if (!fp.Ignore)
                            sb.AppendLine(fp.Name + ": " + fp.Position.ToString());
                    }
                }

                if (ComparisonFin.FinOutline.FeatureSet.Features != null)
                {
                    foreach (var f in ComparisonFin.FinOutline.FeatureSet.Features.Values)
                    {
                        sb.AppendLine(f.Name + ": " + f.Value.ToString("N2"));
                    }
                }

                return sb.ToString();
            }
        }

        public OutlineWindowViewModel()
        {
            ContourPointSize = 3;
            FeaturePointSize = 8;
            NumWaveletLevels = 7;
            RediscoverVisibility = System.Windows.Visibility.Visible;
            ComparisonTextVisibility = System.Windows.Visibility.Collapsed;
        }

        public OutlineWindowViewModel(DarwinDatabase database, DatabaseFin databaseFin)
            : this()
        {
            Database = database;
            DatabaseFin = new DatabaseFin(databaseFin);

            SetupDisplayElements();
        }

        public OutlineWindowViewModel(DarwinDatabase database, DatabaseFin databaseFin, DatabaseFin comparisonFin)
            : this()
        {
            Database = database;
            DatabaseFin = new DatabaseFin(databaseFin);
            ComparisonFin = new DatabaseFin(comparisonFin);
            RediscoverVisibility = System.Windows.Visibility.Collapsed;
            ComparisonTextVisibility = System.Windows.Visibility.Visible;
            ContourPointSize = 5;
            FeaturePointSize = 20;
            ComparisonFeaturePointSize = 28;

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

            if (ComparisonFin == null)
            {
                clippedContour?.ClipToBounds();
            }
            else
            {
                int xMin = clippedContour.XMin;
                int yMin = clippedContour.YMin;

                var clippedComparisonContour = new Contour(ComparisonFin.FinOutline.ChainPoints);

                if (clippedComparisonContour.XMin < xMin)
                    xMin = clippedComparisonContour.XMin;

                if (clippedComparisonContour.YMin < yMin)
                    yMin = clippedComparisonContour.YMin;

                clippedContour?.ClipToBounds(xMin, yMin);
                clippedComparisonContour?.ClipToBounds(xMin, yMin);

                clippedComparisonContour?.SetFeaturePointPositions(ComparisonFin.FinOutline?.FeaturePointPositions);

                DisplayComparisonContour = clippedComparisonContour;
            }

            clippedContour?.SetFeaturePointPositions(featurePositions);

            DisplayContour = clippedContour;

            Chain newChain = new Chain(DatabaseFin.FinOutline?.Chain);

            newChain.Smooth7();

            DisplayChain = newChain;

            if (ComparisonFin != null)
            {
                Chain newComparisonChain = new Chain(ComparisonFin.FinOutline?.Chain);
                newComparisonChain.Smooth7();
                DisplayComparisonChain = newComparisonChain;
            }
        }

        private void RecalcXMaxYMax()
        {
            int xMax = 0;
            int yMax = 0;

            if (DisplayContour != null)
            {
                xMax = DisplayContour.XMax;
                yMax = DisplayContour.YMax;
            }

            if (DisplayComparisonContour != null)
            {
                if (DisplayComparisonContour.XMax > xMax)
                    xMax = DisplayComparisonContour.XMax;

                if (DisplayComparisonContour.YMax > yMax)
                    yMax = DisplayComparisonContour.YMax;
            }

            XMax = xMax;
            YMax = yMax;
        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
