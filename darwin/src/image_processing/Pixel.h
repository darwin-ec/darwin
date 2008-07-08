//*******************************************************************
//   file: Pixel.h
//
// author: Adam Russell
//
//   mods:
//
//*******************************************************************

#ifndef PIXEL_H
#define PIXEL_H

#include "types.h"
#include "../utility.h"

using namespace std;

const unsigned MAX_PIXEL_INTENSITY = 255;

// ColorPixel and GrayPixel should really both be derived from a 
// common abstract class.  However, doing this means the sizeof()
// these classes won't be entirely dependent on their data members.
// So, because I'd really like to use write() and read() to do
// operations on an entire array of pixels at once, rather than
// calling a member function for each pixel, I'm making them
// separate, technically unrelated classes.  Because of the way
// they're being used, though, they have to have the same member
// functions, and each of these member functions has to operate
// similarly.  Bad design in favor of greater speed.

class ColorPixel {
public:
	ColorPixel& operator= (const ColorPixel& p);
	ColorPixel& operator= (unsigned i);

	bool operator> (unsigned i) const;
	bool operator< (unsigned i) const;
	bool operator<=(int i) const;
	bool operator>=(int i) const;
	bool operator==(unsigned i) const;
	bool operator==(const ColorPixel& p) const;
	bool operator!=(unsigned i) const;

/*	friend int operator-(const ColorPixel& p, int i);
	friend int operator-(int i, const ColorPixel& p);
	friend ColorPixel operator-(const ColorPixel& left, const ColorPixel& right);
	
	friend int operator+(const ColorPixel& p, int i);
	friend int operator+(int i, const ColorPixel& p);
	friend ColorPixel operator+(const ColorPixel& left, const ColorPixel& right);
		
	friend ColorPixel operator/(const ColorPixel& left, const ColorPixel& right);

	friend ColorPixel operator*(const ColorPixel& left, float right);
*/
	ColorPixel(byte r, byte g, byte b);
	ColorPixel();

	byte getRed() const;
	byte getGreen() const;
	byte getBlue() const;
	bool setRGB(byte r, byte g, byte b); // This is bad

	byte getIntensity() const;
	bool setIntensity(unsigned i);

	bool isColor() const;

	//103AT SAH
	byte getCyan() const;//103AT SAH
	byte getMagenta() const;//103AT SAH
	byte getYellow() const;//103AT SAH
	byte getBlack() const;//103AT SAH
	bool setCMKY(byte c, byte m, byte y, byte k);//103AT SAH

protected:
	byte mRed;
	byte mGreen;
	byte mBlue;
};

class GrayPixel {
public:
	GrayPixel& operator=(const GrayPixel& p);
	GrayPixel& operator=(unsigned i);
	GrayPixel& operator=(const ColorPixel& p); 

	bool operator>(unsigned i) const;
	bool operator<(unsigned i) const;
	bool operator<=(unsigned i) const;
	bool operator>=(unsigned i) const;

	bool operator==(unsigned i) const;
	bool operator!=(unsigned i) const;
	
	friend inline int operator*(const GrayPixel& p, int i);
	friend inline int operator*(int i, const GrayPixel& p);
	friend inline int operator*(const GrayPixel& left, const GrayPixel& right);

	friend inline int operator/(const GrayPixel& p, int i);
	friend inline int operator/(int i, const GrayPixel& p);
	friend inline int operator/(const GrayPixel& left, const GrayPixel& right);

	friend inline int operator+(const GrayPixel& p, int i);
	friend inline int operator+(int i, const GrayPixel& p);
	friend inline int operator+(const GrayPixel& left, const GrayPixel& right);

//	friend inline int operator-(const GrayPixel& p, int i);
//	friend inline int operator-(int i, const GrayPixel& p);
//	friend inline int operator-(const GrayPixel& left, const GrayPixel& right);

	friend inline int operator%(const GrayPixel& p, int i);
	friend inline int operator%(int i, const GrayPixel& p);
	friend inline int operator%(const GrayPixel& left, const GrayPixel& right);

	GrayPixel(byte r, byte g, byte b);
	GrayPixel();

	byte getRed() const;
	byte getGreen() const;
	byte getBlue() const;
	bool setRGB(byte r, byte g, byte b); // This is bad

	byte getIntensity() const;
	bool setIntensity(byte i);

	bool isColor() const;
protected:
	// Only 256 shades.. gotta fix this at some point
	byte mIntensity;
};

class BinaryPixel {
public:
	BinaryPixel() : mValue(BLACK)
	{ }

