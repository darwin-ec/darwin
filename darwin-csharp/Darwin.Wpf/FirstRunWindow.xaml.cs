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
