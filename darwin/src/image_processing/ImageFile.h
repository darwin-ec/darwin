//                                            *
//   file: ImageFile.h
//
// author: Adam Russel
//
//   mods: K R Debure (2006)
//         -- support for JPEG images
//         J H Stewman (2008)
//         -- PNG file support
//         -- Fixes to improper loading of grayscale JPG & BMP images
//
// Notes: 
// 	* There's not a lot of error checking, so invalid image files can
// 	  cause some weird problems.
//
// 	* The design of this class is not as good as it could be: it
// 	  dynamically allocates the data array itself rather than using
// 	  base class routines.
//
//                                            *

//#define DEBUG
#ifndef IMAGEFILE_H
#define IMAGEFILE_H

#ifdef WIN32
#define PATH_SLASH "\\"
#else
#define PATH_SLASH "/"
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#include <fstream>
#pragma warning(disable:4786) //  1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>

#include "../Error.h"
#include "Image.h"
#include "types.h"
#include "../utility.h"

#include "ImageMod.h"
#include "transform.h" //  1.95

//  Includes for JPEG reader
#include <stdio.h>
#ifdef WIN32
extern "C"{
/  2.22 - these two macros get redifined inside jpeglib.h (jconfig.vc) */
#undef HAVE_STDDEF_H
#undef HAVE_STDLIB_H
#include "../../jpeg-6b/src/jpeglib.h" 
#include "../../jpeg-6b/src/jerror.h"
}
#else
extern "C"{
// this is a hack - the new Gtk+2.24 stuff is in my (JHS) home dir
// -I$(HOME)/gtk/inst/include must be set to build / make for Mac
#include "jpeglib.h" 
#include "jerror.h"
}
#endif
#include <setjmp.h>

#include "png.h"
#include "../../png/pngFile.h"

#ifdef DEBUG
#include <iostream>
#endif

template <class PIXEL_TYPE>
class ImageFile : public Image<PIXEL_TYPE> {
public:
	// The default constructor tries to open the specified file
	// and guess its format.  From there, it passes the job on
	// to the appropriate handler.
	//
	// Parameters:
	// 	string filename - The name of the image file.
	// 	
	ImageFile(const std::string &filename);
	ImageFile(); //  2.0 - moved implementation below
	ImageFile(unsigned nRows, unsigned nCols);
	ImageFile(unsigned nRows, unsigned nCols, PIXEL_TYPE *data);

	//  1.5 - new generic load
	void load(const std::string &filename);

	// save
	// 	Writes the contents of this image file to disk
	// under the specified filename.  Attempts to guess the
	// filetype based on the filename extension.
	//
	// Parameters:
	// 	string filename - The name of the file this image
	// 			  is to be saved under.
	// Return: bool - indicating whether the function was
	// 		  successful.
	bool save(const std::string &filename);

	//  1.8 - used to initiate saving modified image, with modification list
	//         and original image filename inside PGM file as comments
	bool save_wMods(std::string filename,
					std::string originalFilename,
					ImageModList theMods);

	bool writeRawData(std::ofstream& outfile) const;
	bool readRawData(std::ifstream& infile);
	
	///////////////////////////////////////////////////////////////////
	// Some error classes that get thrown as exceptions.
	class ImageFileNotFound : public Error {
	public:
		ImageFileNotFound() : Error("There was an error reading the specified\n"
					 "file.  Please check that the filename is\n"
					 "correct.")
		{ }

		ImageFileNotFound(const std::string &s)
			: Error("Image file not found: " + s)
		{ }
	};

	class UnsupportedImageFormat : public Error {
	public:
		UnsupportedImageFormat() : Error("Unsupported image format.\n"
						 "Darwin supports BMP, PNG, JPG, PGM and PPM files.")
		{ }

		UnsupportedImageFormat(const std::string &s)
			: Error("Unsupported image format in: " + s)
		{ }
	};

	class BadFilename : public Error {
		public: BadFilename() : Error("Bad filename.") { }
	};

	class SaveError : public Error {
		public: SaveError() : Error("Problem writing to file.") { }
	};
	// End of error stuff
	///////////////////////////////////////////////////////////////////

	// allow access to base class members without resolution
	using Image<PIXEL_TYPE>::mData;
	using Image<PIXEL_TYPE>::mRows;
	using Image<PIXEL_TYPE>::mCols;
	using Image<PIXEL_TYPE>::mInitialized;

	//  1.5 - new member to allow access to scale value stored in comment field
	// of *-withDarwinMods.ppm" file -- this scale is the ratio of the modified
	// image scale and the Normalized fin outline scale
	float mNormScale; //  1.5
	ImageModList mImageMods; //  1.8
	std::string mOriginalImageFilename; //  1.8

	bool mBuiltFromMods; //  2.0

	bool loadPNGcommentsOnly(const std::string &filename); //  1.85

private:

	bool loadJPG(const std::string &filename);
	bool loadPNM(std::ifstream& infile);
	bool loadRawPGM(std::ifstream& infile);
	bool loadAsciiPGM(std::ifstream& infile);
	bool loadRawPPM(std::ifstream& infile);
	bool loadPNG(const std::string &filename);

	bool loadBMP(std::ifstream& infile);

