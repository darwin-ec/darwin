//*******************************************************************
//   file: ColorImage.cxx
//
// author: Adam Russell
//
//   mods: J H Stewman (7/22/2005)
//         -- reformatting of code and addition of comment blocks
//
//*******************************************************************

#include "ColorImage.h"
#include "../utility.h"
#ifndef WIN32
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#include "../../pixmaps/question.xpm" //***1.5
#include "../../pixmaps/fnf.xpm" //***1.5

using namespace std;

void ColorImage::createDefaultImageFromXPM(char **xpm)
{
	int colors, chPerColor;

	sscanf(xpm[0], "%d %d %d %d", &mCols, &mRows, &colors, &chPerColor);

	string line, colorCode;
	//char colorCodeTemp[8];
	//char colorValTemp[32];
	string colorValStr;
	ColorPixel cVal;
	map<string,ColorPixel> colorValue;
		
	for (int c = 0; c < colors; c++)
	{
		line = xpm[c+1];
		colorCode = line.substr(0,chPerColor);
		line = line.substr(line.find('c',chPerColor)+2);

		colorValStr = line;
		if (colorValStr == "None")
			cVal.setRGB(0,0,0);
		else
		{
			int r,g,b;
			r = hexPairToNum(colorValStr[1], colorValStr[2]);
			g = hexPairToNum(colorValStr[3], colorValStr[4]);
			b = hexPairToNum(colorValStr[5], colorValStr[6]);
			cVal.setRGB(r,g,b);
		}
		colorValue[colorCode] = cVal;
	}

	mInitialized = true;
	mData = new ColorPixel[mRows * mCols];
	for (int r = 0; r < mRows; r++)
	{
		line = xpm[r+1+colors];
		for (int c = 0; c < mCols; c++)
			mData[r*mCols + c] = colorValue[line.substr(c*chPerColor,chPerColor)];
	}
}

//*******************************************************************
//
// ColorImage::ColorImage(const std::string &filename)
//
//    CONSTRUCTOR - Creates ColorImage from named file.
//
//***1.5 - this is NO LONGER an inline function
//
ColorImage::ColorImage(const std::string &filename)
	: ImageFile<ColorPixel>(/*filename*/)
{
	try
	{
		ImageFile<ColorPixel>::load(filename);
	}
	catch (ImageFileNotFound fnf)
	{
		//***1.5 - image file not found
		printf("[ImgName] %s\n",filename.c_str());
		createDefaultImageFromXPM(fnf_xpm); // was question_xpm
	}
	catch (Error e)
	{
		throw;
	}
}

//*******************************************************************
//
// ColorImage::ColorImage(unsigned nRows, unsigned nCols, GrayPixel *data)
//
//    CONSTRUCTOR
//
//    PRE: Parameters specify a nonempty image
//
ColorImage::ColorImage(unsigned nRows, unsigned nCols, GrayPixel *data)
	: ImageFile<ColorPixel>() //***1.0LK
{
	if (nRows == 0 || nCols == 0)
		throw Error("Bad image dimensions in ColorImage ctor.");

	if (NULL == data)
		throw Error("NULL data in ColorImage ctor.");

	mRows = nRows;
	mCols = nCols;

	mData = new ColorPixel[mRows * mCols];

	for (unsigned r = 0; r < mRows; r++)
		for (unsigned c = 0; c < mCols; c++)
			mData[r * mCols + c] = data[r * mCols + c].getIntensity();
	
	mInitialized = true;
}

