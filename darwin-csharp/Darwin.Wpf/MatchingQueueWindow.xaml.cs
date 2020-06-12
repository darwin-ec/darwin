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
using System.Windows.Threading;

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
            if (_vm.MatchingQueue.MatchRunning)
            {
                _vm.CancelMatching = true;
                _matchingWorker.CancelAsync();
            }
            else
            {
                this.Close();
            }
        }

        private void RemoveButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.SelectedFin != null)
                _vm.MatchingQueue.Fins.Remove(_vm.SelectedFin);
        }

        private void AddFinzButton_Click(object sender, RoutedEventArgs e)
        {
            var openDialog = new OpenFileDialog();
            openDialog.InitialDirectory = Options.CurrentUserOptions.CurrentTracedFinsPath;

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
                    _vm.MatchingQueue.Fins.Add(fin);
                    _vm.SelectedFin = _vm.MatchingQueue.Fins.Last();
                }
            }
        }

        private void MatchWork(object sender, DoWorkEventArgs e)
        {
            if (_vm.MatchingQueue.Fins.Count < 1)
                return;

            bool done = false;
            _vm.MatchingQueue.MatchRunning = true;

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
                    // TODO: Put this logic inside the MatchingQueue class?
                    if (_vm.MatchingQueue.Matches.Count < currentIndex + 1)
                    {
                        // This needs to run on the UI thread since it affects dependency objects
                        Dispatcher.BeginInvoke(new Action(() =>
                        {
                            _vm.SelectedFin = _vm.MatchingQueue.Fins[currentIndex];
                        }), DispatcherPriority.Background);

                        _vm.MatchingQueue.Matches.Add(new Match(
                            _vm.MatchingQueue.Fins[currentIndex],
                            _vm.MatchingQueue.Database, null));
                    }

                    // Do Work
                    // TODO: The registration method is hardcoded here.
                    float percentComplete = _vm.MatchingQueue.Matches[currentIndex].MatchSingleFin(
                                        _vm.MatchingQueue.RegistrationMethod,
                                        (int)RangeOfPointsType.AllPoints, // TODO: This is hacky, since we have a radio button, but it's not straightforward
                                        _vm.MatchingQueue.Database.SelectableCategories.Where(c => c.IsSelected).ToList(),
                                        (_vm.MatchingQueue.RangeOfPoints == RangeOfPointsType.AllPoints) ? true : false, // TODO: Not straightforward
                                        true);

                    int roundedProgress = (int)Math.Round(percentComplete * 100);

                    _vm.CurrentUnknownPercent = roundedProgress;

                    var totalProgress = (int)Math.Round(((float)currentIndex / _vm.MatchingQueue.Fins.Count + percentComplete / _vm.MatchingQueue.Fins.Count) * 100);

                    _vm.QueueProgressPercent = totalProgress;
                    _matchingWorker.ReportProgress(roundedProgress);

                    if (percentComplete >= 1.0)
                    {
                        //***1.5 - sort the results here, ONCE, rather than as list is built
                        _vm.MatchingQueue.Matches[currentIndex].MatchResults.Sort();

                        if (currentIndex >= _vm.MatchingQueue.Fins.Count - 1)
                        {
                            _vm.SaveMatchResults();
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

        private void SaveQueueButton_Click(object sender, RoutedEventArgs e)
        {
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.InitialDirectory = Options.CurrentUserOptions.CurrentMatchQueuePath;
            dlg.FileName = "Untitled";
            dlg.DefaultExt = ".que";
            dlg.Filter = CustomCommands.QueueFilenameFilter;

            if (dlg.ShowDialog() == true)
            {
                _vm.SaveQueue(dlg.FileName);
            }
        }

        private void LoadQueueButton_Click(object sender, RoutedEventArgs e)
        {
            var openQueueDialog = new OpenFileDialog();
            openQueueDialog.Filter = CustomCommands.QueueFilenameFilter;
            openQueueDialog.InitialDirectory = Options.CurrentUserOptions.CurrentMatchQueuePath;

            if (openQueueDialog.ShowDialog() == true)
            {
                _vm.LoadQueue(openQueueDialog.FileName);
            }
        }

        private void ViewResultsButton_Click(object sender, RoutedEventArgs e)
        {
            var openQueueResultsDialog = new OpenFileDialog();
            openQueueResultsDialog.Filter = CustomCommands.QueueResultsFilenameFilter;
            openQueueResultsDialog.InitialDirectory = Options.CurrentUserOptions.CurrentMatchQueueResultsPath;

            if (openQueueResultsDialog.ShowDialog() == true)
            {
                //OpenDatabase(openDatabaseDialog.FileName, true);
            }
        }

        private void RunMatchButton_Click(object sender, RoutedEventArgs e)
        {
            _matchingWorker.RunWorkerAsync();
        }
    }
}