	bool saveRawPPM(const std::string &filename) const;
	bool saveRawPGM(const std::string &filename) const;
	bool saveAsciiPGM(const std::string &filename) const;
	bool savePNG(const std::string &filename);
	bool savePNGwThumbOnly(const std::string &filename); //  1.95

};

template <class PIXEL_TYPE>
ImageFile<PIXEL_TYPE>::ImageFile(const std::string &filename)
	: Image<PIXEL_TYPE>(),
	  mNormScale(1.0f), //  1.5
	  mOriginalImageFilename(""), //  1.9
	  mBuiltFromMods(false) //  2.0
{
	//printf("LoadingC: %s\n",filename.c_str());
	std::ifstream infile(filename.c_str(), std::ios::binary);

	if (!infile) throw ImageFileNotFound(filename); //  1.5 - passing filename now

	//printf("LoadingC: %s\n",filename.c_str());

	char firstBytes[2];
	infile.read(firstBytes, 2);

#ifdef DEBUG
	std::cout << "Reading from: " << filename << " first bytes: "
	     << (int)((unsigned char)firstBytes[0]) << " " <<(int)((unsigned char)firstBytes[1]) << std::endl;
#endif

	if (firstBytes[0] == 'P' && '1' <= firstBytes[1] && firstBytes[1] <= '6') {
		//We probably have a PNM file
		loadPNM(infile);

	} else if (firstBytes[0] == 'B' && firstBytes[1] == 'M') {
		//We probably have a BMP file
		loadBMP(infile);

	} else if ((unsigned char)firstBytes[0] == 0xFF && (unsigned char)firstBytes[1] == 0xD8) {
		//We probably have a JPG file
#ifdef DEBUG
		std::cout << "JPEG detected" << std::endl;
#endif
		infile.close ();
		loadJPG(filename);

	} else if ((unsigned char)firstBytes[0] == 137 && firstBytes[1] == 'P') {
		// probably have a PNG file
		infile.close();
		loadPNG(filename); // this will reopen and close file

	} else {
		infile.close ();
		throw UnsupportedImageFormat();
	}
}

template <class PIXEL_TYPE>
inline ImageFile<PIXEL_TYPE>::ImageFile() //  2.0 - moved here so intitialization is done properly
	: Image<PIXEL_TYPE>(),
	  mNormScale(1.0f), //  1.5
	  mOriginalImageFilename(""), //  1.9
	  mBuiltFromMods(false) //  2.0
{ }

template <class PIXEL_TYPE>
inline ImageFile<PIXEL_TYPE>::ImageFile(unsigned nRows, unsigned nCols)
	: Image<PIXEL_TYPE>(nRows, nCols),
	  mNormScale(1.0f), //  1.5
	  mOriginalImageFilename(""), //  1.9
	  mBuiltFromMods(false) //  2.0
{ }

template <class PIXEL_TYPE>
inline ImageFile<PIXEL_TYPE>::ImageFile(unsigned nRows, unsigned nCols, PIXEL_TYPE *data)
	: Image<PIXEL_TYPE>(nRows, nCols, data),
	  mNormScale(1.0f), //  1.5
	  mOriginalImageFilename(""), //  1.9
	  mBuiltFromMods(false) //  2.0
{ }

///////////////////////////
template <class PIXEL_TYPE>
void ImageFile<PIXEL_TYPE>::load(const std::string &filename)
{
	//printf("LoadingL: %s\n",filename.c_str());
	std::ifstream infile(filename.c_str(), std::ios::binary);

	if (!infile) throw ImageFileNotFound(filename); //  1.5 - passing filename now

	//printf("LoadingL: %s\n",filename.c_str());

	char firstBytes[2];
	infile.read(firstBytes, 2);

#ifdef DEBUG
	std::cout << "Reading from: " << filename << " first bytes: "
	     << (int)((unsigned char)firstBytes[0]) << " " <<(int)((unsigned char)firstBytes[1]) << std::endl;
#endif

	mNormScale = 1.0f;  //  1.9
	mOriginalImageFilename = ""; //  1.9

	if (firstBytes[0] == 'P' && '1' <= firstBytes[1] && firstBytes[1] <= '6') {
		//We probably have a PNM file
		loadPNM(infile);

	} else if (firstBytes[0] == 'B' && firstBytes[1] == 'M') {
		//We probably have a BMP file
		loadBMP(infile);

	} else if ((unsigned char)firstBytes[0] == 0xFF && (unsigned char)firstBytes[1] == 0xD8) {
		//We probably have a JPG file
#ifdef DEBUG
		std::cout << "JPEG detected" << std::endl;
#endif
		infile.close ();
		loadJPG(filename);

	} else if ((unsigned char)firstBytes[0] == 137 && firstBytes[1] == 'P') {
		// probably have a PNG file
		infile.close();
		loadPNG(filename); // this will reopen and close file

		//  1.9 - if we are going to load and rebuild the modified image
		// from the original image here is the place to do it

		// load PNG file comments only
		//loadPNGcommentsOnly(filename);

		// then load the original image
		//string fullName = filename.substr(0,filename.rfind(PATH_SLASH) + 1);
		//fullName += mOriginalImageFilename;

		//load(fullName);

		// then apply the transformations in sequence

	} else {
		infile.close ();
		throw UnsupportedImageFormat();
	}
}
///////////////////////////

