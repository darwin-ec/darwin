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
        public static IEnumerable<IEnumerable<T>> GetPermutations<T>(IEnumerable<T> items, int count)
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
                    foreach (var result in GetPermutations(items.Skip(i + 1), count - 1))
                        yield return new T[] { item }.Concat(result);
                }

                i += 1;
            }
        }
    }
}
