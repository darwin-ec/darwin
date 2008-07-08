//*******************************************************************
//   file: ErrorDialog.cxx
//
// author: Joshua Gregory (modification of Matthias Wadel's code)
//
//   mods: 
//
// An Exif Extractor Class by Joshua Gregory.  The function of the class 
// is to extract the time and date from a Exif header in a jpeg.  Code 
// was adapted from Jhead by Matthias Wandel www.sentex.net/~mwandel 
//
//*******************************************************************

#include "exif.h"


//--------------------------------------------------------------------------
// Convert a 16 bit unsigned value from file's native byte order
//--------------------------------------------------------------------------
void Put16u(void * Short, unsigned short PutValue, int MotorolaOrder)
{
	if (MotorolaOrder){
		((uchar *)Short)[0] = (uchar)(PutValue>>8);
		((uchar *)Short)[1] = (uchar)PutValue;
	}else{
		((uchar *)Short)[0] = (uchar)PutValue;
		((uchar *)Short)[1] = (uchar)(PutValue>>8);
	}
}

//--------------------------------------------------------------------------
// Convert a 16 bit unsigned value from file's native byte order
//--------------------------------------------------------------------------
int Get16u(void * Short, int MotorolaOrder)
{
	if (MotorolaOrder){
		return (((uchar *)Short)[0] << 8) | ((uchar *)Short)[1];
	}else{
		return (((uchar *)Short)[1] << 8) | ((uchar *)Short)[0];
	}
}

//--------------------------------------------------------------------------
// Convert a 32 bit signed value from file's native byte order
//--------------------------------------------------------------------------
int Get32s(void * Long, int MotorolaOrder)
{
	if (MotorolaOrder){
		return  ((( char *)Long)[0] << 24) | (((uchar *)Long)[1] << 16)
			  | (((uchar *)Long)[2] << 8 ) | (((uchar *)Long)[3] << 0 );
	}else{
		return  ((( char *)Long)[3] << 24) | (((uchar *)Long)[2] << 16)
			  | (((uchar *)Long)[1] << 8 ) | (((uchar *)Long)[0] << 0 );
	}
}

//--------------------------------------------------------------------------
// Convert a 32 bit unsigned value from file's native byte order
//--------------------------------------------------------------------------
unsigned Get32u(void * Long, int MotorolaOrder)
{
	return (unsigned)Get32s(Long, MotorolaOrder) & 0xffffffff;
}



c_Exif::c_Exif(const char *FileName)
{
	
	MotorolaOrder = 0;
	DateTime[0] = 'X'; //***1.8 - must be initialized to something that is NOT a number
	if (ReadJpegFile(FileName)==false)
	{
		//error processing here
	}

}

c_Exif::~c_Exif()
{
	int a;
	for (a=0;a<SectionsRead;a++)
	{
		free(Sections[a].Data);
	}
	SectionsRead=0;
	int HaveAll=0;
}

int c_Exif::ReadJpegFile(const char *FileName)
{

	FILE * infile;
	int ret;

	infile = fopen(FileName, "rb"); // Unix ignores 'b', windows needs it.

	if (infile == NULL) {
		//fprintf(stderr, "can't open '%s'\n",FileName);

		return false;
	}

	// Scan the JPEG headers.
	ret = ReadJpegSections(infile);
	if (!ret){

		//printf("Not JPEG: %s\n",FileName);
	}

	fclose(infile);
	     	
	return true;
}

