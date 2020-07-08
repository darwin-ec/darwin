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
