using Darwin.Collections;
using Darwin.Wpf.Extensions;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using Darwin.Database;
using Darwin.Wpf.Model;
using System.Collections.ObjectModel;
using System.Windows;
using System.IO;

namespace Darwin.Wpf.ViewModel
{
    public class TraceWindowViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

		private string _windowTitle;
		public string WindowTitle
        {
			get
			{
				if (!string.IsNullOrEmpty(_windowTitle))
					return _windowTitle;

				return "Trace";
			}
			set
            {
				_windowTitle = value;
				RaisePropertyChanged("WindowTitle");
            }
        }

		private Bitmap _bitmap;
		public Bitmap Bitmap
		{
			get => _bitmap;
			set
			{
				_bitmap = value;
				RaisePropertyChanged("Bitmap");

				if (_bitmap != null)
				{
					// Copy it to the OriginalBitmap property, as well, if it hasn't been set
					if (OriginalBitmap == null)
						OriginalBitmap = new Bitmap(_bitmap);

					if (BaseBitmap == null)
						BaseBitmap = new Bitmap(_bitmap);

					ImageSource = _bitmap.ToImageSource();
				}
				else if (ImageSource != null)
                {
					ImageSource = null;
                }
			}
		}

		private ImageSource _imageSource;
		public ImageSource ImageSource
		{
			get => _imageSource;
			set
			{
				_imageSource = value;
				RaisePropertyChanged("ImageSource");
			}
		}

		public void UpdateImage()
        {
			ImageSource = _bitmap.ToImageSource();
        }

		private Bitmap _baseBitmap;
		/// <summary>
		/// The "base" bitmap that brightness/contrast operate on.
		/// </summary>
		public Bitmap BaseBitmap
		{
			get => _baseBitmap;
			set
			{
				_baseBitmap = value;
				RaisePropertyChanged("BaseBitmap");
			}
		}

		private Bitmap _originalBitmap;

		/// <summary>
		/// The original bitmap/image prior to any modifications.
		/// </summary>
		public Bitmap OriginalBitmap
		{
			get => _originalBitmap;
			set
			{
				_originalBitmap = value;
				RaisePropertyChanged("OriginalBitmap");
			}
		}

		private Contour _contour;
		public Contour Contour
		{
			get => _contour;
			set
			{
				_contour = value;
				RaisePropertyChanged("Contour");

				if (_contour != null && _contour.Length > 0)
					IdentifyFeaturesEnabled = true;
				else
					IdentifyFeaturesEnabled = false;
			}
		}

		private bool _identifyFeaturesEnabled;
		public bool IdentifyFeaturesEnabled
        {
			get => _identifyFeaturesEnabled;
			set
            {
				_identifyFeaturesEnabled = value;
				RaisePropertyChanged("IdentifyFeaturesEnabled");
            }
        }

		private Contour _backupContour;
		public Contour BackupContour
		{
			get => _backupContour;
			set
			{
				_backupContour = value;
				RaisePropertyChanged("BackupContour");
			}
		}

