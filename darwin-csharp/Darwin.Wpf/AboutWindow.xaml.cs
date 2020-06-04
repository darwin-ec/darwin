using Darwin.Utilities;
using Darwin.Wpf.Controls;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Darwin.Wpf
{
    /// <summary>
    /// Interaction logic for AboutWindow.xaml
    /// </summary>
    public partial class AboutWindow : CustomWindow
    {
        public string Version
        {
            get
            {
                var version = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;

                return version.ToString();
            }
        }

        public AboutWindow()
        {
            InitializeComponent();

            DataContext = this;
        }

        protected override void OnSourceInitialized(EventArgs e)
        {
            HideWindowIcon();
            base.OnSourceInitialized(e);
        }

        private void DarwinWebsiteHyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
           string navigateUri = e.Uri.ToString();
           UrlHelper.OpenUrl(navigateUri);
           e.Handled = true;
        }
    }
}