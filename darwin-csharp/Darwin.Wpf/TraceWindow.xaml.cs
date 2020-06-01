using Darwin.Collections;
using Darwin.Database;
using Darwin.Extensions;
using Darwin.Helpers;
using Darwin.ImageProcessing;
using Darwin.Model;
using Darwin.Wpf.Adorners;
using Darwin.Wpf.Extensions;
using Darwin.Wpf.Model;
using Darwin.Wpf.ViewModel;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Security.Policy;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Xml.Schema;

namespace Darwin.Wpf
{
    /// <summary>
    /// Interaction logic for TraceWindow.xaml
    /// </summary>
    public partial class TraceWindow : Window
    {
		private CroppingAdorner _cropSelector;
		System.Windows.Point? lastCenterPositionOnTarget;
		System.Windows.Point? lastMousePositionOnTarget;
		System.Windows.Point? lastDragPoint;

		private ImagingProvider imagingProviderIndexador = new ImagingProvider();

		private const int PointSize = 2;
		private const int SpaceBetweenPoints = 3;
		private const int EraserBrushSize = 9; // krd 10/28/05
		private const int MaxZoom = 1600;
		private const int MinZoom = 6; //***1.95 - minimum is now 6% of original size

		//private SKColor _backgroundColor;

		private float _currentScale;
		//private SKSize _currentScaledSize;

		//private SKPoint _textPoint;
		//private SKRect _textBounds;

		//private SKImage _image;

		private int _zoomRatioOld = 100;

		// The image won't always fit in the container cleanly -- this is the offset from the top left
		// where we start drawing.
		//private SKPoint _imageOffset;
		//private SKRect _imageBounds;
		//private SKSize _zoomedImageSize;

		private bool _moveInit;
		private int _movePosition;
		private bool _moveFirstRun;
		private bool _moveDrawLine;

		private bool _chopInit;
		private int _chopPosition;
		private int _chopLead;

		private bool _eraseInit = false;

		private string _previousStatusBarMessage;

		private FeaturePointType _moveFeature;

		private TraceWindowViewModel _vm;

		public TraceWindow()
		{
			InitializeComponent();

			//_backgroundColor = SKColors.Transparent;
			//_imageOffset = new SKPoint(0, 0);
			//_zoomedImageSize = SKSize.Empty;

			_movePosition = -1;
			_moveFirstRun = true;
			_moveDrawLine = false;

			_vm = new TraceWindowViewModel
			{
				Bitmap = null,
				Contour = null,
				TraceTool = TraceToolType.Hand,
				ImageLocked = true,
				TraceLocked = false,
				ZoomRatio = 1.0f,
				ZoomValues = new List<double>()
			};

			_vm.ZoomValues.Add(4);
			_vm.ZoomValues.Add(3);
			_vm.ZoomValues.Add(2);
			_vm.ZoomValues.Add(1);
			_vm.ZoomValues.Add(0.75);
			_vm.ZoomValues.Add(0.50);

			this.DataContext = _vm;
		}

		public TraceWindow(TraceWindowViewModel vm)
		{
			InitializeComponent();

			_movePosition = -1;
			_moveFirstRun = true;
			_moveDrawLine = false;

			_vm = vm;

			_vm.ZoomValues.Add(4);
			_vm.ZoomValues.Add(3);
			_vm.ZoomValues.Add(2);
			_vm.ZoomValues.Add(1);
			_vm.ZoomValues.Add(0.75);
			_vm.ZoomValues.Add(0.50);

			this.DataContext = _vm;
		}

		private void img_MouseWheel(object sender, MouseWheelEventArgs e)
		{
			//ImagingProvider.MouseWheel(imgCanvas, imgTransformGroup, e);

		}

		private void Img_MouseMove(object sender, MouseEventArgs e)
		{
			//imagingProviderIndexador.MouseMoveMagnifier(imgCanvas, imgObj, imgCanvasMagnifier, imgMagnifier, e);


		}

		private void Img_MouseDown(object sender, MouseButtonEventArgs e)
		{
			//imagingProviderIndexador.MouseDown(imgCanvas, imgObj, imgTranslateTransform, e);
		}

		private void Img_MouseUp(object sender, MouseButtonEventArgs e)
		{
			imagingProviderIndexador.MouseUp(TraceImage, e);
		}

		private void btnZoomIn_Click(object sender, RoutedEventArgs e)
		{
			ImagingProvider.ZoomIn(imgTransformGroup);
		}

		private void btnZoomOut_Click(object sender, RoutedEventArgs e)
		{
			ImagingProvider.ZoomOut(imgTransformGroup);
		}

		private void btnRotate_Click(object sender, RoutedEventArgs e)
		{
			ImagingProvider.Rotate(imgRotateTransform);

		}

		private void btnFTW_Click(object sender, RoutedEventArgs e)
		{
			//ImagingProvider.FitToContentMagnifier(imgObj, imgTransformGroup, imgCanvas, imgCanvasMagnifier, imgMagnifier);
		}

		private Darwin.Point MapWindowsPointToDarwinPoint(System.Windows.Point point)
		{
			// Clip to image bounds
			if (point.X < 0 || point.Y < 0)
				return Darwin.Point.Empty;

			if (_vm.Bitmap != null && (point.X > _vm.Bitmap.Width || point.Y > _vm.Bitmap.Height))
				return Darwin.Point.Empty;

			return new Darwin.Point((int)Math.Round(point.X), (int)Math.Round(point.Y));
		}

		private System.Windows.Point MapDarwinPointToWindowsPoint(Darwin.Point point)
		{
			return new System.Windows.Point(point.X, point.Y);
		}

