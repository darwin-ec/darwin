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
    /// Interaction logic for FirstRunWindow.xaml
    /// </summary>
    public partial class FirstRunWindow : Window
    {
        private FirstRunViewModel _vm;

        public FirstRunWindow(FirstRunViewModel vm)
        {
            InitializeComponent();

            _vm = vm;
            this.DataContext = _vm;
        }

        private void NewDatabaseButton_Click(object sender, RoutedEventArgs e)
        {
            var mainWindow = this.Owner as MainWindow;

            if (mainWindow != null)
            {
                Close();
                mainWindow.NewDatabaseCommand_Executed(null, null);
            }
        }

        private void OpenDatabaseButton_Click(object sender, RoutedEventArgs e)
        {
            var mainWindow = this.Owner as MainWindow;

            if (mainWindow != null)
            {
                Close();
                mainWindow.OpenDatabaseCommand_Executed(null, null);
            }
        }
    }
}
