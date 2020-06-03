// Based on
// ScatterPlotRender.cs by Charles Petzold, December 2008
// https://docs.microsoft.com/en-us/archive/msdn-magazine/2009/march/foundations-writing-more-efficient-itemscontrols
using Darwin.Collections;
using System;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace Darwin.Wpf.FrameworkElements
{
    public class PointRender : FrameworkElement
    {
        public static readonly DependencyProperty ItemsSourceProperty =
            DependencyProperty.Register("ItemsSource",
                typeof(ObservableNotifiableCollection<Darwin.Point>),
                typeof(PointRender),
                new PropertyMetadata(OnItemsSourceChanged));

        public static readonly DependencyProperty BrushesProperty =
            DependencyProperty.Register("Brushes",
                typeof(Brush[]),
                typeof(PointRender),
                new FrameworkPropertyMetadata(null,
                        FrameworkPropertyMetadataOptions.AffectsRender));

        public static readonly DependencyProperty PointSizeProperty =
            DependencyProperty.Register("PointSize",
                typeof(double),
                typeof(PointVisuals),
                new FrameworkPropertyMetadata(2.0,
                        FrameworkPropertyMetadataOptions.AffectsRender));

        public static readonly DependencyProperty BackgroundProperty =
            Panel.BackgroundProperty.AddOwner(typeof(PointRender));

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

        public Brush Background
        {
            set { SetValue(BackgroundProperty, value); }
            get { return (Brush)GetValue(BackgroundProperty); }
        }

        public double PointSize
        {
            set { SetValue(PointSizeProperty, value); }
            get { return (double)GetValue(PointSizeProperty); }
        }

        static void OnItemsSourceChanged(DependencyObject obj, DependencyPropertyChangedEventArgs args)
        {
            var render = obj as PointRender;
            
            if (render != null)
                render.OnItemsSourceChanged(args);
        }

        void OnItemsSourceChanged(DependencyPropertyChangedEventArgs args)
        {
            if (args.OldValue != null)
            {
                ObservableNotifiableCollection<Darwin.Point> coll = args.OldValue as ObservableNotifiableCollection<Darwin.Point>;
                coll.CollectionChanged -= OnCollectionChanged;
                coll.ItemPropertyChanged -= OnItemPropertyChanged;
            }

            if (args.NewValue != null)
            {
                ObservableNotifiableCollection<Darwin.Point> coll = args.NewValue as ObservableNotifiableCollection<Darwin.Point>;
                coll.CollectionChanged += OnCollectionChanged;
                coll.ItemPropertyChanged += OnItemPropertyChanged;
            }

            InvalidateVisual();
        }

        void OnCollectionChanged(object sender, NotifyCollectionChangedEventArgs args)
        {
            InvalidateVisual();
        }

        void OnItemPropertyChanged(object sender, ItemPropertyChangedEventArgs args)
        {
            InvalidateVisual();
        }

        protected override void OnRender(DrawingContext dc)
        {
            if (Background != System.Windows.Media.Brushes.Transparent)
                dc.DrawRectangle(Background, null, new Rect(RenderSize));

            if (ItemsSource == null || Brushes == null)
                return;

            foreach (Darwin.Point dataPoint in ItemsSource)
            {
                dc.DrawEllipse(Brushes[0], null,
                    new System.Windows.Point(dataPoint.X, dataPoint.Y), PointSize, PointSize);
            }
        }
    }
}
