﻿// This file is part of DARWIN.
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
using System.IO;
using Darwin.Extensions;
using System.Threading;

namespace Darwin.Wpf
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private MainWindowViewModel _vm;
        private const int NumRestoreRetries = 4;
        private const int RestoreSleepMilliseconds = 2000;

        public MainWindow()
        {
            InitializeComponent();

            _vm = new MainWindowViewModel();

            bool openFirstRunWindow = false;
            try
            {
                if (!string.IsNullOrEmpty(Options.CurrentUserOptions.DatabaseFileName))
                {
                    OpenDatabase(Options.CurrentUserOptions.DatabaseFileName, false, true);
                }
                else
                {
                    openFirstRunWindow = true;
                }
            }
            catch (Exception ex)
            {
                Trace.WriteLine(ex);
                MessageBox.Show("There was a problem opening the database. It may have been moved or deleted.\n\n" +
                    "Please use Open Database to find an existing database, or use New Database to create a new one.",
                    "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }

            DatabaseGrid.SetFilter(this.FilterListView);
            StateChanged += MainWindowStateChangeRaised;

            this.DataContext = _vm;

            // Sort by ID code when the window first opens
            if (_vm.DarwinDatabase != null)
                DatabaseGrid.Sort("IDCode", ListSortDirection.Ascending);

            Loaded += delegate
            {
                SelectFirstFin();
            };

            if (openFirstRunWindow)
            {
                ContentRendered += delegate
                {
                    var firstRunVM = new FirstRunViewModel();
                    var firstRunWindow = new FirstRunWindow(firstRunVM);
                    firstRunWindow.Owner = this;
                    firstRunWindow.ShowDialog();

                };
            }
        }

        private void GridHeader_Click(object sender, RoutedEventArgs e)
        {
            var sortableListViewSender = sender as Controls.SortableListView;

            if (sortableListViewSender != null)
            {
                sortableListViewSender.GridViewColumnHeaderClickedHandler(sender, e);
                CheckNextPreviousEnabled();
            }
        }

        private void MainWindowStateChangeRaised(object sender, EventArgs e)
        {
            if (WindowState == WindowState.Maximized)
            {
                MainWindowBorder.BorderThickness = new Thickness(8);
                RestoreButton.Visibility = Visibility.Visible;
                MaximizeButton.Visibility = Visibility.Collapsed;
            }
            else
            {
                MainWindowBorder.BorderThickness = new Thickness(0);
                RestoreButton.Visibility = Visibility.Collapsed;
                MaximizeButton.Visibility = Visibility.Visible;
            }
        }

        private void WindowIcon_MouseDown(object sender, RoutedEventArgs e)
        {
            SystemCommands.ShowSystemMenu(this, WindowIcon.PointToScreen(new System.Windows.Point(0, WindowIcon.ActualHeight)));
        }

        private void CommandBinding_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CommandBinding_Executed_Minimize(object sender, ExecutedRoutedEventArgs e)
        {
            SystemCommands.MinimizeWindow(this);
        }

        private void CommandBinding_Executed_Maximize(object sender, ExecutedRoutedEventArgs e)
        {
            SystemCommands.MaximizeWindow(this);
        }

        private void CommandBinding_Executed_Restore(object sender, ExecutedRoutedEventArgs e)
        {
            SystemCommands.RestoreWindow(this);
        }

        private void CommandBinding_Executed_Close(object sender, ExecutedRoutedEventArgs e)
        {
            SystemCommands.CloseWindow(this);
        }

        private void NewDatabaseCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        public void NewDatabaseCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var newDatabaseVM = new NewDatabaseViewModel();

            var newDatabaseWindow = new NewDatabaseWindow(newDatabaseVM);
            newDatabaseWindow.Owner = this;
            newDatabaseWindow.WindowStartupLocation = WindowStartupLocation.CenterOwner;
            newDatabaseWindow.ShowDialog();
        }

        private void OpenImageCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = _vm.DarwinDatabase != null;
        }

        private void OpenImageCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var openDialog = new OpenFileDialog();
            openDialog.Filter = CustomCommands.OpenImageFilter;
            if (openDialog.ShowDialog() == true)
            {
                var vm = new TraceWindowViewModel(openDialog.FileName, _vm.DarwinDatabase);

                TraceWindow traceWindow = new TraceWindow(vm);
                traceWindow.Show();
            }
        }

        private void OpenTracedFinCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = _vm.DarwinDatabase != null;
        }

        private void OpenTracedFinCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var openDialog = new OpenFileDialog();
            openDialog.Filter = CustomCommands.TracedFinFilter;
            openDialog.InitialDirectory = Options.CurrentUserOptions.CurrentTracedFinsPath;

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

        public void OpenDatabaseCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var openDatabaseDialog = new OpenFileDialog();
            openDatabaseDialog.Title = "Open Database";
            openDatabaseDialog.Filter = CustomCommands.OpenDatabaseFilter;
            openDatabaseDialog.InitialDirectory = Options.CurrentUserOptions.CurrentCatalogPath;

            if (openDatabaseDialog.ShowDialog() == true)
                OpenDatabase(openDatabaseDialog.FileName, true);
        }

        private void CloseDatabaseCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = _vm.DarwinDatabase != null;
        }

        private void CloseDatabaseCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            _vm.SelectedFin = null;
            _vm.Fins = null;
            _vm.DarwinDatabase = null;
            CheckNextPreviousEnabled();
        }

        private void SelectFirstFin()
        {
            if (DatabaseGrid.Items != null && DatabaseGrid.Items.Count > 0)
            {
                DatabaseGrid.SelectedIndex = 0;

                if (_vm.SelectedFin != null)
                    DatabaseGrid.ScrollIntoView(_vm.SelectedFin);
            }
            else if (_vm.Fins?.Count > 0)
            {
                _vm.SelectedFin = _vm.Fins[0];
            }
            else
            {
                _vm.SelectedFin = null;
            }
        }

        public void OpenDatabase(string filename, bool saveOptions = false, bool initialLoad = false)
        {
            var db = CatalogSupport.OpenDatabase(filename, Options.CurrentUserOptions.DefaultCatalogScheme, false);

            if (saveOptions)
            {
                // TODO: Some more logic around this
                Options.CurrentUserOptions.SetLastDatabaseFilename(filename);

                Options.CurrentUserOptions.Save();
            }

            _vm.DarwinDatabase = db;
            // TODO: The thumbnail part is temporary
            // Need to store the thumbnail images more nicely
            _vm.Fins = new ObservableNotifiableCollection<DatabaseFin>(
                _vm.DarwinDatabase
                .GetAllFins()
                .Select(x => { x.ThumbnailFilename = x.ImageFilename; return x; })
                .ToList());

            if (!initialLoad)
                SelectFirstFin();
        }

        public void RefreshDatabase()
        {
            _vm.RefreshDatabase();

            _vm.Fins = new ObservableNotifiableCollection<DatabaseFin>(
                _vm.DarwinDatabase
                .GetAllFins()
                .Select(x => { x.ThumbnailFilename = x.ImageFilename; return x; })
                .ToList());

            SelectFirstFin();
        }

        public void RefreshDatabaseAfterAdd()
        {
            Trace.WriteLine("Start database refresh after add...");
            _vm.Fins = new ObservableNotifiableCollection<DatabaseFin>(
                _vm.DarwinDatabase
                .GetAllFins()
                .Select(x => { x.ThumbnailFilename = x.ImageFilename; return x; })
                .ToList());

            // Select the last fin, and scroll to it so the user can see it
            if (_vm.Fins != null)
            {
                Trace.WriteLine("Selecting added individual...");

                _vm.SelectedFin = _vm.Fins[_vm.Fins.Count - 1];
                DatabaseGrid.ScrollIntoView(_vm.SelectedFin);
            }

            Trace.WriteLine("Refresh complete.");
        }

        private void MatchingQueueCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = _vm.DarwinDatabase != null;
        }

        private void MatchingQueueCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var matchingQueueVM = new MatchingQueueViewModel();

            var matchingQueueWindow = new MatchingQueueWindow(matchingQueueVM);
            matchingQueueWindow.Owner = this;
            matchingQueueWindow.WindowStartupLocation = WindowStartupLocation.CenterOwner;
            matchingQueueWindow.Show();
        }

        private void BackupCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = _vm.DarwinDatabase != null;
        }

        private void BackupCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var backupFilename = _vm.BackupDatabase();
            MessageBox.Show("Your database has been backed up to the file:"
                + Environment.NewLine
                + Environment.NewLine
                + backupFilename, "Backup Complete", MessageBoxButton.OK, MessageBoxImage.Information);
        }

        private void RestoreCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void RestoreWorker(string backupFilename, string surveyArea, string databaseName)
        {
            // We have some files that might still be open.  So we're going to have a retry loop with a sleep
            // and hopefully we get a chance to write over the files, if necessary

            for (int i = 0; i < NumRestoreRetries; i ++)
            {
                try
                {
                    Thread.Sleep(RestoreSleepMilliseconds);

                    // This is hacky, but let's force GC again.  Otherwise, the ListView -> Image often is still holding
                    // exclusive access to thumbnails
                    _vm.CloseDatabase();

                    string dbFullFilename = _vm.RestoreDatabase(backupFilename, surveyArea, databaseName);

                    // Run this back on the UI thread
                    Dispatcher.BeginInvoke(new Action(() =>
                    {
                        OpenDatabase(dbFullFilename, true);
                    }));
                }
                catch
                {
                    continue;
                }

                break;
            }
        }

        private async void RestoreDatabaseAsync(string backupFilename, string surveyArea, string databaseName)
        {
            try
            {
                this.IsHitTestVisible = false;
                Mouse.OverrideCursor = Cursors.Wait;

                await Task.Run(() => RestoreWorker(backupFilename, surveyArea, databaseName));
            }
            finally
            {
                Mouse.OverrideCursor = null;
                this.IsHitTestVisible = true;
            }
        }

        private void RestoreCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var restoreDialog = new OpenFileDialog();
            restoreDialog.Filter = CustomCommands.RestoreFilenameFilter;
            restoreDialog.InitialDirectory = Options.CurrentUserOptions.CurrentBackupsPath;

            if (restoreDialog.ShowDialog() == true)
            {
                string surveyArea;
                string databaseName;

                try
                {
                    _vm.CheckSurveyAreaDatabaseNameFromBackup(restoreDialog.FileName, out surveyArea, out databaseName);

                    if (string.IsNullOrEmpty(surveyArea) || string.IsNullOrEmpty(databaseName))
                        throw new Exception("Empty value when reading from file.");

                    var restoreQuestionResult = MessageBox.Show("You are about to replace the database below with the backup copy you selected.  DO NOT PROCEED " +
                        "if you are attempting to move this database to a different location."
                        + Environment.NewLine
                        + Environment.NewLine
                        + "DARWIN Home: " + Options.CurrentUserOptions.CurrentDarwinHome + Environment.NewLine
                        + "Survey Area: " + surveyArea + Environment.NewLine
                        + "Database: " + databaseName
                        + Environment.NewLine + Environment.NewLine
                        + "Are you sure you want to proceed with restoring the database in the location indicated?",
                        "Caution, restore cannot be undone!", MessageBoxButton.YesNo, MessageBoxImage.Question);

                    if (restoreQuestionResult == MessageBoxResult.Yes)
                    {
                        _vm.CloseDatabase();

                        RestoreDatabaseAsync(restoreDialog.FileName, surveyArea, databaseName);
                    }
                }
                catch (Exception ex)
                {
                    Trace.WriteLine(ex);
                    MessageBox.Show("There was a problem with the backup you selected.  Please choose another or contact support.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                }
            }
        }

        private void ImportFinCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = _vm.DarwinDatabase != null;
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

        private void ExportFinCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = _vm.SelectedFin != null;
        }

        private void ExportFinCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            if (_vm.SelectedFin == null)
            {
                MessageBox.Show("You must select an item in the database first.", "No " + _vm.DarwinDatabase.CatalogScheme.IndividualTerminologyInitialCaps + " Selected",
                    MessageBoxButton.OK, MessageBoxImage.Warning);
            }
            else
            {
                SaveFileDialog dlg = new SaveFileDialog();
                dlg.InitialDirectory = Options.CurrentUserOptions.CurrentTracedFinsPath;
                dlg.Title = "Export finz file for " + _vm.SelectedFin?.IDCode;
                dlg.FileName = _vm.SelectedFin?.IDCode;
                dlg.DefaultExt = ".finz";
                dlg.Filter = CustomCommands.TracedFinFilter;

                if (dlg.ShowDialog() == true)
                {
                    _vm.SaveSelectedItemAsFinz(dlg.FileName);
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

        private void CurrentCatalogSchemeCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = _vm.DarwinDatabase != null;
        }

        private void CurrentCatalogSchemeCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var currentCatalogSchemesVM = new CurrentCatalogSchemeViewModel(_vm.DarwinDatabase);

            var currentCatalogSchemeWindow = new CurrentCatalogSchemeWindow(currentCatalogSchemesVM);
            currentCatalogSchemeWindow.Owner = this;
            currentCatalogSchemeWindow.ShowDialog();
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

        private void DeveloperToolsCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void DeveloperToolsCommand_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            var vm = new DeveloperToolsViewModel(_vm.DarwinDatabase);
            var window = new DeveloperToolsWindow(vm);
            window.Owner = this;
            window.ShowDialog();
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
            if (MessageBox.Show("Are you sure you want to delete this " + _vm.DarwinDatabase.CatalogScheme.IndividualTerminology + " from the database?",
                "Delete Confirmation", MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.Yes)
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
            if (_vm.DarwinDatabase != null && _vm.SelectedFin != null)
            {
                _vm.DarwinDatabase.UpdateIndividual(_vm.SelectedFin);
                _vm.SelectedFin.FieldsChanged = false;
            }
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
            var outlineWindowVM = new OutlineWindowViewModel(_vm.DarwinDatabase, _vm.SelectedFin);

            var outlineWindow = new OutlineWindow(outlineWindowVM);
            outlineWindow.Show();
        }

        private void ViewImageButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.SelectedFin != null)
            {
                var fin = _vm.FullyLoadFin();
                var vm = new TraceWindowViewModel(fin, _vm.DarwinDatabase, "Viewing " + fin.IDCode, this);
                TraceWindow traceWindow = new TraceWindow(vm);
                traceWindow.Show();
            }
        }

        private void ViewOriginalImageButton_Click(object sender, RoutedEventArgs e)
        {
            // Little hacky
            if (_vm.SelectedFin != null)
            {
                var fin = _vm.FullyLoadFin();
                fin.FinOutline.ChainPoints = null;
                fin.FinImage = fin.OriginalFinImage;
                var vm = new TraceWindowViewModel(fin, _vm.DarwinDatabase, "Viewing " + fin.IDCode + " Original Image", this);
                TraceWindow traceWindow = new TraceWindow(vm);
                traceWindow.Show();
            }
        }

        private void FinDataButton_Click(object sender, RoutedEventArgs e)
        {
            // Little hacky
            if (_vm.SelectedFin != null)
            {
                var fin = _vm.FullyLoadFin();
                //TODO: Finish this form
            }
        }

        private void CheckNextPreviousEnabled()
        {
            if (DatabaseGrid.SelectedIndex <= 0)
                _vm.PreviousEnabled = false;
            else if (DatabaseGrid.Items != null && DatabaseGrid.Items.Count > 0)
                _vm.PreviousEnabled = true;

            if (DatabaseGrid.Items != null && DatabaseGrid.Items.Count > 0 && DatabaseGrid.SelectedIndex < DatabaseGrid.Items.Count - 1)
                _vm.NextEnabled = true;
            else
                _vm.NextEnabled = false;
        }

        private void ListPreviousButton_Click(object sender, RoutedEventArgs e)
        {
            if (DatabaseGrid.Items == null || DatabaseGrid.Items.Count < 1)
                return;

            if (DatabaseGrid.SelectedIndex > 0)
            {
                DatabaseGrid.SelectedIndex--;
                DatabaseGrid.ScrollIntoView(_vm.SelectedFin);
            }
            CheckNextPreviousEnabled();
        }

        private void ListNextButton_Click(object sender, RoutedEventArgs e)
        {
            if (DatabaseGrid.Items == null || DatabaseGrid.Items.Count < 1)
                return;

            if (DatabaseGrid.SelectedIndex < DatabaseGrid.Items.Count - 1)
            {
                DatabaseGrid.SelectedIndex++;
                DatabaseGrid.ScrollIntoView(_vm.SelectedFin);
            }
            CheckNextPreviousEnabled();
        }

        private bool FilterListView(object item)
        {
            if (string.IsNullOrEmpty(FilterTextBox.Text))
                return true;

            DatabaseFin filterItem = item as DatabaseFin;

            if (filterItem == null)
                return true;

            if (string.IsNullOrEmpty(filterItem.IDCode))
                return false;

            return filterItem.IDCode.IndexOf(FilterTextBox.Text, StringComparison.OrdinalIgnoreCase) > 0;
        }

        private void DatabaseGrid_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            CheckNextPreviousEnabled();
        }

        private void FilterTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            DatabaseGrid.RefreshFilter();
        }
    }
}
