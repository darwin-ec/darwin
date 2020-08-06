//*******************************************************************
//   file: waveletUtil.cxx
//
// author: ?
//
//   mods:
//
//*******************************************************************

#include <cstdio>
#include <fstream>
#include "Chain.h"
#include "utility.h"
#include "waveletUtil.h"
#include "wavelet/wlcore.h"

using namespace std;

static WL_Filter
	gMZLowPassFilter = {
		{"mz", "sbfilter", NULL},
		4,
		2,
		NULL //{0.125, 0.375, 0.375, 0.125}
	},

	gMZHighPassFilter = {
		{"mz", "sbfilter", NULL},
		2,
		0,
		NULL // ­2.0, 2.0
	};

static const double NORMALIZATION_COEFFICIENTS[5] =
    { 1.50, 1.12, 1.03, 1.01, 1.0 };

double normalizationCoeff(int level)
{
    if (level <= 0)
	return 0.0;

    if (level < 5)
	return NORMALIZATION_COEFFICIENTS[level - 1];

    return 1.0;
}

void modulusMaxima(const double *src, double *modmax, int length)
{
    if (length < 2)
	throw Error("Bad length argument in modulusMaxima()");

    for (int k = 0; k < length; k++)
	modmax[k] = 0.0;

    bool increase = false;

    if (src[1] >= src[0])
	increase = true;

    // this is sloppy, i know
    // ... i can't think today
    for (int m = 1; m < length; m++) {
	if (increase) {
	    for (; m < length; m++) {
		if (src[m] < src[m - 1]) {
		    modmax[m - 1] = src[m - 1];
		    increase = false;
		    break;
		}
	    }
	} else {
	    for (; m < length; m++) {
		if (src[m] > src[m - 1]) {
		    modmax[m - 1] = src[m - 1];
		    increase = true;
		    break;
		}
	    }
	}
    }
}
	
void waveGenCoeffFiles(
	const string &name,
	const Chain *chain,
	int levels)
{
    if (NULL == chain || levels < 0)
	throw Error("Invalid argument to waveGenCoeffFiles()");

    int numPoints = chain->length();

    try {
	// First, make a copy without the first value in the chain,
	// since the first value skews the rest of the chain and is
	// unnecessary for our purposes here
	double *src = new double[numPoints - 1];

	memcpy(src, &((*chain)[1]), (numPoints - 1) * sizeof(double));
	// Now set up the variables needed to perform a wavelet
	// transform on the chain
	double **continuousResult;
	continuousResult = (double **) WL_Calloc2Dmem(levels + 1,
						      nextPowerOfTwo
						      (numPoints - 1),
						      sizeof(double)
	    );

	// Now perform the transformation
	WL_FrwtVector(src,
		      continuousResult,
		      numPoints - 1,
		      levels,
		      getMZLowPassFilter(), getMZHighPassFilter());

	char filename[200];
	sprintf(filename, "transforms/%s-chain", name.c_str());
	ofstream out(filename);

	for (int c = 0; c < numPoints - 1; c++)
	    out << src[c] << endl;
	out.close();

	for (int i = 1; i <= levels; i++) {
	    char fname[200];
	    sprintf(fname, "transforms/%s-level%d", name.c_str(), i);
	    ofstream outFile(fname);
	    for (int j = 0; j < numPoints - 1; j++)
		outFile << continuousResult[i][j] *
		    normalizationCoeff(i) << endl;
	    outFile.close();
	}

	// ... and clean up
	WL_Free2Dmem(continuousResult);
	delete[]src;

    } catch(...) {
	throw;
    }
}

WL_Filter* getMZLowPassFilter()
{
	return &gMZLowPassFilter;
}

WL_Filter* getMZHighPassFilter()
{
	return &gMZHighPassFilter;
}

void initFilters()
{
	gMZLowPassFilter.coefs = new double[4];

	gMZLowPassFilter.coefs[0] = 0.125;
	gMZLowPassFilter.coefs[1] = 0.375;
	gMZLowPassFilter.coefs[2] = 0.375;
	gMZLowPassFilter.coefs[3] = 0.125;
	
	gMZHighPassFilter.coefs = new double[2];

	gMZHighPassFilter.coefs[0] = -2.0;
	gMZHighPassFilter.coefs[1] = 2.0;
}

void destroyFilters()
{
	delete[] gMZLowPassFilter.coefs;
	delete[] gMZHighPassFilter.coefs;
}
