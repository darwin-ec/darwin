// This is originally based off Microsoft's WPF examples of RubberbandAdorner and CroppingAdorner, though
// it's been modified a lot.
// Original copyright and license from https://github.com/microsoft/WPF-Samples
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;

namespace Darwin.Wpf.Adorners
{
    public class CroppingAdorner : Adorner
    {
        static public DependencyProperty FillProperty = Shape.FillProperty.AddOwner(typeof(CroppingAdorner));

        public Brush Fill
        {
            get
            {
                return GetValue(FillProperty) as Brush;
            }
            set
            {
                SetValue(FillProperty, value);
            }
        }

        private static void FillPropChanged(DependencyObject o, DependencyPropertyChangedEventArgs args)
        {
            CroppingAdorner croppingAdorner = o as CroppingAdorner;

            if (croppingAdorner != null)
                croppingAdorner._cropMask.Fill = (Brush)args.NewValue;
        }

        public static readonly RoutedEvent CropEvent = EventManager.RegisterRoutedEvent(
            "Crop", RoutingStrategy.Bubble, typeof(RoutedEventHandler), typeof(CroppingAdorner));

        public event RoutedEventHandler Crop
        {
            add
            {
                base.AddHandler(CroppingAdorner.CropEvent, value);
            }
            remove
            {
                base.RemoveHandler(CroppingAdorner.CropEvent, value);
            }
        }


        private readonly UIElement _adornedElement;
        private readonly RectangleGeometry _geometry;
        private System.Windows.Point _anchorPoint;
        // PuncturedRect to hold the "Cropping" portion of the adorner
        private PuncturedRect _cropMask;
        private Rect _selectRect;

        private Canvas _thumbsCanvas;

        private bool _drawingSelection;

        private readonly Thumb _bottomLeft;
        private readonly Thumb _bottomRight;

        // Cropping adorner uses Thumbs for visual elements.  
        // The Thumbs have built-in mouse input handling.
        private readonly Thumb _topLeft;
        private readonly Thumb _topRight;

        private Size _thumbSize;

        private System.Windows.Point ThumbOffset
        {
            get
            {
                return new System.Windows.Point(_thumbSize.Width / 2, _thumbSize.Height / 2);
            }
        }

        private System.Windows.Point ThumbTopLeftPoint { get; set; }
        private System.Windows.Point ThumbTopRightPoint { get; set; }
        private System.Windows.Point ThumbBottomLeftPoint { get; set; }
        private System.Windows.Point ThumbBottomRightPoint { get; set; }

        // To store and manage the adorner's visual children.
        private readonly VisualCollection _visualChildren;

        public CroppingAdorner(UIElement adornedElement)
            : base(adornedElement)
        {
            CropEnabled = false;
            _visualChildren = new VisualCollection(this);

            _drawingSelection = false;
            _adornedElement = adornedElement;
            _selectRect = new Rect();
            _geometry = new RectangleGeometry();

            var dashes = new DoubleCollection(new double[] { 6, 1 });

            Rubberband = new Path
            {
                Data = _geometry,
                StrokeThickness = 2,
                Stroke = Brushes.White,
                StrokeDashArray = dashes,
                Opacity = 0.6,
                Visibility = Visibility.Hidden
            };

            _visualChildren.Add(Rubberband);

            // A transparent black background to show outside the crop area over the rest
            // of the image
            Fill = new SolidColorBrush(Color.FromArgb(110, 0, 0, 0));

            _cropMask = new PuncturedRect();
            _cropMask.IsHitTestVisible = false;
            _cropMask.RectInterior = _selectRect;
            _cropMask.Fill = Fill;
            _cropMask.Visibility = Visibility.Hidden;

            _visualChildren.Add(_cropMask);

            // We're going to put the thumbs in a canvas, since the events and placement
            // actually work correctly in a canvas.
            _thumbsCanvas = new Canvas();
            _thumbsCanvas.HorizontalAlignment = HorizontalAlignment.Stretch;
            _thumbsCanvas.VerticalAlignment = VerticalAlignment.Stretch;
            _visualChildren.Add(_thumbsCanvas);

            // Call a helper method to initialize the Thumbs
            // with a customized cursors.

            _thumbSize = new Size(10, 10);
            BuildAdornerCorner(ref _topLeft, Cursors.SizeNWSE);
            BuildAdornerCorner(ref _topRight, Cursors.SizeNESW);
            BuildAdornerCorner(ref _bottomLeft, Cursors.SizeNESW);
            BuildAdornerCorner(ref _bottomRight, Cursors.SizeNWSE);

            // Add handlers for resizing.
            _bottomLeft.DragDelta += HandleBottomLeft;
            _bottomRight.DragDelta += HandleBottomRight;
            _topLeft.DragDelta += HandleTopLeft;
            _topRight.DragDelta += HandleTopRight;

            MouseMove += DrawSelection;
            MouseUp += EndSelection;

            SetElementVisibility(Visibility.Hidden);
        }

