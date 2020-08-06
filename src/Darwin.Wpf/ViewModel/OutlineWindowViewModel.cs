// This file is part of DARWIN.
// Copyright (C) 1994 - 2020
//
// DARWIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DARWIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DARWIN.  If not, see<https://www.gnu.org/licenses/>.

using Darwin.Collections;
using Darwin.Database;
using Darwin.Features;
using Darwin.Matching;
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

        private ObservableNotifiableCollection<CoordinateFeaturePoint> _coordinateFeaturePoints;
        public ObservableNotifiableCollection<CoordinateFeaturePoint> CoordinateFeaturePoints
        {
            get
            {
                if (_coordinateFeaturePoints == null)
                    _coordinateFeaturePoints = new ObservableNotifiableCollection<CoordinateFeaturePoint>();

                return _coordinateFeaturePoints;
            }
            set
            {
                _coordinateFeaturePoints = value;
                RaisePropertyChanged("CoordinateFeaturePoints");
            }
        }

        private Result _matchResult;

        public string DisplayText
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
                        sb.AppendLine(f.Name + ": " + f.Value?.ToString("N2"));
                    }
                }

                if (DatabaseFin.FinOutline.FeatureSet.CoordinateFeaturePointList != null)
                {
                    foreach (var f in DatabaseFin.FinOutline.FeatureSet.CoordinateFeaturePointList)
                    {
                        if (f.Coordinate != null)
                            sb.AppendLine(f.Name + ": (" + f.Coordinate?.X + ", " + f.Coordinate?.Y + ")");
                    }
                }

                if (_matchResult != null)
                {
                    if (_matchResult.DBRHat != null)
                    {
                        sb.AppendLine();
                        sb.AppendLine("RHat:");
                        for (int i = 0; i < _matchResult.DBRHat.Count; i++)
                            sb.AppendLine(_matchResult.DBRHat[i].ToString("N2"));
                        sb.AppendLine();
                    }

                    if (_matchResult.DBRawRatios != null)
                    {
                        sb.AppendLine("Raw Ratios:");
                        for (int i = 0; i < _matchResult.DBRawRatios.Count; i++)
                            sb.AppendLine(_matchResult.DBRawRatios[i].ToString("N2"));
                        sb.AppendLine();
                    }
                }

                return sb.ToString();
            }
        }

        public string ComparisonDisplayText
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
                        sb.AppendLine(f.Name + ": " + f.Value?.ToString("N2"));
                    }
                }

                if (ComparisonFin.FinOutline.FeatureSet.CoordinateFeaturePointList != null)
                {
                    foreach (var f in ComparisonFin.FinOutline.FeatureSet.CoordinateFeaturePointList)
                    {
                        if (f.Coordinate != null)
                            sb.AppendLine(f.Name + ": (" + f.Coordinate?.X + ", " + f.Coordinate?.Y + ")");
                    }
                }

                if (_matchResult != null)
                {
                    if (_matchResult.RHat != null)
                    {
                        sb.AppendLine();
                        sb.AppendLine("RHat:");
                        for (int i = 0; i < _matchResult.RHat.Count; i++)
                            sb.AppendLine(_matchResult.RHat[i].ToString("N2"));
                        sb.AppendLine();
                    }

                    if (_matchResult.RawRatios != null)
                    {
                        sb.AppendLine("Raw Ratios:");
                        for (int i = 0; i < _matchResult.RawRatios.Count; i++)
                            sb.AppendLine(_matchResult.RawRatios[i].ToString("N2"));
                        sb.AppendLine();
                    }

                    if (_matchResult.RawError != null && _matchResult.RawError.Count > 0)
                    {
                        sb.AppendLine();
                        sb.AppendLine("Raw Error");
                        foreach (var re in _matchResult.RawError)
                        {
                            sb.Append("Factor " + re.FactorIndex + ": ");
                            sb.Append(re.Error);
                            sb.AppendLine(" (" + re.Weight.ToString("P") + ")");
                        }
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

        public OutlineWindowViewModel(DarwinDatabase database,
            DatabaseFin databaseFin,
            DatabaseFin comparisonFin,
            Result matchResult)
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

            _matchResult = matchResult;

            SetupDisplayElements();
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public void RediscoverFeatures()
        {
            if (Database != null)
            {
                var newContour = new Contour(DatabaseFin.FinOutline.ChainPoints);
                DatabaseFin.FinOutline = new Outline(newContour, Database.CatalogScheme.FeatureSetType, DatabaseFin.FinImage, DatabaseFin.Scale);

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

            if (CoordinateFeaturePoints == null)
                CoordinateFeaturePoints = new ObservableNotifiableCollection<CoordinateFeaturePoint>();
            CoordinateFeaturePoints.Clear();

            if (ComparisonFin == null)
            {
                if (DatabaseFin.FinOutline != null &&
                    DatabaseFin.FinOutline.FeatureSet != null &&
                    DatabaseFin.FinOutline.FeatureSet.CoordinateFeaturePointList != null)
                {
                    foreach (var coordFeature in DatabaseFin.FinOutline.FeatureSet.CoordinateFeaturePointList)
                    {
                        var featureCopy = new CoordinateFeaturePoint(coordFeature);
                        featureCopy.Coordinate.X -= clippedContour.XMin;
                        featureCopy.Coordinate.Y -= clippedContour.YMin;
                        CoordinateFeaturePoints.Add(featureCopy);
                    }
                }

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
