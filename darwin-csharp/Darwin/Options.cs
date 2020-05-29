using Microsoft.Extensions.Configuration;
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
        [DefaultValue(1.5f)]
        public float GaussianStdDev { get; set; }

        private const string OptionsFilename = "options.json";

        // Don't let this class be created directly with its constructor
        private Options() { }

        public static Options CurrentUserOptions
        {
            get
            {
                IsolatedStorageFile isoStore = IsolatedStorageFile.GetStore(IsolatedStorageScope.User | IsolatedStorageScope.Assembly, null, null);

                if (isoStore.FileExists(OptionsFilename))
                {
                    using (IsolatedStorageFileStream isoStream = new IsolatedStorageFileStream(OptionsFilename, FileMode.Open, isoStore))
                    {
                        var serializer = new JsonSerializer();

                        using (StreamReader reader = new StreamReader(isoStream))
                        {
                            using (var jsonTextReader = new JsonTextReader(reader))
                            {
                                return serializer.Deserialize<Options>(jsonTextReader);
                            }
                        }
                    }
                }

                return null;
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
    }
}
