----------------------------------------------------------
DARWIN 2.22 Beta Software for Macintosh (README FILE) -- 10/25/2011 - JHS
----------------------------------------------------------
This is a freely distributable version of the DARWIN-2.22 software.

DARWIN (Digital Analysis and Recognition of Whales Images on a Network) has
been under development at Eckerd College, St. Petersburg, FL, since 1993.
Several research versions have been presented at conferences and workshops
over the intervening years.

The software was first demonstrated at the 11th Biennial Conference on the
Biology of Marine Mammals, held in Orlando, FL, in 1995.

This software is provided for your use and evaluation.  Copies may be made
and the software may be in installed on any machine, free of charge.

CAUTION: THIS SOFTWARE IS UNDER DEVELOPMENT.  ALTHOUGH WE WILL MAKE EVERY
EFFORT TO MAINTAIN UPWARD PORTABILITY, FUTURE VERSIONS ARE NOT GUARANTEED TO
BE COMPATIBLE WITH VERSION 2.0 (beta). NO WARRANTEES OR GUARANTEES OF ANY
KIND ARE GIVEN OR IMPLIED.

Please feel free to send comments, critiques and suggestions to the DARWIN
Development Group at darwin@eckerd.edu

SYSTEM REQUIREMENTS
-------------------
This software was originally designed and tested on both Windows XP and Windows
2000 platforms.  This and future versions are also available for use on a 
Macintosh computer running OsX 10.4 or higher.  It has been tested on OsX 10.6.

The installer is provided as a disk image (darwinMac2.22Installer.dmg)

Minimum System
    CPU: Intel based Mac
    RAM: 2+ GB recommended
    Hard Drive: 35 MB required for minimal installation
    Display: 1024 x 768 with 24-bit color graphics card
    Operating System: OsX 10.4 or higher.

INSTALLATION
------------
DARWIN 2.22 now comes with a simple disk image installer.  You may obtain
the installer (darwinMac2.22Installer.dmg) from the DARWIN web site or may copy 
it from a CD or other media.  Double-click "darwinMac2.22Installer.dmg" to mount 
it, drag the "darwinMac.app" application wherever you would like, and you are 
set to run DARWIN.

The darwinMac.app has been successfully run from the Desktop, an individual user's
Applications folder, and a thumb drive.

All data is currently held within the darwinMac.app internal folder structure, 
unless data (catalogs, fins, sighting data, etc) is exported.

The created installation darwinMac.app folder contains the following in its
Contents/MacOS folder ....

    backups        - used in backing up database
    docs           - online user help and other documentation
    licenses-etc   - copies of license and right to use information
    system         - all runtime system files
        bin            - contains executable ("darwin.exe") and GTK+ dlls
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
    darwinMac-readme.txt  - this file

