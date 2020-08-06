// Original license CC BY-SA 3.0 https://creativecommons.org/licenses/by-sa/3.0/
// Based on the StackOverflow answer by Peter Duniho at
// https://stackoverflow.com/questions/5395957/wpf-slider-with-two-thumbs

// This file is part of DARWIN.
// Copyright (C) 1994 - 2020
//
// DARWIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DARWIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DARWIN.  If not, see<https://www.gnu.org/licenses/>.

using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Darwin.Wpf.Controls
{
    /// <summary>
    /// Interaction logic for TwoThumbSlider.xaml
    /// </summary>
    public partial class TwoThumbSlider : UserControl
    {
        public static readonly DependencyProperty MinimumProperty = DependencyProperty.Register("Minimum", typeof(double), typeof(TwoThumbSlider), new UIPropertyMetadata(0d));
        public static readonly DependencyProperty LowerValueProperty = DependencyProperty.Register("LowerValue", typeof(double), typeof(TwoThumbSlider), new UIPropertyMetadata(0d, null, LowerValueCoerceValueCallback));
        public static readonly DependencyProperty UpperValueProperty = DependencyProperty.Register("UpperValue", typeof(double), typeof(TwoThumbSlider), new UIPropertyMetadata(1d, null, UpperValueCoerceValueCallback));
        public static readonly DependencyProperty MaximumProperty = DependencyProperty.Register("Maximum", typeof(double), typeof(TwoThumbSlider), new UIPropertyMetadata(1d));
        public static readonly DependencyProperty IsSnapToTickEnabledProperty = DependencyProperty.Register("IsSnapToTickEnabled", typeof(bool), typeof(TwoThumbSlider), new UIPropertyMetadata(false));
        public static readonly DependencyProperty TickFrequencyProperty = DependencyProperty.Register("TickFrequency", typeof(double), typeof(TwoThumbSlider), new UIPropertyMetadata(0.1d));
        public static readonly DependencyProperty TickPlacementProperty = DependencyProperty.Register("TickPlacement", typeof(TickPlacement), typeof(TwoThumbSlider), new UIPropertyMetadata(TickPlacement.None));
        public static readonly DependencyProperty TicksProperty = DependencyProperty.Register("Ticks", typeof(DoubleCollection), typeof(TwoThumbSlider), new UIPropertyMetadata(null));

        public double Minimum
        {
            get { return (double)GetValue(MinimumProperty); }
            set { SetValue(MinimumProperty, value); }
        }

        public double LowerValue
        {
            get { return (double)GetValue(LowerValueProperty); }
            set { SetValue(LowerValueProperty, value); }
        }

        public double UpperValue
        {
            get { return (double)GetValue(UpperValueProperty); }
            set { SetValue(UpperValueProperty, value); }
        }

        public double Maximum
        {
            get { return (double)GetValue(MaximumProperty); }
            set { SetValue(MaximumProperty, value); }
        }

        public bool IsSnapToTickEnabled
        {
            get { return (bool)GetValue(IsSnapToTickEnabledProperty); }
            set { SetValue(IsSnapToTickEnabledProperty, value); }
        }

        public double TickFrequency
        {
            get { return (double)GetValue(TickFrequencyProperty); }
            set { SetValue(TickFrequencyProperty, value); }
        }

        public TickPlacement TickPlacement
        {
            get { return (TickPlacement)GetValue(TickPlacementProperty); }
            set { SetValue(TickPlacementProperty, value); }
        }

        public DoubleCollection Ticks
        {
            get { return (DoubleCollection)GetValue(TicksProperty); }
            set { SetValue(TicksProperty, value); }
        }

        public static readonly RoutedEvent ValueChangedEvent = EventManager.RegisterRoutedEvent("ValueChangedEvent", RoutingStrategy.Bubble, typeof(RoutedPropertyChangedEventHandler<double>), typeof(TwoThumbSlider));
        public event RoutedPropertyChangedEventHandler<double> ValueChanged
        {
            add { AddHandler(ValueChangedEvent, value); }
            remove { RemoveHandler(ValueChangedEvent, value); }
        }

        public TwoThumbSlider()
        {
            InitializeComponent();
        }

        private static object LowerValueCoerceValueCallback(DependencyObject target, object valueObject)
        {
            TwoThumbSlider targetSlider = (TwoThumbSlider)target;
            double value = (double)valueObject;

            return Math.Min(value, targetSlider.UpperValue);
        }

        private static object UpperValueCoerceValueCallback(DependencyObject target, object valueObject)
        {
            TwoThumbSlider targetSlider = (TwoThumbSlider)target;
            double value = (double)valueObject;
            
            return Math.Max(value, targetSlider.LowerValue);
        }

        private void upperSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            RoutedPropertyChangedEventArgs<double> args = new RoutedPropertyChangedEventArgs<double>(e.OldValue, e.NewValue, ValueChangedEvent);
            RaiseEvent(args);
        }

        private void lowerSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            RoutedPropertyChangedEventArgs<double> args = new RoutedPropertyChangedEventArgs<double>(e.OldValue, e.NewValue, ValueChangedEvent);
            RaiseEvent(args);
        }
    }
}