        public bool CropEnabled { get; set; }
        public Rect SelectRect { get => _selectRect; set => _selectRect = value; }

        public Path Rubberband { get; }
        protected override int VisualChildrenCount => _visualChildren.Count;

        private void ResetThumbPositions()
        {
            ThumbTopLeftPoint = new System.Windows.Point(_geometry.Rect.TopLeft.X, _geometry.Rect.TopLeft.Y);
            ThumbTopRightPoint = new System.Windows.Point(_geometry.Rect.TopRight.X, _geometry.Rect.TopRight.Y);
            ThumbBottomLeftPoint = new System.Windows.Point(_geometry.Rect.BottomLeft.X, _geometry.Rect.BottomLeft.Y);
            ThumbBottomRightPoint = new System.Windows.Point(_geometry.Rect.BottomRight.X, _geometry.Rect.BottomRight.Y);
        }

        private void SetThumbPositions(bool resetPositions = true)
        {
            if (resetPositions)
                ResetThumbPositions();

            if (!double.IsInfinity(ThumbTopLeftPoint.X) && !double.IsNaN(ThumbTopLeftPoint.X))
            {
                Canvas.SetLeft(_topLeft, ThumbTopLeftPoint.X - ThumbOffset.X);
                Canvas.SetTop(_topLeft, ThumbTopLeftPoint.Y - ThumbOffset.Y);
            }

            if (!double.IsInfinity(ThumbTopRightPoint.X) && !double.IsNaN(ThumbTopRightPoint.X))
            {
                Canvas.SetLeft(_topRight, ThumbTopRightPoint.X - ThumbOffset.X);
                Canvas.SetTop(_topRight, ThumbTopRightPoint.Y - ThumbOffset.Y);
            }

            if (!double.IsInfinity(ThumbBottomLeftPoint.X) && !double.IsNaN(ThumbBottomLeftPoint.X))
            {
                Canvas.SetLeft(_bottomLeft, ThumbBottomLeftPoint.X - ThumbOffset.X);
                Canvas.SetTop(_bottomLeft, ThumbBottomLeftPoint.Y - ThumbOffset.Y);
            }

            if (!double.IsInfinity(ThumbBottomRightPoint.X) && !double.IsNaN(ThumbBottomRightPoint.X))
            {
                Canvas.SetLeft(_bottomRight, ThumbBottomRightPoint.X - ThumbOffset.X);
                Canvas.SetTop(_bottomRight, ThumbBottomRightPoint.Y - ThumbOffset.Y);
            }
        }

        protected override Size ArrangeOverride(Size size)
        {
            var finalSize = base.ArrangeOverride(size);
            ((UIElement)GetVisualChild(0))?.Arrange(new Rect(new System.Windows.Point(), finalSize));

            Rect rcExterior = new Rect(0, 0, AdornedElement.RenderSize.Width, AdornedElement.RenderSize.Height);
            _cropMask.RectExterior = rcExterior;
            _cropMask.RectInterior = _selectRect;
            _cropMask.Arrange(rcExterior);

            SetThumbPositions(false);

            _thumbsCanvas.Arrange(new Rect(0, 0, AdornedElement.RenderSize.Width, AdornedElement.RenderSize.Height));

            return finalSize;
        }

