// Original license CC BY-SA 4.0 https://creativecommons.org/licenses/by-sa/4.0/
// Based on the StackOverflow answer by Ernie S at
// https://stackoverflow.com/questions/3127136/observable-stack-and-queue

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

using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;

namespace Darwin.Collections
{
    public class ObservableStack<T> : Stack<T>, INotifyCollectionChanged, INotifyPropertyChanged
    {

        public ObservableStack() : base() { }

        public ObservableStack(IEnumerable<T> collection) : base(collection) { }

        public ObservableStack(int capacity) : base(capacity) { }


        public virtual new T Pop()
        {
            var item = base.Pop();
            OnCollectionChanged(NotifyCollectionChangedAction.Remove, item);

            return item;
        }

        public virtual new void Push(T item)
        {
            base.Push(item);
            OnCollectionChanged(NotifyCollectionChangedAction.Add, item);
        }

        public virtual new void Clear()
        {
            base.Clear();
            OnCollectionChanged(NotifyCollectionChangedAction.Reset, default);
        }

        public virtual event NotifyCollectionChangedEventHandler CollectionChanged;

        protected virtual void OnCollectionChanged(NotifyCollectionChangedAction action, T item)
        {
            CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(
                action
                , item
                , item == null ? -1 : 0)
            );

            RaisePropertyChanged(nameof(Count));
        }

        public virtual event PropertyChangedEventHandler PropertyChanged;

        protected virtual void RaisePropertyChanged(string proertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(proertyName));
        }
    }
}
