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
using Darwin.Utilities;
using System.Diagnostics;

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

            try
            {
                if (!string.IsNullOrEmpty(Options.CurrentUserOptions.DatabaseFileName))
                    OpenDatabase(Options.CurrentUserOptions.DatabaseFileName, false);
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex);
                MessageBox.Show("There was a problem opening the database. It may have been moved or deleted.\n\n" +
                    "Please use Open Database to find an existing database, or use New Database to create a new one.",
                    "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }

            this.DataContext = _vm;
        }

        private void GridHeader_Click(object sender, RoutedEventArgs e)
        {
            var sortableListViewSender = sender as Controls.SortableListView;

            if (sortableListViewSender != null)
                sortableListViewSender.GridViewColumnHeaderClickedHandler(sender, e);
        }

        private void NewDatabaseCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void NewDatabaseCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var newDatabaseVM = new NewDatabaseViewModel();

            var newDatabaseWindow = new NewDatabaseWindow(newDatabaseVM);
            newDatabaseWindow.Owner = this;
            newDatabaseWindow.ShowDialog();
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
                // TODO: Move this logic into the constructor?
                var vm = new TraceWindowViewModel(openDialog.FileName, _vm.DarwinDatabase);

                TraceWindow traceWindow = new TraceWindow(vm);
                traceWindow.Show();
            }
        }

        private void OpenTracedFinCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void OpenTracedFinCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var openDialog = new OpenFileDialog();
            openDialog.Filter = CustomCommands.TracedFinFilter;
            if (openDialog.ShowDialog() == true)
            {
                var fin = CatalogSupport.OpenFinz(openDialog.FileName);

                // TODO: Better error messages?
                if (fin == null)
                {
                    MessageBox.Show("Problem opening finz file.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
                else
                {
                    var vm = new TraceWindowViewModel(fin,
                        _vm.DarwinDatabase);

                    TraceWindow traceWindow = new TraceWindow(vm);
                    traceWindow.Show();
                }
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
            // TODO: The thumbnail part is temporary
            // Need to store the thumbnail images more nicely
            _vm.Fins = new ObservableNotifiableCollection<DatabaseFin>(
                _vm.DarwinDatabase
                .GetAllFins()
                .Select(x => { x.ThumbnailFilename = x.ImageFilename; return x; })
                .ToList());

            if (_vm.Fins?.Count > 0)
                _vm.SelectedFin = _vm.Fins[0];

            if (saveOptions)
            {
                // TODO: Some more logic around this
                Options.CurrentUserOptions.SetLastDatabaseFilename(filename);

                Options.CurrentUserOptions.Save();
            }
        }

        public void RefreshDatabaseAfterAdd()
        {
            _vm.Fins = new ObservableNotifiableCollection<DatabaseFin>(
                _vm.DarwinDatabase
                .GetAllFins()
                .Select(x => { x.ThumbnailFilename = x.ImageFilename; return x; })
                .ToList());

            // Select the last fin, and scroll to it so the user can see it
            if (_vm.Fins != null)
            {
                _vm.SelectedFin = _vm.Fins[_vm.Fins.Count - 1];
                DatabaseGrid.ScrollIntoView(_vm.SelectedFin);
            }
        }

        private void MatchingQueueCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void MatchingQueueCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var matchingQueueVM = new MatchingQueueViewModel();

            var matchingQueueWindow = new MatchingQueueWindow(matchingQueueVM);
            matchingQueueWindow.Owner = this;
            matchingQueueWindow.WindowStartupLocation = WindowStartupLocation.CenterOwner;
            matchingQueueWindow.Show();
        }

        private void ImportFinCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void ImportFinCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var openDialog = new OpenFileDialog();
            openDialog.Title = "Select .finz to Import";
            openDialog.InitialDirectory = Options.CurrentUserOptions.CurrentTracedFinsPath;
            openDialog.Filter = CustomCommands.TracedFinFilter;

            if (openDialog.ShowDialog() == true)
            {
                try
                {
                    var fin = CatalogSupport.OpenFinz(openDialog.FileName);
                    CatalogSupport.SaveToDatabase(_vm.DarwinDatabase, fin);
                    RefreshDatabaseAfterAdd();
                }
                catch (Exception ex)
                {
                    Trace.WriteLine(ex);
                    MessageBox.Show("Sorry, there was a problem importing the finz file.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
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

        private void OptionsCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void OptionsCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var optionsVM = new OptionsWindowViewModel(Options.CurrentUserOptions);

            var optionsWindow = new OptionsWindow(optionsVM);
            optionsWindow.Owner = this;
            optionsWindow.ShowDialog();
        }

        private void CatalogSchemesCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CatalogSchemesCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var catalogSchemesVM = new CatalogSchemesViewModel(Options.CurrentUserOptions.CatalogSchemes);

            var catalogSchemesWindow = new CatalogSchemesWindow(catalogSchemesVM);
            catalogSchemesWindow.Owner = this;
            catalogSchemesWindow.ShowDialog();
        }

        private void AboutCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void AboutCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            AboutWindow aboutWindow = new AboutWindow();
            aboutWindow.Owner = this;
            aboutWindow.ShowDialog();
        }

        private void DocumentationCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void DocumentationCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            UrlHelper.OpenUrl(AppSettings.DocumentationUrl);
        }

        private void DeleteFin_Click(object sender, RoutedEventArgs e)
        {
            if (MessageBox.Show("Are you sure you want to delete this fin from the database?", "Delete Confirmation", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
            {
                _vm.DarwinDatabase.Delete(_vm.SelectedFin);
                var index = _vm.Fins.IndexOf(_vm.SelectedFin);
                _vm.Fins.Remove(_vm.SelectedFin);

                // TODO: The else doesn't work quite right.  Setting SelectedFin null
                // doesn't actually clear out the values being displayed on the right.
                if (index < _vm.Fins.Count)
                    _vm.SelectedFin = _vm.Fins[index];
                else
                    _vm.SelectedFin = null;
            }
        }

        private void SaveFinData_Click(object sender, RoutedEventArgs e)
        {
            _vm.DarwinDatabase.UpdateIndividual(_vm.SelectedFin);
            _vm.SelectedFin.FieldsChanged = false;
        }

        private void OpenImageToolbarButton_Click(object sender, RoutedEventArgs e)
        {
            OpenImageCommand_Executed(null, null);
        }

        private void OpenTracedFinToolbarButton_Click(object sender, RoutedEventArgs e)
        {
            OpenTracedFinCommand_Executed(null, null);
        }

        private void MatchingQueueToolbarButton_Click(object sender, RoutedEventArgs e)
        {
            MatchingQueueCommand_Executed(null, null);
        }

        private void OutlineButton_Click(object sender, RoutedEventArgs e)
        {
            var outlineWindowVM = new OutlineWindowViewModel(_vm.SelectedFin);

            var outlineWindow = new OutlineWindow(outlineWindowVM);
            outlineWindow.Show();
        }
    }
}
