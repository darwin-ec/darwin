using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace Darwin
{
    public class DirectBitmap
    {
        private Bitmap _bitmap;
        public Bitmap Bitmap
        {
            get
            {
                if (IsLocked)
                    UnlockBits();

                return _bitmap;
            }

            set
            {
                if (IsLocked)
                    UnlockBits();

                _bitmap = value;

                Width = _bitmap.Width;
                Height = _bitmap.Height;
            }
        }

        public Bitmap RawBitmap
        {
            get
            {
                return _bitmap;
            }
        }

        public int Stride { get; private set; }
        public int Width { get; private set; }
        public int Height { get; private set; }
        public int BitsPerPixel { get; private set; }
        public int BytesPerPixel
        {
            get
            {
                return BitsPerPixel / 8;
            }
        }

        public bool IsLocked { get; private set; }

        private byte[] _pixelData { get; set; }
        IntPtr _dataPtr;
        BitmapData _bitmapData;

        public DirectBitmap(int width, int height)
        {
            Width = width;
            Height = height;
            _bitmap = new Bitmap(width, height, PixelFormat.Format24bppRgb);
            IsLocked = false;
        }

        public DirectBitmap(Bitmap source)
        {
            _bitmap = new Bitmap(source);
            Width = _bitmap.Width;
            Height = _bitmap.Height;
            IsLocked = false;
        }

        public DirectBitmap(DirectBitmap source)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            if (source.Bitmap == null)
                throw new ArgumentNullException("source.Bitmap");

            _bitmap = new Bitmap(source.Bitmap);
            Width = _bitmap.Width;
            Height = _bitmap.Height;
            IsLocked = false;
        }

        public Color GetPixel(int x, int y)
        {
            if (!IsLocked && _pixelData == null && _bitmap != null)
                LockBits();

            // Get start index of the specified pixel
            int i = (y * Stride) + x * BytesPerPixel;

            if (i > _pixelData.Length - BytesPerPixel)
                throw new IndexOutOfRangeException();

            if (BitsPerPixel == 32)
            {
                byte b = _pixelData[i];
                byte g = _pixelData[i + 1];
                byte r = _pixelData[i + 2];
                byte a = _pixelData[i + 3];
                return Color.FromArgb(a, r, g, b);
            }
            
            if (BitsPerPixel == 24)
            {
                byte b = _pixelData[i];
                byte g = _pixelData[i + 1];
                byte r = _pixelData[i + 2];
                return Color.FromArgb(r, g, b);
            }
            
            if (BitsPerPixel == 8)
            {
                byte c = _pixelData[i];
                return Color.FromArgb(c, c, c);
            }

            throw new NotImplementedException();
        }

        public void SetPixel(int x, int y, Color color)
        {
            if (!IsLocked)
                LockBits();

            int i = (y * Stride) + x * BytesPerPixel;

            if (BitsPerPixel == 32)
            {
                _pixelData[i] = color.B;
                _pixelData[i + 1] = color.G;
                _pixelData[i + 2] = color.R;
                _pixelData[i + 3] = color.A;
            }
            else if (BitsPerPixel == 24)
            {
                _pixelData[i] = color.B;
                _pixelData[i + 1] = color.G;
                _pixelData[i + 2] = color.R;
            }
            else if (BitsPerPixel == 8)
            {
                _pixelData[i] = color.B;
            }
        }

        private void LockBits()
        {
            if (IsLocked)
                return;

            BitsPerPixel = Image.GetPixelFormatSize(_bitmap.PixelFormat);

            if (BitsPerPixel != 8 && BitsPerPixel != 24 && BitsPerPixel != 32)
                throw new NotImplementedException("Unsupported color depth");

            // Lock bitmap and return bitmap data
            _bitmapData = _bitmap.LockBits(new Rectangle(0, 0, Width, Height),
                ImageLockMode.ReadWrite,
                _bitmap.PixelFormat);

            Stride = Math.Abs(_bitmapData.Stride);

            int bytes = Math.Abs(Stride) * Height;
            _pixelData = new byte[bytes];

            _dataPtr = _bitmapData.Scan0;

            Marshal.Copy(_dataPtr, _pixelData, 0, _pixelData.Length);

            IsLocked = true;
        }

        private void UnlockBits()
        {
            if (!IsLocked)
                return;

            Marshal.Copy(_pixelData, 0, _dataPtr, _pixelData.Length);

            _bitmap.UnlockBits(_bitmapData);

            IsLocked = false;
        }
    }
}
