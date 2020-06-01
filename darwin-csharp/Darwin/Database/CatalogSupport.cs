using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Text;

namespace Darwin.Database
{
    public static class CatalogSupport
    {
		public const string FinzDatabaseFilename = "database.db";

		public const string SurveyAreasFolderName = "surveyAreas";
		public const string CatalogFolderName = "catalog";
		public const string TracedFinsFolderName = "tracedFins";
		public const string MatchQueuesFolderName = "matchQueues";
		public const string MatchQResultsFolderName = "matchQResults";
		public const string SightingsFolderName = "sightings";

        public static DarwinDatabase OpenDatabase(string databaseFilename, Options o, bool create, string area = "default")
        {
			CatalogScheme cat = new CatalogScheme();
			DarwinDatabase db = null;

			if (create)
			{
				RebuildFolders(o.CurrentDataPath, area);
				// should ONLY end up here with IFF we are NOT converting an old database
				int id = o.CurrentDefaultCatalogScheme;
				cat.SchemeName = o.DefinedCatalogSchemeName[id];
				cat.CategoryNames = o.DefinedCatalogCategoryName[id]; // this is a vector
				db = null; //new SQLiteDatabase(databaseFilename, o, cat, create);
			}

			return db;
		}

        public static DatabaseFin OpenFinz(string filename)
		{
			if (string.IsNullOrEmpty(filename))
				throw new ArgumentNullException(nameof(filename));

			string uniqueDirectoryName = filename.Replace(".", string.Empty) + "_" + Guid.NewGuid().ToString().Replace("-", string.Empty);
			string fullDirectoryName = Path.Combine(Path.GetTempPath(), uniqueDirectoryName);

			try
			{ 
				Directory.CreateDirectory(fullDirectoryName);

				ZipFile.ExtractToDirectory(filename, fullDirectoryName);

				string dbFilename = Path.Combine(fullDirectoryName, FinzDatabaseFilename);

				if (!File.Exists(dbFilename))
					return null;

				var db = OpenDatabase(dbFilename, Options.CurrentUserOptions, false);
			}
			finally
            {
				if (Directory.Exists(fullDirectoryName))
					Directory.Delete(fullDirectoryName, true);
			}

			return null;
			//string baseimgfilename;
			//string tempdir("");
			//tempdir += gOptions->mTempDirectory;//getenv("TEMP");
			//tempdir += PATH_SLASH;
			//tempdir += extractBasename(archive);

			//systemUnzip(archive, tempdir);

			//Options o = Options();
			//o.mDatabaseFileName = tempdir + PATH_SLASH + "database.db";

			//if (!SQLiteDatabase::isType(o.mDatabaseFileName))
			//	return NULL;

			//Database* db = openDatabase(&o, false);

			//if (db->status() != Database::loaded)
			//{
			//	delete db;
			//	return NULL;
			//}

			//DatabaseFin<ColorImage>* fin = db->getItem(0); // first and only fin


			//// construct absolute file paths and open images
			//baseimgfilename = extractBasename(fin->mImageFilename);
			//fin->mImageFilename = tempdir + PATH_SLASH + baseimgfilename;
			//fin->mModifiedFinImage = new ColorImage(fin->mImageFilename);
			//fin->mOriginalImageFilename = tempdir + PATH_SLASH + extractBasename(fin->mModifiedFinImage->mOriginalImageFilename);

			//if ("" != fin->mOriginalImageFilename)
			//{
			//	fin->mFinImage = new ColorImage(fin->mOriginalImageFilename);
			//}

			//fin->mImageMods = fin->mModifiedFinImage->mImageMods;

			//// fixes an issue with the MatchResults trying to re-save the fin
			//fin->mFinFilename = archive;

			//delete db;

			//return fin;
		}

		public static void RebuildFolders(string home, string area)
		{
			if (string.IsNullOrEmpty(home))
				throw new ArgumentNullException(nameof(home));

			if (string.IsNullOrEmpty(area))
				throw new ArgumentNullException(nameof(area));

			Trace.WriteLine("Creating folders...");

			var surveyAreasPath = Path.Combine(new string[] { home, SurveyAreasFolderName, area });

			// Note that CreateDirectory won't do anything if the path already exists, so no need
			// to check first.
			Directory.CreateDirectory(surveyAreasPath);
			Directory.CreateDirectory(Path.Combine(surveyAreasPath, CatalogFolderName));
			Directory.CreateDirectory(Path.Combine(surveyAreasPath, TracedFinsFolderName));
			Directory.CreateDirectory(Path.Combine(surveyAreasPath, MatchQueuesFolderName));
			Directory.CreateDirectory(Path.Combine(surveyAreasPath, MatchQResultsFolderName));
			Directory.CreateDirectory(Path.Combine(surveyAreasPath, SightingsFolderName));
		}
	}
}