        //protected override void OnRender(DrawingContext drawingContext)
        //{
        //    var brush2 = new SolidColorBrush
        //    {
        //        Color = Colors.Black,
        //        Opacity = 0.5
        //    };
        //    drawingContext.DrawRectangle(brush2, null, new Rect(0, 0, _thumbsCanvas.ActualWidth, _thumbsCanvas.ActualHeight));
        //    if (Rubberband != null && Rubberband.Visibility == Visibility.Visible)
        //    {
        //        var brush = new SolidColorBrush
        //        {
        //            Color = Colors.Black,
        //            Opacity = 0.5
        //        };
        //        drawingContext.DrawRectangle(brush, null, new Rect(0, 0, _thumbsCanvas.ActualWidth, _thumbsCanvas.ActualHeight));
        //    }
        //    base.OnRender(drawingContext);
        //}

        //protected override Geometry GetLayoutClip(Size layoutSlotSize)
        //{
        //    //_adornedElement.
        //   // var adornedRect = _adornedElement.TransformToVisual(Application.Current.MainWindow).TransformBounds(LayoutInformation.GetLayoutSlot(_adornedElement);
        //    // Add a group that includes the whole window except the adorned control
        //    GeometryGroup grp = new GeometryGroup();
        //    //grp.Children.Add(new RectangleGeometry(_adornedElement.GetBou));
        //    //grp.Children.Add(new RectangleGeometry(new Rect(layoutSlotSize)));
        //    //grp.Children.Add(new RectangleGeometry(new Rect(0, 0, _thumbsCanvas.ActualWidth, _thumbsCanvas.ActualHeight)));

        //    grp.Children.Add(new RectangleGeometry(new Rect(0, 0, 200, 200)));
        //    if (_selectRect != null)
        //        grp.Children.Add(new RectangleGeometry(_selectRect));

        //    return grp;
        //}

        public void StartSelection(System.Windows.Point anchorPoint)
        {
            if (CropEnabled && _selectRect != null && _selectRect.Contains(anchorPoint))
            {
                RaiseEvent(new RoutedEventArgs(CropEvent, this));
                SetElementVisibility(Visibility.Hidden);
            }
            else
            {
                _drawingSelection = true;
                _anchorPoint = anchorPoint;
                _selectRect.Size = new Size(2, 2);
                _selectRect.Location = _anchorPoint;
                _geometry.Rect = _selectRect;

                if (Visibility.Visible != Rubberband.Visibility)
                    SetElementVisibility(Visibility.Visible);
            }
        }

        private void SetElementVisibility(Visibility visibility)
        {
            Rubberband.Visibility = visibility;
            _topLeft.Visibility = visibility;
            _topRight.Visibility = visibility;
            _bottomLeft.Visibility = visibility;
            _bottomRight.Visibility = visibility;
            _cropMask.Visibility = visibility;
        }

        private void DrawSelection(object sender, MouseEventArgs e)
        {
            if (_drawingSelection && e.LeftButton == MouseButtonState.Pressed)
            {
                var mousePosition = e.GetPosition(_adornedElement);
                _selectRect.X = mousePosition.X < _anchorPoint.X ? mousePosition.X : _anchorPoint.X;
                _selectRect.Y = mousePosition.Y < _anchorPoint.Y ? mousePosition.Y : _anchorPoint.Y;
                _selectRect.Width = Math.Abs(mousePosition.X - _anchorPoint.X);
                _selectRect.Height = Math.Abs(mousePosition.Y - _anchorPoint.Y);
                _geometry.Rect = _selectRect;

                _cropMask.RectInterior = _selectRect;

                SetThumbPositions();

                var layer = AdornerLayer.GetAdornerLayer(_adornedElement);
                layer.InvalidateArrange();
            }
        }

        private void EndSelection(object sender, MouseButtonEventArgs e)
        {
            SetThumbPositions();

            if (3 >= _selectRect.Width || 3 >= _selectRect.Height)
            {
                SetElementVisibility(Visibility.Hidden);
                CropEnabled = false;
            }
            else
            {
                CropEnabled = true;
            }
            _drawingSelection = false;
            ReleaseMouseCapture();
        }

