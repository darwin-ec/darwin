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

        public void RediscoverAllFeatures()
        {
            if (Database != null)
            {
                foreach (var individual in Database.AllFins)
                {
                    var newFeatureOutline = new Outline(individual.FinOutline.ChainPoints, SelectedScheme.FeatureSetType);
                    newFeatureOutline.RediscoverFeaturePoints(SelectedScheme.FeatureSetType, individual);
                    individual.FinOutline = newFeatureOutline;
                    Database.UpdateOutline(individual, true);
                }

                Database.InvalidateCache();
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
