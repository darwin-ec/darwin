//*******************************************************************
//   file: IntensityContour.cxx
//
// author: Scott Hale
//
//   mods: J H Stewman (1/21/2008)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin
{
    public class IntensityContourCyan : IntensityContour
    {
        public IntensityContourCyan(Bitmap bitmap, Contour contour,
            int left, int top, int right, int bottom)
            : base(bitmap, contour, left, top, right, bottom)
        {
            // TODO
            throw new NotImplementedException();
        }
    }
}