        private void CalculateRectAndRedraw()
        {
            SetThumbPositions(false);

            _geometry.Rect = _selectRect = new Rect(
                ThumbTopLeftPoint.X < ThumbTopRightPoint.X ? ThumbTopLeftPoint.X : ThumbTopRightPoint.X,
                ThumbTopLeftPoint.Y < ThumbBottomLeftPoint.Y ? ThumbTopLeftPoint.Y : ThumbBottomLeftPoint.Y,
                Math.Abs(ThumbTopLeftPoint.X - ThumbTopRightPoint.X),
                Math.Abs(ThumbTopLeftPoint.Y - ThumbBottomLeftPoint.Y));

            _cropMask.RectInterior = _selectRect;

            var layer = AdornerLayer.GetAdornerLayer(_adornedElement);
            layer.InvalidateArrange();
        }

        // Handler for Cropping from the bottom-left.
        private void HandleBottomLeft(object sender, DragDeltaEventArgs args)
        {
            args.Handled = true;

            ThumbBottomLeftPoint = new System.Windows.Point(
                ThumbBottomLeftPoint.X + args.HorizontalChange,
                ThumbBottomLeftPoint.Y + args.VerticalChange);

            ThumbTopLeftPoint = new System.Windows.Point(
                ThumbBottomLeftPoint.X,
                ThumbTopLeftPoint.Y);

            ThumbBottomRightPoint = new System.Windows.Point(
                ThumbBottomRightPoint.X,
                ThumbBottomLeftPoint.Y);

            CalculateRectAndRedraw();
        }

        // Handler for Cropping from the bottom-right.
        private void HandleBottomRight(object sender, DragDeltaEventArgs args)
        {
            args.Handled = true;

            ThumbBottomRightPoint = new System.Windows.Point(
                ThumbBottomRightPoint.X + args.HorizontalChange,
                ThumbBottomRightPoint.Y + args.VerticalChange);

            ThumbBottomLeftPoint = new System.Windows.Point(
                ThumbBottomLeftPoint.X,
                ThumbBottomRightPoint.Y);

            ThumbTopRightPoint = new System.Windows.Point(
                ThumbBottomRightPoint.X,
                ThumbTopRightPoint.Y);

            CalculateRectAndRedraw();
        }

        // Handler for Cropping from the top-right.
        private void HandleTopRight(object sender, DragDeltaEventArgs args)
        {
            args.Handled = true;

            ThumbTopRightPoint = new System.Windows.Point(
                ThumbTopRightPoint.X + args.HorizontalChange,
                ThumbTopRightPoint.Y + args.VerticalChange);

            ThumbBottomRightPoint = new System.Windows.Point(
                ThumbTopRightPoint.X,
                ThumbBottomRightPoint.Y);

            ThumbTopLeftPoint = new System.Windows.Point(
                ThumbTopLeftPoint.X,
                ThumbTopRightPoint.Y);

            CalculateRectAndRedraw();
        }

        // Handler for Cropping from the top-left.
        private void HandleTopLeft(object sender, DragDeltaEventArgs args)
        {
            args.Handled = true;

            ThumbTopLeftPoint = new System.Windows.Point(
                ThumbTopLeftPoint.X + args.HorizontalChange,
                ThumbTopLeftPoint.Y + args.VerticalChange);

            ThumbTopRightPoint = new System.Windows.Point(
                ThumbTopRightPoint.X,
                ThumbTopLeftPoint.Y);

            ThumbBottomLeftPoint = new System.Windows.Point(
                ThumbTopLeftPoint.X,
                ThumbBottomLeftPoint.Y);

            CalculateRectAndRedraw();
        }

        // Helper method to instantiate the corner Thumbs, set the Cursor property, 
        // set some appearance properties, and add the elements to the visual tree.
        private void BuildAdornerCorner(ref Thumb cornerThumb, Cursor customizedCursor)
        {
            if (cornerThumb != null)
                return;

            cornerThumb = new Thumb { Cursor = customizedCursor };

            cornerThumb.Height = _thumbSize.Height;
            cornerThumb.Width = _thumbSize.Width;
            cornerThumb.Background = Brushes.Transparent;

            _thumbsCanvas.Children.Add(cornerThumb);
        }

        protected override Visual GetVisualChild(int index) => _visualChildren[index];
    }
}