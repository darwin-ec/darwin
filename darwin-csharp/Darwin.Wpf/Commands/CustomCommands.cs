using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Input;

namespace Darwin.Wpf.Commands
{
    public static class CustomCommands
    {
        public static readonly RoutedUICommand NewDatabase = new RoutedUICommand(
            "New Database",
            "New Database",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.N, ModifierKeys.Control)
            }
        );

        public static readonly RoutedUICommand OpenImage = new RoutedUICommand(
            "Open Image",
            "Open Image",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.O, ModifierKeys.Control)
            }
        );
        public static string OpenImageFilter = "Image files|*.jpg;*.bmp;*.png;*.tif;*.ppm|All files|*.*";

        public static readonly RoutedUICommand OpenTracedFin = new RoutedUICommand(
            "Open Traced Fin",
            "Open Traced Fin",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.T, ModifierKeys.Control)
            }
        );
        public static string OpenTracedFinFilter = "Finz files|*.finz";

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

        public static readonly RoutedUICommand ImportCatalog = new RoutedUICommand(
            "Import catalog",
            "Import catalog",
            typeof(CustomCommands),
            null);

        public static readonly RoutedUICommand ImportFin = new RoutedUICommand(
            "Import fin (.finz)",
            "Import fin (.finz)",
            typeof(CustomCommands),
            null);

        public static readonly RoutedUICommand ExportCatalog = new RoutedUICommand(
            "Export catalog",
            "Export catalog",
            typeof(CustomCommands),
            null);

        public static readonly RoutedUICommand ExportFin = new RoutedUICommand(
            "Export fin (.finz)",
            "Export fin (.finz)",
            typeof(CustomCommands),
            null);

        public static readonly RoutedUICommand ExportImages = new RoutedUICommand(
            "Export images (full-size)",
            "Export images (full-size)",
            typeof(CustomCommands),
            null);

        public static readonly RoutedUICommand Exit = new RoutedUICommand(
            "Exit",
            "Exit",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.F4, ModifierKeys.Alt)
            }
        );

        public static readonly RoutedUICommand MatchingQueue = new RoutedUICommand(
            "Matching Queue",
            "Matching Queue",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.Q, ModifierKeys.Control)
            }
        );

        public static readonly RoutedUICommand Backup = new RoutedUICommand(
            "Backup",
            "Backup",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                        new KeyGesture(Key.B, ModifierKeys.Control)
            }
        );

        public static readonly RoutedUICommand Restore = new RoutedUICommand(
            "Restore",
            "Restore",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.R, ModifierKeys.Control)
            }
        );

        public static readonly RoutedUICommand Options = new RoutedUICommand(
            "Options",
            "Options",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.P, ModifierKeys.Control)
            }
        );

        public static readonly RoutedUICommand About = new RoutedUICommand(
            "About Darwin",
            "About Darwin",
            typeof(CustomCommands)
        );

        public static readonly RoutedUICommand Documentation = new RoutedUICommand(
            "Documentation",
            "Documentation",
            typeof(CustomCommands)
        );
    }
}
