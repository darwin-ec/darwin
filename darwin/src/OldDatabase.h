//*******************************************************************
//   file: OldDatabase.h
//
// author: Adam Russell
//
//   mods: J H Stewman (9/27/2005)
//         -- change to allow relative storage of image files
//            (now, stored filename has path, if any, related only to
//             the catalog category within which it falls)
//         -- current implementation: ALL images are copies of originals
//            and are stored in the same place as "darwin.db"
//         -- 9/29/2005
//            OldDatabase version 3 - stores catalog category labels in OldDatabase
//
//         -- total rewrite as non template class inheriting from
//            Database ABSTRACT class (7/8/2008) - version 1.99

//*******************************************************************
//
// Darwin's fin OldDatabase class.
//
// The format is as follows :
// 
// [Offset to footer] (4 bytes)
// [OldDatabase version number : current is 3] (4 bytes)
// [Number of Catalog Categories in use] (4 bytes)
// [Catalog Category Names] ** Each terminated by '\n'
// [...DATA...]
// [Footer]
//
// Darwin-0.3.8 - DB version 2: Addendum : New DatabaseFin Data structure
// [Data Position] (4 Bytes)
// [Image Filename] (char[255]) **Delimited by '\n'
// [Number of FloatContour Points] (4 bytes)
// [FloatContour Points ...] (Number*2*sizeof(float) bytes)
// [Feature Point Positions] (5*sizeof(int) bytes)
// [Thumbnail Pixmap] (25*25 bytes)
// [Short Description] (char[255]) **Delimited by '\n'
//

#ifndef OldDatabase_H
#define OldDatabase_H

#include "Database.h"  //***1.99

#include "DatabaseFin.h"
#include "Error.h"

#include "Options.h"

#include <fstream>
#include <string>
#include <strstream>
#include <vector>
#include <algorithm> // for std::sort()
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef DEBUG
#include <iostream>
#endif

#ifdef WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "/"
#endif

#define NOT_IN_LIST -1

//***185 - moved this to Config.h
//#define CURRENT_DBVERSION 3
/*
typedef enum {
	DB_SORT_NAME,
	DB_SORT_ID,
	DB_SORT_DATE,
	DB_SORT_ROLL,
	DB_SORT_LOCATION,
	DB_SORT_DAMAGE,
	DB_SORT_DESCRIPTION
} db_sort_t;

*/

class OldDatabase : public Database //***1.99 - now a derived type
{
	public:

		OldDatabase(Options *o, CatalogScheme cat, bool createEmptyDB);

		~OldDatabase();
		
		virtual void createEmptyDatabase(Options *o);

		virtual unsigned long add(DatabaseFin<ColorImage>* data); 

		virtual DatabaseFin<ColorImage>* getItemAbsolute(unsigned pos);

		virtual DatabaseFin<ColorImage>* getItem(unsigned pos);
		// virtual DatabaseFin<ColorImage>* getItemByName(std::string name);

		// virtual ItemInfo* getItemInfo(unsigned pos);

		void convert(std::string filename);
		void writeFooter();

		void DeleteFinFromList(DatabaseFin<ColorImage> *Fin);
                
		virtual void Delete(DatabaseFin<ColorImage> *Fin);

		virtual bool OldDatabase::openStream();
		virtual bool OldDatabase::closeStream();

		static bool isType(std::string filePath);


	protected:

		std::fstream mDbFile;

		virtual DatabaseFin<ColorImage>* getItem(unsigned pos, std::vector<std::string> *theList);
};


#endif // OldDatabase_H

