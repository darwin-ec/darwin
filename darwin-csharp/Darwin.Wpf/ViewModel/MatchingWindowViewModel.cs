using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace Darwin.Wpf.ViewModel
{
    public class MatchingWindowViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private void RaisePropertyChanged(string propertyName)
        {
            var handler = PropertyChanged;
            if (handler == null) return;

            handler(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
