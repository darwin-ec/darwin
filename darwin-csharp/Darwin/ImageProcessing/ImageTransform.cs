using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;

namespace Darwin.ImageProcessing
{
    public static class ImageTransform
    {
        public static Bitmap CropBitmap(Bitmap bitmap, Rectangle cropRect)
        {
			if (bitmap == null)
				throw new ArgumentNullException(nameof(bitmap));

			Bitmap croppedBitmap = new Bitmap(cropRect.Width, cropRect.Height);

			using (Graphics g = Graphics.FromImage(croppedBitmap))
			{
				g.DrawImage(bitmap, new Rectangle(0, 0, croppedBitmap.Width, croppedBitmap.Height),
								 cropRect,
								 GraphicsUnit.Pixel);
			}

			return croppedBitmap;
		}
    }
}