	BinaryPixel& operator=(unsigned i);
	BinaryPixel& operator=(const GrayPixel& p);
	BinaryPixel& operator=(const ColorPixel& p);

	bool operator==(bool i) const {
		return (i == mValue);
	}

	bool setRGB(byte r, byte g, byte b) {
		if (r > 0 || g > 0 || b > 0)
			mValue = WHITE;
		else
			mValue = BLACK;

		return true;
	}
	
	byte getIntensity() const {
		if (mValue == WHITE)
			return MAX_PIXEL_INTENSITY;
		else
			return 0;
	}
	
	bool setIntensity(byte i) {
		if (i > 0)
			mValue = WHITE;
		else
			mValue = BLACK;

		return true;
	}

	bool isColor() const { return false; }

protected:
	bool mValue;
};


inline
ColorPixel::ColorPixel()
	: mRed (0),
	  mGreen (0),
	  mBlue (0)
{ }

inline
ColorPixel::ColorPixel(byte r, byte g, byte b)
	: mRed (r),
	  mGreen (g),
	  mBlue (b)
{ }

inline
ColorPixel& ColorPixel::operator= (const ColorPixel& p)
{
	// Not that this operator is really necessary
	// since this class has no dynamically allocated
	// members
	
	if (this == &p) return *this; // handle self assignment

	mRed = p.getRed ();
	mGreen = p.getGreen ();
	mBlue = p.getBlue ();

	return *this;
}

inline
ColorPixel& ColorPixel::operator= (unsigned i)
{
	this->setIntensity(i);

	return *this;
}

inline
bool ColorPixel::operator> (unsigned i) const
{
	return ((int)i < (int)round(mRed * .299 + mGreen *.587 + mBlue *.114));
}

inline
bool ColorPixel::operator>=(int i) const
{
	return ((int)this->getIntensity() >= i);
}

inline
bool ColorPixel::operator< (unsigned i) const
{
	return ((int)i > (int)round(mRed * .299 + mGreen * .587 + mBlue * .114));
}

inline
bool ColorPixel::operator<=(int i) const
{
	return ((int)this->getIntensity() <= i);
}
inline
bool ColorPixel::operator== (unsigned i) const
{
	return ((int)i == (int) round(mRed * .299 + mGreen * .587 + mBlue * .114));
}

inline
bool ColorPixel::operator==(const ColorPixel& p) const
{
	if (p.mRed != mRed || p.mBlue != mBlue || p.mGreen != mGreen)
		return false;
	return true;
}

inline
bool ColorPixel::operator!=(unsigned i) const
{
	return ((int)i != (int) round(mRed * .299 + mGreen * .587 + mBlue * .114));
}

inline int operator-(const ColorPixel& p, int i)
{
	return (int)p.getIntensity() - i;
}

inline int operator-(int i, const ColorPixel& p)
{
	return i - (int)p.getIntensity();
}

inline ColorPixel operator-(const ColorPixel& left, const ColorPixel& right)
{
	ColorPixel p;

	int red, grn, blu;

	red = left.getRed() - right.getRed();
	grn = left.getGreen() - right.getGreen();
	blu = left.getBlue() - right.getBlue();

	if (red < 0)
		red = 0;
	else if (red > 255) // not really possible, but i'm paranoid
		red = 255;

	if (grn < 0)
		grn = 0;
	else if (grn > 255)
		grn = 255;

	if (blu < 0)
		blu = 0;
	else if (blu > 255)
		blu = 255;

	p.setRGB((byte)red, (byte)grn, (byte)blu);

	return p;
}

inline ColorPixel operator+(const ColorPixel& left, const ColorPixel& right)
{
	ColorPixel p;

	int red, grn, blu;

	red = left.getRed() + right.getRed();
	grn = left.getGreen() + right.getGreen();
	blu = left.getBlue() + right.getBlue();

	if (red < 0)
		red = 0;
	else if (red > 255)
		red = 255;

	if (grn < 0)
		grn = 0;
	else if (grn > 255)
		grn = 255;

	if (blu < 0)
		blu = 0;
	else if (blu > 255)
		blu = 255;

	p.setRGB((byte)red, (byte)grn, (byte)blu);

	return p;
}

inline ColorPixel operator*(const ColorPixel& left, float right)
{
	ColorPixel p;

	int red, grn, blu;

	red = (int) round(left.getRed() * right);
	grn = (int) round(left.getGreen() * right);
	blu = (int) round(left.getBlue() * right);

	if (red < 0)
		red = 0;
	else if (red > 255)
		red = 255;

	if (grn < 0)
		grn = 0;
	else if (grn > 255)
		grn = 255;

	if (blu < 0)
		blu = 0;
	else if (blu > 255)
		blu = 255;

	p.setRGB((byte)red, (byte)grn, (byte)blu);

	return p;
}

