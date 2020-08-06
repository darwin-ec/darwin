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

using Darwin.Utilities;
using Darwin.Wpf.Controls;
using Darwin.Wpf.ViewModel;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Darwin.Wpf
{
    /// <summary>
    /// Interaction logic for AboutWindow.xaml
    /// </summary>
    public partial class AboutWindow : CustomWindow
    {
        public string Version
        {
            get
            {
                // Assembly Version
                //var version = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;

                // Assembly File Version
                var version = FileVersionInfo.GetVersionInfo(System.Reflection.Assembly.GetExecutingAssembly().Location).FileVersion;
                return version.ToString();
            }
        }

        public AboutWindow()
        {
            InitializeComponent();

            DataContext = this;
        }

        protected override void OnSourceInitialized(EventArgs e)
        {
            HideWindowIcon();
            base.OnSourceInitialized(e);
        }

        private void DarwinWebsiteHyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
           string navigateUri = e.Uri.ToString();
           UrlHelper.OpenUrl(navigateUri);
           e.Handled = true;
        }

        private void OKButton_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void AboutWindow_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
                Close();
        }

        private void LegalHyperlink_Click(object sender, RoutedEventArgs e)
        {
            var vm = new LegalViewModel();
            var window = new LegalWindow(vm);
            window.Owner = this;
            window.ShowDialog();
        }
    }
}