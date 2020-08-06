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
    /// Interaction logic for CurrentCatalogSchemeWindow.xaml
    /// </summary>
    public partial class CurrentCatalogSchemeWindow : Window
    {
        private CurrentCatalogSchemeViewModel _vm;
        public CurrentCatalogSchemeWindow(CurrentCatalogSchemeViewModel vm)
        {
            InitializeComponent();

            _vm = vm;
            DataContext = _vm;
        }

        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.SaveCatalogScheme();
            MainWindow mainWindow = Application.Current.MainWindow as MainWindow;

            if (mainWindow != null)
                mainWindow.RefreshDatabase();

            Close();
        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

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

        private void AddCategoryNameButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.AddCategory();
        }

        private void RemoveCategoryNameButton_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show(this, "Sorry, you can't currently remove a category from " + Environment.NewLine +
                "the current database.", "Can't Remove", MessageBoxButton.OK, MessageBoxImage.Information);
            //_vm.RemoveCategory();
        }

        private void UpCategoryNameButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.MoveSelectedCategoryUp();
        }

        private void DownCategoryNameButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.MoveSelectedCategoryDown();
        }

        private void RediscoverDatabaseFeatures_Click(object sender, RoutedEventArgs e)
        {
            var result = MessageBox.Show("Warning: This will overwrite all features with algorithmically discovered feature points.  There is no undo on this feature."
                + Environment.NewLine + Environment.NewLine +
                "Are you sure you want to continue?", "Warning", MessageBoxButton.YesNo, MessageBoxImage.Exclamation);

            if (result == MessageBoxResult.Yes)
            {
                _vm.RediscoverAllFeatures();

                MessageBox.Show("Feature discovery complete." + Environment.NewLine + "Your database has been updated.",
                    "Complete", MessageBoxButton.OK, MessageBoxImage.Information);
                MainWindow mainWindow = Application.Current.MainWindow as MainWindow;

                if (mainWindow != null)
                    mainWindow.RefreshDatabase();
            }
        }
    }
}
