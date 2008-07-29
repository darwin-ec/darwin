//*******************************************************************
//   file: utility.h
//
// author: ?
//
//   mods:
// 
//*******************************************************************

#ifndef UTILITY_HH
#define UTILITY_HH

#include <math.h>
#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include <ctype.h>
#include "constants.h"
#include <iostream>
#include <sstream>
#include <fstream>

float round(float n);
double round(double n);
std::string quoted(std::string& s);
std::string upperCase(std::string& s);
std::string lowerCase(std::string& s);
int byteStringToInt (unsigned char* byteString, int length);
int byteSwap (int num);
double rtod(double angle_radians);
double dtor(double angle_degrees);
double distance(double x1, double y1, double x2, double y2);
int nextPowerOfTwo(int x);
double log2(double x);
std::string convertTripletToHexString(unsigned char a, unsigned char b, unsigned char c);
std::string convertCharToHexString(unsigned char a);
void freePixmapString(char **pix, int numRows);

// makeVectorDyadic
// 	Makes the vector src's length a power of two by padding the end
// 	with 0.0.
void makeVectorDyadic(const double *src, int length, double **dst, int *dstLength);

inline
float round(float n) { return n + 0.5; }

inline
double round(double n) { return n + 0.5; }

// make a quoted "s" version of the string s
inline
std::string quoted(std::string& s)
{
	std::string r = "\"";
	r += s;
	r += "\"";
	return r;
}

// Make an uppercase copy of s
inline
std::string upperCase(std::string& s) {
	char* buf = new char[s.length()];
	s.copy (buf, s.length());

	for (unsigned int i =0;i < s.length(); i++)
		buf[i] = toupper(buf[i]);

	std::string r (buf, s.length());
	delete buf;
	return r;
}

// Make a lowercase copy of s
inline
std::string lowerCase(std::string& s) {
	char* buf = new char[s.length()];
	s.copy (buf, s.length());

	for (unsigned int i =0;i < s.length(); i++)
		buf[i] = tolower(buf[i]);

	std::string r (buf, s.length());
	delete buf;
	return r;
}

inline
int byteStringToInt (unsigned char* byteString, int length)
{
	return (byteString[0] << 24) | (byteString[1] << 16) | (byteString[2] << 8) | byteString[3];
}

inline
int byteSwap (int num)
{
	return ((num&0xFF000000)>>24 | (num&0x00FF0000)>>8 | (num&0xFF00)<<8 | (num&0xFF)<<24);
}

/*****************************************************************************/
/*TAKES A DOUBLE RADIAN AND RETURNS A ROUNDED DEGREE INTEGER*/
inline
double rtod(double angle_radians)
{
    double temp = 0.0;

    temp = angle_radians * 180 / PI;
    return temp;
}

/*****************************************************************************/
/*TAKES A DOUBLE DEGREE MEASURE AND RETURNS AN APPROXIMATED RADIAN MEASURE*/
inline
double dtor(double angle_degrees)
{
    double temp = 0.0;

    temp = angle_degrees * PI / 180;
    return temp;
}

inline
double distance(double x1, double y1, double x2, double y2)
{
	return (sqrt((x2 - x1)*(x2 - x1) + (y2 - y1) *(y2 - y1)));
}

inline
void makeVectorDyadic(
		const double *src,
		int length,
		double **dst,
		int *dstLength)
{
	if (length <= 0) {
		*dst = NULL;
		*dstLength = 0;
		return;
	}

	double powerOfTwo = 0.0;

	for (int i = 0; (double)length > powerOfTwo; i++)
		powerOfTwo = pow(2, i);

	*dstLength = (int) powerOfTwo;

	*dst = new double[*dstLength];

	memcpy(*dst, src, length * sizeof(double));

	for (int j = length; j < *dstLength; j++)
		(*dst)[j] = 0.0;
}

inline int nextPowerOfTwo(int x)
{
	double powerOfTwo = 0.0;

	for (int i = 0; (double)x > powerOfTwo; i++)
		powerOfTwo = pow(2, i);

	return (int)powerOfTwo;
}

inline
double log2(double x)
{
	return (log(x) / log(2));
}
inline
std::string convertTripletToHexString(
		unsigned char a,
		unsigned char b,
		unsigned char c)
{
	std::string
		aString = convertCharToHexString(a),
		bString = convertCharToHexString(b),
		cString = convertCharToHexString(c);

	return aString + bString + cString;
}

