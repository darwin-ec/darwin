using Darwin.Wpf.Commands;
using Darwin.Wpf.ViewModel;
using Darwin.Database;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Drawing;
using Darwin.Collections;
using System.ComponentModel;

namespace Darwin.Wpf
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private MainWindowViewModel _vm;

        public MainWindow()
        {
            InitializeComponent();

            _vm = new MainWindowViewModel();

            if (!string.IsNullOrEmpty(Options.CurrentUserOptions.DatabaseFileName))
                OpenDatabase(Options.CurrentUserOptions.DatabaseFileName, false);

            this.DataContext = _vm;
        }

        private void GridHeader_Click(object sender, RoutedEventArgs e)
        {
            var sortableListViewSender = sender as Controls.SortableListView;

            if (sortableListViewSender != null)
                sortableListViewSender.GridViewColumnHeaderClickedHandler(sender, e);
        }

        private void OpenImageCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void OpenImageCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var openDialog = new OpenFileDialog();
            openDialog.Filter = CustomCommands.OpenImageFilter;
            if (openDialog.ShowDialog() == true)
            {
                var img = System.Drawing.Image.FromFile(openDialog.FileName);

                var bitmap = new Bitmap(img);
                // TODO: Hack for HiDPI -- this should be more intelligent.
                bitmap.SetResolution(96, 96);

                // TODO: Move this logic into the constructor?
                var vm = new TraceWindowViewModel(bitmap);

                TraceWindow traceWindow = new TraceWindow(vm);
                traceWindow.Show();
            }
        }

        private void OpenDatabaseCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void OpenDatabaseCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var openDatabaseDialog = new OpenFileDialog();
            openDatabaseDialog.Filter = CustomCommands.OpenDatabaseFilter;
            openDatabaseDialog.InitialDirectory = Options.CurrentUserOptions.CurrentDataPath;

            if (openDatabaseDialog.ShowDialog() == true)
            {
                OpenDatabase(openDatabaseDialog.FileName, true);
            }
        }

        private void OpenDatabase(string filename, bool saveOptions = false)
        {
            var db = CatalogSupport.OpenDatabase(filename, Options.CurrentUserOptions, false);
            _vm.DarwinDatabase = db;
            _vm.Fins = new ObservableNotifiableCollection<DatabaseFin>(_vm.DarwinDatabase.GetAllFins());

            if (saveOptions)
            {
                // TODO: Some more logic around this
                Options.CurrentUserOptions.SetLastDatabaseFilename(filename);

                Options.CurrentUserOptions.Save();
            }
        }

        private void ExitCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void ExitCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void DeleteFin_Click(object sender, RoutedEventArgs e)
        {
            if (MessageBox.Show("Are you sure you want to delete this fin from the database?", "Delete Confirmation", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
            {
                // TODO: Delete
            }
        }

        private void SaveFinData_Click(object sender, RoutedEventArgs e)
        {
            // TODO: Save the fin
        }
    }
}