		private void Button_Click(object sender, RoutedEventArgs e)
		{
			var openFile = new OpenFileDialog();

			// ShowDialog() returns a nullable bool, so we specifically need to test
			// whether it equals true
			if (openFile.ShowDialog() == true)
			{
				Trace.WriteLine(openFile.FileName);

				var img = System.Drawing.Image.FromFile(openFile.FileName);

				var bitmap = new Bitmap(img);
				// TODO: Hack for HiDPI -- this should be more intelligent.
				bitmap.SetResolution(96, 96);

				_vm.Bitmap = bitmap;

				//_vm.Bitmap = new Bitmap(img);
				////_image = _originalBitmap.ToSKImage();

				
				//_vm.Bitmap.SetResolution(96, 96);


				//this.skiaElement.InvalidateVisual();
				//ImagingProvider.LoadImage(_vm.Bitmap, TraceImage);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// traceReset: used by DeleteContourDialog object to prepare for
		//             redrawing the contour
		//
		private void TraceReset()
		{
			if (_vm.Contour != null)
				_vm.Contour = null;

			//***1.4 - new code to remove Outline, if loaded from fin file
			if (_vm.Outline != null)
				_vm.Outline = null;

			// TODO
			//***1.95 - new code to remove DatabaseFin, if loaded from fin file
			// or if created for a previous Save or Add To Database operation
			// from which we have returned to setp 1 (Modifiy the image)
			//
			/*if (NULL != mFin)
			{
				delete mFin;
				mFin = NULL;
			}*/

			// TODO
			_vm.TraceSnapped = false; //***051TW
			_vm.TraceLocked = false;

			_vm.TraceFinalized = false;
			//mNormScale = 1.0f; //***051TW
		}

		//*******************************************************************
		// Point clicked for AutoTrace -- 103AT SAH
		private void TraceAddAutoTracePoint(Darwin.Point p, bool shiftKeyDown) // AT103 SAH
		{
			if (p.IsEmpty)
				return;

			if (_vm.Contour == null)
				_vm.Contour = new Contour();

			_vm.Contour.AddPoint(p);

			if (_vm.Contour.NumPoints > 1)
			{
				//Check that the first point is further left than the second (otherwise we crash)
				//that is that [0].x < [1].x
				if (_vm.Contour[0].X >= _vm.Contour[1].X)
				{
					// TODO: Make this look like an error
					MessageBox.Show("Please click the start of the leading edge first and the end of the trailing edge second.\n\nNote: the dolphin must swim to your left.");

					//remove current marks
					TraceReset();
					return;
				}

				// Run trace
				Trace.WriteLine("PLEASE WAIT:\n   Automatically detecting rough fin outline ....");

				//***1.96 - figure out how to pass the image the user sees ONLY
				int left, top, right, bottom;
				GetViewedImageBoundsNonZoomed(out left, out top, out right, out bottom);

				//Perform intensity trace (SCOTT) (Aug/2005) Code: 101AT
				Contour trace = null;

				if (!shiftKeyDown)
					//trace = new IntensityContour(mNonZoomedImage,mContour); //101AT --Changed IntensityContour declaration to Contour
					trace = new IntensityContour(
						_vm.Bitmap,
						_vm.Contour,
						left, top, right, bottom); //***1.96 added window bounds 

				/* test trimAndReorder
				// reverse and trim off 20 points
				trace->trimAndReorder((*trace)[trace->length()-10],(*trace)[10]);
				printf("%d %d %d %d\n",(*trace)[0].x,(*trace)[0].y,
					(*trace)[trace->length()-1].x,(*trace)[trace->length()-1].y);
				// trim off 20 more points
				trace->trimAndReorder((*trace)[10],(*trace)[trace->length()-10]);
				printf("%d %d %d %d\n",(*trace)[0].x,(*trace)[0].y,
					(*trace)[trace->length()-1].x,(*trace)[trace->length()-1].y);
				// reverse again trim off 20 more points
				trace->trimAndReorder((*trace)[trace->length()-10],(*trace)[10]);
				printf("%d %d %d %d\n",(*trace)[0].x,(*trace)[0].y,
					(*trace)[trace->length()-1].x,(*trace)[trace->length()-1].y);
				*/

				if (!shiftKeyDown && trace != null && trace.NumPoints > 100)
				{
					//101AT -- changed min to 100 pt contour - JHS
					Trace.WriteLine("\n   Using edge detection and active contours to refine outline placement ....");
					_vm.Contour = trace;//101AT
					TraceSnapToFin(false, left, top, right, bottom);//101AT
																	//this.skiaElement.InvalidateVisual(); //101AT
				}
				else
				{//101AT
					Trace.WriteLine("\n   Trying Cyan Intensity AutoTrace ...");

					//102AT Add hooks for cyan intensity trace
					trace = new IntensityContourCyan(
						_vm.Bitmap,
						_vm.Contour,
						left, top, right, bottom); //***1.96 added window bounds 

					if (trace.NumPoints > 100)
					{//102AT
						Trace.WriteLine("Using edge detection and active contours to refine outline placement\n   (with cyan intensity image) ....\n");

						_vm.Contour = trace;//102AT
						TraceSnapToFin(true, left, top, right, bottom);//102AT
																	   //this.skiaElement.InvalidateVisual(); //102AT
					}
					else
					{
						// TODO: Make this look more like an error message
						MessageBox.Show("Auto trace could not determine the fin outline.\n\nPlease trace outline by hand.");

						//remove contour
						TraceReset();

						// TODO
						//select pencil
						_vm.TraceTool = TraceToolType.Pencil;
						StatusBarMessage.Text = "Please hand trace the fin outline above";

						return;
					}//102AT

				}

				// Successful trace
				_vm.TraceTool = TraceToolType.Eraser;

				StatusBarMessage.Text = "Make any Needed Corrections to Outline Trace using Tools Above";

			} //102AT
		}

		private void ZoomIn(System.Windows.Point point)
		{
			lastMousePositionOnTarget = point;

			ZoomSlider.Value += 1;
		}

		private void ZoomOut(System.Windows.Point point)
		{
			lastMousePositionOnTarget = point;

			ZoomSlider.Value -= 1;
		}

		// TODO: Do we really need this, or is this not ported correctly?
		private void GetViewedImageBoundsNonZoomed(out int left, out int top, out int right, out int bottom)
		{
			int
				width = _vm.Bitmap.Width,
				height = _vm.Bitmap.Height;

			left = 0;
			top = 0;
			right = left + width - 1; //SAH--BUG FIX , to ;
			bottom = top + height - 1;

			//GtkAdjustment
			//	* adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(mScrolledWindow));

			//if (adj->page_size < height)
			//{
			//	height = adj->page_size;
			//	top = adj->value;
			//	if (top < 0)
			//		top = 0;
			//	bottom = top + height - 1;
			//	if (bottom >= mImage->getNumRows())
			//		bottom = mImage->getNumRows() - 1;
			//}

			//adj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(mScrolledWindow));

			//if (adj->page_size < width)
			//{
			//	width = adj->page_size;
			//	left = adj->value;
			//	if (left < 0)
			//		left = 0;
			//	right = left + width - 1;
			//	if (right >= mImage->getNumCols())
			//		right = mImage->getNumCols() - 1;
			//}

			//// since we are in a scrollable window, the offsets are 0,0 and the
			//// drawable width & height are the same as the image rows & cols

			//this->zoomMapPointsToOriginal(left, top);
			//this->zoomMapPointsToOriginal(right, bottom);
		}

		private void TraceAddNormalPoint(Darwin.Point point)
		{
			if (point.IsEmpty)
				return;

			if (_vm.Contour == null)
				_vm.Contour = new Contour();

			_vm.Contour.AddPoint(point.X, point.Y);
		}

		//***1.96 - modified with virtual cropping bounds
		// 1.1 - new multiscale version
		/////////////////////////////////////////////////////////////////////
		// traceSnapToFin: called to perform the active contour based fit of
		//    trace to fin outline and remove knots.  This is called after
		//    the initial trace but before user has oportunity to clean
		//    up the trace, add/remove points, etc
		//
		//  Modified for multiscale active contour processing 01/23/06 krd
		private void TraceSnapToFin(bool useCyan, int left, int top, int right, int bottom)
		{
			if (_vm.Contour == null || _vm.TraceLocked) //***006FC
				return;

			float[] energyWeights = new float[]{
				AppSettings.SnakeEnergyContinuity,
				AppSettings.SnakeEnergyLinearity,
				AppSettings.SnakeEnergyEdge
			};

			// full size and currently viewed scale edgeMagImage
			Bitmap EdgeMagImage;
			Bitmap smallEdgeMagImage;
			Bitmap temp;

			if (useCyan)
				temp = _vm.Bitmap.ToCyanIntensity(); //***1.96 - copy to cyan
			else
				temp = _vm.Bitmap.ToGrayscale(); //***1.96 - copy to grayscale

			Bitmap temp2 = BitmapHelper.CropBitmap(temp, left, top, right, bottom); //***1.96 - then crop

			Bitmap EdgeImage = EdgeDetection.CannyEdgeDetection(
					//temp,
					temp2, //***1.96
					out EdgeMagImage,
					true,
					AppSettings.GaussianStdDev,
					AppSettings.CannyLowThreshold,
					AppSettings.CannyHighThreshold);


			//***1.96 - copy edgeMagImage into area of entire temp image bounded by 
			//          left, top, right, bottom
			for (int r = 0; r < EdgeMagImage.Height; r++)
			{
				for (int c = 0; c < EdgeMagImage.Width; c++)
				{
					// set up area within *temp as the real EdgeMagImage
					temp.SetPixel(left + c, top + r, EdgeMagImage.GetPixel(c, r));
				}
			}

			EdgeMagImage = temp; //***1.96

			//EdgeMagImage->save("EdgeMagImg.png");

			// create initial evenly spaced contour scaled to zoomed image
			double spacing = 3; //***005CM declaration moved ouside if() below
								// contour is initially spaced with larger of 1) space for 200 points or
								// 2) at three pixels (200 points limits hi res pics from having many points)

			//scale mContour to current scale of analysis (image) first time thru
			Contour smallContour = null; // current scale contour
			if (_vm.Contour.NumPoints > 2)
			{
				spacing = _vm.Contour.GetTotalDistanceAlongContour() / 200.0;
				if (spacing < SpaceBetweenPoints)
					spacing = SpaceBetweenPoints;

				Contour evenContour = _vm.Contour.EvenlySpaceContourPoints(spacing);
				smallContour = evenContour.CreateScaledContour(_zoomRatioOld / 100.0f, 0, 0); // small sized countour
			}

			int ratio = _zoomRatioOld;
			int chunkSize;  // used to divide up iterations among the scales
			if (ratio <= 100)
				chunkSize = (int)(AppSettings.SnakeMaximumIterations / (100.0 / ratio * 2 - 1));
			else
				chunkSize = AppSettings.SnakeMaximumIterations;
			int tripNum = 1;  // one trip at each scale

			// at each ZoomRatio, create an EdgeMagImage and a contour at that scale
			// and process with active contour
			while (ratio <= 100 || tripNum == 1)
			{ // if ratio > 100 take at least one trip
				int iterations = (int)(Math.Pow(2.0, tripNum - 1) * chunkSize);

				// resize EdgeMagImage to current scale
				if (ratio != 100)
					smallEdgeMagImage = BitmapHelper.ResizePercentageNearestNeighboar(EdgeMagImage, ratio);
				else
					smallEdgeMagImage = EdgeMagImage;

				for (int i = 0; i < iterations; i++)
				{
					// TODO: Hardcoded neighborhood of 3
					Snake.MoveContour(ref smallContour, smallEdgeMagImage, 3, energyWeights);
					if (i % 5 == 0)
					{
						// scale repositioned contour to viewed scale for display (mContour)
						_vm.Contour = smallContour.CreateScaledContour(100.0f / ratio, 0, 0);

						// TODO
						// this->refreshImage();

						//TODO
						//skiaElement.InvalidateVisual();
					}
				} // end for (i=0; i< iterations)

				// scale repositioned contour to next larger scale for more repositioning
				ratio *= 2;  // double ratio for next level		
				if (ratio <= 100)
				{
					smallContour = smallContour.CreateScaledContour(2.0f, 0, 0);
				}
				tripNum++;

			} // end while  

			// features such as glare spots may cause outline points to bunch and wrap
			// during active contour process
			_vm.Contour.RemoveKnots(spacing); //***005CM
		}

		// Point added to middle of contour after trace has been finalized
		private void TraceAddExtraPoint(Darwin.Point point)
		{
			if (_vm.Contour == null || point.IsEmpty)
				return;

			AddContourUndo(_vm.Contour);

			// use MovePoint code to display newly added point
			_moveInit = true;
			_movePosition = _vm.Contour.AddPointInOrder(point.X, point.Y);
		}

		private void TraceChopOutline(Darwin.Point point)
		{
			if (_vm.Contour == null)
			{
				_movePosition = -1;
				return;
			}

			AddContourUndo(_vm.Contour);

			_chopPosition = _vm.Contour.FindPositionOfClosestPoint(point.X, point.Y);

			if (_chopPosition >= 0)
			{
				int start, stop;

				if ((_vm.Contour.NumPoints - _chopPosition) > _chopPosition)
				{
					_chopLead = 1;
					start = 0;
					stop = _chopPosition;
				}
				else
				{
					_chopLead = 0;
					start = _chopPosition;
					stop = _vm.Contour.NumPoints;
				}

				for (var i = start; i < stop; i++)
					_vm.Contour[i].Type = PointType.Chopping;
			}
        }

		private void TraceChopOutlineUpdate(Darwin.Point point)
		{
			if (_vm.Contour == null)
			{
				_movePosition = -1;
				return;
			}

			var newChopPosition = _vm.Contour.FindPositionOfClosestPoint(point.X, point.Y);

			if (newChopPosition >= 0)
			{
				int start, stop;

				if ((_vm.Contour.NumPoints - newChopPosition) > newChopPosition)
				{
					_chopLead = 1;
					start = 0;
					stop = newChopPosition;
				}
				else
				{
					_chopLead = 0;
					start = newChopPosition;
					stop = _vm.Contour.NumPoints;
				}

				for (var i = 0; i < _vm.Contour.NumPoints; i++)
				{
					if (i >= start && i < stop)
					{
						_vm.Contour[i].Type = PointType.Chopping;
					}
					else if (_vm.Contour[i].Type == PointType.Chopping)
                    {
						_vm.Contour[i].Type = PointType.Normal;
                    }
				}
				_chopPosition = newChopPosition;
			}
		}

		private void TraceChopOutlineFinal()
		{
			if (_vm.Contour == null || -1 == _chopPosition)
				return;

			int start, stop;

			if (_chopLead > 0)
			{
				start = 0;
				stop = _chopPosition;
			}
			else
			{
				start = _chopPosition;
				stop = _vm.Contour.Length;
			}

			for (var i = start; i < stop; i++)
			{
				Trace.WriteLine("removing " + i);
				_vm.Contour.RemovePoint(start);
			}
		}

		private void TraceErasePoint(Darwin.Point point)
		{
			if (_vm.Contour == null || point.IsEmpty || !_eraseInit)
				return;

			int
				numRows = _vm.Bitmap.Height,
				numCols = _vm.Bitmap.Width;

			int
				row_start, col_start,
				row_end, col_end;

			//offset = (int)Math.Round(EraserBrushSize / _vm.ZoomRatio / (float)2);  // krd 10/28/05

			row_start = point.Y - EraserBrushSize;
			col_start = point.X - EraserBrushSize;

			if (row_start < 0)
				row_start = 0;

			if (col_start < 0)
				col_start = 0;

			row_end = point.Y + EraserBrushSize;
			col_end = point.X + EraserBrushSize;

			if (row_end >= numRows)
				row_end = numRows - 1;

			if (col_end >= numCols)
				col_end = numCols - 1;

			for (int r = row_start; r <= row_end; r++)
				for (int c = col_start; c <= col_end; c++)
					_vm.Contour.RemovePoint(c, r);

			// 1.4 - remove empty Contour so other functions do not have to check for same
			if (_vm.Contour.Length == 0)
				_vm.Contour = null;
		}

		private void TraceMovePointInit(Darwin.Point point)
		{
			if (_vm.Contour == null)
			{
				_movePosition = -1;
				return;
			}

			_moveInit = true;

			AddContourUndo(_vm.Contour);

			_movePosition = _vm.Contour.FindPositionOfClosestPoint(point.X, point.Y);

			_vm.Contour[_movePosition].Type = PointType.Moving;
		}

		private void TraceMovePointUpdate(Darwin.Point point)
		{
			if (_vm.Contour == null || -1 == _movePosition || point.IsEmpty)
				return;

			_vm.Contour[_movePosition].SetPosition(point.X, point.Y);
		}

		private void TraceMovePointFinalize(Darwin.Point point)
		{
			if (_vm.Contour == null || -1 == _movePosition)
				return;
			
			if (point.IsEmpty)
			{
				// Leave the position of this point whatever it was before we went out of bounds
				_vm.Contour[_movePosition].Type = PointType.Normal;
			}
			else
			{
				_vm.Contour[_movePosition] = new Darwin.Point
				{
					X = point.X,
					Y = point.Y,
					Type = PointType.Normal
				};
			}

			_movePosition = -1;
		}

		//*******************************************************************
		//
		// void TraceWindow::traceMoveFeaturePointInit(int x, int y)
		//
		//    Finds identity of closest feature point and initiates movement
		//    of same.
		//
		private void TraceMoveFeaturePointInit(System.Windows.Point point)
		{
			// Bail out if feature points have not already been identified
			if (_vm.Contour == null || _vm.Outline == null || !_vm.TraceFinalized)
				return;

			_moveInit = true;

			_moveFeature = (FeaturePointType)_vm.Outline.FindClosestFeaturePoint(new PointF(point.X, point.Y));

			if (FeaturePointType.NoFeature == _moveFeature)
				return;

			//***054 - indicate which point is being moved

			_previousStatusBarMessage = StatusBarMessage.Text; // now we have a copy of the label string

			switch (_moveFeature)
			{
				case FeaturePointType.Tip:
					StatusBarMessage.Text = "Moving TIP -- Drag into position and release mouse button.";
					break;
				case FeaturePointType.Notch:
					StatusBarMessage.Text = "Moving NOTCH -- Drag into position and release mouse button.";
					break;
				case FeaturePointType.PointOfInflection:
					StatusBarMessage.Text = "Moving END OF OUTLINE -- Drag into position and release mouse button.";
					break;
				case FeaturePointType.LeadingEdgeBegin:
					StatusBarMessage.Text = "Moving BEGINNING OF OUTLINE -- Drag into position and release mouse button.";
					break;
				case FeaturePointType.LeadingEdgeEnd:
					//***1.8 - no longer show or move LE_END -- force it to TIP
					_moveFeature = FeaturePointType.Tip;
					//TODO
					//gtk_label_set_text(GTK_LABEL(mStatusLabel),
					//		_("Moving TIP -- Drag into position and release mouse button."));
					break;
				default:
					StatusBarMessage.Text = "Cannot determine selected feature. Try Again!";
					break;
			}

			_movePosition = _vm.Outline.GetFeaturePoint(_moveFeature);

			//p.x = (int)Math.Round(_vm.Contour[_movePosition].X / mNormScale);
			//p.y = (int)Math.Round(_vm.Contour[_movePosition].Y / mNormScale);

			TraceMoveFeaturePointDisplay(point.ToDarwinPoint());
		}


		//*******************************************************************
		//
		// void TraceWindow::traceMoveFeaturePointFinalize(int x, int y)
		//
		//
		//
		private void TraceMoveFeaturePointFinalize(Darwin.Point point)
		{
			if (_vm.Contour == null || (int)FeaturePointType.NoFeature == _movePosition)
				return;

			// TODO: Shouldn't this do something with point on mouse up in case it's different than move?

			// set new location of feature
			_vm.Outline.SetFeaturePoint(_moveFeature, _movePosition);

			// reset message label
			StatusBarMessage.Text = _previousStatusBarMessage;
		}


		//*******************************************************************
		//
		// void TraceWindow::traceMoveFeaturePointUpdate(int x, int y)
		//
		//
		//
		private void TraceMoveFeaturePointUpdate(Darwin.Point point)
		{
			if (_vm.Contour == null || FeaturePointType.NoFeature == _moveFeature)
				return;

			if (point.IsEmpty)
				return;

			//zoomMapPointsToOriginal(x, y);
			//ensurePointInBounds(x, y);

			//x = (int)Math.Round(x * mNormScale);
			//y = (int)Math.Round(y * mNormScale);

			// find location of closest contour point

			int posit = _vm.Contour.FindPositionOfClosestPoint(point.X, point.Y);

			if (posit < -1)
				return;

			// enforce constraints on feature movement so their order cannot be changed

			switch (_moveFeature)
			{
				case FeaturePointType.LeadingEdgeBegin:
					if ((0 < posit) && (posit < _vm.Outline.GetFeaturePoint(_moveFeature + 1)))
						_movePosition = posit;
					break;
				//***1.8 - moving of LE_END no longer supported
				//case LE_END :
				//***1.8 - moving of TIP now constrained by LE_BEGIN and NOTCH
				case FeaturePointType.Tip:
					if ((_vm.Outline.GetFeaturePoint(FeaturePointType.LeadingEdgeBegin) < posit) &&
						(posit < _vm.Outline.GetFeaturePoint(_moveFeature + 1)))
						_movePosition = posit;
					break;
				case FeaturePointType.Notch:
					if ((_vm.Outline.GetFeaturePoint(_moveFeature - 1) < posit) &&
						(posit < _vm.Outline.GetFeaturePoint(_moveFeature + 1)))
						_movePosition = posit;
					break;
				case FeaturePointType.PointOfInflection:
					if ((_vm.Outline.GetFeaturePoint(_moveFeature - 1) < posit) &&
						(posit < (_vm.Outline.Length - 1)))
						_movePosition = posit;
					break;
			}

			//x = (int)Math.Round(_vm.Contour[_movePosition].X / mNormScale);
			//y = (int)Math.Round(_vm.Contour[_movePosition].Y / mNormScale);

			TraceMoveFeaturePointDisplay(point);
		}

		//*******************************************************************
		//
		// void TraceWindow::traceMoveFeaturePointDisplay(int xC, int yC)
		//
		//    Handles display of the feature point at each point as it is being
		//    dragged to new location.
		//
		private void TraceMoveFeaturePointDisplay(Darwin.Point point)
		{
			if (_vm.Contour == null || FeaturePointType.NoFeature == _moveFeature)
				return;

			int contourLength = _vm.Contour.Length;

			//int
			//	xLeft, xRight, yTop, yBot;

			//static int
			//	lastXLeft, lastXRight, lastYTop, lastYBot;

			//int highlightPointSize = 4 * _vm.ZoomPointSize;

			//if (mZoomRatio != 100)
			//	highlightPointSize = 4 * zoomPointSize();

			//int halfHighPtSize = highlightPointSize / 2;

			//zoomMapPointsToZoomed(xC, yC);

			//xLeft = xC - halfHighPtSize;
			//xRight = xC + halfHighPtSize;
			//yBot = yC + halfHighPtSize;
			//yTop = yC - halfHighPtSize;

			/*
			// 055TW - removed code to prevent artifacts in images with
			// mZoomRatio less than 100% -- these bounds checks are not
			// needed ???? -- in any case they must be done in zoomed image
			// coordinates
			if (xLeft < 0)
				xLeft = 0;
			if (yTop < 0)
				yTop = 0;
			if (xRight >= (int) mNonZoomedImage->getNumCols())
				xRight = mNonZoomedImage->getNumCols() - 1;
			if (yBot >= (int) mNonZoomedImage->getNumRows())
				yBot = mNonZoomedImage->getNumRows() - 1;

			zoomMapPointsToZoomed(xLeft, yTop);
			zoomMapPointsToZoomed(xRight, yBot);
			*/

			// save bounds of this box for next round, even though we may widen
			// the box a bit below to cover last box and this one
			//	int
			//		saveXLeft(xLeft), saveXRight(xRight), 
			//saveYTop(yTop), saveYBot(yBot);

			//	if (mMoveInit)
			//	{
			//		mMoveInit = false;
			//	}
			//	else
			//	{
			//		if (lastXLeft < xLeft)
			//			xLeft = lastXLeft;

			//		if (lastXRight > xRight)
			//			xRight = lastXRight;

			//		if (lastYTop < yTop)
			//			yTop = lastYTop;

			//		if (lastYBot > yBot)
			//			yBot = lastYBot;
			//	}

			//	lastXLeft = saveXLeft;
			//	lastXRight = saveXRight;
			//	lastYTop = saveYTop;
			//	lastYBot = saveYBot;

			//	// redraw image within tiny window 

			//	gdk_draw_rgb_image(
			//		mDrawingArea->window,
			//		mDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
			//		xLeft + mZoomXOffset, yTop + mZoomYOffset,
			//		xRight - xLeft,
			//		yBot - yTop,
			//		GDK_RGB_DITHER_NONE,
			//		(guchar*)(mImage->getData() + yTop * mImage->getNumCols() + xLeft),
			//		mImage->getNumCols() * mImage->bytesPerPixel());

			//	// redraw portion of contour within tiny window

			//	int zoomedPointSize = zoomPointSize();

			//	unsigned numPoints = mContour->length();

			//	float nscale = 1.0 / mNormScale;

			//	for (unsigned i = 0; i < numPoints; i++)
			//	{
			//		int zoomedX = nscale * (*mContour)[i].x;
			//		int zoomedY = nscale * (*mContour)[i].y;

			//		zoomMapPointsToZoomed(zoomedX, zoomedY);

			//		if ((zoomedX >= xLeft) && (zoomedX <= xRight) && (zoomedY >= yTop) && (zoomedY <= yBot))
			//			gdk_draw_rectangle(
			//				mDrawingArea->window,
			//				mGC,
			//				TRUE,
			//				zoomedX - zoomedPointSize / 2 + mZoomXOffset,
			//				zoomedY - zoomedPointSize / 2 + mZoomYOffset,
			//				zoomedPointSize,
			//				zoomedPointSize);
			//	}

			//	// redraw two adjacent features if they are inside the box

			//	if ((LE_BEGIN < _moveFeature) && (_moveFeature - 1 != FeaturePointType.LeadingEdgeBegin)) //***1.8 not LE_END
			//	{
			//		point_t p = mOutline->getFeaturePointCoords(mMoveFeature - 1);
			//		int xC = (int)round(nscale * p.x);
			//		int yC = (int)round(nscale * p.y);
			//		if (mZoomRatio != 100)
			//			zoomMapPointsToZoomed(xC, yC);
			//		gdk_draw_rectangle(
			//			mDrawingArea->window,
			//			mGC,
			//			TRUE,
			//			xC - halfHighPtSize + mZoomXOffset,
			//			yC - halfHighPtSize + mZoomYOffset,
			//			highlightPointSize,
			//			highlightPointSize);
			//	}

			//	if ((_moveFeature < FeaturePointType.PointOfInflection) && (_moveFeature + 1 != FeaturePointType.LeadingEdgeEnd)) //***1.8 not LE_END
			//	{
			//		Model.Point p = _vm.Outline.GetFeaturePointCoords(_moveFeature + 1);
			//		int xC = (int)Math.Round(nscale * p.x);
			//		int yC = (int)Math.Round(nscale * p.y);
			//		if (mZoomRatio != 100)
			//			zoomMapPointsToZoomed(xC, yC);
			//		gdk_draw_rectangle(
			//			mDrawingArea->window,
			//			mGC,
			//			TRUE,
			//			xC - halfHighPtSize + mZoomXOffset,
			//			yC - halfHighPtSize + mZoomYOffset,
			//			highlightPointSize,
			//			highlightPointSize);
			//	}

			//	// Lastly, draw feature point in current (dragged to) location

			//	gdk_draw_rectangle(
			//		mDrawingArea->window,
			//		mMovingGC,
			//		TRUE,
			//		xC - halfHighPtSize + mZoomXOffset,
			//		yC - halfHighPtSize + mZoomYOffset,
			//		highlightPointSize,
			//		highlightPointSize);
		}

		private void CropApply(Object sender, RoutedEventArgs args)
        {
			int x = (int)Math.Round(_cropSelector.SelectRect.X);
			int y = (int)Math.Round(_cropSelector.SelectRect.Y);
			int width = (int)Math.Round(_cropSelector.SelectRect.Width);
			int height = (int)Math.Round(_cropSelector.SelectRect.Height);

			AddImageUndo(ImageModType.IMG_crop,
				x, y,
				x + width, y + height);

			System.Drawing.Rectangle cropRect = new System.Drawing.Rectangle(
				x,
				y,
				width,
				height);

			var croppedBitmap = ImageTransform.CropBitmap(_vm.Bitmap, cropRect);

			_vm.Bitmap = _vm.BaseBitmap = croppedBitmap;
		}

		private void RotateInit(Darwin.Point point)
		{
			//mRotateXCenter = mImage->getNumCols() / 2;
			//mRotateYCenter = mImage->getNumRows() / 2;

			//mRotateXStart = x;
			//mRotateYStart = mImage->getNumRows() - y;

			//mRotateStartAngle = atan2(
			//		(double)(mRotateYStart - mRotateYCenter),
			//		(double)(mRotateXStart - mRotateXCenter));

			//mRotateOriginalImage = new ColorImage(mImage);
		}

		private void RotateUpdate(Darwin.Point point)
		{
			//float angle = atan2(
			//		(double)(mImage->getNumRows() - y - mRotateYCenter),
			//		(double)(x - mRotateXCenter))

			//		- mRotateStartAngle;

			//delete mImage;
			//mImage = rotateNN(mRotateOriginalImage, angle);

			//refreshImage();
		}

		private void RotateFinal(System.Windows.Point point)
		{
			//float angle = atan2(
			//		(double)(mImage->getNumRows() - y - mRotateYCenter),
			//		(double)(x - mRotateXCenter))

			//		- mRotateStartAngle;

			//delete mRotateOriginalImage;

			//addUndo(mNonZoomedImage);
			//ColorImage* temp = mNonZoomedImage;
			//mNonZoomedImage = rotate(mNonZoomedImage, angle);
			//delete temp;
			//zoomUpdate(false);
		}


		private void TraceTool_Checked(object sender, RoutedEventArgs e)
		{
			if (TraceCanvas != null)
			{
				// Set the appropriate cursor
				// TODO: Maybe this could be done better with databinding
				switch (_vm.TraceTool)
				{
					case TraceToolType.AddPoint:
					case TraceToolType.AutoTrace:
						TraceCanvas.Cursor = Resources["AutoTraceCursor"] as Cursor;
						break;

					case TraceToolType.Crop:
						//TraceCanvas.Cursor = Cursors.SizeNWSE;
						var layer = AdornerLayer.GetAdornerLayer(TraceCanvas);
						_cropSelector = new CroppingAdorner(TraceCanvas);
						_cropSelector.Crop += CropApply;
						layer.Add(_cropSelector);
						break;

					case TraceToolType.ChopOutline:
						TraceCanvas.Cursor = Resources["ChopOutlineCursor"] as Cursor;
						break;

					case TraceToolType.Eraser:
						TraceCanvas.Cursor = Resources["EraserCursor"] as Cursor;
						break;

					case TraceToolType.Hand:
						TraceCanvas.Cursor = Cursors.Hand;
						break;

					case TraceToolType.Magnify:
						TraceCanvas.Cursor = Resources["MagnifyCursor"] as Cursor;
						break;

					case TraceToolType.MovePoint:
						TraceCanvas.Cursor = Cursors.Hand;
						break;

					case TraceToolType.Pencil:
						TraceCanvas.Cursor = Resources["PencilCursor"] as Cursor;
						break;

					default:
						TraceCanvas.Cursor = Cursors.Arrow;
						break;
				}
			}
		}

		private void TraceScrollViewer_ScrollChanged(object sender, ScrollChangedEventArgs e)
		{
			if (e.ExtentHeightChange != 0 || e.ExtentWidthChange != 0)
			{
				System.Windows.Point? targetBefore = null;
				System.Windows.Point? targetNow = null;

				if (!lastMousePositionOnTarget.HasValue)
				{
					if (lastCenterPositionOnTarget.HasValue)
					{
						var centerOfViewport = new System.Windows.Point(TraceScrollViewer.ViewportWidth / 2, TraceScrollViewer.ViewportHeight / 2);
						System.Windows.Point centerOfTargetNow = TraceScrollViewer.TranslatePoint(centerOfViewport, imgViewBox);

						targetBefore = lastCenterPositionOnTarget;
						targetNow = centerOfTargetNow;
					}
				}
				else
				{
					targetBefore = lastMousePositionOnTarget;
					targetNow = Mouse.GetPosition(imgViewBox);

					lastMousePositionOnTarget = null;
				}

				if (targetBefore.HasValue)
				{
					double dXInTargetPixels = targetNow.Value.X - targetBefore.Value.X;
					double dYInTargetPixels = targetNow.Value.Y - targetBefore.Value.Y;

					double multiplicatorX = e.ExtentWidth / imgViewBox.Width;
					double multiplicatorY = e.ExtentHeight / imgViewBox.Height;

					double newOffsetX = TraceScrollViewer.HorizontalOffset - dXInTargetPixels * multiplicatorX;
					double newOffsetY = TraceScrollViewer.VerticalOffset - dYInTargetPixels * multiplicatorY;

					if (double.IsNaN(newOffsetX) || double.IsNaN(newOffsetY))
					{
						return;
					}

					TraceScrollViewer.ScrollToHorizontalOffset(newOffsetX);
					TraceScrollViewer.ScrollToVerticalOffset(newOffsetY);
				}
			}
		}

		private void TraceScrollViewer_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
		{
			switch (_vm.TraceTool)
			{
				case TraceToolType.Hand:
					TraceScrollViewer.Cursor = Cursors.Arrow;
					TraceScrollViewer.ReleaseMouseCapture();
					lastDragPoint = null;
					e.Handled = true;
					break;
			}
		}

		private void TraceScrollViewer_MouseMove(object sender, MouseEventArgs e)
		{
			if (_vm.TraceTool == TraceToolType.Hand)
			{
				if (e.LeftButton == MouseButtonState.Pressed && lastDragPoint.HasValue)
				{
					System.Windows.Point posNow = e.GetPosition(TraceScrollViewer);

					double dX = posNow.X - lastDragPoint.Value.X;
					double dY = posNow.Y - lastDragPoint.Value.Y;

					lastDragPoint = posNow;

					TraceScrollViewer.ScrollToHorizontalOffset(TraceScrollViewer.HorizontalOffset - dX);
					TraceScrollViewer.ScrollToVerticalOffset(TraceScrollViewer.VerticalOffset - dY);
				}
			}
		}

		private void TraceScrollViewer_PreviewMouseWheel(object sender, MouseWheelEventArgs e)
		{
			lastMousePositionOnTarget = Mouse.GetPosition(imgViewBox);

			if (e.Delta > 0)
			{
				ZoomSlider.Value += 1;
			}
			if (e.Delta < 0)
			{
				ZoomSlider.Value -= 1;
			}

			e.Handled = true;
		}

		private void TraceScrollViewer_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
		{
			var clickedPoint = e.GetPosition(TraceScrollViewer);

			bool shiftKeyDown = false;

			if (Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift))
				shiftKeyDown = true;

			if (clickedPoint.X <= TraceScrollViewer.ViewportWidth && clickedPoint.Y < TraceScrollViewer.ViewportHeight) //make sure we still can use the scrollbars
			{
				switch (_vm.TraceTool)
				{
					case TraceToolType.Hand:
						TraceScrollViewer.Cursor = Cursors.SizeAll;
						lastDragPoint = clickedPoint;
						Mouse.Capture(TraceScrollViewer);
						e.Handled = true;
						break;

					case TraceToolType.Magnify:
						if (shiftKeyDown)
							ZoomOut(clickedPoint);
						else
							ZoomIn(clickedPoint);

						e.Handled = true;
						break;
				}
			}
		}

		private void ZoomSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
		{
			if (TraceImage != null && TraceImage.Source != null && ScaleTransform != null)
			{
				ScaleTransform.ScaleX = e.NewValue;
				ScaleTransform.ScaleY = e.NewValue;

				var centerOfViewport = new System.Windows.Point(TraceScrollViewer.ViewportWidth / 2, TraceScrollViewer.ViewportHeight / 2);
				lastCenterPositionOnTarget = TraceScrollViewer.TranslatePoint(centerOfViewport, imgViewBox);
			}
		}

		private void ShowOutlineDeleteDialog()
        {
			var result = MessageBox.Show("Delete the current outline of the fin?", "Delete Confirmation", MessageBoxButton.YesNo);

			if (result == MessageBoxResult.Yes)
				TraceReset();
		}

		private void TraceCanvas_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
		{
			var clickedPoint = e.GetPosition(this.TraceCanvas);
			var bitmapPoint = MapWindowsPointToDarwinPoint(clickedPoint);

			bool shiftKeyDown = false;

			if (Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift))
				shiftKeyDown = true;

			switch (_vm.TraceTool)
			{
				case TraceToolType.AutoTrace:
					if (!_vm.TraceLocked)
					{
						if (_vm.Contour != null && _vm.Contour.NumPoints >= 2) //If the it has 0 or 1 points, it's ok to use.
							ShowOutlineDeleteDialog();
						else
							TraceAddAutoTracePoint(bitmapPoint, shiftKeyDown); //103AT SAH

						e.Handled = true;
					}
					break;

				case TraceToolType.Pencil:
					if (_vm.ImageLocked && !_vm.TraceLocked)
					{ //***051TW
						if (_vm.Contour != null && _vm.Contour.Length > 0) //***1.4TW - empty is OK to reuse
							ShowOutlineDeleteDialog();
						else
							TraceAddNormalPoint(bitmapPoint);
					}
					e.Handled = true;
					break;

				case TraceToolType.AddPoint:
					Mouse.Capture(TraceCanvas);
					TraceAddExtraPoint(bitmapPoint);
					e.Handled = true;
					break;

				case TraceToolType.Eraser:
					Mouse.Capture(TraceCanvas);
					AddContourUndo(_vm.Contour);
					_eraseInit = true;
					break;

				case TraceToolType.MovePoint:
					Mouse.Capture(TraceCanvas);
					TraceMovePointInit(bitmapPoint);
					e.Handled = true;
					break;

				case TraceToolType.MoveFeature: //***006PM new case to move Notch
					TraceMoveFeaturePointInit(clickedPoint); //***051TW
					e.Handled = true;
					break;

				case TraceToolType.Crop:
					_cropSelector.CaptureMouse();
					_cropSelector.StartSelection(clickedPoint);

					e.Handled = true;
					break;

				case TraceToolType.ChopOutline: //** 1.5 krd - chop outline to end of trace 
					TraceChopOutline(bitmapPoint);
					e.Handled = true;
					break;

				case TraceToolType.Rotate:
					RotateInit(bitmapPoint);
					e.Handled = true;
					break;
			}
		}

