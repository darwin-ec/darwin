using Darwin.Database;
using Darwin.Testing;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.IO.IsolatedStorage;
using System.Text;

namespace Darwin
{
    public class Options
    {
        public const string DarwinDataFolderName = "darwinPhotoIdData";
        public const string SurveyAreasFolderName = "surveyAreas";
        public const string DefaultSurveyAreaName = "default";
        public const string CatalogFolderName = "catalog";
        public const string TracedFinsFolderName = "tracedFins";
        public const string MatchQueuesFolderName = "matchQueues";
        public const string MatchQResultsFolderName = "matchQResults";
        public const string SightingsFolderName = "sightings";

        // TODO: This gets overwritten by finz stuff.  Might want to change
        //public string DatabaseFileName { get; set; }

        public string CurrentDarwinHome { get; set; }
        public string DatabaseFileName { get; set; } = string.Empty;
        public string CurrentDataPath { get; set; }

        public string CurrentSurveyArea { get; set; } = string.Empty;

        public IndividualIDSettingsType IndividualIDSettings { get; set; } = IndividualIDSettingsType.ShowIDs;

        [JsonIgnore]
        public string CurrentMatchQueueResultsPath
        {
            get
            {
                if (string.IsNullOrEmpty(CurrentDataPath))
                    return string.Empty;

                return Path.Combine(CurrentDataPath, CurrentSurveyArea, MatchQResultsFolderName);
            }
        }

        [JsonIgnore]
        public string CurrentCatalogPath
        {
            get
            {
                if (string.IsNullOrEmpty(CurrentDataPath))
                    return string.Empty;

                return Path.Combine(CurrentDataPath, CurrentSurveyArea, CatalogFolderName);
            }
        }

        [JsonIgnore]
        public string CurrentMatchQueuePath
        {
            get
            {
                if (string.IsNullOrEmpty(CurrentDataPath))
                    return string.Empty;

                return Path.Combine(CurrentDataPath, CurrentSurveyArea, MatchQueuesFolderName);
            }
        }

        [JsonIgnore]
        public string CurrentTracedFinsPath
        {
            get
            {
                if (string.IsNullOrEmpty(CurrentDataPath))
                    return string.Empty;

                return Path.Combine(CurrentDataPath, CurrentSurveyArea, TracedFinsFolderName);
            }
        }

        [JsonIgnore]
        public string CurrentSurveyAreaPath
        {
            get
            {
                if (string.IsNullOrEmpty(CurrentDataPath))
                    return string.Empty;

                return Path.Combine(CurrentDataPath, CurrentSurveyArea);
            }
        }

        //[JsonIgnore]
        //public string CurrentSurveyAreasPath
        //{
        //    get
        //    {
        //        // We're going to walk back up one level from the data path
        //        var dirInfo = new DirectoryInfo(Path.GetDirectoryName(CurrentDataPath));

        //        // Fall back if there's a problem getting the parent
        //        if (dirInfo == null || dirInfo.Parent == null)
        //            return Path.GetDirectoryName(CurrentDataPath);
        //        else
        //            return dirInfo.Parent.FullName;
        //    }
        //}

        [JsonIgnore]
        public string CurrentSightingsPath
        {
            get
            {
                if (string.IsNullOrEmpty(CurrentDataPath))
                    return string.Empty;

                return Path.Combine(CurrentDataPath, CurrentSurveyArea, SightingsFolderName);
            }
        }

        [DefaultValue(0)]
        public int DefaultCatalogScheme { get; set; } = 0;
        public List<CatalogScheme> CatalogSchemes { get; set; }

        [DefaultValue(50)]
        public int SnakeMaximumIterations { get; set; } = 50;
        [DefaultValue(9.0f)]
        public float SnakeEnergyContinuity { get; set; } = 9.0f;
        [DefaultValue(3.0f)]
        public float SnakeEnergyLinearity { get; set; } = 3.0f;
        [DefaultValue(3.0f)]
        public float SnakeEnergyEdge { get; set; } = 3.0f;

