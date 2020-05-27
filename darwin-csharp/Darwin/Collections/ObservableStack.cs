// CC BY-SA 4.0 https://creativecommons.org/licenses/by-sa/4.0/
// Based on the StackOverflow answer by Ernie S at
// https://stackoverflow.com/questions/3127136/observable-stack-and-queue

using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Text;

namespace Darwin.Collections
{
    public class ObservableStack<T> : Stack<T>, INotifyCollectionChanged, INotifyPropertyChanged
    {
        #region Constructors

        public ObservableStack() : base() { }

        public ObservableStack(IEnumerable<T> collection) : base(collection) { }

        public ObservableStack(int capacity) : base(capacity) { }

        #endregion

        #region Overrides

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

        #endregion

        #region CollectionChanged

        public virtual event NotifyCollectionChangedEventHandler CollectionChanged;

        protected virtual void OnCollectionChanged(NotifyCollectionChangedAction action, T item)
        {
            CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(
                action
                , item
                , item == null ? -1 : 0)
            );

            OnPropertyChanged(nameof(Count));
        }

        #endregion

        #region PropertyChanged

        public virtual event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged(string proertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(proertyName));
        }

        #endregion
    }
}
