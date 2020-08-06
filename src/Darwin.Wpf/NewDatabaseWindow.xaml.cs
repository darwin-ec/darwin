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
    /// Interaction logic for NewDatabaseWindow.xaml
    /// </summary>
    public partial class NewDatabaseWindow : Window
    {
        private NewDatabaseViewModel _vm;

        public NewDatabaseWindow(NewDatabaseViewModel vm)
        {
            InitializeComponent();

            _vm = vm;
            DataContext = _vm;
        }

        private void OKButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var fullDatabaseName = _vm.CreateNewDatabase();

                MessageBox.Show("Your database has been created." + Environment.NewLine +
                    "It will be loaded now.", "Database Created", MessageBoxButton.OK, MessageBoxImage.Information);

                MainWindow mainWindow = Application.Current.MainWindow as MainWindow;

                if (mainWindow != null)
                    mainWindow.OpenDatabase(fullDatabaseName, true);

                Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error Creating Database", MessageBoxButton.OK, MessageBoxImage.Exclamation);
            }
        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void RadioButton_Click(object sender, RoutedEventArgs e)
        {
            NewSurveyAreaTextbox.Focus();
        }

        private void BrowseDarwinHome_Click(object sender, RoutedEventArgs e)
        {
            using (var dialog = new System.Windows.Forms.FolderBrowserDialog())
            {
                dialog.SelectedPath = _vm.DarwinHome;
                dialog.Description = "Pick a Folder for your DARWIN Home";
                System.Windows.Forms.DialogResult result = dialog.ShowDialog();

                if (result == System.Windows.Forms.DialogResult.OK)
                {
                    _vm.DarwinHome = dialog.SelectedPath;
                }
            }
        }
    }
}