        [DefaultValue(1.5f)]
        public float GaussianStdDev { get; set; } = 1.5f;
        [DefaultValue(0.15f)]
        public float CannyLowThreshold { get; set; }  = 0.15f;
        [DefaultValue(0.85f)]
        public float CannyHighThreshold { get; set; }  = 0.85f;

        [DefaultValue(3)]
        public float DrawingPointSize { get; set; } = 3.0f;

        private const string OptionsFilename = "options.json";

        // Don't let this class be created directly with its constructor
        private Options() { }

        private static Options _currentUserOptions;

        public static Options CurrentUserOptions
        {
            get
            {
                if (_currentUserOptions != null)
                    return _currentUserOptions;

                IsolatedStorageFile isoStore = IsolatedStorageFile.GetStore(IsolatedStorageScope.User | IsolatedStorageScope.Assembly, null, null);

                // TODO: Should be some error handling in here if the file isn't the right format/etc.
                if (isoStore.FileExists(OptionsFilename))
                {
                    using (IsolatedStorageFileStream isoStream = new IsolatedStorageFileStream(OptionsFilename, FileMode.Open, isoStore))
                    {
                        var serializer = new JsonSerializer();

                        using (StreamReader reader = new StreamReader(isoStream))
                        {
                            using (var jsonTextReader = new JsonTextReader(reader))
                            {
                                _currentUserOptions = serializer.Deserialize<Options>(jsonTextReader);
                            }
                        }
                    }
                }

                if (_currentUserOptions == null)
                    _currentUserOptions = new Options();

                CheckCurrentDarwinHome(ref _currentUserOptions);
                CheckCurrentDataPath(ref _currentUserOptions);
                CheckCatalogSchemes(ref _currentUserOptions);

                return _currentUserOptions;
            }
        }

        /// <summary>
        /// Copy constructor
        /// </summary>
        /// <param name="options"></param>
        public Options(Options options)
        {
            if (options == null)
                return;

            CurrentDarwinHome = options.CurrentDarwinHome;
            DatabaseFileName = options.DatabaseFileName;
            CurrentDataPath = options.CurrentDataPath;
            CurrentSurveyArea = options.CurrentSurveyArea;

            CannyHighThreshold = options.CannyHighThreshold;
            CannyLowThreshold = options.CannyLowThreshold;
            GaussianStdDev = options.GaussianStdDev;

            DrawingPointSize = options.DrawingPointSize;

            SnakeEnergyContinuity = options.SnakeEnergyContinuity;
            SnakeEnergyEdge = options.SnakeEnergyEdge;
            SnakeEnergyLinearity = options.SnakeEnergyLinearity;
            SnakeMaximumIterations = options.SnakeMaximumIterations;

            DefaultCatalogScheme = options.DefaultCatalogScheme;

            if (options.CatalogSchemes != null)
                CatalogSchemes = new List<CatalogScheme>(options.CatalogSchemes);

            IndividualIDSettings = options.IndividualIDSettings;
        }

        public void Save(bool reloadOptions = false)
        {
            IsolatedStorageFile isoStore = IsolatedStorageFile.GetStore(IsolatedStorageScope.User | IsolatedStorageScope.Assembly, null, null);

            using (IsolatedStorageFileStream isoStream = new IsolatedStorageFileStream(OptionsFilename, FileMode.OpenOrCreate, isoStore))
            {
                using (StreamWriter writer = new StreamWriter(isoStream))
                {
                    var serializedJson = JsonConvert.SerializeObject(this);
                    writer.Write(serializedJson);
                }
            }

            // Null out _currentUserOptions so they'll be reloaded next
            // time they're accessed.
            if (reloadOptions)
                _currentUserOptions = null;
        }

        public void SetLastDatabaseFilename(string filename)
        {
            DatabaseFileName = filename;

            // We're going to walk back up two levels
            var dirInfo = new DirectoryInfo(Path.GetDirectoryName(filename));
            CurrentSurveyArea = string.Empty;

            // Fall back if there's a problem getting the parent
            if (dirInfo == null || dirInfo.Parent == null)
            {
                CurrentDataPath = Path.GetDirectoryName(filename);
            }
            else
            {
                if (dirInfo.Parent.Parent == null)
                {
                    CurrentDataPath = dirInfo.Parent.FullName;
                }
                else
                {
                    CurrentDataPath = dirInfo.Parent.Parent.FullName;
                    CurrentSurveyArea = Path.GetFileName(dirInfo.Parent.FullName);
                }
            }
        }

