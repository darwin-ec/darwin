// This file is part of DARWIN.
// Copyright (C) 1994 - 2020
//
// DARWIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DARWIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DARWIN.  If not, see<https://www.gnu.org/licenses/>.

using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Input;

namespace Darwin.Wpf.Commands
{
    public static class CustomCommands
    {
        public static readonly RoutedUICommand NewDatabase = new RoutedUICommand(
            "New Database...",
            "New Database",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.N, ModifierKeys.Control)
            }
        );

        public static readonly RoutedUICommand OpenImage = new RoutedUICommand(
            "Open Image...",
            "Open Image",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.O, ModifierKeys.Control)
            }
        );
        public static string OpenImageFilter = "Image files|*.jpg;*.bmp;*.png;*.tif|All files|*.*";

        public static readonly RoutedUICommand OpenTracedFin = new RoutedUICommand(
            "Open Trace...",
            "Open Trace",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.T, ModifierKeys.Control)
            }
        );
        public static string TracedFinFilter = "Finz files|*.finz";

        public static readonly RoutedUICommand OpenDatabase = new RoutedUICommand(
            "Open Database...",
            "Open Database",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.D, ModifierKeys.Control)
            }
        );
        public static string OpenDatabaseFilter = "Database files (*.db)|*.db";

        public static readonly RoutedUICommand CloseDatabase = new RoutedUICommand(
            "Close Database",
            "Close Database",
            typeof(CustomCommands)
        );

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
            "Matching Queue...",
            "Matching Queue",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.Q, ModifierKeys.Control)
            }
        );

        public static string QueueFilenameFilter = "Darwin Queue (*.que)|*.que";
        public static string QueueResultsFilenameFilter = "Darwin Queue Results (*.res)|*.res";

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
        public static string RestoreFilenameFilter = "Backup archives (*.zip)|*.zip";

        public static readonly RoutedUICommand CurrentCatalogScheme = new RoutedUICommand(
            "Current Catalog Scheme...",
            "Catalog Catalog Scheme",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.D1, ModifierKeys.Control)
            }
        );

        public static readonly RoutedUICommand CatalogSchemes = new RoutedUICommand(
            "Catalog Scheme Presets...",
            "Catalog Scheme Presets",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.D2, ModifierKeys.Control)
            }
        );

        public static readonly RoutedUICommand Options = new RoutedUICommand(
            "Options...",
            "Options",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.P, ModifierKeys.Control)
            }
        );

        public static readonly RoutedUICommand DeveloperTools = new RoutedUICommand(
            "Developer Tools...",
            "Developer Tools",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.F12)
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

        public static readonly RoutedUICommand Save = new RoutedUICommand(
            "Save",
            "Save",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.S, ModifierKeys.Control)
            }
        );

        public static readonly RoutedUICommand Undo = new RoutedUICommand(
            "Undo",
            "Undo",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.Z, ModifierKeys.Control)
            }
        );

        public static readonly RoutedUICommand Redo = new RoutedUICommand(
            "Redo",
            "Redo",
            typeof(CustomCommands),
            new InputGestureCollection()
            {
                new KeyGesture(Key.Y, ModifierKeys.Control)
            }
        );
    }
}