template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::save(const std::string &filename)
{
	if (filename == "") throw BadFilename();

	// Find the filename extension.
	
	std::string::size_type idx = filename.rfind(".");

	
	std::string extension;
	
	if (idx == std::string::npos)
		extension = "";
	else
		extension = filename.substr(idx + 1);
	
#ifdef DEBUG
	std::cout << "Saving " << filename << " which is of type " << extension << std::endl;
#endif

	transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	if ("ppm" == extension)
		saveRawPPM(filename);
	else if ("pgm" == extension)
		saveRawPGM(filename);
	else if ("png" == extension)
		savePNG(filename);
	else
		throw UnsupportedImageFormat("save()");

	return true; // should remove this now that exception stuff is in..but lazy
}

/////////////////////////////////////////////////////////////////////
//
template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::save_wMods(std::string filename,
									   std::string originalFilename,
									   ImageModList theMods)
{
	if (filename == "") throw BadFilename();

	// Find the filename extension.
	
	std::string::size_type idx = filename.rfind(".");

	
	std::string extension;
	
	if (idx == std::string::npos)
		extension = "";
	else
		extension = filename.substr(idx + 1);
	
#ifdef DEBUG
	std::cout << "Saving " << filename << " which is of type " << extension << std::endl;
#endif
	transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	if ("png" == extension)
	{
		mImageMods = theMods;                      // set so savePNG() can access them
		mOriginalImageFilename = originalFilename; // set so savePNG() can access them
		//savePNG(filename);
		savePNGwThumbOnly(filename); //  1.95 - test thumbnail only
	}
	else
		throw UnsupportedImageFormat("save_wMods() called without PGM file extension");

	return true; // should remove this now that exception stuff is in..but lazy
}
/////////////////////////////////////////////////////////////////////
//
template <class PIXEL_TYPE>
inline bool ImageFile<PIXEL_TYPE>::writeRawData(std::ofstream& outfile) const
{
	outfile.write((char*)mData, sizeof(PIXEL_TYPE) * mRows * mCols);
	return true;
}

template <class PIXEL_TYPE>
inline bool ImageFile<PIXEL_TYPE>::readRawData(std::ifstream& infile)
{
	// Very dangerous =]
	infile.read((char*)mData, sizeof(PIXEL_TYPE) * mRows * mCols);
	return true;
}

template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::loadJPG(const std::string &filename)
/* The code for reading JPEG files comes from the jpeg-6b code
 * developed by the IJG.  See source code in jpeg directory
 */
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * infile;                /* source file */
  JSAMPARRAY buffer;            /* Output row buffer */
  int row_stride;               /* physical row width in output buffer */
  int rowcount=0;

  /* We reopen the file as a FILE* since the jpeg C code expects it */
  if ((infile = fopen(filename.c_str(), "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename.c_str());
    return 0;
  }

  /* Step 1: allocate and initialize JPEG decompression object */
  /* We set up the normal JPEG error routines, 
     We also need to add code to override error_exit. See commented code below */
  cinfo.err = jpeg_std_error(&jerr);
  
  /*jerr.pub.error_exit = my_error_exit; // for now exit(1) if error on read */
  /* Establish the setjmp return context for my_error_exit to use. 
  if (setjmp(jerr.setjmp_buffer)) {
    // If we get here, the JPEG code has signaled an error.
    // We need to clean up the JPEG object, close the input file, and return.     
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  } replace w/ one at end of line begin w/establish */

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */
  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */
  (void) jpeg_read_header(&cinfo, TRUE);
  
  /* Step 5: Start decompressor, (step 4, setting the decompression
   * parameters, is done with info read from the header 
   */
 
  (void) jpeg_start_decompress(&cinfo);
  row_stride = cinfo.output_width * cinfo.output_components;
#ifdef DEBUG
  std::cout << "outComponents: " << cinfo.output_components << std::endl; //  1.95
#endif
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */
  mRows = cinfo.output_height;
  mCols = cinfo.output_width;
  mData = new PIXEL_TYPE[mRows * mCols];
  JSAMPROW curRowR;
  unsigned r=0;
  while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
	curRowR = buffer[0];
    /* Put_scanline into mData. */
    for(unsigned c=0; c < mCols; c++){
		if (cinfo.output_components == 3)
			mData[r * mCols + c].setRGB(*(curRowR), *(curRowR+1), *(curRowR+2));
		else if (cinfo.output_components == 1)
			mData[r * mCols + c].setRGB(*(curRowR), *(curRowR), *(curRowR));
		else 
			throw Error("This JPEG image is neither grayscale nor true color.\n"
				            "DARWIN cannot handle this image format.");
		//curRowR += 3;
		curRowR += cinfo.output_components; //  1.95
	}
	r++;
  }

  /* Step 7: Finish decompression */
  (void) jpeg_finish_decompress(&cinfo);

  /* Step 8: Release JPEG decompression object */
  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  fclose(infile);
  mInitialized = true;
	
  return true;
}


