using Darwin.Database;
using Darwin.Wpf.Model;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows;

namespace Darwin.Wpf.ViewModel
{
    public class NewDatabaseViewModel : INotifyPropertyChanged
    {
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
            NewDatabaseSurveyAreaType = NewDatabaseSurveyAreaType.Existing;

            CatalogSchemes = new ObservableCollection<CatalogScheme>(Options.CurrentUserOptions.CatalogSchemes);
            SelectedCatalogScheme = CatalogSchemes.Where(cs => cs.IsDefault).FirstOrDefault();

            var existingSurveyAreas = CatalogSupport.GetExistingSurveyAreas();

            if (existingSurveyAreas == null)
            {
                ExistingSurveyAreas = new ObservableCollection<string>();
            }
            else
            {
                ExistingSurveyAreas = new ObservableCollection<string>(existingSurveyAreas);
                SelectedSurveyArea = ExistingSurveyAreas.FirstOrDefault();
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