inline int operator+(const ColorPixel& p, int i)
{
	return (int)p.getIntensity() + i;
}

inline int operator+(int i, const ColorPixel& p)
{
	return i + (int)p.getIntensity();
}

inline ColorPixel operator/(const ColorPixel& left, const ColorPixel& right)
{
	ColorPixel p;
	
	if (0 == right.getIntensity()) return p;

	p = left;

	p.setIntensity(left.getIntensity() / right.getIntensity());
	
	return p;
}

inline
byte ColorPixel::getRed() const { return mRed; }

inline
byte ColorPixel::getGreen() const { return mGreen; }

inline
byte ColorPixel::getBlue() const { return mBlue; }

inline
bool ColorPixel::setRGB(byte r, byte g, byte b)
{
	mRed = r;
	mGreen = g;
	mBlue = b;

	return true;
}

//102AT -- OpenIssue: Should there exist a CMKYPixel in addition to the ColorPixel??
inline
byte ColorPixel::getCyan() const {//103AT SAH
	byte c=255-mRed;
	byte m=255-mGreen;
	byte y=255-mBlue;
	
	byte k = getMin(c,m,y);

	if (k==255) {
		c=0;
		m=0;
		y=0;
	} else {
		c=(c-k);
		m=(m-k);
		y=(y-k);
	}

	return c;
}

inline
byte ColorPixel::getMagenta() const {//103AT SAH
	byte c=255-mRed;
	byte m=255-mGreen;
	byte y=255-mBlue;
	
	byte k = getMin(c,m,y);

	if (k==255) {
		c=0;
		m=0;
		y=0;
	} else {
		c=(c-k);
		m=(m-k);
		y=(y-k);
	}

	return m;
}

inline
byte ColorPixel::getYellow() const {//103AT SAH
	byte c=255-mRed;
	byte m=255-mGreen;
	byte y=255-mBlue;
	
	byte k = getMin(c,m,y);

	if (k==255) {
		c=0;
		m=0;
		y=0;
	} else {
		c=(c-k);
		m=(m-k);
		y=(y-k);
	}

	return y;
}

inline
byte ColorPixel::getBlack() const {//103AT SAH
	byte c=255-mRed;
	byte m=255-mGreen;
	byte y=255-mBlue;
	
	byte k = getMin(c,m,y);

	if (k==255) {
		c=0;
		m=0;
		y=0;
	} else {
		c=(c-k);
		m=(m-k);
		y=(y-k);
	}

	return k;
}

//103AT SAH
inline
bool ColorPixel::setCMKY(byte c, byte m, byte y, byte k) {//I don't know what it means that I'm copying a function with a comment "This is bad"
	mRed=255-(c+k);
	mGreen=255-(m+k);
	mBlue=255-(y+k);
}


inline
byte ColorPixel::getIntensity() const
{
	return ((byte) round (mRed * .299 + mGreen *.587 + mBlue *.114));
}

inline
bool ColorPixel::setIntensity(unsigned i)
{
	float inphase = 0.596f * mRed - 0.275f * mGreen - 0.321f * mBlue;
	float quadrature = 0.212f * mRed - 0.523f * mGreen + 0.311f * mBlue;

	int redTemp = (int) round(i + 0.956 * inphase + 0.621 * quadrature);
	int greenTemp = (int) round(i - 0.272 * inphase - 0.647 * quadrature);
	int blueTemp = (int) round(i - 1.108 * inphase + 1.705 * quadrature);

	if (redTemp > 255)
		mRed = 255;
	
	else if (redTemp < 0) // Just in case!
		mRed = 0;
	
	else
		mRed = (byte) redTemp;

	if (greenTemp > 255)
		mGreen = 255;
	
	else if (greenTemp < 0)
		mGreen = 0;
	
	else
		mGreen = (byte) greenTemp;

	if (blueTemp > 255)
		mBlue = 255;
	
	else if (blueTemp < 0)
		mBlue = 0;
	
	else
		mBlue = (byte) blueTemp;

	return true;
}

inline
bool ColorPixel::isColor() const
{
	return true;
}

inline
GrayPixel::GrayPixel()
	: mIntensity (0)
{ }

inline
GrayPixel::GrayPixel(byte r, byte g, byte b)
	: mIntensity ((byte) round(r * .299 + g * .587 + b * .114))
{ }