RUNNING Darwin
--------------

    DARWIN may be started several ways, but the best way is to simply 
    double-click "darwinMac.app"

    All versions of DARWIN since v2.0 use a NEW method of storing saved 
    Fin Traces. These .FINZ files each contain the Fin Trace, a copy of 
    the original image, and a modified image reflecting any changes made 
    using DARWIN's Trace Window routines.  You may double-click on any 
    .FINZ file and DARWIN will open as Fin Viewer.

    NOTE: The opening of FINZ files on the Mac is still being debugged.

    DARWIN no longer depends on any environment variables.  If you have
    a previous installation and have set an environment variable called
    DARWINHOME, you will probably want to remove it from your environment
    variable list.  

    The program will create an initial configuration file ".darwinrc" 
    in the system folder the first time it runs.  This file contains 
    various configurable color values, active contour constants, etc.  
    Most importantly for the user, it saves the name and location of 
    all survey areas, catalogs, and catalog category naming schemes. 
    YOU MAY LOSE ACCESS TO CATALOGS OR DATA IF YOU MODIFIED THIS FILE 
    MANUALLY.  The only catalog category scheme included with the 
    software is the Eckerd College scheme.  However, you may create, 
    save and use additional schemes to create your own databases.

    IF THIS IS THE ***FIRST*** INSTALLATION OF DARWIN YOU HAVE USED ...

    The program will open the first time without a database.  We recommend 
    that you use the menu option "File/Open Database" to locate and open
    the "surveyAreas/sample/catalog/sampleNew.db" database.  There are also 
    "surveyAreas/sample/fin-images" which you may use to familiarize
    yourself with the software and its usage.

    When you are ready, you may use the menu option "File/New Database" to
    create a database of your own. You may create this database in the 
    "surveyAreas/default" folder or you may create a totally different survey
    area and database of your own.  The catalog category naming scheme you
    desire to use may be selected from a pull-down menu.  If you want to
    create your own scheme, do so by using the main window menu option 
    "Settings/Catalog Scheme/Define New Scheme" before you create the new
    database.  Once a new database is created, its category names are fixed
    and cannot be changed.
    
    IF YOU ***UPGRADED*** YOUR PREVIOUS VERSION OF DARWIN ...

    DARWIN will read your existing configuration file, and will attempt
    to open the database you last used with the OLDER version.  The database
    format has changed significantly, so you will be prompted to BACKUP and
    CONVERT your database to the new database format.  If you performed a
    BACKUP prior to INSTALLING DARWIN 2.0, then you can uncheck the
    backup related options.  Click OK to perform the conversion.  All
    subsequent access to this catalog (database) will be performed using
    DARWIN 2.0 and this new SQL compatible database.

    IF YOU ***INSTALLED*** DARWIN 2.0 IN A DIFFERENT FOLDER THAN YOUR OLDER 
    VERSION ...

    DARWIN 2.0 will create a new configuration file and will open without
    any open catalog.  Use the "File/Import/Import Catalog" menu option to
    open a file chooser, then find your BACKUP or EXPORT archive in your
    OLDER DARWIN folder's "backups" folder, and click "Open."  You will then
    be required to name a new Survey Area within which to complete
    the IMPORT of your catalog and then Click "OK."  The catalog will be
    imported into your new Survey Area. Since the catalog is being imported 
    from the older version of DARWIN, you will now be prompted to BACKUP and 
    CONVERT the database file to the new SQL compatible format.  Un-select 
    the backup options and click "OK."  Your catalog will now open normally, 
    and you can use it as you previously did using the older version of 
    DARWIN.

    WARNING: Do NOT open an OLD database in your old installation folder
    by using the "File/Open Database" option from DARWIN 2.0.  Although the
    conversion and opening will occur correctly, the location of the "new"
    database will still be in the "old" location, and will in fact destroy
    the "old" version of the database file.  This means you will NOT be able
    to go back and use the older version of DARWIN on the "old" database
    if you so desire in the future.  ALWAYS use the IMPORT process to move
    a catalog from an older version into DARWIN 2.0.

    NOTE: All catalogs (databases) created within a given survey area will
    share the same "catalog" folder.  This means that their database files
    and cataloged images will be mixed within the same folder.

    Whenever the program runs, several AUTOMATIC backups are created.  If you
    have a program crash and lose your configuration or a database, these 
    files may be useful in restoring your system.

    system/.darwinrc~
        a copy of the last successfully loaded configuration file.  If the
        program cannot find "system/.darwinrc" or if it is corrupted, the
        program will automatically restore "system/.darwinrc" from 
        "system/.darwinrc~" -- If this fails a new initial distribution
        "system/.darwinrc" file is created.  You may rebuild the list of
        KNOWN databases, by opening each in turn the next time you run the
        program.  Category naming schemes will have to be manually rebuilt
        and saved.

    system/lastLoadOf_xxx_yyy.db
        a copy of the "surveyAreas/xxx/catalog/yyy.db" database file made
        as it was last loaded.  This copy does not contain any changes made 
        to the database while the program was running.

        If the "surveyAreas/xxx/catalog/yyy.db" file is lost or becomes
        corrupted, you may manually restore the catalog (database) to the
        same state it was in when last loaded by manually replacing the 
        "surveyAreas/xxx/catalog/yyy.db" file with a COPY of the
        "system/lastLoadOf_xxx_yyy.db" file.

        If you have a more recent FULL BACKUP, you can accomplish the same
        thing by using the menu option "File/Restore" in the main window.
        FULL BACKUPS include all associated catalog images as well as the 
        database (*.db) file.

