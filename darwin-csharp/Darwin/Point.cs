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
        Moving = 2
    }

    public class Point : INotifyPropertyChanged
    {
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
                OnPropertyChanged("X");
            }
        }

        public int Y
        {
            get => y;
            set
            {
                IsEmpty = false;
                y = value;
                OnPropertyChanged("Y");
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
                    OnPropertyChanged("Type");
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

        // this is first one '=='
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

        // this is second one '!='
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

        public void SetPosition(int x, int y)
        {
            X = x;
            Y = y;
        }

        protected virtual void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
