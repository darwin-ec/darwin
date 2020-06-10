//*******************************************************************
//   file: IntensityContour.cxx
//
// author: Scott Hale
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

using Darwin.Extensions;
using System.Drawing;

namespace Darwin
{
    public class IntensityContourCyan : IntensityContour
    {
        public IntensityContourCyan(Bitmap bitmap, Contour contour,
            int left, int top, int right, int bottom)
            : base(bitmap, contour, left, top, right, bottom)
        {
            DirectBitmap cyanImage = new DirectBitmap(bitmap);
            cyanImage.ToCyanIntensity();
            GetPointsFromBitmap(ref cyanImage, contour, left, top, right, bottom);
        }
    }
}
