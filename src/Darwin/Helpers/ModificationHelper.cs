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
				// TODO: This is a little awkward
				ImageModType modType;
				int val1, val2, val3, val4;
				mod.Get(out modType, out val1, out val2, out val3, out val4);

				switch (mod.Op)
				{
					case ImageModType.IMG_flip:
						result.RotateFlip(RotateFlipType.RotateNoneFlipX);
						break;

					case ImageModType.IMG_rotate90cw:
						result.RotateFlip(RotateFlipType.Rotate90FlipNone);
						break;

					case ImageModType.IMG_rotate90ccw:
						result.RotateFlip(RotateFlipType.Rotate270FlipNone);
						break;

					case ImageModType.IMG_brighten:
						result = result.AlterBrightness(val1);
						break;

					case ImageModType.IMG_contrast:
						result = result.EnhanceContrastMinMax((byte)val1, (byte)val2);
						break;

					case ImageModType.IMG_contrast2:
						result = result.EnhanceContrast(val1);
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
