//*******************************************************************
//   file: ConfigFile.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (7/22/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include "ConfigFile.h"

#include <iostream>

using namespace std;

//*******************************************************************
//
// ConfigFile::ConfigFile (void)
//
//    DEFAULT CONSTRUTOR - does nothing at this time.
//
ConfigFile::ConfigFile (void)
{
	//blah
}


//*******************************************************************
//
// ConfigFile::ConfigFile (const string fileName)
//
//    This is the constructor we'd like to use most of the time.
//    It loads configuration from the file with the specified name.
//
//    PARAMETERS: The name of the configuration file.
//
ConfigFile::ConfigFile (const string fileName)
{
	// keep a copy of the filename
	fName = fileName;

	fileio.open (fName.c_str (), ios::in);
		
	cout << "Loading configuration file ...\n  \"" << fName << "\"" << endl;

	if (fileio.fail ()) {
		cout << "Process failed ... attempting to load backup." << endl;
		if (fileio.is_open ())
			fileio.close ();
		return; //***1.85
	}

	char line[255];

	// create a string with the separator for use with the string tokenizer
	char separator[2];
	sprintf(separator, "%c", SEPARATOR); 

	ConfigItem c;
	char desc[253], val[253];
	
	while (fileio.getline (line, 255) && fileio.good ()) {
		if (line[0] != COMMENT && strlen (line) >= 3) {
			strcpy (desc, strtok (line, separator));
			strcpy (val, strtok (NULL, separator));

			stripSpaces (desc);
			stripSpaces (val);
			
			//string d = desc;
			//string v = val;
			c.setItem (desc, val);
			itemList.push_back(c);
		}
	}
}


//*******************************************************************
//
// ConfigFile::~ConfigFile()
//
//    DESTRUCTOR -- simply closes file if necessary.
//
ConfigFile::~ConfigFile()
{
	if (fileio.is_open ())
		fileio.close ();
}

//*******************************************************************
//
// ConfigFile::ClearList()
//
//    creates EMPTY itemList -- use before rebuilding ConfigFile from Options
//
void ConfigFile::ClearList()
{
	itemList.clear();
}

//*******************************************************************
//
// ConfigFile::size()
//
//    returns number of configuration items in list
//
int ConfigFile::size()
{
	return itemList.size();
}


//*******************************************************************
//
// bool ConfigFile::open(const string fileName)
//
//    Opens a new configuration file.
//
//    PARAMETERS: Name of file to open.
//    RETURN: true if successful, false otherwise
//
bool ConfigFile::open(const string fileName)
{
	// keep a copy of the filename
	int n = fileName.rfind(".bak");
	if (n == string::npos)
		fName = fileName;
	else
		fName = fileName.substr(0,n); //***1.85 - strip the ".bak" if found

	fileio.clear(); //***1.85 - just in case a previous use failed

	fileio.open (fileName.c_str (), ios::in); // use entire filename (including ".bak" if any)

	cout << "Loading configuration file ...\n  \"" << fileName << "\"" << endl;

	if (fileio.fail ())
	{
		cout << "Process failed ... using default configuration." << endl; //***1.85
		return false;
	}

	char line[255];

	// create a string with the separator for use with the string tokenizer
	char separator[2];
	sprintf (separator, "%c", SEPARATOR); 

	ConfigItem c;
	char desc[255-2], val[255-2];
	
	while (fileio.getline (line, 255) && fileio.good ()) {
		if (line[0] != COMMENT && strlen (line) >= 3) {
			strcpy (desc, strtok (line, separator));
			strcpy (val, strtok (NULL, separator));

			stripSpaces (desc);
			stripSpaces (val);
			
			string d = desc;
			string v = val;
			c.setItem (d, v);
			itemList.push_back(c);
		}
	}

	return true;
}


