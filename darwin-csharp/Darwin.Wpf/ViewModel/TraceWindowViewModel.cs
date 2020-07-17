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
					FeatureToolsVisibility = Visibility.Collapsed;
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

		private Visibility _matchVisibility;
		public Visibility MatchVisibility
		{
			get => _matchVisibility;
			set
			{
				_matchVisibility = value;
				RaisePropertyChanged("MatchVisibility");
			}
		}

		private Visibility _topToolbarVisibility;
		public Visibility TopToolbarVisibility
		{
			get => _topToolbarVisibility;
			set
			{
				_topToolbarVisibility = value;
				RaisePropertyChanged("TopToolbarVisibility");
			}
		}

		private Visibility _saveVisibility;
		public Visibility SaveVisibility
		{
			get => _saveVisibility;
			set
			{
				_saveVisibility = value;
				RaisePropertyChanged("SaveVisibility");
			}
		}

		public bool ViewerMode { get; set; }

		private Visibility _addToDatabaseVisibility;
		public Visibility AddToDatabaseVisibility
        {
			get => _addToDatabaseVisibility;
			set
			{
				_addToDatabaseVisibility = value;
				RaisePropertyChanged("AddToDatabaseVisibility");
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
					TraceToolsVisibility = Visibility.Collapsed;
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


		private bool _preventPropagation;

		private float _zoomRatio;
		public float ZoomRatio
		{
			get => _zoomRatio;
			set
			{
				_zoomRatio = value;

				RaisePropertyChanged("ZoomRatio");

				if (_preventPropagation)
				{
					_preventPropagation = false;
				}
				else
				{
					var zoomSlider = (float)Math.Round(Math.Log(_zoomRatio) / Math.Log(2), 4);

					if (zoomSlider != _zoomSlider)
					{
						_preventPropagation = true;
						ZoomSlider = zoomSlider;
					}
				}
			}
		}

		private float _zoomSlider;
		public float ZoomSlider
        {
			get => _zoomSlider;
			set
            {
				_zoomSlider = value;

				RaisePropertyChanged("ZoomSlider");

				if (_preventPropagation)
                {
					_preventPropagation = false;
                }
                else
				{ 
					var newRatio = (float)Math.Round(Math.Pow(2, _zoomSlider), 4);

					if (ZoomRatio != newRatio)
					{
						_preventPropagation = true;
						ZoomRatio = newRatio;
					}
				}
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

		private ObservableCollection<Category> _categories;
		public ObservableCollection<Category> Categories
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

		public string IndividualTerminology
		{
			get
			{
				return Database?.CatalogScheme?.IndividualTerminology;
			}
		}

		public string IndividualTerminologyInitialCaps
		{
			get
			{
				return Database?.CatalogScheme?.IndividualTerminologyInitialCaps;
			}
		}

		private MatchingResultsWindow _matchingResultsWindow;

		private TraceWindowViewModel()
        {
			TopToolbarVisibility = Visibility.Visible;
			TraceToolsVisibility = Visibility.Visible;
			SaveVisibility = Visibility.Visible;
			MatchVisibility = Visibility.Visible;
			AddToDatabaseVisibility = Visibility.Visible;

			//DatabaseFin = new DatabaseFin();
			TraceStep = TraceStepType.TraceOutline;
			Categories = new ObservableCollection<Category>();
			AttachEvents();
		}

		public TraceWindowViewModel(DatabaseFin fin, bool viewOnly = false)
			: this()
        {
			LoadFin(fin);

			if (viewOnly)
			{
				TraceTool = TraceToolType.Hand;
				MatchVisibility = Visibility.Collapsed;
				SaveVisibility = Visibility.Collapsed;
				AddToDatabaseVisibility = Visibility.Collapsed;
				ViewerMode = true;

				TopToolbarVisibility = Visibility.Collapsed;
				TraceLocked = true;
				TraceFinalized = true;
				FeatureToolsVisibility = Visibility.Collapsed;
			}
		}

		public TraceWindowViewModel(DatabaseFin fin, DarwinDatabase db)
			: this(fin)
        {
			Database = db;
			Categories = db.Categories;
		}

		// Little hacky, keeping a reference to the MatchResultsWindow, so we can close it when adding to the DB
		public TraceWindowViewModel(DatabaseFin fin, DarwinDatabase db, string windowTitle, MatchingResultsWindow matchingResultsWindow)
			: this(fin, db)
        {
			WindowTitle = windowTitle;

			if (matchingResultsWindow != null)
			{
				_matchingResultsWindow = matchingResultsWindow;
				MatchVisibility = Visibility.Collapsed;
				SaveVisibility = Visibility.Collapsed;

				TopToolbarVisibility = Visibility.Collapsed;
				TraceLocked = true;
				TraceFinalized = true;
				FeatureToolsVisibility = Visibility.Collapsed;
			}
        }

		public TraceWindowViewModel(DatabaseFin fin, DarwinDatabase db, string windowTitle, MainWindow mainWindow, bool viewOnly = false)
			: this(fin, db)
		{
			WindowTitle = windowTitle;

			if (mainWindow != null || viewOnly)
			{
				TraceTool = TraceToolType.Hand;
				MatchVisibility = Visibility.Collapsed;
				SaveVisibility = Visibility.Collapsed;
				AddToDatabaseVisibility = Visibility.Collapsed;
				ViewerMode = true;

				TopToolbarVisibility = Visibility.Collapsed;
				TraceLocked = true;
				TraceFinalized = true;
				FeatureToolsVisibility = Visibility.Collapsed;
			}
		}

		public TraceWindowViewModel(string imageFilename, DarwinDatabase db)
			: this()
		{
			DatabaseFin = new DatabaseFin();

			if (db.Categories != null && db.Categories.Count > 0)
				DatabaseFin.DamageCategory = db.Categories[0].Name;

			Categories = db.Categories;

			Database = db;

			//ImageLocked = false;
			TraceLocked = false;

			NormScale = 1.0f;

			TraceStep = TraceStepType.TraceOutline;
			TraceTool = TraceToolType.Hand;
			ZoomRatio = 1.0f;
			ZoomValues = new List<double>();

			OpenImage(imageFilename);

			WindowTitle = Path.GetFileName(imageFilename);
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
			CatalogSupport.SaveFinz(Database.CatalogScheme, DatabaseFin, filename);
        }

		public void SaveToDatabase()
        {
			UpdateDatabaseFin();
			CatalogSupport.SaveToDatabase(Database, DatabaseFin);

			// Check whether we have a reference to the MatchingResultsWindow.  If so,
			// we got this fin passed back as a match/no match.  When we add to the database,
			// let's close the matching results window.
			if (_matchingResultsWindow != null)
				_matchingResultsWindow.Close();
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

			if (DatabaseFin.ImageMods == null)
				DatabaseFin.ImageMods = new List<ImageMod>();

			var addedMods = new List<ImageMod>();

			if (UndoItems != null)
			{
				addedMods = UndoItems
					.Where(u => u.ImageMod != null && (u.ModificationType == ModificationType.Image || u.ModificationType == ModificationType.Both))
					.Select(u => u.ImageMod)
					.ToList();
            }

			// We're adding mods in case we opened up an old Finz file/etc.  So we don't blow away previous
			// mods done to the image.
			DatabaseFin.ImageMods.Concat(addedMods);
		}

		private void LoadFin(DatabaseFin fin)
		{
			WindowTitle = fin.IDCode;

			// TODO: Hack for HiDPI
			fin.FinImage.SetResolution(96, 96);

			DatabaseFin = fin;

			Bitmap = fin.FinImage ?? fin.OriginalFinImage;

			if (fin.FinOutline == null || fin.FinOutline.ChainPoints == null)
				Contour = null;
			else
				Contour = new Contour(fin.FinOutline, fin.Scale);

			Outline = fin.FinOutline;

			if (Categories == null)
				Categories = new ObservableCollection<Category>();

			if (!Categories.Any(c => c.Name == fin?.DamageCategory))
			{
				Categories.Add(new Category
				{
					Name = fin?.DamageCategory
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
			PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
		}
    }
}
