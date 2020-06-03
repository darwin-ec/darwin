using Darwin.Collections;
using Darwin.Model;
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

namespace Darwin.Wpf.ViewModel
{
    public class TraceWindowViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

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

		// TODO: Do we need both finalized & locked?
		private bool _traceFinalized;
		public bool TraceFinalized
		{
			get => _traceFinalized;
			set
			{
				_traceFinalized = value;
				RaisePropertyChanged("TraceFinalized");
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

		private bool _imageLocked;
		public bool ImageLocked
		{
			get => _imageLocked;
			set
			{
				_imageLocked = value;
				RaisePropertyChanged("ImageLocked");
			}
		}

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

		public TraceWindowViewModel()
        {
			DatabaseFin = new DatabaseFin();
			AttachEvents();
		}

		public TraceWindowViewModel(Bitmap bitmap)
		{
			DatabaseFin = new DatabaseFin();

			Bitmap = bitmap;
			ImageLocked = false;
			TraceLocked = false;

			TraceTool = TraceToolType.Hand;
			ZoomRatio = 1.0f;
			ZoomValues = new List<double>();

			AttachEvents();
		}

		public TraceWindowViewModel(Bitmap bitmap, Contour contour, Outline outline, bool imageLocked, bool traceLocked)
        {
			DatabaseFin = new DatabaseFin();

			Bitmap = bitmap;
			Contour = contour;
			Outline = outline;
			ImageLocked = imageLocked;
			TraceLocked = traceLocked;

			TraceTool = TraceToolType.Hand;
			ZoomRatio = 1.0f;
			ZoomValues = new List<double>();

			AttachEvents();
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