Help / Documentation
--------------------
The "docs" folder contains an HTML User's Guide ("usersguide.htm").  You can
open it by double-clicking it, or you can access it from the menu option
"Help/Documentation" in the Main Window of the program.

Sample Catalog
--------------

There is a sample catalog included in the "surveyAreas/samples" folder. You
may find it useful to open this sample the first time you use the software.
All sample dolphin images have been provided courtesy of the Eckerd College
Dolphin Project.

Supported Image File Formats
----------------------------
The program currently reads JPEG, BMP, PNG, PNM, PGM(ASCII & Raw formats)
and PPM (Raw format) image files.  Other image formats will be supported
in future versions of the software as needs are identified.

EXIF data from JPEG files are automatically extracted and used to fill
sighting data entries wherever possible.  Currently the image creation
date/time is extracted and displayed when a new image is loaded into the
Trace Window for processing.

Backup & Restore
----------------
These processes have been provided in the software since version 1.85.
From the main window the user may make a backup of the CURRENTLY LOADED
database using the menu option "File/Backup."  The backup is created in
a standard location and is named automatically.  

Given the currently loaded database ...

    surveyAreas/default/catalog/darwin.db

a backup created today (5/15/2007) would be created as a zipped archive
containing the database file "darwin.db" and all associated images, and this
archive would be saved in the "backups" folder as ...

    backups/default_darwin_MAY_15_2007.zip

any subsequent backups created the same day would be stored with numeric 
suffixes as follows ...

    backups/default_darwin_MAY_15_2007[2].zip
    backups/default_darwin_MAY_15_2007[3].zip

Backups must be manually removed when or if space becomes an issue.

If the user needs to restore a database from an existing backup, the menu 
option "File/Restore" from the main window allows selection of a backup 
file.  The catalog file and all associated images will be extracted from
the backup file (zipped archive) into exactly the same survey area from
which it came.

Given the selected backup archive ...

    backups/default_darwin_MAY_15_2007[2].zip

The database file itself ...

    surveyAreas/default/catalog/darwin.db 

would be replaced with the copy from the archive.  Any missing images
associated with the database in the folder ...

   surveyAreas/default/catalog/

would also be replaced from the archive.  However, existing images
in the catalog folder will NOT be overwritten.

Export & Import of Catalogs
---------------------------
These processes are provided in the software beginning with version 1.85.
They are similar to Backup and Restore.  However, the user is allowed 
wider control over the location and naming of the zipped archive when
exporting, and a completely new survey area is required when importing.
Export and import are intended to allow movement of a catalog (database)
from one machine to another or one installation to another.

From the main window the user may export the CURRENTLY LOADED database
using the menu option "File/Export/Export Catalog."  A zipped archive 
containing the database file and all associated images is created in a 
location and with a name selected by the user.

From the main window the user may import an exported archive by using the
"File/Import/Import Catalog" menu option.  The user must then select the 
archive file to be imported AND enter a name for a new survey area within 
which to import the catalog.  Imports into existing survey areas are not 
allowed in this version of the software because of the difficulties 
associated with maintaining existing database associations with existing 
images while extracting large numbers of additional images into the catalog 
folder.