		private void TraceCanvas_MouseMove(object sender, MouseEventArgs e)
		{
			var clickedPoint = e.GetPosition(this.TraceCanvas);
			var imagePoint = MapWindowsPointToDarwinPoint(clickedPoint);

			if (imagePoint.IsEmpty)
				CursorPositionMessage.Text = string.Empty;
			else
				CursorPositionMessage.Text = string.Format("{0}, {1}", imagePoint.X, imagePoint.Y);

			//bool shiftKeyDown = false;

			//if (Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift))
			//	shiftKeyDown = true;

			if (e.LeftButton == MouseButtonState.Pressed)
			{
				Trace.WriteLine(clickedPoint);

				switch (_vm.TraceTool)
				{
					case TraceToolType.Pencil:
						TraceAddNormalPoint(imagePoint);
						e.Handled = true;
						break;

					case TraceToolType.Eraser:
						TraceErasePoint(imagePoint);
						e.Handled = true;
						break;

					case TraceToolType.AddPoint:   // krd - treat add point as move point
					case TraceToolType.MovePoint:
						TraceMovePointUpdate(imagePoint);
						e.Handled = true;
						break;

					case TraceToolType.MoveFeature: //***006PM new case to move Notch
						TraceMoveFeaturePointUpdate(imagePoint); //***051TW
						e.Handled = true;
						break;

					//case TraceToolType.Crop:
					//	CropUpdate(imagePoint);
					//	e.Handled = true;
					//	break;

					case TraceToolType.ChopOutline:
						TraceChopOutlineUpdate(imagePoint);
						e.Handled = true;
						break;

					case TraceToolType.Rotate:
						RotateUpdate(imagePoint);
						e.Handled = true;
						break;
				}
			}
		}

