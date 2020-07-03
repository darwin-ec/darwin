using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Utilities
{
    public static class StringExtensions
    {
        public static string ToFirstCharacterUpper(this string s)
        {
            if (string.IsNullOrEmpty(s))
                return string.Empty;

            if (s.Length == 1)
                return s.ToUpper();

            return char.ToUpper(s[0]) + s.Substring(1);
        }
    }
}