		private Outline _outline;
		public Outline Outline
		{
			get => _outline;
			set
			{
				_outline = value;
				RaisePropertyChanged("Outline");
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

		private TraceToolType _traceTool;
		public TraceToolType TraceTool
		{
			get => _traceTool;
			set
			{
				_traceTool = value;
				RaisePropertyChanged("TraceTool");
			}
		}

		private TraceStepType _traceStep;
		public TraceStepType TraceStep
		{
			get => _traceStep;
			set
			{
				_traceStep = value;
				RaisePropertyChanged("TraceStep");
			}
		}

		private float _normScale;
		public float NormScale
        {
			get => _normScale;
            set
            {
				_normScale = value;
				RaisePropertyChanged("NormScale");
            }
        }

		private bool _traceLocked;
		public bool TraceLocked
		{
			get => _traceLocked;
			set
			{
				_traceLocked = value;
				RaisePropertyChanged("TraceLocked");
			}
		}

		private Visibility _traceToolsVisibility;
		public Visibility TraceToolsVisibility
        {
			get => _traceToolsVisibility;
			set
            {
				_traceToolsVisibility = value;
				RaisePropertyChanged("TraceToolsVisibility");

				if (_traceToolsVisibility == Visibility.Visible)
					FeatureToolsVisibility = Visibility.Hidden;
				else
					FeatureToolsVisibility = Visibility.Visible;
            }
        }

		private Visibility _featureToolsVisibility;
		public Visibility FeatureToolsVisibility
		{
			get => _featureToolsVisibility;
			set
			{
				_featureToolsVisibility = value;
				RaisePropertyChanged("FeatureToolsVisibility");
			}
		}


		// TODO: Do we need both finalized & locked?
		private bool _traceFinalized;
		public bool TraceFinalized
		{
			get => _traceFinalized;
			set
			{
				_traceFinalized = value;
				RaisePropertyChanged("TraceFinalized");

				if (TraceFinalized)
					TraceToolsVisibility = Visibility.Hidden;
				else
					TraceToolsVisibility = Visibility.Visible;
			}
		}

		private bool _traceSnapped;
		public bool TraceSnapped
		{
			get => _traceSnapped;
			set
			{
				_traceSnapped = value;
				RaisePropertyChanged("TraceSnapped");
			}
		}

		//private bool _imageLocked;
		//public bool ImageLocked
		//{
		//	get => _imageLocked;
		//	set
		//	{
		//		_imageLocked = value;
		//		RaisePropertyChanged("ImageLocked");
		//	}
		//}

		private float _zoomRatio;
		public float ZoomRatio
		{
			get => _zoomRatio;
			set
			{
				_zoomRatio = value;
				RaisePropertyChanged("ZoomRatio");
			}
		}

		private List<double> _zoomValues;
		public List<double> ZoomValues
		{
			get => _zoomValues;
			set
			{
				_zoomValues = value;
				RaisePropertyChanged("ZoomValues");
			}
		}

		public float ZoomPointSize
		{
			get
			{
				if (_zoomRatio > 0.5f)
					return Options.CurrentUserOptions.DrawingPointSize * _zoomRatio;

				return Options.CurrentUserOptions.DrawingPointSize;
			}
		}

		public bool UndoEnabled
		{
			get
			{
				if (_undoItems == null)
					return false;

				return _undoItems.Count > 0;
			}
		}
		public ObservableStack<Modification> _undoItems;
		public ObservableStack<Modification> UndoItems
		{
			get => _undoItems;
			set
			{
				_undoItems = value;
				RaisePropertyChanged("UndoItems");
				RaisePropertyChanged("UndoEnabled");
			}
		}

		public bool RedoEnabled
        {
            get
            {
				if (_redoItems == null)
					return false;

				return _redoItems.Count > 0;
            }
        }
		public ObservableStack<Modification> _redoItems;
		public ObservableStack<Modification> RedoItems
		{
			get => _redoItems;
			set
			{
				_redoItems = value;
				RaisePropertyChanged("RedoItems");
				RaisePropertyChanged("RedoEnabled");
			}
		}

		private ObservableCollection<DBDamageCategory> _categories;
		public ObservableCollection<DBDamageCategory> Categories
		{
			get => _categories;
			set
			{
				_categories = value;
				RaisePropertyChanged("Categories");
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

		public TraceWindowViewModel()
        {
			TraceToolsVisibility = Visibility.Visible;
			DatabaseFin = new DatabaseFin();
			TraceStep = TraceStepType.TraceOutline;
			Categories = new ObservableCollection<DBDamageCategory>();
			AttachEvents();
		}

		public TraceWindowViewModel(DatabaseFin fin)
        {
			TraceToolsVisibility = Visibility.Visible;
			LoadFin(fin);

			AttachEvents();
		}

		public TraceWindowViewModel(DatabaseFin fin, DarwinDatabase db, List<DBDamageCategory> categories)
        {
			TraceToolsVisibility = Visibility.Visible;
			Database = db;
			Categories = new ObservableCollection<DBDamageCategory>(categories);

			LoadFin(fin);
			
			AttachEvents();
		}

		public TraceWindowViewModel(string imageFilename, DarwinDatabase db, List<DBDamageCategory> categories)
		{
			TraceToolsVisibility = Visibility.Visible;
			DatabaseFin = new DatabaseFin();

			if (categories != null && categories.Count > 0)
				DatabaseFin.DamageCategory = categories[0].name;

			Categories = new ObservableCollection<DBDamageCategory>(categories);

			Database = db;

			//ImageLocked = false;
			TraceLocked = false;

			NormScale = 1.0f;

			TraceStep = TraceStepType.TraceOutline;
			TraceTool = TraceToolType.Hand;
			ZoomRatio = 1.0f;
			ZoomValues = new List<double>();

			OpenImage(imageFilename);

			AttachEvents();
		}

		public TraceWindowViewModel(
			Bitmap bitmap,
			Contour contour,
			Outline outline,
			//bool imageLocked,
			bool traceLocked,
			DarwinDatabase db,
			List<DBDamageCategory> categories)
        {
			TraceToolsVisibility = Visibility.Visible;

			DatabaseFin = new DatabaseFin();

			if (categories != null && categories.Count > 0)
				DatabaseFin.DamageCategory = categories[0].name;

			Bitmap = bitmap;
			Contour = contour;
			Outline = outline;

			Categories = new ObservableCollection<DBDamageCategory>(categories);

			Database = db;

			//ImageLocked = imageLocked;
			// TODO: Clean these flags up, maybe?
			TraceLocked = traceLocked;
			TraceFinalized = traceLocked;
			
			NormScale = 1.0f;

			TraceTool = TraceToolType.Hand;
			ZoomRatio = 1.0f;
			ZoomValues = new List<double>();

			AttachEvents();
		}

		public void OpenImage(string filename)
        {
			var img = System.Drawing.Image.FromFile(filename);

			var bitmap = new Bitmap(img);
			// TODO: Hack for HiDPI -- this should be more intelligent.
			bitmap.SetResolution(96, 96);

			Bitmap = bitmap;

			DatabaseFin.OriginalFinImage = new Bitmap(bitmap);
			DatabaseFin.ImageFilename = DatabaseFin.OriginalImageFilename = filename;
		}

		public void SaveFinz(string filename)
        {
			UpdateDatabaseFin();
			CatalogSupport.SaveFinz(DatabaseFin, filename);
        }

		public void SaveToDatabase()
        {
			UpdateDatabaseFin();
			CatalogSupport.SaveToDatabase(Database, DatabaseFin);
		}

		public void SaveSightingData()
        {
			if (DatabaseFin != null)
            {
				// TODO: Filename logic should probably be elsewhere
				var filename = Path.Combine(Options.CurrentUserOptions.CurrentSightingsPath, "SightingDataLogForArea_" + Options.CurrentUserOptions.CurrentSurveyArea + ".txt");
				DatabaseFin.SaveSightingData(filename);
			}
        }

		public void UpdateDatabaseFin()
        {
			DatabaseFin.Scale = NormScale;
			DatabaseFin.FinOutline = Outline;
			DatabaseFin.FinImage = new Bitmap(Bitmap);

			if (UndoItems == null)
			{
				DatabaseFin.ImageMods = new List<ImageMod>();
			}
			else
            {
				DatabaseFin.ImageMods = UndoItems
					.Where(u => u.ImageMod != null && (u.ModificationType == ModificationType.Image || u.ModificationType == ModificationType.Both))
					.Select(u => u.ImageMod)
					.ToList();
            }
		}

		private void LoadFin(DatabaseFin fin)
		{
			WindowTitle = fin.IDCode;

			// TODO: Hack for HiDPI
			fin.FinImage.SetResolution(96, 96);

			DatabaseFin = fin;

			Bitmap = fin.FinImage ?? fin.OriginalFinImage;
			Contour = new Contour(fin.FinOutline, fin.Scale);
			Outline = fin.FinOutline;

			if (Categories == null)
				Categories = new ObservableCollection<DBDamageCategory>();

			if (!Categories.Any(c => c.name == fin?.DamageCategory))
			{
				Categories.Add(new DBDamageCategory
				{
					name = fin?.DamageCategory
				});
			}

			// ImageLocked = true;
			TraceLocked = true;
			TraceFinalized = true;

			NormScale = (float)fin.Scale;

			TraceTool = TraceToolType.Hand;
			ZoomRatio = 1.0f;
			ZoomValues = new List<double>();
		}

		private void AttachEvents()
        {
			UndoItems = new ObservableStack<Modification>();
			UndoItems.CollectionChanged += UndoItemsCollectionChanged;
			RedoItems = new ObservableStack<Modification>();
			RedoItems.CollectionChanged += RedoItemsCollectionChanged;
		}

		private void UndoItemsCollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
			RaisePropertyChanged("UndoEnabled");
		}

		private void RedoItemsCollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
		{
			RaisePropertyChanged("RedoEnabled");
		}

		private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