		private void TraceCanvas_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
		{
			Trace.WriteLine("left mouse button up");

			var clickedPoint = e.GetPosition(this.TraceCanvas);
			var bitmapPoint = MapWindowsPointToDarwinPoint(clickedPoint);

			//bool shiftKeyDown = false;

			//if (Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift))
			//	shiftKeyDown = true;

			switch (_vm.TraceTool)
			{
				case TraceToolType.Pencil:
					if (!_vm.TraceSnapped)
					{ //***051TW
					  // it is possible for user to trace completely OUTSIDE of the image, so Contour is never created
						if (null == _vm.Contour) //***1.982 - fail quietly
							return;

						//***1.4TW - shorter contours cannot be procesed without errors in following 
						// processes, so reset trace and force retrace
						if (_vm.Contour.Length < 100)
						{
							TraceReset();
							MessageBox.Show("This outline trace is too short to\nprocess further.  Please retrace.");
						}
						else
						{
							int left, top, right, bottom; //***1.96
							GetViewedImageBoundsNonZoomed(out left, out top, out right, out bottom); //***1.96
							TraceSnapToFin(false, left, top, right, bottom); //***006FC,***1.96
						}
					}
					e.Handled = true;
					break;

				case TraceToolType.AddPoint:   // krd - treat add point as move point, erase lines
				case TraceToolType.MovePoint:
					TraceCanvas.ReleaseMouseCapture();
					TraceMovePointFinalize(bitmapPoint);
					e.Handled = true;
					break;

				case TraceToolType.MoveFeature: //***006PM new case to move Notch
					TraceMoveFeaturePointFinalize(bitmapPoint); //***051TW
					e.Handled = true;
					break;

				//case TraceToolType.Crop:
				//	CropFinalize(bitmapPoint);
				//	e.Handled = true;
				//	break;

				case TraceToolType.ChopOutline: // *** 1.5 krd - chop outline to end of trace 
					TraceChopOutlineFinal();
					e.Handled = true;
					break;

				case TraceToolType.Eraser:
					TraceCanvas.ReleaseMouseCapture();
					_eraseInit = false;
					e.Handled = true;
					break;

				case TraceToolType.Rotate:
					RotateFinal(clickedPoint);
					e.Handled = true;
					break;
			}
		}

