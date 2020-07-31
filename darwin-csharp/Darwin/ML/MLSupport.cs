using CsvHelper;
using Darwin.Database;
using Darwin.Helpers;
using System;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;
using System.Drawing.Imaging;
using System.Globalization;
using System.IO;
using System.Text;

namespace Darwin.ML
{
    public static class MLSupport
    {
        public static void SaveDatasetImages(string datasetDirectory, DarwinDatabase database)
        {
            if (datasetDirectory == null)
                throw new ArgumentNullException(nameof(datasetDirectory));

            if (database == null)
                throw new ArgumentNullException(nameof(database));

            if (!Directory.Exists(datasetDirectory))
                throw new ArgumentOutOfRangeException(nameof(datasetDirectory));

            Trace.WriteLine("Starting dataset export...");

            const string ImagesDirectoryName = "images";
            const int ImageWidth = 224;
            const int ImageHeight = 224;
            const string CsvFilename = "darwin_coordinates.csv";

            string fullImagesDirectory = Path.Combine(datasetDirectory, ImagesDirectoryName);

            Directory.CreateDirectory(fullImagesDirectory);

            var csvRecords = new List<MLCsvRecord>();

            int individualNum = 1;
            foreach (var dbFin in database.AllFins)
            { 
                var fin = CatalogSupport.FullyLoadFin(dbFin);

                if (!string.IsNullOrEmpty(fin?.IDCode))
                    Trace.WriteLine("Exporting " + fin.IDCode);

                if (fin.FinOutline.FeatureSet.CoordinateFeaturePoints == null ||
                    fin.FinOutline.FeatureSet.CoordinateFeaturePoints.Count < 1 ||
                    !fin.FinOutline.FeatureSet.CoordinateFeaturePoints.ContainsKey(Features.FeaturePointType.Eye))
                {
                    // If we don't have the features we need, skip to the next one
                    continue;
                }
                int minX = (int)Math.Floor(fin.FinOutline.ChainPoints.MinX() / fin.Scale);
                int minY = (int)Math.Floor(fin.FinOutline.ChainPoints.MinY() / fin.Scale);
                int maxX = (int)Math.Ceiling(fin.FinOutline.ChainPoints.MaxX() / fin.Scale);
                int maxY = (int)Math.Ceiling(fin.FinOutline.ChainPoints.MaxY() / fin.Scale);

                // Figure out the ratio
                var resizeRatioX = (float)ImageWidth / (maxX - minX);
                var resizeRatioY = (float)ImageHeight / (maxY - minY);

                if (resizeRatioX > resizeRatioY)
                {
                    // We're X constrained, so expand the X
                    var extra = ((maxY - minY) - (maxX - minX)) * ((float)ImageWidth / ImageHeight);
                    minX -= (int)Math.Round(extra / 2);
                    maxX += (int)Math.Round(extra / 2);

                    if (minX < 0)
                    {
                        maxX += (0 - minX);
                        minX = 0;
                    }
                    
                    if (maxX > fin.FinImage.Width)
                    {
                        minX -= maxX - fin.FinImage.Width;
                        maxX = fin.FinImage.Width;
                    }

                    if (minX < 0)
                        minX = 0;
                    if (maxX > fin.FinImage.Width)
                        maxX = fin.FinImage.Width;
                }
                else
                {
                    // We're Y constrained, so expand the Y
                    var extra = ((maxX - minX) - (maxY - minY)) * ((float)ImageHeight / ImageWidth);
                    minY -= (int)Math.Round(extra / 2);
                    maxY += (int)Math.Round(extra / 2);

                    if (minY < 0)
                    {
                        maxY += (0 - minY);
                        minY = 0;
                    }

                    if (maxY > fin.FinImage.Height)
                    {
                        minY -= maxY - fin.FinImage.Height;
                        maxY = fin.FinImage.Height;
                    }

                    if (minY < 0)
                        minY = 0;
                    if (maxY > fin.FinImage.Height)
                        maxY = fin.FinImage.Height;
                }

                var workingImage = BitmapHelper.CropBitmap(fin.FinImage,
                    minX, minY,
                    maxX, maxY);

                // We've hopefully already corrected for the aspect ratio above
                workingImage = BitmapHelper.ResizeBitmap(workingImage, ImageWidth, ImageHeight);

                float xRatio = (float)ImageWidth / (maxX - minX);
                float yRatio = (float)ImageHeight / (maxY - minY);

                string imageFilename = individualNum.ToString().PadLeft(6, '0') + ".jpg";

                workingImage.Save(Path.Combine(fullImagesDirectory, imageFilename), ImageFormat.Jpeg);

                csvRecords.Add(new MLCsvRecord
                {
                    image = imageFilename,
                    eye_x = xRatio * (float)(fin.FinOutline.FeatureSet.CoordinateFeaturePoints[Features.FeaturePointType.Eye].Coordinate.X / fin.Scale - minX),
                    eye_y = yRatio * (float)(fin.FinOutline.FeatureSet.CoordinateFeaturePoints[Features.FeaturePointType.Eye].Coordinate.Y / fin.Scale - minY),
                    nasalfold_x = xRatio * (float)(fin.FinOutline.FeatureSet.CoordinateFeaturePoints[Features.FeaturePointType.NasalLateralCommissure].Coordinate.X / fin.Scale - minX),
                    nasalfold_y = yRatio * (float)(fin.FinOutline.FeatureSet.CoordinateFeaturePoints[Features.FeaturePointType.NasalLateralCommissure].Coordinate.Y / fin.Scale - minY)
                });

                individualNum += 1;
            }

            using (var writer = new StreamWriter(Path.Combine(datasetDirectory, CsvFilename), false))
            using (var csv = new CsvWriter(writer, CultureInfo.InvariantCulture))
            {
                csv.WriteRecords(csvRecords);
            }

            Trace.WriteLine("done.");
        }
    }
}
