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
using System.Data.SQLite;
using System.Globalization;
using System.Text;

namespace Darwin.Utilities
{
    public static class SQLiteReaderExtensions
    {
        public static int SafeGetInt(this SQLiteDataReader rdr, string columnName)
        {
            var colIndex = rdr.GetOrdinal(columnName);

            if (rdr.IsDBNull(colIndex))
                return default(int);

            return Convert.ToInt32(rdr[colIndex]);
        }

        public static long SafeGetInt64(this SQLiteDataReader rdr, string columnName)
        {
            var colIndex = rdr.GetOrdinal(columnName);

            if (rdr.IsDBNull(colIndex))
                return default(long);

            return Convert.ToInt64(rdr[colIndex]);
        }

        public static double SafeGetDouble(this SQLiteDataReader rdr, string columnName)
        {
            var colIndex = rdr.GetOrdinal(columnName);

            if (rdr.IsDBNull(colIndex))
                return default(double);

            return Convert.ToDouble(rdr[colIndex]);
        }

        public static double SafeGetDouble(this SQLiteDataReader rdr, string columnName, double defaultValue)
        {
            var colIndex = rdr.GetOrdinal(columnName);

            if (rdr.IsDBNull(colIndex))
                return defaultValue;

            return Convert.ToDouble(rdr[colIndex]);
        }

        public static int? SafeGetNullableInt(this SQLiteDataReader rdr, string columnName)
        {
            var colIndex = rdr.GetOrdinal(columnName);

            if (rdr.IsDBNull(colIndex))
                return null;

            return Convert.ToInt32(rdr[colIndex]);
        }

        public static double? SafeGetNullableDouble(this SQLiteDataReader rdr, string columnName)
        {
            var colIndex = rdr.GetOrdinal(columnName);

            if (rdr.IsDBNull(colIndex))
                return null;

            return Convert.ToDouble(rdr[colIndex]);
        }

        // TODO: This one might not be necessary
        public static string SafeGetString(this SQLiteDataReader rdr, string columnName)
        {
            var colIndex = rdr.GetOrdinal(columnName);

            if (rdr.IsDBNull(colIndex))
                return null;

            return rdr.GetString(colIndex);
        }

        public static string SafeGetStringStripNone(this SQLiteDataReader rdr, string columnName)
        {
            var colIndex = rdr.GetOrdinal(columnName);

            if (rdr.IsDBNull(colIndex))
                return null;

            var s = rdr.GetString(colIndex);

            if (s == "NONE")
                return null;

            return s;
        }
    }
}
