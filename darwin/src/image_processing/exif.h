//*******************************************************************
//   file: exif.h
//
// author: Joshua Gregory from Matthias Wandel's original code
//
//   mods: 
//
//
// Exif Extractor Class Header by Joshua Gregory.  The function of the 
// class is to extract the time and date from a Exif header in a jpeg.  
// Code was adapted from Jhead by Matthias Wandel www.sentex.net/~mwandel 
//*******************************************************************

#ifndef _EXIF_H
#define _EXIF_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#pragma warning(disable:4786) //***1.95 removes warnings about id string length in debug version
#include <string>
#include <time.h>
#include <errno.h>
#include <ctype.h>


//this will allow the function to work in unix and windows without extra porting.
#ifdef _WIN32
    #include <sys/utime.h>

#else
    #include <utime.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <errno.h>
    #include <limits.h>
#endif

// using namespace std;

//define a unsigned char as uchar.  Easier to type.
typedef unsigned char uchar;

//Define Max sections of jpeg
#define MAX_SECTIONS 40
#define MAX_DATE_COPIES 10

//Define my tags
#define M_SOF0  0xC0            // Start Of Frame N
#define M_SOF1  0xC1            // N indicates which compression process
#define M_SOF2  0xC2            // Only SOF0-SOF2 are now in common use
#define M_SOF3  0xC3
#define M_SOF5  0xC5            // NB: codes C4 and CC are NOT SOF markers
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8            // Start Of Image (beginning of datastream)
#define M_EOI   0xD9            // End Of Image (end of datastream)
#define M_SOS   0xDA            // Start Of Scan (begins compressed data)
#define M_JFIF  0xE0            // Jfif marker
#define M_EXIF  0xE1            // Exif marker
#define M_COM   0xFE            // COMment 
#define M_DQT   0xDB
#define M_DHT   0xC4
#define M_DRI   0xDD


//Determines where the information is, we only care about datetime
#define TAG_MAKE               0x010F
#define TAG_MODEL              0x0110
#define TAG_ORIENTATION        0x0112
#define TAG_DATETIME           0x0132
#define TAG_THUMBNAIL_OFFSET   0x0201
#define TAG_THUMBNAIL_LENGTH   0x0202
#define TAG_EXPOSURETIME       0x829A
#define TAG_FNUMBER            0x829D
#define TAG_EXIF_OFFSET        0x8769
#define TAG_EXPOSURE_PROGRAM   0x8822
#define TAG_GPSINFO            0x8825
#define TAG_ISO_EQUIVALENT     0x8827
#define TAG_DATETIME_ORIGINAL  0x9003
#define TAG_DATETIME_DIGITIZED 0x9004
#define TAG_SHUTTERSPEED       0x9201
#define TAG_APERTURE           0x9202
#define TAG_EXPOSURE_BIAS      0x9204
#define TAG_MAXAPERTURE        0x9205
#define TAG_SUBJECT_DISTANCE   0x9206
#define TAG_METERING_MODE      0x9207
#define TAG_LIGHT_SOURCE       0x9208
#define TAG_FLASH              0x9209
#define TAG_FOCALLENGTH        0x920A
#define TAG_MAKER_NOTE         0x927C
#define TAG_USERCOMMENT        0x9286
#define TAG_EXIF_IMAGEWIDTH    0xa002
#define TAG_EXIF_IMAGELENGTH   0xa003
#define TAG_INTEROP_OFFSET     0xa005
#define TAG_FOCALPLANEXRES     0xa20E
#define TAG_FOCALPLANEUNITS    0xa210
#define TAG_EXPOSURE_INDEX     0xa215
#define TAG_EXPOSURE_MODE      0xa402
#define TAG_WHITEBALANCE       0xa403
#define TAG_DIGITALZOOMRATIO   0xA404
#define TAG_FOCALLENGTH_35MM   0xa405

const int BytesPerFormat[] = {0,1,1,2,4,8,1,1,2,4,8,4,8};

//holds the jpeg file sections in memory
typedef struct {
    uchar *  Data;
    int      Type;
    unsigned Size;
}Section_t;

//holds the hex value for the tag
typedef struct {
    unsigned short Tag;
    char * Desc;
}TagTable_t;

class c_Exif
{
public:
	//constuctor
	c_Exif(const char * FileName);
	~c_Exif();

	//accessor functions
	std::string GetDate();
	std::string GetTime();
	
private:
	unsigned char * LastExifRefd;
	unsigned char * DirWithThumbnailPtrs;
	double FocalplaneXRes;
	double FocalplaneUnits;
	int ExifImageWidth;
	int MotorolaOrder;
	uchar DateTime [20];
	int  numDateTimeTags;
	std::string Date;
	std::string Time;
	int SectionsRead;
	int HaveAll;
	Section_t Sections[MAX_SECTIONS];

	// for fixing the rotation.  This is for future implemtation
	void * OrientationPtr;
	int    OrientationNumFormat;

	//processes the directories in the exif header.
	//It uses recursion because there could be many sub directories
	void ProcessExifDir(unsigned char * DirStart, unsigned char * OffsetBase, 
        unsigned ExifLength, int NestingLevel);
	//process the EXIF header
	void process_EXIF (unsigned char * ExifSection, unsigned int length);
	//process the different flags with in the jpeg header
	int ReadJpegSections (FILE * infile);
	//opens the file to be read
	int ReadJpegFile(const char * FileName);

	void ConvertDateTime();
	
			
};

#endif