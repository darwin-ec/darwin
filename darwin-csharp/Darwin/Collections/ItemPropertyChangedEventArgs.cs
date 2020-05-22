using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Collections
{
    public class ItemPropertyChangedEventArgs : PropertyChangedEventArgs
    {
        object item;

        public ItemPropertyChangedEventArgs(object item,
                                            string propertyName)
            : base(propertyName)
        {
            this.item = item;
        }

        public object Item
        {
            get { return item; }
        }
    }

    public delegate void ItemPropertyChangedEventHandler(object sender,
                                        ItemPropertyChangedEventArgs args);
}
