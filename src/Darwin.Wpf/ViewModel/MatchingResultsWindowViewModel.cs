﻿// This file is part of DARWIN.
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

using Darwin.Database;
using Darwin.Helpers;
using Darwin.Matching;
using Darwin.Wpf.Extensions;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Media;

namespace Darwin.Wpf.ViewModel
{
    public class MatchingResultsWindowViewModel : INotifyPropertyChanged
    {
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

        private Result _selectedResult;
        public Result SelectedResult
        {
            get => _selectedResult;
            set
            {
                _selectedResult = value;
                RaisePropertyChanged("SelectedResult");

                LoadSelectedResult();
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

        private ImageSource _unknownImageSource;
        public ImageSource UnknownImageSource
        {
            get => _unknownImageSource;
            set
            {
                _unknownImageSource = value;
                RaisePropertyChanged("UnknownImageSource");
            }
        }

        private bool _showIDColumn;
        public bool ShowIDColumn
        {
            get => _showIDColumn;
            set
            {
                _showIDColumn = value;
                RaisePropertyChanged("ShowIDColumn");
            }
        }

        private bool _showInfoColumns;
        public bool ShowInfoColumns
        {
            get => _showInfoColumns;
            set
            {
                _showInfoColumns = value;
                RaisePropertyChanged("ShowInfoColumns");
            }
        }

        private bool _nextEnabled;
        public bool NextEnabled
        {
            get => _nextEnabled;
            set
            {
                _nextEnabled = value;
                RaisePropertyChanged("NextEnabled");
            }
        }

        private bool _previousEnabled;
        public bool PreviousEnabled
        {
            get => _previousEnabled;
            set
            {
                _previousEnabled = value;
                RaisePropertyChanged("PreviousEnabled");
            }
        }

        private bool _unknownShowOriginalImage;
        public bool UnknownShowOriginalImage
        {
            get => _unknownShowOriginalImage;
            set
            {
                _unknownShowOriginalImage = value;
                RaisePropertyChanged("UnknownShowOriginalImage");

                if (_unknownShowOriginalImage && DatabaseFin.OriginalFinImage != null)
                    UnknownImageSource = DatabaseFin.OriginalFinImage.ToImageSource();
                else
                    UnknownImageSource = DatabaseFin.FinImage.ToImageSource();
            }
        }

        private bool _selectedShowOriginalImage;
        public bool SelectedShowOriginalImage
        {
            get => _selectedShowOriginalImage;
            set
            {
                _selectedShowOriginalImage = value;
                RaisePropertyChanged("SelectedShowOriginalImage");
                LoadSelectedResult();
            }
        }

        public int CurrentSelectedIndex
        {
            get
            {
                if (SelectedResult == null)
                    return -1;

                return MatchResults.Results.IndexOf(SelectedResult);
            }
        }

        private Contour _unknownContour;
        public Contour UnknownContour
        {
            get => _unknownContour;
            set
            {
                _unknownContour = value;
                RaisePropertyChanged("UnknownContour");
            }
        }

        private Contour _dbContour;
        public Contour DBContour
        {
            get => _dbContour;
            set
            {
                _dbContour = value;
                RaisePropertyChanged("DBContour");
            }
        }

        private double _contourXOffset;
        public double ContourXOffset
        {
            get => _contourXOffset;
            set
            {
                _contourXOffset = value;
                RaisePropertyChanged("ContourXOffset");
            }
        }

        private double _contourYOffset;
        public double ContourYOffset
        {
            get => _contourYOffset;
            set
            {
                _contourYOffset = value;
                RaisePropertyChanged("ContourYOffset");
            }
        }

        private double _contourWidth;
        public double ContourWidth
        {
            get => _contourWidth;
            set
            {
                _contourWidth = value;
                RaisePropertyChanged("ContourWidth");
            }
        }

        private double _contourHeight;
        public double ContourHeight
        {
            get => _contourHeight;
            set
            {
                _contourHeight = value;
                RaisePropertyChanged("ContourHeight");
            }
        }

        public string IndividualTerminology
        {
            get
            {
                return Database.CatalogScheme.IndividualTerminology;
            }
        }

        public string IndividualTerminologyInitialCaps
        {
            get
            {
                return Database.CatalogScheme.IndividualTerminologyInitialCaps;
            }
        }

        public string SelectedLabelText
        {
            get
            {
                return "Selected " + Database.CatalogScheme.IndividualTerminologyInitialCaps;
            }
        }

        public string UnknownLabelText
        {
            get
            {
                return "Unknown " + Database.CatalogScheme.IndividualTerminologyInitialCaps;
            }
        }

        public string MatchSelectedOrientationText
        {
            get
            {
                return "Match Selected " + Database.CatalogScheme.IndividualTerminologyInitialCaps + " Orientation";
            }
        }

        public string MatchesText
        {
            get
            {
                return "Matches Selected " + Database.CatalogScheme.IndividualTerminologyInitialCaps;
            }
        }

        public string NoMatchText
        {
            get
            {
                return "No Match - New  " + Database.CatalogScheme.IndividualTerminologyInitialCaps;
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

            // TODO: These should really come from the window
            ContourWidth = 200;
            ContourHeight = 200;

            ShowIDColumn = true;
            ShowInfoColumns = true;
            AutoScroll = false;

            NextEnabled = true;
            PreviousEnabled = true;

            DatabaseFin = unknownFin;

            if (DatabaseFin != null && DatabaseFin.FinImage != null)
                UnknownImageSource = DatabaseFin.FinImage.ToImageSource();

            MatchResults = matchResults;
            Database = database;

            if (MatchResults.Results != null && MatchResults.Results.Count > 0)
            {
                SelectedResult = MatchResults.Results[0];
                LoadSelectedResult();
            }
        }

        public DatabaseFin FullyLoadFinByID(long id)
        {
            DatabaseFin finCopy = null;

            var dbFin = Database.AllFins.Where(f => f.ID == id).FirstOrDefault();

            if (dbFin != null)
                finCopy = CatalogSupport.FullyLoadFin(dbFin);

            return finCopy;
        }

        private void LoadSelectedResult()
        {
            if (SelectedResult != null)
            {
                UpdateOutlines(SelectedResult.unknownContour, SelectedResult.dbContour);

                DatabaseFin selectedFin = FullyLoadFinByID(SelectedResult.DatabaseID);

                if (SelectedShowOriginalImage)
                    SelectedImageSource = selectedFin.OriginalFinImage.ToImageSource();
                else
                    SelectedImageSource = selectedFin.FinImage.ToImageSource();
            }
        }

        public void SaveMatchResults(out string finzSaveFilename, out string resultsSaveFilename)
        {
            string baseFilename = (string.IsNullOrEmpty(DatabaseFin.FinFilename)) ? DatabaseFin.ImageFilename : DatabaseFin.FinFilename;
            finzSaveFilename = Path.Combine(Options.CurrentUserOptions.CurrentTracedFinsPath, Path.GetFileNameWithoutExtension(baseFilename) + ".finz");
            finzSaveFilename = CatalogSupport.SaveFinz(Database.CatalogScheme, DatabaseFin, finzSaveFilename, false);

            string resultsFilename = Path.GetFileNameWithoutExtension(Database.Filename) + "-DB-match-for-" + Path.GetFileNameWithoutExtension(baseFilename) + ".res";
            resultsSaveFilename = Path.Combine(Options.CurrentUserOptions.CurrentMatchQueueResultsPath, resultsFilename);
            MatchResults.Save(resultsSaveFilename);
        }

        private void UpdateOutlines(FloatContour unknownContour, FloatContour dbContour)
        {
            if (unknownContour == null || dbContour == null)
                return;

            Contour unk, db;
            double xOffset, yOffset;
            FloatContour.FitContoursToSize(ContourWidth, ContourHeight, unknownContour, dbContour, out unk, out db, out xOffset, out yOffset);

            ContourXOffset = xOffset;
            ContourYOffset = yOffset;
            UnknownContour = unk;
            DBContour = db;
        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
