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
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Text;

namespace Darwin.Collections
{
    public class ObservableNotifiableCollection<T> :
                ObservableCollection<T> where T : INotifyPropertyChanged
    {
        private bool _disableEvents = false;

        public ItemPropertyChangedEventHandler ItemPropertyChanged;
        public EventHandler CollectionCleared;

        public ObservableNotifiableCollection()
            : base()
        { }

        public ObservableNotifiableCollection(IEnumerable<T> collection)
            : base(collection)
        { }

        public ObservableNotifiableCollection(List<T> list)
            : base(list)
        { }

        protected override void OnCollectionChanged(NotifyCollectionChangedEventArgs args)
        {
            if (_disableEvents)
                return;

            base.OnCollectionChanged(args);

            if (args.NewItems != null)
                foreach (INotifyPropertyChanged item in args.NewItems)
                    item.PropertyChanged += OnItemPropertyChanged;

            if (args.OldItems != null)
                foreach (INotifyPropertyChanged item in args.OldItems)
                    item.PropertyChanged -= OnItemPropertyChanged;
        }

        protected void OnItemPropertyChanged(object sender, PropertyChangedEventArgs args)
        {
            if (_disableEvents)
                return;

            ItemPropertyChanged?.Invoke(this, new ItemPropertyChangedEventArgs(sender, args.PropertyName));
        }

        public void DisableEvents()
        {
            _disableEvents = true;
        }

        protected override void ClearItems()
        {
            foreach (INotifyPropertyChanged item in Items)
                item.PropertyChanged -= OnItemPropertyChanged;

            CollectionCleared?.Invoke(this, EventArgs.Empty);

            base.ClearItems();
        }
    }
}
