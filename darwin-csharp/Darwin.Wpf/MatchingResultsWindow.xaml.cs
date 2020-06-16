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

namespace Darwin.Wpf
{
    /// <summary>
    /// Interaction logic for MatchingResultsWindow.xaml
    /// </summary>
    public partial class MatchingResultsWindow : Window
    {
        private MatchingResultsWindowViewModel _vm;

        public MatchingResultsWindow(MatchingResultsWindowViewModel vm)
        {
            InitializeComponent();

            _vm = vm;
            this.DataContext = _vm;
        }

        private void GridHeader_Click(object sender, RoutedEventArgs e)
        {
            var sortableListViewSender = sender as Controls.SortableListView;

            if (sortableListViewSender != null)
                sortableListViewSender.GridViewColumnHeaderClickedHandler(sender, e);
        }

        private void SelectedShowOriginalImage_Click(object sender, RoutedEventArgs e)
        {

        }

        private void UnknownShowOriginalImage_Click(object sender, RoutedEventArgs e)
        {

        }

        private void UnknownMatchSelectedFinOrientation_Click(object sender, RoutedEventArgs e)
        {

        }

        private void PreviousButton_Click(object sender, RoutedEventArgs e)
        {
            int currentIndex = _vm.CurrentSelectedIndex;

            if (currentIndex >= 1)
                _vm.SelectedResult = _vm.MatchResults.Results[currentIndex - 1];
        }

        private void NextButton_Click(object sender, RoutedEventArgs e)
        {
            int currentIndex = _vm.CurrentSelectedIndex;

            if (currentIndex >= 0)
                _vm.SelectedResult = _vm.MatchResults.Results[currentIndex + 1];
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
    }
}