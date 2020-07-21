using Darwin.Collections;
using Darwin.Database;
using Darwin.Helpers;
using Darwin.Wpf.Extensions;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Media;

namespace Darwin.Wpf.ViewModel
{
    public class MainWindowViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public string WindowTitle
        {
            get
            {
                if (DarwinDatabase == null)
                    return "DARWIN";

                return Path.GetFileName(DarwinDatabase.Filename) + " - DARWIN";
            }
        }

        private DarwinDatabase _darwinDatabase;
        public DarwinDatabase DarwinDatabase
        {
            get
            {
                return _darwinDatabase;
            }
            set
            {
                _darwinDatabase = value;
                RaisePropertyChanged("FeatureSetTypeDisplay");
                RaisePropertyChanged("FeatureSetTypeVisibility");
                RaisePropertyChanged("DarwinDatabase");
                RaisePropertyChanged("WindowTitle");
            }
        }

        public Visibility FeatureSetTypeVisibility
        {
            get
            {
                if (string.IsNullOrEmpty(FeatureSetTypeDisplay))
                    return Visibility.Collapsed;

                return Visibility.Visible;
            }
        }
        public string FeatureSetTypeDisplay
        {
            get
            {
                if (DarwinDatabase == null || DarwinDatabase.CatalogScheme == null)
                    return string.Empty;

                return DarwinDatabase.CatalogScheme.CollectionTerminology;
            }
        }

        private DatabaseFin _selectedFin;
        public DatabaseFin SelectedFin
        {
            get => _selectedFin;
            set
            {
                _selectedFin = value;
                RaisePropertyChanged("SelectedFin");

                LoadSelectedFin();
            }
        }

        private ImageSource _selectedImageSource;
        public ImageSource SelectedImageSource
        {
            get => _selectedImageSource;
            set
            {
                _selectedImageSource = value;
                RaisePropertyChanged("SelectedImageSource");
            }
        }

        private ImageSource _selectedOriginalImageSource;
        public ImageSource SelectedOriginalImageSource
        {
            get => _selectedOriginalImageSource;
            set
            {
                _selectedOriginalImageSource = value;
                RaisePropertyChanged("SelectedOriginalImageSource");
            }
        }

        private Contour _selectedContour;
        public Contour SelectedContour
        {
            get => _selectedContour;
            set
            {
                _selectedContour = value;
                RaisePropertyChanged("SelectedContour");
            }
        }


        private ObservableNotifiableCollection<DatabaseFin> _fins;
        public ObservableNotifiableCollection<DatabaseFin> Fins
        {
            get
            {
                if (_fins == null)
                    _fins = new ObservableNotifiableCollection<DatabaseFin>();

                return _fins;
            }
            set
            {
                _fins = value;
                RaisePropertyChanged("Fins");
            }
        }

        private bool _nextEnabled;
        public bool NextEnabled
        {
            get => _nextEnabled;
            set
            {
                _nextEnabled = value;
                RaisePropertyChanged("NextEnabled");
            }
        }

        private bool _previousEnabled;
        public bool PreviousEnabled
        {
            get => _previousEnabled;
            set
            {
                _previousEnabled = value;
                RaisePropertyChanged("PreviousEnabled");
            }
        }

        public MainWindowViewModel()
        {
            _darwinDatabase = null;
            _fins = new ObservableNotifiableCollection<DatabaseFin>();
        }

        public void RefreshDatabase()
        {
            // This should probably do more
            RaisePropertyChanged("FeatureSetTypeDisplay");
            RaisePropertyChanged("FeatureSetTypeVisibility");
        }

        public string BackupDatabase()
        {
            return CatalogSupport.BackupDatabase(DarwinDatabase);
        }

        public void SaveSelectedItemAsFinz(string filename)
        {
            if (SelectedFin == null)
                return;

            if (string.IsNullOrEmpty(filename))
                throw new ArgumentNullException(nameof(filename));

            var finCopy = FullyLoadFin();
            CatalogSupport.SaveFinz(DarwinDatabase.CatalogScheme, finCopy, filename, true);
        }

        public DatabaseFin FullyLoadFin()
        {
            DatabaseFin finCopy = null;

            if (SelectedFin != null)
                finCopy = CatalogSupport.FullyLoadFin(SelectedFin);

            return finCopy;
        }

        public void CheckSurveyAreaDatabaseNameFromBackup(string backupFile, out string surveyArea, out string databaseName)
        {
            CatalogSupport.CheckSurveyAreaDatabaseNameFromBackup(backupFile, out surveyArea, out databaseName);
        }

        public void CloseDatabase()
        {
            DarwinDatabase = null;
            SelectedFin = null;
            Fins = null;
            SelectedImageSource = null;
            SelectedOriginalImageSource = null;
            CatalogSupport.CloseDatabase(DarwinDatabase);
        }

        public string RestoreDatabase(string backupFile, string surveyArea, string databaseName)
        {
            return CatalogSupport.RestoreDatabase(backupFile, surveyArea, databaseName);
        }

        private void LoadSelectedFin()
        {
            if (SelectedFin == null)
            {
                SelectedImageSource = null;
                SelectedOriginalImageSource = null;
                SelectedContour = null;
            }
            else
            {
                // TODO: Cache images?
                if (!string.IsNullOrEmpty(SelectedFin.ImageFilename))
                {
                    CatalogSupport.UpdateFinFieldsFromImage(Options.CurrentUserOptions.CurrentSurveyAreaPath, SelectedFin);

                    SelectedContour = new Contour(SelectedFin.FinOutline.ChainPoints, SelectedFin.Scale);

                    string fullImageFilename = Path.Combine(Options.CurrentUserOptions.CurrentSurveyAreaPath,
                        (string.IsNullOrEmpty(SelectedFin.OriginalImageFilename)) ? SelectedFin.ImageFilename : SelectedFin.OriginalImageFilename);

                    if (File.Exists(fullImageFilename))
                    {
                        try
                        {
                            var img = System.Drawing.Image.FromFile(fullImageFilename);

                            var bitmap = new Bitmap(img);
                            // TODO: Hack for HiDPI -- this should be more intelligent.
                            bitmap.SetResolution(96, 96);

                            SelectedOriginalImageSource = bitmap.ToImageSource();

                            // TODO: Refactor this so we're not doing it every time, which is a little crazy
                            if (SelectedFin.ImageMods != null && SelectedFin.ImageMods.Count > 0)
                            {
                                bitmap = ModificationHelper.ApplyImageModificationsToOriginal(bitmap, SelectedFin.ImageMods);
                               // TODO: HiDPI hack
                                bitmap.SetResolution(96, 96);
                            }

                            // We're directly changing the source, not the bitmap property on DatabaseFin
                            SelectedImageSource = bitmap.ToImageSource();
                        }
                        catch (Exception ex)
                        {
                            // TODO
                            MessageBox.Show(ex.ToString());
                        }
                    }
                }
            }
        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
