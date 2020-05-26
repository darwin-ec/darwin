﻿// Based on 
// ScatterPlotVisuals.cs by Charles Petzold, December 2008
// https://docs.microsoft.com/en-us/archive/msdn-magazine/2009/march/foundations-writing-more-efficient-itemscontrols

using Darwin.Collections;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace Darwin.Wpf.FrameworkElements
{ 
    public class PointVisuals : FrameworkElement
    {
        VisualCollection visualChildren;

        public static readonly DependencyProperty ItemsSourceProperty =
            DependencyProperty.Register("ItemsSource",
                typeof(ObservableNotifiableCollection<Darwin.Point>),
                typeof(PointVisuals),
                new PropertyMetadata(OnItemsSourceChanged));

        public static readonly DependencyProperty BrushesProperty =
            DependencyProperty.Register("Brushes",
                typeof(Brush[]),
                typeof(PointVisuals),
                new FrameworkPropertyMetadata(null,
                        FrameworkPropertyMetadataOptions.AffectsRender));

        public static readonly DependencyProperty BackgroundProperty =
            Panel.BackgroundProperty.AddOwner(typeof(PointRender));

        public PointVisuals()
        {
            visualChildren = new VisualCollection(this);
            ToolTip = string.Empty;
        }

        public ObservableNotifiableCollection<Darwin.Point> ItemsSource
        {
            set { SetValue(ItemsSourceProperty, value); }
            get { return (ObservableNotifiableCollection<Darwin.Point>)GetValue(ItemsSourceProperty); }
        }

        public Brush[] Brushes
        {
            set { SetValue(BrushesProperty, value); }
            get { return (Brush[])GetValue(BrushesProperty); }
        }
        public Pen[] Pens { get; set; }

        public Brush Background
        {
            set { SetValue(BackgroundProperty, value); }
            get { return (Brush)GetValue(BackgroundProperty); }
        }

        static void OnItemsSourceChanged(DependencyObject obj, DependencyPropertyChangedEventArgs args)
        {
            (obj as PointVisuals).OnItemsSourceChanged(args);
        }

        protected void OnItemsSourceChanged(DependencyPropertyChangedEventArgs args)
        {
            visualChildren.Clear();

            if (args.OldValue != null)
            {
                ObservableNotifiableCollection<Darwin.Point> coll = args.OldValue as ObservableNotifiableCollection<Darwin.Point>;
                coll.CollectionCleared -= OnCollectionCleared;
                coll.CollectionChanged -= OnCollectionChanged;
                coll.ItemPropertyChanged -= OnItemPropertyChanged;
            }

            if (args.NewValue != null)
            {
                ObservableNotifiableCollection<Darwin.Point> coll = args.NewValue as ObservableNotifiableCollection<Darwin.Point>;
                coll.CollectionCleared += OnCollectionCleared;
                coll.CollectionChanged += OnCollectionChanged;
                coll.ItemPropertyChanged += OnItemPropertyChanged;

                CreateVisualChildren(coll);
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
                    // TODO: Line thickness
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
            Darwin.Point point = args.Item as Darwin.Point;

            RemoveVisualChild(point);
            CreateVisualChild(point);
            //foreach (Visual child in visualChildren)
            //{
            //    DrawingVisualPlus drawingVisual = child as DrawingVisualPlus;

            //    if (dataPoint == drawingVisual.DataPoint)
            //    {
            //        if (dataPoint.Type == PointType.Moving)
            //        {
            //            dc.DrawEllipse(Brushes[0], null, new System.Windows.Point(dataPoint.X, dataPoint.Y), 3, 3);

            //            if (dataPoint.Type == PointType.Moving && Pens != null)
            //            {
            //                DrawMovingLines(dataPoint, dc);
            //            }
            //        }

            //        // Assume only VariableX or VariableY are changing
            //        TranslateTransform xform = drawingVisual.Transform as TranslateTransform;

            //        if (xform != null)
            //        {
            //            if (args.PropertyName == "X")
            //                xform.X = RenderSize.Width * dataPoint.X;

            //            else if (args.PropertyName == "Y")
            //                xform.Y = RenderSize.Height * dataPoint.Y;
            //        }
            //    }
            //}
        }

        protected void CreateVisualChildren(ICollection coll)
        {
            foreach (object obj in coll)
            {
                Darwin.Point point  = obj as Darwin.Point;

                if (point != null)
                    CreateVisualChild(point);
            }
        }

        protected void CreateVisualChild(Darwin.Point point)
        {
            DrawingVisualPlus drawingVisual = new DrawingVisualPlus();
            drawingVisual.DataPoint = point;
            DrawingContext dc = drawingVisual.RenderOpen();

            switch (point.Type)
            {
                case PointType.Chopping:
                    dc.DrawEllipse(Brushes[1], null, new System.Windows.Point(point.X, point.Y), 3, 3);
                    break;

                case PointType.Normal:
                default:
                    dc.DrawEllipse(Brushes[0], null, new System.Windows.Point(point.X, point.Y), 3, 3);
                    break;
            }

            if (point.Type == PointType.Moving && Pens != null)
            {
                DrawMovingLines(point, dc);
            }

            //drawingVisual.Transform = new TranslateTransform(RenderSize.Width * dataPoint.X,
            //                                                 RenderSize.Height * dataPoint.Y);

            dc.Close();
            visualChildren.Add(drawingVisual);
        }

        private void DrawMovingLines(Darwin.Point dataPoint, DrawingContext dc)
        {
            var pointIndex = ItemsSource.IndexOf(dataPoint);

            if (pointIndex >= 0)
            {
                if (pointIndex >= 1)
                {
                    dc.DrawLine(Pens[0],
                        new System.Windows.Point(ItemsSource[pointIndex - 1].X, ItemsSource[pointIndex - 1].Y),
                        new System.Windows.Point(dataPoint.X, dataPoint.Y));
                }

                if (pointIndex < ItemsSource.Count - 1)
                {
                    dc.DrawLine(Pens[0],
                        new System.Windows.Point(ItemsSource[pointIndex + 1].X, ItemsSource[pointIndex + 1].Y),
                        new System.Windows.Point(dataPoint.X, dataPoint.Y));
                }
            }
        }

        protected void RemoveVisualChildren(ICollection coll)
        {
            foreach (object obj in coll)
            {
                Darwin.Point point = obj as Darwin.Point;

                if (point != null)
                    RemoveVisualChild(point);
            }
        }
        
        protected void RemoveVisualChild(Darwin.Point point)
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
                            xform.X = sizeInfo.NewSize.Width * drawingVisual.DataPoint.X;

                        if (sizeInfo.HeightChanged)
                            xform.Y = sizeInfo.NewSize.Height * drawingVisual.DataPoint.Y;
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

        protected override void OnRender(DrawingContext dc)
        {
            //dc.DrawRectangle(Background, null, new Rect(RenderSize));
        }

        protected override void OnToolTipOpening(ToolTipEventArgs e)
        {
            HitTestResult result = VisualTreeHelper.HitTest(this, Mouse.GetPosition(this));

            if (result.VisualHit is DrawingVisualPlus)
            {
                DrawingVisualPlus drawingVisual = result.VisualHit as DrawingVisualPlus;
                Darwin.Point dataPoint = drawingVisual.DataPoint;
                ToolTip = String.Format("X={0}, Y={1}", dataPoint.X, dataPoint.Y);
            }
            base.OnToolTipOpening(e);
        }

        protected override void OnToolTipClosing(ToolTipEventArgs e)
        {
            ToolTip = string.Empty;
            base.OnToolTipClosing(e);
        }

        class DrawingVisualPlus : DrawingVisual
        {
            public Darwin.Point DataPoint { get; set; }
        }
    }
}
