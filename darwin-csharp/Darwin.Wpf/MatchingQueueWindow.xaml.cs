using Darwin.Database;
using Darwin.Wpf.Commands;
using Darwin.Wpf.ViewModel;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
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
    /// Interaction logic for MatchingQueueWindow.xaml
    /// </summary>
    public partial class MatchingQueueWindow : Window
    {
        private MatchingQueueViewModel _vm;

        public MatchingQueueWindow(MatchingQueueViewModel vm)
        {
            InitializeComponent();

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
    }
}