template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::loadPNM(std::ifstream& infile)
{
	char
		PNMType[255],
		maxpixval[255], //not used much yet
		junk[255];

	int
		numRows,
		numCols;

	infile.seekg (0, std::ios::beg);

	// Grab the first line which should include the PNM #
	infile.getline (PNMType, 255); 

	while (infile.peek() == '#') //If there's a comment
	{
		infile.getline (junk,255,'\n'); //ignore the line
		//  1.5 - new code to grab scale if found in a comment line
		std::string num = junk;
		if (num.substr(0,11) == "#NormScale:")
		{
			num = num.substr(11);  // grab rest of line (the number)
			mNormScale = atof(num.c_str());
		}
	}

	infile >> numCols >> numRows; //Get the row and column information

	if (numCols <= 0 || numRows <= 0) {
		infile.close ();
		return false;
	}

	infile.getline (junk,255,'\n'); //Trash this line
	infile.getline (maxpixval, 255, '\n');

	switch (PNMType[1]) {
		case '2':	
				mRows = numRows;
				mCols = numCols;
				return loadAsciiPGM(infile);
				break;

		case '5':
				mRows = numRows;
				mCols = numCols;
				return loadRawPGM(infile);
				break;

		case '6':
				mRows = numRows;
				mCols = numCols;
				return loadRawPPM(infile);
				break;
		case '1':
		case '3':
		case '4':
		default:
				// Can't read this type of PNM =(
				return false;
	}
}

template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::loadAsciiPGM(std::ifstream& infile)
{
	unsigned pixVal;

	mData = new PIXEL_TYPE[mRows * mCols];

	for(unsigned r =0; r < mRows && infile.good(); r++)
		for(unsigned c=0; c < mCols && infile.good() && infile >> pixVal; c++)
			mData[r * mCols + c] = pixVal;

	mInitialized = true;

	return true;
}


template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::loadRawPGM(std::ifstream& infile)
{
	mData = new PIXEL_TYPE[mRows * mCols];

	if (mData[0].isColor()) { // Color.. we'll have to do something slightly fancier
		byte pixVal;
		
		for (unsigned r=0; r < mRows; r++)
			for (unsigned c = 0; c < mCols && infile >> pixVal; c++)
				mData[r * mCols + c] = pixVal;

	} else // It's gray, just read() the entire thing at once =)
		infile.read((char*)mData, sizeof(PIXEL_TYPE) * mRows * mCols);
		
	mInitialized = true;

	return true;
}

template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::loadRawPPM(std::ifstream& infile)
{
	mData = new PIXEL_TYPE[mRows * mCols];

	if (mData[0].isColor())
		infile.read((char*)mData, sizeof(PIXEL_TYPE) * mRows * mCols);
	else {
		byte rgb[3];
		
		for (unsigned r = 0; r < mRows; r++) {
			for (unsigned c = 0; c < mCols; c++) {
				infile.read((char*)rgb, 3);
				mData[r * mCols + c].setRGB(rgb[0], rgb[1], rgb[2]);
			}
		}
	}		
	mInitialized = true;
	
	return true;
}

