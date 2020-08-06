# DARWIN

You can read more about DARWIN at [http://darwin.eckerd.edu/](http://darwin.eckerd.edu/).

DARWIN is a software system which allows marine scientists to maintain information for the study of various behavioral and ecological patterns of bottlenose dolphins, *Tursiops truncatus*. The software provides a graphical user interface to access a collection of digital dorsal fin images along with textual information which describes individual animals as well as relevant sighting data. Users may query the system with the name of a specific individual or the entire collection may be sorted and viewed based upon sighting location, sighting date, or damage category. Alternatively, the researcher may query a database of previously identified dolphin dorsal fin images with an image of an unidentified dolphin's fin. DARWIN responds with a rank ordered list of database fin images that most closely resemble the query image.

More recent support has been added for Alaskan brown bears, *Ursus arctos*, and is still in progress.

There is a sample catalog in the **sample** folder in the GitHub repo.  You can open it in DARWIN by picking **sample\catalog\sampleNew.db** in the Open Database.

## Building

The current version of DARWIN is mostly C# and WPF, and is based/ported on the previous C++ version.  The core class libraries are .NET standard, but the WPF frontend is Windows only.  DARWIN has been tested with [Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/).  To build and run the project, open **Darwin.sln** in the top level directory, and run the **Darwin.WPF** project.

The previous version of DARWIN can be found under the cpp-version folder.  The previous version is C++/C with a GTK+ frontend, and should build on Windows, macOS, and Linux.  Please read the **DarwinDeveloperDocs.txt** file in that folder for more detailed instructions.

## License

DARWIN is licensed under the [GNU General Public License, Version 3](https://www.gnu.org/licenses/gpl-3.0.html).