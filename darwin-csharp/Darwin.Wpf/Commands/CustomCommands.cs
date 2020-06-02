using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Input;

namespace Darwin.Wpf.Commands
{
    public static class CustomCommands
    {
        public static readonly RoutedUICommand Open = new RoutedUICommand(
            "Open",
            "Open",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.O, ModifierKeys.Control)
            }
        );
        public static string OpenFilter = "Image files|*.jpg;*.bmp;*.png;*.tif;*.ppm|All files|*.*";

        public static readonly RoutedUICommand OpenDatabase = new RoutedUICommand(
            "Open Database",
            "Open Database",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.D, ModifierKeys.Control)
            }
        );
        public static string OpenDatabaseFilter = "Database files (*.db)|*.db";

        public static readonly RoutedUICommand Exit = new RoutedUICommand(
            "Exit",
            "Exit",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.F4, ModifierKeys.Alt)
            }
        );
    }
}
