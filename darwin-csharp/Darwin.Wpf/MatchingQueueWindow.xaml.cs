using Darwin.Database;
using Darwin.Matching;
using Darwin.Wpf.Commands;
using Darwin.Wpf.ViewModel;
using Microsoft.Win32;
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
    /// Interaction logic for MatchingQueueWindow.xaml
    /// </summary>
    public partial class MatchingQueueWindow : Window
    {
        private BackgroundWorker _matchingWorker = new BackgroundWorker();
        private MatchingQueueViewModel _vm;

        public MatchingQueueWindow(MatchingQueueViewModel vm)
        {
            InitializeComponent();
            
            _matchingWorker.WorkerReportsProgress = true;
            //_matchingWorker.ProgressChanged += ProgressChanged;
            _matchingWorker.DoWork += MatchWork;
            _matchingWorker.RunWorkerCompleted += MatchWorker_RunWorkerCompleted;

            _vm = vm;
            this.DataContext = vm;
        }

        private void GridHeader_Click(object sender, RoutedEventArgs e)
        {
            var sortableListViewSender = sender as Controls.SortableListView;

            if (sortableListViewSender != null)
                sortableListViewSender.GridViewColumnHeaderClickedHandler(sender, e);
        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void RemoveButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.SelectedFin != null)
                _vm.Fins.Remove(_vm.SelectedFin);
        }

        private void AddFinzButton_Click(object sender, RoutedEventArgs e)
        {
            var openDialog = new OpenFileDialog();
            openDialog.Filter = CustomCommands.OpenTracedFinFilter;
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
                    _vm.Fins.Add(fin);
                    _vm.SelectedFin = _vm.Fins.Last();
                }
            }
        }

        private void MatchWork(object sender, DoWorkEventArgs e)
        {
            if (_vm.Fins.Count < 1)
                return;

            bool done = false;
            _vm.MatchRunning = true;

            int currentIndex = 0;

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
                    if (_vm.Matches.Count < currentIndex - 1)

                        _vm.Matches.Add(new Match(_vm.Fins[currentIndex], _vm.Database, null));


                    // Do Work
                    // TODO: The registration method is hardcoded here.
                    float percentComplete = _vm.Matches[currentIndex].MatchSingleFin(
                                        _vm.RegistrationMethod,
                                        (int)RangeOfPointsType.AllPoints, // TODO: This is hacky, since we have a radio button, but it's not straightforward
                                        _vm.Database.SelectableCategories.Where(c => c.IsSelected).ToList(),
                                        (_vm.RangeOfPoints == RangeOfPointsType.AllPoints) ? true : false, // TODO: Not straightforward
                                        true);

                    int roundedProgress = (int)Math.Round(percentComplete * 100);

                    _vm.CurrentUnknownPercent = roundedProgress;

                    var totalProgress = (int)Math.Round(((float)currentIndex / _vm.Fins.Count + percentComplete / _vm.Fins.Count) * 100);

                    _vm.QueueProgressPercent = totalProgress;
                    _matchingWorker.ReportProgress(roundedProgress);

                    // TODO: Verify this comparison always works correctly
                    if (percentComplete >= 1.0)
                    {
                        //***1.5 - sort the results here, ONCE, rather than as list is built
                        _vm.Matches[currentIndex].MatchResults.Sort();

                        if (currentIndex >= _vm.Fins.Count - 1)
                        {
                            done = true;
                        }
                        else
                        {
                            currentIndex++;
                        }
                    }
                }
            } while (!done);
        }

        private void MatchWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            if (!_vm.CancelMatching)
            {
                // TODO

                    // Matching is done, go to the results window
                //    var matchingResultsWindowVM = new MatchingResultsWindowViewModel(
                //        _vm.DatabaseFin,
                //        _vm.Match.MatchResults,
                //        _vm.Database);
                //    var matchingResultsWindow = new MatchingResultsWindow(matchingResultsWindowVM);
                //    matchingResultsWindow.Show();
                //}
            }

            this.Close();
        }
    }
}