template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::loadBMP(std::ifstream& infile)
{
	unsigned char byteString[4];

	int
		readTemp,
		numRows,
		numCols,
		startpos,
		bitsPerPixel;

	byte map[256][4];    //  1.95 - in case we need a color table
	unsigned char pixID; //  1.95

	// read start pos of image
	infile.seekg(10, std::ios::beg);

	infile.read((char*)byteString, 4);
	readTemp = byteStringToInt(byteString, 4);
	
	startpos = byteSwap(readTemp);

	// read the size of the header
	infile.read ((char*)byteString, 4);
	readTemp = byteStringToInt(byteString, 4);
	int cbFix = byteSwap(readTemp);

	int colorPlanes, compression, colorsImportant, colorsUsed;

	switch (cbFix) {
		// MS Windows Bitmap
		case 40: 
			// read numbers of columns and rows
			infile.read((char*)byteString, 4);
			readTemp = byteStringToInt(byteString, 4);
	
			numCols =  byteSwap(readTemp);

			infile.read((char*)byteString, 4);
			readTemp = byteStringToInt(byteString, 4);
	
			numRows = byteSwap(readTemp);
			
			infile.read((char*)byteString, 2);
			byteString[2] = byteString[3] = 0;
			readTemp = byteStringToInt(byteString, 4);
			colorPlanes = byteSwap(readTemp);
			
			infile.read((char*)byteString, 2);
			byteString[2] = byteString[3] = 0;
			readTemp = byteStringToInt(byteString, 4);
			bitsPerPixel = byteSwap(readTemp);

			infile.read((char*)byteString, 4);
			readTemp = byteStringToInt(byteString, 4);
			compression = byteSwap(readTemp);

			if (compression != 0)
				throw Error("This image is compressed.  DARWIN\n"
				            "cannot handle compressed BMPs.");

			// Move past some header junk
			// ImageSize, XPixelsPerM, YPixelsPerM
			infile.seekg(12, std::ios::cur);
			
			infile.read((char*)byteString, 4);
			readTemp = byteStringToInt(byteString, 4);
			colorsUsed = byteSwap(readTemp);

			infile.read((char*)byteString, 4);
			readTemp = byteStringToInt(byteString, 4);
			colorsImportant = byteSwap(readTemp);

			break;
			
		default:
			throw UnsupportedImageFormat("readBMP()");
			// not that the break needs to be here...
			break;
	}
		
	bool topDown = false;
	// check if image is stored bottom up or topdown
  	if (numRows < 0) {
		numRows *= -1;
		topDown = true;
	}
	
	// Check to see if this is a nice easy kind of BMP
	// to read.  If not, tell the calling function this
	// wasn't a successful read.
	if ((bitsPerPixel != 24) && (bitsPerPixel != 8)) //  1.95 add support for 256 color images
		throw UnsupportedImageFormat("readBMP()");

	if (bitsPerPixel == 8) //  1.95
	{
		// build color map
		for (int i = 0; i < colorsUsed; i++)
		{
			byte rgb[4];
			infile.read((char*)rgb, 4 * sizeof(byte));
			map[i][0] = rgb[0];
			map[i][1] = rgb[1];
			map[i][2] = rgb[2];
			map[i][3] = 0;
#ifdef DEBUG
			std::cout << (int)map[i][0] << (int)map[i][1] << (int)map[i][3] << std::endl;
#endif
		}
	}

	// skip to the bitmap mData
	infile.seekg(startpos, std::ios::beg); //  1.95 moved from above previous check

	mRows = numRows;
	mCols = numCols;
	mData = new PIXEL_TYPE[mRows * mCols];

#ifdef DEBUG
	std::cout << "Reading BMP file:" << std::endl
		 << "Starting position: " << startpos << std::endl
	     << numCols << " x " << numRows << std::endl
	     << bitsPerPixel << " bpp" << std::endl;
	
	if (topDown)
		std::cout << "Top down.";
	else
		std::cout << "Bottom up.";

	std::cout << std::endl;
#endif

	byte rgb[3];

	int numBytes;

	if (topDown) {
		for (unsigned r = 0; r < mRows; r++) {
			
			numBytes = 0;
			
			for (unsigned c = 0; c < mCols; c++) {
				if (bitsPerPixel == 24)
				{
					infile.read((char*)rgb, 3 * sizeof(byte));

					// should do this from the read to check
					// for errors
					numBytes += 3;
				
					mData[r * mCols + c].setRGB(rgb[2], rgb[1], rgb[0]);
				}
				else if (bitsPerPixel == 8)
				{
					infile.read((char*)&pixID, sizeof(byte));

					numBytes += 1;
				
					mData[r * mCols + c].setRGB(map[pixID][2], map[pixID][1], map[pixID][0]);
				}
			}

			// Rows are padded to multiples of 4
			// bytes...read to the end of this row
			// This is a really lazy way to do it
			while (numBytes % 4) {
				infile.get();
				numBytes++;
			}
		}
	} else { //bottom up
		for (int r = (int)mRows - 1; r >= 0; r--) {

			numBytes = 0;

			for (int c = 0; c < (int)mCols; c++) {
				if (bitsPerPixel == 24)
				{
					infile.read((char*)rgb, 3 * sizeof(byte));
					// should do this from the read to check
					// for errors
					numBytes += 3;

					mData[r * mCols + c].setRGB(rgb[2], rgb[1], rgb[0]);
				}
				else if (bitsPerPixel == 8)
				{
					infile.read((char*)&pixID, sizeof(byte));

					numBytes += 1;
				
					mData[r * mCols + c].setRGB(map[pixID][2], map[pixID][1], map[pixID][0]);
				}
			}

			// Rows are padded to multiples of 4
			// bytes...read to the end of this row
			// This is a really lazy way to do it
			while (numBytes % 4) {
				infile.get();
				numBytes++;
			}
		}
	}
	mInitialized = true;

	return true;
}

