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
    /// Interaction logic for MatchingWindow.xaml
    /// </summary>
    public partial class MatchingWindow : Window
    {
        private MatchingWindowViewModel _vm;
        public MatchingWindow(MatchingWindowViewModel vm)
        {
            InitializeComponent();

            _vm = vm;
            this.DataContext = _vm;
        }

        private void StartButton_Click(object sender, RoutedEventArgs e)
        {

        }

        private void PauseButton_Click(object sender, RoutedEventArgs e)
        {

        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {

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
    }
}
