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
