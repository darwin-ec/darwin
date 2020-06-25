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
                    var s = dialog.SelectedPath;

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
