using Darwin.Database;
using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin
{
    public enum ModificationType
    {
        Image,
        Contour,
        Both
    }

    public class Modification
    {
        public ModificationType ModificationType { get; set; }
        public ImageMod ImageMod { get; set; }
        public Contour Contour { get; set; }
    }
}
