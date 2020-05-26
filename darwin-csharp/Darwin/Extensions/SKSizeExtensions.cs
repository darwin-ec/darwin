using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Darwin.Extensions
{
    public static class SKSizeExtensions
    {
        //public static SKSize ResizeKeepAspectRatio(this SKSize size, float maxWidth, float maxHeight, out float ratioUsed)
        //{
        //    // Figure out the resize ratios
        //    float ratioX = maxWidth / size.Width;
        //    float ratioY = maxHeight / size.Height;

        //    // Use whichever multiplier is smaller
        //    ratioUsed = ratioX < ratioY ? ratioX : ratioY;

        //    // Now we can get the new height and width
        //    return new SKSize(size.Width * ratioUsed, size.Height * ratioUsed);
        //}

        //public static SKSize ResizeKeepAspectRatio(this SKSize size, float maxWidth, float maxHeight)
        //{
        //    float ratio;
        //    return ResizeKeepAspectRatio(size, maxWidth, maxHeight, out ratio);
        //}
    }
}
