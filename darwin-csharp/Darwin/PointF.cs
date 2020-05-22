using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Darwin
{
    // TODO: Make we should make the properties nullable?
    public class PointF
    {
        public float X { get; set; }
        public float Y { get; set; }
        public float Z { get; set; }

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
    }
}
