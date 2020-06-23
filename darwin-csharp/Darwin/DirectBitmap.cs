using Darwin.Extensions;
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

        private int _bitsPerPixel;
        public int BitsPerPixel
        {
            get
            {
                return _bitsPerPixel;
            }
            private set
            {
                _bitsPerPixel = value;
                BytesPerPixel = _bitsPerPixel / 8;
            }
        }

        public int BytesPerPixel { get; private set; }

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
            // TODO: Different byte order for big endian?
            if (!IsLocked)
                LockBits();

            // Get start index of the specified pixel
            int i = (y * Stride) + x * BytesPerPixel;

            if (i > _pixelData.Length - BytesPerPixel)
                throw new IndexOutOfRangeException();

            if (BitsPerPixel == 32)
            {
                return Color.FromArgb(
                    _pixelData[i + 3],
                    _pixelData[i + 2],
                    _pixelData[i + 1],
                    _pixelData[i]);
            }
            
            if (BitsPerPixel == 24)
            {
                return Color.FromArgb(
                    _pixelData[i + 2],
                    _pixelData[i + 1],
                    _pixelData[i]);
            }
            
            if (BitsPerPixel == 8)
            {
                return Color.FromArgb(
                    _pixelData[i],
                    _pixelData[i],
                    _pixelData[i]);
            }

            throw new NotImplementedException();
        }

        public byte GetIntensity(int x, int y)
        {
            if (!IsLocked)
                LockBits();

            // Get start index of the specified pixel
            int i = (y * Stride) + x * BytesPerPixel;

            if (i > _pixelData.Length - BytesPerPixel)
                throw new IndexOutOfRangeException();

            if (BitsPerPixel == 32)
            {
                // Note: Ignoring Alpha (which is index i)
                return ColorExtensions.GetIntensity(_pixelData[i + 3], _pixelData[i + 2], _pixelData[i + 1]);
            }

            if (BitsPerPixel == 24)
            {
                return ColorExtensions.GetIntensity(_pixelData[i + 2], _pixelData[i + 1], _pixelData[i]);
            }

            if (BitsPerPixel == 8)
            {
                return _pixelData[i];
            }

            throw new NotImplementedException();
        }

        public void SetPixel(int x, int y, Color color)
        {
            // TODO: Different byte order for big endian?
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

            Width = _bitmap.Width;
            Height = _bitmap.Height;

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

            _pixelData = null;
            IsLocked = false;
        }
    }
}
