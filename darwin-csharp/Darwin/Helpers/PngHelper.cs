using Darwin.Database;
using Darwin.Utilities;
using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Helpers
{
    public static class PngHelper
    {
        public static void ParsePngText(string filename, out float normScale, out List<ImageMod> imageMods, out bool thumbOnly, out string originalFilename)
        {
			var pngChunkReader = new PngChunkReader(filename);

			imageMods = new List<ImageMod>();
			originalFilename = null;
			thumbOnly = false;
			normScale = 1.0f;

			foreach (var c in pngChunkReader.Chunks)
			{
				var type = Encoding.UTF8.GetString(c.Type);

				// It's actually capitalized like this
				if (!string.IsNullOrEmpty(type) && type == "tEXt")
                {
					var val = Encoding.UTF8.GetString(c.Data);

					if (!string.IsNullOrEmpty(val))
                    {
						if (val.StartsWith("NormScale"))
                        {
							var splitVals = val.Split('\0');

							if (splitVals.Length >= 2)
								normScale = (float)Convert.ToDouble(splitVals[1]);
						}
						else if (val.StartsWith("OriginalImage"))
                        {
							var splitVals = val.Split('\0');

							if (splitVals.Length >= 2)
								originalFilename = splitVals[1];
                        }
						else if (val.StartsWith("ThumbOnly"))
                        {
							var splitVals = val.Split('\0');

							if (splitVals.Length >= 2 && splitVals[1].ToLower() == "yes")
								thumbOnly = true;
						}
						else if (val.StartsWith("ImageMod"))
                        {
							var splitVals = val.Split('\0');

							if (splitVals.Length >= 2)
                            {
								//"%d %d %d %d %d"
								int op = 0;
								int val1 = 0;
								int val2 = 0;
								int val3 = 0;
								int val4 = 0;

								var secondLevelSplit = splitVals[1].Split(' ');

								if (secondLevelSplit.Length >= 1)
									op = Convert.ToInt32(secondLevelSplit[0]);

								if (secondLevelSplit.Length >= 2)
									op = Convert.ToInt32(secondLevelSplit[1]);

								if (secondLevelSplit.Length >= 3)
									op = Convert.ToInt32(secondLevelSplit[2]);

								if (secondLevelSplit.Length >= 4)
									op = Convert.ToInt32(secondLevelSplit[3]);

                                if (secondLevelSplit.Length >= 5)
                                    op = Convert.ToInt32(secondLevelSplit[4]);

								var mod = new ImageMod((ImageModType)op, val1, val2, val3, val4);
								imageMods.Add(mod);
							}
						}							
                    }
				}
				// "tEXt"
				//
				var v = System.Text.Encoding.Default.GetString(c.Data);
				// Author\0DARWIN-2.22 Dolphin PhotoID Software
				// "NormScale\02.225349"
				// "OriginalImage\0goblin.jpg"
				//ImageMod %d %d %d %d %d
				//ThumbOnly\0yes
			}
		}
    }
}
