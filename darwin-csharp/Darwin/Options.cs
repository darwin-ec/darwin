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

        // TODO: This gets overwritten by finz stuff.  Might want to change
        //public string DatabaseFileName { get; set; }

        public string DatabaseFileName { get; set; } = string.Empty;
        public string CurrentDataPath { get; set; }

        public int CurrentDefaultCatalogScheme { get; set; }
        public string CurrentDefaultCatalogSchemeName { get; set; }
        public string CurrentSurveyArea { get; set; } = string.Empty;
        public List<string> DefinedCatalogSchemeName { get; set; }
        public List<List<string>> DefinedCatalogCategoryName { get; set; }

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

                CheckCurrentDataPath(ref _currentUserOptions);

                return _currentUserOptions;
            }
        }

        public void Save()
        {
            IsolatedStorageFile isoStore = IsolatedStorageFile.GetStore(IsolatedStorageScope.User | IsolatedStorageScope.Assembly, null, null);

            using (IsolatedStorageFileStream isoStream = new IsolatedStorageFileStream(OptionsFilename, FileMode.CreateNew, isoStore))
            {
                using (StreamWriter writer = new StreamWriter(isoStream))
                {
                    var serializedJson = JsonConvert.SerializeObject(this);
                    writer.Write(serializedJson);
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
                        options.CurrentDataPath = myDocumentsWithDarwinPhotoId;
                    else
                        options.CurrentDataPath = myDocumentsPath;
                }
            }
        }
    }
}
