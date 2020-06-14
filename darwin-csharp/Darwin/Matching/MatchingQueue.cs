using Darwin.Collections;
using Darwin.Database;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;

namespace Darwin.Matching
{
    public class MatchingQueue : INotifyPropertyChanged
    {
        // TODO: These need to be populated
        public int NumValidTimes { get; set; }
        public int TotalTime { get; set; }
        public int NumInvalidTimes { get; set; }

        public int NumNoID { get; set; }

        private List<Match> _matches;
        public List<Match> Matches
        {
            get
            {
                if (_matches == null)
                    _matches = new List<Match>();

                return _matches;
            }
            set
            {
                _matches = value;
                RaisePropertyChanged("Matches");
            }
        }

        private ObservableNotifiableCollection<DatabaseFin> _fins;
        public ObservableNotifiableCollection<DatabaseFin> Fins
        {
            get
            {
                if (_fins == null)
                    _fins = new ObservableNotifiableCollection<DatabaseFin>();

                return _fins;
            }
            set
            {
                _fins = value;
                RaisePropertyChanged("Fins");
                CheckQueueRunnable();
            }
        }

        private bool _matchRunning;
        public bool MatchRunning
        {
            get => _matchRunning;
            set
            {
                _matchRunning = value;
                RaisePropertyChanged("MatchRunning");
                CheckQueueRunnable();
            }
        }

        private bool _queueRunnable;
        public bool QueueRunnable
        {
            get => _queueRunnable;
            set
            {
                _queueRunnable = value;
                RaisePropertyChanged("QueueRunnable");
            }
        }

        private DarwinDatabase _database;
        public DarwinDatabase Database
        {
            get => _database;
            set
            {
                _database = value;
                RaisePropertyChanged("Database");
            }
        }

        private RegistrationMethodType _registrationMethod;
        public RegistrationMethodType RegistrationMethod
        {
            get => _registrationMethod;
            set
            {
                _registrationMethod = value;
                RaisePropertyChanged("RegistrationMethod");
            }
        }

