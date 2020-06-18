using Darwin.Collections;
using Darwin.Database;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Text;

namespace Darwin.Wpf.ViewModel
{
    public class CatalogSchemesViewModel : INotifyPropertyChanged
    {
        private ObservableNotifiableCollection<CatalogScheme> _catalogSchemes;
        public ObservableNotifiableCollection<CatalogScheme> CatalogSchemes
        {
            get => _catalogSchemes;
            set
            {
                _catalogSchemes = value;
                RaisePropertyChanged("CatalogSchemes");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        public CatalogSchemesViewModel()
        {

        }

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
