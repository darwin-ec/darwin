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
    /// Interaction logic for MatchingResultsWindow.xaml
    /// </summary>
    public partial class MatchingResultsWindow : Window
    {
        private MatchingResultsWindowViewModel _vm;

        public MatchingResultsWindow(MatchingResultsWindowViewModel vm)
        {
            InitializeComponent();

            _vm = vm;
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

        }

        private void NextButton_Click(object sender, RoutedEventArgs e)
        {

        }

        private void MatchesSelectedFinButton_Click(object sender, RoutedEventArgs e)
        {

        }

        private void NoMatchNewFinButton_Click(object sender, RoutedEventArgs e)
        {

        }

        private void ReturnToMatchingDialogButton_Click(object sender, RoutedEventArgs e)
        {

        }

        private void SaveMatchResultsButton_Click(object sender, RoutedEventArgs e)
        {

        }

        private void DoneButton_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}