		private void ZoomComboBox_TextChanged(object sender, TextChangedEventArgs e)
		{
			string text = ZoomComboBox.Text;
			double num;
			if (double.TryParse(text, out num) && _vm.ZoomValues.Contains(num / 100))
			{
				//slider.Value = num / 100;
				Dispatcher.BeginInvoke(new Action(() =>
				{
					var textBox = ZoomComboBox.Template.FindName("PART_EditableTextBox", ZoomComboBox) as TextBox;
					if (textBox != null)
						textBox.CaretIndex = textBox.Text.Length - 1;
				}));
			}
		}

        private void FlipHorizontalButton_Click(object sender, RoutedEventArgs e)
        {
			//TODO
			//// ***1.75 - set location of center based on slider
			//if (traceWin->mScrolledWindow->allocation.height < traceWin->mImage->getNumRows())
			//{
			//	GtkAdjustment* adj = gtk_scrolled_window_get_vadjustment(
			//		GTK_SCROLLED_WINDOW(traceWin->mScrolledWindow));
			//	double half = 0.5 * traceWin->mScrolledWindow->allocation.height;
			//	traceWin->mImageCenterY = (adj->value + half) / (adj->upper - adj->lower);
			//}
			//if (traceWin->mScrolledWindow->allocation.width < traceWin->mImage->getNumCols())
			//{
			//	GtkAdjustment* adj = gtk_scrolled_window_get_hadjustment(
			//		GTK_SCROLLED_WINDOW(traceWin->mScrolledWindow));
			//	double half = 0.5 * traceWin->mScrolledWindow->allocation.width;
			//	traceWin->mImageCenterX = (adj->value + half) / (adj->upper - adj->lower);
			//}
			////***1.75 - flip desired / current center
			//traceWin->mImageCenterX = 1.0 - traceWin->mImageCenterX;

			if (_vm.Bitmap != null)
			{
				
				_vm.Bitmap.RotateFlip(RotateFlipType.RotateNoneFlipX);
				_vm.BaseBitmap = new Bitmap(_vm.Bitmap);
				_vm.UpdateImage();

				AddImageUndo(ImageModType.IMG_flip);
			}
		}

