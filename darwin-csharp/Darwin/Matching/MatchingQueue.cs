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
                            var areaPath = Path.Combine(Options.CurrentUserOptions.CurrentDataPath, splitLine[1]);

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

            string summary = GetSummary();
            string summaryFilename = Path.Combine(directoryName, "results-summary");
            File.WriteAllText(summaryFilename, summary);

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

        public string GetSummary()
        {
            if (Fins == null || Matches == null)
                return string.Empty;

            StringBuilder sb = new StringBuilder();

            sb.AppendLine();
            sb.AppendLine("Matching completed.");
            sb.Append("\tAverage time per match: " + ((NumValidTimes == 0) ? "[None]" : (TotalTime / NumValidTimes).ToString()) + " over ");

            if (NumValidTimes == 0)
                sb.AppendLine("no valid times.");
            else if (NumValidTimes == 1)
                sb.AppendLine("1 valid time.");
            else
                sb.AppendLine(NumValidTimes + " valid times.");

            if (NumInvalidTimes > 0)
            {
                sb.Append("Warning: ");

                if (NumInvalidTimes == 1)
                    sb.AppendLine("1 set of results didn't have a valid time entered.");
                else
                    sb.AppendLine(NumInvalidTimes + " sets of results didn't have valid times entered.");
            }

            sb.AppendLine();

            var finsWithID = Fins.Where(f => !string.IsNullOrEmpty(f.IDCode)).ToList();
            int rankingSum = 0;
            int numFinsWithID = 0;
            int numTop10 = 0;
            int bestRank = int.MaxValue;
            int worstRank = int.MinValue;
            foreach (var finWithID in finsWithID)
            {
                int index = Fins.IndexOf(finWithID);

                var firstSameIDMatch = Matches[index]?.MatchResults?.Results?
                    .Where(r => !string.IsNullOrEmpty(r.IDCode) && r.IDCode.ToLower().Trim() == finWithID.IDCode.ToLower().Trim())
                    .FirstOrDefault();

                if (firstSameIDMatch != null)
                {
                    numFinsWithID += 1;
                    // Our index is 0 based, but we want rank to start at 1, so we add 1
                    int rank = Matches[index].MatchResults.Results.IndexOf(firstSameIDMatch) + 1;
                    sb.Append(Path.GetFileName(finWithID.FinFilename));
                    sb.Append(": The ID is ranked ");
                    sb.AppendLine(rank.ToString());
                    rankingSum += rank;

                    if (rank <= 10)
                        numTop10 += 1;

                    if (rank < bestRank)
                        bestRank = rank;
                    if (rank > worstRank)
                        worstRank = rank;
                }
            }

            if (numFinsWithID > 0)
            {
                sb.AppendLine();

                if (numFinsWithID == 1)
                    sb.AppendLine("Out of 1 fin with an ID");
                else
                    sb.AppendLine("Out of " + numFinsWithID + " fins with IDs");

                sb.AppendLine("\tAverage rank: " + (float)rankingSum / numFinsWithID);

                if (numTop10 == 0)
                    sb.AppendLine("\tNo fins ranked in the top ten.");
                else if (numTop10 == 1)
			        sb.AppendLine("\t1 fin (" + (float)1 / numFinsWithID * 100.0 + "%) ranked in the top ten.");
                 else
			        sb.AppendLine("\t" + numTop10 + " fins (" + (float)numTop10 / numFinsWithID * 100.0 + "%) ranked in the top ten.");

                sb.AppendLine();

				sb.AppendLine("\tBest rank: " + bestRank);
                sb.AppendLine("\tWorst rank: " + worstRank);
                sb.AppendLine();
            }

            if (Fins.Count - numFinsWithID == 0)
		        sb.AppendLine("All fins had an ID provided.");
            else if (Fins.Count - numFinsWithID == 1)
		        sb.AppendLine("1 fin with no ID provided." );
            else
		        sb.AppendLine((Fins.Count - numFinsWithID) + " fins with no ID provided.");

            return sb.ToString();
        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
