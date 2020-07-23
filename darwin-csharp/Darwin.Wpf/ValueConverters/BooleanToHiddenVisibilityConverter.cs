using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text;
using System.Windows;
using System.Windows.Data;

namespace Darwin.Wpf.ValueConverters
{
    /// <summary>
    /// Same thing as BooleanToHiddenVisibilityConverter, except it converts to Visibility.Hidden rather
    /// than Visibility.Collapsed when the value is false.
    /// </summary>
    public class BooleanToHiddenVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            bool flag = false;
            if (value is bool)
            {
                flag = (bool)value;
            }
            else if (value is bool?)
            {
                bool? nullable = (bool?)value;
                flag = nullable.HasValue ? nullable.Value : false;
            }
            return (flag ? Visibility.Visible : Visibility.Hidden);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return Binding.DoNothing;
        }
    }
}
