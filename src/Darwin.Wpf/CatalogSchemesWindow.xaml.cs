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

using Darwin.Wpf.Helpers;
using Darwin.Wpf.ViewModel;
using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace Darwin.Wpf
{
    /// <summary>
    /// Interaction logic for CatalogSchemesWindow.xaml
    /// </summary>
    public partial class CatalogSchemesWindow : Window
    {
        private CatalogSchemesViewModel _vm;

        public CatalogSchemesWindow(CatalogSchemesViewModel vm)
        {
            InitializeComponent();

            _vm = vm;

            DataContext = _vm;
        }

        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            Options.CurrentUserOptions.CatalogSchemes = new List<Database.CatalogScheme>(_vm.CatalogSchemes);
            Options.CurrentUserOptions.Save();
            Close();
        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void AddButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.AddCatalogScheme();

            //ListBoxItem selectedItem = SchemesListBox.ItemContainerGenerator.ContainerFromIndex(SchemesListBox.SelectedIndex - 1) as ListBoxItem;

            //if (selectedItem != null)
            //{
            //    ContentPresenter contentPresenter = FindVisualChild<ContentPresenter>(selectedItem);
            //    DataTemplate dataTemplate = contentPresenter.ContentTemplate;
            //    TextBox textBox = dataTemplate.FindName("CatalogSchemeName", contentPresenter) as TextBox;

            //    if (textBox != null)
            //        textBox.Dispatcher.BeginInvoke(new Action(() => textBox.SelectAll()));
            //}
        }

        // This is used by both ListBox controls
        private void ListBoxItem_SelectCurrentItem(object sender, KeyboardFocusChangedEventArgs e)
        {
            ListBoxItem item = sender as ListBoxItem;

            if (item != null && !item.IsSelected)
                item.IsSelected = true;

            if (item != null)
            {
                ContentPresenter contentPresenter = ControlHelper.FindVisualChild<ContentPresenter>(item);
                DataTemplate dataTemplate = contentPresenter.ContentTemplate;
                TextBox textBox = dataTemplate.FindName("CatalogSchemeName", contentPresenter) as TextBox;

                if (textBox == null)
                    textBox = dataTemplate.FindName("CategoryNameTextBox", contentPresenter) as TextBox;

                if (textBox != null)
                    textBox.Dispatcher.BeginInvoke(new Action(() => textBox.SelectAll()));
            }
        }

        private void RemoveButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.SelectedScheme != null && _vm.SelectedScheme.IsBuiltIn)
            {
                MessageBox.Show("You can't remove the Eckerd College scheme.", "Can't Remove", MessageBoxButton.OK, MessageBoxImage.Warning);
            }
            else
            {
                _vm.RemoveSelectedScheme();
            }
        }

        private void SetDefaultButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.SetSelectedDefault();
        }

        private void UpButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.MoveSelectedUp();
        }

        private void DownButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.MoveSelectedDown();
        }

        private void AddCategoryNameButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.AddCategory();
        }

        private void RemoveCategoryNameButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.RemoveCategory();
        }

        private void UpCategoryNameButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.MoveSelectedCategoryUp();
        }

        private void DownCategoryNameButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.MoveSelectedCategoryDown();
        }
    }
}