//*******************************************************************
//
// ColorImage::~ColorImage()
//
//    DESTRUCTOR - mData must be freed
//
/*ColorImage::~ColorImage()
{
	if (NULL != mData)
		delete mData;
}
*/
//*******************************************************************
//
// ColorImage operator/(const ColorImage& numerator, const ColorImage& denominator)
//
//    Not in use at this time. Is this needed?
//
/*
ColorImage operator/(const ColorImage& numerator, const ColorImage& denominator)
{
	unsigned numRows = numerator.mRows;
	unsigned numCols = numerator.mCols;
	
	if (numRows != denominator.mRows || numCols != denominator.mCols)
		throw ImageSizeMismatch("operator/");
	
	// allocate enough room to store the image in a float array
	float *redTemp = new float[numRows * numCols];
	float *greenTemp = new float[numRows * numCols];
	float *blueTemp = new float[numRows * numCols];
	
	for (unsigned r = 0; r < numRows; r++) {
		for (unsigned c = 0; c < numCols; c++) {

			// First, we'll do the Red channel
			if (denominator.mData[r * numCols + c].getRed() == 0) // we don't want to be dividing by zero
				redTemp[r * numCols + c] = (float)numerator.mData[r * numCols + c].getRed();
			else
				redTemp[r * numCols + c] = (float) numerator.mData[r * numCols + c].getRed() / denominator.mData[r * numCols + c].getRed();
		
			// Then, the Blue...
			if (denominator.mData[r * numCols + c].getBlue() == 0) // we don't want to be dividing by zero
				blueTemp[r * numCols + c] = (float)numerator.mData[r * numCols + c].getBlue();
			else
				blueTemp[r * numCols + c] = (float)numerator.mData[r * numCols + c].getBlue() / denominator.mData[r * numCols + c].getBlue();

			// And, finally, the Green... did you notice it's not in rgb order? =/ hmm...
			if (denominator.mData[r * numCols + c].getGreen() == 0) // we don't want to be dividing by zero
				greenTemp[r * numCols + c] = (float)numerator.mData[r * numCols + c].getGreen();
			else
				greenTemp[r * numCols + c] = (float)numerator.mData[r * numCols + c].getGreen() / denominator.mData[r * numCols + c].getGreen();
		}
	}
	unsigned redMin, redMax;
	numerator.findMinMaxRed(redMin, redMax);

	unsigned greenMin, greenMax;
	numerator.findMinMaxGreen(greenMin, greenMax);

	unsigned blueMin, blueMax;
	numerator.findMinMaxBlue(blueMin, blueMax);

	ColorImage temp(numRows, numCols);
	temp.rescale(redTemp, greenTemp, blueTemp,
			redMin, redMax,
			greenMin, greenMax,
			blueMin, blueMax);

        delete[] redTemp;
	delete[] greenTemp;
	delete[] blueTemp;

	return temp;
}
*/


//*******************************************************************
//
// GrayImage* ColorImage::redImage() const
//
//    Returns red coponent of ColorImage as a GrayImage.
//
GrayImage* ColorImage::redImage() const
{
	GrayImage* redImage = new GrayImage(mRows, mCols);

	for (unsigned r = 0; r < mRows; r++)
		for (unsigned c = 0; c < mCols; c++)
			(*redImage)(r, c) = mData[r * mCols + c].getRed();
	return redImage;
}


//*******************************************************************
//
//
// GrayImage* ColorImage::greenImage() const
//
//    Returns green coponent of ColorImage as a GrayImage.
//
GrayImage* ColorImage::greenImage() const
{
	GrayImage* greenImage = new GrayImage(mRows, mCols);

	for (unsigned r = 0; r < mRows; r++)
		for (unsigned c = 0; c < mCols; c++)
			(*greenImage)(r, c) = mData[r * mCols + c].getGreen();
	return greenImage;
}


//*******************************************************************
//
//
// GrayImage* ColorImage::blueImage() const
//
//    Returns blue coponent of ColorImage as a GrayImage.
//
GrayImage* ColorImage::blueImage() const
{
	GrayImage* blueImage = new GrayImage(mRows, mCols);

	for (unsigned r = 0; r < mRows; r++)
		for (unsigned c = 0; c < mCols; c++)
			(*blueImage)(r, c) = mData[r * mCols + c].getBlue();
	return blueImage;
}


