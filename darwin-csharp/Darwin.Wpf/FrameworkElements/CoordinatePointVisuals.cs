// Based on 
// ScatterPlotVisuals.cs by Charles Petzold, December 2008
// https://docs.microsoft.com/en-us/archive/msdn-magazine/2009/march/foundations-writing-more-efficient-itemscontrols

using Darwin.Collections;
using Darwin.Features;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace Darwin.Wpf.FrameworkElements
{ 
    public class CoordinatePointVisuals : FrameworkElement
    {
        VisualCollection visualChildren;

        public static readonly DependencyProperty ItemsSourceProperty =
            DependencyProperty.Register("ItemsSource",
                typeof(ObservableNotifiableCollection<CoordinateFeaturePoint>),
                typeof(CoordinatePointVisuals),
                new PropertyMetadata(OnItemsSourceChanged));

        public static readonly DependencyProperty BrushesProperty =
            DependencyProperty.Register("Brushes",
                typeof(Brush[]),
                typeof(CoordinatePointVisuals),
                new FrameworkPropertyMetadata(null,
                        FrameworkPropertyMetadataOptions.AffectsRender));

        public static readonly DependencyProperty PointSizeProperty =
            DependencyProperty.Register("PointSize",
                typeof(double),
                typeof(CoordinatePointVisuals),
                new FrameworkPropertyMetadata(2.0,
                        FrameworkPropertyMetadataOptions.AffectsRender));

        public static readonly DependencyProperty TextSizeProperty =
            DependencyProperty.Register("TextSize",
                typeof(double),
                typeof(CoordinatePointVisuals),
                new FrameworkPropertyMetadata(12.0,
                        FrameworkPropertyMetadataOptions.AffectsRender));

        public static readonly DependencyProperty HighlightPointSizeProperty =
            DependencyProperty.Register("HighlightPointSizeProperty",
                typeof(double),
                typeof(CoordinatePointVisuals),
                new FrameworkPropertyMetadata(22.0,
                        FrameworkPropertyMetadataOptions.AffectsRender));

        public static readonly DependencyProperty FeaturePointSizeProperty =
            DependencyProperty.Register("FeaturePointSize",
                typeof(double),
                typeof(CoordinatePointVisuals),
                new FrameworkPropertyMetadata(6.0,
                        FrameworkPropertyMetadataOptions.AffectsRender));

        public static readonly DependencyProperty ContourScaleProperty =
            DependencyProperty.Register("ContourScale",
                typeof(double),
                typeof(CoordinatePointVisuals),
                new FrameworkPropertyMetadata(1.0,
                        FrameworkPropertyMetadataOptions.AffectsRender));

        public static readonly DependencyProperty XOffsetProperty =
            DependencyProperty.Register("XOffset",
                typeof(double),
                typeof(CoordinatePointVisuals),
                new FrameworkPropertyMetadata(0.0,
                        FrameworkPropertyMetadataOptions.AffectsRender));

        public static readonly DependencyProperty YOffsetProperty =
            DependencyProperty.Register("YOffset",
                typeof(double),
                typeof(CoordinatePointVisuals),
                new FrameworkPropertyMetadata(0.0,
                        FrameworkPropertyMetadataOptions.AffectsRender));

        //public static readonly DependencyProperty BackgroundProperty =
        //    Panel.BackgroundProperty.AddOwner(typeof(PointRender));

        public CoordinatePointVisuals()
        {
            visualChildren = new VisualCollection(this);
            //ToolTip = string.Empty;
        }

        public ObservableNotifiableCollection<CoordinateFeaturePoint> ItemsSource
        {
            set { SetValue(ItemsSourceProperty, value); }
            get { return (ObservableNotifiableCollection<CoordinateFeaturePoint>)GetValue(ItemsSourceProperty); }
        }

        public Brush[] Brushes
        {
            set { SetValue(BrushesProperty, value); }
            get { return (Brush[])GetValue(BrushesProperty); }
        }
        public Pen[] Pens { get; set; }
        public Brush HighlightBrush { get; set; }
        //public Brush Background
        //{
        //    set { SetValue(BackgroundProperty, value); }
        //    get { return (Brush)GetValue(BackgroundProperty); }
        //}

        public double PointSize
        {
            set { SetValue(PointSizeProperty, value); }
            get { return (double)GetValue(PointSizeProperty); }
        }

        public double TextSize
        {
            set { SetValue(TextSizeProperty, value); }
            get { return (double)GetValue(TextSizeProperty); }
        }

        public double HighlightPointSize
        {
            set { SetValue(HighlightPointSizeProperty, value); }
            get { return (double)GetValue(HighlightPointSizeProperty); }
        }

        public double FeaturePointSize
        {
            set { SetValue(FeaturePointSizeProperty, value); }
            get { return (double)GetValue(FeaturePointSizeProperty); }
        }


        public double ContourScale
        {
            set { SetValue(ContourScaleProperty, value); }
            get { return (double)GetValue(ContourScaleProperty); }
        }

        public double XOffset
        {
            set { SetValue(XOffsetProperty, value); }
            get { return (double)GetValue(XOffsetProperty); }
        }

        public double YOffset
        {
            set { SetValue(YOffsetProperty, value); }
            get { return (double)GetValue(YOffsetProperty); }
        }

        static void OnItemsSourceChanged(DependencyObject obj, DependencyPropertyChangedEventArgs args)
        {
            (obj as CoordinatePointVisuals).OnItemsSourceChanged(args);
        }

        protected void OnItemsSourceChanged(DependencyPropertyChangedEventArgs args)
        {
            visualChildren.Clear();

            if (args.OldValue != null)
            {
                ObservableNotifiableCollection<CoordinateFeaturePoint> coll = args.OldValue as ObservableNotifiableCollection<CoordinateFeaturePoint>;
                coll.CollectionCleared -= OnCollectionCleared;
                coll.CollectionChanged -= OnCollectionChanged;
                coll.ItemPropertyChanged -= OnItemPropertyChanged;
            }

            if (args.NewValue != null)
            {
                ObservableNotifiableCollection<CoordinateFeaturePoint> coll = args.NewValue as ObservableNotifiableCollection<CoordinateFeaturePoint>;
                coll.CollectionCleared += OnCollectionCleared;
                coll.CollectionChanged += OnCollectionChanged;
                coll.ItemPropertyChanged += OnItemPropertyChanged;

                CreateVisualChildren(coll);
            }
        }

        // Little hacky
        protected void RebuildHighlightBrush()
        {
            if (HighlightBrush == null)
            {
                HighlightBrush = new SolidColorBrush(Colors.Lime);
                HighlightBrush.Opacity = 0.2;
            }
        }

        protected void RebuildPens()
        {
            if (Brushes == null)
            {
                Pens = null;
            }
            else
            {
                Pens = new Pen[Brushes.Length];
                for (var i = 0; i < Brushes.Length; i++)
                {
                    // TODO: Line thickness?
                    Pens[i] = new Pen(Brushes[i], 2);
                }
            }
        }

        protected void OnCollectionCleared(object sender, EventArgs args)
        {
            RemoveVisualChildren(visualChildren);
        }

        protected void OnCollectionChanged(object sender, NotifyCollectionChangedEventArgs args)
        {
            if (args.OldItems != null)
                RemoveVisualChildren(args.OldItems);

            if (args.NewItems != null)
            {
                // TODO: Make this more efficient.
                RebuildPens();
                CreateVisualChildren(args.NewItems);
            }
        }

        protected void OnItemPropertyChanged(object sender, ItemPropertyChangedEventArgs args)
        {
            CoordinateFeaturePoint point = args.Item as CoordinateFeaturePoint;

            RemoveVisualChild(point);
            CreateVisualChild(point);
        }

        protected void CreateVisualChildren(ICollection coll)
        {
            foreach (object obj in coll)
            {
                CoordinateFeaturePoint point  = obj as CoordinateFeaturePoint;

                if (point != null)
                    CreateVisualChild(point);
            }
        }

        protected void CreateVisualChild(CoordinateFeaturePoint point)
        {
            RebuildHighlightBrush();

            DrawingVisualPlus drawingVisual = new DrawingVisualPlus();
            drawingVisual.DataPoint = point;
            DrawingContext dc = drawingVisual.RenderOpen();

            if (point.IsEmpty || point.Coordinate.IsEmpty)
                return;

            switch (point.Coordinate.Type)
            {
                //    case PointType.Chopping:
                //        dc.DrawEllipse(Brushes[1], null, new System.Windows.Point(point.Coordinate.X / ContourScale + XOffset, point.Y / ContourScale + YOffset), PointSize, PointSize);
                //        break;

                //    case PointType.Feature:
                //        dc.DrawEllipse(Brushes[2], null, new System.Windows.Point(point.Coordinate.X / ContourScale + XOffset, point.Y / ContourScale + YOffset), FeaturePointSize, FeaturePointSize);
                //        break;

                case PointType.FeatureMoving:
                    dc.DrawEllipse(Brushes[1], null, new System.Windows.Point(point.Coordinate.X / ContourScale + XOffset, point.Coordinate.Y / ContourScale + YOffset), PointSize, PointSize);
                    break;

                case PointType.Normal:
                default:
                    dc.DrawEllipse(Brushes[2], null, new System.Windows.Point(point.Coordinate.X / ContourScale + XOffset, point.Coordinate.Y / ContourScale + YOffset), FeaturePointSize, FeaturePointSize);
                    dc.DrawEllipse(HighlightBrush, null, new System.Windows.Point(point.Coordinate.X / ContourScale + XOffset, point.Coordinate.Y / ContourScale + YOffset), HighlightPointSize, HighlightPointSize);

                    if (!string.IsNullOrEmpty(point.Name))
                    {
                        FormattedText formattedText = new FormattedText(
                            point.Name,
                            CultureInfo.GetCultureInfo("en-us"),
                            FlowDirection.LeftToRight,
                            new Typeface("Arial"),
                            TextSize,
                            Brushes[0]);

                        dc.DrawText(formattedText,
                            new System.Windows.Point(point.Coordinate.X / ContourScale + XOffset - formattedText.Width / 2, point.Coordinate.Y / ContourScale + YOffset + formattedText.Height + HighlightPointSize / 2  + 1));
                    }
                    break;
            }

            dc.Close();

            visualChildren.Add(drawingVisual);
        }

        //private void DrawMovingLines(CoordinateFeaturePoint dataPoint, DrawingContext dc)
        //{
        //    var pointIndex = ItemsSource.IndexOf(dataPoint);

        //    if (pointIndex >= 0)
        //    {
        //        if (pointIndex >= 1)
        //        {
        //            dc.DrawLine(Pens[0],
        //                new System.Windows.Point(ItemsSource[pointIndex - 1].X / ContourScale + XOffset, ItemsSource[pointIndex - 1].Y / ContourScale + YOffset),
        //                new System.Windows.Point(dataPoint.X / ContourScale + XOffset, dataPoint.Y / ContourScale));
        //        }

        //        if (pointIndex < ItemsSource.Count - 1)
        //        {
        //            dc.DrawLine(Pens[0],
        //                new System.Windows.Point(ItemsSource[pointIndex + 1].X / ContourScale + XOffset, ItemsSource[pointIndex + 1].Y / ContourScale + YOffset),
        //                new System.Windows.Point(dataPoint.X / ContourScale + XOffset, dataPoint.Y / ContourScale + YOffset));
        //        }
        //    }
        //}

        protected void RemoveVisualChildren(ICollection coll)
        {
            LoopTop:
            foreach (object obj in coll)
            {
                DrawingVisualPlus point = obj as DrawingVisualPlus;

                if (point != null && point.DataPoint != null)
                {
                    RemoveVisualChild(point.DataPoint);
                    // Collection gets modified, so the foreach will throw an exception if we keep going like this
                    goto LoopTop;
                }
            }
        }
        
        protected void RemoveVisualChild(CoordinateFeaturePoint point)
        {
            List<DrawingVisualPlus> removeList = new List<DrawingVisualPlus>();

            foreach (Visual child in visualChildren)
            {
                DrawingVisualPlus drawingVisual = child as DrawingVisualPlus;
                if (drawingVisual.DataPoint == point)
                {
                    removeList.Add(drawingVisual);
                    break;
                }
            }
            foreach (DrawingVisualPlus drawingVisual in removeList)
                visualChildren.Remove(drawingVisual);
        }

        protected override void OnRenderSizeChanged(SizeChangedInfo sizeInfo)
        {
            foreach (Visual child in visualChildren)
            {
                DrawingVisualPlus drawingVisual = child as DrawingVisualPlus;

                if (drawingVisual != null)
                {
                    TranslateTransform xform = drawingVisual.Transform as TranslateTransform;

                    if (xform != null)
                    {
                        if (sizeInfo.WidthChanged)
                            xform.X = sizeInfo.NewSize.Width * (drawingVisual.DataPoint.Coordinate.X / ContourScale + XOffset);

                        if (sizeInfo.HeightChanged)
                            xform.Y = sizeInfo.NewSize.Height * (drawingVisual.DataPoint.Coordinate.Y / ContourScale + YOffset);
                    }
                }
            }
            base.OnRenderSizeChanged(sizeInfo);
        }

        protected override int VisualChildrenCount
        {
            get
            {
                return visualChildren.Count;
            }
        }

        protected override Visual GetVisualChild(int index)
        {
            if (index < 0 || index >= visualChildren.Count)
                throw new ArgumentOutOfRangeException("index");

            return visualChildren[index];
        }

        //protected override void OnRender(DrawingContext dc)
        //{
        //    //if (Background != System.Windows.Media.Brushes.Transparent)
        //    //    dc.DrawRectangle(Background, null, new Rect(RenderSize));
        //}

        //protected override void OnToolTipOpening(ToolTipEventArgs e)
        //{
        //    HitTestResult result = VisualTreeHelper.HitTest(this, Mouse.GetPosition(this));

        //    if (result.VisualHit is DrawingVisualPlus)
        //    {
        //        DrawingVisualPlus drawingVisual = result.VisualHit as DrawingVisualPlus;
        //        Darwin.Point dataPoint = drawingVisual.DataPoint;
        //        ToolTip = String.Format("X={0}, Y={1}", dataPoint.X / ContourScale, dataPoint.Y / ContourScale);
        //    }
        //    base.OnToolTipOpening(e);
        //}

        //protected override void OnToolTipClosing(ToolTipEventArgs e)
        //{
        //    ToolTip = string.Empty;
        //    base.OnToolTipClosing(e);
        //}

        class DrawingVisualPlus : DrawingVisual
        {
            public CoordinateFeaturePoint DataPoint { get; set; }
        }
    }
}
