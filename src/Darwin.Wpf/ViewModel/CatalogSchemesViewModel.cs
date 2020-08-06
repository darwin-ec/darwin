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
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Linq;
using System.Text;

namespace Darwin.Wpf.ViewModel
{
    public class CatalogSchemesViewModel : INotifyPropertyChanged
    {
        private ObservableNotifiableCollection<CatalogScheme> _catalogSchemes;
        public ObservableNotifiableCollection<CatalogScheme> CatalogSchemes
        {
            get => _catalogSchemes;
            set
            {
                _catalogSchemes = value;
                RaisePropertyChanged("CatalogSchemes");
            }
        }

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

        public event PropertyChangedEventHandler PropertyChanged;

        public CatalogSchemesViewModel(List<CatalogScheme> schemes)
        {
            CatalogSchemes = new ObservableNotifiableCollection<CatalogScheme>(schemes);

            if (CatalogSchemes != null && CatalogSchemes.Count > 0)
                SelectedScheme = CatalogSchemes[0];
        }

        public void AddCatalogScheme()
        {
            var newScheme = new CatalogScheme
            {
                SchemeName = "<New Scheme>",
                Categories = new System.Collections.ObjectModel.ObservableCollection<Category>(),
                IsDefault = false,
                IsBuiltIn = false
            };

            CatalogSchemes.Add(newScheme);

            SelectedScheme = newScheme;
        }

        public void RemoveSelectedScheme()
        {
            if (SelectedScheme == null)
                return;

            if (SelectedScheme.IsBuiltIn)
                throw new Exception("Can't remove built-in scheme!");

            var idx = CatalogSchemes.IndexOf(SelectedScheme);

            CatalogSchemes.Remove(SelectedScheme);

            if (idx > CatalogSchemes.Count - 1)
                idx = CatalogSchemes.Count - 1;

            SelectedScheme = CatalogSchemes[idx];

            if (!CatalogSchemes.Any(cs => cs.IsDefault))
            {
                var builtinScheme = CatalogSchemes.Where(cs => cs.IsBuiltIn).FirstOrDefault();

                if (builtinScheme != null)
                    builtinScheme.IsDefault = true;
            }
        }

        public void SetSelectedDefault()
        {
            if (SelectedScheme == null)
                return;

            foreach (var scheme in CatalogSchemes.Where(cs => cs.IsDefault))
                scheme.IsDefault = false;

            SelectedScheme.IsDefault = true;
        }

        public void MoveSelectedUp()
        {
            if (SelectedScheme == null)
                return;

            var idx = CatalogSchemes.IndexOf(SelectedScheme);

            if (idx > 0)
                CatalogSchemes.Move(idx, idx - 1);
        }

        public void MoveSelectedDown()
        {
            if (SelectedScheme == null)
                return;

            var idx = CatalogSchemes.IndexOf(SelectedScheme);

            if (idx < CatalogSchemes.Count - 1)
                CatalogSchemes.Move(idx, idx + 1);
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
