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
using System.ComponentModel;
using System.Text;

namespace Darwin
{
    public enum PointType
    {
        Normal = 0,
        Feature = 1,
        Moving = 2,
        Chopping = 3,
        FeatureMoving = 4,
        Flagged = 99
    }

    public class Point : INotifyPropertyChanged
    {
        public static readonly Point Empty = new Point { IsEmpty = true };

        private int x;
        private int y;
        private PointType type;

        public bool IsEmpty { get; set; }

        public int X
        {
            get => x;
            set
            {
                IsEmpty = false;
                x = value;
                RaisePropertyChanged("X");
            }
        }

        public int Y
        {
            get => y;
            set
            {
                IsEmpty = false;
                y = value;
                RaisePropertyChanged("Y");
            }
        }

        public PointType Type
        {
            set
            {
                if (type != value)
                {
                    IsEmpty = false;
                    type = value;
                    RaisePropertyChanged("Type");
                }
            }
            get { return type; }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public Point()
        {
            IsEmpty = true;
        }

        public Point(int x, int y)
        {
            X = x;
            Y = y;
            IsEmpty = false;
        }

        public Point(int x, int y, PointType type)
        {
            X = x;
            Y = y;
            Type = type;
            IsEmpty = false;
        }

        public static bool operator ==(Darwin.Point p1, Darwin.Point p2)
        {
            // If both are null, or both are same instance, return true.
            if (System.Object.ReferenceEquals(p1, p2))
            {
                return true;
            }

            // If one is null, but not both, return false.
            if (((object)p1 == null) || ((object)p2 == null))
            {
                return false;
            }

            return p1.X == p2.X && p1.Y == p2.Y;
        }

        public static bool operator !=(Darwin.Point p1, Darwin.Point p2)
        {
            // If both are null, or both are same instance, return false.
            if (System.Object.ReferenceEquals(p1, p2))
            {
                return false;
            }

            // If one is null, but not both, return true.
            if (((object)p1 == null) || ((object)p2 == null))
            {
                return true;
            }

            return p1.X != p2.X || p1.Y != p2.Y;
        }

        // this is third one 'Equals'
        public override bool Equals(object obj)
        {
            var p = obj as Darwin.Point;

            return p != null && p.X == X && p.Y == Y;
        }

        public override int GetHashCode()
        {
            return x ^ y;
        }

        public void SetPosition(int x, int y)
        {
            X = x;
            Y = y;
        }

        protected virtual void RaisePropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