//*******************************************************************
//
// bool ConfigFile::save()
//
//    Saves whatever configuration items have been changed or added to
//    the list.
//
//    PARAMETERS: None
//    RETURN: true if successful, false otherwise
//
bool ConfigFile::save()
{
	ofstream outFile;

	outFile.open (fName.c_str (), ios::out | ios::trunc);

	if (outFile.fail ())
		return false;

	list<ConfigItem>::iterator it = itemList.begin();
	
	while (it != itemList.end()) {
		outFile << it->getDescription () << "=" 
			<< it->getValue () << endl;
		it++;
	}

	outFile.close ();

	return true;
}


//*******************************************************************
//
// bool ConfigFile::save(const string fileName)
//
//    Saves whatever configuration items are open to the file indicated.
//    Overwrites the file if it's already present.
//
//    PARAMETERS: Name of file to save to.
//    RETURN: true if successful, false otherwise
//
bool ConfigFile::save(const string fileName)
{
	ofstream outFile;

	outFile.open (fileName.c_str (), ios::out | ios::trunc);

	if (outFile.fail ())
		return false;

	list<ConfigItem>::iterator it = itemList.begin();

	while (it != itemList.end()) {
		outFile << it->getDescription () << "=" 
			<< it->getValue () << endl;
		it++;
	}

	outFile.close ();

	return true;
}


//*******************************************************************
//
// void ConfigFile::addItem(const string itemDesc, char value)
//
//    Adds an item and its value to this configuration file.
//    This particular function is for single character values.
//
//    PARAMETERS:
//       itemDesc - String description of this item.
//       value - the actual value of this item.
//    RETURN: None
//
void ConfigFile::addItem(const string itemDesc, char value)
{
	char tmp[2];
	
	tmp[0] = value;
	tmp[1] = 0;

	list<ConfigItem>::iterator it = itemList.begin();
	bool found = false;

	while (it != itemList.end() && !found) {
		if (it->equals(itemDesc)) {
			found = true;
			it->setValue (tmp);
			it->setDirty ();
		}
		it++;
	}

	if (!found) {
		ConfigItem c(itemDesc, tmp);
		c.setDirty();
		itemList.push_back(c);
	}
}


//*******************************************************************
//
// void ConfigFile::addItem (const string itemDesc, int value)
//
//    Adds an item and its value to this configuration file.
//    This particular function is for integer values.
//
//    PARAMETERS:
//       itemDesc - String description of this item.
//       value - the actual value of this item.
//    RETURN: None
//
void ConfigFile::addItem (const string itemDesc, int value)
{
	char tmp[11]; // Assuming up to ten digits.. bad, I know
	sprintf (tmp, "%d", value);

	list<ConfigItem>::iterator it = itemList.begin();
	bool found = false;

	while (it != itemList.end() && !found) {
		if (it->equals (itemDesc)) {
			found = true;
			it->setValue (tmp);
			it->setDirty ();
		}
		it++;
	}
	
	if (!found) {
		ConfigItem c(itemDesc, tmp);
		c.setDirty();
		itemList.push_back(c);
	}
}


//*******************************************************************
//
// void ConfigFile::addItem (const string itemDesc, float value)
//
//    Adds an item and its value to this configuration file.
//    This particular function is for floating point values.
//
//    PARAMETERS:
//       itemDesc - String description of this item.
//       value - the actual value of this item.
//    RETURN: None
//
void ConfigFile::addItem (const string itemDesc, float value)
{
	char tmp[20]; // Magic number for number of digits!
	sprintf (tmp, "%f", value);

	list<ConfigItem>::iterator it = itemList.begin();
	
	bool found = false;
	while (it != itemList.end() && !found) {
		if (it->equals(itemDesc)) {
			found = true;
			it->setValue (tmp);
			it->setDirty ();
		}
		it++;
	}
	
	if (!found) {
		ConfigItem c(itemDesc, tmp);
		c.setDirty();
		itemList.push_back(c);
	}
}