//*******************************************************************
//
// void ColorImage::findMinMaxRed(unsigned& min, unsigned& max) const
//
//    Returns MINIMUM and MAXIMUM red values of image.
//
//
void ColorImage::findMinMaxRed(unsigned& min, unsigned& max) const
{
	max = min = (unsigned)mData[0].getRed();

	for (unsigned r  = 0; r < mRows; r++) {
		for (unsigned c = 0; c < mCols; c++) {
			if (mData[r * mCols + c].getRed() > max)
				max = (unsigned)mData[r * mCols + c].getRed();

			if (mData[r * mCols + c].getRed() < min)
				min = (unsigned)mData[r * mCols + c].getRed();
		}
	}
}


//*******************************************************************
//
//
// void ColorImage::findMinMaxGreen(unsigned& min, unsigned& max) const
//
//    Returns MINIMUM and MAXIMUM green values of image.
//
//
void ColorImage::findMinMaxGreen(unsigned& min, unsigned& max) const
{
	max = min = (unsigned)mData[0].getGreen();

	for (unsigned r  = 0; r < mRows; r++) {
		for (unsigned c = 0; c < mCols; c++) {
			if (mData[r * mCols + c].getGreen() > max)
				max = (unsigned)mData[r * mCols + c].getGreen();

			if (mData[r * mCols + c].getGreen() < min)
				min = (unsigned)mData[r * mCols + c].getGreen();
		}
	}
}


//*******************************************************************
//
//
// void ColorImage::findMinMaxBlue(unsigned& min, unsigned& max) const
//
//    Returns MINIMUM and MAXIMUM blue values of image.
//
//
void ColorImage::findMinMaxBlue(unsigned& min, unsigned& max) const
{
	max = min = (unsigned)mData[0].getBlue();

	for (unsigned r  = 0; r < mRows; r++) {
		for (unsigned c = 0; c < mCols; c++) {
			if (mData[r * mCols + c].getBlue() > max)
				max = (unsigned)mData[r * mCols + c].getBlue();

			if (mData[r * mCols + c].getBlue() < min)
				min = (unsigned)mData[r * mCols + c].getBlue();
		}
	}
}


//*******************************************************************
//
// void ColorImage::rescale(...)
//
//    Only called from operator/ function taht is commented out above.
//    What is purose of this function?
//
void ColorImage::rescale(float *redTemp, float *greenTemp, float *blueTemp,
		unsigned redMin, unsigned redMax,
		unsigned greenMin, unsigned greenMax,
		unsigned blueMin, unsigned blueMax)
{
	float
		newRedMin, newRedMax,
		newGreenMin, newGreenMax,
		newBlueMin, newBlueMax;

	newRedMin = newRedMax = redTemp[0];
	newGreenMin = newGreenMax = greenTemp[0];
	newBlueMin = newBlueMax = blueTemp[0];

	for (unsigned r = 0; r < mRows; r++) {
		for (unsigned c = 0; c < mCols; c++) {
			
			// The red...
			if (redTemp[r * mCols + c] > newRedMax)
				newRedMax = redTemp[r * mCols + c];
			if (redTemp[r * mCols + c] < newRedMin)
				newRedMin = redTemp[r * mCols + c];

			// et, the green...
			if (greenTemp[r * mCols + c] > newGreenMax)
				newGreenMax = greenTemp[r * mCols + c];
			if (greenTemp[r * mCols + c] < newGreenMin)
				newGreenMin = greenTemp[r * mCols + c];

			// and last, but not least, blue...
			if (blueTemp[r * mCols + c] > newBlueMax)
				newBlueMax = blueTemp[r * mCols + c];
			if (blueTemp[r * mCols + c] < newBlueMin)
				newBlueMin = blueTemp[r * mCols + c];
		}
	}

	float redScale = (float)(redMax - redMin) / (newRedMax - newRedMin);
	float greenScale = (float)(greenMax - greenMin) / (newGreenMax - newGreenMin);
	float blueScale = (float)(blueMax - blueMin) / (newBlueMax - newBlueMin);

	for (unsigned i = 0; i < mRows; i++) {
		for (unsigned j = 0; j < mCols; j++) {
                        mData[i * mCols + j].setRGB(
					(int)round(redScale * redTemp[i * mCols + j]),
					(int)round(greenScale * greenTemp[i * mCols + j]),
					(int)round(blueScale * blueTemp[i * mCols + j]));
		}
	}
}
