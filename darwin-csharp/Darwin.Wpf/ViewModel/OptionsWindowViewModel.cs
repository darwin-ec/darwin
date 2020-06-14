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