int c_Exif::ReadJpegSections (FILE * infile)
{
	int a;
	
	SectionsRead=0;

	a = fgetc(infile);

	if (a != 0xff || fgetc(infile) != M_SOI){
		return false;
	}

	for(;;){
		int itemlen;
		int marker = 0;
		int ll,lh, got;
		uchar * Data;

		if (SectionsRead >= MAX_SECTIONS){
			//printf("Too many sections in jpg file");
			
		}
		
	for (a=0;a<7;a++){
		marker = fgetc(infile);
		if (marker != 0xff) break;

		if (a >= 6){
			//printf("too many padding bytes\n");
			return false;
		}
	}
	
	if (marker == 0xff)
	{
		// 0xff is legal padding, but if we get that many, something's wrong.
		//printf("too many padding bytes!");	
	}

	Sections[SectionsRead].Type = marker;
	
	// Read the length of the section.  You read in two 8 bit chars.
	//You then shift the first bits over and combined it to the second.
	lh = fgetc(infile);
	ll = fgetc(infile);
	itemlen = (lh << 8) | ll;

	if (itemlen < 2)
	{
		//printf("invalid marker");
	}
	
	Sections[SectionsRead].Size = itemlen;

	Data = (uchar *)malloc(itemlen);
	
	if (Data == NULL)
	{
		//printf("Could not allocate memory");
	}
        
	Sections[SectionsRead].Data = Data;

	// Store first two pre-read bytes.
	Data[0] = (uchar)lh;
	Data[1] = (uchar)ll;

	got = fread(Data+2, 1, itemlen-2, infile); // Read the whole section.
	if (got != itemlen-2)
	{
		//printf("Premature end of file?");
	}
	SectionsRead += 1;
	
	switch(marker){

		case M_SOS:
			//this is the start of the image.  If you want to load the jpg image
			//do it here.
			return true;
		case M_EOI:   // in case it's a tables-only JPEG stream
			return false;
		case M_EXIF:
			// Seen files from some 'U-lead' software with Vivitar scanner
			// that uses marker 31 for non exif stuff.  Thus make sure 
			// it says 'Exif' in the section before treating it as exif.
			if (memcmp(Data+2, "Exif", 4) == 0){
				process_EXIF(Data, itemlen);
			}else{
				// Discard this section.
				free(Sections[--SectionsRead].Data);
			}
			break;
		case M_SOF0: 
		case M_SOF1: 
		case M_SOF2: 
		case M_SOF3: 
		case M_SOF5: 
		case M_SOF6: 
		case M_SOF7: 
		case M_SOF9: 
		case M_SOF10:
		case M_SOF11:
		case M_SOF13:
		case M_SOF14:
		case M_SOF15:
			//this marker is useful for the image dimensions
			break;
		default:
			// Skip any other sections.
			break;
		}
	}
}

void c_Exif::process_EXIF (unsigned char * ExifSection, unsigned int length)
{
	int FirstOffset;

	FocalplaneXRes = 0;
	FocalplaneUnits = 0;
	ExifImageWidth = 0;
	OrientationPtr = NULL;
	
	{   
		// Check the EXIF header component
		static uchar ExifHeader[] = "Exif\0\0";
		if (memcmp(ExifSection+2, ExifHeader,6)){
			//printf("Incorrect Exif header",0,0);
			return;
		}
	}
	
	if (memcmp(ExifSection+8,"II",2) == 0){
		MotorolaOrder = 0;
	}else{
		if (memcmp(ExifSection+8,"MM",2) == 0){
			MotorolaOrder = 1;
		}else{
			return;
		}
	}

	// Check the next value for correctness.
	if (Get16u(ExifSection+10, MotorolaOrder) != 0x2a){
		return;
	}
	
	FirstOffset = Get32u(ExifSection+12, MotorolaOrder);
	if (FirstOffset < 8 || FirstOffset > 16){
		// I used to ensure this was set to 8 (website I used indicated its 8)
		// but PENTAX Optio 230 has it set differently, and uses it as offset. (Sept 11 2002)
        
	}

	LastExifRefd = ExifSection;
	DirWithThumbnailPtrs = NULL;

	// First directory starts 16 bytes in.  All offset are relative to 8 bytes in.
	ProcessExifDir(ExifSection+8+FirstOffset, ExifSection+8, length-6, 0);
}