///////////
template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::loadPNG(const std::string &filename)
{

	char name[512];
	sprintf(name,"%s",filename.c_str()); // work around const parameter

	png_bytep ppbImageData = NULL;
	int piChannels;
	png_color pBkgColor;
	
	png_textp comment;    // array of strings for image file comments
	int num_comments;    // count of comments

	int rows, cols;

	int rval = PngLoadImage (name, &ppbImageData, &cols, &rows, &piChannels, &pBkgColor,
		&comment, &num_comments);

	// if it is a thumbnail only, then modified image must be rebuilt from original
	bool thumbOnly = false; //  1.95 

	// code to grab scale, original name and image modifications if found on comment lines
	mImageMods.clear();
	mOriginalImageFilename = "";
	for (int iC = 0; iC < num_comments; iC++)
	{
		std::string thisKey = comment[iC].key;
		std::string thisVal = comment[iC].text;
		//printf("%s: %s\n",thisKey.c_str(), thisVal.c_str());
		if (thisKey == "ThumbOnly")
		{
			thumbOnly = (strcmp("yes",thisVal.c_str()) == 0);
		}
		else if (thisKey == "NormScale")
		{
			mNormScale = atof(thisVal.c_str());
			//printf("got mNormScale: %f\n",mNormScale);
		}
		else if (thisKey == "OriginalImage")
		{
			mOriginalImageFilename = thisVal;
		}
		else if (thisKey == "ImageMod")
		{
			int op, v1, v2, v3, v4;
			sscanf(comment[iC].text,"%d %d %d %d %d", &op, &v1, &v2, &v3, &v4);
			mImageMods.add(ImageMod((ImageMod::ImageModType)op, v1, v2, v3, v4));
		}
		// free space allocated in PngLoadImage() using calloc
		delete [] comment[iC].key;
		delete [] comment[iC].text;
	}

	if (! thumbOnly)
	{
		// need code here to set mCols, mRows, and mData
		mRows = rows;
		mCols = cols;
		mData = new PIXEL_TYPE[mRows * mCols];

		for (unsigned i=0; i < mRows /*image->ReturnNumRows()*/; i++)
			for (unsigned j=0; j < mCols /*image->ReturnNumCols()*/; j++)
			{
				mData[i * mCols + j].setRGB(
					*(ppbImageData+(i*mCols*3+j*3)),
					*(ppbImageData+(i*mCols*3+j*3+1)),
					*(ppbImageData+(i*mCols*3+j*3+2)));
			}
	}
	else //  1.95 - all new code below, to build modified image from original
	{
		// this PNG file contains only a thumbnail, so the modified image must
		// be rebuilt from the original and the sequence of image mods

		std::string 
			fullOriginalName = filename.substr(0,filename.rfind(PATH_SLASH)+1) 
		                     + mOriginalImageFilename;

		ImageFile *temp = new ImageFile(fullOriginalName); // load original

		ImageFile *temp2;

		//mRows = temp->mRows;
		//mCols = temp->mCols;
		//mData = new PIXEL_TYPE[mRows * mCols]; // assume color image

		ImageMod mod(ImageMod::IMG_none);
		int op, v1, v2, v3, v4;
		int r,c;

		bool gotOne = mImageMods.first(mod); // get first mod
		while (gotOne)
		{
			mod.get((ImageMod::ImageModType &)op,v1,v2,v3,v4);
			switch (mod.getType())
			{
			case ImageMod::IMG_none:
				break;
			case ImageMod::IMG_flip:
				temp2 = flipHorizontally(temp);
				delete temp;
				temp = temp2;
				break;
			case ImageMod::IMG_contrast:
				temp2 = enhanceContrast(temp, v1, v2);
				delete temp;
				temp = temp2;
				break;
			case ImageMod::IMG_brighten:
				temp2 = alterBrightness(temp, v1);
				delete temp;
				temp = temp2;
				break;
			case ImageMod::IMG_crop:
				temp2 = crop(temp, v1, v2, v3, v4);
				delete temp;
				temp = temp2;
				break;
			case ImageMod::IMG_undo:
				break;
			case ImageMod::IMG_redo:
				break;
			default:
				break;
			}	
				
			gotOne = mImageMods.next(mod); // get next mod
		}
					
		mBuiltFromMods = thumbOnly; //  2.0

		mRows = temp->mRows;
		mCols = temp->mCols;
		mData = new PIXEL_TYPE[mRows * mCols]; // assume color image
	
		memcpy(mData,temp->mData,mRows*mCols*3);
	
		delete temp; //  1.95
	}

	delete [] comment;

	delete [] ppbImageData; // free allocated memory

	mInitialized = true; // otherwise mData never deleted by destructor

	return (rval != 0);
}

///////////

template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::loadPNGcommentsOnly(const std::string &filename)
{

	char name[512];
	sprintf(name,"%s",filename.c_str()); // work around const parameter
	
	png_textp comment;      // array of strings for image file comments
	int num_comments(0);    // count of comments

	int rval = PngLoadImageComments (name, &comment, &num_comments);

	// loading of comments may fail for seveal reasons.  One way it can fail
	// is when the function was called during database backup with an older database 
	// not containing PNG images as the modified images.  If reading comments fails
	// we leave the modified image filename and comments at default values

	// code to grab scale, original name and image modifications if found on comment lines
	mImageMods.clear();
	mOriginalImageFilename = "";

	if (rval) //  1.93 - comments were really read from PNG file
	{
		for (int iC = 0; iC < num_comments; iC++)
		{
			std::string thisKey = comment[iC].key;
			std::string thisVal = comment[iC].text;
			printf("%s: %s\n",thisKey.c_str(), thisVal.c_str());
			if (thisKey == "NormScale")
			{
				mNormScale = atof(thisVal.c_str());
				//printf("got mNormScale: %f\n",mNormScale);
			}
			else if (thisKey == "OriginalImage")
			{
				mOriginalImageFilename = thisVal;
			}
			else if (thisKey == "ImageMod")
			{
				int op, v1, v2, v3, v4;
				sscanf(comment[iC].text,"%d %d %d %d %d", &op, &v1, &v2, &v3, &v4);
				mImageMods.add(ImageMod((ImageMod::ImageModType)op, v1, v2, v3, v4));
			}
			// free space allocated in PngLoadImageComments() using calloc
			delete [] comment[iC].key;
			delete [] comment[iC].text;
		}

		delete [] comment;
	}

	mInitialized = true; // otherwise mData never deleted by destructor

	return (rval != 0);
}

///////////

template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::saveRawPPM(const std::string &filename) const
{
	std::ofstream outfile(filename.c_str(), std::ios::binary);

	if (!outfile) throw SaveError();

	outfile <<"P6"<<std::endl
		<< "#Created by: " << PACKAGE << " " << VERSION << std::endl
		<< "#NormScale: " << mNormScale << std::endl //  1.5 - save the normalization scale
		<< mCols << " " << mRows << std::endl
		<< 255 << "\n";

	if (mData[0].isColor())
		outfile.write((char*)mData, sizeof(PIXEL_TYPE) * mRows * mCols);
	else {
		for (unsigned r = 0; r < mRows; r++) {
			for (unsigned c = 0; c < mCols; c++) {
				outfile << mData[r * mCols + c].getIntensity()
					<< mData[r * mCols + c].getIntensity()
					<< mData[r * mCols + c].getIntensity();
			}
		}
	}

	outfile.close();

	return true;
}