        private void BrightnessSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
			if (_vm?.Bitmap != null)
            {
				int val = Convert.ToInt32(BrightnessSlider.Value);
				// TODO: Not quite right -- we need more copies or better logic if other changes have been made.
				_vm.Bitmap = _vm.BaseBitmap.AlterBrightness(val);

				AddImageUndo(ImageModType.IMG_brighten, val);
            }
        }

        private void ContrastSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
			if (_vm?.Bitmap != null)
			{
				byte lowerValue = Convert.ToByte(ContrastSlider.LowerValue);
				byte upperValue = Convert.ToByte(ContrastSlider.UpperValue);

				// TODO: Not quite right -- we need more copies or better logic if other changes have been made.
				_vm.Bitmap = _vm.BaseBitmap.EnhanceContrast(lowerValue, upperValue);

				AddImageUndo(ImageModType.IMG_contrast, lowerValue, upperValue);
			}
		}

        #region Undo and Redo

        private void RedoButton_Click(object sender, RoutedEventArgs e)
        {
			if (_vm.RedoItems.Count > 0)
			{
				var mod = _vm.RedoItems.Pop();

				switch (mod.ModificationType)
				{
					case ModificationType.Contour:
						_vm.UndoItems.Push(new Modification
						{
							ModificationType = ModificationType.Contour,
							Contour = new Contour(_vm.Contour)
						});

						_vm.Contour = new Contour(mod.Contour);
						break;

					case ModificationType.Image:
						// TODO: Need to update the sliders based on op.
						_vm.UndoItems.Push(new Modification
						{
							ModificationType = ModificationType.Image,
							ImageMod = mod.ImageMod
						});

						ApplyImageModificationsToOriginal(_vm.UndoItems);
						break;
				}
			}
		}

        private void UndoButton_Click(object sender, RoutedEventArgs e)
        {
			if (_vm.UndoItems.Count > 0)
            {
				var mod = _vm.UndoItems.Pop();

				switch (mod.ModificationType)
                {
					case ModificationType.Contour:
						_vm.RedoItems.Push(new Modification
						{
							ModificationType = ModificationType.Contour,
							Contour = new Contour(_vm.Contour)
						});

						_vm.Contour = new Contour(mod.Contour);
						break;

					case ModificationType.Image:
						// TODO: Need to update the sliders based on op.
						_vm.RedoItems.Push(new Modification
						{
							ModificationType = ModificationType.Image,
							ImageMod = mod.ImageMod
						});

						ApplyImageModificationsToOriginal(_vm.UndoItems);
						break;
                }
            }
        }

		private void AddContourUndo(Contour contour)
        {
			_vm.UndoItems.Push(new Modification
			{
				ModificationType = ModificationType.Contour,
				Contour = new Contour(contour)
			});

			_vm.RedoItems.Clear();
        }

		private void AddImageUndo(ImageModType modType, int val1 = 0, int val2 = 0, int val3 = 0, int val4 = 0)
        {
			// Look at the top of the stack and see if we have brighten operations there, if the current op is a brighten.
			// We're doing this because we'll get a ton of events come through at a time as the slider is moved, and we
			// really only want to store a brightness change as one op.
			if (modType == ImageModType.IMG_brighten && _vm.UndoItems.Count > 0)
            {
				while (_vm.UndoItems.Peek().ModificationType == ModificationType.Image && _vm.UndoItems.Peek().ImageMod.Op == ImageModType.IMG_brighten)
					_vm.UndoItems.Pop();
            }

			// Look at the top of the stack and see if we have contrast operations there, if the current op is a contrast change.
			// We're doing this because we'll get a ton of events come through at a time as the slider is moved, and we
			// really only want to store a contrast change as one op.
			if (modType == ImageModType.IMG_contrast && _vm.UndoItems.Count > 0)
			{
				while (_vm.UndoItems.Peek().ModificationType == ModificationType.Image && _vm.UndoItems.Peek().ImageMod.Op == ImageModType.IMG_contrast)
					_vm.UndoItems.Pop();
			}

			_vm.UndoItems.Push(new Modification
			{
				ModificationType = ModificationType.Image,
				ImageMod = new ImageMod(modType, val1, val2, val3, val4)
			});

			_vm.RedoItems.Clear();
		}

		/// <summary>
		/// Reapplies image modifications to an original.  TODO: Not factored correctly. TODO:Probably not efficient.
		/// </summary>
		/// <param name="modifications"></param>
		private void ApplyImageModificationsToOriginal(ObservableStack<Modification> modifications)
		{
			if (modifications == null)
				throw new ArgumentNullException(nameof(modifications));

			// Rebuild a list in FIFO order. TODO: There's probably a more efficient way to do this.
			var modificationList = new List<Modification>();

			foreach (var stackMod in modifications)
			{
				modificationList.Insert(0, stackMod);
			}

			_vm.Bitmap = new Bitmap(_vm.OriginalBitmap);

			foreach (var mod in modificationList)
			{
				if (mod.ImageMod != null && mod.ModificationType == ModificationType.Image)
				{
					// TODO: This is really awkward
					ImageModType modType;
					int val1, val2, val3, val4;
					mod.ImageMod.Get(out modType, out val1, out val2, out val3, out val4);

					switch (mod.ImageMod.Op)
					{
						case ImageModType.IMG_flip:
							_vm.Bitmap.RotateFlip(RotateFlipType.RotateNoneFlipX);
							break;

						case ImageModType.IMG_brighten:
							_vm.Bitmap.AlterBrightness(val1);
							break;

						case ImageModType.IMG_contrast:
							_vm.Bitmap.EnhanceContrast((byte)val1, (byte)val2);
							break;

						case ImageModType.IMG_crop:
							var cropRect = new System.Drawing.Rectangle(val1, val2, val3 - val1, val4 - val2);
							_vm.Bitmap = ImageTransform.CropBitmap(_vm.Bitmap, cropRect);
							break;

						default:
							throw new NotImplementedException();
					}
				}
			}

			_vm.BaseBitmap = new Bitmap(_vm.Bitmap);
			// Just in case
			_vm.UpdateImage();
		}

		#endregion
	}
}
