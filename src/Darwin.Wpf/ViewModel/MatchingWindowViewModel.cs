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

using Darwin.Database;
using Darwin.Features;
using Darwin.Matching;
using Darwin.Wpf.Model;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Text;
using System.Windows;

namespace Darwin.Wpf.ViewModel
{
    public class MatchingWindowViewModel : INotifyPropertyChanged
    {
        private ObservableCollection<Category> _categories;
        public ObservableCollection<Category> Categories
        {
            get => _categories;
            set
            {
                _categories = value;
                RaisePropertyChanged("Categories");
            }
        }

        private ObservableCollection<Category> _selectableCategories;
        public ObservableCollection<Category> SelectableCategories
        {
            get => _selectableCategories;
            set
            {
                _selectableCategories = value;
                RaisePropertyChanged("SelectableCategories");
            }
        }

        private RegistrationMethodType _registrationMethod;
        public RegistrationMethodType RegistrationMethod
        {
            get => _registrationMethod;
            set
            {
                _registrationMethod = value;
                RaisePropertyChanged("RegistrationMethod");
            }
        }

        private RangeOfPointsType _rangeOfPoints;
        public RangeOfPointsType RangeOfPoints
        {
            get => _rangeOfPoints;
            set
            {
                _rangeOfPoints = value;
                RaisePropertyChanged("RangeOfPoints");
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

        private bool _showRegistration;
        public bool ShowRegistration
        {
            get => _showRegistration;
            set
            {
                _showRegistration = value;
                RaisePropertyChanged("ShowRegistration");
            }
        }

        private bool _matchRunning;
        public bool MatchRunning
        {
            get => _matchRunning;
            set
            {
                _matchRunning = value;
                RaisePropertyChanged("MatchRunning");
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

        private int _matchProgressPercent;
        public int MatchProgressPercent
        {
            get => _matchProgressPercent;
            set
            {
                _matchProgressPercent = value;
                RaisePropertyChanged("MatchProgressPercent");
            }
        }

        private Match _match;
        public Match Match
        {
            get => _match;
            set
            {
                _match = value;
                RaisePropertyChanged("Match");
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

        private Visibility _progressBarVisibility;
        public Visibility ProgressBarVisibility
        {
            get => _progressBarVisibility;
            set
            {
                _progressBarVisibility = value;
                RaisePropertyChanged("ProgressBarVisibility");
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

        //private double _contourXOffset;
        //public double ContourXOffset
        //{
        //    get => _contourXOffset;
        //    set
        //    {
        //        _contourXOffset = value;
        //        RaisePropertyChanged("ContourXOffset");
        //    }
        //}

        //private double _contourYOffset;
        //public double ContourYOffset
        //{
        //    get => _contourYOffset;
        //    set
        //    {
        //        _contourYOffset = value;
        //        RaisePropertyChanged("ContourYOffset");
        //    }
        //}

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

        public event PropertyChangedEventHandler PropertyChanged;

        public MatchingWindowViewModel(DatabaseFin databaseFin,
            DarwinDatabase database)
        {
            DatabaseFin = databaseFin;
            Categories = database.Categories;
            RegistrationMethod = RegistrationMethodType.TrimOptimalTip;
            RangeOfPoints = RangeOfPointsType.AllPoints;
            Database = database;
            ShowRegistration = false;

            // TODO: These should really come from the window
            ContourWidth = 250;
            ContourHeight = 250;

            if (database.CatalogScheme.FeatureSetType == Features.FeatureSetType.DorsalFin)
            {
                Match = new Match(DatabaseFin,
                    Database,
                    UpdateOutlines,
                    RegistrationMethod,
                    (RangeOfPoints == RangeOfPointsType.AllPoints) ? true : false);
            }
            else
            {
                Match = new Match(DatabaseFin,
                    Database,
                    UpdateOutlines,
                    null,
                    true);
            }

            UpdateOutlines(DatabaseFin.FinOutline.ChainPoints, null);

            ProgressBarVisibility = Visibility.Hidden;

            InitializeSelectableCategories();
        }

        private void UpdateOutlines(FloatContour unknownContour, FloatContour dbContour)
        {
            if (!ShowRegistration || (unknownContour == null && dbContour == null))
                return;

            Contour unk, db;
            double xOffset, yOffset;
            FloatContour.FitContoursToSize(ContourWidth, ContourHeight, unknownContour, dbContour, out unk, out db, out xOffset, out yOffset);

            if (unk != null)
            {
                unk.XOffset = xOffset;
                unk.YOffset = yOffset;
            }

            if (db != null)
            {
                db.XOffset = xOffset;
                db.YOffset = yOffset;
            }

            UnknownContour = unk;
            DBContour = db;
        }

        private void InitializeSelectableCategories()
        {
            SelectableCategories = new ObservableCollection<Category>();

            if (Categories != null)
            {
                foreach (var cat in Categories)
                {
                    SelectableCategories.Add(new Category
                    {
                        IsSelected = true,
                        Name = cat.Name
                    });
                }
            }
        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
