using Darwin.Collections;
using Darwin.Database;
using Darwin.Extensions;
using Darwin.Features;
using Darwin.Helpers;
using Darwin.ImageProcessing;
using Darwin.Wpf.Adorners;
using Darwin.Wpf.Commands;
using Darwin.Wpf.Extensions;
using Darwin.Wpf.Model;
using Darwin.Wpf.ViewModel;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Security.Cryptography.Xml;
using System.Security.Policy;
using System.Text;
using Path = System.IO.Path;
using FileInfo = System.IO.FileInfo;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Windows.Threading;
using System.Xml.Schema;

namespace Darwin.Wpf
{
    /// <summary>
    /// Interaction logic for TraceWindow.xaml
    /// </summary>
    public partial class TraceWindow : Window
    {
		private const int ImagePadding = 20;
		private const string NoTraceErrorMessageDatabase = "You must trace your image before it can be added to the database.";
		private const string NoIDErrorMessage = "You must enter an ID Code before\nyou can add a {0} to the database.";
		private const string NoCategoryErrorMessage = "You must select a category before proceeding.";

		private CroppingAdorner _cropSelector;
		System.Windows.Point? lastCenterPositionOnTarget;
		System.Windows.Point? lastMousePositionOnTarget;
		System.Windows.Point? lastDragPoint;

		//private const int PointSize = 2;
		private const int SpaceBetweenPoints = 3;
		private const int EraserBrushSize = 9; // krd 10/28/05
		//private const int MaxZoom = 1600;
		//private const int MinZoom = 6; //***1.95 - minimum is now 6% of original size

		private int _movePosition;
		private bool _drawingWithPencil;

		private int _chopPosition;
		private int _chopLead;

		private bool _eraseInit = false;

		private string _previousStatusBarMessage;

		private FeaturePointType _moveFeature;
        private bool _moveFeatureUseCoordinate;
		private CoordinateFeaturePoint _moveCoordinateFeature;

		private TraceWindowViewModel _vm;

		public TraceWindow(TraceWindowViewModel vm)
		{
			InitializeComponent();

			_moveFeature = FeaturePointType.NoFeature;
			_movePosition = -1;

			_vm = vm;

			if (_vm.TraceLocked)
            {
                _vm.TraceTool = TraceToolType.MoveFeature;
                _vm.TraceStep = TraceStepType.IdentifyFeatures;
            }

			if (_vm.ViewerMode)
            {
				_vm.TraceTool = TraceToolType.Hand;
            }

			_vm.ZoomValues.Add(16);
			_vm.ZoomValues.Add(8);
			_vm.ZoomValues.Add(4);
			_vm.ZoomValues.Add(2);
			_vm.ZoomValues.Add(1);
			_vm.ZoomValues.Add(0.75);
			_vm.ZoomValues.Add(0.50);
			_vm.ZoomValues.Add(0.25);
			_vm.ZoomValues.Add(0.125);
			_vm.ZoomValues.Add(0.0625);

			// We need to wait until the window is loaded before the scroll viewer will have ActualWidth and ActualHeight
			// so that we can compare to the bitmap to compute the initial zoom ratio, if needed.
			Loaded += delegate
			{
				if (_vm.Bitmap != null) // && ((_vm.Bitmap.Width + ImagePadding) > TraceScrollViewer.ActualWidth || (_vm.Bitmap.Height + ImagePadding) > TraceScrollViewer.ActualHeight))
				{
					var heightRatio = TraceScrollViewer.ActualHeight / (_vm.Bitmap.Height + ImagePadding);
					var widthRatio = TraceScrollViewer.ActualWidth / (_vm.Bitmap.Width + ImagePadding);
					if (widthRatio < heightRatio)
					{
						// the width is the restrictive dimension
						_vm.ZoomRatio = (float)Math.Round(widthRatio, 2);
					}
					else
					{
						_vm.ZoomRatio = (float)Math.Round(heightRatio, 2);
					}
				}
			};
			StateChanged += TraceWindowStateChangeRaised;
			this.DataContext = _vm;
		}

		private void TraceWindowStateChangeRaised(object sender, EventArgs e)
		{
			if (WindowState == WindowState.Maximized)
			{
				TraceWindowBorder.BorderThickness = new Thickness(8);
				RestoreButton.Visibility = Visibility.Visible;
				MaximizeButton.Visibility = Visibility.Collapsed;
			}
			else
			{
				TraceWindowBorder.BorderThickness = new Thickness(0);
				RestoreButton.Visibility = Visibility.Collapsed;
				MaximizeButton.Visibility = Visibility.Visible;
			}
		}

		private void WindowIcon_MouseDown(object sender, RoutedEventArgs e)
		{
			SystemCommands.ShowSystemMenu(this, WindowIcon.PointToScreen(new System.Windows.Point(0, WindowIcon.ActualHeight)));
		}

		private void CommandBinding_CanExecute(object sender, CanExecuteRoutedEventArgs e)
		{
			e.CanExecute = true;
		}

		private void CommandBinding_Executed_Minimize(object sender, ExecutedRoutedEventArgs e)
		{
			SystemCommands.MinimizeWindow(this);
		}

		private void CommandBinding_Executed_Maximize(object sender, ExecutedRoutedEventArgs e)
		{
			SystemCommands.MaximizeWindow(this);
		}

		private void CommandBinding_Executed_Restore(object sender, ExecutedRoutedEventArgs e)
		{
			SystemCommands.RestoreWindow(this);
		}

