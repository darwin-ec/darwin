using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Database
{
    public class Category : BaseEntity, INotifyPropertyChanged
    {
        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                _name = value;
                RaisePropertyChanged("Name");
            }
        }

        private bool _isSelected;
        public bool IsSelected
        {
            get => _isSelected;

            set
            {
                _isSelected = value;
                RaisePropertyChanged("IsSelected");
            }
        }

        private int _order;
        public int Order
        {
            get => _order;
            
            set
            {
                _order = value;
                RaisePropertyChanged("Order");
            }
        }

        public Category()
        {

        }

        public Category(string name)
        {
            Name = name;
        }

        public Category(Category category)
        {
            ID = category.ID;
            Name = category.Name;
            IsSelected = category.IsSelected;
            Order = category.Order;
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
