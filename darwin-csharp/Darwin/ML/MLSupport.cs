using CsvHelper;
using Darwin.Database;
using Darwin.Helpers;
using System;
using System.Collections.Generic;
using System.Data;
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

            const string ImagesDirectoryName = "images";
            const int ImageWidth = 96;
            const int ImageHeight = 96;
            const string CsvFilename = "darwin_coordinates.csv";

            string fullImagesDirectory = Path.Combine(datasetDirectory, ImagesDirectoryName);

            Directory.CreateDirectory(fullImagesDirectory);

            var csvRecords = new List<MLCsvRecord>();

            int individualNum = 1;
            foreach (var dbFin in database.AllFins)
            {
                var fin = CatalogSupport.FullyLoadFin(dbFin);

                if (fin.FinOutline.FeatureSet.CoordinateFeaturePoints == null ||
                    fin.FinOutline.FeatureSet.CoordinateFeaturePoints.Count < 1 ||
                    !fin.FinOutline.FeatureSet.CoordinateFeaturePoints.ContainsKey(Features.FeaturePointType.Eye))
                {
                    // If we don't have the features we need, skip to the next one
                    continue;
                }
                var workingImage = BitmapHelper.CropBitmap(fin.FinImage,
                    (int)Math.Floor(fin.FinOutline.ChainPoints.MinX()), (int)Math.Floor(fin.FinOutline.ChainPoints.MinY()),
                    (int)Math.Ceiling(fin.FinOutline.ChainPoints.MaxX()), (int)Math.Ceiling(fin.FinOutline.ChainPoints.MaxY()));

                // Note that we do not want to maintain aspect ratio
                workingImage = BitmapHelper.ResizeBitmap(workingImage, ImageWidth, ImageHeight);

                string imageFilename = individualNum.ToString().PadLeft(6, '0') + ".jpg";

                workingImage.Save(Path.Combine(fullImagesDirectory, imageFilename), ImageFormat.Jpeg);

                csvRecords.Add(new MLCsvRecord
                {
                    image = imageFilename,
                    eye_x = (float)(fin.FinOutline.FeatureSet.CoordinateFeaturePoints[Features.FeaturePointType.Eye].Coordinate.X - Math.Floor(fin.FinOutline.ChainPoints.MinX())),
                    eye_y = (float)(fin.FinOutline.FeatureSet.CoordinateFeaturePoints[Features.FeaturePointType.Eye].Coordinate.Y - Math.Floor(fin.FinOutline.ChainPoints.MinY()))
                });

                individualNum += 1;
            }

            using (var writer = new StreamWriter(Path.Combine(datasetDirectory, CsvFilename), false))
            using (var csv = new CsvWriter(writer, CultureInfo.InvariantCulture))
            {
                csv.WriteRecords(csvRecords);
            }
        }
    }
}
