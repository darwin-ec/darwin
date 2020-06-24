using Darwin.Database;
using Newtonsoft.Json.Converters;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace Darwin.Wpf.ViewModel
{
    public class CurrentCatalogSchemeViewModel :INotifyPropertyChanged
    {
        private CatalogScheme _selectedScheme;
        public CatalogScheme SelectedScheme
        {
            get => _selectedScheme;
            set
            {
                _selectedScheme = value;

                RaisePropertyChanged("SelectedScheme");

                SelectedCategory = _selectedScheme.Categories?.FirstOrDefault();
            }
        }

        private Category _selectedCategory;
        public Category SelectedCategory
        {
            get => _selectedCategory;
            set
            {
                _selectedCategory = value;
                RaisePropertyChanged("SelectedCategory");
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

        public event PropertyChangedEventHandler PropertyChanged;

        public CurrentCatalogSchemeViewModel(DarwinDatabase database)
        {
            Database = database;

            SelectedScheme = new CatalogScheme(database.CatalogScheme);
        }

        public void SaveCatalogScheme()
        {
            Database.SetCatalogScheme(SelectedScheme);
        }

        public void AddCategory()
        {
            if (SelectedScheme == null)
                return;

            var category = new Category("<New Category>");
            SelectedScheme.Categories.Add(category);
            SelectedCategory = category;
        }

        public void RemoveCategory()
        {
            if (SelectedScheme == null || SelectedCategory == null)
                return;

            var idx = SelectedScheme.Categories.IndexOf(SelectedCategory);

            SelectedScheme.Categories.Remove(SelectedCategory);

            if (idx > SelectedScheme.Categories.Count - 1)
                idx = SelectedScheme.Categories.Count - 1;

            SelectedCategory = SelectedScheme.Categories[idx];
        }

        public void MoveSelectedCategoryUp()
        {
            if (SelectedScheme == null || SelectedCategory == null)
                return;

            var idx = SelectedScheme.Categories.IndexOf(SelectedCategory);

            if (idx > 0)
                SelectedScheme.Categories.Move(idx, idx - 1);
        }

        public void MoveSelectedCategoryDown()
        {
            if (SelectedScheme == null || SelectedCategory == null)
                return;

            var idx = SelectedScheme.Categories.IndexOf(SelectedCategory);

            if (idx < SelectedScheme.Categories.Count - 1)
                SelectedScheme.Categories.Move(idx, idx + 1);
        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