//*******************************************************************
//
// void ConfigFile::addItem (const string itemDesc, double value)
//
//    Adds an item and its value to this configuration file.
//    This particular function is for double values.
//
//    PARAMETERS:
//       itemDesc - String description of this item.
//       value - the actual value of this item.
//    RETURN: None
//
void ConfigFile::addItem (const string itemDesc, double value)
{
	char tmp[20]; // Another magic number!
	sprintf (tmp, "%f", value);

	list<ConfigItem>::iterator it = itemList.begin();
	
	bool found = false;

	while (it != itemList.end() && !found) {
		if (it->equals(itemDesc)) {
			found = true;
			it->setValue (tmp);
			it->setDirty ();
		}
		it++;
	}
	
	if (!found) {
		ConfigItem c(itemDesc, tmp);
		c.setDirty();
		itemList.push_back(c);
	}
}


//*******************************************************************
//
// void ConfigFile::addItem (const string itemDesc, const string value)
//
//    Adds an item and its value to this configuration file.
//    This particular function is for string values.
//
//    PARAMETERS:
//       itemDesc - String description of this item.
//       value - the actual value of this item.
//    RETURN: None
//
void ConfigFile::addItem (const string itemDesc, const string value)
{
	list<ConfigItem>::iterator it = itemList.begin();
	
	bool found = false;

	while (it != itemList.end() && !found) {
		if (it->equals(itemDesc)) {
			found = true;
			it->setValue (value);
			it->setDirty ();
		}
		it++;
	}
	
	if (!found) {
		ConfigItem c(itemDesc, value);
		c.setDirty();
		itemList.push_back(c);
	}
}

//***1.65 - new version of function
//*******************************************************************
//
// void ConfigFile::addItem (const string itemDesc, bool value)
//
//    Adds an item and its value to this configuration file.
//    This particular function is for string values.
//
//    PARAMETERS:
//       itemDesc - String description of this item.
//       value - the actual value of this item.
//    RETURN: None
//
void ConfigFile::addItem (const string itemDesc, bool value)
{
	char tmp[20]; // Another magic number!
	if (value)
		sprintf (tmp, "true");
	else
		sprintf (tmp, "false");

	list<ConfigItem>::iterator it = itemList.begin();
	
	bool found = false;

	while (it != itemList.end() && !found) {
		if (it->equals(itemDesc)) {
			found = true;
			it->setValue (tmp);
			it->setDirty ();
		}
		it++;
	}
	
	if (!found) {
		ConfigItem c(itemDesc, tmp);
		c.setDirty();
		itemList.push_back(c);
	}
}

//*******************************************************************
//
// bool ConfigFile::getItem (const string itemDesc, char &value)
//
//    Returns the value of the item that matches the description, if found.  
//
//    PARAMETERS:
//       itemDesc - description of the item we're looking for. 	
//       value - The place to store the value of the item in question. 
//    RETURN: None
//
bool ConfigFile::getItem (const string itemDesc, char &value)
{
	list<ConfigItem>::iterator it = itemList.begin();

	bool found = false;

	while (it != itemList.end() && !found) {
		if (it->equals(itemDesc)) {
			found = true;
			string val = it->getValue ();
			value = val[0];
		}
		it++;
	}

	if (!found)
		value = 0;

	return found;
}


//*******************************************************************
//
// bool ConfigFile::getItem (const string itemDesc, int &value)
//
//    Returns the value of the item that matches the description, if found.  
//
//    PARAMETERS:
//       itemDesc - description of the item we're looking for. 
//       value - The place to store the value of the item in question. 
//    RETURN: None
//
bool ConfigFile::getItem (const string itemDesc, int &value)
{
	list<ConfigItem>::iterator it = itemList.begin();
	
	bool found = false;
	
	while (it != itemList.end() && !found) {
		if (it->equals(itemDesc)) {
			found = true;
			value = atoi (it->getValue().c_str());
		}
		it++;
	}

	if (!found)
		value = 0;

	return found;
}


