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
    /// Interaction logic for CatalogSchemesWindow.xaml
    /// </summary>
    public partial class CatalogSchemesWindow : Window
    {
        private CatalogSchemesViewModel _vm;

        public CatalogSchemesWindow(CatalogSchemesViewModel vm)
        {
            InitializeComponent();

            _vm = vm;

            DataContext = _vm;
        }

        private void SaveButton_Click(object sender, RoutedEventArgs e)
        {
            Options.CurrentUserOptions.CatalogSchemes = new List<Database.CatalogScheme>(_vm.CatalogSchemes);
            Options.CurrentUserOptions.Save();
            Close();
        }

        private void CancelButton_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void AddButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.AddCatalogScheme();

            //ListBoxItem selectedItem = SchemesListBox.ItemContainerGenerator.ContainerFromIndex(SchemesListBox.SelectedIndex - 1) as ListBoxItem;

            //if (selectedItem != null)
            //{
            //    ContentPresenter contentPresenter = FindVisualChild<ContentPresenter>(selectedItem);
            //    DataTemplate dataTemplate = contentPresenter.ContentTemplate;
            //    TextBox textBox = dataTemplate.FindName("CatalogSchemeName", contentPresenter) as TextBox;

            //    if (textBox != null)
            //        textBox.Dispatcher.BeginInvoke(new Action(() => textBox.SelectAll()));
            //}
        }

        private T FindVisualChild<T>(DependencyObject obj) where T : DependencyObject
        {
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(obj); i++)
            {
                DependencyObject child = VisualTreeHelper.GetChild(obj, i);
                if (child != null && child is T)
                    return (T)child;
                else
                {
                    T childOfChild = FindVisualChild<T>(child);
                    if (childOfChild != null)
                        return childOfChild;
                }
            }
            return null;
        }

        // This is used by both ListBox controls
        private void ListBoxItem_SelectCurrentItem(object sender, KeyboardFocusChangedEventArgs e)
        {
            ListBoxItem item = sender as ListBoxItem;

            if (item != null && !item.IsSelected)
                item.IsSelected = true;

            if (item != null)
            {
                ContentPresenter contentPresenter = FindVisualChild<ContentPresenter>(item);
                DataTemplate dataTemplate = contentPresenter.ContentTemplate;
                TextBox textBox = dataTemplate.FindName("CatalogSchemeName", contentPresenter) as TextBox;

                if (textBox == null)
                    textBox = dataTemplate.FindName("CategoryNameTextBox", contentPresenter) as TextBox;

                if (textBox != null)
                    textBox.Dispatcher.BeginInvoke(new Action(() => textBox.SelectAll()));
            }
        }

        private void RemoveButton_Click(object sender, RoutedEventArgs e)
        {
            if (_vm.SelectedScheme != null && _vm.SelectedScheme.IsBuiltIn)
            {
                MessageBox.Show("You can't remove the Eckerd College scheme.", "Can't Remove", MessageBoxButton.OK, MessageBoxImage.Warning);
            }
            else
            {
                _vm.RemoveSelectedScheme();
            }
        }

        private void SetDefaultButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.SetSelectedDefault();
        }

        private void UpButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.MoveSelectedUp();
        }

        private void DownButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.MoveSelectedDown();
        }

        private void AddCategoryNameButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.AddCategory();
        }

        private void RemoveCategoryNameButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.RemoveCategory();
        }

        private void UpCategoryNameButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.MoveSelectedCategoryUp();
        }

        private void DownCategoryNameButton_Click(object sender, RoutedEventArgs e)
        {
            _vm.MoveSelectedCategoryDown();
        }
    }
}
