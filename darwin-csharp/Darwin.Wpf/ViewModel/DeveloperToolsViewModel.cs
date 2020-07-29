using Darwin.Database;
using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Wpf.ViewModel
{
    public class DeveloperToolsViewModel : BaseViewModel
    {
        private DarwinDatabase _database;
        public DarwinDatabase Database
        {
            get => _database;
            set
            {
                _database = value;
                RaisePropertyChanged("Database");
            }
        }

        public DeveloperToolsViewModel(DarwinDatabase database)
        {
            WindowTitle = "Developer Tools";
            Database = database;
        }
    }
}