        private RangeOfPointsType _rangeOfPoints;
        public RangeOfPointsType RangeOfPoints
        {
            get => _rangeOfPoints;
            set
            {
                _rangeOfPoints = value;
                RaisePropertyChanged("RangeOfPoints");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public MatchingQueue(DarwinDatabase database, RegistrationMethodType registrationMethod, RangeOfPointsType rangeOfPoints)
        {
            Database = database;
            RegistrationMethod = registrationMethod;
            RangeOfPoints = rangeOfPoints;

        }

        public void CheckQueueRunnable()
        {
            if (_fins != null && _fins.Count > 0 && !MatchRunning)
            {
                QueueRunnable = true;
            }
            else
            {
                QueueRunnable = false;
            }
        }

        /// <summary>
        /// This method saves a Queue to a file.  It will overwrite a file if one already exists with the same name.
        /// </summary>
        /// <param name="filename"></param>
        public void SaveQueue(string filename)
        {
            if (string.IsNullOrEmpty(filename))
                throw new ArgumentNullException(nameof(filename));

            if (Fins.Count < 1)
                throw new IndexOutOfRangeException();

            // This will overwrite if the file already exists
            using (StreamWriter file = new StreamWriter(filename, false))
            {
                foreach (var fin in Fins)
                {
                    file.WriteLine("<full> " + fin.FinFilename);
                }
            }
        }

        public void LoadQueue(string filename)
        {
            if (string.IsNullOrEmpty(filename))
                throw new ArgumentNullException(nameof(filename));

            if (!File.Exists(filename))
                throw new Exception("Couldn't find file " + filename);

            Fins.Clear();
            MatchRunning = false;
            Matches.Clear();

            var lines = File.ReadAllLines(filename);

            if (lines != null)
            {
                foreach (var line in lines)
                {
                    var splitLine = line.Split(new char[] { ' ' });

                    if (splitLine.Length > 1)
                    {
                        string loadFilename = string.Empty;

                        if (splitLine[0] == "<full>" && File.Exists(splitLine[1]))
                        {
                            loadFilename = splitLine[1];
                        }
                        else if (splitLine[0] == "<area>")
                        {
                            var areaPath = Path.Combine(Options.CurrentUserOptions.CurrentSurveyAreasPath, splitLine[1]);

                            if (File.Exists(areaPath))
                                loadFilename = areaPath;
                        }
                        else if (splitLine[0] == "<home>")
                        {
                            var homePath = Path.Combine(Options.CurrentUserOptions.CurrentDarwinHome, splitLine[1]);

                            if (File.Exists(homePath))
                                loadFilename = homePath;
                        }

                        // Fallback
                        if (string.IsNullOrEmpty(loadFilename))
                        {
                            string filenameOnly = Path.GetFileName(splitLine[1]);
                            string fileInTracedFins = Path.Combine(Options.CurrentUserOptions.CurrentTracedFinsPath, filenameOnly);

                            if (File.Exists(fileInTracedFins))
                                loadFilename = fileInTracedFins;
                        }

                        // If we found the file, load it.  Otherwise, just ignore it.
                        if (!string.IsNullOrEmpty(loadFilename))
                        {
                            Fins.Add(CatalogSupport.OpenFinz(loadFilename));
                        }
                    }
                }
            }

            CheckQueueRunnable();
        }

        public void SaveMatchResults(string directoryName)
        {
            if (string.IsNullOrEmpty(directoryName))
                throw new ArgumentNullException(nameof(directoryName));

            if (!Directory.Exists(directoryName))
                throw new Exception("Couldn't find directory " + directoryName);

            var currentFiles = Directory.GetFiles(directoryName);

            if (currentFiles != null && currentFiles.Length > 0)
            {
                // If there are files already there, let's move them into a backup folder
                string backupDirectory = "Backup-" + DateTime.Now.ToString("yyyy-MM-dd-hh-mm-ss-ff");
                string fullBackupDirectory = Path.Combine(directoryName, backupDirectory);
                Directory.CreateDirectory(fullBackupDirectory);

                foreach (var oldFile in currentFiles)
                {
                    File.Move(oldFile, Path.Combine(fullBackupDirectory, Path.GetFileName(oldFile)));
                }
            }

            if (Matches != null && Matches.Count > 0)
            {
                foreach (var match in Matches)
                {
                    string filename = Path.GetFileNameWithoutExtension(Database.Filename) + "-DB-match-for-" + Path.GetFileNameWithoutExtension(match.UnknownFin.FinFilename) + ".res";
                    string filenameWithPath = Path.Combine(Options.CurrentUserOptions.CurrentMatchQueueResultsPath, filename);
                    match.MatchResults.Save(filenameWithPath);
                }
            }
        }

  //      public string GetSummary()
  //      {
  //          StringBuilder sb = new StringBuilder();

  //          sb.AppendLine();
  //          sb.AppendLine("Matching completed.");
  //          sb.Append("\tAverage time per match: " + ((NumValidTimes == 0) ? "[None]" : (TotalTime / NumValidTimes).ToString()) + " over ");

  //          if (NumValidTimes == 0)
  //              sb.AppendLine("no valid times.");
  //          else if (NumValidTimes == 1)
  //              sb.AppendLine("1 valid time.");
  //          else
  //              sb.AppendLine(NumValidTimes + " valid times.");

  //          if (NumInvalidTimes > 0)
  //          {
		//        sb.Append("Warning: ");

  //              if (NumInvalidTimes == 1)
  //                  sb.AppendLine("1 set of results didn't have a valid time entered.");
  //              else
		//	        sb.AppendLine(NumInvalidTimes + " sets of results didn't have valid times entered.");
  //          }

  //          //***1.2 - list files and theie individual rankings
  //          for (int idx = 0; idx < mNumID; idx++)
  //          {
  //              char numStr[16];
  //              ifstream inFile;

  //              //string path = getenv("DARWINHOME");
  //              //***1.85 - everything is now relative to the current survey area
  //              string path = gOptions->mCurrentSurveyArea;
  //              path += PATH_SLASH;
  //              path += "matchQResults";
  //              path += PATH_SLASH;
  //              string resultFilename = "results-unknown-";
  //              sprintf(numStr, "%d", idx);
  //              resultFilename += numStr;

  //              inFile.open((path + resultFilename).c_str());
  //              if (!inFile.fail())
  //              {
		//	out << resultFilename << ":";
  //                  string line;
  //                  getline(inFile, line); // fin ID
  //                  getline(inFile, line); // fin File
		//	out << line.substr(line.find_last_of(PATH_SLASH) + 1) << ":";
  //                  getline(inFile, line); // database File
  //                  getline(inFile, line); // ranking
		//	out << line << endl;
  //                  inFile.close();
  //              }
  //              inFile.clear();
  //          }

  //          if (mNumID > 0)
  //          {
		//out << endl;

  //              if (mNumID == 1)
		//	out << "Out of 1 fin with an ID" << endl;

  //      else
		//	out << "Out of " << mNumID << " fins with IDs" << endl;

  //              cout << "\tAverage rank: " << (float)mSum / mNumID << endl;

  //              if (mNumTopTen == 0)
		//	out << "\tNo fins ranked in the top ten.";


  //      else if (mNumTopTen == 1)
		//	out << "\t1 fin (" << (float)1 / mNumID * 100.0
  //               << "%) ranked in the top ten.";


  //      else
		//	out << "\t" << mNumTopTen << " fins ("
  //               << (float)mNumTopTen / mNumID * 100.0
  //               << "%) ranked in the top ten.";
					
		//	out << endl << endl;

  //              if (!mFirstRun)
  //              {
		//		out << "\tBest rank: " << mBestRank << endl
  //                   << "\tWorst rank: " << mWorstRank << endl
  //                   << endl;
  //              }
  //          }

  //          if (NumNoID == 0)
		//out << "All fins had an ID provided." << endl;

  //  else if (NumNoID == 1)
		//out << "1 fin with no ID provided." << endl;

  //  else
		//out << NumNoID << " fins with no ID provided." << endl;

  //          return sb.ToString();
  //      }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
