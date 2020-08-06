DARWIN PhotoID Software -- Version 2.22 (darwinMac.App) -- README

This software was originally developed for a Windows platform.  This is the
first Macintosh version.  It should install and run successfully on any Mac
(32 or 64 bit) running MacOSx 10.4 or later.  Primary testing has been
on a Macbook Pro running MacOSx 10.6.8.  It has not yet been tested on Lion.

The DARWIN PhotoID software is primarily intended as an aid to biologists
performing photo-identification of bottlenose dolphins (Trusiops truncatus).
It has been used successfully with numerous other dolphin, shark and whale 
species, where the primary identification is through the shape and features
along the profile edge of the dorsal fin. The Windows versions of DARWIN 
have been used in the field and the lab since version 1.0 (2005) to study 
popultions small and large.  At Eckerd College a DARWIN catalog of more than
600 individuals is the primary Photo-identifiction catalog, and has been 
for several years.

DARWIN allows the user to ...

Load dorsal fin images
Perform image enhancements
  Reorient, adjust contrast & brightness, crop, ...
Save enhanced images
Create a representiation of the fin outline
  This may be done automatically, or manually
Correct placement of the outline and its features
Enter basic sighting data related to each image
Save results of above as a FIN (finz) file
  Containing both images (original and enhanced), outline & sighting data
Add FINs to a catalog (database)
Match FINs to any existing catalog to determine Identity
  This is done either interactively or as a queued (batch) process
  Searches for Identity (Matching) can be restricted by CATALOG Category
    or may be performed against the entire CATALOG  
Review results of catolog matching
  This is through a prioritized list of candidates matching a new FIN
Save match results for review at a later time
Build lists of new FINs to be matched in an unattended, batch manner
  All match results for all matches are saved for later review
Create new SURVEY AREAs and CATALOGs
Review, modify, add and delete FINs from any CATALOG
Export sighting data from a CATALOG
  Tab separated spreadsheet readable format (Suitable for SOCPROG, etc)
Export individual FINs from a catalg
Export and Import entire CATALOGs to/from other DARWIN versions
Create and Restore from BACKUPs
View previously savd FINs
Load and match previously saved FINs
Create, save and use any desired catalog category SCHEME
  The Eckerd College scheme is the default

====================
To INSTALL DarwinMac
====================

(1) Download "darwinMac2.22Installer.dmg" or copy it from disk or other media 
(2) Double-click the "darwinMac2.22Installer.dmg" icon to mount the disk image, 
and open the "darwinMac2.22Installer" itself.
  Since you are reading this you may already have performed steps 1 & 2.
(3) Drag the "darwinMac.app" to your Applications folder or wherever else you 
desire the application to reside.  DARWIN can be installed on and run from an 
external drive if desired. You may want to create an alias for DARWIN on your
desktop or elsewhere, as well.
(4) Close the installer and eject the installer disk mage.

================
To RUN DarwinMac
================

Simply click the "darwinMac.app" icon in your Applications folder, or 
double-click the darwinMac.app alias -- and DARWIN will start.

The DARWIN program is installed with a small Sample Catalog.  However, this
Sample Catalog will not open automatically.  You may use the "File>Open Database"
menu option to bring up a FileChooser window.  Use that to locate the 
".../surveyAreas/sample/catalog/" folder, select the "sampleNew.db" file and
click "Open."

There are additional images of the same 10 dolphins in the folder named
".../surveyArea/sample/fin-images/"

All of the DARWIN folders were traditionally located within a single folder 
system having DARWINHOME as its root folder.  DARWINHOME was always within the 
application itself.  This has been changed in version 2.22, and you may now 
create any data location for use with DARWIN.  However, there is a remnant 
of the original folder structure within the "darwinMac.app/Content/MacOS/" 
folder until you run the application the first time.

==============
Run First Time
==============

When you run the darwinMac.app the first time, it will automatically create
the following folder structure in your Documents folder.

    darwinPhotoIdData
        surveyAreas
	    default
	    sample
        backups

The two survey areas have the same internal folders and sample data shown 
in the next section.  These are MOVED out of the application folder so
they will not be accidentally lost if you remove the application from 
your Mac.

=========================
Folders within DARWINHOME (the application MacOS folder)
=========================

    backups        - used in backing up database
    docs           - online user help and other documentation
    licenses-etc   - copies of license and right to use information
    system         - all runtime system files
        bin            - contains executable ("darwin") and 7z
        etc            - GTK+ and other
        lib            - GTK+ and other runtime library files
    surveyAreas    - contains all sighting and catalog related data
        sample         - sample survey area (catalog, images, etc)
            catalog       - folder for catalogs
            fin-images    - sample dorsal fin images
            matchQueues   - folder for queued matches 
            matchQResults - folder for saved results of fin matches
            sightings     - folder for sighting information (future use)
            tracedFins    - folder for saved fin tracings
        default        - default survey area (catalogs, sightings, data)
            catalog       - folder for catalogs
            matchQueues   - folder for queued matches 
            matchQResults - folder for saved results of fin matches
            sightings     - folder for sighting information (future use)
            tracedFins    - folder for saved fin tracings
    darwinMac-readme.txt  - another readme file

=============
Other details 
=============

From within DARWIN you can now create a DATA FOLDER in any location.  All 
DATA FOLDERs must end in a folder named "DarwinPhotoIdData" and may be
created manually, or from the File Save Dialog during any "Create New 
Database" or "Import Catalog" operation.

New SURVEY AREAs may be created as desired.  Creating a new SURVEY AREA 
will create an additional collection of folders similar to the "sample" 
SURVEY AREA shown above.

Within a given SURVEY AREA you may create as many CATALOGs as desired.
Each CATALOG has its own database file (*.db) and collection of images. All
sighting data and fin outlines are saved in the database file, while the
images are stored in the ctalog folder separately.

Any CATALOGs can be Backed-up or Exported as a zipped archive containing
the databae file and ALL related images.  Backups are named automatically
and are placed in the "backups" folder.  Exports may be named as desired
and may be saved anywhere.

All individually saved FINs (*.finz files) are saved in the current SURVEY 
AREA's tracedFins folder (for example, ".../surveyAreas/sample/tracedFins")
by default.

All MATCH QUEUEs (lists of FIN files for unattended matching) are saved
in the current SURVEY AREA's "matchQueues" folder and the results of
those matches are saved in the "matchQResults" folder.

When sighting data is Exported, the default location for saving the file
is in the "sightings" folder.

===============================
Reporting Issues - Getting Help
===============================

Please send comments, complaints, suggestions, and any other feedback to us
at "darwin@eckerd.edu" and visit our web site (http://darwin.eckerd.edu/) for
updates on software, conference and workshop notices, help and more.

We hope this software will assist you in your study of marine mammals.

John Stewman
Kelly Debure
(and host of student researchers and programmers)
DARWIN has been in constant development since 1993 and is free software.
