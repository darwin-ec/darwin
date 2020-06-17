using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace Darwin.Utilities
{
    public static class FileHelper
    {
        public const int MaxFilenameTries = 20;

        public static string FindUniqueFilename(string filename)
        {
            if (!File.Exists(filename))
                return filename;

            for (int i = 1; i <= MaxFilenameTries; i++)
            {
                var newFilenameTry = filename.Insert(filename.LastIndexOf("."), " (" + i.ToString() + ")");

                if (!File.Exists(newFilenameTry))
                    return newFilenameTry;                    
            }

            throw new Exception("Filename " + filename + " already exists!");
        }
    }
}