		private void CommandBinding_Executed_Close(object sender, ExecutedRoutedEventArgs e)
		{
			SystemCommands.CloseWindow(this);
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

		private PointF MapWindowsPointToPointF(System.Windows.Point point)
        {
			// Note we are using the scaling here!
			return new PointF(point.X * _vm.NormScale, point.Y * _vm.NormScale);
        }

		private PointF MapDarwinPointToPointF(Darwin.Point point)
		{
			// Note we are using the scaling here!
			return new PointF(point.X * _vm.NormScale, point.Y * _vm.NormScale);
		}

		private void Button_Click(object sender, RoutedEventArgs e)
		{
			var openFile = new OpenFileDialog();

			// ShowDialog() returns a nullable bool, so we specifically need to test
			// whether it equals true
			if (openFile.ShowDialog() == true)
			{
				Trace.WriteLine(openFile.FileName);

				_vm.OpenImage(openFile.FileName);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// traceReset: used by DeleteContourDialog object to prepare for
		//             redrawing the contour
		//
		private void TraceReset()
		{
			if (_vm.Contour != null)
			{
				AddContourUndo(_vm.Contour);
				_vm.Contour = null;
			}

			//***1.4 - new code to remove Outline, if loaded from fin file
			if (_vm.Outline != null)
				_vm.Outline = null;

			// TODO
			_vm.TraceSnapped = false; //***051TW
			_vm.TraceLocked = false;

			_vm.TraceFinalized = false;
			//mNormScale = 1.0f; //***051TW
		}

		internal IntensityContour CreateIntensityContour(Bitmap bitmap, Contour contour, int left, int top, int right, int bottom)
		{
			return new IntensityContour(
						bitmap,
						contour,
						left, top, right, bottom);
		}

		internal IntensityContour CreateIntensityContourCyan(Bitmap bitmap, Contour contour, int left, int top, int right, int bottom)
		{
			return new IntensityContourCyan(
						bitmap,
						contour,
						left, top, right, bottom);
		}

		//*******************************************************************
		// Point clicked for AutoTrace -- 103AT SAH
		private async void TraceAddAutoTracePoint(Darwin.Point p, bool shiftKeyDown) // AT103 SAH
		{
			if (p.IsEmpty)
				return;

			if (_vm.Contour == null)
				_vm.Contour = new Contour();

			AddContourUndo(_vm.Contour);
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
				Trace.WriteLine("PLEASE WAIT:\n  Automatically detecting rough fin outline ....");

				//***1.96 - figure out how to pass the image the user sees ONLY
				int left, top, right, bottom;
				GetViewedImageBoundsNonZoomed(out left, out top, out right, out bottom);

				//Perform intensity trace (SCOTT) (Aug/2005) Code: 101AT
				Contour trace = null;

				if (!shiftKeyDown)
				{
					try
					{
						this.IsHitTestVisible = false;
						Mouse.OverrideCursor = Cursors.Wait;

						//trace = new IntensityContour(mNonZoomedImage,mContour); //101AT --Changed IntensityContour declaration to Contour
						trace = await Task.Run(() => CreateIntensityContour(_vm.Bitmap,
							_vm.Contour,
							left, top, right, bottom));
					}
                    finally
                    {
						Mouse.OverrideCursor = null;
						this.IsHitTestVisible = true;
                    }
				}

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
										//TraceSnapToFin(false, left, top, right, bottom);//101AT
										//this.skiaElement.InvalidateVisual(); //101AT

					TraceSnapToFinAsync(false, left, top, right, bottom);
				}
				else
				{//101AT
					Trace.WriteLine("\n   Trying Cyan Intensity AutoTrace ...");

					//102AT Add hooks for cyan intensity trace
					try
					{
						this.IsHitTestVisible = false;
						Mouse.OverrideCursor = Cursors.Wait;

						//trace = new IntensityContour(mNonZoomedImage,mContour); //101AT --Changed IntensityContour declaration to Contour
						trace = await Task.Run(() => CreateIntensityContourCyan(_vm.Bitmap,
							_vm.Contour,
							left, top, right, bottom));
					}
					finally
					{
						Mouse.OverrideCursor = null;
						this.IsHitTestVisible = true;
					}

					if (trace.NumPoints > 100)
					{//102AT
						Trace.WriteLine("Using edge detection and active contours to refine outline placement\n   (with cyan intensity image) ....\n");

						_vm.Contour = trace;//102AT

						TraceSnapToFinAsync(true, left, top, right, bottom);
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
			if (!_drawingWithPencil || point.IsEmpty)
				return;

			if (_vm.Contour == null)
				_vm.Contour = new Contour();

			_vm.Contour.AddPoint(point.X, point.Y);
		}

		private async void TraceSnapToFinAsync(bool useCyan, int left, int top, int right, int bottom)
        {
			try
			{
				this.IsHitTestVisible = false;
				Mouse.OverrideCursor = Cursors.Wait;

				// TODO: The multi-scale version is a port of what was in the C++ version.  Trying one without scaling.
				//await Task.Run(() => TraceSnapToFinMultiScale(useCyan, left, top, right, bottom));
				await Task.Run(() => TraceSnapToFinNoScale(useCyan, left, top, right, bottom));
			}
			finally
			{
				Mouse.OverrideCursor = null;
				this.IsHitTestVisible = true;
			}
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
		private void TraceSnapToFinMultiScale(bool useCyan, int left, int top, int right, int bottom)
		{
			if (_vm.Contour == null || _vm.TraceLocked || _vm.Contour.NumPoints < 3) //***006FC
				return;

			float[] energyWeights = new float[]{
				Options.CurrentUserOptions.SnakeEnergyContinuity,
				Options.CurrentUserOptions.SnakeEnergyLinearity,
				Options.CurrentUserOptions.SnakeEnergyEdge
			};

			// full size and currently viewed scale edgeMagImage
			DirectBitmap EdgeMagImage;
			DirectBitmap smallEdgeMagImage;

			DirectBitmap temp;

			if (useCyan)
			{
				temp = new DirectBitmap(_vm.Bitmap);
				temp.ToCyanIntensity(); //***1.96 - copy to cyan
			}
			else
			{
				temp = DirectBitmapHelper.ConvertToDirectBitmapGrayscale(_vm.Bitmap);
			}

            DirectBitmap temp2 = DirectBitmapHelper.CropBitmap(temp, left, top, right, bottom); //***1.96 - then crop

			DirectBitmap EdgeImage = EdgeDetection.CannyEdgeDetection(
					//temp,
					temp2, //***1.96
					out EdgeMagImage,
					true,
					Options.CurrentUserOptions.GaussianStdDev,
					Options.CurrentUserOptions.CannyLowThreshold,
					Options.CurrentUserOptions.CannyHighThreshold);


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

			// Scale mContour to current scale of analysis (image) first time thru

			spacing = _vm.Contour.GetTotalDistanceAlongContour() / 200.0;
			if (spacing < SpaceBetweenPoints)
				spacing = SpaceBetweenPoints;

			float ratio = _vm.ZoomRatio * 100f;

			Contour evenContour = _vm.Contour.EvenlySpaceContourPoints(spacing);
			Contour scaledContour = evenContour.CreateScaledContour(ratio / 100.0f, 0, 0); // small sized countour
																								  
			int chunkSize;  // used to divide up iterations among the scales

			if (ratio <= 100)
				chunkSize = (int)(Options.CurrentUserOptions.SnakeMaximumIterations / (100.0 / ratio * 2 - 1));
			else
				chunkSize = Options.CurrentUserOptions.SnakeMaximumIterations;

			int tripNum = 1;  // one trip at each scale

			// at each ZoomRatio, create an EdgeMagImage and a contour at that scale
			// and process with active contour
			while (ratio <= 100 || tripNum == 1)
			{
				// if ratio > 100 take at least one trip
				int iterations = (int)(Math.Pow(2.0, tripNum - 1) * chunkSize);

				// Resize EdgeMagImage to current scale
				if (ratio != 100)
					smallEdgeMagImage = new DirectBitmap(BitmapHelper.ResizePercentageNearestNeighbor(EdgeMagImage.Bitmap, ratio));
				else
					smallEdgeMagImage = EdgeMagImage;

				for (int i = 0; i < iterations; i++)
				{
					// TODO: Hardcoded neighborhood of 3
					Snake.MoveContour(ref scaledContour, smallEdgeMagImage, 3, energyWeights);
					if (i % 5 == 0)
					{
						// scale repositioned contour to viewed scale for display (mContour)
						Dispatcher.BeginInvoke(new Action(() =>
						{
							_vm.Contour = scaledContour.CreateScaledContour(100.0f / ratio, 0, 0);
						}), DispatcherPriority.Background);
					}
				}

				// Scale repositioned contour to next larger scale for more repositioning
				ratio *= 2;  // double ratio for next level
				scaledContour = scaledContour.CreateScaledContour(2.0f, 0, 0);

				tripNum++;
			}

			// Scale the contour back to "normal" and evenly space the points again
			scaledContour = scaledContour.CreateScaledContour(100.0f / ratio, 0, 0);

			// Features such as glare spots may cause outline points to bunch and wrap
			// during active contour process
			Dispatcher.BeginInvoke(new Action(() =>
			{
				_vm.Contour = new Contour(scaledContour);
				_vm.Contour.RemoveKnots(spacing); //***005CM
			}), DispatcherPriority.Background);
		}

		/// <summary>
		/// traceSnapToFin: called to perform the active contour based fit of
		///    trace to fin outline and remove knots.  This is called after
		///    the initial trace but before user has oportunity to clean
		///    up the trace, add/remove points, etc
		/// </summary>
		/// <param name="useCyan"></param>
		/// <param name="left"></param>
		/// <param name="top"></param>
		/// <param name="right"></param>
		/// <param name="bottom"></param>
		private void TraceSnapToFinNoScale(bool useCyan, int left, int top, int right, int bottom)
		{
			if (_vm.Contour == null || _vm.TraceLocked || _vm.Contour.NumPoints < 3) //***006FC
				return;

			float[] energyWeights = new float[]{
				Options.CurrentUserOptions.SnakeEnergyContinuity,
				Options.CurrentUserOptions.SnakeEnergyLinearity,
				Options.CurrentUserOptions.SnakeEnergyEdge
			};

			DirectBitmap temp;

			if (useCyan)
			{
				temp = new DirectBitmap(_vm.Bitmap);
				temp.ToCyanIntensity(); //***1.96 - copy to cyan
			}
			else
			{
				temp = DirectBitmapHelper.ConvertToDirectBitmapGrayscale(_vm.Bitmap);
			}
			

			DirectBitmap temp2 = DirectBitmapHelper.CropBitmap(temp, left, top, right, bottom); //***1.96 - then crop
			DirectBitmap edgeMagImage;
			DirectBitmap EdgeImage = EdgeDetection.CannyEdgeDetection(
					//temp,
					temp2, //***1.96
					out edgeMagImage,
					true,
					Options.CurrentUserOptions.GaussianStdDev,
					Options.CurrentUserOptions.CannyLowThreshold,
					Options.CurrentUserOptions.CannyHighThreshold);


			//***1.96 - copy edgeMagImage into area of entire temp image bounded by 
			//          left, top, right, bottom
			for (int r = 0; r < edgeMagImage.Height; r++)
			{
				for (int c = 0; c < edgeMagImage.Width; c++)
				{
					// set up area within *temp as the real EdgeMagImage
					temp.SetPixel(left + c, top + r, edgeMagImage.GetPixel(c, r));
				}
			}

			edgeMagImage = temp; //***1.96

			//EdgeMagImage->save("EdgeMagImg.png");

			// create initial evenly spaced contour scaled to zoomed image
			double spacing = 3; //***005CM declaration moved ouside if() below
								// contour is initially spaced with larger of 1) space for 200 points or
								// 2) at three pixels (200 points limits hi res pics from having many points)

			// Scale mContour to current scale of analysis (image) first time thru

			spacing = _vm.Contour.GetTotalDistanceAlongContour() / 200.0;
			if (spacing < SpaceBetweenPoints)
				spacing = SpaceBetweenPoints;

			float ratio = _vm.ZoomRatio * 100f;

			Contour evenContour = _vm.Contour.EvenlySpaceContourPoints(spacing);

			for (int i = 0; i < Options.CurrentUserOptions.SnakeMaximumIterations; i++)
			{
				// TODO: Hardcoded neighborhood of 3
				Snake.MoveContour(ref evenContour, edgeMagImage, 3, energyWeights);
				if (i % 5 == 0)
				{
					// Make yet another copy to avoid a potential concurrency issue
					var displayContour = new Contour(evenContour);
					// Display progress
					Dispatcher.BeginInvoke(new Action(() =>
					{
						_vm.Contour = new Contour(displayContour);
					}), DispatcherPriority.Background);
				}
			}

			// Features such as glare spots may cause outline points to bunch and wrap
			// during active contour process
			Dispatcher.BeginInvoke(new Action(() =>
			{
				_vm.Contour = new Contour(evenContour);
				_vm.Contour.RemoveKnots(spacing); //***005CM
			}), DispatcherPriority.Background);
		}

		// Point added to middle of contour after trace has been finalized
		private void TraceAddExtraPoint(Darwin.Point point)
		{
			if (_vm.Contour == null || point.IsEmpty)
				return;

			AddContourUndo(_vm.Contour);

			// use MovePoint code to display newly added point
			if (_vm.Contour.Scale == 1.0)
				_vm.Contour.AddPointInOrder(point.X, point.Y);
			else
				_vm.Contour.AddPointInOrder((int)Math.Round(point.X * _vm.Contour.Scale), (int)Math.Round(point.Y * _vm.Contour.Scale));
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

			AddContourUndo(_vm.Contour);

			_movePosition = _vm.Contour.FindPositionOfClosestPoint(point.X, point.Y);

			_vm.Contour[_movePosition].Type = PointType.Moving;
		}

		private void TraceMovePointUpdate(Darwin.Point point)
		{
			if (_vm.Contour == null || -1 == _movePosition || point.IsEmpty)
				return;

			if (_vm.Contour.Scale == 1.0)
				_vm.Contour[_movePosition].SetPosition(point.X, point.Y);
			else
				_vm.Contour[_movePosition].SetPosition((int)Math.Round(point.X * _vm.Contour.Scale), (int)Math.Round(point.Y * _vm.Contour.Scale));
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
				if (_vm.Contour.Scale == 1.0)
				{
					_vm.Contour[_movePosition] = new Darwin.Point
					{
						X = point.X,
						Y = point.Y,
						Type = PointType.Normal
					};
				}
				else
                {
					_vm.Contour[_movePosition] = new Darwin.Point
					{
						X = (int)Math.Round(point.X * _vm.Contour.Scale),
						Y = (int)Math.Round(point.Y * _vm.Contour.Scale),
						Type = PointType.Normal
					};
				}
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

			//_moveInit = true;
			float featureDistance;
			var feature = _vm.Outline.FindClosestFeaturePointWithDistance(MapWindowsPointToPointF(point), out featureDistance);


			float coordinateFeatureDistance;
			var coordinateFeature = CoordinateFeaturePoint.FindClosestCoordinateFeaturePointWithDistance(_vm.CoordinateFeaturePoints,
																								MapWindowsPointToPointF(point),
                                                                                                out coordinateFeatureDistance);
			if (feature.IsEmpty && coordinateFeature.IsEmpty)
				return;

			string displayName;
			if (feature.IsEmpty || coordinateFeatureDistance < featureDistance)
            {
				// We're going to move the coordinate point rather than a feature point on the outline
				_moveFeatureUseCoordinate = true;

				displayName = coordinateFeature.Name;

				_movePosition = (int)coordinateFeature.Type;
                _moveCoordinateFeature = coordinateFeature;
				_moveCoordinateFeature.Coordinate.Type = PointType.FeatureMoving;
				Trace.WriteLine("Moving coordinate feature point");
			}
			else
            {
				// We're going to move a feature point on the outline
				_moveFeatureUseCoordinate = false;

				_moveFeature = feature.Type;

				if (FeaturePointType.NoFeature == _moveFeature)
					return;

				displayName = feature.Name;

				_movePosition = feature.Position;

				Trace.WriteLine("Moving from position: " + _movePosition);

				_vm.Contour[_movePosition].Type = PointType.FeatureMoving;
			}

			if (!string.IsNullOrEmpty(displayName))
			{
				// ***054 - indicate which point is being moved
				_previousStatusBarMessage = StatusBarMessage.Text; // now we have a copy of the label string

				StatusBarMessage.Text = "Moving " + displayName + " -- Drag into position and release mouse button.";
			}
		}

		private void TraceMoveFeaturePointUpdate(Darwin.Point point)
		{
			if (_vm.Contour == null || _movePosition == -1)
				return;

			if (point.IsEmpty)
				return;

			if (_moveFeatureUseCoordinate)
			{
				//_moveCoordinateFeature.Coordinate.SetPosition(point.X, point.Y);
				//Trace.WriteLine("Updating coordinate feature point");
				var mappedPoint = MapDarwinPointToPointF(point);
				_moveCoordinateFeature.Coordinate.SetPosition((int)Math.Round(mappedPoint.X), (int)Math.Round(mappedPoint.Y));
			}
			else
			{
				if (FeaturePointType.NoFeature == _moveFeature)
					return;

				// Find location of closest contour point
				int posit = _vm.Contour.FindPositionOfClosestPoint(point.X, point.Y);

				if (posit < -1)
					return;

				// Set the previous point to normal
				_vm.Contour[_movePosition].Type = PointType.Normal;
				
				int previous, next;
				_vm.Outline.GetNeighboringFeaturePositions(_moveFeature, out previous, out next);

				// Enforce constraints on feature movement so their order cannot be changed
				if (posit > previous && posit < next)
					_movePosition = posit;

				_vm.Contour[_movePosition].Type = PointType.FeatureMoving;

				Trace.WriteLine("Moving position: " + _movePosition);
			}
		}

		private void TraceMoveFeaturePointFinalize(Darwin.Point point)
		{
			if (_vm.Contour == null)
				return;

			// TODO: Shouldn't this do something with point on mouse up in case it's different than move?
			if (_moveFeatureUseCoordinate)
			{
				_moveCoordinateFeature.Coordinate.Type = PointType.Normal;
			}
			else
			{
				if ((int)FeaturePointType.NoFeature == _movePosition)
					return;

				// set new location of feature
				_vm.Outline.SetFeaturePoint(_moveFeature, _movePosition);

				_vm.Contour[_movePosition].Type = PointType.Feature;

				Trace.WriteLine("Final moving position: " + _movePosition);
			}

			_moveFeatureUseCoordinate = false;
			_moveCoordinateFeature = null;
			_moveFeature = FeaturePointType.NoFeature;
			_movePosition = -1;

			// Reset message label
			StatusBarMessage.Text = _previousStatusBarMessage;
		}

		private void CropApply(Object sender, RoutedEventArgs args)
        {
			int x = (int)Math.Round(_cropSelector.SelectRect.X);
			int y = (int)Math.Round(_cropSelector.SelectRect.Y);
			int width = (int)Math.Round(_cropSelector.SelectRect.Width);
			int height = (int)Math.Round(_cropSelector.SelectRect.Height);

			if (_vm.Contour == null || _vm.Contour.Length < 1)
			{
				AddImageUndo(ImageModType.IMG_crop,
					x, y,
					x + width, y + height);
			}
			else
            {
				AddContourAndImageUndo(
					_vm.Contour,
					ImageModType.IMG_crop,
					x, y,
					x + width, y + height);

				_vm.Contour.Crop(x, y, x + width, y + height);
			}

			System.Drawing.Rectangle cropRect = new System.Drawing.Rectangle(
				x,
				y,
				width,
				height);

			var croppedBitmap = ImageTransform.CropBitmap(_vm.Bitmap, cropRect);

			_vm.Bitmap = _vm.BaseBitmap = croppedBitmap;

			// Force trace tool checked again to reset stuff
			TraceTool_Checked(null, null);
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
				var adornerLayer = AdornerLayer.GetAdornerLayer(TraceCanvas);

				if (_cropSelector != null)
				{
					adornerLayer.Remove(_cropSelector);
					_cropSelector = null;
				}

				// Set the appropriate cursor
				switch (_vm.TraceTool)
				{
					case TraceToolType.AddPoint:
					case TraceToolType.AutoTrace:
						TraceCanvas.Cursor = Resources["AutoTraceCursor"] as Cursor;
						break;

					case TraceToolType.Crop:
						TraceCanvas.Cursor = Cursors.Cross;
						_cropSelector = new CroppingAdorner(TraceCanvas);
						_cropSelector.Crop += CropApply;
						adornerLayer.Add(_cropSelector);
						break;

					case TraceToolType.ChopOutline:
						TraceCanvas.Cursor = Resources["ChopOutlineCursor"] as Cursor;
						break;

					case TraceToolType.Eraser:
						TraceCanvas.Cursor = Resources["EraserCursor"] as Cursor;
						break;

					case TraceToolType.Hand:
						TraceCanvas.Cursor = Resources["OpenHand"] as Cursor;
						//TraceCanvas.Cursor = Cursors.Hand;
						break;

					case TraceToolType.Magnify:
						TraceCanvas.Cursor = Resources["Magnify2Cursor"] as Cursor;
						break;

					case TraceToolType.MovePoint:
						TraceCanvas.Cursor = Cursors.Hand;
						break;

					case TraceToolType.Pencil:
						TraceCanvas.Cursor = Resources["Pencil2Cursor"] as Cursor;
						break;

					default:
						TraceCanvas.Cursor = Cursors.Arrow;
						break;
				}
			}
		}

		private void ZoomIn(System.Windows.Point point)
		{
			lastMousePositionOnTarget = point;

			double newValue = _vm.ZoomSlider + 1;

			if (newValue > ZoomSlider.Maximum)
				newValue = ZoomSlider.Maximum;

			_vm.ZoomSlider = (float)newValue;

			ScaleTransform.ScaleX = _vm.ZoomRatio;
			ScaleTransform.ScaleY = _vm.ZoomRatio;
		}

		private void ZoomOut(System.Windows.Point point)
		{
			lastMousePositionOnTarget = point;

			double newValue = _vm.ZoomSlider - 1;

			if (newValue < ZoomSlider.Minimum)
				newValue = ZoomSlider.Minimum;

			_vm.ZoomSlider = (float)newValue;

			ScaleTransform.ScaleX = _vm.ZoomRatio;
			ScaleTransform.ScaleY = _vm.ZoomRatio;
		}

		private void TraceScrollViewer_ScrollChanged(object sender, ScrollChangedEventArgs e)
		{
			if (e.ExtentHeightChange != 0 || e.ExtentWidthChange != 0)
			{
                System.Windows.Point? targetBefore = null;
                System.Windows.Point? targetNow = null;

                if (lastMousePositionOnTarget == null)
                {
                    if (lastCenterPositionOnTarget != null)
                    {
                        var centerOfViewport = new System.Windows.Point(TraceScrollViewer.ViewportWidth / 2, TraceScrollViewer.ViewportHeight / 2);
                        System.Windows.Point centerOfTargetNow = TraceScrollViewer.TranslatePoint(centerOfViewport, ImageViewBox);

                        targetBefore = lastCenterPositionOnTarget;
                        targetNow = centerOfTargetNow;
                    }
                }
                else
                {
                    targetBefore = lastMousePositionOnTarget;
                    targetNow = Mouse.GetPosition(ImageViewBox);

                    lastMousePositionOnTarget = null;
                }

                if (targetBefore != null)
                {
					double dXInTargetPixels = targetNow.Value.X - targetBefore.Value.X;
					double dYInTargetPixels = targetNow.Value.Y - targetBefore.Value.Y;

                    double newOffsetX = TraceScrollViewer.HorizontalOffset - dXInTargetPixels * ScaleTransform.ScaleX;
                    double newOffsetY = TraceScrollViewer.VerticalOffset - dYInTargetPixels * ScaleTransform.ScaleY;

                    if (!double.IsNaN(newOffsetX) && !double.IsNaN(newOffsetY))
                    {
						TraceScrollViewer.ScrollToHorizontalOffset(newOffsetX);
						TraceScrollViewer.ScrollToVerticalOffset(newOffsetY);
					}
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
			if (e.Delta > 0)
				ZoomIn(e.GetPosition(ImageViewBox));
			else if (e.Delta < 0)
				ZoomOut(e.GetPosition(ImageViewBox));

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
				ScaleTransform.ScaleX = _vm.ZoomRatio;
				ScaleTransform.ScaleY = _vm.ZoomRatio;

				var centerOfViewport = new System.Windows.Point(TraceScrollViewer.ViewportWidth / 2, TraceScrollViewer.ViewportHeight / 2);
				lastCenterPositionOnTarget = TraceScrollViewer.TranslatePoint(centerOfViewport, ImageViewBox);
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
					if (!_vm.TraceLocked)
					{ //***051TW
						if (_vm.Contour != null && _vm.Contour.Length > 0) //***1.4TW - empty is OK to reuse
						{
							ShowOutlineDeleteDialog();
						}
						else
						{
							AddContourUndo(_vm.Contour);
							_drawingWithPencil = true;
							TraceAddNormalPoint(bitmapPoint);
						}
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
					Mouse.Capture(TraceCanvas);
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

		private Cursor _previousCursor;
		private void TraceCanvas_MouseMove(object sender, MouseEventArgs e)
		{
			var clickedPoint = e.GetPosition(this.TraceCanvas);
			var imagePoint = MapWindowsPointToDarwinPoint(clickedPoint);

			if (imagePoint.IsEmpty)
				CursorPositionMessage.Text = string.Empty;
			else
				CursorPositionMessage.Text = string.Format("{0}, {1}", imagePoint.X, imagePoint.Y);

			if (e.LeftButton == MouseButtonState.Pressed)
			{
				//Trace.WriteLine(clickedPoint);
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
			else
            {
				switch (_vm.TraceTool)
                {
					case TraceToolType.Crop:
						if (_cropSelector != null && _cropSelector.CropEnabled &&
						_cropSelector.SelectRect != null && _cropSelector.SelectRect.Contains(clickedPoint))
						{
							if (_previousCursor == null)
								_previousCursor = TraceCanvas.Cursor;

							if (Cursor != Cursors.ScrollAll)
								TraceCanvas.Cursor = Cursors.ScrollAll;
						}
						else if (_previousCursor != null)
						{
							TraceCanvas.Cursor = _previousCursor;
							_previousCursor = null;
						}
						break;
				}
            }
		}

		private async void TraceCanvas_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
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
					_drawingWithPencil = false;

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

							TraceSnapToFinAsync(false, left, top, right, bottom);
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
					TraceCanvas.ReleaseMouseCapture();
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
			if (double.TryParse(text, out num) && _vm.ZoomValues.Contains(num))
			{
				Dispatcher.BeginInvoke(new Action(() =>
				{
					var textBox = ZoomComboBox.Template.FindName("PART_EditableTextBox", ZoomComboBox) as TextBox;
					if (textBox != null)
						textBox.Text = num.ToString("P0");
				}));
				_vm.ZoomRatio = (float)num;
			}
		}

        private void FlipHorizontalButton_Click(object sender, RoutedEventArgs e)
        {
			if (_vm.Bitmap != null)
			{
				_vm.Bitmap.RotateFlip(RotateFlipType.RotateNoneFlipX);
				_vm.BaseBitmap = new Bitmap(_vm.Bitmap);
				_vm.UpdateImage();

				if (_vm.Contour != null && _vm.Contour.Length > 0)
				{
					AddContourAndImageUndo(_vm.Contour, ImageModType.IMG_flip);

					_vm.Contour.FlipHorizontally(_vm.Bitmap.Width);
				}
				else
                {
					AddImageUndo(ImageModType.IMG_flip);
                }
			}
		}

		private void RotateCWButton_Click(object sender, RoutedEventArgs e)
		{
			if (_vm.Bitmap != null)
			{
				_vm.Bitmap.RotateFlip(RotateFlipType.Rotate90FlipNone);
				_vm.BaseBitmap = new Bitmap(_vm.Bitmap);
				_vm.UpdateImage();

				if (_vm.Contour != null && _vm.Contour.Length > 0)
				{
					AddContourAndImageUndo(_vm.Contour, ImageModType.IMG_rotate90cw);

					_vm.Contour.Rotate90CW(_vm.Bitmap.Height, _vm.Bitmap.Width);
				}
				else
				{
					AddImageUndo(ImageModType.IMG_rotate90cw);
				}
			}
		}

		private void RotateCCWButton_Click(object sender, RoutedEventArgs e)
		{
			if (_vm.Bitmap != null)
			{
				_vm.Bitmap.RotateFlip(RotateFlipType.Rotate270FlipNone);
				_vm.BaseBitmap = new Bitmap(_vm.Bitmap);
				_vm.UpdateImage();

				if (_vm.Contour != null && _vm.Contour.Length > 0)
				{
					AddContourAndImageUndo(_vm.Contour, ImageModType.IMG_rotate90ccw);

					_vm.Contour.Rotate90CCW(_vm.Bitmap.Height, _vm.Bitmap.Width);
				}
				else
				{
					AddImageUndo(ImageModType.IMG_rotate90ccw);
				}
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
				// With a two thumb slider, but changing the contrast function
				//byte lowerValue = Convert.ToByte(ContrastSlider.LowerValue);
				//byte upperValue = Convert.ToByte(ContrastSlider.UpperValue);

				var value = Convert.ToInt32(ContrastSlider.Value);

				// TODO: Not quite right -- we need more copies or better logic if other changes have been made.
				_vm.Bitmap = _vm.BaseBitmap.EnhanceContrast(value);

				AddImageUndo(ImageModType.IMG_contrast2, Convert.ToInt32(value));
			}
		}

		/////////////////////////////////////////////////////////////////////
		// traceFinalize:  Locks in trace after user cleanup, but before
		//    user is allowed to move feature points (tip, notch, etc)
		//
		private void TraceFinalize()
		{
			if (_vm.Contour == null || !_vm.TraceLocked) //***006FC
				return;

			_vm.BackupContour = new Contour(_vm.Contour);

			// After even spacing and normalization fin height will be approx 600 units
			_vm.NormScale = _vm.Contour.NormalizeContour(); //***006CN
			_vm.Contour.RemoveKnots(3.0);       //***006CN

			_vm.Contour = _vm.Contour.EvenlySpaceContourPoints(3.0); //***006CN

			_vm.Outline = new Outline(_vm.Contour, _vm.Database.CatalogScheme.FeatureSetType); //***008OL

			_vm.LoadCoordinateFeaturePoints();

			_vm.TraceFinalized = true; //***006PD moved from beginning of function

			_vm.TraceTool = TraceToolType.MoveFeature;
		}

		private void TraceStep_Checked(object sender, RoutedEventArgs e)
		{
			switch (_vm.TraceStep)
            {
				case TraceStepType.TraceOutline:
					if (_vm.TraceFinalized)
					{
						if (_vm.BackupContour != null)
						{
							_vm.Contour = _vm.BackupContour;
						}
						else
                        {
							var tempContour = new Contour(_vm.Contour, true);
							
							double spacing = tempContour.GetTotalDistanceAlongContour() / 200.0;
							if (spacing < SpaceBetweenPoints)
								spacing = SpaceBetweenPoints;

							_vm.Contour = tempContour.EvenlySpaceContourPoints(spacing);
						}

						_vm.Outline = null;

						_vm.TraceLocked = false;
						_vm.TraceFinalized = false;
						_vm.TraceTool = TraceToolType.MovePoint;
					}
					break;

				case TraceStepType.IdentifyFeatures:
					if (!_vm.TraceFinalized)
					{
						_vm.TraceLocked = true;
						TraceFinalize();
						_vm.TraceTool = TraceToolType.MoveFeature;
					}
					break;
            }
		}

		#region Undo and Redo

		private void RedoCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
		{
			if (_vm == null)
				e.CanExecute = false;
			else
				e.CanExecute = _vm.RedoEnabled;
		}

		private void RedoCommand_Executed(object sender, RoutedEventArgs e)
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

						_vm.Contour = (mod.Contour == null) ? null : new Contour(mod.Contour);
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

					case ModificationType.Both:
						// TODO: Need to update the sliders based on op.
						_vm.UndoItems.Push(new Modification
						{
							ModificationType = ModificationType.Both,
							ImageMod = mod.ImageMod,
							Contour = new Contour(_vm.Contour)
						});
						_vm.Contour = (mod.Contour == null) ? null : new Contour(mod.Contour);
						ApplyImageModificationsToOriginal(_vm.UndoItems);
						break;
				}

				// Force the trace tool checked which does things like reset the crop adorner
				TraceTool_Checked(null, null);
			}
		}

		private void UndoCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
		{
			if (_vm == null)
				e.CanExecute = false;
			else
				e.CanExecute = _vm.UndoEnabled;
		}

		private void UndoCommand_Executed(object sender, RoutedEventArgs e)
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
							Contour = (_vm.Contour == null) ? null : new Contour(_vm.Contour)
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

					case ModificationType.Both:
						// TODO: Need to update the sliders based on op.
						_vm.RedoItems.Push(new Modification
						{
							ModificationType = ModificationType.Both,
							ImageMod = mod.ImageMod,
							Contour = (_vm.Contour == null) ? null : new Contour(_vm.Contour)
						});
						_vm.Contour = new Contour(mod.Contour);
						ApplyImageModificationsToOriginal(_vm.UndoItems);
						break;
                }

				// Force the trace tool checked which does things like reset the crop adorner
				TraceTool_Checked(null, null);
			}
        }


		private void AddContourAndImageUndo(Contour contour, ImageModType modType, int val1 = 0, int val2 = 0, int val3 = 0, int val4 = 0)
        {
			// Note:  Does not handle brightness/contrast correctly, but they're not using this method
			// as of the time this comment was written.
			var modifiation = new Modification
			{
				ModificationType = ModificationType.Both,
				Contour = new Contour(contour),
				ImageMod = new ImageMod(modType, val1, val2, val3, val4)
			};

			_vm.UndoItems.Push(modifiation);
			_vm.RedoItems.Clear();
		}

		private void AddContourUndo(Contour contour)
        {
			if (contour == null)
				contour = new Contour();

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
				while (_vm.UndoItems.Count > 0 && _vm.UndoItems.Peek().ModificationType == ModificationType.Image && _vm.UndoItems.Peek().ImageMod.Op == ImageModType.IMG_brighten)
					_vm.UndoItems.Pop();
            }

			// Look at the top of the stack and see if we have contrast operations there, if the current op is a contrast change.
			// We're doing this because we'll get a ton of events come through at a time as the slider is moved, and we
			// really only want to store a contrast change as one op.
			if (modType == ImageModType.IMG_contrast2 && _vm.UndoItems.Count > 0)
			{
				while (_vm.UndoItems.Count > 0 && _vm.UndoItems.Peek().ModificationType == ModificationType.Image && _vm.UndoItems.Peek().ImageMod.Op == ImageModType.IMG_contrast2)
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
				modificationList.Insert(0, stackMod);

			_vm.Bitmap = ModificationHelper.ApplyImageModificationsToOriginal(_vm.OriginalBitmap, modificationList);

			_vm.BaseBitmap = new Bitmap(_vm.Bitmap);
			// Just in case
			_vm.UpdateImage();
		}

        #endregion

        #region Command Buttons
        private void MatchButton_Click(object sender, RoutedEventArgs e)
        {
			if (_vm.Contour == null && _vm.Outline == null)
			{
				MessageBox.Show("You must trace your image before it can be matched.", "Not Traced", MessageBoxButton.OK, MessageBoxImage.Error);
			}
			else if (_vm.Database.AllFins.Count < 1)
			{
				var result = MessageBox.Show("The database is empty, so there is nothing to match against." + Environment.NewLine +
					string.Format("Would you like to just add this {0} to the database?", _vm.Database.CatalogScheme.IndividualTerminology), "Nothing to Match", MessageBoxButton.YesNo);

				if (result == MessageBoxResult.Yes)
					AddToDatabaseButton_Click(null, null);
			}
			else
			{
				try
				{
					this.IsHitTestVisible = false;
					Mouse.OverrideCursor = Cursors.Wait;

					if (_vm.Outline == null)
					{
						_vm.TraceLocked = true;
						TraceFinalize();
					}

					_vm.UpdateDatabaseFin();

					var matchingWindowVM = new MatchingWindowViewModel(_vm.DatabaseFin, _vm.Database);
					var matchingWindow = new MatchingWindow(matchingWindowVM);

					var mainWindow = Application.Current.MainWindow as MainWindow;

					if (mainWindow != null)
						matchingWindow.Owner = mainWindow;

					this.Close();
					matchingWindow.Show();
				}
				finally
				{
					Mouse.OverrideCursor = null;
					this.IsHitTestVisible = true;
				}
			}
        }

		private void SaveCommand_CanExecute(object sender, CanExecuteRoutedEventArgs e)
		{
			e.CanExecute = true;
		}

		private async void SaveCommand_Executed(object sender, RoutedEventArgs e)
		{
			if (_vm.Contour == null && _vm.Outline == null)
			{
				MessageBox.Show("You must trace your image before it can be saved.", "Not Traced", MessageBoxButton.OK, MessageBoxImage.Warning);
			}
			else
			{
				if (_vm.Outline == null)
				{
					_vm.TraceLocked = true;
					TraceFinalize();
				}

				SaveFileDialog dlg = new SaveFileDialog();

				if (string.IsNullOrEmpty(_vm.DatabaseFin.FinFilename))
				{
					dlg.InitialDirectory = Options.CurrentUserOptions.CurrentTracedFinsPath;
					if (string.IsNullOrEmpty(_vm.DatabaseFin.IDCode))
						dlg.FileName = "Untitled";
					else
						dlg.FileName = _vm.DatabaseFin.IDCode;
				}
				else
				{
					dlg.InitialDirectory = new FileInfo(_vm.DatabaseFin.FinFilename).Directory.FullName;
					dlg.FileName = Path.GetFileName(_vm.DatabaseFin.FinFilename);
				}

				dlg.DefaultExt = ".finz";
				dlg.Filter = CustomCommands.TracedFinFilter;

				if (dlg.ShowDialog() == true)
				{
					try
					{
						this.IsHitTestVisible = false;
						Mouse.OverrideCursor = Cursors.Wait;
						
						await Task.Run(() => _vm.SaveFinz(dlg.FileName));
						StatusBarMessage.Text = "Finz file saved.";

						if (_vm.TraceStep == TraceStepType.TraceOutline)
							TraceStep_Checked(null, null);
					}
					catch (Exception ex)
                    {
						Mouse.OverrideCursor = null;
						this.IsHitTestVisible = true;
						Trace.Write(ex);
						MessageBox.Show("There was an error saving your Finz file." + Environment.NewLine + Environment.NewLine +
							"Please try again or contact support.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
					}
					finally
					{
						Mouse.OverrideCursor = null;
						this.IsHitTestVisible = true;
					}
				}
                else
                {
					if (_vm.TraceStep == TraceStepType.TraceOutline)
						TraceStep_Checked(null, null);
				}
			}
		}

        private async void AddToDatabaseButton_Click(object sender, RoutedEventArgs e)
        {
			try
			{
				if (_vm.Contour == null && _vm.Outline == null)
				{
					MessageBox.Show(NoTraceErrorMessageDatabase, "Not Traced", MessageBoxButton.OK, MessageBoxImage.Warning);
				}
				else if (_vm.DatabaseFin == null || string.IsNullOrEmpty(_vm.DatabaseFin.IDCode))
				{
					MessageBox.Show(string.Format(NoIDErrorMessage, _vm.Database.CatalogScheme.IndividualTerminology), "No ID", MessageBoxButton.OK, MessageBoxImage.Warning);
				}
				else if (string.IsNullOrEmpty(_vm.DatabaseFin.DamageCategory) || _vm.Categories.Count < 1 || _vm.DatabaseFin.DamageCategory.ToUpper() == _vm.Categories[0]?.Name?.ToUpper())
				{
					MessageBox.Show(NoCategoryErrorMessage, "No Category", MessageBoxButton.OK, MessageBoxImage.Error);
				}
				else
				{
					try
					{
						this.IsHitTestVisible = false;
						Mouse.OverrideCursor = Cursors.Wait;

						if (_vm.Outline == null)
						{
							_vm.TraceLocked = true;
							TraceFinalize();
						}

						await Task.Run(() => _vm.SaveToDatabase());
						StatusBarMessage.Text = "Finz file saved.";

						if (_vm.TraceStep == TraceStepType.TraceOutline)
							TraceStep_Checked(null, null);

						// Refresh main window
						var mainWindow = Application.Current.MainWindow as MainWindow;

						if (mainWindow != null)
							mainWindow.RefreshDatabaseAfterAdd();
					}
					finally
					{
						Mouse.OverrideCursor = null;
						this.IsHitTestVisible = true;
					}

					Close();
				}
			}
			catch (Exception ex)
            {
				Trace.WriteLine(ex);
				MessageBox.Show("Sorry, something went wrong adding to the database.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
			}
		}

        private void ExportDataButton_Click(object sender, RoutedEventArgs e)
        {
			if (_vm.DatabaseFin != null)
            {
				_vm.SaveSightingData();
				StatusBarMessage.Text = "Sighting data exported.";
            }
        }

        #endregion
    }
}
