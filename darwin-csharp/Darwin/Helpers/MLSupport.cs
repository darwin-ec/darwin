using CsvHelper;
using Darwin.Database;
using Darwin.Helpers;
using Darwin.ML;
using System;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;

namespace Darwin.Helpers
{
    public static class MLSupport
    {
        public const string ImagesDirectoryName = "images";
        public const int ImageWidth = 224;
        public const int ImageHeight = 224;
        public const string CsvFilename = "darwin_coordinates.csv";

        public static MLImage ConvertDatabaseFinToMLImage(Bitmap image, FloatContour contour, double scale)
        {
            int xMin = (int)Math.Floor(contour.MinX() / scale);
            int yMin = (int)Math.Floor(contour.MinY() / scale);
            int xMax = (int)Math.Ceiling(contour.MaxX() / scale);
            int yMax = (int)Math.Ceiling(contour.MaxY() / scale);

            // Figure out the ratio
            var resizeRatioX = (float)ImageWidth / (xMax - xMin);
            var resizeRatioY = (float)ImageHeight / (yMax - yMin);

            if (resizeRatioX > resizeRatioY)
            {
                // We're X constrained, so expand the X
                var extra = ((yMax - yMin) - (xMax - xMin)) * ((float)ImageWidth / ImageHeight);
                xMin -= (int)Math.Round(extra / 2);
                xMax += (int)Math.Round(extra / 2);

                if (xMin < 0)
                {
                    xMax += (0 - xMin);
                    xMin = 0;
                }

                if (xMax > image.Width)
                {
                    xMin -= xMax - image.Width;
                    xMax = image.Width;
                }

                if (xMin < 0)
                    xMin = 0;
                if (xMax > image.Width)
                    xMax = image.Width;
            }
            else
            {
                // We're Y constrained, so expand the Y
                var extra = ((xMax - xMin) - (yMax - yMin)) * ((float)ImageHeight / ImageWidth);
                yMin -= (int)Math.Round(extra / 2);
                yMax += (int)Math.Round(extra / 2);

                if (yMin < 0)
                {
                    yMax += (0 - yMin);
                    yMin = 0;
                }

                if (yMax > image.Height)
                {
                    yMin -= yMax - image.Height;
                    yMax = image.Height;
                }

                if (yMin < 0)
                    yMin = 0;
                if (yMax > image.Height)
                    yMax = image.Height;
            }

            var workingImage = BitmapHelper.CropBitmap(image,
                xMin, yMin,
                xMax, yMax);

            // We've hopefully already corrected for the aspect ratio above
            workingImage = BitmapHelper.ResizeBitmap(workingImage, ImageWidth, ImageHeight);

            float xRatio = (float)ImageWidth / (xMax - xMin);
            float yRatio = (float)ImageHeight / (yMax - yMin);

            return new MLImage
            {
                Image = workingImage,
                XMin = xMin,
                XMax = xMax,
                YMin = yMin,
                YMax = yMax,
                XRatio = xRatio,
                YRatio = yRatio
            };
        }

        public static void SaveDatasetImages(string datasetDirectory, DarwinDatabase database)
        {
            if (datasetDirectory == null)
                throw new ArgumentNullException(nameof(datasetDirectory));

            if (database == null)
                throw new ArgumentNullException(nameof(database));

            if (!Directory.Exists(datasetDirectory))
                throw new ArgumentOutOfRangeException(nameof(datasetDirectory));

            Trace.WriteLine("Starting dataset export...");

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

                var mlImage = ConvertDatabaseFinToMLImage(fin.FinImage, fin.FinOutline.ChainPoints, fin.Scale);

                string imageFilename = individualNum.ToString().PadLeft(6, '0') + ".jpg";

                mlImage.Image.Save(Path.Combine(fullImagesDirectory, imageFilename), ImageFormat.Jpeg);

                csvRecords.Add(new MLCsvRecord
                {
                    image = imageFilename,
                    eye_x = mlImage.XRatio * (float)(fin.FinOutline.FeatureSet.CoordinateFeaturePoints[Features.FeaturePointType.Eye].Coordinate.X / fin.Scale - mlImage.XMin),
                    eye_y = mlImage.YRatio * (float)(fin.FinOutline.FeatureSet.CoordinateFeaturePoints[Features.FeaturePointType.Eye].Coordinate.Y / fin.Scale - mlImage.YMin),
                    nasalfold_x = mlImage.XRatio * (float)(fin.FinOutline.FeatureSet.CoordinateFeaturePoints[Features.FeaturePointType.NasalLateralCommissure].Coordinate.X / fin.Scale - mlImage.XMin),
                    nasalfold_y = mlImage.YRatio * (float)(fin.FinOutline.FeatureSet.CoordinateFeaturePoints[Features.FeaturePointType.NasalLateralCommissure].Coordinate.Y / fin.Scale - mlImage.YMin)
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

        public static float[] PredictCoordinates(Bitmap image, FloatContour chainPoints, double scale)
        {
            Trace.WriteLine("Predicting coordinates with " + AppSettings.MLModelFilename + " model using Emgu.TF.Lite / TensorFlow Lite");

            var model = new MLModel(Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), AppSettings.MLModelFilename));

            var mlImage = ConvertDatabaseFinToMLImage(image, chainPoints, scale);
            var directBmp = new DirectBitmap(mlImage.Image);

            //var floatArray = directBmp.ToScaledRGBFloatArray();

            // This must match what the model expects. E.g., this is what Keras on TF for Resnet uses:
            var floatArray = directBmp.ToScaledTensorFlowRGBPreprocessInput();
            
            var coordinates = model.Run(floatArray);

            Trace.WriteLine("Raw predicted coordinates:");

            // Scale/translate the coordinates back to the original image
            for (int i = 0; i < coordinates.Length; i++)
            {
                Trace.Write(" " + coordinates[i]);
                if (i % 2 == 0)
                    coordinates[i] = (float)((coordinates[i] / mlImage.XRatio + mlImage.XMin) * scale);
                else
                    coordinates[i] = (float)((coordinates[i] / mlImage.YRatio + mlImage.YMin) * scale);
            }

            Trace.WriteLine(" ");

            // Rescale the coordinates back to the original
            return coordinates;
        }
    }
}
