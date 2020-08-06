DARWIN Software (c)2005

Digital Archiving and Whale Recognition in a Network

This is software to facilitate the automatic recognition of toothed whales 
(currently Bottlenose Dolphins) from digital images of their dorsal fins.  
The software also supports stoorage of images and related sighting data in
an easily accessible database.

Software is written in C++ and has been successfully built for execution
on PC/Windows and Linux platforms.

To set up the software source for building on a PC running Windows ...

Download and unzip the darwinXX.zip file in a convenient location.

The following directory/file structure should exist.

darwin                              -- the main folder
   READMEwin32.txt                  -- this README file

   darwin-0.4.2                     -- the darwin software folder
      filters
      intl
      msvc                          -- the MS Visual C++ 6.0 workspace & project files
      pixmaps
      po
      src                           -- the C++ source file tree
      .....                         -- the Linux build files (configure.in, Makefile.am, ...)

   gtk                              -- GTK+ related runtime and developer folders
      include
      lib
      etc
      bin

   pix                              -- dolphin images

NOTES:
======
1 - The C++ project is set up to expect all of the GTK+ related files to be in 
the current location within the distribution.  Needed include and lib paths are
specified in the project settings.  You should be able to compile and link without
changing any global Visual Studio settings.

(a) The paths for the includes are specified in the ...
    "Project/Settings/C++/Category:Preprocessor/Additional Inclulde Directories"
(b) The paths for the libraries are specified in the ...
    "Project/Settings/Link/Category:Input/Additional Library Path"

2 - In order to execute the resulting program you will have to set your Path 
environment variable in Windows to include the path to the gtk\bin folder.  On 
our machines we actually copy the entire gtk folderto c:\gtk and the add 
c:\gtk\bin to our Path.

(a) After changing your Path you will most likely need to exit Visual Studio and 
restart it in order to run the darwin exedcutable from within the IDE.  Visual 
Studio does not seem to recognize the change without the restart.

3 - The versions of GTK+ and related packages we are currently using are ...

atk-1.8.0
atk-dev-1.8.0
gettext-runtime-0.13.1
glib-2.4.7
glib-dev-2.4.7
gtk+-2.4.14
gtk+-dev-2.4.14
libiconv-1.9.1.bin.woe32
pango-1.4.1
pango-dev-1.4.1

All of these are available in complete form from http://gtk.org

