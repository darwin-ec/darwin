//*******************************************************************
//   file: ImageMod.cxx
//
// author: J H Stewman (1/24/2007)
//
//   mods: 
//
// contains classes for keeping track of applied image modifications
//
// used in TraceWindow to build list of modifications applied to
// original image in preparation for tracing, or to reproduce
// same sequence when loading a previsously traced and saved fin
//
// used in MatchResultsWindow when loading results or changing 
// selected fin, so that the modified image can be recreated from
// the original for both selected and unknown fins
//
//*******************************************************************

#include "ImageMod.h"

using namespace std;
//using ImageMod::ImageModType; // vc++6.0 for ImageModType returned by getType function

////////////////////////// ImageMod functions ///////////////////////

ImageMod::ImageMod(ImageModType op, int val1, int val2, int val3, int val4)
	: min(0),max(0),
	  amount(0),
	  xMin(0),yMin(0),xMax(0),yMax(0)
{
	// the values are used depending on the ImageModtype
	this->op = op;
	if ((IMG_flip == op) || (IMG_undo == op) || (IMG_redo == op) || (IMG_none == op))
	{
		// op == IMAG_flip, IMG_undo, or IMG_redo ... then no values used
	}
	else if (IMG_contrast == op)
	{
		// op == IMG_contrast, min is val1, and max is val2
		min = val1;
		max = val2;
	}
	else if (IMG_brighten == op)
	{
		// op == IMG_brighten, amount is val1
		amount = val1;
	}
	else if (IMG_crop == op)
	{
		// op == IMG_crop, xMin is val1, yMin is val2, xMax is val3, yMax is val4
		xMin = val1;
		yMin = val2;
		xMax = val3;
		yMax = val4;
	}
	else
		this->op = IMG_none;
}

void ImageMod::set(ImageModType op, int val1, int val2, int val3, int val4)
{
	// the values are used depending on the ImageModtype
	this->op = op;
	if ((IMG_flip == op) || (IMG_undo == op) || (IMG_redo == op))
	{
		// op == IMAG_flip, IMG_undo, or IMG_redo ... then no values used
		min = max = amount = xMin = yMin = xMax = yMax = 0;
	}
	else if (IMG_contrast == op)
	{
		// op == IMG_contrast, min is val1, and max is val2
		min = val1;
		max = val2;
		amount = xMin = yMin = xMax = yMax = 0;
	}
	else if (IMG_brighten == op)
	{
		// op == IMG_brighten, amount is val1
		amount = val1;
		min = max = xMin = yMin = xMax = yMax = 0;
	}
	else if (IMG_crop == op)
	{
		// op == IMG_crop, xMin is val1, yMin is val2, xMax is val3, yMax is val4
		xMin = val1;
		yMin = val2;
		xMax = val3;
		yMax = val4;
		min = max = amount = 0;
	}
	else
		this->op = IMG_none;
}

void ImageMod::get(ImageModType &op, int &val1, int &val2, int &val3, int &val4)
{
	// the values are used depending on the ImageModtype
	op = this->op;
	if ((IMG_flip == op) || (IMG_undo == op) || (IMG_redo == op) || (IMG_none == op))
	{
		// op == IMAG_flip, IMG_undo, or IMG_redo ... then no values used
		val1 = val2 = val3 = val4 = 0;
	}
	else if (IMG_contrast == op)
	{
		// op == IMG_contrast, min is val1, and max is val2
		val1 = min;
		val2 = max;
		val3 = val4 = 0;
	}
	else if (IMG_brighten == op)
	{
		// op == IMG_brighten, amount is val1
		val1 = amount;
		val2 = val3 = val4 = 0;
	}
	else if (IMG_crop == op)
	{
		// op == IMG_crop, xMin is val1, yMin is val2, xMax is val3, yMax is val4
		val1 = xMin;
		val2 = yMin;
		val3 = xMax;
		val4 = yMax;
	}
	else
		printf("error in modList::get()\n"); // shouldn't get here

}

// ImageModType ImageMod::getType(void) // vc++6.0
ImageMod::ImageModType ImageMod::getType(void) // vc++2011
{
	return op;
}

//////////////////////// ImageModList functions /////////////////////

ImageModList::ImageModList()
{
	modList.clear();
	//it = NULL; // is this right fo an iterator
}

ImageModList::~ImageModList()
{
	// may not be needed
	modList.clear();
}

void ImageModList::clear()
{
	// create empty list
	modList.clear();
}

bool ImageModList::empty()
{
	// indicates list state
	return (modList.size() == 0);
}

int ImageModList::size()
{
	// number of modifications applied
	return modList.size();
}

// set of functions to access image modifications

bool ImageModList::next(ImageMod &mod)
{
	if (modList.size() == 0)
		return false;          // list is empty, nothing to return

	++it; // advance iterator
	if (it == modList.end())
		return false;          // went past last modification, nothing to return

	mod = *it;                 // found next modification, so return it
	return true;
}

bool ImageModList::previous(ImageMod &mod)
{
	if (modList.size() == 0)
		return false;          // list is empty, nothing to return

	--it; // back iterator up one position
	if (it == modList.end())
		return false;          // went past first modification, nothing to return

	mod = *it;                 // found previous modification, so return it
	return true;
}

bool ImageModList::current(ImageMod &mod)
{
	if (modList.size() == 0)
		return false;          // list is empty, nothing to return

	if (it == modList.end())
		return false;          // went past first of last modification, nothing to return

	mod = *it;                 // found return current modification
	return true;
}

bool ImageModList::last(ImageMod &mod)
{
	if (modList.size() == 0)
		return false;          // list is empty, nothing to return

	it = modList.end();
	--it; // back up one position, to last modification
	mod = *it;
	return true;
}

bool ImageModList::first(ImageMod &mod)
{
	if (modList.size() == 0)
		return false;          // list is empty, nothing to return

	it = modList.begin();
	mod = *it;
	return true;
}

// set of functions to access image modifications

void ImageModList::add(ImageMod mod)
{
	modList.push_back(mod);
	it = modList.begin();
	--it;
	--it;
}

bool ImageModList::remove()
{
	// always removes last modification, if any

	if (modList.size() == 0)
		return false;
	
	--it;
	modList.pop_back();
	return true;
}

bool ImageModList::imageIsReversed()
{

	//***1.8 - if information available, set up FRAME label to indicate any image reversal
	bool flipped = false;
	for (list<ImageMod>::iterator itLocal = modList.begin(); itLocal != modList.end(); ++itLocal)
	{
		if (itLocal->getType() == ImageMod::IMG_flip)
			flipped = ! flipped;
	}
	return flipped;
}
