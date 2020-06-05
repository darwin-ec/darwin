﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Darwin
{
    public class PointF : INotifyPropertyChanged
    {
        public static readonly PointF Empty = new PointF { IsEmpty = true };
        public event PropertyChangedEventHandler PropertyChanged;

        private float x;
        private float y;
        private float z;
        private PointType type;

        public bool IsEmpty { get; set; }

        public float X
        {
            get => x;
            set
            {
                IsEmpty = false;
                x = value;
                OnPropertyChanged("X");
            }
        }

        public float Y
        {
            get => y;
            set
            {
                IsEmpty = false;
                y = value;
                OnPropertyChanged("Y");
            }
        }

        public float Z
        {
            get => z;
            set
            {
                IsEmpty = false;
                z = value;
                OnPropertyChanged("Z");
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

        public PointF()
        {

        }

        public PointF(float x, float y)
        {
            X = x;
            Y = y;
        }

        public PointF(double x, double y)
        {
            X = (float)x;
            Y = (float)y;
        }

        public PointF(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public double Dot2D(PointF point2)
        {
            return X * point2.X + Y * point2.Y;
        }

        public PointF Multiply(double t)
        {
            return new PointF
            {
                X = (float)(this.X * t),
                Y = (float)(this.Y * t),
                Z = (float)(this.Z * t)
            };
        }

        public double Norm2D()
        {
            var selfDot = Dot2D(this);
            return Math.Sqrt(selfDot);
        }

        public PointF Subtract(PointF point2)
        {
            return new PointF
            {
                X = this.X - point2.X,
                Y = this.Y - point2.Y,
                Z = this.Z - point2.Z
            };
        }

        public PointF Add(PointF point2)
        {
            return new PointF
            {
                X = this.X + point2.X,
                Y = this.Y + point2.Y,
                Z = this.Z + point2.Z
            };
        }

        public static bool operator ==(Darwin.PointF p1, Darwin.PointF p2)
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

            return p1.X == p2.X && p1.Y == p2.Y && p1.Z == p2.Z;
        }

        public static bool operator !=(Darwin.PointF p1, Darwin.PointF p2)
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

            return p1.X != p2.X || p1.Y != p2.Y || p1.Z != p2.Z;
        }

        // this is third one 'Equals'
        public override bool Equals(object obj)
        {
            var p = obj as Darwin.PointF;

            return p != null && p.X == X && p.Y == Y && p.Z == Z;
        }

        public override int GetHashCode()
        {
            unchecked // Overflow is fine, just wrap
            {
                int hash = 17;
                // Suitable nullity checks etc, of course :)
                hash = hash * 23 + X.GetHashCode();
                hash = hash * 23 + Y.GetHashCode();
                hash = hash * 23 + Z.GetHashCode();
                return hash;
            }
        }

        protected virtual void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}