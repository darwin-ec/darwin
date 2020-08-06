

//*******************************************************************
//   file: DummyDatabase.h
//

//

#ifndef DummyDatabase_H
#define DummyDatabase_H

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

class DummyDatabase : public Database //***1.99 - now a derived type
{
	public:

		DummyDatabase(Options *o, bool createEmptyDB)
			:
			Database() {
		};

		~DummyDatabase() {};
		
		virtual void createEmptyDatabase(Options *o) {};

		virtual unsigned long add(DatabaseFin<ColorImage>* data) {return 0;}; 

		virtual DatabaseFin<ColorImage>* getItemAbsolute(unsigned pos) {return NULL;};

		virtual DatabaseFin<ColorImage>* getItem(unsigned pos) { return NULL;} ;
		// virtual DatabaseFin<ColorImage>* getItemByName(std::string name);

		// virtual ItemInfo* getItemInfo(unsigned pos);
                
		virtual void Delete(DatabaseFin<ColorImage> *Fin) {};

		virtual bool openStream() {return false;};
		virtual bool closeStream() {return false;};

		void setStatus(db_status_t status) { mDBStatus = status; };

	protected:

		virtual DatabaseFin<ColorImage>* getItem(unsigned pos, std::vector<std::string> *theList) {return NULL;};
};


#endif // DummyDatabase_H
