//*******************************************************************
//   file: ImageMod.h
//
// author: J H Stewman 1/24/2007
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

#ifndef IMAGE_MOD_H
#define IMAGE_MOD_H

#include <list> // the STL list

class ImageMod
{
	public:

		enum ImageModType {IMG_none, IMG_flip, IMG_contrast, IMG_brighten, IMG_crop, IMG_undo, IMG_redo};

		ImageMod(ImageModType op, int val1=0, int val2=0, int val3=0, int val4=0);
			// the values are used depending on the ImageModtype
			// op == IMAG_flip, no values used
			// op == IMG_contrast, min is val1, and max is val2
			// op == IMG_brighten, amount is val1
			// op == IMG_crop, xMin is val1, yMin is val2, xMax is val3, yMax is val4
			// op == IMG_undo, no values used
			// op == IMG_redo, no values used

		// no need for special destructor, copy constructor or assignment op

		void set(ImageModType op, int val1=0, int val2=0, int val3=0, int val4=0);
			// the values are used depending on the ImageModtype
			// op == IMAG_flip, no values used
			// op == IMG_contrast, min is val1, and max is val2
			// op == IMG_brighten, amount is val1
			// op == IMG_crop, xMin is val1, yMin is val2, xMax is val3, yMax is val4
			// op == IMG_undo, no values used
			// op == IMG_redo, no values used
		
		void get(ImageModType &op, int &val1, int &val2, int &val3, int &val4);
			// the values are used depending on the ImageModtype
			// op == IMAG_flip, no values used
			// op == IMG_contrast, min is val1, and max is val2
			// op == IMG_brighten, amount is val1
			// op == IMG_crop, xMin is val1, yMin is val2, xMax is val3, yMax is val4
			// op == IMG_undo, no values used
			// op == IMG_redo, no values used

		ImageModType getType(void);

	private:
		ImageModType 
			op;              // image modification type
		int 
			min, max,        // values used in contrast modification
			amount,          // amount adjusted +/- for brightness adjustment
			xMin, yMin,      // boundaries for cropping
			xMax, yMax;      // ditto
};

class ImageModList
{
	// this is a list of image modifications and an interator for accessing the
	// current place in the transformation list
	public:
		ImageModList();
		~ImageModList(); // may not be needed

		void clear(); // create empty list
		bool empty(); // indicates list state
		int size();   // number of modifications applied

		// set of functions to access image modifications
		bool next(ImageMod &mod);
		bool previous(ImageMod &mod);
		bool current(ImageMod &mod);
		bool last(ImageMod &mod);
		bool first(ImageMod &mod);

		// set of functions to access image modifications
		void add(ImageMod mod);
		bool remove();  // always removes last modification

		bool imageIsReversed(void); // indicates state of image (horizontal reversal)

	private:
		std::list<ImageMod> 
			modList;         // the actual list of image modifications
		std::list<ImageMod>::iterator
			it;
};

#endif