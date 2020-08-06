//*******************************************************************
//   file: ConfigFile.h
//
// author: Adam Russell
//
//   mods: J H Stewman (4/12/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "utility.h"

#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <list>
#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef DEBUG
#include <iostream>
#endif

const char SEPARATOR = '='; // character to separate fields in config file
const char COMMENT = '#'; // character used to indicate comment line


//*******************************************************************

class ConfigItem
{
	protected:
		bool dirty;		// indicates whether this item will need to be saved
		std::string itemDesc;	// description of this item
		std::string value;	// value of this item as a string to be easily printable

	public:
		// Default Constructor
		ConfigItem (void) {
			dirty = false;
		}

		// Constructor
		ConfigItem (const std::string d, const std::string v) {
			itemDesc = d;
			value = v;
		}

		// Copy constructor
		ConfigItem (const ConfigItem &c) {
			itemDesc = c.getDescription ();
			value = c.getValue ();
			dirty = c.isDirty ();
		}

		// Sets descriptor and value of item
		void setItem(const std::string d, const std::string v) {
			itemDesc = d;
			value = v;
		}

		// Sets value of item
		void setValue(std::string v) {
			value = v;
		}

		// Returns item's description
		std::string getDescription() const {
			return itemDesc;
		}

		// Returns item's value
		std::string getValue() const {
			return value;
		}

		// Sets dirty "bit" to true
		void setDirty() {
			dirty = true;
		}

		// Determines whether the specified description matches item's
		// description.
		//    PARAMETERS:
		//       string desc - Specified description. (Note: CASE INSENSITIVE)
		//    RETURN: true if there's a match, false otherwise
		//
		bool equals (std::string desc) {
			return caseInsensitiveStringCompare(desc, itemDesc);
		}

		// Incicates whether item has been changed (is dirty)
		bool isDirty() const {
			return dirty;
		}

		// Overloaded Assignment operator
		const ConfigItem &operator=(const ConfigItem &right) {
			if (&right != this) { // Check for self assignment
				itemDesc = right.getDescription ();
				value = right.getValue ();
				dirty = right.isDirty ();
			}

			return *this;
		}

		// Prints item's description and value in console.
#ifdef DEBUG
		void print (void) {
			std::cout << "Item description: " << itemDesc << "\tValue: " << value << std::endl;
		}
#endif

};


//*******************************************************************

class ConfigFile
{
	public:
		// Constructor, Copy constructor & destructor
		ConfigFile (void);
		ConfigFile (const std::string fileName);
		~ConfigFile (void);
	
		//***1.4 - so we can restructure darwin.cfg if exists in older format
		void ClearList(); // created EMPTY ConfigFile::itemList

		// file manipulation functions
		bool open (const std::string fileName);
		bool save (void);
		bool save (const std::string fileName);

		int size(); //***1.85

		// functions that ADD items to configuration
		void addItem (const std::string itemDesc, char value);
		void addItem (const std::string itemDesc, int value);
		void addItem (const std::string itemDesc, float value);
		void addItem (const std::string itemDesc, double value);
		void addItem (const std::string itemDesc, const std::string value);
		void addItem (const std::string itemDesc, bool value); //***1.65

		// functions that RETURN items in configuration
		bool getItem (const std::string itemDesc, char &value);
		bool getItem (const std::string itemDesc, int &value);
		bool getItem (const std::string itemDesc, float &value);
		bool getItem (const std::string itemDesc, double &value);
		bool getItem (const std::string itemDesc, std::string &value);
		bool getItem (const std::string itemDesc, bool &value); //***1.65

	protected:
		std::fstream fileio;
		std::list<ConfigItem> itemList;
		std::string fName;

		bool alreadyExists (const std::string itemDesc);
		void stripSpaces (char* str);
};

#endif