        /// <summary>
        /// Sets a default current data path if one isn't already set.
        /// </summary>
        /// <param name="options">We don't actually have to pass this as ref, but prefer it this way for clarity</param>
        private static void CheckCurrentDataPath(ref Options options)
        {
            if (options == null)
                return;

            if (string.IsNullOrEmpty(options.CurrentDataPath))
            {
                var myDocumentsPath = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);

                if (Directory.Exists(myDocumentsPath))
                {
                    string myDocumentsWithDarwinPhotoId = Path.Combine(myDocumentsPath, DarwinDataFolderName);

                    if (Directory.Exists(myDocumentsWithDarwinPhotoId))
                    {
                        options.CurrentDataPath = myDocumentsWithDarwinPhotoId;
                    }
                    else
                    {
                        options.CurrentDataPath = myDocumentsPath;
                    }
                }
            }
            else if (string.IsNullOrEmpty(options.CurrentSurveyArea))
            {
                DirectoryInfo datapathInfo = new DirectoryInfo(options.CurrentDataPath);

                if (datapathInfo != null && datapathInfo.Parent != null)
                {
                    options.CurrentSurveyArea = Path.GetFileName(options.CurrentDataPath);
                    options.CurrentDataPath = datapathInfo.Parent.FullName;
                }
            }
        }

        private static void CheckCurrentDarwinHome(ref Options options)
        {
            if (options == null)
                return;

            if (string.IsNullOrEmpty(options.CurrentDarwinHome))
            {
                var myDocumentsPath = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);

                if (Directory.Exists(myDocumentsPath))
                {
                    string myDocumentsWithDarwinPhotoId = Path.Combine(myDocumentsPath, DarwinDataFolderName);

                    if (Directory.Exists(myDocumentsWithDarwinPhotoId))
                    {
                        options.CurrentDarwinHome = myDocumentsWithDarwinPhotoId;
                    }
                    else
                    {
                        try
                        {
                            Directory.CreateDirectory(myDocumentsWithDarwinPhotoId);
                            options.CurrentDarwinHome = myDocumentsWithDarwinPhotoId;
                        }
                        catch
                        {
                            options.CurrentDarwinHome = myDocumentsPath;
                        }
                    }
                }
            }
        }

        private static void CheckCatalogSchemes(ref Options options)
        {
            if (options == null)
                return;

            if (options.CatalogSchemes == null)
                options.CatalogSchemes = new List<CatalogScheme>();

            if (options.CatalogSchemes.Count < 1)
            {
                var defaultScheme = new CatalogScheme
                {
                    SchemeName = "Eckerd College",
                    CategoryNames = new List<string>()
                };

                defaultScheme.CategoryNames.Add("NONE");  // shown as "Unspecified" in database and pull-down lists
                defaultScheme.CategoryNames.Add("Upper");
                defaultScheme.CategoryNames.Add("Middle");
                defaultScheme.CategoryNames.Add("Lower");
                defaultScheme.CategoryNames.Add("Upper-Middle");
                defaultScheme.CategoryNames.Add("Upper-Lower");
                defaultScheme.CategoryNames.Add("Middle-Lower");
                defaultScheme.CategoryNames.Add("Leading Edge");
                defaultScheme.CategoryNames.Add("Entire");
                defaultScheme.CategoryNames.Add("Tip-Nick");
                defaultScheme.CategoryNames.Add("Missing Tip");
                defaultScheme.CategoryNames.Add("Extended Tip");
                defaultScheme.CategoryNames.Add("Peduncle");
                defaultScheme.CategoryNames.Add("Pergatory");

                options.CatalogSchemes.Add(defaultScheme);
            }
        }
    }
}
