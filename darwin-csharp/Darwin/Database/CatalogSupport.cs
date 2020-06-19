using Darwin.Helpers;
using Darwin.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Data.SQLite;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;

namespace Darwin.Database
{
    public static class CatalogSupport
    {
		public const string FinzDatabaseFilename = "database.db";

        public static DarwinDatabase OpenDatabase(string databaseFilename,
			CatalogScheme catalogScheme, bool create, string area = "default")
        {
			string filename = databaseFilename;
			if (create)
			{
				filename = Path.Combine(Options.CurrentUserOptions.CurrentDataPath, area, Options.CatalogFolderName, filename);

				if (File.Exists(filename))
					throw new Exception("The database " + filename + Environment.NewLine +
						"already exists.  Please pick another name.");

				RebuildFolders(Options.CurrentUserOptions.CurrentDataPath, area);
			}

            DarwinDatabase db = new SQLiteDatabase(filename, catalogScheme, create);

            return db;
		}

		public static void UpdateFinFieldsFromImage(string basePath, DatabaseFin fin)
        {
			// TODO:
			// Probably not the best "flag" to look at.  We're saving OriginalImageFilename in the database for newer fins
			if (string.IsNullOrEmpty(fin.OriginalImageFilename))
			{
				List<ImageMod> imageMods;
				bool thumbOnly;
				string originalFilename;
				float normScale;

				string fullFilename = Path.Combine(basePath, fin.ImageFilename);

				PngHelper.ParsePngText(fullFilename, out normScale, out imageMods, out thumbOnly, out originalFilename);

				fin.ImageMods = imageMods;
				fin.Scale = normScale;

				// This is a little hacky, but we're going to get the bottom directory name, and append that to
				// the filename below.
				var bottomDirectoryName = Path.GetFileName(Path.GetDirectoryName(fullFilename));

				if (!string.IsNullOrEmpty(originalFilename))
				{
					originalFilename = Path.Combine(bottomDirectoryName, originalFilename);

					// TODO Original isn't right -- need to replay imagemods, maybe?
					fin.OriginalImageFilename = originalFilename;
					fin.ImageFilename = originalFilename;
				}
				// TODO: Save these changes back to the database?
			}
		}

		public static DatabaseFin OpenFinz(string filename)
		{
			if (string.IsNullOrEmpty(filename))
				throw new ArgumentNullException(nameof(filename));

			string uniqueDirectoryName = Path.GetFileName(filename).Replace(".", string.Empty) + "_" + Guid.NewGuid().ToString().Replace("-", string.Empty);
			string fullDirectoryName = Path.Combine(Path.GetTempPath(), uniqueDirectoryName);

			try
			{ 
				Directory.CreateDirectory(fullDirectoryName);

				ZipFile.ExtractToDirectory(filename, fullDirectoryName);

				string dbFilename = Path.Combine(fullDirectoryName, FinzDatabaseFilename);

				if (!File.Exists(dbFilename))
					return null;

                var db = OpenDatabase(dbFilename, Options.CurrentUserOptions.DefaultCatalogScheme, false);

                // First and only fin
                var fin = db.AllFins[0];

				fin.FinFilename = filename;

                var baseimgfilename = Path.GetFileName(fin.ImageFilename);
                fin.ImageFilename = Path.Combine(fullDirectoryName, baseimgfilename);

				List<ImageMod> imageMods;
				bool thumbOnly;
				string originalFilename;
				float normScale;

				PngHelper.ParsePngText(fin.ImageFilename, out normScale, out imageMods, out thumbOnly, out originalFilename);

				if (imageMods != null && imageMods.Count > 0)
					fin.ImageMods = imageMods;

				if (normScale != 1.0)
					fin.Scale = normScale;

				// TODO: Do something with thumbOnly?

				// We're loading the image this way because Bitmap keeps a lock on the original file, and
				// we want to try to delete the file below.  So we open the file in another object in a using statement
				// then copy it over to our actual working object.
				using (var imageFromFile = (Bitmap)Image.FromFile(fin.ImageFilename))
				{
					fin.FinImage = new Bitmap(imageFromFile);
					fin.FinImage?.SetResolution(96, 96);
				}

				if (!string.IsNullOrEmpty(originalFilename))
				{
					fin.OriginalImageFilename = Path.Combine(fullDirectoryName, Path.GetFileName(originalFilename));

					using (var originalImageFromFile = (Bitmap)Image.FromFile(fin.OriginalImageFilename))
					{
						fin.OriginalFinImage = new Bitmap(originalImageFromFile);
						fin.OriginalFinImage?.SetResolution(96, 96);

						if (fin.ImageMods != null)
						{
							fin.FinImage = ModificationHelper.ApplyImageModificationsToOriginal(fin.OriginalFinImage, fin.ImageMods);
							fin.FinImage?.SetResolution(96, 96);
						}
					}
				}

				return fin;
            }
			catch
            {
				// TODO: Probably should have better handling here
				return null;
            }
			finally
            {
				try
				{
					Trace.WriteLine("Trying to remove temporary files for finz file.");

					SQLiteConnection.ClearAllPools();

					GC.Collect();
					GC.WaitForPendingFinalizers();

					if (Directory.Exists(fullDirectoryName))
						Directory.Delete(fullDirectoryName, true);
				}
				catch (Exception ex)
                {
					Trace.Write("Couldn't remove temporary files:");
					Trace.WriteLine(ex);
                }
			}
		}