inline
std::string convertCharToHexString(unsigned char a)
{
	// I'm retarded at the moment.. so this function is ugly
	int secondHexVal = ((int) a) % 16;
	int firstHexVal = ((int) a) / 16;
	std::string hexString;

	// ok, this is ugly, I should have used an ostrstream for the
	// 0..9 digits cases
	switch (firstHexVal) {
		case 0:
			hexString = "0";
			break;
		case 1:
			hexString = "1";
			break;
		case 2:
			hexString = "2";
			break;
		case 3:
			hexString = "3";
			break;
		case 4:
			hexString = "4";
			break;
		case 5:
			hexString = "5";
			break;
		case 6:
			hexString = "6";
			break;
		case 7:
			hexString = "7";
			break;
		case 8:
			hexString = "8";
			break;
		case 9:
			hexString = "9";
			break;
		case 10:
			hexString = "A";
			break;
		case 11:
			hexString = "B";
			break;
		case 12:
			hexString = "C";
			break;
		case 13:
			hexString = "D";
			break;
		case 14:
			hexString = "E";
			break;
		default:
			hexString = "F";
			break;
	}

	switch (secondHexVal) {
		case 0:
			hexString += "0";
			break;
		case 1:
			hexString += "1";
			break;
		case 2:
			hexString += "2";
			break;
		case 3:
			hexString += "3";
			break;
		case 4:
			hexString += "4";
			break;
		case 5:
			hexString += "5";
			break;
		case 6:
			hexString += "6";
			break;
		case 7:
			hexString += "7";
			break;
		case 8:
			hexString += "8";
			break;
		case 9:
			hexString += "9";
			break;
		case 10:
			hexString += "A";
			break;
		case 11:
			hexString += "B";
			break;
		case 12:
			hexString += "C";
			break;
		case 13:
			hexString += "D";
			break;
		case 14:
			hexString += "E";
			break;
		default:
			hexString += "F";
			break;
	}
	return hexString;
}

inline
void freePixmapString(char **pix, int numRows)
{
	if (NULL == pix)
		return;

	for (int i = 0; i < numRows; i++)
		delete[] pix[i];
	delete[] pix;
}


//***1.5 - for manipulating xpm values read in hex
inline
unsigned char hexPairToNum (char first, char second)
{
	unsigned char val;

	if ('0' <= first && first <= '9')
		val = first - '0';
	else
		val = 10 + toupper(first) - 'A';
	val = val * 16;
	if ('0' <= second && second <= '9')
		val = val + (second - '0');
	else
		val = val + (10 + toupper(second) - 'A');

	return val;
}

//102AT, 103AT
inline
int getMin(int a, int b, int c) {//Isn't there a Math library call to do this?
	if (a<=b && a<=c) return a;
	else if (b<=a && b<=c) return b;
	else return c;
}

inline
void stripCRLF(std::string &line)
{
	for (int i = 0; i < line.length(); ++i)
		if (line[i] == '\r' || line[i] == '\n')
			line[i] = ' ';
}

inline
std::string extractBasename(std::string filename)
{
	int pos = filename.find_last_of(PATH_SLASH);
	
	if (pos >= 0)
			return filename.substr(pos+1);
	
	return filename;
}

inline
std::string extractPath(std::string filename)
{
	int pos = filename.find_last_of(PATH_SLASH);
	
	if (pos >= 0)
			return filename.substr(0, pos);
	
	return filename;
}


inline
void systemCopy(std::string src, std::string dest)
{
	std::string command = "copy";
	command += " \"" + src + "\"";
	command += " \"" + dest + "\"";
	system(command.c_str());
}

inline
void systemMkdir(std::string path)
{
	std::string cmd = "mkdir";
	cmd += " \"" + path + "\"";
	system(cmd.c_str());
}

inline
void systemRmdir(std::string path)
{
	std::string cmd = "rmdir /s /q";
	cmd += " \"" + path + "\"";
	system(cmd.c_str());
}

inline
void systemZip(std::string path, std::string archive)
{
	std::string cmd = "7z.exe a -tzip";
	cmd += " \"" + archive + "\" ";
	cmd += " \"" + path + "\" ";
	system(cmd.c_str());
}

inline
void systemUnzip(std::string archive, std::string dest)
{
	std::string cmd = "7z.exe x -aoa -o";
	cmd += "\"" + dest + "\" ";
	cmd += " \"" + archive + "\" ";
	system(cmd.c_str());
}

inline
bool fileExists(std::string filename)
{
	std::ifstream testFile(filename.c_str());
	if (! testFile.fail())
	{
		testFile.close();
		return true;
	}

	return false;
}

inline
std::string generateUniqueName(std::string filename)
{
	if(! fileExists(filename) || filename.find_last_of(".")==std::string::npos)
		return filename;


	int number = 1; // so we start with [2] below

	std::string first_half = filename.substr(0, filename.find_last_of(".")); // JHS
	std::string second_half = filename.substr(filename.find_last_of("."));
	std::string filename2 = filename;

	while (fileExists(filename2)) {
		std::stringstream s;
		s << first_half << "[" << (++number) << "]" << second_half;
		filename2=s.str();
	}
	return filename2;

}


#endif

