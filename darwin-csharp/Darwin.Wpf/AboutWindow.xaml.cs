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
                // Assembly Version
                //var version = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;

                // Assembly File Version
                var version = FileVersionInfo.GetVersionInfo(System.Reflection.Assembly.GetExecutingAssembly().Location).FileVersion;
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

        private void OKButton_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void AboutWindow_PreviewKeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Escape)
                Close();
        }
    }
}