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
using Darwin.Wpf.Model;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;

namespace Darwin.Wpf.ViewModel
{
    public class NewDatabaseViewModel : INotifyPropertyChanged
    {
        private string _darwinHome;
        public string DarwinHome
        {
            get => _darwinHome;
            set
            { 
                _darwinHome = value;
                RaisePropertyChanged("DarwinHome");
                RaisePropertyChanged("DatabaseFilename");
                RefreshSurveyAreas();
            }
        }

        public string DatabaseFilename
        {
            get
            {
                string surveyArea;

                if (_newDatabaseSurveyAreaType == NewDatabaseSurveyAreaType.Existing)
                    surveyArea = _selectedSurveyArea;
                else
                    surveyArea = _newSurveyArea;

                if (string.IsNullOrEmpty(surveyArea))
                {
                    surveyArea = "[Enter Survey Area]";
                }

                return CatalogSupport.CalculateDatabaseFilename(DarwinHome, _databaseName, surveyArea);
            }
        }

        private NewDatabaseSurveyAreaType _newDatabaseSurveyAreaType;
        public NewDatabaseSurveyAreaType NewDatabaseSurveyAreaType
        {
            get => _newDatabaseSurveyAreaType;
            set
            {
                _newDatabaseSurveyAreaType = value;
                RaisePropertyChanged("NewDatabaseSurveyAreaType");

                if (_newDatabaseSurveyAreaType == NewDatabaseSurveyAreaType.New)
                {
                    NewSurveyAreaVisibility = Visibility.Visible;
                    ExistingSurveyAreaVisibility = Visibility.Collapsed;
                }
                else
                {
                    NewSurveyAreaVisibility = Visibility.Collapsed;
                    ExistingSurveyAreaVisibility = Visibility.Visible;
                }

                RaisePropertyChanged("DatabaseFilename");
            }
        }

        private string _selectedSurveyArea;
        public string SelectedSurveyArea
        {
            get => _selectedSurveyArea;
            set
            {
                _selectedSurveyArea = value;
                RaisePropertyChanged("SelectedSurveyArea");
                RaisePropertyChanged("DatabaseFilename");
            }
        }

        private string _newSurveyArea;
        public string NewSurveyArea
        {
            get => _newSurveyArea;
            set
            {
                _newSurveyArea = value;
                RaisePropertyChanged("NewSurveyArea");
                RaisePropertyChanged("DatabaseFilename");
            }
        }

        private ObservableCollection<string> _existingSurveyAreas;
        public ObservableCollection<string> ExistingSurveyAreas
        {
            get => _existingSurveyAreas;
            set
            {
                _existingSurveyAreas = value;
                RaisePropertyChanged("ExistingSurveyAreas");
                RaisePropertyChanged("DatabaseFilename");
            }
        }

        private ObservableCollection<CatalogScheme> _catalogSchemes;
        public ObservableCollection<CatalogScheme> CatalogSchemes
        {
            get => _catalogSchemes;
            set
            {
                _catalogSchemes = value;
                RaisePropertyChanged("CatalogSchemes");
                RaisePropertyChanged("DatabaseFilename");
            }
        }

        private CatalogScheme _selectedCatalogScheme;
        public CatalogScheme SelectedCatalogScheme
        {
            get => _selectedCatalogScheme;
            set
            {
                _selectedCatalogScheme = value;
                RaisePropertyChanged("SelectedCatalogScheme");
                RaisePropertyChanged("DatabaseFilename");
            }
        }

        private string _databaseName;
        public string DatabaseName
        {
            get => _databaseName;
            set
            {
                _databaseName = value;
                RaisePropertyChanged("DatabaseName");
                RaisePropertyChanged("DatabaseFilename");
            }
        }

        private Visibility _existingSurveyAreaVisibility;
        public Visibility ExistingSurveyAreaVisibility
        {
            get => _existingSurveyAreaVisibility;
            set
            {
                _existingSurveyAreaVisibility = value;
                RaisePropertyChanged("ExistingSurveyAreaVisibility");
            }
        }

        private Visibility _newSurveyAreaVisibility;
        public Visibility NewSurveyAreaVisibility
        {
            get => _newSurveyAreaVisibility;
            set
            {
                _newSurveyAreaVisibility = value;
                RaisePropertyChanged("NewSurveyAreaVisibility");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public NewDatabaseViewModel()
        {
            DarwinHome = Options.CurrentUserOptions.CurrentDarwinHome;
            NewSurveyArea = Options.DefaultSurveyAreaName;
            NewDatabaseSurveyAreaType = NewDatabaseSurveyAreaType.Existing;

            CatalogSchemes = new ObservableCollection<CatalogScheme>(Options.CurrentUserOptions.CatalogSchemes);
            SelectedCatalogScheme = CatalogSchemes.Where(cs => cs.IsDefault).FirstOrDefault();

            RefreshSurveyAreas();
        }

        private void RefreshSurveyAreas()
        {
            var existingSurveyAreas = CatalogSupport.GetExistingSurveyAreas(DarwinHome);

            if (existingSurveyAreas == null)
            {
                ExistingSurveyAreas = new ObservableCollection<string>();
                NewDatabaseSurveyAreaType = NewDatabaseSurveyAreaType.New;
            }
            else
            {
                ExistingSurveyAreas = new ObservableCollection<string>(existingSurveyAreas);
                SelectedSurveyArea = ExistingSurveyAreas.FirstOrDefault();

                if (SelectedSurveyArea == null)
                    NewDatabaseSurveyAreaType = NewDatabaseSurveyAreaType.New;
            }
        }

        public string CreateNewDatabase()
        {
            string surveyArea;

            if (NewDatabaseSurveyAreaType == NewDatabaseSurveyAreaType.Existing)
                surveyArea = SelectedSurveyArea;
            else
                surveyArea = NewSurveyArea;

            if (string.IsNullOrEmpty(surveyArea))
            {
                if (NewDatabaseSurveyAreaType == NewDatabaseSurveyAreaType.Existing)
                    throw new Exception("Please select an existing survey area, or create a new survey area if there aren't any existing survey areas to select.");
                else
                    throw new Exception("Please enter a survey area name.");
            }

            if (SelectedCatalogScheme == null)
                throw new Exception("Please select a catalog scheme.");

            if (string.IsNullOrEmpty(DatabaseName))
                throw new Exception("Please enter a database name.");

            var db = CatalogSupport.CreateAndOpenDatabase(DatabaseFilename, surveyArea, SelectedCatalogScheme);

            return db.Filename;
        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