void c_Exif::ProcessExifDir(unsigned char * DirStart, unsigned char * OffsetBase, 
		unsigned ExifLength, int NestingLevel)
{
    
//	uchar * DateTimePointers[MAX_DATE_COPIES];
	int de;
	int NumDirEntries;
	unsigned ThumbnailOffset = 0;
	unsigned ThumbnailSize = 0;
	char IndentString[25];

	if (NestingLevel > 4)
	{
		return;
	}

	memset(IndentString, ' ', 25);
	IndentString[NestingLevel * 4] = '\0';
	
	NumDirEntries = Get16u(DirStart, MotorolaOrder);
	#define DIR_ENTRY_ADDR(Start, Entry) (Start+2+12*(Entry))

	{
		unsigned char * DirEnd;
		DirEnd = DIR_ENTRY_ADDR(DirStart, NumDirEntries);
		if (DirEnd+4 > (OffsetBase+ExifLength)){
			if (DirEnd+2 == OffsetBase+ExifLength || DirEnd == OffsetBase+ExifLength){
				// Version 1.0 of josh exif 2000 would truncate a bit too much.
				// This also caught later on as well.
			}else{

				return;
			}
		}
		if (DirEnd > LastExifRefd) LastExifRefd = DirEnd;
	}

	for (de=0;de<NumDirEntries;de++){
		int Tag, Format, Components;
		unsigned char * ValuePtr;
		int ByteCount;
		uchar * DirEntry;
		DirEntry = DIR_ENTRY_ADDR(DirStart, de);

		Tag = Get16u(DirEntry, MotorolaOrder);
		Format = Get16u(DirEntry+2, MotorolaOrder);
		Components = Get32u(DirEntry+4, MotorolaOrder);

		//if ((Format-1) >= NUM_FORMATS) {
			// (-1) catches illegal zero case as unsigned underflows to positive large.
			//  continue;//remove if useless
		//}

		ByteCount = Components * BytesPerFormat[Format];
		
		if (ByteCount > 4){
			unsigned OffsetVal;
			OffsetVal = Get32u(DirEntry+8, MotorolaOrder);
			// If its bigger than 4 bytes, the dir entry contains an offset.
			if (OffsetVal+ByteCount > ExifLength){
				// Bogus pointer offset and / or bytecount value
				continue;//remove if useless
			}
			ValuePtr = OffsetBase+OffsetVal;
		}else{
			// 4 bytes or less and value is in the dir entry itself
			ValuePtr = DirEntry+8;
		}

		if (LastExifRefd < ValuePtr+ByteCount){
			// Keep track of last byte in the exif header that was actually referenced.
			// That way, we know where the discardable thumbnail data begins.
			LastExifRefd = ValuePtr+ByteCount;
		}
		
		switch(Tag)
		{
		
		//TAG_DATETIME TAG_DATETIME_ORIGINAL
		case TAG_DATETIME_ORIGINAL:
			strncpy((char *)DateTime, (char *)ValuePtr, 19);
			DateTime[19] = '\0'; //***1.8 - just to be sure it is terminated
		case TAG_DATETIME:
			if (!isdigit(DateTime[0]))
			{
				// If we don't already have a DATETIME_ORIGINAL, use whatever
				// time fields we may have.
				strncpy((char *)DateTime, (char *)ValuePtr, 19);
				DateTime[19] = '\0'; //***1.8 - just to be sure it is terminated
			}

			/*if (numDateTimeTags >= MAX_DATE_COPIES)
			{    
				break;//remove if useless
			}
			DateTimePointers[numDateTimeTags++] = ValuePtr;*/
			ConvertDateTime();
			break;
		case TAG_EXIF_OFFSET:
		case TAG_INTEROP_OFFSET:
			{
				unsigned char * SubdirStart;
				SubdirStart = OffsetBase + Get32u(ValuePtr, MotorolaOrder);
				ProcessExifDir(SubdirStart, OffsetBase, ExifLength, NestingLevel+1);
				continue;
			}
			break;
		}
	}
	
	// In addition to linking to subdirectories via exif tags, 
	// there's also a potential link to another directory at the end of each
	// directory.  this has got to be the result of a comitee!
	unsigned char * SubdirStart;
	unsigned Offset;

	if (DIR_ENTRY_ADDR(DirStart, NumDirEntries) + 4 <= OffsetBase+ExifLength)
	{
		Offset = Get32u(DirStart+2+12*NumDirEntries, MotorolaOrder);
		if (Offset)
		{
			SubdirStart = OffsetBase + Offset;
			if (SubdirStart > OffsetBase+ExifLength || SubdirStart < OffsetBase)
			{
				if (SubdirStart > OffsetBase && SubdirStart < OffsetBase+ExifLength+20)
				{
					// Jhead 1.3 or earlier would crop the whole directory!
					// As Jhead produces this form of format incorrectness, 
					// I'll just let it pass silently
				}
				else
				{
					if (SubdirStart <= OffsetBase+ExifLength)
					{
						ProcessExifDir(SubdirStart, OffsetBase, ExifLength, NestingLevel+1);
					}
				}
			}
		}
		else
		{
			// The exif header ends before the last next directory pointer.
		}
	}
}

void c_Exif::ConvertDateTime()
{

	string Year;
	string Month;
	string Day;
	
	string temp = string((char*)DateTime);
	
	Year = temp.substr(0,4);
	Month = temp.substr(5,2);
	Day = temp.substr(8,2);
		
	switch(atoi(Month.c_str()))
	{
	case 1:
		Month = "Jan";
		break;
	case 2:
		Month = "Feb";
		break;
	case 3:
		Month = "Mar";
		break;
	case 4:
		Month = "Apr";
		break;
	case 5:
		Month = "May";
		break;
	case 6:
		Month = "Jun";
		break;
	case 7:
		Month = "Jul";
		break;
	case 8:
		Month = "Aug";
		break;
	case 9:
		Month = "Sep";
		break;
	case 10:
		Month = "Oct";
		break;
	case 11:
		Month = "Nov";
		break;
	case 12:
		Month = "Dec";
		break;
	}

	Date = Day + "-" + Month + "-" + Year;
	Time = temp.substr(11,8);
	
	//printf("date: %s %s\n",Date.c_str(),Time.c_str());
}


string c_Exif::GetDate()
{
	return Date;
}

string c_Exif::GetTime()
{
	return Time;
}