Export & Import of Finz
-----------------------
This is a NEW feature of DARWIN 2.0.  Individual TRACED FINS may now be 
exported from the CURRENTLY OPEN catalog or imported into the CURRENTLY
OPEN catalog. A new compressed archive version of a DARWIN Fin is created 
in a "*.finz" file.  This FINZ file contains the Fin outline, features, 
data and copies of both original and DARWIN modified images.  All older
style "*.fin" files can be loaded with DARWIN 2.0, but only FINZ files will
be created by this and later versions.

To Export a fin, select the "File/Export/Export Fin (*.finz)" menu option.
A scrollable list of fins in the database will appear.  You may select one 
or more fins from this list to be exported. Selection of a range of fins
is supported using the SHIFT key along with mouse clicks.  Selection of
multiple fins that are not in a contiguous range is supported using the
CTRL key along with mouse clicks.  If a single fin is selected and
"Save Fin(s)" is clicked, you will be provided with a file save dialog and
must type a name for the FINZ file and click "Save."  If multiple
fins are selected and "Save Fin(s)" is clicked, then you must select a 
folder into which the FINZ files will be saved and click "Save."  Filenames
are automatically created from the fin ID for each saved fin when exporting
multiple fins at once.

To Import a fin, select the "File/Import/Import Fin (*.finz) menu option.
An open file chooser will appear.  Select the fin or fins to be imported 
into the CURRENTLY OPEN catalog, and click "Open." The effect of importing
a fin or fins is the same as if each FINZ file had been opened using the
"File/Open Traced Fin" menu option, and had then clicked the "Add to Database"
button in the Trace Window.

===========================================================================
From here to end of file is a listing of all User interface components
It is primarily of interest to developers and may be ignored by most users.
===========================================================================

SOFTWARE Features and User Interface
====================================

-----------
Main Window
-----------
Title
    "DARWIN - " <plus path and name of loaded database file>

Menu Options
    File
        New Database
        Open Image
        Open Traced Fin
        Open Database
        Matching Queue
        Backup
        Restore
        Import
	    Import Catalog 
	    Import Fin (*.finz)           <-- NEW in DARWIN 2.0
        Export
	    Export Catalog
	    Export Fin (*.finz)           <-- NEW in DARWIN 2.0
        Exit
    Settings
        Options
        Catalog Schemes
            New Scheme
            View/Edit Schemes
            Set Default Scheme
    Data
        Export Selected Data
    Help
        Documentation
        About

Buttons
    Open (Image)
    Open (Fin)
    Queue
    Goto
        <find dolphin by ID>
    Previous
    Next
    Modify

Displays
    List of Known Dolphins (Left Pane)
    Tabbed notebook (Right Pane)
        Modified Image of selected dolphin
        Original Image of selected dolphin
        Fin Outline of selected dolphin
        Data for selected dolphin
           ID
            Name
            Date of Sighting
            Roll and Frame
            Location Code
            Damage Category
            Short Description

Popups from Main Window
    Image Viewer
    Outline Viewer

-----------------------------------------
Trace Window (opened with UNTRACED image)
-----------------------------------------
Enter from <Open Image> button or <File - Open Image> menu option on Main Window

Title
    <Path and Name of loaded fin image>

Tracing Sequence Buttons
    Undo
    Redo
    Modify Image
        <active initially>
    Trace Outline
    Locate Features

Tool Buttons
    <for Modifying Images>
        Magnify
        Flip Horizontally
        Enhance Contrast 
        Adjust Brightness
        Crop
    <for Manual Outline Tracing>
        Magnify
        Manually trace outline
        Insert outline points
        Adjust outline point location
        Remove outline points
        Trim outline ends
    <for Feature Adjustment>
        Magnify
        Drag and Drop feature

Message area

Numerical Display
    Image Scale
    Selected Image Coordinates

Image Area
    <adjustable and scrollable>

Data Fields
    ID Code
    Name
    Date of Sighting
    Roll and Frame
    Location Code
    Damage Category
    Short Description

Main Action Buttons
    Export Data
    Match
    Save
    Add to Database
    Close

