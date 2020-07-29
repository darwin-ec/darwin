using Darwin.ML;
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
