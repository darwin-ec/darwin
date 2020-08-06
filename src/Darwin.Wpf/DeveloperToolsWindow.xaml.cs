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

using Darwin.Helpers;
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
    /// Interaction logic for DeveloperToolsWindow.xaml
    /// </summary>
    public partial class DeveloperToolsWindow : Window
    {
        private DeveloperToolsViewModel _vm;
        public DeveloperToolsWindow(DeveloperToolsViewModel vm)
        {
            InitializeComponent();

            _vm = vm;
            this.DataContext = _vm;
        }

        private void CloseButton_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void ExportDatasetButton_Click(object sender, RoutedEventArgs e)
        {
            using (var dialog = new System.Windows.Forms.FolderBrowserDialog())
            {
                dialog.Description = "Pick an output folder for the training dataset";
                System.Windows.Forms.DialogResult result = dialog.ShowDialog();

                if (result == System.Windows.Forms.DialogResult.OK)
                {
                    try
                    {
                        this.IsHitTestVisible = false;
                        Mouse.OverrideCursor = Cursors.Wait;

                        MLSupport.SaveDatasetImages(dialog.SelectedPath, _vm.Database);

                        MessageBox.Show("Dataset generation complete.", "Complete", MessageBoxButton.OK, MessageBoxImage.Information);
                    }
                    finally
                    {
                        Mouse.OverrideCursor = null;
                        this.IsHitTestVisible = true;
                    }
                }
            }
		}
    }
}
