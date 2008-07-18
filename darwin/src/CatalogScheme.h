/////////////////////////////////////////////////////////////////////
//   file: CatalogScheme.h
//
// author: J H Stewman
//
//   date: 7/18/2008
//
// new catalog scheme class - begin integrating with all code - JHS
//
/////////////////////////////////////////////////////////////////////

#ifndef CATALOGSCHEME_H
#define CATALOGSCHEME_H

class CatalogScheme {
public:
	CatalogScheme()
	:	schemeName("")
	{
		categoryNames.clear();
	}

	// members

	std::string 
		schemeName;

	std::vector<std::string> 
		categoryNames;
};

#endif
