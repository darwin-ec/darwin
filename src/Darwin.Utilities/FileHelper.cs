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
