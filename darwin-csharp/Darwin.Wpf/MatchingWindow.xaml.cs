using Darwin.Matching;
using Darwin.Wpf.ViewModel;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading;
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
    /// Interaction logic for MatchingWindow.xaml
    /// </summary>
    public partial class MatchingWindow : Window
    {
        private BackgroundWorker _matchingWorker = new BackgroundWorker();

        private MatchingWindowViewModel _vm;
        public MatchingWindow(MatchingWindowViewModel vm)
        {
            InitializeComponent();

            _matchingWorker.WorkerReportsProgress = true;
            //_matchingWorker.ProgressChanged += ProgressChanged;
            _matchingWorker.DoWork += MatchWork;
            _matchingWorker.RunWorkerCompleted += MatchWorker_RunWorkerCompleted;

            _vm = vm;
            this.DataContext = _vm;
        }

        private void StartButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.ProgressBarVisibility = Visibility.Visible;
            _matchingWorker.RunWorkerAsync();
        }

        private void PauseButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.PauseMatching)
            {
                PauseButton.Content = "Pause";
                _vm.PauseMatching = false;
            }
            else
            {
                PauseButton.Content = "Continue";
                _vm.PauseMatching = true;
            }
        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.MatchRunning)
            {
                _vm.CancelMatching = true;
                _matchingWorker.CancelAsync();
            }
            else
            {
                this.Close();
            }
        }

        private void SelectAllCategoriesButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.SelectableCategories != null)
            {
                foreach (var cat in _vm.SelectableCategories)
                    cat.IsSelected = true;
            }
        }

        private void ClearAllCategoriesButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.SelectableCategories != null)
            {
                foreach (var cat in _vm.SelectableCategories)
                    cat.IsSelected = false;
            }
        }

        private void MatchWork(object sender, DoWorkEventArgs e)
        {
            bool done = false;
            _vm.MatchRunning = true;
            do
            {
                if (_matchingWorker.CancellationPending)
                {
                    e.Cancel = true;
                    done = true;
                }
                else if (_vm.PauseMatching)
                {
                    // Sleep for a small amount of time
                    Thread.Sleep(100);
                }
                else
                {
                    // Do Work
                    float percentComplete = _vm.Match.MatchSingleFin(
                                      _vm.RegistrationMethod,
                                      (int)RangeOfPointsType.AllPoints, // TODO: This is hacky, since we have a radio button, but it's not straightforward
                                      _vm.SelectableCategories.Where(c => c.IsSelected).ToList(),
                                      (_vm.RangeOfPoints == RangeOfPointsType.AllPoints) ? true : false, // TODO: Not straightforward
                                      true);

                    int roundedProgress = (int)Math.Round(percentComplete * 100);

                    _vm.MatchProgressPercent = roundedProgress;
                    _matchingWorker.ReportProgress(roundedProgress);

                    // TODO: Verify this comparison always works correctly
                    if (percentComplete >= 1.0)
                    {
                        //***1.5 - sort the results here, ONCE, rather than as list is built
                        _vm.Match.MatchResults.Sort();
                        done = true;
                    }
                }
            } while (!done);
        }

        private void MatchWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (!_vm.CancelMatching)
            {
                if (_vm.Match.MatchResults == null || _vm.Match.MatchResults.Count < 1)
                {
                    MessageBox.Show("Selected Catalog Categories are ALL EMPTY!");
                }
                else
                {
                    // Matching is done, go to the results window
                    var matchingResultsWindowVM = new MatchingResultsWindowViewModel(
                        _vm.DatabaseFin,
                        _vm.Match.MatchResults,
                        _vm.Database);
                    var matchingResultsWindow = new MatchingResultsWindow(matchingResultsWindowVM);
                    matchingResultsWindow.Show();
                }
            }

            this.Close();
        }
    }
}
