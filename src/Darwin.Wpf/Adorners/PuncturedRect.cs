// Based on code by darrellp at https://www.codeproject.com/Articles/23158/A-Photoshop-like-Cropping-Adorner-for-WPF 
// Original license Code Project Open License: https://www.codeproject.com/info/cpol10.aspx

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
using System.Windows;
using System.Windows.Media;
using System.Windows.Shapes;

namespace Darwin.Wpf.Adorners
{
    public class PuncturedRect : Shape
	{
		#region Dependency properties
		public static readonly DependencyProperty RectInteriorProperty =
			DependencyProperty.Register(
				"RectInterior",
				typeof(Rect),
				typeof(FrameworkElement),
				new FrameworkPropertyMetadata(
					new Rect(0, 0, 0, 0),
					FrameworkPropertyMetadataOptions.AffectsRender,
					null,
					new CoerceValueCallback(CoerceRectInterior),
					false
				),
				null
			);

		private static object CoerceRectInterior(DependencyObject d, object value)
		{
			PuncturedRect pr = (PuncturedRect)d;
			Rect rcExterior = pr.RectExterior;
			Rect rcProposed = (Rect)value;
			double left = Math.Max(rcProposed.Left, rcExterior.Left);
			double top = Math.Max(rcProposed.Top, rcExterior.Top);
			double width = Math.Min(rcProposed.Right, rcExterior.Right) - left;
			double height = Math.Min(rcProposed.Bottom, rcExterior.Bottom) - top;

			if (width < 0 || height < 0)
				return pr.RectInterior;

			rcProposed = new Rect(left, top, width, height);
			return rcProposed;
		}

		public Rect RectInterior
		{
			get { return (Rect)GetValue(RectInteriorProperty); }
			set { SetValue(RectInteriorProperty, value); }
		}

		public static readonly DependencyProperty RectExteriorProperty =
			DependencyProperty.Register(
				"RectExterior",
				typeof(Rect),
				typeof(FrameworkElement),
				new FrameworkPropertyMetadata(
					new Rect(0, 0, double.MaxValue, double.MaxValue),
					FrameworkPropertyMetadataOptions.AffectsMeasure |
					FrameworkPropertyMetadataOptions.AffectsArrange |
					FrameworkPropertyMetadataOptions.AffectsParentMeasure |
					FrameworkPropertyMetadataOptions.AffectsParentArrange |
					FrameworkPropertyMetadataOptions.AffectsRender,
					null,
					null,
					false
				),
				null
			);

		public Rect RectExterior
		{
			get { return (Rect)GetValue(RectExteriorProperty); }
			set { SetValue(RectExteriorProperty, value); }
		}
		#endregion

		#region Constructors
		public PuncturedRect() : this(new Rect(0, 0, double.MaxValue, double.MaxValue), new Rect()) { }

		public PuncturedRect(Rect rectExterior, Rect rectInterior)
		{
			RectInterior = rectInterior;
			RectExterior = rectExterior;
		}
		#endregion

		#region Geometry
		protected override Geometry DefiningGeometry
		{
			get
			{
				PathGeometry pthgExt = new PathGeometry();
				PathFigure pthfExt = new PathFigure();
				pthfExt.StartPoint = RectExterior.TopLeft;
				pthfExt.Segments.Add(new LineSegment(RectExterior.TopRight, false));
				pthfExt.Segments.Add(new LineSegment(RectExterior.BottomRight, false));
				pthfExt.Segments.Add(new LineSegment(RectExterior.BottomLeft, false));
				pthfExt.Segments.Add(new LineSegment(RectExterior.TopLeft, false));
				pthgExt.Figures.Add(pthfExt);

				Rect rectIntSect = Rect.Intersect(RectExterior, RectInterior);
				PathGeometry pthgInt = new PathGeometry();
				PathFigure pthfInt = new PathFigure();
				pthfInt.StartPoint = rectIntSect.TopLeft;
				pthfInt.Segments.Add(new LineSegment(rectIntSect.TopRight, false));
				pthfInt.Segments.Add(new LineSegment(rectIntSect.BottomRight, false));
				pthfInt.Segments.Add(new LineSegment(rectIntSect.BottomLeft, false));
				pthfInt.Segments.Add(new LineSegment(rectIntSect.TopLeft, false));
				pthgInt.Figures.Add(pthfInt);

				CombinedGeometry cmbg = new CombinedGeometry(GeometryCombineMode.Exclude, pthgExt, pthgInt);
				return cmbg;
			}
		}
		#endregion
	}
}
