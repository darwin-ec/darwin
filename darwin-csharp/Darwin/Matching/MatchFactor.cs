using Darwin.Database;
using Darwin.Features;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Matching
{
    public delegate MatchError ErrorBetweenIndividualOutlinesDelegate(
        List<FeaturePointType> contourControlPoints,
        ErrorBetweenOutlinesDelegate errorBetweenOutlines,
        UpdateDisplayOutlinesDelegate updateOutlines,
        DatabaseFin unknownFin,
        DatabaseFin databaseFin,
        MatchOptions options);

    public delegate MatchError ErrorBetweenIndividualFeaturesDelegate(
            RatioComparison ratioComparison,
            DatabaseFin unknownFin,
            DatabaseFin databaseFin,
            MatchOptions options);

    public delegate MatchError ErrorBetweenIndividualFeaturePointsDelegate(
        List<FeaturePointType> contourControlPoints,
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

    public enum MatchFactorType
    {
        Outline = 0,
        FeaturePoint = 1
    }

    public class MatchFactor : INotifyPropertyChanged
    {
        private MatchFactorType _matchFactorType;
        public MatchFactorType MatchFactorType
        {
            get => _matchFactorType;
            set
            {
                _matchFactorType = value;
                RaisePropertyChanged("MatchFactorType");
            }
        }
        private ErrorBetweenIndividualOutlinesDelegate _errorBetweenIndividualOutlines;
        public ErrorBetweenIndividualOutlinesDelegate ErrorBetweenIndividualOutlines
        {
            get => _errorBetweenIndividualOutlines;
            set
            {
                _errorBetweenIndividualOutlines = value;
                RaisePropertyChanged("ErrorBetweenIndividualOutlines");
            }
        }

        private ErrorBetweenIndividualFeaturesDelegate _errorBetweenIndividualFeatures;
        public ErrorBetweenIndividualFeaturesDelegate ErrorBetweenIndividualFeatures
        {
            get => _errorBetweenIndividualFeatures;
            set
            {
                _errorBetweenIndividualFeatures = value;
                RaisePropertyChanged("ErrorBetweenIndividualFeatures");
            }
        }

        private List<FeaturePointType> _contourControlPoints;
        public List<FeaturePointType> ContourControlPoints
        {
            get => _contourControlPoints;
            set
            {
                _contourControlPoints = value;
                RaisePropertyChanged("ContourControlPoints");
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

        private RatioComparison _ratioComparison;
        
        public event PropertyChangedEventHandler PropertyChanged;

        // Prevent this from being created without using one of the factory methods below
        private MatchFactor()
        { }

        public static MatchFactor CreateOutlineFactor(
            float weight,
            List<FeaturePointType> contourControlPoints,
            ErrorBetweenOutlinesDelegate errorMethod,
            ErrorBetweenIndividualOutlinesDelegate errorBetweenIndividuals,
            MatchOptions options = null)
        {
            return new MatchFactor
            {
                MatchFactorType = MatchFactorType.Outline,
                Weight = weight,
                ContourControlPoints = contourControlPoints,
                ErrorBetweenOutlines = errorMethod,
                ErrorBetweenIndividualOutlines = errorBetweenIndividuals,
                UpdateOutlines = null,
                MatchOptions = options
            };
        }

        public static MatchFactor CreateOutlineFactor(
            float weight,
            List<FeaturePointType> contourControlPoints,
            ErrorBetweenOutlinesDelegate errorMethod,
            ErrorBetweenIndividualOutlinesDelegate errorBetweenIndividuals,
            UpdateDisplayOutlinesDelegate updateOutlines,
            MatchOptions options = null)
        {
            return new MatchFactor
            {
                MatchFactorType = MatchFactorType.Outline,
                Weight = weight,
                ContourControlPoints = contourControlPoints,
                ErrorBetweenOutlines = errorMethod,
                ErrorBetweenIndividualOutlines = errorBetweenIndividuals,
                UpdateOutlines = updateOutlines,
                MatchOptions = options
            };
        }

        public static MatchFactor CreateFeaturePointFactor(
            float weight,
            List<FeaturePointType> benchmarkFeatures,
            List<FeaturePointType> landmarkFeatures,
            int numberOfDesiredRatios,
            List<DatabaseFin> allDatabaseIndividuals,
            ErrorBetweenIndividualFeaturesDelegate errorBetweenIndividualFeatures,
            MatchOptions options = null)
        {
            var ratioComparison = FeaturePointErrorFunctions.ComputeInitialEigenRatios(
                benchmarkFeatures,
                landmarkFeatures,
                numberOfDesiredRatios,
                allDatabaseIndividuals);

            return new MatchFactor
            {
                _ratioComparison = ratioComparison,
                MatchFactorType = MatchFactorType.FeaturePoint,
                Weight = weight,
                ErrorBetweenIndividualFeatures = errorBetweenIndividualFeatures,
                MatchOptions = options
            };
        }

        public MatchError FindErrorBetweenIndividuals(DatabaseFin unknownFin, DatabaseFin databaseFin)
        {
            if (MatchFactorType == MatchFactorType.Outline)
                return ErrorBetweenIndividualOutlines(ContourControlPoints, ErrorBetweenOutlines, UpdateOutlines, unknownFin, databaseFin, MatchOptions);

            if (MatchFactorType == MatchFactorType.FeaturePoint)
                return ErrorBetweenIndividualFeatures(_ratioComparison, unknownFin, databaseFin, MatchOptions);

            throw new NotImplementedException();
        }

        protected virtual void RaisePropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
