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

using Darwin.Extensions;
using Darwin.Helpers;
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

        public DirectBitmap(int width, int height, PixelFormat format)
        {
            Width = width;
            Height = height;

            _bitmap = new Bitmap(width, height, format);
            if (format == PixelFormat.Format8bppIndexed)
            {
                // There is no regular 8bpp grayscale in .NET, only 16 bit or 8 bit indexed.
                // So we're creating an 8bpp indexed and making the palette 0 -> 255 grayscale
                _bitmap.Palette = ColorExtensions.GetGrayscaleColorPalette(_bitmap);
            }

            IsLocked = false;
        }

        public DirectBitmap(Bitmap source)
        {
            if (source == null)
                throw new ArgumentNullException(nameof(source));

            Width = source.Width;
            Height = source.Height;
            IsLocked = false;

            if (source.PixelFormat != PixelFormat.Format8bppIndexed)
            {
                _bitmap = new Bitmap(source);
            }
            else
            {
                _bitmap = BitmapHelper.Copy8bppIndexed(source);
            }
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

        public void SetPixelByte(int x, int y, byte val)
        {
            if (!IsLocked)
                LockBits();

            if (BitsPerPixel != 8)
                throw new NotImplementedException();

            int i = (y * Stride) + x;
            _pixelData[i] = val;
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
        
        public float[] ToScaledRGBFloatArray()
        {
            float[] result = new float[Width * Height * 3];

            int idx = 0;
            for (int y = 0; y < Height; y++)
            {
                for (int x = 0; x < Width; x++)
                {
                    var pixel = GetPixel(x, y);
                    result[idx++] = pixel.R / 255.0f;
                    result[idx++] = pixel.G / 255.0f;
                    result[idx++] = pixel.B / 255.0f;
                }
            }

            return result;
        }

        public float[] ToScaledTensorFlowRGBPreprocessInput()
        {
            float[] result = new float[Width * Height * 3];

            int idx = 0;
            for (int y = 0; y < Height; y++)
            {
                for (int x = 0; x < Width; x++)
                {
                    var pixel = GetPixel(x, y);
                    result[idx++] = pixel.R / 127.5f - 1;
                    result[idx++] = pixel.G / 127.5f - 1;
                    result[idx++] = pixel.B / 127.5f - 1;
                }
            }

            return result;
        }

        /// <summary>
        /// Returns a scaled float array, as if this is a grayscale image with each byte
        /// divided by 255.0f
        /// </summary>
        /// <returns></returns>
        public float[] ToScaledGrayFloatArray()
        {
            float[] result = new float[Width * Height];

            int idx = 0;
            for (int y = 0; y < Height; y++)
            {
                for (int x = 0; x < Width; x++)
                {
                    result[idx++] = GetIntensity(x, y) / 255.0f;
                }
            }

            return result;
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
