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
    public class FeaturePoint : BaseEntity, INotifyPropertyChanged
    {
        public static readonly FeaturePoint Empty = new FeaturePoint { IsEmpty = true };

        private string _name;
        private FeaturePointType _type;
        private bool _userSetPosition;
        private bool _isEmpty;
        private bool _ignore;

        public bool IsEmpty
        {
            get => _isEmpty;
            set
            {
                _isEmpty = value;
                RaisePropertyChanged("IsEmpty");
            }
        }

        public string Name
        {
            get => _name;
            set
            {
                _name = value;
                RaisePropertyChanged("Name");
            }
        }

        public FeaturePointType Type
        {
            get => _type;
            set
            {
                _type = value;
                RaisePropertyChanged("Type");
            }
        }

        public bool UserSetPosition
        {
            get => _userSetPosition;
            set
            {
                _userSetPosition = value;
                RaisePropertyChanged("UserSetPosition");
                IsEmpty = false;
            }
        }

        public bool Ignore
        {
            get => _ignore;
            set
            {
                _ignore = value;
                RaisePropertyChanged("Ignore");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public FeaturePoint()
        {
            IsEmpty = true;
        }

        public FeaturePoint(FeaturePoint featurePoint)
        {
            _name = featurePoint._name;
            _type = featurePoint._type;
            _userSetPosition = featurePoint._userSetPosition;
            _isEmpty = featurePoint._isEmpty;
            _ignore = featurePoint._ignore;
        }

        protected void RaisePropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}