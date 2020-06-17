using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace Darwin.Extensions
{
    public static class StringExtensions
    {
        public static string StripCRLFTab(this string s)
        {
            return Regex.Replace(s, @"\t|\n|\r", "");
        }
    }
}
