using Darwin.Database;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Matching
{
    public delegate MatchError ErrorBetweenIndividualsDelegate(
        ErrorBetweenOutlinesDelegate errorBetweenOutlines,
        UpdateDisplayOutlinesDelegate updateOutlines,
        DatabaseFin unknownFin,
        DatabaseFin databaseFin,
        MatchOptions options);

    public delegate double ErrorBetweenOutlinesDelegate(
                            FloatContour c1, // Mapped unknown fin 
                            int begin1,
                            int mid1,
                            int end1,
                            FloatContour c2, // Evenly spaced database fin
                            int begin2,
                            int mid2,
                            int end2);

    public delegate void UpdateDisplayOutlinesDelegate(FloatContour unknownContour, FloatContour databaseContour);

    public class MatchFactor : INotifyPropertyChanged
    {
        private ErrorBetweenIndividualsDelegate _errorBetweenIndividuals;
        public ErrorBetweenIndividualsDelegate ErrorBetweenIndividuals
        {
            get => _errorBetweenIndividuals;
            set
            {
                _errorBetweenIndividuals = value;
                RaisePropertyChanged("ErrorBetweenIndividuals");
            }
        }

        private ErrorBetweenOutlinesDelegate _errorBetweenOutlines;
        public ErrorBetweenOutlinesDelegate ErrorBetweenOutlines
        {
            get => _errorBetweenOutlines;
            set
            {
                _errorBetweenOutlines = value;
                RaisePropertyChanged("ErrorBetweenOutlines");
            }
        }

        private UpdateDisplayOutlinesDelegate _updateOutlines;
        public UpdateDisplayOutlinesDelegate UpdateOutlines
        {
            get => _updateOutlines;
            set
            {
                _updateOutlines = value;
                RaisePropertyChanged("UpdateOutlines");
            }
        }

        private MatchOptions _matchOptions;
        public MatchOptions MatchOptions
        {
            get => _matchOptions;
            set
            {
                _matchOptions = value;
                RaisePropertyChanged("MatchOptions");
            }
        }

        private float _weight;
        public float Weight
        {
            get => _weight;
            set
            {
                _weight = value;
                RaisePropertyChanged("Weight");
            }
        }
        
        public event PropertyChangedEventHandler PropertyChanged;

        // Prevent this from being created without using one of the factory methods below
        private MatchFactor()
        { }

        public static MatchFactor CreateOutlineFactor(
            float weight,
            ErrorBetweenOutlinesDelegate errorMethod,
            ErrorBetweenIndividualsDelegate errorBetweenIndividuals,
            UpdateDisplayOutlinesDelegate updateOutlines,
            MatchOptions options = null)
        {
            return new MatchFactor
            {
                Weight = weight,
                ErrorBetweenOutlines = errorMethod,
                ErrorBetweenIndividuals = errorBetweenIndividuals,
                UpdateOutlines = updateOutlines,
                MatchOptions = options
            };
        }

        public MatchError FindErrorBetweenIndividuals(DatabaseFin unknownFin, DatabaseFin databaseFin)
        {
            return ErrorBetweenIndividuals(ErrorBetweenOutlines, UpdateOutlines, unknownFin, databaseFin, MatchOptions);
        }


        protected virtual void RaisePropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