------------------------------------------------
Trace Window (opened with previously traced fin)
------------------------------------------------
Enter from <Open Traced Fin> button or <File - Open Fin> menu option on Main Window

Title
    <Name of loaded/traced fin file>

Tracing Sequence Buttons
    Undo
    Redo
    Modify Image
        <locked initially>
    Trace Outline
        <locked initially>
    Locate Features
        <locked initially>
    Unlock Traced Fin
        <not available in version 1.95>

Tool Buttons
    <for Feature Adjustment>
        Magnify
    Unlock Traced Fin
        <not available in version 1.95>

Message area

Numerical Display
    Image Scale
    Selected Image Coordinates

Image Area
    <adjustable and scrollable>

Data Fields
    ID Code
    Name
    Date of Sighting
    Roll and Frame
    Location Code
    Damage Category
    Short Description

Main Action Buttons
    Export Data
    Match
    Save
    Add to Database
    Close               <-- ONLY option available if opened as FINZ Viewer

---------------------
Matching Queue Window
---------------------
Enter from <Queue> button or <Matching Queue> menu option on Main Window

Title
    "Matching Queue"

Traced Fin List

Image Area

Status bars
    Percent of Queue Processed
    Percent of Database Matched against current unknown fin

Buttons
    Add
    Remove
    Run Match
    View Results
    Save Queue
    Load Queue
    Cancel

---------------------------------------
Catalog Scheme Window (New Scheme Mode)
---------------------------------------
Enter from <Catalog Scheme - Define New Scheme> menu option on Main Window

Title
    "Catalog Schemes ..."

Data Entry Areas
    Scheme Name
    Scheme Category Name List

Buttons
    Cancel
    OK

---------------------------------------------
Catalog Scheme Window (ViewEdit Schemes Mode)
---------------------------------------------
Enter from <Catalog Scheme - View/Edit> menu option on Main Window

Title
    "Catalog Schemes ..."

Data Lists
    Scheme Names
    Category Names

Pop-Up Menus (right mouse button)
    Scheme Name List Options
        Rename Scheme
        Remove Scheme
    Category Name List Options
        Rename Category
        Insert New Category
        Remove Category

Buttons
    Allow Editing
    Cancel
    OK

-------------------------------------------------
Catalog Scheme Window (Select Active Scheme Mode)
-------------------------------------------------
Enter from <Catalog Scheme - Select Active> menu option on Main Window

Title
    "Catalog Schemes ..."

Data Lists
    Scheme Names
    Category Names

Buttons
    Cancel
    OK

---------------
Matching Window
---------------
Enter from <Match> button on Trace Window

Title
    "Matching Fin ..."

Category Selection Section
    Checkboxes
    Buttons
        Select All
        Clear All

Search Method Section
    Radio Buttons (for each alignment method)
        Quick & Dirty
        Iteratively
    Radio Buttons (for restricting part of outline matched)
        All Points
        Trailing Edge Points Only

Outline Registration Section
    Visual Display Area
    Check Box to enable/disable display

Progress Meter

Main Buttons 
    Start/Stop
    Pause/Continue
    Cancel

--------------------
Match Results Window
--------------------
Enter from Matching Window (at completion of matching process) or 
from <View Results> button on Matching Queue Window

Title
    "Matching Results" <and results file name, if results displayed from file>

Selected Fin Area
    Image
    Buttons
        Show Original / Show Modified
Unknown Fin Area
    Image
    Buttons
        Show Original / Show Modified
        Match Selected Fin Orientation / Show Original Orientation

Database Fin Selection Area
    Scrolling List
    Buttons
        Info
        Hide/Show ID's
        Previous
        Next
        Scroll (on/off)

Registered Outlines Display

Popups from Match Results Window
    Image Viewer
    Outline Viewer

Main Buttons
    Update Selected Fin
    Add New Fin to Catalog
    Return to Matching Dialog
    Save Match Results
    Done