//*******************************************************************
//
// bool ConfigFile::getItem (const string itemDesc, float &value)
//
//    Returns the value of the item that matches the description, if found.  
//
//    PARAMETERS:
//       itemDesc - description of the item we're looking for. 
//       value - The place to store the value of the item in question. 
//    RETURN: None
//
bool ConfigFile::getItem (const string itemDesc, float &value)
{
	list<ConfigItem>::iterator it = itemList.begin();
	
	bool found = false;

	while (it != itemList.end() && !found) {
		if (it->equals (itemDesc)) {
			found = true;
			value = (float) atof(it->getValue().c_str());
		}
		it++;
	}

	if (!found)
		value = 0.0f;

	return found;
}


//*******************************************************************
//
// bool ConfigFile::getItem(const string itemDesc, double &value)
//
//    Returns the value of the item that matches the description, if found.  
//
//    PARAMETERS:
//       itemDesc - description of the item we're looking for. 
//          (Case INSENSITIVE!)
//       value - The place to store the value of the item in question. 
//    RETURN: None
//
bool ConfigFile::getItem(const string itemDesc, double &value)
{
	list<ConfigItem>::iterator it = itemList.begin();
	
	bool found = false;

	while (it != itemList.end() && !found) {
		if (it->equals(itemDesc)) {
			found = true;
			value = atof (it->getValue().c_str());
		}
		it++;
	}

	if (!found)
		value = 0;
	return found;
}


//*******************************************************************
//
// bool ConfigFile::getItem(const string itemDesc, string &value)
//
//    Returns the value of the item that matches the description, if found.  
//
//    PARAMETERS:
//       itemDesc - description of the item we're looking for. 	
//       value - The place to store the value of the item in question. 
//    RETURN: None
//
bool ConfigFile::getItem(const string itemDesc, string &value)
{
	list<ConfigItem>::iterator it = itemList.begin();

	bool found = false;

	while (it != itemList.end() && !found) {
		if (it->equals(itemDesc)) {
			found = true;
			value = it->getValue();
		}
		it++;
	}
	return found;
}

//***1.65 - new version of function
//*******************************************************************
//
// bool ConfigFile::getItem(const string itemDesc, bool &value)
//
//    Returns the value of the item that matches the description, if found.  
//
//    PARAMETERS:
//       itemDesc - description of the item we're looking for. 	
//       value - The place to store the value of the item in question. 
//    RETURN: None
//
bool ConfigFile::getItem(const string itemDesc, bool &value)
{
	list<ConfigItem>::iterator it = itemList.begin();

	bool found = false;

	while (it != itemList.end() && !found) {
		if (it->equals(itemDesc)) {
			found = true;
			value = (it->getValue() == "true");
		}
		it++;
	}
	return found;
}


//*******************************************************************
//
// bool ConfigFile::alreadyExists(const string itemDesc)
//
//    Utility function to test whether or not an item with the specified
//    description already exists in the list.
//
//    PARAMETERS: Item description.
//    RETURN: true if item is in the list, false otherwise
//
bool ConfigFile::alreadyExists(const string itemDesc)
{
	list<ConfigItem>::iterator it = itemList.begin();

	while (it != itemList.end()) {
		if (it->equals(itemDesc))
			return true;
		it++;
	}

	// If we get here, it's not in the list
	return false;
}


//*******************************************************************
//
// void ConfigFile::stripSpaces(char* str)
//
//    Utility function to remove initial and trailing spaces from tokens.
//    Warning: alters the string it is passed.
//
//    PARAMETERS:
//       char* str - string to strip spaces from.
//    RETURN: None.
//
void ConfigFile::stripSpaces(char* str)
{
	int length = strlen(str);

	// Get rid of trailing spaces first
	for (int j = length - 1; str[j] == ' ' && j >= 0; j--)
		str[j] = '\0';	

	// And now, the initial spaces
	length = strlen (str);
	int numInitialSpaces = 0;
	
	for (;str[numInitialSpaces] == ' ' && numInitialSpaces < length; numInitialSpaces++);

	if (numInitialSpaces > 0) {
		// go for i <= length to get the string terminator
		for (int i = numInitialSpaces; i <= length; i++)
			str[i-numInitialSpaces] = str[i];
	}
}
