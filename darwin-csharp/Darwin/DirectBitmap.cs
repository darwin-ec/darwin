// Modified Disposal, Constructor, etc./


// Based on the StackOverflow answer by A. Konzel at
// https://stackoverflow.com/questions/24701703/c-sharp-faster-alternatives-to-setpixel-and-getpixel-for-bitmaps-for-windows-f
// Original license CC BY-SA 3.0 https://creativecommons.org/licenses/by-sa/3.0/

using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace Darwin
{
    public class DirectBitmap : IDisposable
    {
        public Bitmap Bitmap { get; private set; }
        public int[] Bits { get; private set; }
        public int Height { get; private set; }
        public int Width { get; private set; }

        protected GCHandle BitsHandle { get; private set; }
        private bool isDisposed;

        public DirectBitmap(int width, int height)
        {
            isDisposed = false;
            Width = width;
            Height = height;
            Bits = new Int32[width * height];
            BitsHandle = GCHandle.Alloc(Bits, GCHandleType.Pinned);
            Bitmap = new Bitmap(width, height, width * 4, PixelFormat.Format32bppPArgb, BitsHandle.AddrOfPinnedObject());
        }

        public DirectBitmap(Bitmap bitmap)
        {
            if (bitmap == null)
                throw new ArgumentNullException(nameof(bitmap));

            isDisposed = false;
            Width = bitmap.Width;
            Height = bitmap.Height;
            Bits = new Int32[bitmap.Width * bitmap.Height];
            BitsHandle = GCHandle.Alloc(Bits, GCHandleType.Pinned);
            Bitmap = new Bitmap(Height, Width, Width * 4, PixelFormat.Format32bppPArgb, BitsHandle.AddrOfPinnedObject());

            using (var g = Graphics.FromImage(Bitmap))
            {
                g.DrawImage(bitmap,
                    new Rectangle(0, 0, Width, Height));
            }
        }

        public void SetPixel(int x, int y, Color colour)
        {
            int index = x + (y * Width);
            int col = colour.ToArgb();

            Bits[index] = col;
        }

        public Color GetPixel(int x, int y)
        {
            int index = x + (y * Width);
            int col = Bits[index];
            Color result = Color.FromArgb(col);

            return result;
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (isDisposed)
                return;

            if (disposing)
            {
                Bitmap.Dispose();
                BitsHandle.Free();
            }

            isDisposed = true;
        }

        ~DirectBitmap()
        {
            Dispose(false);
        }
    }
}
