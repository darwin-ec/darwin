//*******************************************************************
//   file: ImageMod.cxx
//
// author: J H Stewman (1/24/2007)
//
//   mods: 
//
// contains classes for keeping track of applied image modifications
//
// used in TraceWindow to build list of modifications applied to
// original image in preparation for tracing, or to reproduce
// same sequence when loading a previsously traced and saved fin
//
// used in MatchResultsWindow when loading results or changing 
// selected fin, so that the modified image can be recreated from
// the original for both selected and unknown fins
//
//*******************************************************************

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace Darwin.Database
{
    public enum ImageModType
    {
        IMG_none,
        IMG_flip,
        IMG_contrast,
        IMG_brighten,
        IMG_crop,
        IMG_undo,
        IMG_redo
    }

    public class ImageMod
    {
        public ImageModType Op { get; set; }              // image modification type
        private int
            min, max,        // values used in contrast modification
            amount,          // amount adjusted +/- for brightness adjustment
            xMin, yMin,      // boundaries for cropping
            xMax, yMax;      // ditto

        // the values are used depending on the ImageModtype
        // op == IMAG_flip, no values used
        // op == IMG_contrast, min is val1, and max is val2
        // op == IMG_brighten, amount is val1
        // op == IMG_crop, xMin is val1, yMin is val2, xMax is val3, yMax is val4
        // op == IMG_undo, no values used
        // op == IMG_redo, no values used
        public ImageMod(ImageModType op, int val1 = 0, int val2 = 0, int val3 = 0, int val4 = 0)
        {
            // the values are used depending on the ImageModtype
            Op = op;
            if ((ImageModType.IMG_flip == op) || (ImageModType.IMG_undo == op) || (ImageModType.IMG_redo == op) || (ImageModType.IMG_none == op))
            {
                // op == IMAG_flip, IMG_undo, or IMG_redo ... then no values used
            }
            else if (ImageModType.IMG_contrast == op)
            {
                // op == IMG_contrast, min is val1, and max is val2
                min = val1;
                max = val2;
            }
            else if (ImageModType.IMG_brighten == op)
            {
                // op == IMG_brighten, amount is val1
                amount = val1;
            }
            else if (ImageModType.IMG_crop == op)
            {
                // op == IMG_crop, xMin is val1, yMin is val2, xMax is val3, yMax is val4
                xMin = val1;
                yMin = val2;
                xMax = val3;
                yMax = val4;
            }
            else
                Op = ImageModType.IMG_none;
        }


        // the values are used depending on the ImageModtype
        // op == IMAG_flip, no values used
        // op == IMG_contrast, min is val1, and max is val2
        // op == IMG_brighten, amount is val1
        // op == IMG_crop, xMin is val1, yMin is val2, xMax is val3, yMax is val4
        // op == IMG_undo, no values used
        // op == IMG_redo, no values used
        public void Set(ImageModType op, int val1 = 0, int val2 = 0, int val3 = 0, int val4 = 0)
        {
            // the values are used depending on the ImageModtype
            Op = op;
            if ((ImageModType.IMG_flip == op) || (ImageModType.IMG_undo == op) || (ImageModType.IMG_redo == op))
            {
                // op == IMAG_flip, IMG_undo, or IMG_redo ... then no values used
                min = max = amount = xMin = yMin = xMax = yMax = 0;
            }
            else if (ImageModType.IMG_contrast == op)
            {
                // op == IMG_contrast, min is val1, and max is val2
                min = val1;
                max = val2;
                amount = xMin = yMin = xMax = yMax = 0;
            }
            else if (ImageModType.IMG_brighten == op)
            {
                // op == IMG_brighten, amount is val1
                amount = val1;
                min = max = xMin = yMin = xMax = yMax = 0;
            }
            else if (ImageModType.IMG_crop == op)
            {
                // op == IMG_crop, xMin is val1, yMin is val2, xMax is val3, yMax is val4
                xMin = val1;
                yMin = val2;
                xMax = val3;
                yMax = val4;
                min = max = amount = 0;
            }
            else
                Op = ImageModType.IMG_none;
        }

        // the values are used depending on the ImageModtype
        // op == IMAG_flip, no values used
        // op == IMG_contrast, min is val1, and max is val2
        // op == IMG_brighten, amount is val1
        // op == IMG_crop, xMin is val1, yMin is val2, xMax is val3, yMax is val4
        // op == IMG_undo, no values used
        // op == IMG_redo, no values used
        public void Get(out ImageModType op, out int val1, out int val2, out int val3, out int val4)

        {
            val1 = val2 = val3 = val4 = 0;
            // the values are used depending on the ImageModtype
            op = Op;
            if ((ImageModType.IMG_flip == op) || (ImageModType.IMG_undo == op) || (ImageModType.IMG_redo == op) || (ImageModType.IMG_none == op))
            {
                // op == IMAG_flip, IMG_undo, or IMG_redo ... then no values used
                val1 = val2 = val3 = val4 = 0;
            }
            else if (ImageModType.IMG_contrast == op)
            {
                // op == IMG_contrast, min is val1, and max is val2
                val1 = min;
                val2 = max;
                val3 = val4 = 0;
            }
            else if (ImageModType.IMG_brighten == op)
            {
                // op == IMG_brighten, amount is val1
                val1 = amount;
                val2 = val3 = val4 = 0;
            }
            else if (ImageModType.IMG_crop == op)
            {
                // op == IMG_crop, xMin is val1, yMin is val2, xMax is val3, yMax is val4
                val1 = xMin;
                val2 = yMin;
                val3 = xMax;
                val4 = yMax;
            }
            else
                Trace.WriteLine("error in modList::get()"); // shouldn't get here
        }
    }
}