inline
GrayPixel& GrayPixel::operator= (const GrayPixel& p)
{
	// Not that this operator is really necessary
	// since this class has no dynamically allocated
	// members
	
	if (this == &p) return *this; // handle self assignment

	mIntensity = p.getIntensity();

	return *this;
}

inline
GrayPixel& GrayPixel::operator= (unsigned i)
{
	mIntensity = (byte)i;

	return *this;
}


inline
GrayPixel& GrayPixel::operator=(const ColorPixel& p)
{
	mIntensity = (byte) round(.299 * p.getRed() + .587 * p.getGreen() + .114 * p.getBlue());

	return *this;
}

inline
bool GrayPixel::operator> (unsigned i) const
{
	return (mIntensity > i);
}

inline
bool GrayPixel::operator<(unsigned i) const
{
	return (mIntensity < i);
}

inline
bool GrayPixel::operator<=(unsigned i) const
{
	return (mIntensity <= i);
}

inline
bool GrayPixel::operator>=(unsigned i) const
{
	return (mIntensity >= i);
}


inline int operator*(const GrayPixel& p, int i)
{
	return i * p.mIntensity;
}

inline int operator*(int i, const GrayPixel& p)
{
	return i * p.mIntensity;
}

inline int operator*(const GrayPixel& left, const GrayPixel& right)
{
	return ((int)left.mIntensity * (int)right.mIntensity);
}

inline int operator/(const GrayPixel& p, int i)
{
	if (0 == i) return 0;
	return p.mIntensity / i;
}

inline int operator/(const GrayPixel& left, const GrayPixel& right)
{
	if (0 == right.mIntensity) return 0;

	return ((int)left.mIntensity / (int)right.mIntensity);
}

inline int operator/(int i, const GrayPixel& p)
{
	if (0 == p.getIntensity()) return 0;

	return i / p.getIntensity();
}

inline int operator+(const GrayPixel& p, int i)
{
	return p.getIntensity() + i;
}

inline int operator+(int i, const GrayPixel& p)
{
	return p.getIntensity() + i;
}

inline int operator+(const GrayPixel& left, const GrayPixel& right)
{
	return ((int)left.getIntensity() + (int)right.getIntensity());
}

inline int operator-(const GrayPixel& p, int i)
{
	return p.getIntensity() - i;
}

inline int operator-(int i, const GrayPixel& p)
{
	return i - p.getIntensity();
}

inline int operator-(const GrayPixel& left, const GrayPixel& right)
{
	return ((int)left.getIntensity() - (int)right.getIntensity());
}

inline int operator%(const GrayPixel& p, int i)
{
	if (0 == i) return 0;

	return p.getIntensity() % i;
}

inline int operator%(int i, const GrayPixel& p)
{
	if (0 == p.getIntensity()) return 0;

	return i % p.getIntensity();
}

inline int operator%(const GrayPixel& left, const GrayPixel& right)
{
	if (0 == right.mIntensity) return 0;
	
	return ((int)left.getIntensity() % (int)right.getIntensity());
}
inline
bool GrayPixel::operator==(unsigned i) const
{
	return (mIntensity == i);
}

inline
bool GrayPixel::operator!=(unsigned i) const
{
	return (mIntensity != i);
}

inline
byte GrayPixel::getRed() const { return mIntensity; }

inline
byte GrayPixel::getGreen() const { return mIntensity; }

inline
byte GrayPixel::getBlue() const { return mIntensity; }

inline
bool GrayPixel::setRGB(byte r, byte g, byte b)
{
	mIntensity = (byte) round(r * .299 + g * .587 + b * .114);

	return true;
}

inline
byte GrayPixel::getIntensity() const
{
	return mIntensity;
}

inline
bool GrayPixel::setIntensity(byte i)
{
	mIntensity = i;
	return true;
}

inline
bool GrayPixel::isColor() const
{
	return false;
}

// BinaryPixel stuff
inline
BinaryPixel& BinaryPixel::operator= (const GrayPixel& p)
{
	if (p.getIntensity() > 0)
		mValue = WHITE;
	else
		mValue = BLACK;

	return *this;
}

inline
BinaryPixel& BinaryPixel::operator= (unsigned i)
{
	if (i > 0)
		mValue = WHITE;
	else
		mValue = BLACK;

	return *this;
}

inline
BinaryPixel& BinaryPixel::operator=(const ColorPixel& p)
{
	if (p.getRed() > 0 || p.getGreen() > 0 || p.getBlue() > 0)
		mValue = WHITE;
	else
		mValue = BLACK;

	return *this;
}

#endif
