using Darwin.Database;
using Darwin.Helpers;
using Darwin.Matching;
using Darwin.Wpf.Extensions;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Media;

namespace Darwin.Wpf.ViewModel
{
    public class MatchingResultsWindowViewModel : INotifyPropertyChanged
    {
        private bool _hideInfo;
        public bool HideInfo
        {
            get => _hideInfo;
            set
            {
                _hideInfo = value;
                RaisePropertyChanged("HideInfo");
            }
        }

        private bool _hideIDs;
        public bool HideIDs
        {
            get => _hideIDs;
            set
            {
                _hideIDs = value;
                RaisePropertyChanged("HideIDs");
            }
        }

        private bool _autoScroll;
        public bool AutoScroll
        {
            get => _autoScroll;
            set
            {
                _autoScroll = value;
                RaisePropertyChanged("AutoScroll");
            }
        }

        private DatabaseFin _databaseFin;
        public DatabaseFin DatabaseFin
        {
            get => _databaseFin;
            set
            {
                _databaseFin = value;
                RaisePropertyChanged("DatabaseFin");
            }
        }

        private MatchResults _matchResults;
        public MatchResults MatchResults
        {
            get => _matchResults;
            set
            {
                _matchResults = value;
                RaisePropertyChanged("MatchResults");
            }
        }

        private DarwinDatabase _database;
        public DarwinDatabase Database
        {
            get => _database;
            set
            {
                _database = value;
                RaisePropertyChanged("Database");
            }
        }

        private Result _selectedResult;
        public Result SelectedResult
        {
            get => _selectedResult;
            set
            {
                _selectedResult = value;
                RaisePropertyChanged("SelectedResult");

                LoadSelectedResult();
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

        private ImageSource _unknownImageSource;
        public ImageSource UnknownImageSource
        {
            get => _unknownImageSource;
            set
            {
                _unknownImageSource = value;
                RaisePropertyChanged("UnknownImageSource");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public MatchingResultsWindowViewModel(
            DatabaseFin unknownFin,
            MatchResults matchResults,
            DarwinDatabase database)
        {
            if (unknownFin == null)
                throw new ArgumentNullException(nameof(unknownFin));

            if (matchResults == null)
                throw new ArgumentNullException(nameof(matchResults));

            if (database == null)
                throw new ArgumentNullException(nameof(database));

            HideInfo = false;
            HideIDs = false;
            AutoScroll = false;

            DatabaseFin = unknownFin;

            if (DatabaseFin != null && DatabaseFin.FinImage != null)
                UnknownImageSource = DatabaseFin.FinImage.ToImageSource();

            MatchResults = matchResults;
            Database = database;
        }

        private void LoadSelectedResult()
        {
            if (SelectedResult != null)
            {
                // TODO: Cache images?
                if (!string.IsNullOrEmpty(SelectedResult.ImageFilename))
                {
                    //CatalogSupport.UpdateFinFieldsFromImage(Options.CurrentUserOptions.CurrentDataPath, SelectedFin);

                    //SelectedContour = new Contour(SelectedResult.dbContour, SelectedResult..Scale);

                    string fullImageFilename = Path.Combine(Options.CurrentUserOptions.CurrentDataPath, SelectedResult.ImageFilename);

                    if (File.Exists(fullImageFilename))
                    {
                        try
                        {
                            DatabaseFin tempFin = new DatabaseFin()
                            {
                                ImageFilename = SelectedResult.ImageFilename
                            };

                            CatalogSupport.UpdateFinFieldsFromImage(Options.CurrentUserOptions.CurrentDataPath, tempFin);

                            var img = System.Drawing.Image.FromFile(fullImageFilename);

                            var bitmap = new Bitmap(img);
                            // TODO: Hack for HiDPI -- this should be more intelligent.
                            bitmap.SetResolution(96, 96);

                            // TODO: Refactor this so we're not doing it every time, which is a little crazy
                            if (tempFin.ImageMods != null && tempFin.ImageMods.Count > 0)
                                bitmap = ModificationHelper.ApplyImageModificationsToOriginal(bitmap, tempFin.ImageMods);

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
