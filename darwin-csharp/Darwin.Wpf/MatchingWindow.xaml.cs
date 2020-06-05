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
    }
}
