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
using System.Linq;
using System.Text;

namespace Darwin.Utilities
{
    public static class EnumerableHelper
    {
        // Based on answer by Daniel Hilgarth at
        // https://stackoverflow.com/questions/12249051/unique-combinations-of-list
        // Original license CC BY-SA 3.0 https://creativecommons.org/licenses/by-sa/3.0/
        public static IEnumerable<IEnumerable<T>> GetUniquePermutations<T>(IEnumerable<T> items, int count)
        {
            int i = 0;
            foreach (var item in items)
            {
                if (count == 1)
                {
                    yield return new T[] { item };
                }
                else
                {
                    foreach (var result in GetUniquePermutations(items.Skip(i + 1), count - 1))
                        yield return new T[] { item }.Concat(result);
                }

                i += 1;
            }
        }

        public static IEnumerable<IEnumerable<T>> PermutationsWithRepetition<T>(IEnumerable<T> items, int count)
        {
            if (count <= 0)
            {
                yield return new T[] { };
            }
            else
            {
                foreach (var item in items)
                {
                    if (count == 1)
                    {
                        yield return new T[] { item };
                    }
                    else
                    {
                        foreach (var result in PermutationsWithRepetition(items, count - 1))
                            yield return new T[] { item }.Concat(result);
                    }
                }
            }
        }
    }
}
