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

using Darwin.Wavelet;
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
    /// Interaction logic for OutlineWindow.xaml
    /// </summary>
    public partial class OutlineWindow : Window
    {
        private OutlineWindowViewModel _vm;

        public OutlineWindow(OutlineWindowViewModel vm)
        {
            InitializeComponent();

            _vm = vm;

            this.DataContext = _vm;
        }

        private void CloseButton_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void GenerateCoefficientsButton_Click(object sender, RoutedEventArgs e)
        {
            using (var dialog = new System.Windows.Forms.FolderBrowserDialog())
            {
                dialog.Description = "Pick an output folder for the coefficient files.";
                System.Windows.Forms.DialogResult result = dialog.ShowDialog();

                if (result == System.Windows.Forms.DialogResult.OK)
                {
                    WaveletUtil.GenerateCoefficientFiles(
                        dialog.SelectedPath,
                        _vm.DatabaseFin.IDCode,
                        _vm.DatabaseFin.FinOutline.Chain.Data,
                        _vm.NumWaveletLevels);

                    MessageBox.Show("Coefficient File Generation Complete", "Generate Coefficient Files", MessageBoxButton.OK);
                }
            }
        }

        private void RediscoverFeaturesButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.RediscoverFeatures();
        }
    }
}
