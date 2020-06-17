using Darwin.Database;
using Darwin.Wpf.ViewModel;
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
        private const int AutoScrollSeconds = 4;

        private MatchingResultsWindowViewModel _vm;
        private List<GridViewColumn> _allColumns;
        private DispatcherTimer _autoScrollTimer;
        public MatchingResultsWindow(MatchingResultsWindowViewModel vm)
        {
            InitializeComponent();

            _autoScrollTimer = new DispatcherTimer();
            _autoScrollTimer.Interval = TimeSpan.FromSeconds(AutoScrollSeconds);
            _autoScrollTimer.Tick += AutoScrollTimer_Tick;

            _vm = vm;
            this.DataContext = _vm;

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
                sortableListViewSender.GridViewColumnHeaderClickedHandler(sender, e);
        }

        private void UnknownMatchSelectedFinOrientation_Click(object sender, RoutedEventArgs e)
        {

        }

        private void PreviousButton_Click(object sender, RoutedEventArgs e)
        {
            int currentIndex = _vm.CurrentSelectedIndex;

            if (currentIndex >= 1)
            {
                _vm.SelectedResult = _vm.MatchResults.Results[currentIndex - 1];
                DatabaseGrid.ScrollIntoView(_vm.SelectedResult);
            }
        }

        private void NextButton_Click(object sender, RoutedEventArgs e)
        {
            int currentIndex = _vm.CurrentSelectedIndex;

            if (currentIndex >= 0)
            {
                _vm.SelectedResult = _vm.MatchResults.Results[currentIndex + 1];
                DatabaseGrid.ScrollIntoView(_vm.SelectedResult);
            }
        }

        private void MatchesSelectedFinButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.DatabaseFin.IDCode = _vm.SelectedResult.IDCode;
            if (!string.IsNullOrEmpty(_vm.SelectedResult.Name))
                _vm.DatabaseFin.Name = _vm.SelectedResult.Name;

            var vm = new TraceWindowViewModel(_vm.DatabaseFin, _vm.Database,
                "Matches [" + _vm.SelectedResult.IDCode + "] - Add to Database as Additional Fin Image", this);

            TraceWindow traceWindow = new TraceWindow(vm);

            traceWindow.Show();
        }

        private void NoMatchNewFinButton_Click(object sender, RoutedEventArgs e)
        {
            var vm = new TraceWindowViewModel(_vm.DatabaseFin, _vm.Database,
                "No Match - Add to Database as NEW Fin/Image", this);
                        TraceWindow traceWindow = new TraceWindow(vm);
                        traceWindow.Show();
        }

        private void ReturnToMatchingDialogButton_Click(object sender, RoutedEventArgs e)
        {
            var matchingWindowVM = new MatchingWindowViewModel(_vm.DatabaseFin, _vm.Database,
                new ObservableCollection<DBDamageCategory>(_vm.Database?.Categories));
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
                    int currentIndex = _vm.MatchResults.Results.IndexOf(_vm.SelectedResult);
                    int nextIndex = currentIndex += 1;
                    if (currentIndex >= _vm.MatchResults.Results.Count - 1)
                        nextIndex = 0;

                    _vm.SelectedResult = _vm.MatchResults.Results[nextIndex];
                    DatabaseGrid.ScrollIntoView(_vm.SelectedResult);
                }
            }
        }
    }
}