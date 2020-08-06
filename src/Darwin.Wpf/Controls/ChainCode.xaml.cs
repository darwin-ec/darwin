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
    /// Interaction logic for ChainCode.xaml
    /// </summary>
    public partial class ChainCode : UserControl
    {
        public static readonly DependencyProperty ChainProperty =
            DependencyProperty.Register("Chain",
                typeof(Chain),
                typeof(ChainCode),
                new PropertyMetadata(OnChainChanged));

        public static readonly DependencyProperty BrushProperty =
            DependencyProperty.Register("Brush",
                typeof(Brush),
                typeof(ChainCode),
                new UIPropertyMetadata(null));

        public static readonly DependencyProperty StrokeThicknessProperty =
            DependencyProperty.Register("StrokeThickness",
                typeof(double),
                typeof(ChainCode),
                new FrameworkPropertyMetadata(1.0,
                        FrameworkPropertyMetadataOptions.AffectsRender));

        public Chain Chain
        {
            set { SetValue(ChainProperty, value); }
            get
            {
                return GetValue(ChainProperty) as Chain;
            }
        }

        public Brush Brush
        {
            set { SetValue(BrushProperty, value); }
            get { return GetValue(BrushProperty) as Brush; }
        }

        public double StrokeThickness
        {
            set { SetValue(StrokeThicknessProperty, value); }
            get { return (double)GetValue(StrokeThicknessProperty); }
        }

        public ChainCode()
        {
            InitializeComponent();

            DrawChainCode();
        }

        static void OnChainChanged(DependencyObject obj, DependencyPropertyChangedEventArgs args)
        {
            (obj as ChainCode).OnChainChanged(args);
        }

        protected void OnChainChanged(DependencyPropertyChangedEventArgs args)
        {
            DrawChainCode();
        }

        protected override void OnRenderSizeChanged(SizeChangedInfo sizeInfo)
        {
            DrawChainCode();
            base.OnRenderSizeChanged(sizeInfo);
        }

        private void DrawChainCode()
        {
            ChainCanvas.Children.Clear();

            if (Chain == null || ChainCanvas.ActualWidth == 0 || ChainCanvas.ActualHeight == 0)
                return;

            double
                yMin = Chain.Min(), //***008OL
                yMax = Chain.Max(); //***008OL

            double vertRatio, angle;

            if (Math.Abs(yMax) > Math.Abs(yMin))
            {
                vertRatio = (double)ChainCanvas.ActualHeight / (Math.Abs(yMax) * 2);
                angle = Math.Abs(yMax);
            }
            else
            {
                vertRatio = (double)ChainCanvas.ActualHeight / (Math.Abs(yMin) * 2);
                angle = Math.Abs(yMin);
            }

            double horizRatio = ((double)ChainCanvas.ActualWidth) / Chain.Length;

            int
                prevXCoord = 0,
                prevYCoord = (int)Math.Round(Math.Abs(Chain[0] - angle) * vertRatio); //***008OL

            for (int i = 1; i < Chain.Length; i++)
            {    //***008OL
                int xCoord = (int)Math.Round(i * horizRatio);
                int yCoord = (int)Math.Round(Math.Abs(Chain[i] - angle) * vertRatio); //***008OL

                Line line = new Line
                {
                    Stroke = Brush,
                    StrokeThickness = StrokeThickness,
                    X1 = prevXCoord,
                    Y1 = prevYCoord,
                    X2 = xCoord,
                    Y2 = yCoord,
                    SnapsToDevicePixels = true
                };
                line.SetValue(RenderOptions.EdgeModeProperty, EdgeMode.Aliased);
                ChainCanvas.Children.Add(line);

                prevXCoord = xCoord;
                prevYCoord = yCoord;
            }
        }
    }
}
