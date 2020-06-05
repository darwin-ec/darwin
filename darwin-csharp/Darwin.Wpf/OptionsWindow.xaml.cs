using Darwin.Wpf.Controls;
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
    /// Interaction logic for OptionsWindow.xaml
    /// </summary>
    public partial class OptionsWindow : CustomWindow
    {
        private OptionsWindowViewModel _vm;

        public OptionsWindow(OptionsWindowViewModel vm)
        {
            InitializeComponent();

            _vm = vm;
            this.DataContext = _vm;
        }

        protected override void OnSourceInitialized(EventArgs e)
        {
            HideWindowIcon();
            base.OnSourceInitialized(e);
        }


    }
}
