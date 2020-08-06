//*******************************************************************
//   file: waveletUtil.h
//
// author: ?
//
//   mods:
// 
//*******************************************************************

#ifndef WAVELETUTIL_H
#define WAVELETUTIL_H

#pragma warning(disable:4786) //***1.95 removes debug warnings in <string> <vector> <map> etc
#include <string>
#include "Chain.h"
#include "wavelet/wlcore.h"

void waveGenCoeffFiles(
		const std::string &name,
		const Chain *chain,
		int levels);

double normalizationCoeff(int level);
void modulusMaxima(const double *src, double *modmax, int length);

// ugly hack!!!
// (I didn't do this the right way because it's really simple...btw,
// this is so that a filter file doesn't have to be passed around with
// the darwin executable)
WL_Filter* getMZLowPassFilter();
WL_Filter* getMZHighPassFilter();

// needs to be called before the get*s above =( I call it in main.cxx
void initFilters();

void destroyFilters();

#endif
