// This file is part of DARWIN.
// Copyright (C) 1994 - 2020
//
// DARWIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DARWIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DARWIN.  If not, see<https://www.gnu.org/licenses/>.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Features
{
    public enum FeatureType
    {
        LeadingEdgeAngle = 0,
        HasMouthDent = 1,
        BrowCurvature = 2,
        NasionDepth = 3
    }

    public class Feature : BaseEntity, INotifyPropertyChanged
    {
        public static readonly Feature Empty = new Feature { IsEmpty = true };
        private bool _isEmpty;
        private FeatureType _type;
        private string _name;
        private double? _value;

        public string Name
        {
            get => _name;
            set
            {
                _name = value;
                RaisePropertyChanged("Name");
                IsEmpty = false;
            }
        }

        public FeatureType Type
        {
            get => _type;
            set
            {
                _type = value;
                RaisePropertyChanged("Type");
                IsEmpty = false;
            }
        }

        public double? Value
        {
            get => _value;
            set
            {
                _value = value;
                RaisePropertyChanged("Value");
                IsEmpty = false;
            }
        }

        public bool IsEmpty
        {
            get => _isEmpty;
            set
            {
                _isEmpty = value;
                RaisePropertyChanged("IsEmpty");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public Feature()
        {
            IsEmpty = true;
        }

        protected virtual void RaisePropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