--------------------------
Outline Information Window
--------------------------
Enter by clicking on <Fin Outline of Selected Dolphin Display> in Main Window or
from clicking on <matched Outlines display> on Match Results Window

Title
    "Outline Information"

Fin Outline Area
    One outline (if entered from Main Window)
    Two aligned outlines (if entered from Match Results Window

Chain Code Area
    One chain code plot (if entered from Main Window)
    Two aligned chain code plots (if entered from Match Results Window

Information Display Area
    Fin ID's
    Fin point counts
    Positions of key outline feature points (Begin, Tip, Notch, End)

Buttons
    Generate Coefficients (with level selection)
    Close

-------------------------------------------
File Chooser Dialog (Open Fin Image Mode)
-------------------------------------------
Enter from <Open Image> button or <File - Open Image> menu option on Main Window

Title
    "Select a File"

Folder tree
    Display
    Buttons
        Add
        Remove

Folder path
    <default - %DARWINHOME%>

File List
    <scrollable>

File Type (drop down list)
    <default - *.jpg *.bmp *.png *.ppm *.pgm>

Buttons
    Cancel
    Open

Display Areas
    Image Preview

-----------------------------------
File Chooser Dialog (Open Fin Mode)
-----------------------------------
Enter from <Open Fin> button or <File - Open Fin> menu option on Main Window

Title
    "Select a Traced Fin (*.fin)"

Folder tree
    Display
    Buttons
        Add
        Remove

Folder path
    <default - %DARWINHOME%\SurveyAreas\*\TracedFins>

File List
    <scrollable>

File Type (drop down list)
    <default - *.fin>

Buttons
    Cancel
    Open

----------------------------------------
File Chooser Dialog (Open Database Mode)
----------------------------------------
Enter from <File - Open Database> menu option on Main Window

Title
    "Select a Database (*.db)"

Folder tree
    Display
    Buttons
        Add
        Remove

Folder path
    <default - %DARWINHOME%\SurveyAreas\*\catalog>

File List
    <scrollable>

File Type (drop down list)
    <default - *.db>

Buttons
    Cancel
    Open

---------------------------------------------
File Chooser Dialog (Open Match Results Mode)
---------------------------------------------
Enter from <View Results> button on Matching Queue Window

Title
    "Select Results File ..."

Folder tree
    Display
    Buttons
        Add
        Remove

Folder path
    <default - %DARWINHOME%\SurveyAreas\*\MatchQResults>

File List
    <scrollable>

File Type (drop down list)
    <default - *.res>

Buttons
    Cancel
    Open

-------------------------------------------
File Chooser Dialog (Open Match Queue Mode)
-------------------------------------------
Enter from <Load Queue> button on Matching Queue Window

Title
    "Load Matching Queue (*.que)"
Folder tree
    Display
    Buttons
        Add
        Remove

Folder path
    <default - %DARWINHOME%\SurveyAreas\*\MatchQueues>

File List
    <scrollable>

File Type (drop down list)
    <default - *.que>

Buttons
    Cancel
    Open

------------------------------------------
File Chooser Dialog (Export Database Mode)
------------------------------------------
Enter from <File - Export> menu option on Main Window

Title
    "Enter filename for the EXPORT Database Archive (*.zip)"

Initial (restricted) view
    Filename Entry Area

    Current Folder Drop Down

    Selector
        Browse for other Folders (switches between restricted and expanded views)

    Buttons
        Cancel
        Save

Expanded View
    Folder tree
        Display
        Buttons
            Add
            Remove

    Folder path
        <default - %DARWINHOME%\Backups>

    Create Folder Button

    File List
        <scrollable>

    File Type (drop down list)
        <default - *.zip>

    Buttons
        Cancel
        Save

------------------------------------------
File Chooser Dialog (Import Database Mode)
------------------------------------------
Enter from <File - Import> menu option on Main Window

Title
    "Select a Database Backup Archive (*.zip)"

Folder tree
    Display
    Buttons
        Add
        Remove

Folder path
    <default - %DARWINHOME%\Backups>

File List
    <scrollable>

File Type (drop down list)
    <default - *.zip>

Buttons
    Cancel
    Open

--------------------------------------
File Chooser Dialog (Import Finz Mode)
--------------------------------------
Enter from <File - Import - Import Fin (*.finz)> menu option on Main Window

Title
    "Select a Traced Fin (*.fin *.finz)"

Folder tree
    Display
    Buttons
        Add
        Remove

Folder path
    <default - %DARWINHOME%\surveyAreas\*\tracedFins>

File List
    <scrollable>
    <multiple selection allowed>

File Type (drop down list)
    <default - *.fin *.finz>

Buttons
    Cancel
    Open

-------------------------------------------
File Chooser Dialog (Restore Database Mode)
-------------------------------------------
Enter from <File - Restore> menu option on Main Window

Title
    "Select a Database Backup Archive (*.zip)"

Folder tree
    Display
    Buttons
        Add
        Remove

Folder path
    <default - %DARWINHOME%\Backups>

File List
    <scrollable>

File Type (drop down list)
    <default - *.zip>

Buttons
    Cancel
    Open

-----------------------------------------------
File Chooser Dialog (Export Selected Data Mode)
-----------------------------------------------
Enter from <OK> button on Export Selected Data Window

Title
    "Enter filename for the <tab> separated data file (*.txt)"

Initial (restricted) view
    Filename Entry Area

    Current Folder Drop Down

    Selector
        Browse for other Folders (switches between restricted and expanded views)

    Buttons
        Cancel
        Save

Expanded View
    Folder tree
        Display
        Buttons
            Add
            Remove

    Folder path
        <default - %DARWINHOME%\Backups>

    Create Folder Button

    File List
        <scrollable>

    File Type (drop down list)
        <default - *.txt>

    Buttons
        Cancel
        Save

-------------------
New Database Window
-------------------
Enter from <File - New Database> menu option on Main Window

Title
    "Survey Areas and Databases ..."

Darwin Splash Image

Existing Survey Areas List
    <selectable>

Existing Databases List
    <list of databases in selected Survey Area>

What to Create Selection
    New Database (within selected Survey Area)
    New Survey Area (and new database)

Select Catalog Scheme (drop down list)

Entry Areas
    New Survey Area Name
    Name Database name

Buttons
    Cancel
    OK

-------------
Backup Window
-------------
Enter from <File - Backup> menu option on Main Window

Title
    "Information"

Message area
    <a simple message indicating backup commencement>

Button
    Ok

-------------------------------------------------------
Modify Database Entry Window (entered from Main Window)
-------------------------------------------------------
Entered from <Modify> button on Main Window

Title
    <path and filename of MODIFIED fin image>

Fin Image

Known Information entry fields
    ID
    Name
    Date of Sighting
    Roll & Frame
    Location Code
    Damage Category (drop down list)
    Short Description

Buttons
    Save
    Delete
    Cancel

----------------------------------------------------------------
Modify Database Entry Window (entered from Match Results Window)
----------------------------------------------------------------
Entered from <Update Selected Fin> or <Add New Fin to Catalog> buttons on Match Results Window

Title
    <path and filename of ORIGINAL fin image>

Fin Image

Known Information entry fields
    ID
    Name
    Date of Sighting
    Roll & Frame
    Location Code
    Damage Category (drop down list)
    Short Description

Buttons
    Add to database
    Cancel

---------------------------
Export Selected Data Window
---------------------------
Enter from <Data - Export Selected Data> menu option on Main Window

Title
    "Exporting Selected Data ..."

Category Selection Section
    Checkboxes
        ID
        Name
        Date of Sighting
        Roll & Frame
        Location Code
        Damage Category (drop down list)
        Short Description
        Modified Image Name
        Original Image Name
    Buttons
        Select All
        Clear All

Export Method Selection
    Tab Separated
    To Printer (not available in version 1.95)

Buttons
    Cancel
    OK

------------------
Image Popup Window
------------------
Enter by clicking on Fin Image in Main Window or Match Results Window

Title
    <dolphin ID, "Selected Fin" or "Unknown Fin" depending on calling context>

Scrollable Image

Tools
    Magnifying Glass

Buttons
    Flip Horizontally
    Zoom In
    Zoom Out
    Close

------------
About Window
------------
Enter from <Help - About> menu option on Main Window

Title
    "About ..."

Darwin Splash Image

Information Displayed
    Current Version
    Web site URL
    Contributors (Research and Programming)
    Financial Support

Buttons
    OK

--------------
Options Window
--------------
Enter from <Settings - Options> menu option on Main Window

Title
    "Options"

Tabs
    General
        Toolbar Settings
            Text only
            Pictures Only
            Text and Pictures
        Dolphin ID Settings
            Show ID's in Windows
            Hide / Use Fake ID's (for blind testing)
    Tracing
        Tracing Color Selection
        Active Contours
            Max Iterations (50 is default)
            Continuity Energy (9 is default)
            Linearity Energy (3 is default)
            Edge Energy (3 is default)
    Image Processing
        Gaussian Standard Deviation (1.5 is default)
        Low threshold (0.15  is default)
        High Threshold (0.85 is default)
    Matching
        <unused in version 1.95>
    Fonts
        Current Font
        Sample text in current Font
        Change Font Button

Buttons
    OK
    Cancel

---------------------
Font Selection Dialog
---------------------
Enter from <Change Font> button on <Fonts Tab> on Options Window

Title
    "Select a font ..."

Font family List
    <selectable>

Style List
    <selectable>

Size List
    <selectable>

Preview Window
    <shows "abcdefghijk ABCDEFGHIJK" in selected font/style/size

Buttons
    Cancel
    OK

------------------------------------------
File Chooser Dialog (Save Single Fin Mode)
------------------------------------------
Enter from <Save> button on Trace Window or from <Save Fin(s)> button
on Export Fin Dialog (with one fin selected)

Title
    "Enter filename for the Traced Fin File (*.finz)"

Initial (restricted) view
    Filename Entry Area

    Current Folder Drop Down

    Selector
        Browse for other Folders (switches between restricted and expanded views)

    Buttons
        Cancel
        Save

Expanded View
    Folder tree
        Display
        Buttons
            Add
            Remove

    Folder path
        <default - %DARWINHOME%\surveyAreas\*\tracedFins>

    Create Folder Button

    File List
        <scrollable>

    File Type (drop down list)
        <default - *.finz>

    Buttons
        Save
        Cancel

---------------------------------------------
File Chooser Dialog (Save Multiple Fins Mode)
---------------------------------------------
Enter from <Save Fin(s)> button on Export Fin Dialog 
(with multiple fins selected)

Title
    "Enter directory for the Traced Fin Files (*.finz)"

Initial (Expanded) View
    Folder tree
        Display
        Buttons
            Add
            Remove

    Folder path
        <default - %DARWINHOME%\surveyAreas\*\tracedFins>

    Create Folder Button

    File List
        <scrollable>

    File Type (drop down list)
        <default - *.finz>

    Buttons
        Save
        Cancel

-----------------------
Convert Database Dialog 
-----------------------
Entered from attempt to open and OLD (non SQL) database file

Title
    Conversion of Database ...

Preliminary Actions
    Backup Catalog database file
    Backup All Catalog Images

Conversion to Perform (Radio Buttons)
    Convert to SQLite database

Buttons
    OK
    Cancel

-----------------
Fin Export Dialog
-----------------
Entered from <File - Export - Export Fin (*.finz)> menu item in Main Window

Title
    DARWIN - Export FINZ (from *.db)

Displays
    List of Known Dolphins (multiple selection allowed)

Buttons
    Goto
        <find dolphin by ID>
    Save
    Cancel
