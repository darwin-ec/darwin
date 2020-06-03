using Darwin.Collections;
using Darwin.Database;
using Darwin.Extensions;
using Darwin.ImageProcessing;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;

namespace Darwin.Helpers
{
    public static class ModificationHelper
    {
		/// <summary>
		/// Reapplies image modifications to an original.  TODO: Not factored correctly. TODO:Probably not efficient.
		/// </summary>
		/// <param name="modifications"></param>
		public static Bitmap ApplyImageModificationsToOriginal(Bitmap bitmap, List<Modification> modifications)
		{
			if (bitmap == null)
				throw new ArgumentNullException(nameof(bitmap));

			if (modifications == null)
				throw new ArgumentNullException(nameof(modifications));

			var mods = modifications
				.Where(x => x.ImageMod != null && (x.ModificationType == ModificationType.Image || x.ModificationType == ModificationType.Both))
				.Select(x => x.ImageMod)
				.ToList();

			if (mods == null || mods.Count < 1)
				return new Bitmap(bitmap);

			return ApplyImageModificationsToOriginal(bitmap, mods);
		}

		public static Bitmap ApplyImageModificationsToOriginal(Bitmap bitmap, List<ImageMod> mods)
        {
			if (bitmap == null)
				throw new ArgumentNullException(nameof(bitmap));

			if (mods == null)
				throw new ArgumentNullException(nameof(mods));

			Bitmap result = new Bitmap(bitmap);
			foreach (var mod in mods)
			{
				// TODO: This is really awkward
				ImageModType modType;
				int val1, val2, val3, val4;
				mod.Get(out modType, out val1, out val2, out val3, out val4);

				switch (mod.Op)
				{
					case ImageModType.IMG_flip:
						result.RotateFlip(RotateFlipType.RotateNoneFlipX);
						break;

					case ImageModType.IMG_brighten:
						result.AlterBrightness(val1);
						break;

					case ImageModType.IMG_contrast:
						result.EnhanceContrast((byte)val1, (byte)val2);
						break;

					case ImageModType.IMG_crop:
						var cropRect = new System.Drawing.Rectangle(val1, val2, val3 - val1, val4 - val2);
						result = ImageTransform.CropBitmap(result, cropRect);
						break;

					default:
						throw new NotImplementedException();
				}
			}

			return result;
		}
	}
}