		public static List<string> GetExistingDatabaseNames()
        {
			if (!Directory.Exists(Options.CurrentUserOptions.CurrentCatalogPath))
				return null;

			DirectoryInfo dirInfo = new DirectoryInfo(Options.CurrentUserOptions.CurrentCatalogPath);

			var directories = dirInfo.GetFiles();

			List<string> results = new List<string>();

			foreach (var dir in directories)
			{
				results.Add(dir.Name);
			}

			return results;
		}

		public static List<string> GetExistingSurveyAreas()
        {
			if (!Directory.Exists(Options.CurrentUserOptions.CurrentDataPath))
				return null;

			DirectoryInfo dirInfo = new DirectoryInfo(Options.CurrentUserOptions.CurrentDataPath);

			var directories = dirInfo.GetDirectories();

			List<string> results = new List<string>();

			foreach (var dir in directories)
            {
				results.Add(dir.Name);
            }

			return results;
        }

		public static string SaveFinz(DatabaseFin fin, string filename, bool forceFilename = true)
        {
			if (fin == null)
				throw new ArgumentNullException(nameof(fin));

			if (string.IsNullOrEmpty(filename))
				throw new ArgumentNullException(nameof(filename));

			if (!filename.ToLower().EndsWith(".finz"))
				filename += ".finz";

			string uniqueDirectoryName = Path.GetFileName(filename).Replace(".", string.Empty) + "_" + Guid.NewGuid().ToString().Replace("-", string.Empty);
			string fullDirectoryName = Path.Combine(Path.GetTempPath(), uniqueDirectoryName);

			try
			{
				Directory.CreateDirectory(fullDirectoryName);

				string originalDestination = Path.Combine(fullDirectoryName, Path.GetFileName(fin.OriginalImageFilename));
				
				if (File.Exists(fin.OriginalImageFilename))
					File.Copy(fin.OriginalImageFilename, originalDestination);
				else if (fin.OriginalFinImage != null)
					fin.OriginalFinImage.Save(originalDestination);

				fin.OriginalImageFilename = originalDestination;

				// replace ".finz" with "_wDarwinMods.png" for modified image filename

				fin.ImageFilename = Path.Combine(fullDirectoryName, Path.GetFileNameWithoutExtension(filename) + AppSettings.DarwinModsFilenameAppendPng);

				fin.FinImage.Save(fin.ImageFilename);

				string dbFilename = Path.Combine(fullDirectoryName, "database.db");

				CatalogScheme cat = Options.CurrentUserOptions.CatalogSchemes.Where(cs => cs.IsDefault).FirstOrDefault();

				if (cat == null)
					cat = new CatalogScheme();

				if (cat.Categories == null)
					cat.Categories = new ObservableCollection<Category>();

				if (!cat.Categories.ToList().Exists(c => c != null && c.Name?.ToUpper() == fin.DamageCategory.ToUpper()))
					cat.Categories.Add(new Category(fin.DamageCategory));

				SQLiteDatabase db = new SQLiteDatabase(dbFilename, cat, true);
				db.Add(fin);

				// The below before we try to create a ZIP is because SQLite tries to hold onto the database file
				// even after the connections are closed
				SQLiteConnection.ClearAllPools();

				GC.Collect();
				GC.WaitForPendingFinalizers();

				string realFilename = filename;
				if (!forceFilename)
					realFilename = FileHelper.FindUniqueFilename(realFilename);
				else if (File.Exists(realFilename))
					File.Delete(realFilename);

				ZipFile.CreateFromDirectory(fullDirectoryName, realFilename);

				return realFilename;
			}
			finally
			{
				try
				{
					Trace.WriteLine("Trying to remove temporary files for finz file.");

					SQLiteConnection.ClearAllPools();

					GC.Collect();
					GC.WaitForPendingFinalizers();

					if (Directory.Exists(fullDirectoryName))
						Directory.Delete(fullDirectoryName, true);
				}
				catch (Exception ex)
				{
					Trace.Write("Couldn't remove temporary files:");
					Trace.WriteLine(ex);
				}
			}
		}

