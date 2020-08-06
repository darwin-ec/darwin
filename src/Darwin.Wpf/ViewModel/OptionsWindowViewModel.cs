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

namespace Darwin.Wpf.ViewModel
{
    public class OptionsWindowViewModel : INotifyPropertyChanged
    {
        private Options _options;
        public Options Options
        {
            get => _options;
            set
            {
                _options = value;
                RaisePropertyChanged("Options");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public OptionsWindowViewModel(Options options)
        {
            Options = new Options(options);
        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