template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::saveRawPGM(const std::string &filename) const
{
	std::ofstream outfile(filename.c_str(), std::ios::binary);

	if (!outfile) throw SaveError();

	outfile << "P5"<<std::endl
		<< "#Created by: " << (PACKAGE) << " " << (VERSION) << std::endl
		<< mCols << " " << mRows << std::endl
		<< 255 << "\n";

	if (mData[0].isColor()) {
		for (unsigned r = 0; r < mRows; r++)
			for (unsigned c = 0; c < mCols; c++)
				outfile << mData[r * mCols + c].getIntensity();
	} else
		outfile.write((char*)mData, sizeof(PIXEL_TYPE) * mRows * mCols);
		
	outfile.close();

	return true;
}

template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::saveAsciiPGM(const std::string &filename) const
{
	std::ofstream outfile(filename.c_str());

	if (!outfile) throw SaveError();

	outfile << "P2"<<std::endl
		<< "#Created by: " << VERSION << std::endl
		<< mCols << " " << mRows << std::endl
		<< 255 <<std::endl;
	
	for (unsigned i=0; i < mRows /*image->ReturnNumRows()*/; i++)
		for (unsigned j=0; j < mCols /*image->ReturnNumCols()*/; j++)
			outfile << mData[i * mCols + j].getIntensity() << " ";

	outfile.close();
	return true;
}

//  1.95 - new function to write PNG file (with Thumnail & xformations ONLY)

template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::savePNGwThumbOnly(const std::string &filename)
{
	png_color bkgColor = {127, 127, 127};
	
	//png_byte *data = new png_byte[mRows*mCols*3];

	// NOTE: thumbnail will be width x 128, or 128 x height, where width and height
	// are in the range 1..128.

	int 
		rowsTall(128), startRow(0), 
		colsWide(128), startCol(0);

	if (mCols > mRows)
	{
		// image is wider than it is tall
		rowsTall = (int)((128.0 * mRows / mCols) + 0.5);
		startRow = (128 - rowsTall) / 2;
	}
	else
	{
		// image is taller than it is wide
		colsWide = (int)((128.0 * mCols / mRows) + 0.5);
		startCol = (128 - colsWide) / 2;
	}

	float 
		dx = (float)mCols / colsWide,
		dy = (float)mRows / rowsTall;

	png_byte *data = new png_byte[rowsTall*colsWide*3];

	for (unsigned i=0; i < 128; i++)
		for (unsigned j=0; j < 128; j++)
		{
			if ((startRow <= i) && (i < startRow+rowsTall) &&
				(startCol <= j) && (j < startCol+colsWide))
			{
				// create a thumbnail with same aspect ratio as original, 
				// and with largest dimension of 128
				*(data+((i-startRow)*colsWide*3+(j-startCol)*3)) = 
						mData[(int)(dy*(i-startRow))*mCols + (int)(dx*(j-startCol))].getRed();
				*(data+((i-startRow)*colsWide*3+(j-startCol)*3+1)) = 
						mData[(int)(dy*(i-startRow))*mCols + (int)(dx*(j-startCol))].getGreen();
				*(data+((i-startRow)*colsWide*3+(j-startCol)*3+2)) = 
						mData[(int)(dy*(i-startRow))*mCols + (int)(dx*(j-startCol))].getBlue();
			}
		}

	char name[256];
	sprintf(name,"%s",filename.c_str());

	printf("PNG filename: %s\n",name);

	// set comments to be written to image file
	int MaxComments = 3; // leave room for Author, NormScale and ThumbOnly
	const int MAX_NAME = 128;
	char nameBuffer[MAX_NAME]; // short name, no path info
	const int MAX_SCALE = 32;
	char scaleBuffer[MAX_SCALE]; // just a floating point number
	const int MAX_MOD = 64;
	char thumbBuffer[4]; // "yes" or "no"
	char *modBuffer;      // the image modifications
	if ("" != mOriginalImageFilename)
		MaxComments++;
	MaxComments += mImageMods.size();
	png_textp text_ptr = new png_text[MaxComments]; // make room for all comments
	int num_text = 0;

	std::string authorString = "DARWIN-";
	authorString += VERSION;
	authorString += " Dolphin PhotoID Software";
	char authorBuffer[256];
	sprintf(authorBuffer,"%s",authorString.c_str());

	text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
	text_ptr[num_text].key = "Author";
	text_ptr[num_text].text = authorBuffer;
	num_text++;

	text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
	text_ptr[num_text].key = "NormScale";
	sprintf(scaleBuffer,"%f",mNormScale); // mNormScale must be set BEFORE save_wMods() called
	text_ptr[num_text].text = scaleBuffer;
	num_text++;

	if ("" != mOriginalImageFilename)
	{
		text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
		text_ptr[num_text].key = "OriginalImage";
		sprintf(nameBuffer,"%s",mOriginalImageFilename.c_str());
		text_ptr[num_text].text = nameBuffer;
		num_text++;
	}

	text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
	text_ptr[num_text].key = "ThumbOnly";
	sprintf(thumbBuffer,"yes");
	text_ptr[num_text].text = thumbBuffer;
	num_text++;

	if (mImageMods.size() > 0)
	{
		ImageMod imod(ImageMod::IMG_none);
		modBuffer = new char[MAX_MOD*mImageMods.size()]; // each mod is up to 4 ints
		// get the first one
		if (mImageMods.first(imod))
		{
			ImageMod::ImageModType op;
			int v1, v2, v3, v4;
			imod.get(op, v1, v2, v3, v4);
			sprintf(modBuffer,"%d %d %d %d %d", (int)op, v1, v2, v3, v4);
			text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
			text_ptr[num_text].key = "ImageMod";
			text_ptr[num_text].text = modBuffer;
			num_text++;
		}
		//build list of comments reflecting image modifications
		for (int j = 1; j < mImageMods.size(); j++)
		{
			// get the next one
			if (mImageMods.next(imod))
			{
				ImageMod::ImageModType op;
				int v1, v2, v3, v4;
				imod.get(op, v1, v2, v3, v4);
				sprintf((modBuffer+j*MAX_MOD),"%d %d %d %d %d", (int)op, v1, v2, v3, v4);
				text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
				text_ptr[num_text].key = "ImageMod";
				text_ptr[num_text].text = (modBuffer+j*MAX_MOD);
				num_text++;
			}
		}
	}

	int success = PngSaveImage (name, data, colsWide, rowsTall, bkgColor, text_ptr, num_text);

	if (mImageMods.size() > 0)
		delete modBuffer; //  1.9 - wait until buffer use is complete, THEN delete it

	delete data;
	delete [] text_ptr;

	return (success != 0);
}


