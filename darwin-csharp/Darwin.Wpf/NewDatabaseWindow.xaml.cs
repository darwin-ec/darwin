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
    /// Interaction logic for NewDatabaseWindow.xaml
    /// </summary>
    public partial class NewDatabaseWindow : Window
    {
        private NewDatabaseViewModel _vm;

        public NewDatabaseWindow(NewDatabaseViewModel vm)
        {
            InitializeComponent();

            _vm = vm;
            DataContext = _vm;
        }
    }
}
