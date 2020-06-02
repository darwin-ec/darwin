using Darwin.Collections;
using Darwin.Database;
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

                LoadSelectedImages();
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

        public MainWindowViewModel()
        {
            _darwinDatabase = null;
            _fins = new ObservableNotifiableCollection<DatabaseFin>();
        }

        private void LoadSelectedImages()
        {
            if (SelectedFin != null)
            {
                // TODO: Cache images?
                if (!string.IsNullOrEmpty(SelectedFin.ImageFilename))
                {
                    string fullImageFilename = Path.Combine(Options.CurrentUserOptions.CurrentDataPath, SelectedFin.ImageFilename);

                    if (File.Exists(fullImageFilename))
                    {
                        try
                        {
                            var img = System.Drawing.Image.FromFile(fullImageFilename);

                            var bitmap = new Bitmap(img);
                            // TODO: Hack for HiDPI -- this should be more intelligent.
                            bitmap.SetResolution(96, 96);

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
