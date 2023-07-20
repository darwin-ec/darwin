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

using Darwin.Database;
using Darwin.Wpf.ViewModel;
using Darwin.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace Darwin.Wpf
{
    /// <summary>
    /// Interaction logic for MatchingResultsWindow.xaml
    /// </summary>
    public partial class MatchingResultsWindow : Window
    {
        private const int AutoScrollSeconds = 1;

        private MatchingResultsWindowViewModel _vm;
        private List<GridViewColumn> _allColumns;
        private DispatcherTimer _autoScrollTimer;
        public MatchingResultsWindow(MatchingResultsWindowViewModel vm)
        {
            InitializeComponent();

            _vm = vm;
            this.DataContext = _vm;

            _autoScrollTimer = new DispatcherTimer();
            _autoScrollTimer.Interval = TimeSpan.FromSeconds(AutoScrollSeconds);
            _autoScrollTimer.Tick += AutoScrollTimer_Tick;

            // After the window is loaded, make a backup of all the columns  in the list.  (This is
            // so we can have copies as we hide/show columns.)
            Loaded += delegate
            {
                _allColumns = new List<GridViewColumn>();
                foreach (var column in GridView.Columns)
                {
                    _allColumns.Add(column);
                }
            };
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

        private void UnknownMatchSelectedFinOrientation_Click(object sender, RoutedEventArgs e)
        {
            // TODO
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

        private void PreviousButton_Click(object sender, RoutedEventArgs e)
        {
            if (DatabaseGrid.Items == null || DatabaseGrid.Items.Count < 1)
                return;

            if (DatabaseGrid.SelectedIndex > 0)
            {
                DatabaseGrid.SelectedIndex--;
                DatabaseGrid.ScrollIntoView(_vm.SelectedResult);
            }
            CheckNextPreviousEnabled();
        }

        private void NextButton_Click(object sender, RoutedEventArgs e)
        {
            if (DatabaseGrid.Items == null || DatabaseGrid.Items.Count < 1)
                return;

            if (DatabaseGrid.SelectedIndex < DatabaseGrid.Items.Count - 1)
            {
                DatabaseGrid.SelectedIndex++;
                DatabaseGrid.ScrollIntoView(_vm.SelectedResult);
            }
            CheckNextPreviousEnabled();
        }

        private void MatchesSelectedFinButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.DatabaseFin.IDCode = _vm.SelectedResult.IDCode;
            if (!string.IsNullOrEmpty(_vm.SelectedResult.Name))
                _vm.DatabaseFin.Name = _vm.SelectedResult.Name;

            var vm = new TraceWindowViewModel(_vm.DatabaseFin, _vm.Database,
                "Matches ["
                + _vm.SelectedResult.IDCode
                + "] - Add to Database as Additional "
                + _vm.Database.CatalogScheme.IndividualTerminology.ToFirstCharacterUpper()
                + " Image", this);

            TraceWindow traceWindow = new TraceWindow(vm);

            traceWindow.Show();
        }

        private void NoMatchNewFinButton_Click(object sender, RoutedEventArgs e)
        {
            var vm = new TraceWindowViewModel(_vm.DatabaseFin, _vm.Database,
                "No Match - Add to Database as NEW "
                + _vm.Database.CatalogScheme.IndividualTerminology.ToFirstCharacterUpper()
                + "/Image", this);

            TraceWindow traceWindow = new TraceWindow(vm);
            traceWindow.Show();
        }

        private void ReturnToMatchingDialogButton_Click(object sender, RoutedEventArgs e)
        {
            var matchingWindowVM = new MatchingWindowViewModel(_vm.DatabaseFin, _vm.Database);
            var matchingWindow = new MatchingWindow(matchingWindowVM);
            this.Close();
            matchingWindow.Show();
        }

        private void SaveMatchResultsButton_Click(object sender, RoutedEventArgs e)
        {
            string finzFilename;
            string resultsFilename;

            _vm.SaveMatchResults(out finzFilename, out resultsFilename);

            MessageBox.Show(
                "Match Results Saved:" + Environment.NewLine + Environment.NewLine +
                "Trace File: " + finzFilename + Environment.NewLine + Environment.NewLine +
                "Results File: " + resultsFilename,
                "Results Saved", MessageBoxButton.OK, MessageBoxImage.Information);
        }

        private void DoneButton_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        private void HideIDsButton_Click(object sender, RoutedEventArgs e)
        {
            ShowHideColumns();
        }

        private void InfoButton_Click(object sender, RoutedEventArgs e)
        {
            ShowHideColumns();
        }

        private void ShowHideColumns()
        {
            if (_allColumns != null)
            {
                GridView.Columns.Clear();

                for (int i = 0; i < _allColumns.Count; i++)
                {
                    // This is basically hardcoding the column indices and is a little fragile.
                    if (i == 3 && !_vm.ShowIDColumn)
                        continue;

                    if ((i == 1 || i >= 4) && !_vm.ShowInfoColumns)
                        continue;

                    GridView.Columns.Add(_allColumns[i]);
                }
            }
        }

        private void AutoScrollButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.AutoScroll)
                _autoScrollTimer.Start();
            else
                _autoScrollTimer.Stop();
        }

        private void AutoScrollTimer_Tick(object sender, EventArgs e)
        {
            if (_vm.AutoScroll && _vm.MatchResults?.Results?.Count > 0)
            {
                if (_vm.SelectedResult == null)
                {
                    _vm.SelectedResult = _vm.MatchResults.Results[0];
                }
                else
                {
                    if (DatabaseGrid.SelectedIndex < DatabaseGrid.Items.Count - 1)
                        DatabaseGrid.SelectedIndex++;
                    else
                        DatabaseGrid.SelectedIndex = 0;

                    DatabaseGrid.ScrollIntoView(_vm.SelectedResult);
                    CheckNextPreviousEnabled();
                }
            }
        }

        private void DatabaseGrid_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            CheckNextPreviousEnabled();
        }

        private void ViewSelectedImageButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.SelectedResult != null)
            {
                var fin = _vm.FullyLoadFinByID(_vm.SelectedResult.DatabaseID);

                fin.FinOutline.ChainPoints = null;

                if (_vm.SelectedShowOriginalImage)
                    fin.FinImage = fin.OriginalFinImage;

                var vm = new TraceWindowViewModel(fin, _vm.Database, "Viewing Selected: " + fin.IDCode, null, true);
                TraceWindow traceWindow = new TraceWindow(vm);
                traceWindow.Show();
            }
        }

        private void ViewUnknownImageButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.SelectedResult != null)
            {
                var fin = new DatabaseFin(_vm.DatabaseFin);

                fin.FinOutline.ChainPoints = null;

                if (_vm.UnknownShowOriginalImage)
                    fin.FinImage = fin.OriginalFinImage;

                var vm = new TraceWindowViewModel(fin, _vm.Database, "Viewing Unknown", null, true);
                TraceWindow traceWindow = new TraceWindow(vm);
                traceWindow.Show();
            }
        }

        private void ViewOutlineInformationButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.SelectedResult != null)
            {
                var selectedDBFin = _vm.FullyLoadFinByID(_vm.SelectedResult.DatabaseID);

                selectedDBFin.FinOutline.ChainPoints = _vm.SelectedResult.dbContour;

                DatabaseFin copyUnknown = new DatabaseFin(_vm.DatabaseFin);
                copyUnknown.FinOutline.ChainPoints = _vm.SelectedResult.unknownContour;

                var outlineWindowVM = new OutlineWindowViewModel(_vm.Database, selectedDBFin, copyUnknown, _vm.SelectedResult);

                var outlineWindow = new OutlineWindow(outlineWindowVM);

                outlineWindow.Show();
            }
        }
    }
}