//  1.8 - new function to reformat data and write PNG file

template <class PIXEL_TYPE>
bool ImageFile<PIXEL_TYPE>::savePNG(const std::string &filename)
{
	png_color bkgColor = {127, 127, 127};
	
	png_byte *data = new png_byte[mRows*mCols*3];

	for (unsigned i=0; i < mRows /*image->ReturnNumRows()*/; i++)
		for (unsigned j=0; j < mCols /*image->ReturnNumCols()*/; j++)
		{
			*(data+(i*mCols*3+j*3)) = mData[i * mCols + j].getRed();
			*(data+(i*mCols*3+j*3+1)) = mData[i * mCols + j].getGreen();
			*(data+(i*mCols*3+j*3+2)) = mData[i * mCols + j].getBlue();
		}

	char name[256];
	sprintf(name,"%s",filename.c_str());

	printf("PNG filename: %s\n",name);

	// set comments to be written to image file
	int MaxComments = 2;
	const int MAX_NAME = 128;
	char nameBuffer[MAX_NAME]; // short name, no path info
	const int MAX_SCALE = 32;
	char scaleBuffer[MAX_SCALE]; // just a floating point number
	const int MAX_MOD = 64;
	char *modBuffer;      // the image modifications
	if ("" != mOriginalImageFilename)
		MaxComments++;
	MaxComments += mImageMods.size();
	png_textp text_ptr = new png_text[MaxComments]; // make room for all comments
	int num_text = 0;

	std::string authorString = "DARWIN-";
	authorString += VERSION;
	authorString += " Dolphin PhotoID Software";
	char authorBuffer[256];
	sprintf(authorBuffer,"%s",authorString.c_str());

	text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
	text_ptr[num_text].key = "Author";
	text_ptr[num_text].text = authorBuffer;
	num_text++;

	text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
	text_ptr[num_text].key = "NormScale";
	sprintf(scaleBuffer,"%f",mNormScale); // mNormScale must be set BEFORE save_wMods() called
	text_ptr[num_text].text = scaleBuffer;
	num_text++;

	if ("" != mOriginalImageFilename)
	{
		text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
		text_ptr[num_text].key = "OriginalImage";
		sprintf(nameBuffer,"%s",mOriginalImageFilename.c_str());
		text_ptr[num_text].text = nameBuffer;
		num_text++;
	}

	if (mImageMods.size() > 0)
	{
		ImageMod imod(ImageMod::IMG_none);
		modBuffer = new char[MAX_MOD*mImageMods.size()]; // each mod is up to 4 ints
		// get the first one
		if (mImageMods.first(imod))
		{
			ImageMod::ImageModType op;
			int v1, v2, v3, v4;
			imod.get(op, v1, v2, v3, v4);
			sprintf(modBuffer,"%d %d %d %d %d", (int)op, v1, v2, v3, v4);
			text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
			text_ptr[num_text].key = "ImageMod";
			text_ptr[num_text].text = modBuffer;
			num_text++;
		}
		//build list of comments reflecting image modifications
		for (int j = 1; j < mImageMods.size(); j++)
		{
			// get the next one
			if (mImageMods.next(imod))
			{
				ImageMod::ImageModType op;
				int v1, v2, v3, v4;
				imod.get(op, v1, v2, v3, v4);
				sprintf((modBuffer+j*MAX_MOD),"%d %d %d %d %d", (int)op, v1, v2, v3, v4);
				text_ptr[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
				text_ptr[num_text].key = "ImageMod";
				text_ptr[num_text].text = (modBuffer+j*MAX_MOD);
				num_text++;
			}
		}
	}

	int success = PngSaveImage (name, data, mCols, mRows, bkgColor, text_ptr, num_text);

	if (mImageMods.size() > 0)
		delete modBuffer; // 1.9 - wait until buffer use is complete, THEN delete it

	delete data;
	delete [] text_ptr;

	return (success != 0);
}


#endif