		public static void SaveToDatabase(DarwinDatabase database, DatabaseFin databaseFin)
        {
			if (database == null)
				throw new ArgumentNullException(nameof(database));

			if (databaseFin == null)
				throw new ArgumentNullException(nameof(databaseFin));

			if (string.IsNullOrEmpty(databaseFin.OriginalImageFilename) || databaseFin.OriginalFinImage == null)
				throw new ArgumentOutOfRangeException(nameof(databaseFin));

			// First, copy the images to the catalog folder

			// Check the original image.  If we still have the actual original image file, just copy it.  If
			// not, then save the one we have in memory to the folder.

			string originalImageSaveAs = Path.Combine(Options.CurrentUserOptions.CurrentCatalogPath, Path.GetFileName(databaseFin.OriginalImageFilename));

			// If we already have an item in the database with the same filename, try a few others
			if (File.Exists(originalImageSaveAs))
				originalImageSaveAs = FileHelper.FindUniqueFilename(originalImageSaveAs);

			if (File.Exists(databaseFin.OriginalImageFilename))
            {
				File.Copy(databaseFin.OriginalImageFilename, originalImageSaveAs);
            }
			else
            {
				databaseFin.OriginalFinImage.Save(originalImageSaveAs);
            }

			// Now save the modified image (or the original if for some reason we don't have the modified one)
			string modifiedImageSaveAs = Path.Combine(Options.CurrentUserOptions.CurrentCatalogPath,
				Path.GetFileNameWithoutExtension(originalImageSaveAs) + AppSettings.DarwinModsFilenameAppendPng);

			if (databaseFin.FinImage != null)
            {
				databaseFin.FinImage.Save(modifiedImageSaveAs);
            }
			else
            {
				databaseFin.OriginalFinImage.Save(modifiedImageSaveAs);
            }

			// Now let's overwrite the filenames without any paths
			databaseFin.OriginalImageFilename = Path.GetFileName(originalImageSaveAs);
			databaseFin.ImageFilename = Path.GetFileName(modifiedImageSaveAs);

			// Finally, add it to the database
			database.Add(databaseFin);
        }

		public static void RebuildFolders(string currentDataPath, string area)
		{
			if (string.IsNullOrEmpty(currentDataPath))
				throw new ArgumentNullException(nameof(currentDataPath));

			if (string.IsNullOrEmpty(area))
				throw new ArgumentNullException(nameof(area));

			Trace.WriteLine("Creating folders...");

			var surveyAreasPath = Path.Combine(new string[] { currentDataPath, area });

			// Note that CreateDirectory won't do anything if the path already exists, so no need
			// to check first.
			Directory.CreateDirectory(surveyAreasPath);
			Directory.CreateDirectory(Path.Combine(surveyAreasPath, Options.CatalogFolderName));
			Directory.CreateDirectory(Path.Combine(surveyAreasPath, Options.TracedFinsFolderName));
			Directory.CreateDirectory(Path.Combine(surveyAreasPath, Options.MatchQueuesFolderName));
			Directory.CreateDirectory(Path.Combine(surveyAreasPath, Options.MatchQResultsFolderName));
			Directory.CreateDirectory(Path.Combine(surveyAreasPath, Options.SightingsFolderName));
		}
	}
}
