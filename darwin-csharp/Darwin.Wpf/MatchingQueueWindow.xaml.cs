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
    }
}
